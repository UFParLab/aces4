/*
 * profile_interpreter.cpp
 *
 *  Created on: Oct 1, 2014
 *      Author: njindal
 */

#include "profile_interpreter.h"
#include "profile_timer.h"
#include "sip_tables.h"
#include "sial_printer.h"
#include "worker_persistent_array_manager.h"

namespace sip {

ProfileInterpreter::ProfileInterpreter(const SipTables& sipTables,
		ProfileTimer& profile_timer, SialxTimer& sialx_timer, SialPrinter* printer,
		WorkerPersistentArrayManager* persistent_array_manager):
		SialxInterpreter(sipTables, &sialx_timer, printer, persistent_array_manager),
		profile_timer_(profile_timer), last_seen_pc_(-1),
		sialx_timer_(sialx_timer){
}

ProfileInterpreter::~ProfileInterpreter() {}

inline ProfileTimer::Key ProfileInterpreter::make_profile_timer_key(const std::string& opcode_name, const std::list<BlockSelector>& selector_list){
	std::vector<ProfileTimer::BlockInfo> block_infos;
	block_infos.reserve(selector_list.size());

	std::list<BlockSelector>::const_iterator it = selector_list.begin();
	for (; it != selector_list.end(); ++it){
		const BlockSelector &bsel = *it;
		BlockShape bs;
		int rank = bsel.rank_;
		if (sip_tables_.array_rank(bsel.array_id_) == 0) { //this "array" was declared to be a scalar.  Nothing to remove from selector stack.
			continue;
		}
		else if (sip_tables_.is_contiguous(bsel.array_id_) && bsel.rank_ == 0) { //this is a static array provided without a selector, block is entire array
			Block::BlockPtr block = data_manager_.contiguous_array_manager_.get_array(bsel.array_id_);
			bs = block->shape();
			// Modify rank so that the correct segments & indices are stored
			// in the profile timer.
			rank = sip_tables_.array_rank(bsel.array_id_);
		} else {
			BlockId bid = get_block_id_from_selector(bsel);
			bs = sip_tables_.shape(bid);
		}
		ProfileTimer::BlockInfo bi (rank, bsel.index_ids_, bs.segment_sizes_);
		block_infos.push_back(bi);
	}
	ProfileTimer::Key key(opcode_name, block_infos);
	return key;

}


inline ProfileTimer::Key ProfileInterpreter::make_profile_timer_key(opcode_t opcode, const std::list<BlockSelector>& selector_list){
	return make_profile_timer_key(opcodeToName(opcode), selector_list);
}

void ProfileInterpreter::pre_interpret(int pc){
	SialxInterpreter::pre_interpret(pc);
	opcode_t opcode = op_table_.opcode(pc);
	profile_timer_trace(pc, opcode);
}

void ProfileInterpreter::post_interpret(int oldpc, int newpc){
	SialxInterpreter::post_interpret(oldpc, newpc);
	profile_timer_trace(newpc, invalid_op);
}

void ProfileInterpreter::profile_timer_trace(int pc, opcode_t opcode){

	if (pc == last_seen_pc_)
		return;
	else if (last_seen_pc_ >= 0){
		double total_time = sialx_timer_[last_seen_pc_].get_last_recorded_total_time_and_clear();
		double communication_time = sialx_timer_[last_seen_pc_].get_last_recorded_communication_time_and_clear();
		double computation_time = total_time - communication_time;
		profile_timer_.record_time(last_seen_key_, computation_time, last_seen_pc_);
		last_seen_pc_ = -1;
	} else if (opcode != invalid_op){

		switch (opcode){

		/////////////////////////////////////////////// 0 blocks
		case do_op:
		case enddo_op:
		case where_op:
		case pardo_op:
		case endpardo_op:				// WARNING : Variable number of blocks are garbage collected

		case allocate_op:
		case deallocate_op:
		case allocate_contiguous_op:
		case deallocate_contiguous_op:
		case create_op:
		case delete_op:
		case print_string_op:
		case print_scalar_op:
		case print_int_op:
		case print_index_op:
		case print_block_op:
		case println_op:
		case set_persistent_op:
		case restore_persistent_op:
		case goto_op:
		case jump_if_zero_op:
		case stop_op:
		case exit_op:
		case call_op:
		case push_block_selector_op:
		case string_load_literal_op:
		case int_load_literal_op:
		case int_load_value_op:
		case int_store_op:
		case int_add_op:
		case int_subtract_op:
		case int_multiply_op:
		case int_divide_op:
		case int_equal_op:
		case int_nequal_op:
		case int_ge_op:
		case int_le_op:
		case int_gt_op:
		case int_lt_op:
		case int_neg_op:
		case cast_to_int_op:
		case index_load_value_op:
		case scalar_load_value_op:
		case scalar_store_op:
		case scalar_add_op:
		case scalar_subtract_op:
		case scalar_multiply_op:
		case scalar_divide_op:
		case scalar_exp_op:
		case scalar_eq_op:
		case scalar_ne_op:
		case scalar_ge_op:
		case scalar_le_op:
		case scalar_gt_op:
		case scalar_lt_op:
		case scalar_neg_op:
		case scalar_sqrt_op:
		case idup_op:
		case iswap_op:
		case sswap_op:
		{
			std::list<BlockSelector> bs_list;
			ProfileTimer::Key key = make_profile_timer_key(opcode, bs_list);
			last_seen_key_ = key;
			last_seen_pc_ = pc;
		}
			break;

		/////////////////////////////////////////////// 1 blocks
		// 1 block - in instruction
		case block_fill_op:
		case block_scale_op:
		case block_accumulate_scalar_op:
		{
			std::list<BlockSelector> bs_list;
			bs_list.push_back(BlockSelector(arg0(pc), arg1(pc), index_selectors(pc)));
			ProfileTimer::Key key = make_profile_timer_key(opcode, bs_list);

			last_seen_key_ = key;
			last_seen_pc_ = pc;
		}
		break;

		// 1 block - in selector stack
		case block_load_scalar_op:
		case get_op:
		{
			std::list<BlockSelector> bs_list;
			bs_list.push_back(BlockSelector(block_selector_stack_.top()));
			ProfileTimer::Key key = make_profile_timer_key(opcode, bs_list);
			last_seen_key_ = key;
			last_seen_pc_ = pc;
		}
		break;

		/////////////////////////////////////////////// 2 blocks
		// 1 block - in selector stack
		// 1 block - in instruction
		case block_copy_op:
		case block_scale_assign_op:
		{
			std::list<BlockSelector> bs_list;
			bs_list.push_back(block_selector_stack_.top());
			bs_list.push_back(BlockSelector(arg0(pc), arg1(pc), index_selectors(pc)));
			ProfileTimer::Key key = make_profile_timer_key(opcode, bs_list);

			last_seen_key_ = key;
			last_seen_pc_ = pc;

		}
		break;

		// 2 blocks - in selector stack
		case block_permute_op:
		case block_contract_to_scalar_op:
		case put_accumulate_op:
		case put_replace_op:
		{
			std::list<BlockSelector> bs_list;
			BlockSelector lhs_bs = block_selector_stack_.top();
			block_selector_stack_.pop();
			BlockSelector rhs_bs = block_selector_stack_.top();
			bs_list.push_back(lhs_bs);
			bs_list.push_back(rhs_bs);
			ProfileTimer::Key key = make_profile_timer_key(opcode, bs_list);
			block_selector_stack_.push(lhs_bs);

			last_seen_key_ = key;
			last_seen_pc_ = pc;
		}
		break;

		/////////////////////////////////////////////// 3 blocks
		// 2 blocks - in selector stack
		// 1 block  - in instruction
		case block_add_op:
		case block_subtract_op:
		case block_contract_op:
		{
			std::list<BlockSelector> bs_list;
			BlockSelector lhs_bs = block_selector_stack_.top();
			block_selector_stack_.pop();
			BlockSelector rhs_bs = block_selector_stack_.top();
			bs_list.push_back(BlockSelector(arg0(pc), arg1(pc), index_selectors(pc)));
			bs_list.push_back(lhs_bs);
			bs_list.push_back(rhs_bs);
			ProfileTimer::Key key = make_profile_timer_key(opcode, bs_list);
			block_selector_stack_.push(lhs_bs);
			last_seen_key_ = key;
			last_seen_pc_ = pc;
		}
		break;

		/////////////////////////////////////////////// Variable number of blocks
		// 0 - 6 blocks - in selector stack
		case execute_op:
		{
			int num_args = arg1(pc);
			int func_slot = arg0(pc);
			std::string user_sub_name = sip_tables_.special_instruction_manager().name(func_slot);
			std::list<BlockSelector> bs_list;
			for(int i=0; i<num_args; ++i){
				bs_list.push_back(block_selector_stack_.top());
				block_selector_stack_.pop();
			}
			ProfileTimer::Key key = make_profile_timer_key(user_sub_name, bs_list);
			last_seen_key_ = key;
			last_seen_pc_ = pc;
			for (int i=0; i<num_args; ++i){
				BlockSelector& bs = bs_list.back();
				block_selector_stack_.push(bs);
				bs_list.pop_back();
			}

		}
		break;
	}
	}

}

} /* namespace sip */
