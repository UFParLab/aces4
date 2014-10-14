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

	// Counting time for these opcodes
	// is relevant
	virtual void handle_block_copy_op(int pc);
	virtual void handle_block_permute_op(int pc);
	virtual void handle_block_fill_op(int pc);
	virtual void handle_block_scale_op(int pc) ;
	virtual void handle_block_scale_assign_op(int pc) ;
	virtual void handle_block_accumulate_scalar_op(int pc);
	virtual void handle_block_add_op(int pc);
	virtual void handle_block_subtract_op(int pc);
	virtual void handle_block_contract_op(int pc);
	virtual void handle_block_contract_to_scalar_op(int pc);
	virtual void handle_block_load_scalar_op(int pc);
	virtual void handle_execute_op(int pc);


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
