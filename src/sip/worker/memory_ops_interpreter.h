/*
 * memory_ops_interpreter.h
 *
 *  Created on: Oct 23, 2014
 *      Author: njindal
 */

#ifndef MEMORY_OPS_INTERPRETER_H_
#define MEMORY_OPS_INTERPRETER_H_

#include "config.h"
#include "sialx_interpreter.h"

namespace sip {

/**
 * Interprets a sialx program while only doing
 * block allocation & deallocation operations.
 * To be used to scale up systems, debugging & testing.
 */
class MemoryOpsInterpreter: public SialxInterpreter {
public:
	MemoryOpsInterpreter(const SipTables& sipTables, SialxTimer* sialx_timer);
	virtual ~MemoryOpsInterpreter();

    virtual void pre_interpret(int pc);

	virtual void handle_execute_op(int pc);
	virtual void handle_get_op(int pc);
	virtual void handle_broadcast_static(int pc) {}
	virtual void handle_put_accumulate_op(int pc);
	virtual void handle_put_replace_op(int pc);
	virtual void handle_create_op(int pc) {}
	virtual void handle_delete_op(int pc);
	virtual void handle_collective_sum_op(int pc) {  expression_stack_.pop();}
	virtual void handle_assert_same_op(int pc) {}
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

//	virtual void handle_print_string_op(int pc) { control_stack_.pop();}
//	virtual void handle_print_scalar_op(int pc) {  expression_stack_.pop();}
//	virtual void handle_print_int_op(int pc) { control_stack_.pop();}
//	virtual void handle_print_index_op(int pc) { control_stack_.pop();}
//	virtual void handle_print_block_op(int pc) { block_selector_stack_.pop(); }
//	virtual void handle_println_op(int pc) {}
	virtual void handle_gpu_on_op(int pc) {}
	virtual void handle_gpu_off_op(int pc) {}
	virtual void handle_set_persistent_op(int pc) {}
	virtual void handle_restore_persistent_op(int pc){}





};

} /* namespace sip */

#endif /* MEMORY_OPS_INTERPRETER_H_ */
