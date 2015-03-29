/*
 * block_consistency_interpreter.cpp
 *
 *  Created on: Mar 27, 2015
 *      Author: njindal
 */

#include <block_consistency_interpreter.h>
#include "loop_manager.h"

namespace sip {

BlockConsistencyInterpreter::BlockConsistencyInterpreter(int num_workers, const SipTables& sipTables):
		SialxInterpreter(sipTables, NULL, NULL, NULL),
		num_workers_(num_workers), sip_tables_(sipTables), current_worker_(0),
		pc_of_previous_barrier_(-1), pc_of_next_barrier_(-1){
}

BlockConsistencyInterpreter::~BlockConsistencyInterpreter() {}

void BlockConsistencyInterpreter::handle_pardo_op(int &pc){
	/** What a particular worker would do */
//	LoopManager* loop = new StaticTaskAllocParallelPardoLoop(arg1(pc),
//					index_selectors(pc), data_manager_, sip_tables_,
//					worker_rank_, num_workers_);
	int num_where_clauses = arg2(pc);
	int num_indices = arg1(pc);
	LoopManager* loop = new BalancedTaskAllocParallelPardoLoop(num_indices,
				index_selectors(pc), data_manager_, sip_tables_, num_where_clauses,
				this, current_worker_, num_workers_, iteration_);
	loop_start(pc, loop);
}

void BlockConsistencyInterpreter::handle_sip_barrier_op(int pc){
	std::cout << "At barrier, Current Worker " << current_worker_ << ", pc_of_last_barrier_ = " << pc_of_previous_barrier_ << ", pc = " << pc << std::endl;
	if (current_worker_ >= 0 && current_worker_ < num_workers_){
		pc_of_next_barrier_ = pc;
		current_worker_ += 1;
		pc_ = pc_of_previous_barrier_ + 1;
	}

	if (current_worker_ == num_workers_){
		current_worker_ = 0;
		blocks_consistency_map_.clear();
		arrays_marked_for_deletion_.clear();
		pc_of_previous_barrier_ = pc;
		pc_ = pc_of_next_barrier_;
	}
}

void BlockConsistencyInterpreter::handle_program_end(int& pc){
	std::cout << " Current Worker " << current_worker_ << ", pc_of_last_barrier_ = " << pc_of_previous_barrier_ << ", pc = " << pc << std::endl;
	if (current_worker_ >= 0 && current_worker_ < num_workers_){
		pc_of_next_barrier_ = pc;
		current_worker_ += 1;
		pc_ = pc_of_previous_barrier_ + 1;
	}

	if (current_worker_ == num_workers_){
		current_worker_ = 0;
		blocks_consistency_map_.clear();
		arrays_marked_for_deletion_.clear();
		pc_of_previous_barrier_ = pc;
		pc_ = pc_of_next_barrier_;
	}
}

void BlockConsistencyInterpreter::handle_get_op(int pc) {
	BlockId id = get_block_id_from_selector_stack();
	int array_id = id.array_id();
	ArrayBlockConsistencyMap& array_consistency_map = blocks_consistency_map_[array_id];
	DistributedBlockConsistency& distributed_block_consistency_map = array_consistency_map[id];
	bool consistent = distributed_block_consistency_map.update_and_check_consistency(SIPMPIConstants::GET, current_worker_);
	if (!consistent){
		std::stringstream err_ss;
		err_ss << "Incorrect GET Block Semantics for " << id << " from worker " << current_worker_ << " at line " << line_number();
		fail_with_exception(err_ss.str(), line_number());
	}
}

void BlockConsistencyInterpreter::handle_put_accumulate_op(int pc) {
	BlockId rhs_id = get_block_id_from_selector_stack();
	BlockId lhs_id = get_block_id_from_selector_stack();
	int array_id = lhs_id.array_id();
	ArrayBlockConsistencyMap& array_consistency_map = blocks_consistency_map_[array_id];
	DistributedBlockConsistency& distributed_block_consistency_map = array_consistency_map[lhs_id];
	bool consistent = distributed_block_consistency_map.update_and_check_consistency(SIPMPIConstants::PUT_ACCUMULATE, current_worker_);
	if (!consistent){
		std::stringstream err_ss;
		err_ss << "Incorrect PUT_ACCUMULATE Block Semantics for " << lhs_id << " from worker " << current_worker_ << " at line " << line_number();
		fail_with_exception(err_ss.str(), line_number());
	}
}

void BlockConsistencyInterpreter::handle_put_replace_op(int pc) {
	BlockId rhs_id = get_block_id_from_selector_stack();
	BlockId lhs_id = get_block_id_from_selector_stack();
	int array_id = lhs_id.array_id();
	ArrayBlockConsistencyMap& array_consistency_map = blocks_consistency_map_[array_id];
	DistributedBlockConsistency& distributed_block_consistency_map = array_consistency_map[lhs_id];
	bool consistent = distributed_block_consistency_map.update_and_check_consistency(SIPMPIConstants::PUT, current_worker_);
	if (!consistent){
		std::stringstream err_ss;
		err_ss << "Incorrect PUT Block Semantics for " << lhs_id << " from worker " << current_worker_ << " at line " << line_number();
		fail_with_exception(err_ss.str(), line_number());
	}

}

void BlockConsistencyInterpreter::handle_delete_op(int pc){
	int array_id = arg0(pc);
	arrays_marked_for_deletion_[array_id] = true;
}


void BlockConsistencyInterpreter::handle_execute_op(int pc) {

	int num_args = arg1(pc) ;
	int func_slot = arg0(pc);

	// Do the super instruction if the number of args is > 1
	// and if the parameters are only written to (all 'w')
	// and if all the arguments are either static or scalar
	bool all_write = true;
	if (num_args >= 1){
		const std::string signature(
				sip_tables_.special_instruction_manager().get_signature(func_slot));
		for (int i=0; i<signature.size(); i++){
			if (signature[i] != 'w'){
				all_write = false;
				break;
			}
		}
	} else {
		all_write = false;
	}
	bool all_scalar_or_static = true;

	std::string opcode_name = sip_tables_.special_instruction_manager().name(func_slot);
	std::list<BlockSelector> bs_list;
	for (int i=0; i<num_args; i++){
		BlockSelector& bs = block_selector_stack_.top();
		bs_list.push_back(bs);
		if (!sip_tables_.is_contiguous(bs.array_id_) && !sip_tables_.is_scalar(bs.array_id_))
			all_scalar_or_static = false;

		block_selector_stack_.pop();
	}

	if (all_write && all_scalar_or_static){
		SIP_LOG(std::cout << "Executing Super Instruction "
				<< sip_tables_.special_instruction_manager().name(func_slot)
				<< " at line " << line_number() << std::endl);
		// Push back blocks on stack and execute the super instruction.
		while (!bs_list.empty()){
			BlockSelector bs = bs_list.back();
			block_selector_stack_.push(bs);
			bs_list.pop_back();
		}
		SialxInterpreter::handle_execute_op(pc);
	}



}



} /* namespace sip */
