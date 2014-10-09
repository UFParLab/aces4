/*
 * sipmap_interpreter.h
 *
 *  Created on: Oct 9, 2014
 *      Author: njindal
 */

#ifndef SIPMAP_INTERPRETER_H_
#define SIPMAP_INTERPRETER_H_

#include "sialx_interpreter.h"

namespace sip {
class SIPMaPTimer;
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


	// Functions that don't modify the program counter.
	// These functions have been disabled (overriden with empty implementations
	// so that this interpreter does not do any actual work.
	// It should only construct ProfileTimer::Key s for all operations.

	// Functions that modify the program counter
	// are passed in a reference to it.
//	virtual void handle_goto_op(int &pc);
//	virtual void handle_jump_if_zero_op(int &pc);
//	virtual void handle_stop_op(int &pc);
//	virtual void handle_return_op(int &pc);
//	virtual void handle_do_op(int &pc);
//	virtual void handle_loop_end(int &pc);
//	virtual void handle_exit_op(int &pc);
//	virtual void handle_where_op(int &pc);
//	virtual void handle_pardo_op(int &pc);
//	virtual void handle_endpardo_op(int &pc);
//	virtual void handle_call_op(int &pc);

	// Functions that don't modify the program counter.
	virtual void handle_execute_op(int pc) {
		int num_args = arg1(pc) ;
		for (int i=0; i<num_args; i++)
			block_selector_stack_.pop();
	}
	virtual void handle_sip_barrier_op(int pc) {}
	virtual void handle_broadcast_static_op(int pc) {}
//	virtual void handle_push_block_selector_op(int pc);
	virtual void handle_allocate_op(int pc) {}
	virtual void handle_deallocate_op(int pc) {}
	virtual void handle_allocate_contiguous_op(int pc) {}
	virtual void handle_deallocate_contiguous_op(int pc) {}
	virtual void handle_get_op(int pc) {}
	virtual void handle_put_accumulate_op(int pc) { block_selector_stack_.pop(); }
	virtual void handle_put_replace_op(int pc) {block_selector_stack_.pop(); }
	virtual void handle_create_op(int pc) {}
	virtual void handle_delete_op(int pc) {}

//	virtual void handle_string_load_literal_op(int pc);
//	virtual void handle_int_load_value_op(int pc);
//	virtual void handle_int_load_literal_op(int pc);
//	virtual void handle_int_store_op(int pc);
//	virtual void handle_index_load_value_op(int pc);
//	virtual void handle_int_add_op(int pc);
//	virtual void handle_int_subtract_op(int pc);
//	virtual void handle_int_multiply_op(int pc);
//	virtual void handle_int_divide_op(int pc);
//	virtual void handle_int_equal_op(int pc);
//	virtual void handle_int_nequal_op(int pc);
//	virtual void handle_int_ge_op(int pc);
//	virtual void handle_int_le_op(int pc);
//	virtual void handle_int_gt_op(int pc);
//	virtual void handle_int_lt_op(int pc);
//	virtual void handle_int_neg_op(int pc);
//	virtual void handle_cast_to_int_op(int pc);
//	virtual void handle_scalar_load_value_op(int pc);
//	virtual void handle_scalar_store_op(int pc);
//	virtual void handle_scalar_add_op(int pc);
//	virtual void handle_scalar_subtract_op(int pc);
//	virtual void handle_scalar_multiply_op(int pc);
//	virtual void handle_scalar_divide_op(int pc);
//	virtual void handle_scalar_exp_op(int pc);
//	virtual void handle_scalar_eq_op(int pc);
//	virtual void handle_scalar_ne_op(int pc);
//	virtual void handle_scalar_ge_op(int pc);
//	virtual void handle_scalar_le_op(int pc);
//	virtual void handle_scalar_gt_op(int pc);
//	virtual void handle_scalar_lt_op(int pc);
//	virtual void handle_scalar_neg_op(int pc);
//	virtual void handle_scalar_sqrt_op(int pc);
//	virtual void handle_cast_to_scalar_op(int pc);

	virtual void handle_collective_sum_op(int pc) {  expression_stack_.pop();}
	virtual void handle_assert_same_op(int pc) {}
	virtual void handle_block_copy_op(int pc) {
		block_selector_stack_.pop();
		block_selector_stack_.pop();
	}
	virtual void handle_block_permute_op(int pc) {
		block_selector_stack_.pop();
		block_selector_stack_.pop();
	}
	virtual void handle_block_fill_op(int pc) {  expression_stack_.pop();}
	virtual void handle_block_scale_op(int pc) {  expression_stack_.pop();}
	virtual void handle_block_scale_assign_op(int pc) {
		block_selector_stack_.pop();
		expression_stack_.pop();
	}
	virtual void handle_block_accumulate_scalar_op(int pc) {  expression_stack_.pop();}
	virtual void handle_block_add_op(int pc) {
		block_selector_stack_.pop();
		block_selector_stack_.pop();
	}
	virtual void handle_block_subtract_op(int pc) {
		block_selector_stack_.pop();
		block_selector_stack_.pop();
	}
	virtual void handle_block_contract_op(int pc) {
		block_selector_stack_.pop();
		block_selector_stack_.pop();
	}
	virtual void handle_block_contract_to_scalar_op(int pc) {
		block_selector_stack_.pop();
		block_selector_stack_.pop();
		expression_stack_.push(0.0);
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
//	virtual void handle_idup_op(int pc);
//	virtual void handle_iswap_op(int pc);
//	virtual void handle_sswap_op(int pc);



private:
	const ProfileTimerStore& profile_timer_store_;
	SIPMaPTimer& sipmap_timer_;

};

} /* namespace sip */

#endif /* SIPMAP_INTERPRETER_H_ */
