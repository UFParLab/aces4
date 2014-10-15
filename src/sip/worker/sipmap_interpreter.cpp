/*
 * sipmap_interpreter.cpp
 *
 *  Created on: Oct 9, 2014
 *      Author: njindal
 */

#include "sipmap_interpreter.h"
#include "profile_timer_store.h"
#include "sipmap_timer.h"

namespace sip {

SIPMaPInterpreter::SIPMaPInterpreter(const SipTables& sipTables, const ProfileTimerStore& profile_timer_store, SIPMaPTimer& sipmap_timer):
	SialxInterpreter(sipTables, NULL, NULL, NULL), profile_timer_store_(profile_timer_store), sipmap_timer_(sipmap_timer){
}

SIPMaPInterpreter::~SIPMaPInterpreter() {}


inline ProfileTimer::Key SIPMaPInterpreter::make_profile_timer_key(const std::string& opcode_name, const std::list<BlockSelector>& selector_list){
	std::vector<ProfileTimer::BlockInfo> block_infos;
	block_infos.reserve(selector_list.size());

	std::list<BlockSelector>::const_iterator it = selector_list.begin();
	for (; it != selector_list.end(); ++it){
		const BlockSelector &bsel = *it;
		BlockShape bs;
		int rank = bsel.rank_;
		if (sip_tables_.array_rank(bsel.array_id_) == 0) { //this "array" was declared to be a scalar.  Nothing to remove from selector stack.)
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

inline ProfileTimer::Key SIPMaPInterpreter::make_profile_timer_key(opcode_t opcode, const std::list<BlockSelector>& selector_list){
	return make_profile_timer_key(opcodeToName(opcode), selector_list);
}




void SIPMaPInterpreter::handle_execute_op(int pc) {
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
	ProfileTimer::Key key = make_profile_timer_key(opcode_name, bs_list);
	std::pair<long, long> timer_count_pair = profile_timer_store_.get_from_store(key);
	double time = timer_count_pair.first / (double)timer_count_pair.second;
	sipmap_timer_.record_time(line_number(), time);

	if (all_write && all_scalar_or_static){
		std::cout << "Executing Super Instruction "
				<< sip_tables_.special_instruction_manager().name(func_slot)
				<< " at line " << line_number() << std::endl;
		// Push back blocks on stack and execute the super instruction.
		while (!bs_list.empty()){
			BlockSelector bs = bs_list.back();
			block_selector_stack_.push(bs);
			bs_list.pop_back();
		}
		SialxInterpreter::handle_execute_op(pc);
	}

}

void SIPMaPInterpreter::handle_block_copy_op(int pc) {
	std::list<BlockSelector> bs_list;
	bs_list.push_back(block_selector_stack_.top());
	block_selector_stack_.pop();
	bs_list.push_back(BlockSelector(arg0(pc), arg1(pc), index_selectors(pc)));
	ProfileTimer::Key key = make_profile_timer_key(block_copy_op, bs_list);
	std::pair<long, long> timer_count_pair = profile_timer_store_.get_from_store(key);
	double time = timer_count_pair.first / (double)timer_count_pair.second;
	sipmap_timer_.record_time(line_number(), time);
}
void SIPMaPInterpreter::handle_block_permute_op(int pc) {
	std::list<BlockSelector> bs_list;
	bs_list.push_back(block_selector_stack_.top());
	block_selector_stack_.pop();
	bs_list.push_back(block_selector_stack_.top());
	block_selector_stack_.pop();
	ProfileTimer::Key key = make_profile_timer_key(block_permute_op, bs_list);
	std::pair<long, long> timer_count_pair = profile_timer_store_.get_from_store(key);
	double time = timer_count_pair.first / (double)timer_count_pair.second;
	sipmap_timer_.record_time(line_number(), time);
}
void SIPMaPInterpreter::handle_block_fill_op(int pc) {
	std::list<BlockSelector> bs_list;
	expression_stack_.pop();
	bs_list.push_back(BlockSelector(arg0(pc), arg1(pc), index_selectors(pc)));
	ProfileTimer::Key key = make_profile_timer_key(block_fill_op, bs_list);
	std::pair<long, long> timer_count_pair = profile_timer_store_.get_from_store(key);
	double time = timer_count_pair.first / (double)timer_count_pair.second;
	sipmap_timer_.record_time(line_number(), time);
}
void SIPMaPInterpreter::handle_block_scale_op(int pc) {
	std::list<BlockSelector> bs_list;
	expression_stack_.pop();
	bs_list.push_back(BlockSelector(arg0(pc), arg1(pc), index_selectors(pc)));
	ProfileTimer::Key key = make_profile_timer_key(block_scale_op, bs_list);
	std::pair<long, long> timer_count_pair = profile_timer_store_.get_from_store(key);
	double time = timer_count_pair.first / (double)timer_count_pair.second;
	sipmap_timer_.record_time(line_number(), time);
}
void SIPMaPInterpreter::handle_block_scale_assign_op(int pc) {
	std::list<BlockSelector> bs_list;
	bs_list.push_back(block_selector_stack_.top());
	block_selector_stack_.pop();
	bs_list.push_back(BlockSelector(arg0(pc), arg1(pc), index_selectors(pc)));
	expression_stack_.pop();
	ProfileTimer::Key key = make_profile_timer_key(block_scale_assign_op, bs_list);
	std::pair<long, long> timer_count_pair = profile_timer_store_.get_from_store(key);
	double time = timer_count_pair.first / (double)timer_count_pair.second;
	sipmap_timer_.record_time(line_number(), time);
}
void SIPMaPInterpreter::handle_block_accumulate_scalar_op(int pc) {
	std::list<BlockSelector> bs_list;
	expression_stack_.pop();
	bs_list.push_back(BlockSelector(arg0(pc), arg1(pc), index_selectors(pc)));
	ProfileTimer::Key key = make_profile_timer_key(block_accumulate_scalar_op, bs_list);
	std::pair<long, long> timer_count_pair = profile_timer_store_.get_from_store(key);
	double time = timer_count_pair.first / (double)timer_count_pair.second;
	sipmap_timer_.record_time(line_number(), time);
}
void SIPMaPInterpreter::handle_block_add_op(int pc) {
	std::list<BlockSelector> bs_list;
	bs_list.push_back(BlockSelector(arg0(pc), arg1(pc), index_selectors(pc)));
	bs_list.push_back(block_selector_stack_.top());
	block_selector_stack_.pop();
	bs_list.push_back(block_selector_stack_.top());
	block_selector_stack_.pop();
	ProfileTimer::Key key = make_profile_timer_key(block_add_op, bs_list);
	std::pair<long, long> timer_count_pair = profile_timer_store_.get_from_store(key);
	double time = timer_count_pair.first / (double)timer_count_pair.second;
	sipmap_timer_.record_time(line_number(), time);
}
void SIPMaPInterpreter::handle_block_subtract_op(int pc) {
	std::list<BlockSelector> bs_list;
	bs_list.push_back(BlockSelector(arg0(pc), arg1(pc), index_selectors(pc)));
	bs_list.push_back(block_selector_stack_.top());
	block_selector_stack_.pop();
	bs_list.push_back(block_selector_stack_.top());
	block_selector_stack_.pop();
	ProfileTimer::Key key = make_profile_timer_key(block_subtract_op, bs_list);
	std::pair<long, long> timer_count_pair = profile_timer_store_.get_from_store(key);
	double time = timer_count_pair.first / (double)timer_count_pair.second;
	sipmap_timer_.record_time(line_number(), time);
}
void SIPMaPInterpreter::handle_block_contract_op(int pc) {
	std::list<BlockSelector> bs_list;
	bs_list.push_back(BlockSelector(arg0(pc), arg1(pc), index_selectors(pc)));
	bs_list.push_back(block_selector_stack_.top());
	block_selector_stack_.pop();
	bs_list.push_back(block_selector_stack_.top());
	block_selector_stack_.pop();
	ProfileTimer::Key key = make_profile_timer_key(block_contract_op, bs_list);
	std::pair<long, long> timer_count_pair = profile_timer_store_.get_from_store(key);
	double time = timer_count_pair.first / (double)timer_count_pair.second;
	sipmap_timer_.record_time(line_number(), time);
}
void SIPMaPInterpreter::handle_block_contract_to_scalar_op(int pc) {
	std::list<BlockSelector> bs_list;
	bs_list.push_back(block_selector_stack_.top());
	block_selector_stack_.pop();
	bs_list.push_back(block_selector_stack_.top());
	block_selector_stack_.pop();
	expression_stack_.push(0.0);
	ProfileTimer::Key key = make_profile_timer_key(block_contract_to_scalar_op, bs_list);
	std::pair<long, long> timer_count_pair = profile_timer_store_.get_from_store(key);
	double time = timer_count_pair.first / (double)timer_count_pair.second;
	sipmap_timer_.record_time(line_number(), time);
}

void SIPMaPInterpreter::handle_block_load_scalar_op(int pc){
	BlockSelector &bs = block_selector_stack_.top();
	if (sip_tables_.is_contiguous(bs.array_id_)){
		SialxInterpreter::handle_block_load_scalar_op(pc);
	} else {
		block_selector_stack_.pop();
		expression_stack_.push(0.0);
	}
}


} /* namespace sip */
