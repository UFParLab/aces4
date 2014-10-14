/*
 * sipmap_interpreter.h
 *
 *  Created on: Oct 9, 2014
 *      Author: njindal
 */

#ifndef SIPMAP_INTERPRETER_H_
#define SIPMAP_INTERPRETER_H_

#include "sialx_interpreter.h"
#include "profile_timer.h"
#include "profile_timer_store.h"
#include "sipmap_timer.h"

#include <utility>

namespace sip {
class ProfileTimerStore;
class SipTables;

/**
 * SIP Modeling and Prediction (SIPMaP) interpreter.
 * Models the Sialx Program based on information
 * made available from the profile interpreter run.
 * Profile information is made available through an
 * instance of ProfileTimerStore which encapsulates
 * a sqlite3 database.
 */
class SIPMaPInterpreter: public SialxInterpreter {
public:
	SIPMaPInterpreter(const SipTables& sipTables, const ProfileTimerStore& profile_timer_store, SIPMaPTimer& sipmap_timer);
	virtual ~SIPMaPInterpreter();



//	virtual void handle_jump_if_zero_op(int &pc);	// TODO - To visit if & else parts of the code.


	virtual void handle_execute_op(int pc) {
		int num_args = arg1(pc) ;
		int func_slot = arg0(pc);
		std::string opcode_name = sip_tables_.special_instruction_manager().name(func_slot);
		std::list<BlockSelector> bs_list;
		for (int i=0; i<num_args; i++){
			bs_list.push_back(block_selector_stack_.top());
			block_selector_stack_.pop();
		}
		ProfileTimer::Key key = make_profile_timer_key(opcode_name, bs_list);
		std::pair<long, long> timer_count_pair = profile_timer_store_.get_from_store(key);
		double time = timer_count_pair.first / (double)timer_count_pair.second;
		sipmap_timer_.record_time(line_number(), time);
	}
	virtual void handle_sip_barrier_op(int pc) {}
	virtual void handle_broadcast_static_op(int pc) {}
	virtual void handle_allocate_op(int pc) {}
	virtual void handle_deallocate_op(int pc) {}
	virtual void handle_allocate_contiguous_op(int pc) {}
	virtual void handle_deallocate_contiguous_op(int pc) {}
	virtual void handle_get_op(int pc) {}
	virtual void handle_put_accumulate_op(int pc) { block_selector_stack_.pop(); }
	virtual void handle_put_replace_op(int pc) {block_selector_stack_.pop(); }
	virtual void handle_create_op(int pc) {}
	virtual void handle_delete_op(int pc) {}

	virtual void handle_collective_sum_op(int pc) {  expression_stack_.pop();}
	virtual void handle_assert_same_op(int pc) {}
	virtual void handle_block_copy_op(int pc) {
		std::list<BlockSelector> bs_list;
		bs_list.push_back(block_selector_stack_.top());
		block_selector_stack_.pop();
		bs_list.push_back(BlockSelector(arg0(pc), arg1(pc), index_selectors(pc)));
		ProfileTimer::Key key = make_profile_timer_key(block_copy_op, bs_list);
		std::pair<long, long> timer_count_pair = profile_timer_store_.get_from_store(key);
		double time = timer_count_pair.first / (double)timer_count_pair.second;
		sipmap_timer_.record_time(line_number(), time);
	}
	virtual void handle_block_permute_op(int pc) {
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
	virtual void handle_block_fill_op(int pc) {
		std::list<BlockSelector> bs_list;
		expression_stack_.pop();
		bs_list.push_back(BlockSelector(arg0(pc), arg1(pc), index_selectors(pc)));
		ProfileTimer::Key key = make_profile_timer_key(block_fill_op, bs_list);
		std::pair<long, long> timer_count_pair = profile_timer_store_.get_from_store(key);
		double time = timer_count_pair.first / (double)timer_count_pair.second;
		sipmap_timer_.record_time(line_number(), time);
	}
	virtual void handle_block_scale_op(int pc) {
		std::list<BlockSelector> bs_list;
		expression_stack_.pop();
		bs_list.push_back(BlockSelector(arg0(pc), arg1(pc), index_selectors(pc)));
		ProfileTimer::Key key = make_profile_timer_key(block_scale_op, bs_list);
		std::pair<long, long> timer_count_pair = profile_timer_store_.get_from_store(key);
		double time = timer_count_pair.first / (double)timer_count_pair.second;
		sipmap_timer_.record_time(line_number(), time);
	}
	virtual void handle_block_scale_assign_op(int pc) {
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
	virtual void handle_block_accumulate_scalar_op(int pc) {
		std::list<BlockSelector> bs_list;
		expression_stack_.pop();
		bs_list.push_back(BlockSelector(arg0(pc), arg1(pc), index_selectors(pc)));
		ProfileTimer::Key key = make_profile_timer_key(block_accumulate_scalar_op, bs_list);
		std::pair<long, long> timer_count_pair = profile_timer_store_.get_from_store(key);
		double time = timer_count_pair.first / (double)timer_count_pair.second;
		sipmap_timer_.record_time(line_number(), time);
	}
	virtual void handle_block_add_op(int pc) {
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
	virtual void handle_block_subtract_op(int pc) {
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
	virtual void handle_block_contract_op(int pc) {
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
	virtual void handle_block_contract_to_scalar_op(int pc) {
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
	virtual void handle_block_load_scalar_op(int pc){
		block_selector_stack_.pop();
		expression_stack_.push(0.0);
	}
	virtual void handle_print_string_op(int pc) { control_stack_.pop();}
	virtual void handle_print_scalar_op(int pc) {  expression_stack_.pop();}
	virtual void handle_print_int_op(int pc) { control_stack_.pop();}
	virtual void handle_print_index_op(int pc) { control_stack_.pop();}
	virtual void handle_print_block_op(int pc) { block_selector_stack_.pop(); }
	virtual void handle_println_op(int pc) {}
	virtual void handle_gpu_on_op(int pc) {}
	virtual void handle_gpu_off_op(int pc) {}
	virtual void handle_set_persistent_op(int pc) {}
	virtual void handle_restore_persistent_op(int pc){}


private:
	const ProfileTimerStore& profile_timer_store_;
	SIPMaPTimer& sipmap_timer_;

	/**
	 * Helper method to construct a profile timer key
	 * to get data from the profile timer store
	 * @param opcode_name
	 * @param selector_list
	 * @return
	 */
	ProfileTimer::Key make_profile_timer_key(const std::string& opcode_name,
			const std::list<BlockSelector>& selector_list);

	/**
	 * Converts an opcode to a string and
	 * delegates to SIPMaPInterpreter#make_profile_timer_key(const std::string&, const std::list<BlockSelector>&)
	 * @param opcode
	 * @param selector_list
	 * @return
	 */
	ProfileTimer::Key make_profile_timer_key(opcode_t opcode,
			const std::list<BlockSelector>& selector_list);


};

} /* namespace sip */

#endif /* SIPMAP_INTERPRETER_H_ */
