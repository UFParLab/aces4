/*
 * block_consistency_interpreter.cpp
 *
 *  Created on: Mar 27, 2015
 *      Author: njindal
 */

#include <block_consistency_interpreter.h>
#include "loop_manager.h"

namespace sip {

BlockConsistencyInterpreter::BlockConsistencyInterpreter(int worker_rank,
		int num_workers, const SipTables& sipTables,  BarrierBlockConsistencyMap& barrier_block_consistency_map) :
		SialxInterpreter(sipTables, NULL, NULL, NULL), worker_rank_(
				worker_rank), num_workers_(num_workers), sip_tables_(sipTables),
				barrier_block_consistency_map_(barrier_block_consistency_map),
				last_seen_barrier_pc_(0){
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
				this, worker_rank_, num_workers_, iteration_);
	loop_start(pc, loop);
}

void BlockConsistencyInterpreter::handle_sip_barrier_op(int pc) { last_seen_barrier_pc_++; }

void BlockConsistencyInterpreter::handle_program_end(int& pc){}

void BlockConsistencyInterpreter::handle_get_op(int pc) {
	PardoSectionConsistencyInfo& pardo_sections_info = barrier_block_consistency_map_[last_seen_barrier_pc_];
	ArrayIdDeletedMap& array_id_deleted_map = pardo_sections_info.array_id_deleted_map;
	ArrayBlockConsistencyMap& blocks_consistency_map_ =  pardo_sections_info.blocks_consistency_map_;
	BlockId id = get_block_id_from_selector_stack();
	int array_id = id.array_id();
	if (array_id_deleted_map[array_id]){
		std::stringstream err_ss;
		err_ss << "From worker " << worker_rank_
				<< " ,trying to GET block "<< id
				<< " of array " << sip_tables_.array_name(array_id)
				<< " which has been deleted at line " << line_number();
		fail_with_exception(err_ss.str(), line_number());
	}
	BlockIdConsistencyMap& array_consistency_map = blocks_consistency_map_[array_id];
	DistributedBlockConsistency& distributed_block_consistency_map = array_consistency_map[id];
	bool consistent = distributed_block_consistency_map.update_and_check_consistency(SIPMPIConstants::GET, worker_rank_);
	if (!consistent){
		std::stringstream err_ss;
		err_ss << "Incorrect GET Block Semantics for " << id << " from worker " << worker_rank_ << " at line " << line_number();
		fail_with_exception(err_ss.str(), line_number());
	}
}

void BlockConsistencyInterpreter::put_sum_into_block_semantics(const BlockId &lhs_id) {
	PardoSectionConsistencyInfo& pardo_sections_info = barrier_block_consistency_map_[last_seen_barrier_pc_];
	ArrayIdDeletedMap& array_id_deleted_map = pardo_sections_info.array_id_deleted_map;
	ArrayBlockConsistencyMap& blocks_consistency_map_ = pardo_sections_info.blocks_consistency_map_;
	int array_id = lhs_id.array_id();
	if (array_id_deleted_map[array_id]) {
		std::stringstream err_ss;
		err_ss << "From worker " << worker_rank_
				<< " ,trying to PUT_ACCUMULATE block " << lhs_id << " of array "
				<< sip_tables_.array_name(array_id)
				<< " which has been deleted at line " << line_number();
		fail_with_exception(err_ss.str(), line_number());
	}
	BlockIdConsistencyMap& array_consistency_map = blocks_consistency_map_[array_id];
	DistributedBlockConsistency& distributed_block_consistency_map = array_consistency_map[lhs_id];
	bool consistent = distributed_block_consistency_map.update_and_check_consistency(SIPMPIConstants::PUT_ACCUMULATE, worker_rank_);
	if (!consistent) {
		std::stringstream err_ss;
		err_ss << "Incorrect PUT_ACCUMULATE Block Semantics for " << lhs_id
				<< " from worker " << worker_rank_ << " at line "
				<< line_number();
		fail_with_exception(err_ss.str(), line_number());
	}
}

void BlockConsistencyInterpreter::handle_put_accumulate_op(int pc) {
	BlockId rhs_id = get_block_id_from_selector_stack();
	BlockId lhs_id = get_block_id_from_selector_stack();
	put_sum_into_block_semantics(lhs_id);
}

void BlockConsistencyInterpreter::handle_put_increment_op(int pc){
	BlockId lhs_id = get_block_id_from_selector_stack();
	put_sum_into_block_semantics(lhs_id);
}
void BlockConsistencyInterpreter::handle_put_scale_op(int pc){
	BlockId lhs_id = get_block_id_from_selector_stack();
	put_sum_into_block_semantics(lhs_id);
}

void BlockConsistencyInterpreter::put_replace_block_semantics(const BlockId &lhs_id) {
	PardoSectionConsistencyInfo& pardo_sections_info = barrier_block_consistency_map_[last_seen_barrier_pc_];
	ArrayIdDeletedMap& array_id_deleted_map = pardo_sections_info.array_id_deleted_map;
	ArrayBlockConsistencyMap& blocks_consistency_map_ = pardo_sections_info.blocks_consistency_map_;
	int array_id = lhs_id.array_id();
	if (array_id_deleted_map[array_id]) {
		std::stringstream err_ss;
		err_ss << "From worker " << worker_rank_ << " ,trying to PUT block "
				<< lhs_id << " of array " << sip_tables_.array_name(array_id)
				<< " which has been deleted at line " << line_number();
		fail_with_exception(err_ss.str(), line_number());
	}
	BlockIdConsistencyMap& array_consistency_map = blocks_consistency_map_[array_id];
	DistributedBlockConsistency& distributed_block_consistency_map = array_consistency_map[lhs_id];
	bool consistent = distributed_block_consistency_map.update_and_check_consistency(SIPMPIConstants::PUT, worker_rank_);
	if (!consistent) {
		std::stringstream err_ss;
		err_ss << "Incorrect PUT Block Semantics for " << lhs_id
				<< " from worker " << worker_rank_ << " at line "
				<< line_number();
		fail_with_exception(err_ss.str(), line_number());
	}
}

void BlockConsistencyInterpreter::handle_put_replace_op(int pc) {
	BlockId rhs_id = get_block_id_from_selector_stack();
	BlockId lhs_id = get_block_id_from_selector_stack();
	put_replace_block_semantics(lhs_id);
}

void BlockConsistencyInterpreter::handle_put_initialize_op(int pc) {
	BlockId lhs_id = get_block_id_from_selector_stack();
	put_replace_block_semantics(lhs_id);
}




void BlockConsistencyInterpreter::handle_delete_op(int pc){
	int array_id = arg0(pc);
	PardoSectionConsistencyInfo& pardo_sections_info = barrier_block_consistency_map_[last_seen_barrier_pc_];
	ArrayIdDeletedMap& array_id_deleted_map = pardo_sections_info.array_id_deleted_map;
	array_id_deleted_map[array_id] = true;
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
		if (opcode_name == "get_my_rank"){
			sip::BlockId block_id0;
			sip::Block::BlockPtr block0 = get_block_from_selector_stack('w', block_id0, true);
			double* data = block0->get_data();
			data[0] = worker_rank_;
		} else {
			SialxInterpreter::handle_execute_op(pc);
		}

	}
}



} /* namespace sip */
