/*
 * Interpreter.cpp
 *
 *  Created on: Aug 5, 2013
 *      Author: Beverly Sanders
 */

#include "interpreter.h"
#include <vector>
#include <assert.h>
//#include <sstream>
#include <string>
#include <algorithm>
#include <iomanip>
#include "loop_manager.h"
#include "special_instructions.h"
#include "block.h"
#include "tensor_ops_c_prototypes.h"

#include "config.h"

// For CUDA Super Instructions
#ifdef HAVE_CUDA
#include "gpu_super_instructions.h"
#endif

namespace sip {

Interpreter* Interpreter::global_interpreter;

Interpreter::Interpreter(SipTables& sipTables, SialxTimer& sialx_timer,
		PersistentArrayManager<Block, Interpreter>* persistent_array_manager) :
		sip_tables_(sipTables), sialx_timers_(sialx_timer), data_manager_(), op_table_(
				sipTables.op_table_), persistent_array_manager_(
				persistent_array_manager), sial_ops_(data_manager_,
				persistent_array_manager) {
	_init(sipTables);
}

Interpreter::~Interpreter() {
}

void Interpreter::_init(SipTables& sipTables) {
	int num_indices = sipTables.index_table_.entries_.size();
	//op_table_ = sipTables.op_table_;
	pc = 0;
	global_interpreter = this;
	gpu_enabled_ = false;
#ifdef HAVE_CUDA
	int devid;
	int rank = 0;
	_init_gpu(&devid, &rank);
#endif
}

void Interpreter::interpret() {
	int nops = op_table_.size();
	interpret(0, nops);
}

void Interpreter::interpret(int pc_start, int pc_end) {
	pc = pc_start;
	while (pc < pc_end) {
		opcode_t opcode = op_table_.opcode(pc);
		sip::check(write_back_list_.empty(),
				"SIP bug:  write_back_list not empty at top of interpreter loop");
		switch (opcode) {
		case contraction_op: {
			sialx_timers_.start_timer(line_number());
			sial_ops_.log_statement(opcode, line_number());
			handle_contraction_op(pc);
			write_back_contiguous();
			sialx_timers_.pause_timer(line_number());
			++pc;
		}
			break;
		case sum_op: {
			sialx_timers_.start_timer(line_number());
			sial_ops_.log_statement(opcode, line_number());
			handle_sum_op(pc, 1.0);
			write_back_contiguous();
			sialx_timers_.pause_timer(line_number());
			++pc;
		}
			break;
		case push_block_selector_op: { //push a block selector on the block_selector_stack to be used in subsequent instructions
			int array_id = op_table_.result_array(pc);
			int rank = op_table_.op1_array(pc); //this is the rank.  Should rename the optable entry fields.  These are inherited from aces3.
			block_selector_stack_.push(
					sip::BlockSelector(array_id, rank,
							op_table_.index_selectors(pc)));
			++pc;
		}
			break;
		case do_op: {
			int index_slot = op_table_.do_index_selector(pc);
			LoopManager* loop = new DoLoop(index_slot, data_manager_,
					sip_tables_);
			loop_start(loop);
		}
			break;
		case enddo_op: {
			loop_end();
		}
			break;
		case get_op: {
			sial_ops_.log_statement(opcode, line_number());
			sip::BlockId id = get_block_id_from_selector_stack();
			sial_ops_.get(id);
			++pc;
		}
			break;
		case user_sub_op: {
			sial_ops_.log_statement(opcode, line_number());
			sialx_timers_.start_timer(line_number());
			handle_user_sub_op(pc);
			write_back_contiguous();
			sialx_timers_.pause_timer(line_number());
			++pc;
		}
			break;
		case put_op: { //this is instruction put a(...) += b(...)
			sial_ops_.log_statement(opcode, line_number());
			sialx_timers_.start_timer(line_number());
			sip::Block::BlockPtr rhs_block = get_block_from_selector_stack('r',
					true);
			sip::BlockId lhs_id = get_block_id_from_selector_stack();
			sial_ops_.put_accumulate(lhs_id, rhs_block);
			sialx_timers_.pause_timer(line_number());
			++pc;
		}
			break;
		case go_to_op: {
			pc = op_table_.go_to_target(pc);
		}
			break;
		case create_op: {
			sial_ops_.log_statement(opcode, line_number());
			sialx_timers_.start_timer(line_number());
			int array_id = sip_tables_.op_table_.result_array(pc);
			sial_ops_.create_distributed(array_id);
			sialx_timers_.pause_timer(line_number());
			++pc;
		}
			break;
		case delete_op: {
			sial_ops_.log_statement(opcode, line_number());
			sialx_timers_.start_timer(line_number());
			int array_id = sip_tables_.op_table_.result_array(pc);
			sial_ops_.delete_distributed(array_id);
			sialx_timers_.pause_timer(line_number());
			++pc;
		}
			break;
		case call_op: {  //compiler ensures every subroutine has return_op
			sial_ops_.log_statement(opcode, line_number());
			int target = op_table_.result_array(pc);
			control_stack_.push(pc + 1);
			pc = target;
		}
			break;
		case return_op: {
			pc = control_stack_.top();
			control_stack_.pop();
		}
			break;
		case jz_op: {
			int result = control_stack_.top();
			control_stack_.pop();
			if (result == 0)
				pc = op_table_.result_array(pc);
			else
				++pc;
		}
			break;
		case stop_op: {
			sip::check(false, "stop command aborted program.");
		}
			break;
//        case sp_add_op : break;  NOT USED in either original or new acesiii compiler, so we don't use here either until int type added
//        case sp_sub_op: break;   NOT USED in either original or new acesiii compiler
//        case sp_mult_op: break;  NOT USED in either original or new acesiii compiler
//        case sp_div_op: break;   NOT USED in either original or new acesiii compiler
		case sp_equal_op:
			//fallthrough all int rel expr evaluated the same way
		case sp_nequal_op:
			//fallthrough
		case sp_ge_op:
			//fallthrough
		case sp_le_op:
			//fallthrough
		case sp_gt_op:
			//fallthrough
		case sp_lt_op: {
			bool result;
			result = evaluate_int_relational_expr(pc);
			control_stack_.push(result);
			++pc;
		}
			break;
		case sp_ldi_op: { //push int literal value, which is stored in the instruction itself.
			int value = op_table_.op1_array(pc);
			control_stack_.push(value);
			++pc;
		}
			break;
		case sp_ldindex_op: { //push index value onto control stack.  Only uses opcode and op1_array entries.  "legacy fields" are ignored.
			int index_slot = op_table_.op1_array(pc);
			int value = index_value(index_slot);
			control_stack_.push(value);
			++pc;
		}
			break;
		case pardo_op: {
			sial_ops_.log_statement(opcode, line_number());
			int num_indices = op_table_.num_indices(pc);
#ifdef HAVE_MPI
			LoopManager* loop = new StaticTaskAllocParallelPardoLoop(
					num_indices, op_table_.index_selectors(pc), data_manager_,
					sip_tables_, SIPMPIAttr::get_instance());
#else
			LoopManager* loop = new SequentialPardoLoop(num_indices,
					op_table_.index_selectors(pc), data_manager_, sip_tables_);
#endif
			loop_start(loop);
		}
			break;
		case endpardo_op: {
			sial_ops_.log_statement(opcode, line_number());
			loop_end();
		}
			break;
		case exit_op: {
			sial_ops_.log_statement(opcode, line_number());
			loop_manager_stack_.top()->set_to_exit();
			pc = control_stack_.top();
		}
			break;
		case assignment_op:
			sial_ops_.log_statement(opcode, line_number());
			sialx_timers_.start_timer(line_number());
			// x = y
			handle_assignment_op(pc);
			write_back_contiguous();
			sialx_timers_.pause_timer(line_number());
			++pc;
			break;
		case cycle_op: {
			sip::check(false, "cycle command not implemented");
		}
			break;
		case self_multiply_op: { // *= scalar  The compiler should generate better code for this
			sial_ops_.log_statement(opcode, line_number());
			sialx_timers_.start_timer(line_number());
			handle_self_multiply_op(pc);
			write_back_contiguous();
			sialx_timers_.pause_timer(line_number());
			++pc;
		}
			break;
		case subtract_op: {
			sial_ops_.log_statement(opcode, line_number());
			sialx_timers_.start_timer(line_number());
			handle_sum_op(pc, -1.0); // (x = y - z) is computed as x = y + (z * -1)
			write_back_contiguous();
			sialx_timers_.pause_timer(line_number());
			++pc;
		}
			break;
		case collective_sum_op: {
			sial_ops_.log_statement(opcode, line_number());
			sialx_timers_.start_timer(line_number());
			int target_array_slot = op_table_.result_array(pc);
			int source_array_slot = op_table_.op1_array(pc);
			sial_ops_.collective_sum(source_array_slot, target_array_slot);
			sialx_timers_.pause_timer(line_number());
			++pc;
		}
			break;
		case divide_op: {
			sial_ops_.log_statement(opcode, line_number());
			int darray = op_table_.result_array(pc);
			int drank = sip_tables_.array_rank(darray);
			int larray = op_table_.op1_array(pc);
			int lrank = sip_tables_.array_rank(larray);
			int rarray = op_table_.op2_array(pc);
			int rrank = sip_tables_.array_rank(rarray);
			check(lrank == 0 && rrank == 0 && drank == 0,
					"divide only defined for scalars");
			double denom = scalar_value(rarray);
			double numer = scalar_value(larray);
			sip::check(denom != 0, "Dividing by 0!", line_number());
			double div = numer / denom;
			set_scalar_value(darray, div);
			++pc;
		}
			break;

		case prepare_op: {
			sial_ops_.log_statement(opcode, line_number());
			sialx_timers_.start_timer(line_number());
			sip::Block::BlockPtr rhs_block = get_block_from_selector_stack('r');
			sip::BlockId lhs_id = get_block_id_from_selector_stack();
			sial_ops_.prepare(lhs_id, rhs_block);
			sialx_timers_.pause_timer(line_number());
			++pc;
		}
			break;
		case request_op: {
			sial_ops_.log_statement(opcode, line_number());
			sialx_timers_.start_timer(line_number());
			sip::BlockId id = get_block_id_from_selector_stack();
			sial_ops_.request(id);
			sialx_timers_.pause_timer(line_number());
			++pc;
		}
			break;
		case put_replace_op: {  // put a(...) = b(...)
			sial_ops_.log_statement(opcode, line_number());
			sialx_timers_.start_timer(line_number());
			sip::Block::BlockPtr rhs_block = get_block_from_selector_stack('r');
			sip::BlockId lhs_id = get_block_id_from_selector_stack();
			sial_ops_.put_replace(lhs_id, rhs_block);
			sialx_timers_.pause_timer(line_number());
			++pc;
		}
			break;
		case tensor_op: {
			sial_ops_.log_statement(opcode, line_number());
			sialx_timers_.start_timer(line_number());
			handle_contraction_op(pc);
			write_back_contiguous();
			sialx_timers_.pause_timer(line_number());
			++pc;
		}
			break;
//        case fl_add_op: break;  NOT USED in either original or new acesiii compiler, so we don't use here either
//        case fl_sub_op: break;  NOT USED in either original or new acesiii compiler, so we don't use here either
//        case fl_mult_op: break; NOT USED in either original or new acesiii compiler, so we don't use here either
//        case fl_div_op: break;  NOT USED in either original or new acesiii compiler, so we don't use here either
		case fl_eq_op: //fallthrough.  All relational exprs handled the same way.
		case fl_ne_op: //fallthrough
		case fl_ge_op: //fallthrough
		case fl_le_op: //fallthrough
		case fl_gt_op: //fallthrough
		case fl_lt_op: { //evaluate the expression (with operands from the expr stack) and push the result
						 //onto the control stack;
			bool result;
			result = evaluate_double_relational_expr(pc);
			control_stack_.push(result);
			++pc;
		}
			break;
		case fl_load_value_op: { //look up value in scalar table and push onto the stack
			int array_table_slot = op_table_.op1_array(pc);
			double value = scalar_value(array_table_slot);
			expression_stack_.push(value);
			++pc;
		}
			break;
		case prepare_increment_op: {
			sial_ops_.log_statement(opcode, line_number());
			sialx_timers_.start_timer(line_number());
			sip::Block::BlockPtr rhs_block = get_block_from_selector_stack('r',
					true);
			sip::BlockId lhs_id = get_block_id_from_selector_stack();
			sial_ops_.prepare_accumulate(lhs_id, rhs_block);
			sialx_timers_.pause_timer(line_number());
			++pc;
		}
			break;
		case allocate_op: {
			sial_ops_.log_statement(opcode, line_number());
			sialx_timers_.start_timer(line_number());
			int array_table_slot = op_table_.result_array(pc);
			int rank = sip_tables_.array_rank(array_table_slot);
			const sip::index_selector_t& index_array =
					op_table_.index_selectors(pc);
			sip::BlockId id = block_id(
					sip::BlockSelector(array_table_slot, rank, index_array));
			data_manager_.block_manager_.allocate_local(id);
			sialx_timers_.pause_timer(line_number());
			++pc;
		}
			break;
		case deallocate_op: {
			sial_ops_.log_statement(opcode, line_number());
			sialx_timers_.start_timer(line_number());
			int array_table_slot = op_table_.result_array(pc);
			int rank = sip_tables_.array_rank(array_table_slot);
			const sip::index_selector_t& index_array =
					op_table_.index_selectors(pc);
			sip::BlockId id = block_id(
					sip::BlockSelector(array_table_slot, rank, index_array));
			data_manager_.block_manager_.deallocate_local(id);
			sialx_timers_.pause_timer(line_number());
			++pc;
		}
			break;
		case sp_ldi_sym_op: { //loads int (aces symbolic constant) onto the control stack
			int int_table_slot = op_table_.op1_array(pc);
			int value = int_value(int_table_slot);
			control_stack_.push(value);
			++pc;
		}
			break;
		case destroy_op: {
			sial_ops_.log_statement(opcode, line_number());
			sialx_timers_.start_timer(line_number());
			int array_id = sip_tables_.op_table_.result_array(pc);
			sial_ops_.destroy_served(array_id);
			sialx_timers_.pause_timer(line_number());
			++pc;
		}
			break;
		case prequest_op: {
			sip::check(false, "prequest statement not implemented");
		}
			break;
		case where_op: {
			if (evaluate_where_clause(pc)) {
				++pc;
			} else {
				pc = control_stack_.top(); //note that this must be in a loop, the enddo (or endX) instruction will pop the value.
			}
		}
			break;
		case print_op: {
			int string_slot = op_table_.print_index(pc);
			std::cout << string_literal(string_slot) << std::flush;
			++pc;
		}
			break;
		case println_op: {
			int string_slot = op_table_.print_index(pc);
			std::cout << string_literal(string_slot) << std::endl << std::flush;
			++pc;
		}
			break;
		case print_index_op: {
			int index_slot = op_table_.print_index(pc);
			std::cout << index_value_to_string(index_slot) << std::endl
					<< std::flush;
			++pc;
		}
			break;
		case print_scalar_op: {
			int array_table_slot = op_table_.print_index(pc);
			double value = scalar_value(array_table_slot);
			std::string name = sip_tables_.array_name(array_table_slot);
			const std::streamsize old = std::cout.precision();
			std::cout << name << " = " << std::setprecision(20) << value
					<< " at line " << op_table_.line_number(pc) << std::endl
					<< std::flush;
			std::cout.precision(old);
			++pc;
		}
			break;
		case dosubindex_op: {
			sial_ops_.log_statement(opcode, line_number());
			int index_slot = op_table_.do_index_selector(pc);
			LoopManager* loop = new SubindexDoLoop(index_slot, data_manager_,
					sip_tables_);
			loop_start(loop);
		}
			break;
		case enddosubindex_op: {
			sial_ops_.log_statement(opcode, line_number());
			loop_end();
		}
			break;
		case slice_op: {
			handle_slice_op(pc);
			write_back_contiguous();
			++pc;
		}
			break;
		case insert_op: {
			handle_insert_op(pc);
			++pc;
		}
			break;
		case sip_barrier_op: //fallthrough. server and sip barriers the same
		case server_barrier_op: {
			sial_ops_.log_statement(opcode, line_number());
			sialx_timers_.start_timer(line_number());
			sial_ops_.sip_barrier();
			sialx_timers_.pause_timer(line_number());
			++pc;
		}
			break;
#ifdef HAVE_CUDA
			case gpu_on_op: {
				sial_ops_.log_statement(opcode, line_number());
				gpu_enabled_= true;
				++pc;
			}
			break;
			case gpu_off_op: {
				sial_ops_.log_statement(opcode, line_number());
				gpu_enabled_ = false;
				++pc;
			}
			break;
			case gpu_allocate_op: {
				sial_ops_.log_statement(opcode, line_number());
				if (gpu_enabled_) {
					sialx_timers_.start_timer(line_number());
					get_gpu_block('w');
					sialx_timers_.pause_timer(line_number());

				}
				++pc;
			}
			break;
			case gpu_free_op: {
				sial_ops_.log_statement(opcode, line_number());
				if (gpu_enabled_) {
					sialx_timers_.start_timer(line_number());
					sip::Block::BlockPtr blk = get_gpu_block('w');
					blk->free_gpu_data();
					sialx_timers_.pause_timer(line_number());
				}
				++pc;
			}
			break;
			case gpu_put_op: {
				sial_ops_.log_statement(opcode, line_number());
				if (gpu_enabled_) {
					sialx_timers_.start_timer(line_number());
					get_gpu_block('w'); // TOOD FIXME Temporary solution
					sialx_timers_.pause_timer(line_number());
					++pc;
				}
				break;
				case gpu_get_op: {
					sial_ops_.log_statement(opcode, line_number());
					if (gpu_enabled_) {
						sialx_timers_.start_timer(line_number());
						get_block_from_selector_stack('r'); //TODO FIXME Temporary solution
						sialx_timers_.pause_timer(line_number());
					}
					++pc;
				}
				break;
			}
#else
		case gpu_on_op:
		case gpu_off_op:
		case gpu_allocate_op:
		case gpu_free_op:
		case gpu_put_op:
		case gpu_get_op:
			sial_ops_.log_statement(opcode, line_number());
			sip::check_and_warn(false,
					"No CUDA Support, ignoring GPU instruction at line ",
					line_number());
			++pc;
			break;

#endif //HAVE_CUDA
		case set_persistent_op: {
			sial_ops_.log_statement(opcode, line_number());
			int array_slot = op_table_.result_array(pc);
			int string_slot = op_table_.op1_array(pc);
			SIP_LOG(
					std::cout << "set_persistent with array " << array_name(array_slot) << " in slot " << array_slot << " and string \"" << string_literal(string_slot) << "\"" << std::endl;)
			sial_ops_.set_persistent(this, array_slot, string_slot);
			++pc;
		}
			break;
		case restore_persistent_op: {
			sial_ops_.log_statement(opcode, line_number());
			int array_slot = op_table_.result_array(pc);
			int string_slot = op_table_.op1_array(pc);
			SIP_LOG(
					std::cout << "restore_persistent with array " << array_name(array_slot) << " in slot " << array_slot << " and string \"" << string_literal(string_slot) << "\"" << std::endl;)
			sial_ops_.restore_persistent(this, array_slot, string_slot);
			++pc;
		}
			break;
		default: {
			std::cout << "opcode =" << opcode;
			check(false, " unimplemented opcode");
		}

		}
	}

	post_sial_program();

}

void Interpreter::post_sial_program() {
	sial_ops_.end_program();
}

void Interpreter::handle_user_sub_op(int pc) {
	int num_args = op_table_.result_array(pc);
	int func_slot = op_table_.user_sub(pc);
	int ierr = 0;
	if (num_args == 0) {
		SpecialInstructionManager::fp0 func =
				sip_tables_.special_instruction_manager_.get_no_arg_special_instruction_ptr(
						func_slot);
		func(ierr);
		check(ierr == 0,
				"error returned from special super instruction"
						+ sip_tables_.special_instruction_manager_.name(
								func_slot));
		return;
	}
	// num_args >= 1.  Set up first argument
	const std::string signature(
			sip_tables_.special_instruction_manager_.get_signature(func_slot));
	sip::BlockId block_id0;
	char intent0 = signature[0];
	sip::BlockSelector arg_selector0 = block_selector_stack_.top();
	int array_id0 = arg_selector0.array_id_;
	int rank0 = sip_tables_.array_rank(array_id0);
	sip::Block::BlockPtr block0 = get_block_from_selector_stack(intent0,
			block_id0, true);
	if (num_args == 1) {
		SpecialInstructionManager::fp1 func =
				sip_tables_.special_instruction_manager_.get_one_arg_special_instruction_ptr(
						func_slot);
		//	typedef void(*fp1)(int& array_slot, int& rank, int * index_values, int& size, int * extents, double * block_data, int& ierr
		func(array_id0, rank0, block_id0.index_values_, block0->size_,
				block0->shape_.segment_sizes_, block0->data_, ierr);
		sip::check(ierr == 0,
				"error returned from special super instruction"
						+ sip_tables_.special_instruction_manager_.name(
								func_slot));
		return;
	}
	// num_args >= 2.  Set up second argument
	sip::BlockId block_id1;
	char intent1 = signature[1];
	sip::BlockSelector arg_selector1 = block_selector_stack_.top();
	int array_id1 = arg_selector1.array_id_;
	int rank1 = sip_tables_.array_rank(array_id1);
	sip::Block::BlockPtr block1 = get_block_from_selector_stack(intent1,
			block_id1);
	if (num_args == 2) {
		SpecialInstructionManager::fp2 func =
				sip_tables_.special_instruction_manager_.get_two_arg_special_instruction_ptr(
						func_slot);

		func(array_id0, rank0, block_id0.index_values_, block0->size_,
				block0->shape_.segment_sizes_, block0->data_, array_id1, rank1,
				block_id1.index_values_, block1->size_,
				block1->shape_.segment_sizes_, block1->data_, ierr);
		sip::check(ierr == 0,
				"error returned from special super instruction"
						+ sip_tables_.special_instruction_manager_.name(
								func_slot));
		return;
	}
	// num_args >=3.  Set up third argument
	sip::BlockId block_id2;
	char intent2 = signature[2];
	sip::BlockSelector arg_selector2 = block_selector_stack_.top();
	int array_id2 = arg_selector2.array_id_;
	int rank2 = sip_tables_.array_rank(array_id2);
	sip::Block::BlockPtr block2 = get_block_from_selector_stack(intent2,
			block_id2);
	if (num_args == 3) {
		SpecialInstructionManager::fp3 func =
				sip_tables_.special_instruction_manager_.get_three_arg_special_instruction_ptr(
						func_slot);

		func(array_id0, rank0, block_id0.index_values_, block0->size_,
				block0->shape_.segment_sizes_, block0->data_, array_id1, rank1,
				block_id1.index_values_, block1->size_,
				block1->shape_.segment_sizes_, block1->data_, array_id2, rank2,
				block_id2.index_values_, block2->size_,
				block2->shape_.segment_sizes_, block2->data_, ierr);
		sip::check(ierr == 0,
				"error returned from special super instruction"
						+ sip_tables_.special_instruction_manager_.name(
								func_slot));
		return;
	}

	//. num_args >= 4.  Set up fourth argument
	sip::BlockId block_id3;
	char intent3 = signature[3];
	sip::BlockSelector arg_selector3 = block_selector_stack_.top();
	int array_id3 = arg_selector3.array_id_;
	int rank3 = sip_tables_.array_rank(array_id3);
	sip::Block::BlockPtr block3 = get_block_from_selector_stack(intent3,
			block_id3);
	if (num_args == 4) {
		SpecialInstructionManager::fp4 func =
				sip_tables_.special_instruction_manager_.get_four_arg_special_instruction_ptr(
						func_slot);
		func(array_id0, rank0, block_id0.index_values_, block0->size_,
				block0->shape_.segment_sizes_, block0->data_, array_id1, rank1,
				block_id1.index_values_, block1->size_,
				block1->shape_.segment_sizes_, block1->data_, array_id2, rank2,
				block_id2.index_values_, block2->size_,
				block2->shape_.segment_sizes_, block2->data_, array_id3, rank3,
				block_id3.index_values_, block3->size_,
				block3->shape_.segment_sizes_, block3->data_, ierr);
		sip::check(ierr == 0,
				"error returned from special super instruction"
						+ sip_tables_.special_instruction_manager_.name(
								func_slot));
		return;
	}
	//. num_args >= 5.  Set up fifth argument
	sip::BlockId block_id4;
	char intent4 = signature[4];
	sip::BlockSelector arg_selector4 = block_selector_stack_.top();
	int array_id4 = arg_selector4.array_id_;
	int rank4 = sip_tables_.array_rank(array_id4);
	sip::Block::BlockPtr block4 = get_block_from_selector_stack(intent4,
			block_id4);
	if (num_args == 5) {
		SpecialInstructionManager::fp5 func =
				sip_tables_.special_instruction_manager_.get_five_arg_special_instruction_ptr(
						func_slot);
		func(array_id0, rank0, block_id0.index_values_, block0->size_,
				block0->shape_.segment_sizes_, block0->data_, array_id1, rank1,
				block_id1.index_values_, block1->size_,
				block1->shape_.segment_sizes_, block1->data_, array_id2, rank2,
				block_id2.index_values_, block2->size_,
				block2->shape_.segment_sizes_, block2->data_, array_id3, rank3,
				block_id3.index_values_, block3->size_,
				block3->shape_.segment_sizes_, block3->data_, array_id4, rank4,
				block_id4.index_values_, block4->size_,
				block4->shape_.segment_sizes_, block4->data_, ierr);
		sip::check(ierr == 0,
				"error returned from special super instruction"
						+ sip_tables_.special_instruction_manager_.name(
								func_slot));
		return;
	}
	//. num_args >= 6.  Set up sixth argument
	sip::BlockId block_id5;
	char intent5 = signature[5];
	sip::BlockSelector arg_selector5 = block_selector_stack_.top();
	int array_id5 = arg_selector5.array_id_;
	int rank5 = sip_tables_.array_rank(array_id5);
	sip::Block::BlockPtr block5 = get_block_from_selector_stack(intent5,
			block_id5);
	if (num_args == 6) {
		SpecialInstructionManager::fp6 func =
				sip_tables_.special_instruction_manager_.get_six_arg_special_instruction_ptr(
						func_slot);
		func(array_id0, rank0, block_id0.index_values_, block0->size_,
				block0->shape_.segment_sizes_, block0->data_, array_id1, rank1,
				block_id1.index_values_, block1->size_,
				block1->shape_.segment_sizes_, block1->data_, array_id2, rank2,
				block_id2.index_values_, block2->size_,
				block2->shape_.segment_sizes_, block2->data_, array_id3, rank3,
				block_id3.index_values_, block3->size_,
				block3->shape_.segment_sizes_, block3->data_, array_id4, rank4,
				block_id4.index_values_, block4->size_,
				block4->shape_.segment_sizes_, block4->data_, array_id5, rank5,
				block_id5.index_values_, block5->size_,
				block5->shape_.segment_sizes_, block5->data_, ierr);
		sip::check(ierr == 0,
				"error returned from special super instruction"
						+ sip_tables_.special_instruction_manager_.name(
								func_slot));
		return;
	}

	sip::check(false,
			"Implementation restriction:  At most 6 arguments to a super instruction supported.  This can be increased if necessary");
}

void Interpreter::handle_slice_op(int pc) {
	int lhs = op_table_.result_array(pc); //slot in array table for lhs
	int rank = sip_tables_.array_table_.rank(lhs);
	bool is_scope_extent = sip_tables_.is_scope_extent(lhs);

	//get rhs block, offsets, and extents
	//this is a special case, because we need to get the superblock
	int rhs = op_table_.op1_array(pc);    //slot in array table for rhs
	sip::BlockSelector rhs_selector = block_selector_stack_.top();
	block_selector_stack_.pop();
	sip::check(data_manager_.is_subblock(rhs_selector), //at least one index is subindex
	"Compiler or sip bug:  rhs should be subblock");
	sip::BlockId rhs_super_block_id = data_manager_.super_block_id(
			rhs_selector);
	sip::Block::BlockPtr super_block = sial_ops_.get_block_for_reading(
			rhs_super_block_id);  //for this
	sip::offset_array_t offsets;
	sip::BlockShape subblock_shape;
	data_manager_.get_subblock_offsets_and_shape(super_block, rhs_selector,
			offsets, subblock_shape);

	//get lhs block
	sip::Block::BlockPtr lhs_block = get_block_from_selector_stack('w', false);
	//calls Dmitry's routine (via the Block method)
	super_block->extract_slice(rank, offsets, lhs_block);
}

void Interpreter::handle_insert_op(int pc) {
	int lhs = op_table_.result_array(pc); //slot in array table for lhs
	int rank = sip_tables_.array_table_.rank(lhs);
	bool is_scope_extent = sip_tables_.is_scope_extent(lhs);

	//get rhs block
	sip::Block::BlockPtr rhs_block = get_block_from_selector_stack('r', false);

	//get lhs block.  This requires getting the superblock
	sip::BlockSelector lhs_selector = block_selector_stack_.top();
	block_selector_stack_.pop();
	sip::BlockId lhs_super_block_id = data_manager_.super_block_id(
			lhs_selector);
	sip::Block::BlockPtr super_block =
			data_manager_.block_manager_.get_block_for_writing(
					lhs_super_block_id, is_scope_extent);
	sip::offset_array_t offsets;
	sip::BlockShape subblock_shape;
	data_manager_.get_subblock_offsets_and_shape(super_block, lhs_selector,
			offsets, subblock_shape);

	super_block->insert_slice(rank, offsets, rhs_block);
}

void Interpreter::handle_assignment_op(int pc) {
	int lhs = op_table_.result_array(pc); //slot in array table for lhs
	int rhs = op_table_.op1_array(pc);    //slot in array table for rhs
	if (is_scalar(lhs) && is_scalar(rhs)) { //both scalars
		set_scalar_value(lhs, scalar_value(rhs));
		return;
	}
	if (is_scalar(lhs)) { //rhs is a data block, indices should be simple
		sip::BlockSelector rhs_selector = block_selector_stack_.top();
		sip::Block::BlockPtr rhs_block = get_block_from_selector_stack('r');
		double value = rhs_block->get_data()[0];
		set_scalar_value(lhs, value);
		return;
	}
	bool is_scope_extent = sip_tables_.is_scope_extent(lhs);
	if (is_scalar(rhs)) { //fill block with scalar value
#ifdef HAVE_CUDA
			if (gpu_enabled_) {
				sip::Block::BlockPtr g_lhs_block = get_gpu_block('w');
				g_lhs_block->gpu_fill(scalar_value(rhs));
				return;
			}
#endif
		sip::Block::BlockPtr lhs_block = get_block_from_selector_stack('w');
		lhs_block->fill(scalar_value(rhs));
		return;
	} else { //both sides are arrays, get blocks, save selectors

		int lhs_rank = sip_tables_.array_rank(lhs);
		int rhs_rank = sip_tables_.array_rank(rhs);

		sip::BlockSelector rhs_selector = block_selector_stack_.top();
		block_selector_stack_.pop();
		sip::BlockSelector lhs_selector = block_selector_stack_.top();
		block_selector_stack_.pop();

		sip::Block::BlockPtr rhs_block = NULL;
		sip::Block::BlockPtr lhs_block = NULL;

		sip::BlockId rhs_blockid;
		sip::BlockId lhs_blockid;

#ifdef HAVE_CUDA
		if (gpu_enabled_) {
			rhs_block = get_gpu_block('r', rhs_selector, rhs_blockid);
			lhs_block = get_gpu_block('w', lhs_selector, lhs_blockid);
		} else
#endif
		{
			rhs_block = get_block('r', rhs_selector, rhs_blockid);
			lhs_block = get_block('w', lhs_selector, lhs_blockid);
		}

		//check for self assignment
		if (lhs_block->get_data() == rhs_block->get_data()) {
			return;
		}

		if (lhs_rank <= rhs_rank) {
			//Do indices match?  If so, this is an ordinary (possibly dimension reducing) copy
			if (std::equal(lhs_selector.index_ids_ + 0,
					lhs_selector.index_ids_ + lhs_rank,
					rhs_selector.index_ids_ + 0)) {

#ifdef HAVE_CUDA
				if (gpu_enabled_) {
					lhs_block->gpu_copy_data(rhs_block);
					return;
				}
#endif
				//indices match, this is an ordinary (possibly dimension reducing) copy
				lhs_block->copy_data_(rhs_block);
				return;
			}
			if (lhs_rank == rhs_rank) {
				//this is a transpose.  calculate permutation
				int permutation[MAX_RANK];
				std::fill(permutation + 0, permutation + MAX_RANK, 0);

				for (int i = 0; i < lhs_rank; ++i) {
					int lhs_index = lhs_selector.index_ids_[i];
					int j;
					for (j = 0;
							j < MAX_RANK
									&& rhs_selector.index_ids_[j] != lhs_index;
							++j) {/* keep looking until matching index found */
					}
					sip::check(j < lhs_rank, "illegal transpose");
					permutation[j] = i;
				}
				for (int i = lhs_rank; i < MAX_RANK; ++i) { //fill in unused dims with -1 to cause failure if accessed
					permutation[i] = -1;
				}
				//DEBUG
				//VFLstd::cout << "permutation matrix: ";
				//VFLfor (int i = 0; i < MAX_RANK; i++)
				//VFL	std::cout << permutation[i] << " ";
				//VFLstd::cout << std::endl;
#ifdef HAVE_CUDA
				if (gpu_enabled_) {
					_gpu_permute(lhs_block->get_gpu_data(), lhs_rank, lhs_block->shape().segment_sizes_, lhs_selector.index_ids_,
							rhs_block->get_gpu_data(), lhs_rank, rhs_block->shape().segment_sizes_ , rhs_selector.index_ids_);
					return;
				}
#endif
				lhs_block->transpose_copy(rhs_block, lhs_rank, permutation);
				return;
			}

			check(false, "illegal indices in assignment", line_number());
		} else { //lhs_rank > rhs_rank, the compiler should have checked that extra indices are simple
			lhs_block->copy_data_(rhs_block);
			return;
		}
	}
	check(false, "illegal indices in assignment");
}

void Interpreter::handle_contraction_op(int pc) {
	//using dmitry's notation, i.e. d = l * r
	//determine rank of arguments
	int darray = op_table_.result_array(pc);
	int drank = sip_tables_.array_rank(darray);
	int larray = op_table_.op1_array(pc);
	int lrank = sip_tables_.array_rank(larray);
	int rarray = op_table_.op2_array(pc);
	int rrank = sip_tables_.array_rank(rarray);

	if (lrank == 0 && rrank == 0) {	//rhs is scalar * scalar
		set_scalar_value(darray, scalar_value(larray) * scalar_value(rarray));
		return;
	}

	if (lrank == 0) {  //rhs is scalar * block
		double lvalue = scalar_value(larray);
		sip::BlockId rid;
		sip::Block::BlockPtr rblock = get_block_from_selector_stack('r', rid);
		sip::BlockId did;
		sip::Block::BlockPtr dblock = get_block_from_selector_stack('w', did);
		double * ddata = dblock->get_data();
		double * rdata = rblock->get_data();
		for (int i = 0; i < dblock->size(); ++i) {
			ddata[i] = lvalue * rdata[i];
		}
		return;
	}
	if (rrank == 0) {  //rhs is block * scalar
		double rvalue = scalar_value(rarray);
		sip::BlockId lid;
		sip::Block::BlockPtr lblock = get_block_from_selector_stack('r', lid);
		sip::BlockId did;
		sip::Block::BlockPtr dblock = get_block_from_selector_stack('w', did);
		double * ddata = dblock->get_data();
		double * rdata = lblock->get_data();
		for (int i = 0; i < dblock->size(); ++i) {
			ddata[i] = rdata[i] * rvalue;
		}
		return;
	}

	//get destination operand info
	bool d_is_scalar = is_scalar(op_table_.result_array(pc));

#ifdef HAVE_CUDA
	if (gpu_enabled_) {
		std::cout<<"Contraction on the GPU at line "<<line_number()<<std::endl;

		sip::BlockSelector rselector = block_selector_stack_.top();
		sip::Block::BlockPtr g_rblock = get_gpu_block('r');

		sip::BlockSelector lselector = block_selector_stack_.top();
		sip::Block::BlockPtr g_lblock = get_gpu_block('r');

		sip::Block::BlockPtr g_dblock;
		sip::index_selector_t g_dselected_index_ids;
		double *g_d;
		if (d_is_scalar) {
			double h_d = 0.0;
			sip::BlockShape scalar_shape;
			g_dblock = new sip::Block(scalar_shape);
			g_dblock->gpu_data_ = _gpu_allocate(1);
			std::fill(g_dselected_index_ids + 0, g_dselected_index_ids + MAX_RANK, sip::unused_index_slot);
		} else {
			sip::BlockSelector dselector = block_selector_stack_.top();
			std::copy(dselector.index_ids_ + 0, dselector.index_ids_ + MAX_RANK, g_dselected_index_ids + 0);
			g_dblock = get_gpu_block('w');
		}

		_gpu_contract(g_dblock->get_gpu_data(), drank, g_dblock->shape().segment_sizes_, g_dselected_index_ids,
				g_lblock->get_gpu_data(), lrank, g_lblock->shape().segment_sizes_, lselector.index_ids_,
				g_rblock->get_gpu_data(), rrank, g_rblock->shape().segment_sizes_, rselector.index_ids_);

		if (d_is_scalar) {
			double h_dbl;
			_gpu_device_to_host(&h_dbl, g_dblock->get_gpu_data(), 1);
			set_scalar_value(op_table_.result_array(pc), h_dbl);
		}

		return;
	}
#endif
	//std::cout<<"Contraction at line "<<line_number()<<std::endl;

	//get right operand info
	sip::BlockSelector rselector = block_selector_stack_.top();
	sip::check(rrank == sip_tables_.array_rank(rselector.array_id_),
			"SIP or compiler bug, inconsistent for r arg in contract op");
	sip::BlockId rid;
	sip::Block::BlockPtr rblock = get_block_from_selector_stack('r', rid);
	sip::check(rarray == rid.array_id(),
			"SIP or compiler bug:  inconsistent array ids in contract op");

	//get left operand info
	sip::BlockSelector lselector = block_selector_stack_.top();
	sip::check(lrank == sip_tables_.array_rank(lselector.array_id_),
			"SIP or compiler bug, inconsistent for l arg in contract op");
	sip::BlockId lid;
	sip::Block::BlockPtr lblock = get_block_from_selector_stack('r', lid);

	//If result of contraction is scalar.
	sip::Block::BlockPtr dblock = NULL;
	sip::index_selector_t dselected_index_ids;
	if (d_is_scalar) {
		double dval = scalar_value(op_table_.result_array(pc));
		dval = 0; // Initialize to 0 as required by Dmitry's Contraction routine
		double *dvalPtr = new double[1];
		*dvalPtr = dval;
		sip::BlockShape scalar_shape;
		dblock = new sip::Block(scalar_shape, dvalPtr);
		std::fill(dselected_index_ids + 0, dselected_index_ids + MAX_RANK,
				sip::unused_index_slot);
	} else {
		sip::BlockSelector dselector = block_selector_stack_.top();
		std::copy(dselector.index_ids_ + 0, dselector.index_ids_ + MAX_RANK,
				dselected_index_ids + 0);
		sip::check(drank == sip_tables_.array_rank(dselector.array_id_),
				"SIP or compiler bug, inconsistent for d arg in contract op");
		sip::BlockId did;
		dblock = get_block_from_selector_stack('w', did);

	}

	//get contraction pattern
	int pattern_size = drank + lrank + rrank;
	std::vector<int> aces_pattern(pattern_size, 0);	// Initialize vector with pattern_size elements of value 0
	std::vector<int>::iterator it;
	it = aces_pattern.begin();

	it = std::copy(dselected_index_ids + 0, dselected_index_ids + drank, it);
	it = std::copy(lselector.index_ids_ + 0, lselector.index_ids_ + lrank, it);
	it = std::copy(rselector.index_ids_ + 0, rselector.index_ids_ + rrank, it);

//	int contraction_pattern[lrank + rrank];
	int contraction_pattern[MAX_RANK * 2];
	int ierr;

	get_contraction_ptrn_(drank, lrank, rrank, aces_pattern.data(),
			contraction_pattern, ierr);
	check(ierr == 0, std::string("error returned from get_contraction_ptrn_"),
			line_number());
//    INPUT:
//    ! - nthreads - number of threads requested;
//    ! - contr_ptrn(1:left_rank+right_rank) - contraction pattern;
//    ! - ltens/ltens_num_dim/ltens_dim_extent - left tensor block / its rank / and dimension extents;
//    ! - rtens/rtens_num_dim/rtens_dim_extent - right tensor block / its rank / and dimension extents;
//    ! - dtens/dtens_num_dim/dtens_dim_extent - destination tensor block (initialized!) / its rank / and dimension extents;
//    !OUTPUT:
//    ! - dtens - modified destination tensor block;
//    ! - ierr - error code (0: success);
//     */
	int nthreads = sip::MAX_OMP_THREADS;
	tensor_block_contract__(nthreads, contraction_pattern, lblock->get_data(),
			lrank, lblock->shape().segment_sizes_, rblock->get_data(), rrank,
			rblock->shape().segment_sizes_, dblock->get_data(), drank,
			dblock->shape().segment_sizes_, ierr);
	//std::cout <<"scalar:" << dblock.get_data()[0] << std::endl;
	if (d_is_scalar)
		set_scalar_value(op_table_.result_array(pc), *(dblock->get_data()));

}

bool Interpreter::evaluate_where_clause(int pc) {
	int op1_slot = op_table_.op1_array(pc);
	int op1_value = index_value(op1_slot);
	int where_op = op_table_.op2_array(pc);
	int op2_slot = op_table_.result_array(pc);
	int op2_value = index_value(op2_slot);
	bool result;
	switch (where_op) {
	case where_eq:
		result = op1_value == op2_value;
		break;
	case where_geq:
		result = op1_value >= op2_value;
		break;
	case where_leq:
		result = op1_value <= op2_value;
		break;
	case where_gt:
		result = op1_value > op2_value;
		break;
	case where_lt:
		result = op1_value < op2_value;
		break;
	case where_neq:
		result = op1_value != op2_value;
		break;
	default: {
		std::cout << "where opcode =" << where_op;
		check(false, " illegal where operator code");
	}
	}
	return result;
}

//the different treatment of where clauses and relational expressions
//is inherited from ACESIII.
bool Interpreter::evaluate_int_relational_expr(int pc) {
	int val2 = control_stack_.top();
	control_stack_.pop();
	int val1 = control_stack_.top();
	control_stack_.pop();
	bool result;
	int opcode = op_table_.opcode(pc);
	switch (opcode) {
	case sp_equal_op:
		result = (val1 == val2);
		break;
	case sp_nequal_op:
		result = (val1 != val2);
		break;
	case sp_ge_op:
		result = (val1 >= val2);
		break;
	case sp_le_op:
		result = (val1 <= val2);
		break;
	case sp_gt_op:
		result = (val1 > val2);
		break;
	case sp_lt_op:
		result = (val1 < val2);
		break;
	default: {
		std::cout << "int relational expression opcode =" << opcode;
		check(false, " illegal operator code");
	}
	}
	return result;
}

bool Interpreter::evaluate_double_relational_expr(int pc) {
	double val2 = expression_stack_.top();
	expression_stack_.pop();
	double val1 = expression_stack_.top();
	expression_stack_.pop();
	bool result;
	int opcode = op_table_.opcode(pc);
	switch (opcode) {
	case fl_eq_op:
		result = (val1 == val2);
		break;
	case fl_ne_op:
		result = (val1 != val2);
		break;
	case fl_ge_op:
		result = (val1 >= val2);
		break;
	case fl_le_op:
		result = (val1 <= val2);
		break;
	case fl_gt_op:
		result = (val1 > val2);
		break;
	case fl_lt_op:
		result = (val1 < val2);
		break;
	default: {
		std::cout << "double relational expression opcode =" << opcode;
		check(false, " illegal operator code");
	}
	}
	return result;
}

void Interpreter::handle_sum_op(int pc, double factor) {
	//using dmitry's notation, i.e. d = l + r

	bool d_is_scalar, l_is_scalar, r_is_scalar;
	d_is_scalar = is_scalar(op_table_.result_array(pc));
	l_is_scalar = is_scalar(op_table_.op1_array(pc));
	r_is_scalar = is_scalar(op_table_.op2_array(pc));
	//TODO introduce debug level--or just delete?
	check(d_is_scalar == l_is_scalar && l_is_scalar == r_is_scalar,
			"compiler of sip bug:  inconsistent array/scalars in sum");

	if (d_is_scalar) {
		double lval = scalar_value(op_table_.op1_array(pc));
		double rval = scalar_value(op_table_.op2_array(pc));
		double dval = lval + (rval * factor);
		set_scalar_value(op_table_.result_array(pc), dval);
		return;
	}
	// this is a  block operation

#ifdef HAVE_CUDA
	if (gpu_enabled_) {
		std::cout<<"Block addition / subtraction on the GPU at line "<<line_number()<<std::endl;
		sip::Block::BlockPtr g_rblock = get_gpu_block('r');
		sip::Block::BlockPtr g_lblock = get_gpu_block('r');
		sip::Block::BlockPtr g_dblock = get_gpu_block('w');
		_gpu_device_to_device(g_dblock->get_gpu_data(), g_lblock->get_gpu_data(), g_lblock->size());
		_gpu_axpy(g_dblock->get_gpu_data(), g_rblock->get_gpu_data(), factor, g_lblock->size());
		return;
	}
#endif
	//get right operand info
//TODO introduce debug level--or just delete?
//	array::BlockSelector rselector = block_selector_stack_.top();
//	int rrank = sipTables_.array_rank(rselector.array_id_);
//	check(rselector.array_id_ == op_table_.op2_array(pc),
//			"compiler or sip bug:  inconsistent array values in sum and selector for r value");
	sip::BlockId rid;
	sip::Block::BlockPtr rblock = get_block_from_selector_stack('r', rid);

	//get left operand info
//TODO introduce debug level--or just delete?
//	array::BlockSelector lselector = block_selector_stack_.top();
//	int lrank = sipTables_.array_rank(lselector.array_id_);
//	check(lselector.array_id_ == op_table_.op1_array(pc),
//			"compiler or sip bug:  inconsistent array values in sum and selector for l value");
	sip::BlockId lid;
	sip::Block::BlockPtr lblock = get_block_from_selector_stack('r', lid);

	//get destination operand info
	sip::BlockSelector dselector = block_selector_stack_.top();
	int drank = sip_tables_.array_rank(dselector.array_id_);
//	check(dselector.array_id_ == op_table_.result_array(pc),
//			"compiler or sip bug:  inconsistent array values in sum and selector for d value");
	sip::BlockId did;
	sip::Block::BlockPtr dblock = get_block_from_selector_stack('w', did);

	/*
	 subroutine tensor_block_add_(nthreads,dim_num,dim_extents,tens0,tens1,scale_fac,ierr) bind(c,name='tensor_block_add__') !PARALLEL
	 !This subroutine adds tensor block <tens1> to tensor block <tens0>:
	 !tens0(:)+=tens1(:)*scale_fac
	 !INPUT:
	 ! - nthreads - number of threads requested;
	 ! - dim_num - number of dimensions;
	 ! - dim_extents(1:dim_num) - dimension extents;
	 ! - tens0, tens1 - tensor blocks;
	 ! - scale_fac - scaling factor;
	 !OUTPUT:
	 ! - tens0 - modified tensor block;
	 ! - ierr - error code (0:success);
	 void tensor_block_add__(int&, int&, int*, double*, double*, double&, int&);
	 */
	int ierr;
	int nthreads = sip::MAX_OMP_THREADS;
	//TODO Fix this eventually
	// Compiler converts x += y into x = x + y.  Dmity's routines only do x+=y.
	//For now, check form.  If x = y + z, then copy y into x, then add z.
	//TODO ALSO consider adding support for x += y*c to SIAL.
	if (lid != did) {
		size_t size = dblock->size();
		std::copy(lblock->get_data() + 0, lblock->get_data() + size,
				dblock->get_data() + 0);
	}

	tensor_block_add__(nthreads, drank, dblock->shape().segment_sizes_,
			dblock->get_data(), rblock->get_data(), factor, ierr);
	check(ierr == 0, "error returned from tensor_block_add__");
}

void Interpreter::handle_self_multiply_op(int pc) {
	check(is_scalar(op_table_.op2_array(pc)),
			"*= only for scalar right hand side", line_number());
	bool d_is_scalar = is_scalar(op_table_.result_array(pc));
	double lval = scalar_value(op_table_.op2_array(pc));
	if (d_is_scalar) {
		double dval = scalar_value(op_table_.result_array(pc));
		dval *= lval;
		set_scalar_value(op_table_.result_array(pc), dval);
	} else {
		sip::BlockSelector dselector = block_selector_stack_.top();
		int drank = sip_tables_.array_rank(dselector.array_id_);
		check(dselector.array_id_ == op_table_.result_array(pc),
				"compiler or sip bug:  inconsistent array values in self multiply and selector for d value",
				line_number());

#ifdef HAVE_CUDA
		if (gpu_enabled_) {
			sip::BlockId did;
			sip::Block::BlockPtr dblock = get_gpu_block('u', did);
			block_selector_stack_.pop();
			dblock->gpu_scale(lval);
			//************ check for write_back_contiguous
		} else

#endif
		{
			//TODO introduce debug level--or just delete?
			sip::BlockId did;
			sip::Block::BlockPtr dblock = get_block_from_selector_stack('u',
					did);
//			block_selector_stack_.pop();
			dblock->scale(lval);
		}
	}
}

void Interpreter::loop_start(LoopManager * loop) {
	int enddo_pc = op_table_.go_to_target(pc);
	if (loop->update()) { //there is at least one iteration of loop
		loop_manager_stack_.push(loop);
		control_stack_.push(pc + 1); //push pc of first instruction of loop body (including where)
		control_stack_.push(enddo_pc); //used by where clauses
		data_manager_.enter_scope();
		++pc;
	} else {
		loop->finalize();
		delete loop;
		pc = enddo_pc + 1; //jump to statement following enddo
	}
}
void Interpreter::loop_end() {
	int own_pc = pc;
	control_stack_.pop(); //remove own location
	data_manager_.leave_scope();
	bool more_iterations = loop_manager_stack_.top()->update();
	if (more_iterations) {
		data_manager_.enter_scope();
		pc = control_stack_.top(); //get the place to jump back to
		control_stack_.push(own_pc); //add own location for where clause in next iteration
	} else {
		LoopManager* loop = loop_manager_stack_.top();
		loop->finalize();
		loop_manager_stack_.pop(); //remove loop from stack
		control_stack_.pop(); //pop address of first instruction in body
		delete loop;
		++pc;
	}
}

sip::BlockId Interpreter::get_block_id_from_selector_stack() {
	sip::BlockSelector selector = block_selector_stack_.top();
	block_selector_stack_.pop();
	return block_id(selector);
}

sip::Block::BlockPtr Interpreter::get_block_from_selector_stack(char intent,
		sip::BlockId& id, bool contiguous_allowed) {
	sip::BlockSelector selector = block_selector_stack_.top();
	block_selector_stack_.pop();
	return get_block(intent, selector, id);
}

sip::Block::BlockPtr Interpreter::get_block_from_selector_stack(char intent,
		bool contiguous_allowed) {
	sip::BlockId block_id;
	return get_block_from_selector_stack(intent, block_id, contiguous_allowed);
}

sip::Block::BlockPtr Interpreter::get_block(char intent,
		sip::BlockSelector& selector, sip::BlockId& id,
		bool contiguous_allowed) {
	int array_id = selector.array_id_;
	sip::Block::BlockPtr block;
	if (sip_tables_.array_rank(selector.array_id_) == 0) { //this "array" was declared to be a scalar.  Nothing to remove from selector stack.
		id = sip::BlockId(array_id);
		block = data_manager_.get_scalar_block(array_id);
		return block;
	}
	if (selector.rank_ == 0) { //this is a static array provided without a selector, block is entire array
		block = data_manager_.contiguous_array_manager_.get_array(array_id);
		id = sip::BlockId(array_id);
		return block;
	}
	//argument is a block with an explicit selector
	sip::check(selector.rank_ == sip_tables_.array_rank(selector.array_id_),
			"SIP or Compiler bug: inconsistent ranks in sipTable and selector");
	id = block_id(selector);
	bool is_contiguous = sip_tables_.is_contiguous(selector.array_id_);
	sip::check(!is_contiguous || contiguous_allowed,
			"using contiguous block in a context that doesn't support it");
	switch (intent) {
	case 'r': {
		block = is_contiguous ?
				data_manager_.contiguous_array_manager_.get_block_for_reading(
						id) :
				sial_ops_.get_block_for_reading(id);
	}
		break;
	case 'w': {
		bool is_scope_extent = sip_tables_.is_scope_extent(selector.array_id_);
		block = is_contiguous ?
				data_manager_.contiguous_array_manager_.get_block_for_updating( //w and u are treated identically for contiguous arrays
						id, write_back_list_) :
				sial_ops_.get_block_for_writing(id, is_scope_extent);
	}
		break;
	case 'u': {
		block = is_contiguous ?
				data_manager_.contiguous_array_manager_.get_block_for_updating(
						id, write_back_list_) :
				sial_ops_.get_block_for_updating(id);
	}
		break;
	default:
		sip::check(false,
				"SIP bug:  illegal or unsupported intent given to get_block");
	}
	return block;

}

void Interpreter::write_back_contiguous() {
	//DEBUG
	int list_size = write_back_list_.size();
	//VFLif (list_size >1){ std::cout << "SIZE OF WRITE BACKLIST = "<< write_back_list_.size() << std::endl;}
	while (!write_back_list_.empty()) {
		sip::WriteBack * ptr = write_back_list_.front();
#ifdef HAVE_CUDA
		if (gpu_enabled_) {
			sip::Block::BlockPtr cblock = ptr->get_block();
			data_manager_.block_manager_.lazy_gpu_read_on_host(cblock);
		}
#endif
		ptr->do_write_back();
		//DEBUG
		//VFLif (list_size > 1){
		//VFLstd::cout << "WRITE_BACK: " << std::endl;
		//VFLstd::cout << *ptr << std::endl;
		//VFL}
		write_back_list_.erase(write_back_list_.begin());
		delete ptr;
	}
	//DEBUG
	//VFLif (list_size > 1) {std::cout << "returning from write_back_contiguous" << std::endl;}
}

#ifdef HAVE_CUDA
sip::Block::BlockPtr Interpreter::get_gpu_block(char intent, sip::BlockId& id, bool contiguous_allowed) {
	sip::BlockSelector selector = block_selector_stack_.top();
	block_selector_stack_.pop();
	return get_gpu_block(intent, selector, id, contiguous_allowed);
}

sip::Block::BlockPtr Interpreter::get_gpu_block(char intent, bool contiguous_allowed) {
	sip::BlockId block_id;
	return get_gpu_block(intent, block_id, contiguous_allowed);
}

sip::Block::BlockPtr Interpreter::get_gpu_block(char intent, sip::BlockSelector& selector, sip::BlockId& id,
		bool contiguous_allowed) {
	id = block_id(selector);
	sip::Block::BlockPtr block;
	bool is_contiguous = sip_tables_.is_contiguous(selector.array_id_);
	sip::check(!is_contiguous || contiguous_allowed,
			"using contiguous block in a context that doesn't support it");

	switch (intent) {
		case 'r': {
			if (is_contiguous) {
				block = data_manager_.contiguous_array_manager_.get_block_for_reading(id);
				data_manager_.block_manager_.lazy_gpu_read_on_device(block);
			} else {
				block = data_manager_.block_manager_.get_gpu_block_for_reading(id);
			}
		}
		break;
		case 'w': {
			bool is_scope_extent = sip_tables_.is_scope_extent(selector.array_id_);
			if (is_contiguous) {
				block = data_manager_.contiguous_array_manager_.get_block_for_updating(
						id, write_back_list_);
				sip::BlockShape shape = sip_tables_.shape(id);
				data_manager_.block_manager_.lazy_gpu_write_on_device(block, id, shape);
			} else {
				block = data_manager_.block_manager_.get_gpu_block_for_writing(id, is_scope_extent);
			}
		}
		break;
		case 'u': {
			if (is_contiguous) {
				block =
				data_manager_.contiguous_array_manager_.get_block_for_updating(
						id, write_back_list_);
				sip::BlockShape shape = sip_tables_.shape(id);
				data_manager_.block_manager_.lazy_gpu_update_on_device(block);
			} else {
				block = data_manager_.block_manager_.get_gpu_block_for_updating(id);
			}
		}
		break;
		default:
		sip::check(false, "illegal or unsupported intent given to get_block");
	}
	return block;
}

sip::Block::BlockPtr Interpreter::get_gpu_block_from_selector_stack(char intent,
		sip::BlockId& id, bool contiguous_allowed) {
	sip::Block::BlockPtr block;
	sip::BlockSelector selector = block_selector_stack_.top();
	block_selector_stack_.pop();
	int array_id = selector.array_id_;
	if (sip_tables_.array_rank(selector.array_id_) == 0) { //this is a scalar
		block = new sip::Block(data_manager_.scalar_address(array_id));
		id = sip::BlockId(array_id);
		return block;
	}
	sip::check(selector.rank_ != 0, "Contiguous arrays not supported for GPU", line_number());

	//argument is a block with an explicit selector
	sip::check(selector.rank_ == sip_tables_.array_rank(selector.array_id_),
			"SIP or Compiler bug: inconsistent ranks in sipTable and selector");
	id = block_id(selector);
	bool is_contiguous = sip_tables_.is_contiguous(selector.array_id_);

	switch (intent) {
		case 'r': {
			block = data_manager_.block_manager_.get_gpu_block_for_reading(id);
		}
		break;
		case 'w': {
			bool is_scope_extent = sip_tables_.is_scope_extent(selector.array_id_);
			block = data_manager_.block_manager_.get_gpu_block_for_writing(id,
					is_scope_extent);
		}
		break;
		case 'u': {
			block = data_manager_.block_manager_.get_gpu_block_for_updating(id); //w and u same for contig
		}
		break;
		default:
		sip::check(false, "illegal or unsupported intent given to get_block");
	}

	return block;
}

#endif

}
/* namespace sip */
