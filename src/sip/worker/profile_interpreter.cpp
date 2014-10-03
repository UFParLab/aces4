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
		ProfileTimer& profile_timer, SialPrinter* printer,
		WorkerPersistentArrayManager* persistent_array_manager):
		SialxInterpreter(sipTables, NULL, printer, persistent_array_manager),
		profile_timer_(profile_timer), last_seen_pc_(-1){
}

ProfileInterpreter::~ProfileInterpreter() {}


inline ProfileTimer::Key ProfileInterpreter::make_profile_timer_key(opcode_t opcode, std::list<BlockSelector> selector_list){
	std::vector<ProfileTimer::BlockInfo> block_infos;
	block_infos.reserve(selector_list.size());

	std::list<BlockSelector>::iterator it = selector_list.begin();
	for (; it != selector_list.end(); ++it){
		BlockSelector &bsel = *it;
		BlockId bid = get_block_id_from_selector(bsel);
		BlockShape bs = sip_tables_.shape(bid);
		ProfileTimer::BlockInfo bi (bsel.rank_, bsel.index_ids_, bs.segment_sizes_);
		block_infos.push_back(bi);
	}
	ProfileTimer::Key key(opcode, block_infos);
	return key;

}

void ProfileInterpreter::pre_interpret(int pc){
	// Profile timers

	opcode_t opcode = op_table_.opcode(pc);

	if (pc == last_seen_pc_)
		return;
	else if (last_seen_pc_ >= 0){
		profile_timer_.pause_timer(last_seen_key_);
		last_seen_pc_ = -1;
	} else {

		switch (opcode){


		// 1 block - in selector stack
		// 1 block - in instruction
		case block_copy_op:
		case block_scale_assign_op:
		{
			std::list<BlockSelector> bs_list;
			bs_list.push_back(block_selector_stack_.top());
			bs_list.push_back(BlockSelector(arg0(pc), arg1(pc), index_selectors(pc)));
			ProfileTimer::Key key = make_profile_timer_key(opcode, bs_list);

			profile_timer_.start_timer(key);
			last_seen_key_ = key;
			last_seen_pc_ = pc;

		}
		break;

		// 2 blocks - in selector stack
		case block_permute_op:
		case block_contract_to_scalar_op:
		{
			std::list<BlockSelector> bs_list;
			BlockSelector lhs_bs = block_selector_stack_.top();
			block_selector_stack_.pop();
			BlockSelector rhs_bs = block_selector_stack_.top();
			bs_list.push_back(lhs_bs);
			bs_list.push_back(rhs_bs);
			bs_list.push_back(block_selector_stack_.top());
			ProfileTimer::Key key = make_profile_timer_key(opcode, bs_list);
			block_selector_stack_.push(lhs_bs);

			profile_timer_.start_timer(key);
			last_seen_key_ = key;
			last_seen_pc_ = pc;
		}
		break;

		// 1 block - in selector stack
		case block_fill_op:
		case block_scale_op:
		case block_accumulate_scalar_op:
		{
			std::list<BlockSelector> bs_list;
			bs_list.push_back(BlockSelector(arg0(pc), arg1(pc), index_selectors(pc)));
			ProfileTimer::Key key = make_profile_timer_key(opcode, bs_list);

			profile_timer_.start_timer(key);
			last_seen_key_ = key;
			last_seen_pc_ = pc;
		}
		break;

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
			bs_list.push_back(block_selector_stack_.top());
			ProfileTimer::Key key = make_profile_timer_key(opcode, bs_list);
			block_selector_stack_.push(lhs_bs);

			profile_timer_.start_timer(key);
			last_seen_key_ = key;
			last_seen_pc_ = pc;
		}
		break;


		// 1 block - in selector stack
		case block_load_scalar_op:
		{
			std::list<BlockSelector> bs_list;
			bs_list.push_back(BlockSelector(block_selector_stack_.top()));
			ProfileTimer::Key key = make_profile_timer_key(opcode, bs_list);

			profile_timer_.start_timer(key);
			last_seen_key_ = key;
			last_seen_pc_ = pc;
		}
		break;
	}
	}

}

} /* namespace sip */
