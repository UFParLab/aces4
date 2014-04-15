/*
 * OpTable.h
 *
 *  Created on: Jul 15, 2013
 *      Author: basbas
 */

#ifndef OPTABLE_H_
#define OPTABLE_H_

#include <vector>
#include "sip.h"
#include "aces_defs.h"
#include "io_utils.h"


namespace sip {

//codes for where clauses
enum where_code_t {
	where_eq = 0,
	where_geq = 1,
	where_leq = 2,
	where_gt = 3,
	where_lt = 4,
	where_neq = 5
};

/**opcodes and optable entry contents
 *
 * An optable entry contains the following fields
 * opcode, op1_array, op2_array, result_array, selector[MAX_RANK], line_number
 *
 * Below, the contents of op1_array, op2_array, result_array, selector[MAX_RANK] for each opcode is indicated
 * Unused slots are indicate with _
 *
 * X-Macros are used to define the opcodes as an enum and a enum-to-string function.
 *
 */

#define SIP_OPCODES \
SIPOP(contraction_op, 101, "contraction", true)	/*  slot of left arg array or scalar, slot of right arg array or scalar, slot of result array or scalar, [_] */\
												/*  expects selectors for all arrays (rank != 0) on selector stack,  pushed from left to right. */\
SIPOP(sum_op, 102, "sum", true)  				/*  slot of left arg array or scalar, slot of right arg array or scalar, slot of result array or scalar, [_] */\
												/*  expects selectors for all arrays (rank != 0) on selector stack,  pushed from left to right. */\
SIPOP(push_block_selector_op, 103, "push_block_selector", false) /* rank,_,array id,[index selectors] */\
SIPOP(do_op, 104, "do", true)  					/*  _,_,optable slot of end_do_op,[slot of index variable,_..] */\
SIPOP(enddo_op, 105, "enddo", true) 				/*  _,_,_,[slot of index variable,_..] */\
SIPOP(get_op, 106, "get", true)  				/*  _,_,array_id,[selector indices]   (should be preceded by a push_selector_op) */\
SIPOP(user_sub_op, 107, "user_sub", true)  		/*  function slot,_,number of arguments,[_] (should be preceded by a push_selector_op for each argument in reverse order) */\
SIPOP(put_op, 108, "put", true)  				/*  rhs array slot,_, lhs array slot,[_] (should be prededed by push_selector_op for lhs and rhs blocks) */\
SIPOP(go_to_op, 109, "go_to", false)  			/*  _,_,optable slot of destination, [_] */\
SIPOP(create_op, 110, "create", true)  			/*  _._,array_id,[_] */\
SIPOP(delete_op, 111, "delete", true)  			/*  _,_,array_id,[_] */\
SIPOP(call_op, 112, "procedure", true) 			/* _,_,pc of subroutine,[_] */\
SIPOP(return_op, 113, "return", true)  			/* _,_,_,[_] */\
SIPOP(jz_op, 114, "jz", false) 					/*  _,_,optable slot of destination, [_]  (pops value of relational expr from control stack, jumps if zero) */\
SIPOP(stop_op, 115, "stop", false)  			/* _,_,_,[_] */\
SIPOP(sp_add_op, 116, "sp_add", false)  		/*  opcode not used */\
SIPOP(sp_sub_op, 117, "sp_sub", false) 			/*  opcode not used */\
SIPOP(sp_mult_op, 118, "sp_mult", false)  		/*  opcode not used */\
SIPOP(sp_div_op, 119, "sp_div", false)  		/*  opcocde not used */\
SIPOP(sp_equal_op, 120, "sp_equal", false)  	/*  _,_,_,[_] */\
												/*  pops arguments from control stack, pushes val SIPOP(of, , "of", true)SIPOP(, , "", true)on control stack */\
SIPOP(sp_nequal_op, 121, "sp_nequal", false)  	/*  _,_,_,[_] */\
												/*  pops arguments from control stack, pushes val of !SIPOP(, , "", true)on control stack */\
SIPOP(sp_ge_op, 122, "sp_ge", false)  			/*  _,_,_,[_] */\
												/*  pops arguments from control stack, rightmost first, pushes val of >SIPOP(, , "", true)on control stack */\
SIPOP(sp_le_op, 123, "sp_le", false)  			/*  _,_,_,[_] */\
												/*  pops arguments from control stack, rightmost first, pushes val of <SIPOP(, , "", true)on control stack */\
SIPOP(sp_gt_op, 124, "sp_gt", false)   			/*  _,_,_,[_] */\
												/*  pops arguments from control stack, rightmost first, pushes val of >  on control stack */\
SIPOP(sp_lt_op, 125, "sp_lt", false)   			/*  _,_,_,[_] */\
												/*  pops arguments from control stack, rightmost first, pushes val of <  on control stack */\
SIPOP(sp_ldi_op, 126, "sp_ldi", false)   		/*  value,_,_,[_] */\
												/* pushes value (of int literal) on control stack */\
SIPOP(sp_ldindex_op, 127, "sp_ldindex", false)  /*  index_value,_,_,[_] */\
												/*  pushes value of index slot on control stack */\
SIPOP(pardo_op, 128, "pardo", true)   			/*  number of indices,_,optable slot of end_pardo_op,[slots for indices used in loop] */\
SIPOP(endpardo_op, 129, "endpardo", true)   		/* _,_,_,[_] */\
												/*  obtains data from control_stack and loop_manager_stack. */\
												/*  see loop_start and loop_end methods in Interpreter */\
SIPOP(exit_op, 130, "exit", false)   				/*  _,_,_,[_] */\
SIPOP(assignment_op, 131, "assignment", true)  	/*  rhs array_table slot,_, lhs array_table slot, [_] */\
												/*  expects block selectors on block_selector stack for non-scalars */\
												/*  this instruction is complex and can handle transpose, dimension-reduction, etc. */\
												/*  see the Interpreter::handle_assignment_op method for details */\
SIPOP(cycle_op, 132, "cycle", false)   			/*  not implemented */\
SIPOP(self_multiply_op, 134, "self_multiply", true) /*  *SIPOP(, , "", true)scalar */\
SIPOP(subtract_op, 135, "subtract", true)   		/*  slot of left arg array or scalar, slot of right arg array or scalar, slot of result array or scalar, [_] */\
												/*  expects selectors for all arrays (rank !SIPOP(, 0, "", true)) on selector stack,  pushed from left to right. */\
SIPOP(collective_sum_op, 136, "collective_sum", true) /*  source_array_slot,_,target_array_slot,[_] */\
SIPOP(divide_op, 137, "divide", true)  			/*  dividend array_table slot, divisor array_table slot, result array_table slot, [_] */\
												/*  only defined for scalars */\
SIPOP(prepare_op, 138, "prepare", true)  		/*  _,_,_,[] */\
												/*  expects block selectors for source and target to be on selector stack */\
SIPOP(request_op, 139, "request", true)  		/*  _,_,_,[] */\
												/*  expects block selector on block_selector_stack */\
SIPOP(compute_integrals_op, 140, "compute_integrals", true)  /*  not used */\
SIPOP(put_replace_op, 141, "put_replace", true) 	/*   _,_,_,[] */\
												/*  expects block selectors for source and target to be on selector stack */\
SIPOP(tensor_op, 142, "tensor", true)  			/*  slot of left arg array or scalar, slot of right arg array or scalar, slot of result array or scalar, [_] */\
												/*  expects selectors for all arrays (rank !SIPOP(, 0, "", true)) on selector stack,  pushed from left to right. */\
SIPOP(fl_add_op, 146, "fl_add", false)  		/*  opcode not used */\
SIPOP(fl_sub_op, 147, "fl_sub", false)  		/*  opcode not used */\
SIPOP(fl_mult_op, 148, "fl_mult", false)  		/*  opcode not used */\
SIPOP(fl_div_op, 149, "fl_div", false)  		/*  opcode  used */\
SIPOP(fl_eq_op, 150, "fl_eq", false)  			/*   _,_,_,[] */\
												/*  obtains arguments from expression stack; pushes result on control stack */\
SIPOP(fl_ne_op, 151, "fl_ne", false)  			/*   _,_,_,[] */\
												/*  obtains arguments from expression stack; pushes result on control stack */\
SIPOP(fl_ge_op, 152, "fl_ge", false)  			/*   _,_,_,[] */\
												/*  obtains arguments from expression stack; pushes result on control stack */\
SIPOP(fl_le_op, 153, "fl_le", false)  			/*   _,_,_,[] */\
												/*  obtains arguments from expression stack; pushes result on control stack */\
SIPOP(fl_gt_op, 154, "fl_gt", false)  			/*   _,_,_,[] */\
												/*  obtains arguments from expression stack; pushes result on control stack */\
SIPOP(fl_lt_op, 155, "fl_lt", false)  			/*   _,_,_,[] */\
												/*  obtains arguments from expression stack; pushes result on control stack */\
SIPOP(fl_load_value_op, 157, "fl_load_value", false)  /*  array_table slot of scalar,_,_,[_] */\
												 /*  push scalar's value onto expression_stack */\
SIPOP(prepare_increment_op, 158, "prepare_increment", true) /*  _,_,_,[] */\
															/*  expects block selectors for source and target to be on selector stack */\
SIPOP(allocate_op, 159, "allocate", true)  		/*  _,_,array_table slot,[index selectors] */\
SIPOP(deallocate_op, 160, "deallocate", true)  	/*  _,_,array_table slot,[index selectors] */\
SIPOP(sp_ldi_sym_op, 161, "sp_ldi_sym", false)  	/*  int_table_slot,_,_,[_] */\
												/*  pushes indicated int value onto control stack */\
SIPOP(destroy_op, 162, "destroy", true)  			/*  _,_,array_table slot,[_] */\
												/*  destroys indicated served array */\
SIPOP(prequest_op, 163, "prequest", true)  		/*  not implemented */\
SIPOP(where_op, 164, "where", false)  			/*  index table slot of arg1, where_op, index_table slot of arg2, [_] */\
												/*  if true, increment pc */\
												/*  other wise go to op_table slot on top of control stack */\
												/*  where_op takes a value from the where_code_t enum. */\
SIPOP(print_op, 165, "print", true)  			/*  _,_,index in string table,[_] */\
												/*  prints the indicated string */\
SIPOP(println_op, 166, "println", true)  		/*  _,_,index in string table,[_] */\
												/*  prints the indicated string followed by \n */\
SIPOP(print_index_op, 167, "print_index", true)	/*  _,_,index_table slot of index to print */\
SIPOP(print_scalar_op, 168, "print_scalar", true) 	/*  _,_,array_table slot of scalar to print */\
SIPOP(assign_scalar_op, 169, "assign_scalar", true) 	/*  not used */\
SIPOP(assign_block_op, 170, "assign_block", true)  	/*  not used */\
SIPOP(assign_fill_op, 180, "assign_fill", true)  		/*  not used */\
SIPOP(assign_transpose_op, 181, "assign_transpose", true) /*  not used */\
SIPOP(dosubindex_op, 182, "dosubindex", true)  		/*  _,_,_,[index_slot,_..] */\
SIPOP(enddosubindex_op, 183, "enddosubindex", true)	/* _,_,_,[_] */\
												/*  obtains data from control_stack and loop_manager_stack. */\
												/*  see loop_start and loop_end methods in Interpreter */\
SIPOP(slice_op, 184, "slice", false) 			/*  slot in array_table for rhs,_,slot in array table for lhs,[_] */\
												/*  expects selector for rhs block on block_selector_stack */\
												/*  expects selector for lhs block on block_selector_stack */\
SIPOP(insert_op, 185, "insert", false) 			/*  _,_,slot in array table for lhs,[_] */\
												/*  expects selector for rhs on block_selector_stack */\
												/*  expects selector for lhs block on block_selector_stack */\
SIPOP(sip_barrier_op, 186, "sip_barrier", true) /* _,_,_,[_] */\
SIPOP(server_barrier_op, 187, "server_barrier", true) /* _,_,_,[_] */\
SIPOP(gpu_on_op, 188, "gpu_on", false)  		/* _,_,_,[_] */\
SIPOP(gpu_off_op, 189, "gpu_off", false)  		/* _,_,_,[_] */\
SIPOP(gpu_allocate_op, 190, "gpu_allocate", true)/* _,_,_,[_] */\
											   	/*  expects selector on block_selector_stack */\
SIPOP(gpu_free_op, 191, "gpu_free", true)  		/* _,_,_,[_] */\
												/*  expects selector on block_selector_stack */\
SIPOP(gpu_put_op, 192, "gpu_put", true)   		/* _,_,_,[_] */\
												/*  expects selector on block_selector_stack */\
SIPOP(gpu_get_op, 193, "gpu_get", true)   		/* _,_,_,[_] */\
												/*  expects selector on block_selector_stack */\
SIPOP(set_persistent_op, 194, "set_persistent", true)  			/*  slot in string_table for name, _ , array_table slot, [_] */\
SIPOP(restore_persistent_op, 195, "restore_persistent", true) 	/*  slot in string_table for name, _ , array_table slot, [_] */\

enum opcode_t {
#define SIPOP(e,n,t,p) e = n,
	SIP_OPCODES
#undef SIPOP
	last_opcode
};

/**
 * Converts an opcode to it's string equivalent
 * @param
 * @return
 */
std::string opcodeToName(opcode_t);

/**
 * Converts an integer to an opcode
 * @param
 * @return
 */
opcode_t intToOpcode(int);

/**
 * Whether a certain opcode is printable
 * @param
 * @return
 */
bool printableOpcode(opcode_t);

class OpTableEntry {
public:
	OpTableEntry();
	~OpTableEntry();

	static void read(OpTableEntry&, setup::InputStream&);
	friend class OpTable;
	friend std::ostream& operator<<(std::ostream&, const OpTableEntry &);
private:
	opcode_t opcode;
	int op1_array;  //also user_sub, and rank in push_block_selector_op
	int op2_array;
	int result_array;
	int selector[MAX_RANK];
//	int user_sub;
	int line_number;
//	DISALLOW_COPY_AND_ASSIGN(OpTableEntry);
};

class OpTable{
public:
	OpTable();
	~OpTable();

	static void read(OpTable&, setup::InputStream&);
	friend std::ostream& operator<<(std::ostream&, const OpTable&);
	friend class SipTables;
	friend class Interpreter;


	//* inlined accesor functions for optable contents
	opcode_t opcode(int pc){
		return entries_.at(pc).opcode;
	}

	int op1_array(int pc){
		return entries_.at(pc).op1_array;
	}

	int op2_array(int pc){
		return entries_.at(pc).op2_array;
	}

	int result_array(int pc){
		return entries_.at(pc).result_array;
	}

	int print_index(int pc){
		return entries_.at(pc).result_array;
	}

	int user_sub(int pc){
		return entries_.at(pc).op1_array;  //shares slot
	}

	int line_number(int pc){
		return entries_.at(pc).line_number;
	}
	int go_to_target(int pc){
		return entries_.at(pc).result_array;
	}

	int size(){
		return entries_.size();
	}

	int num_indices(int pc){
		return entries_.at(pc).op1_array;
	}

    sip::index_selector_t& index_selectors(int pc){
    	return entries_.at(pc).selector;
    }

	int do_index_selector(int pc){
		return entries_.at(pc).selector[0];
	}

private:
	std::vector<OpTableEntry> entries_;
	DISALLOW_COPY_AND_ASSIGN(OpTable);
};

} /* namespace worker */
#endif /* OPTABLE_H_ */


