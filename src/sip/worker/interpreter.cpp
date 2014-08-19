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

#include "aces_defs.h"
#include "loop_manager.h"
#include "special_instructions.h"
#include "block.h"
#include "tensor_ops_c_prototypes.h"
#include "worker_persistent_array_manager.h"
#include "sial_printer.h"
#include "tracer.h"
#include "sial_math.h"
#include "config.h"
#include "sial_math.h"

// For CUDA Super Instructions
#ifdef HAVE_CUDA
#include "gpu_super_instructions.h"
#endif

namespace sip {

Interpreter* Interpreter::global_interpreter;

Interpreter::Interpreter(SipTables& sipTables, SialxTimer& sialx_timer,
		SialPrinter* printer,
		WorkerPersistentArrayManager* persistent_array_manager) :
		sip_tables_(sipTables), sialx_timers_(sialx_timer), printer_(printer), data_manager_(
				sipTables), op_table_(sipTables.op_table_), persistent_array_manager_(
				persistent_array_manager), sial_ops_(data_manager_,
				persistent_array_manager, sipTables){
	_init(sipTables);
}

Interpreter::Interpreter(SipTables& sipTables, SialxTimer& sialx_timer,
		WorkerPersistentArrayManager* persistent_array_manager) :
		sip_tables_(sipTables), sialx_timers_(sialx_timer), printer_(NULL), data_manager_(
				sipTables), op_table_(sipTables.op_table_), persistent_array_manager_(
				persistent_array_manager), sial_ops_(data_manager_,
				persistent_array_manager, sipTables) {
	_init(sipTables);
}

Interpreter::Interpreter(SipTables& sipTables, SialPrinter* printer) :
		sip_tables_(sipTables), op_table_(sipTables.op_table_), sialx_timers_(
				op_table_.max_line_number()), printer_(printer), data_manager_(
				sipTables), persistent_array_manager_(NULL), sial_ops_(
				data_manager_, persistent_array_manager_, sipTables) {
	_init(sipTables);
}

Interpreter::Interpreter(SipTables& sipTables, SialPrinter* printer, WorkerPersistentArrayManager* wpm):
				sip_tables_(sipTables), op_table_(sipTables.op_table_), sialx_timers_(
						op_table_.max_line_number()), printer_(printer), data_manager_(
						sipTables), persistent_array_manager_(wpm), sial_ops_(
						data_manager_, persistent_array_manager_, sipTables) {
			_init(sipTables);
}

Interpreter::~Interpreter() {
	delete tracer_;
}

void Interpreter::_init(SipTables& sip_tables) {
	int num_indices = sip_tables.index_table_.entries_.size();
	//op_table_ = sipTables.op_table_;
	pc = 0;
	global_interpreter = this;
	gpu_enabled_ = false;
	tracer_ = new Tracer(this, sip_tables, std::cout);
	if (printer_ == NULL) printer_ = new SialPrinterForTests(std::cout, sip::SIPMPIAttr::get_instance().global_rank(), sip_tables);
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
		sip::check(write_back_list_.empty() && read_block_list_.empty(),
				"SIP bug:  write_back_list  or read_block_list not empty at top of interpreter loop");

		tracer_->trace(pc, opcode);



		switch (opcode) {
		case goto_op: {
			pc = arg0();
		}
			break;

		case jump_if_zero_op: {
			int i = control_stack_.top();
			control_stack_.pop();
			if (i == 0)
				pc = arg0();
			else
				++pc;
		}
			break;
		case stop_op: {
			fail("sial stop command caused program abort");
		}
			break;
		case call_op: {
			control_stack_.push(pc + 1);
			pc = arg0();
		}
			break;
		case return_op: {
			pc = control_stack_.top();
			control_stack_.pop();
		}
			break;
		case execute_op: {
			handle_user_sub_op(pc);
			contiguous_blocks_post_op();
			++pc;
		}
			break;
		case do_op: {
			int index_slot = index_selectors()[0];
			LoopManager* loop = new DoLoop(index_slot, data_manager_,
					sip_tables_);
			loop_start(loop);
		}
			break;
		case enddo_op: {
			loop_end();
		}
			break;
		case exit_op: {
			loop_manager_stack_.top()->set_to_exit();
			pc = control_stack_.top();
		}
			break;
		case where_op: {
			int where_clause_value = control_stack_.top();
			control_stack_.pop();
			if (where_clause_value) {
				++pc;
			} else
				pc = control_stack_.top(); //note that this clause must be in a loop and the enddo (or other endloop instruction will pop the value
		}
			break;
		case pardo_op: { //TODO refactor to get rid of the ifdefs
			int num_indices = arg1();
#ifdef HAVE_MPI
			LoopManager* loop = new StaticTaskAllocParallelPardoLoop(num_indices,
					index_selectors(), data_manager_, sip_tables_,
					SIPMPIAttr::get_instance());
#else
			LoopManager* loop = new SequentialPardoLoop(num_indices,
					index_selectors(), data_manager_, sip_tables_);
#endif
			loop_start(loop);
		}
			break;
		case endpardo_op: {
			loop_end();
		}
			break;
		case sip_barrier_op: {
			sial_ops_.sip_barrier();
			++pc;
		}
			break;
		case broadcast_static_op: {
			sial_ops_.broadcast_static(arg0(), control_stack_.top());
			control_stack_.pop();
			++pc;
		}
			break;
		case push_block_selector_op: {
			block_selector_stack_.push(
					sip::BlockSelector(arg0(), arg1(), index_selectors()));
			++pc;
		}
			break;
		case allocate_op: {

			sip::BlockId id = block_id(
					BlockSelector(arg0(), arg1(), index_selectors()));
			data_manager_.block_manager_.allocate_local(id);
			++pc;
		}
			break;
		case deallocate_op: {
			sip::BlockId id = block_id(
					sip::BlockSelector(arg0(), arg1(), index_selectors()));
			data_manager_.block_manager_.deallocate_local(id);
			++pc;
		}
			break;

			//Omitting allocate and deallocate contiguous

		case get_op: { //TODO  check this.  Have compiler put block info in instruction?
			sip::BlockId id = get_block_id_from_selector_stack();
			sial_ops_.get(id);
			++pc;
		}
			break;
		case put_accumulate_op: { //put a[..] += b[..]  TODO check documentation
			sip::Block::BlockPtr rhs_block = get_block_from_selector_stack('r',
					true);
			sip::BlockId lhs_id = get_block_id_from_selector_stack();
			sial_ops_.put_accumulate(lhs_id, rhs_block);
			++pc;
		}
			break;
		case put_replace_op: { //put a[...] = b[...]
			sip::Block::BlockPtr rhs_block = get_block_from_selector_stack('r',
					true);
			sip::BlockId lhs_id = get_block_id_from_selector_stack();
			sial_ops_.put_replace(lhs_id, rhs_block);
			++pc;
		}
			break;
		case create_op: {
			sial_ops_.create_distributed(arg0());
			++pc;
		}
			break;
		case delete_op: {
			sial_ops_.delete_distributed(arg0());
			++pc;
		}
			break;
		case string_load_literal_op: {
			control_stack_.push(arg0());
			++pc;
		}
			break;
		case int_load_value_op: {
			control_stack_.push(int_value(arg0()));
			++pc;
		}
			break;
		case int_load_literal_op: {
			control_stack_.push(arg0());
			++pc;
		}
			break;
		case int_store_op: {
			int op = arg1();
			if (op == int_store_op) {
				int value = control_stack_.top();
				set_int_value(arg0(), value);
				control_stack_.pop();
			} else if (op == int_add_op) {
				int val = int_value(arg0());
				set_int_value(arg0(), val + control_stack_.top());
				control_stack_.pop();
			} else if (op == int_subtract_op) {
				int val = int_value(arg0());
				set_int_value(arg0(), val - control_stack_.top());
				control_stack_.pop();
			} else if (op == int_multiply_op) {
				int val = int_value(arg0());
				set_int_value(arg0(), val * control_stack_.top());
				control_stack_.pop();
			} else
				fail("unexpected operator", line_number());
			++pc;
		}
			break;
		case index_load_value_op: {
			control_stack_.push(index_value(arg0()));
			++pc;
		}
			break;
		case int_add_op: {
			int i1 = control_stack_.top();
			control_stack_.pop();
			int i0 = control_stack_.top();
			control_stack_.pop();
			control_stack_.push(i0 + i1);
			++pc;
		}
			break;
		case int_subtract_op: {
			int i1 = control_stack_.top();
			control_stack_.pop();
			int i0 = control_stack_.top();
			control_stack_.pop();
			control_stack_.push(i0 - i1);
			++pc;
		}
			break;
		case int_multiply_op: {
			int i1 = control_stack_.top();
			control_stack_.pop();
			int i0 = control_stack_.top();
			control_stack_.pop();
			control_stack_.push(i0 * i1);
			++pc;
		}
			break;
		case int_divide_op: {
			int i1 = control_stack_.top();
			control_stack_.pop();
			int i0 = control_stack_.top();
			control_stack_.pop();
			control_stack_.push(i0 / i1);
			++pc;
		}
			break;
		case int_equal_op: {
			int i1 = control_stack_.top();
			control_stack_.pop();
			int i0 = control_stack_.top();
			control_stack_.pop();
			control_stack_.push(i0 == i1);
			++pc;
		}
			break;
		case int_nequal_op: {
			int i1 = control_stack_.top();
			control_stack_.pop();
			int i0 = control_stack_.top();
			control_stack_.pop();
			control_stack_.push(i0 != i1);
			++pc;
		}
			break;
		case int_ge_op: {
			int i1 = control_stack_.top();
			control_stack_.pop();
			int i0 = control_stack_.top();
			control_stack_.pop();
			control_stack_.push(i0 >= i1);
			++pc;
		}
			break;
		case int_le_op: {
			int i1 = control_stack_.top();
			control_stack_.pop();
			int i0 = control_stack_.top();
			control_stack_.pop();
			control_stack_.push(i0 <= i1);
			++pc;
		}
			break;
		case int_gt_op: {
			int i1 = control_stack_.top();
			control_stack_.pop();
			int i0 = control_stack_.top();
			control_stack_.pop();
			control_stack_.push(i0 > i1);
			++pc;
		}
			break;
		case int_lt_op: {
			int i1 = control_stack_.top();
			control_stack_.pop();
			int i0 = control_stack_.top();
			control_stack_.pop();
			control_stack_.push(i0 < i1);
			++pc;
		}
			break;
		case int_neg_op: {
			int i = control_stack_.top();
			control_stack_.pop();
			control_stack_.push(-i);
			++pc;
		}
			break;
		case cast_to_int_op: {
			double e = expression_stack_.top();
			expression_stack_.pop();
			int int_val = sial_math::cast_double_to_int(e);
			control_stack_.push(int_val);
			++pc;
		}
			break;
		case scalar_load_value_op: {
			expression_stack_.push(scalar_value(arg0()));
			++pc;
		}
			break;
		case scalar_store_op: {
			int op = arg1();
			if (op == scalar_store_op) {
				set_scalar_value(arg0(), expression_stack_.top());
				expression_stack_.pop();
			} else if (op == scalar_add_op) {
				double val = scalar_value(arg0());
				set_scalar_value(arg0(), val + expression_stack_.top());
				expression_stack_.pop();
			} else if (op == scalar_subtract_op) {
				double val = scalar_value(arg0());
				set_scalar_value(arg0(), val - expression_stack_.top());
				expression_stack_.pop();
			} else if (op == scalar_multiply_op) {
				double val = scalar_value(arg0());
				set_scalar_value(arg0(), val * expression_stack_.top());
				expression_stack_.pop();
			} else
				fail("unexpected operator");
			++pc;
		}
			break;
		case scalar_add_op: {
			double e1 = expression_stack_.top();
			expression_stack_.pop();
			double e0 = expression_stack_.top();
			expression_stack_.pop();
			expression_stack_.push(e0 + e1);
			++pc;
		}
			break;
		case scalar_subtract_op: {
			double e1 = expression_stack_.top();
			expression_stack_.pop();
			double e0 = expression_stack_.top();
			expression_stack_.pop();
			expression_stack_.push(e0 - e1);
			++pc;
		}
			break;
		case scalar_multiply_op: {
			double e1 = expression_stack_.top();
			expression_stack_.pop();
			double e0 = expression_stack_.top();
			expression_stack_.pop();
			expression_stack_.push(e0 * e1);
			++pc;
		}
			break;
		case scalar_divide_op: {
			double e1 = expression_stack_.top();
			expression_stack_.pop();
			double e0 = expression_stack_.top();
			expression_stack_.pop();
			expression_stack_.push(e0 / e1);
			++pc;
		}
			break;
		case scalar_exp_op: {
			double e1 = expression_stack_.top();
			expression_stack_.pop();
			double e0 = expression_stack_.top();
			expression_stack_.pop();
			double value = sial_math::pow(e0,e1);
			expression_stack_.push(value);
			++pc;
		}
			break;
		case scalar_eq_op: {
			double e1 = expression_stack_.top();
			expression_stack_.pop();
			double e0 = expression_stack_.top();
			expression_stack_.pop();
			int value = (e0 == e1);
			control_stack_.push(value);
			++pc;
		}
			break;
		case scalar_ne_op: {
			double e1 = expression_stack_.top();
			expression_stack_.pop();
			double e0 = expression_stack_.top();
			expression_stack_.pop();
			int value = (e0 != e1);
			control_stack_.push(value);
			++pc;
		}
			break;
		case scalar_ge_op: {
			double e1 = expression_stack_.top();
			expression_stack_.pop();
			double e0 = expression_stack_.top();
			expression_stack_.pop();
			int value = (e0 >= e1);
			control_stack_.push(value);
			++pc;
		}
			break;
		case scalar_le_op: {
			double e1 = expression_stack_.top();
			expression_stack_.pop();
			double e0 = expression_stack_.top();
			expression_stack_.pop();
			int value = (e0 <= e1);
			control_stack_.push(value);
			++pc;
		}
			break;
		case scalar_gt_op: {
			double e1 = expression_stack_.top();
			expression_stack_.pop();
			double e0 = expression_stack_.top();
			expression_stack_.pop();
			int value = (e0 > e1);
			control_stack_.push(value);
			++pc;
		}
			break;
		case scalar_lt_op: {
			double e1 = expression_stack_.top();
			expression_stack_.pop();
			double e0 = expression_stack_.top();
			expression_stack_.pop();
			int value = (e0 < e1);
			control_stack_.push(value);
			++pc;
		}
			break;
		case scalar_neg_op: {
			double e = expression_stack_.top();
			expression_stack_.pop();
			expression_stack_.push(-e);
			++pc;
		}
			break;
		case scalar_sqrt_op: {
			double e = expression_stack_.top();
			expression_stack_.pop();
			double value = sial_math::sqrt(e);
			expression_stack_.push(value);
			++pc;
		}
			break;
		case cast_to_scalar_op: {
			int i = control_stack_.top();
			control_stack_.pop();
			expression_stack_.push(double(i));
			++pc;
		}
			break;
		case collective_sum_op: {
			double rhs_value = expression_stack_.top();
			expression_stack_.pop();
			sial_ops_.collective_sum(rhs_value, arg0());
			++pc;
		}
			break;
		case assert_same_op: {
			sial_ops_.assert_same(arg0());
			++pc;
		}
			break;
//		case tensor_op: {
//			//TODO
//			++pc;
//		}
//			break;
		case block_copy_op: {
			//get rhs selector from selector stack
			sip::Block::BlockPtr rhs_block = get_block_from_selector_stack('r', true);
			sip::Block::BlockPtr lhs_block = get_block_from_instruction('w', true);
			//#ifdef HAVE_CUDA
			//		if (gpu_enabled_) {
			//			rhs_block = get_gpu_block('r', rhs_selector, rhs_blockid);
			//			lhs_block = get_gpu_block('w', lhs_selector, lhs_blockid);
			//		} else
			//#endif
			//check for self assignment
			if (lhs_block->get_data() == rhs_block->get_data()) {
						return;
			}
			lhs_block->copy_data_(rhs_block);
			//#ifdef HAVE_CUDA
			//				if (gpu_enabled_) {
			//					lhs_block->gpu_copy_data(rhs_block);
			//					return;
			//				}
			//#endif
			++pc;
		}
		break;

		case block_permute_op: {
			//get lhs block from selector stack.  This order is correct--rhs has been pushed first for good reason in the compiler
			BlockSelector lhs_selector = block_selector_stack_.top();
			sip::Block::BlockPtr lhs_block = get_block_from_selector_stack('w', true);
			//get rhs blcok from selector stack
			BlockSelector rhs_selector = block_selector_stack_.top();
			sip::Block::BlockPtr rhs_block = get_block_from_selector_stack('r', true);  //TODO, check contiguous allowed.
			//permutation vector is in the instructions' index_selector array.

			//Calculate permutation and compare with compiler's.
			//when confident about the compiler, can remove this code.
				int lhs_rank = lhs_selector.rank_;
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
				for (int i = lhs_rank; i < MAX_RANK; ++i) {
					//fill in unused dims with -1 to cause failure if accessed
					permutation[i] = -1;
				}
			//compare caluculated here and compiler generated
				bool OK = true;
			for(int i = 0; i < MAX_RANK && OK; ++i){OK &= (permutation[i] == index_selectors()[i]);}
				check(OK, "permutation vector from compiler differs from sip's", line_number());

				//do the transpose
				lhs_block->transpose_copy(rhs_block, lhs_rank, permutation);

			//#ifdef HAVE_CUDA
			//				if (gpu_enabled_) {
			//					_gpu_permute(lhs_block->get_gpu_data(), lhs_rank, lhs_block->shape().segment_sizes_, lhs_selector.index_ids_,
			//							rhs_block->get_gpu_data(), lhs_rank, rhs_block->shape().segment_sizes_ , rhs_selector.index_ids_);
			//					return;
			//				}
			//#endif
			++pc;
		}
			break;
		case block_fill_op: {
			sip::Block::BlockPtr lhs_block = get_block_from_instruction('w',
					true);
			double rhs = expression_stack_.top();
			lhs_block->fill(rhs);
			expression_stack_.pop();
					//#ifdef HAVE_CUDA
					//			if (gpu_enabled_) {   //FIXME.  This looks OK, but need to double check
					//				sip::Block::BlockPtr g_lhs_block = get_gpu_block('w');
					//				g_lhs_block->gpu_fill(scalar_value(rhs));
					//				return;
					//			}
                   //#endif
			++pc;
		}
			break;
		case scale_block_op: {
			sip::Block::BlockPtr lhs_block = get_block_from_instruction('u',
					true);
			lhs_block->scale(expression_stack_.top());
			expression_stack_.pop();
			++pc;
		}
			break;
		case accumulate_scalar_into_block_op: {
			sip::Block::BlockPtr lhs_block = get_block_from_instruction('u',
					true);
			lhs_block->increment_elements(expression_stack_.top());
			expression_stack_.pop();
			++pc;
		}
			break;
		case block_add_op: {
			handle_block_add(pc);
			++pc;
		}
			break;
		case block_subtract_op: {
			handle_block_subtract(pc);
			++pc;
		}
			break;
		case block_contract_op: {
			int drank = arg0();
			index_selector_t& selectors = index_selectors();
			Block::BlockPtr dblock = get_block_from_instruction('w',true);
			handle_contraction(drank, selectors, dblock->get_data(),const_cast<segment_size_array_t&>(dblock->shape().segment_sizes_) );
			++pc;
		}
			break;
//		case block_contract_accumulate_op: {
//			//TODO
//			++pc;
//		}
//			break;
		case block_contract_to_scalar_op: {
			Block::BlockPtr dblock = data_manager_.scalar_blocks_[arg1()];
			double result;
			segment_size_array_t dummy_dsegment_sizes;
			handle_contraction(0, index_selectors(), &result, dummy_dsegment_sizes);
			expression_stack_.push(result);
			++pc;
		}
			break;
		case block_load_scalar_op: {
			//This instruction pushes the value of a block with all simple indices onto the instruction stack.
			//If this is a frequent occurrance, we should introduce a new block type for this
			sip::Block::BlockPtr block = get_block_from_selector_stack('r',true);
			expression_stack_.push(block->get_data()[0]);
			++pc;
		}
			break;
//		case slice_op: {
//			//TODO
//			++pc;
//		}
//			break;
//		case insert_op: {
//			//TODO
//			++pc;
//		}
//			break;

		case print_string_op: {
			printer_->print_string(string_literal(control_stack_.top()));
			control_stack_.pop();
			if (arg0())
				printer_->endl();
			++pc;
		}
			break;
		case print_scalar_op: {
			int slot = arg1();
			if (slot >= 0)
				printer_->print_scalar(array_name(slot),
						expression_stack_.top(), line_number());
			else
				printer_->print_scalar_value(expression_stack_.top());
			expression_stack_.pop();
			if (arg0())
				printer_->endl();
			++pc;
		}
			break;
		case print_int_op: {
			int slot = arg1();
			if (slot >= 0)
				printer_->print_int(int_name(slot), control_stack_.top(),
						line_number());
			else
				printer_->print_int_value(control_stack_.top());
			control_stack_.pop();
			if (arg0())
				printer_->endl();
			++pc;
		}
			break;
		case print_index_op: {
			int slot = arg1();
			if (slot >= 0)
				printer_->print_index(index_name(slot), control_stack_.top(),
						line_number());
			else
				printer_->print_int_value(control_stack_.top());
			control_stack_.pop();
			if (arg0())
				printer_->endl();
			++pc;
		}
			break;
		case print_block_op: {
			BlockId id;
			int rank = block_selector_stack_.top().rank_;
			int array_slot = block_selector_stack_.top().array_id_;
			Block::BlockPtr block = get_block_from_selector_stack('r',id, true);
			if (rank == 0){
				printer_->print_contiguous(array_slot, block, line_number());
			} else {
			printer_->print_block( id,  block, line_number());
			}
			if (arg0())
				printer_->endl();
			++pc;
		}
		break;
		case gpu_on_op:{
			++pc;
		}
		break;
		case gpu_off_op:{
			++pc;
		}
		break;
		/* FIX ME:  other gpu instruction omitted for now */
		case set_persistent_op: {
			int array_slot = arg1();
			int string_slot = arg0();;
			sial_ops_.set_persistent(this, array_slot, string_slot);
			++pc;
		}
			break;
		case restore_persistent_op: {
			int array_slot = arg1();
			int string_slot = arg0();
			sial_ops_.restore_persistent(this, array_slot, string_slot);
			++pc;
		}
			break;
		case idup_op: {
			control_stack_.push(control_stack_.top());
			++pc;
		}
			break;
		case iswap_op: {
			int i1 = control_stack_.top();
			control_stack_.pop();
			int i0 = control_stack_.top();
			control_stack_.pop();
			control_stack_.push(i1);
			control_stack_.push(i0);
			++pc;
		}
			break;
		case sswap_op: {
			double e1 = expression_stack_.top();
			expression_stack_.pop();
			double e0 = expression_stack_.top();
			expression_stack_.pop();
			expression_stack_.push(e1);
			expression_stack_.push(e0);
			++pc;
		}
			break;
		default: {
			check(false, opcodeToName(opcode) + " not yet implemented ");
		}

		}// switch

		//TODO  only call where necessary
		contiguous_blocks_post_op();
	}// while

} //interpret

void Interpreter::post_sial_program() {
	sial_ops_.end_program();
}

void Interpreter::handle_user_sub_op(int pc) {
	int num_args = arg1();
	int func_slot = arg0();
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
	int block0_size = block0->size();
	segment_size_array_t& seg_sizes0 =
			const_cast<segment_size_array_t&>(block0->shape().segment_sizes_);
	Block::dataPtr data0 = block0->get_data();
	if (num_args == 1) {
		SpecialInstructionManager::fp1 func =
				sip_tables_.special_instruction_manager_.get_one_arg_special_instruction_ptr(
						func_slot);
		//	typedef void(*fp1)(int& array_slot, int& rank, int * index_values, int& size, int * extents, double * block_data, int& ierr
		func(array_id0, rank0, block_id0.index_values_, block0_size, seg_sizes0,
				data0, ierr);
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
	int block1_size = block1->size();
	segment_size_array_t& seg_sizes1 =
			const_cast<segment_size_array_t&>(block1->shape().segment_sizes_);
	Block::dataPtr data1 = block1->get_data();
	if (num_args == 2) {
		SpecialInstructionManager::fp2 func =
				sip_tables_.special_instruction_manager_.get_two_arg_special_instruction_ptr(
						func_slot);

		func(array_id0, rank0, block_id0.index_values_, block0_size, seg_sizes0,
				data0, array_id1, rank1, block_id1.index_values_, block1_size,
				seg_sizes1, data1, ierr);
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
	int block2_size = block2->size();
	segment_size_array_t& seg_sizes2 =
			const_cast<segment_size_array_t&>(block2->shape().segment_sizes_);
	Block::dataPtr data2 = block2->get_data();
	if (num_args == 3) {
		SpecialInstructionManager::fp3 func =
				sip_tables_.special_instruction_manager_.get_three_arg_special_instruction_ptr(
						func_slot);

		func(array_id0, rank0, block_id0.index_values_, block0_size, seg_sizes0,
				data0, array_id1, rank1, block_id1.index_values_, block1_size,
				seg_sizes1, data1, array_id2, rank2, block_id2.index_values_,
				block2_size, seg_sizes2, data2, ierr);
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
	int block3_size = block3->size();
	segment_size_array_t& seg_sizes3 =
			const_cast<segment_size_array_t&>(block3->shape().segment_sizes_);
	Block::dataPtr data3 = block3->get_data();
	if (num_args == 4) {
		SpecialInstructionManager::fp4 func =
				sip_tables_.special_instruction_manager_.get_four_arg_special_instruction_ptr(
						func_slot);
		func(array_id0, rank0, block_id0.index_values_, block0_size, seg_sizes0,
				data0, array_id1, rank1, block_id1.index_values_, block1_size,
				seg_sizes1, data1, array_id2, rank2, block_id2.index_values_,
				block2_size, seg_sizes2, data2, array_id3, rank3,
				block_id3.index_values_, block3_size, seg_sizes3, data3, ierr);
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
	int block4_size = block4->size();
	segment_size_array_t& seg_sizes4 =
			const_cast<segment_size_array_t&>(block4->shape().segment_sizes_);
	Block::dataPtr data4 = block4->get_data();
	if (num_args == 5) {
		SpecialInstructionManager::fp5 func =
				sip_tables_.special_instruction_manager_.get_five_arg_special_instruction_ptr(
						func_slot);
		func(array_id0, rank0, block_id0.index_values_, block0_size, seg_sizes0,
				data0, array_id1, rank1, block_id1.index_values_, block1_size,
				seg_sizes1, data1, array_id2, rank2, block_id2.index_values_,
				block2_size, seg_sizes2, data2, array_id3, rank3,
				block_id3.index_values_, block3_size, seg_sizes3, data3,
				array_id4, rank4, block_id4.index_values_, block4_size,
				seg_sizes4, data4, ierr);
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
	int block5_size = block5->size();
	segment_size_array_t& seg_sizes5 =
			const_cast<segment_size_array_t&>(block5->shape().segment_sizes_);
	Block::dataPtr data5 = block5->get_data();
	if (num_args == 6) {
		SpecialInstructionManager::fp6 func =
				sip_tables_.special_instruction_manager_.get_six_arg_special_instruction_ptr(
						func_slot);
		func(array_id0, rank0, block_id0.index_values_, block0_size, seg_sizes0,
				data0, array_id1, rank1, block_id1.index_values_, block1_size,
				seg_sizes1, data1, array_id2, rank2, block_id2.index_values_,
				block2_size, seg_sizes2, data2, array_id3, rank3,
				block_id3.index_values_, block3_size, seg_sizes3, data3,
				array_id4, rank4, block_id4.index_values_, block4_size,
				seg_sizes4, data4, array_id5, rank5, block_id5.index_values_,
				block5_size, seg_sizes5, data5, ierr);
		sip::check(ierr == 0,
				"error returned from special super instruction"
						+ sip_tables_.special_instruction_manager_.name(
								func_slot));
		return;
	}

	sip::check(false,
			"Implementation restriction:  At most 6 arguments to a super instruction supported.  This can be increased if necessary");
}

void Interpreter::handle_contraction(int drank, index_selector_t& dselected_index_ids, double *ddata, segment_size_array_t& dshape) {
	//using dmitry's notation, i.e. d = l *
//old CUDA STUFF MOVED TO BOTTOM OF FILE

	//get right operand info
	sip::BlockSelector rselector = block_selector_stack_.top();
	sip::BlockId rid;
	sip::Block::BlockPtr rblock = get_block_from_selector_stack('r', rid, true);
	int rrank = sip_tables_.array_rank(rid);

	//get left operand info
	sip::BlockSelector lselector = block_selector_stack_.top();
	sip::BlockId lid;
	sip::Block::BlockPtr lblock = get_block_from_selector_stack('r', lid, true);
	int lrank  = sip_tables_.array_rank(lid);

	//get contraction pattern
	int pattern_size = drank + lrank + rrank;
	std::vector<int> aces_pattern(pattern_size, 0);	// Initialize vector with pattern_size elements of value 0
	std::vector<int>::iterator it;
	it = aces_pattern.begin();

	it = std::copy(dselected_index_ids + 0, dselected_index_ids + drank, it);
	it = std::copy(lselector.index_ids_ + 0, lselector.index_ids_ + lrank, it);
	it = std::copy(rselector.index_ids_ + 0, rselector.index_ids_ + rrank, it);

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
	tensor_block_contract__(nthreads, contraction_pattern,
			lblock->get_data(), lrank,
			const_cast<segment_size_array_t&>(lblock->shape().segment_sizes_),
			rblock->get_data(), rrank,
			const_cast<segment_size_array_t&>(rblock->shape().segment_sizes_),
			ddata, drank, dshape,
			ierr);
}


//
//
//void Interpreter::handle_contraction_op(int pc) {
//	//using dmitry's notation, i.e. d = l * r
//	//determine rank of arguments
////	int darray = op_table_.arg2(pc);
////	int drank = sip_tables_.array_rank(darray);
////	int larray = op_table_.arg0(pc);
////	int lrank = sip_tables_.array_rank(larray);
////	int rarray = op_table_.arg1(pc);
////	int rrank = sip_tables_.array_rank(rarray);
//
//
//	//get right operand info
//	sip::BlockSelector rselector = block_selector_stack_.top();
//	int rarray = rselector.array_id_;
//	int rrank = rselector.rank_
//	sip::BlockId rid;
//	sip::Block::BlockPtr rblock = get_block_from_selector_stack('r', rid);
//
//
//	//get left operand info
//	sip::BlockSelector lselector = block_selector_stack_.top();
//	int larray = lselector.array_id_;
//	int lrank = lselector.rank_;
//	sip::BlockId lid;
//	sip::Block::BlockPtr lblock = get_block_from_selector_stack('r', lid);
//
//	//get destination infor
//	sip::BlockSelector dselector(arg0(), arg1(), index_selectors());
//	int darray = arg1();
//	int drank = arg0();
//	sip::BlockId did;
//	sip::Block::BlockPtr dblock = get_block_from_instruction('r', did);
//
//
//
//
//	BlockId did;
//	Block::BlockPtr dblock = get_block_from_instruction('w', did, true);
//	int drank = sip_tables_array_rank(did.array_id());
//
//
//	BlockId rid;
//	Block::BlockPtr rblock = get_block_from_selector_stack('r', rid, true);
//	int rrank = sip_tables_array_rank(rid.array_id());
//
//	Block lid;
//	Block::BlockPtr lblock = get_block_from_selector_stackk('r', lid, true);
//	int lrank = = sip_tables_array_rank(lid.array_id());
//
//	//get contraction pattern
//	int pattern_size = drank + lrank + rrank;
//	std::vector<int> aces_pattern(pattern_size, 0);	// Initialize vector with pattern_size elements of value 0
//	std::vector<int>::iterator it;
//	it = aces_pattern.begin();
//
//
//	it = std::copy(dselected_index_ids + 0, dselected_index_ids + drank, it);
//	it = std::copy(lselector.index_ids_ + 0, lselector.index_ids_ + lrank, it);
//	it = std::copy(rselector.index_ids_ + 0, rselector.index_ids_ + rrank, it);
//
//	//	int contraction_pattern[lrank + rrank];
//	int contraction_pattern[MAX_RANK * 2];
//	int ierr;
//
//	get_contraction_ptrn_(drank, lrank, rrank, aces_pattern.data(),
//			contraction_pattern, ierr);
//	check(ierr == 0, std::string("error returned from get_contraction_ptrn_"),
//			line_number());
//	//    INPUT:
//	//    ! - nthreads - number of threads requested;
//	//    ! - contr_ptrn(1:left_rank+right_rank) - contraction pattern;
//	//    ! - ltens/ltens_num_dim/ltens_dim_extent - left tensor block / its rank / and dimension extents;
//	//    ! - rtens/rtens_num_dim/rtens_dim_extent - right tensor block / its rank / and dimension extents;
//	//    ! - dtens/dtens_num_dim/dtens_dim_extent - destination tensor block (initialized!) / its rank / and dimension extents;
//	//    !OUTPUT:
//	//    ! - dtens - modified destination tensor block;
//	//    ! - ierr - error code (0: success);
//	//     */
//	int nthreads = sip::MAX_OMP_THREADS;
//	tensor_block_contract__(nthreads, contraction_pattern, lblock->get_data(),
//			lrank,
//			const_cast<segment_size_array_t&>(lblock->shape().segment_sizes_),
//			rblock->get_data(), rrank,
//			const_cast<segment_size_array_t&>(rblock->shape().segment_sizes_),
//			dblock->get_data(), drank,
//			const_cast<segment_size_array_t&>(dblock->shape().segment_sizes_),
//			ierr);
//	//std::cout <<"scalar:" << dblock.get_data()[0] << std::endl;
//	//	if (d_is_scalar){
//	//		set_scalar_value(op_table_.result_array(pc), *(dblock->get_data()));
//	//		delete dblock;
//	//	}
//
//	//}
//
////	sip::check(rrank == sip_tables_.array_rank(rselector.array_id_),
////			"SIP or compiler bug, inconsistent for r arg in contract op");
////	sip::BlockId rid;
////	sip::Block::BlockPtr rblock = get_block_from_selector_stack('r', rid);
////	sip::check(rarray == rid.array_id(),
////			"SIP or compiler bug:  inconsistent array ids in contract op");
//
//
//
//
////#ifdef HAVE_CUDA
//	if (gpu_enabled_) {
//		std::cout<<"Contraction on the GPU at line "<<line_number()<<std::endl;
//
//		sip::BlockSelector rselector = block_selector_stack_.top();
//		sip::Block::BlockPtr g_rblock = get_gpu_block('r');
//
//		sip::BlockSelector lselector = block_selector_stack_.top();
//		sip::Block::BlockPtr g_lblock = get_gpu_block('r');
//
//		sip::Block::BlockPtr g_dblock;
//		sip::index_selector_t g_dselected_index_ids;
//		double *g_d;
//		if (d_is_scalar) {
//			double h_d = 0.0;
//			sip::BlockShape scalar_shape;
//			g_dblock = new sip::Block(scalar_shape);
//			//g_dblock->gpu_data_ = _gpu_allocate(1);
//			g_dblock->allocate_gpu_data();
//			std::fill(g_dselected_index_ids + 0, g_dselected_index_ids + MAX_RANK, sip::unused_index_slot);
//		} else {
//			sip::BlockSelector dselector = block_selector_stack_.top();
//			std::copy(dselector.index_ids_ + 0, dselector.index_ids_ + MAX_RANK, g_dselected_index_ids + 0);
//			g_dblock = get_gpu_block('w');
//		}
//
//		_gpu_contract(g_dblock->get_gpu_data(), drank, g_dblock->shape().segment_sizes_, g_dselected_index_ids,
//				g_lblock->get_gpu_data(), lrank, g_lblock->shape().segment_sizes_, lselector.index_ids_,
//				g_rblock->get_gpu_data(), rrank, g_rblock->shape().segment_sizes_, rselector.index_ids_);
//
//		if (d_is_scalar) {
//			double h_dbl;
//			_gpu_device_to_host(&h_dbl, g_dblock->get_gpu_data(), 1);
//			set_scalar_value(op_table_.arg2(pc), h_dbl);
//		}
//
//		return;
//	}
//#endif
//	//std::cout<<"Contraction at line "<<line_number()<<std::endl;
//
//	//get right operand info
//	sip::BlockSelector rselector = block_selector_stack_.top();
//	sip::check(rrank == sip_tables_.array_rank(rselector.array_id_),
//			"SIP or compiler bug, inconsistent for r arg in contract op");
//	sip::BlockId rid;
//	sip::Block::BlockPtr rblock = get_block_from_selector_stack('r', rid);
//	sip::check(rarray == rid.array_id(),
//			"SIP or compiler bug:  inconsistent array ids in contract op");
//
//	//get left operand info
//	sip::BlockSelector lselector = block_selector_stack_.top();
//	sip::check(lrank == sip_tables_.array_rank(lselector.array_id_),
//			"SIP or compiler bug, inconsistent for l arg in contract op");
//	sip::BlockId lid;
//	sip::Block::BlockPtr lblock = get_block_from_selector_stack('r', lid);
//
//	//If result of contraction is scalar.
//	sip::Block::BlockPtr dblock = NULL;
//	sip::index_selector_t dselected_index_ids;
//	if (d_is_scalar) {
//		//		double *dvalPtr = data_manager_.scalar_address(op_table_.result_array(pc));
//		//		*dvalPtr = 0; // Initialize to 0 as required by Dmitry's Contraction routine
//		//		sip::BlockShape scalar_shape;
//		//		dblock = new sip::Block(scalar_shape, dvalPtr);
//		//		std::fill(dselected_index_ids + 0, dselected_index_ids + MAX_RANK,
//		//				sip::unused_index_slot);
//		dblock = data_manager_.scalar_blocks_[op_table_.arg2(pc)];
//	} else {
//		sip::BlockSelector dselector = block_selector_stack_.top();
//		std::copy(dselector.index_ids_ + 0, dselector.index_ids_ + MAX_RANK,
//				dselected_index_ids + 0);
//		sip::check(drank == sip_tables_.array_rank(dselector.array_id_),
//				"SIP or compiler bug, inconsistent for d arg in contract op");
//		sip::BlockId did;
//		dblock = get_block_from_selector_stack('w', did);
//
//	}
//
//	//get contraction pattern
//	int pattern_size = drank + lrank + rrank;
//	std::vector<int> aces_pattern(pattern_size, 0);	// Initialize vector with pattern_size elements of value 0
//	std::vector<int>::iterator it;
//	it = aces_pattern.begin();
//
//	it = std::copy(dselected_index_ids + 0, dselected_index_ids + drank, it);
//	it = std::copy(lselector.index_ids_ + 0, lselector.index_ids_ + lrank, it);
//	it = std::copy(rselector.index_ids_ + 0, rselector.index_ids_ + rrank, it);
//
//	//	int contraction_pattern[lrank + rrank];
//	int contraction_pattern[MAX_RANK * 2];
//	int ierr;
//
//	get_contraction_ptrn_(drank, lrank, rrank, aces_pattern.data(),
//			contraction_pattern, ierr);
//	check(ierr == 0, std::string("error returned from get_contraction_ptrn_"),
//			line_number());
//	//    INPUT:
//	//    ! - nthreads - number of threads requested;
//	//    ! - contr_ptrn(1:left_rank+right_rank) - contraction pattern;
//	//    ! - ltens/ltens_num_dim/ltens_dim_extent - left tensor block / its rank / and dimension extents;
//	//    ! - rtens/rtens_num_dim/rtens_dim_extent - right tensor block / its rank / and dimension extents;
//	//    ! - dtens/dtens_num_dim/dtens_dim_extent - destination tensor block (initialized!) / its rank / and dimension extents;
//	//    !OUTPUT:
//	//    ! - dtens - modified destination tensor block;
//	//    ! - ierr - error code (0: success);
//	//     */
//	int nthreads = sip::MAX_OMP_THREADS;
//	tensor_block_contract__(nthreads, contraction_pattern, lblock->get_data(),
//			lrank,
//			const_cast<segment_size_array_t&>(lblock->shape().segment_sizes_),
//			rblock->get_data(), rrank,
//			const_cast<segment_size_array_t&>(rblock->shape().segment_sizes_),
//			dblock->get_data(), drank,
//			const_cast<segment_size_array_t&>(dblock->shape().segment_sizes_),
//			ierr);
//	//std::cout <<"scalar:" << dblock.get_data()[0] << std::endl;
//	//	if (d_is_scalar){
//	//		set_scalar_value(op_table_.result_array(pc), *(dblock->get_data()));
//	//		delete dblock;
//	//	}
//
//}






void Interpreter::loop_start(LoopManager * loop) {
	int enddo_pc = arg0();
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

Block::BlockPtr Interpreter::get_block_from_instruction(char intent,
		bool contiguous_allowed) {
	BlockSelector selector(arg0(), arg1(), index_selectors());
	BlockId block_id;
	Block::BlockPtr block = get_block(intent, selector, block_id, contiguous_allowed);
	return block;
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
	return get_block(intent, selector, id, contiguous_allowed);
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
						id, read_block_list_) :
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
		bool is_scope_extent = sip_tables_.is_scope_extent(selector.array_id_);
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

void Interpreter::handle_block_add(int pc) {
	//d = l + r
	Block::BlockPtr rblock = get_block_from_selector_stack('r',  true);
	double *rdata  = rblock->get_data();
	Block::BlockPtr lblock = get_block_from_selector_stack('r',  true);
	double *ldata = lblock->get_data();
	Block::BlockPtr dblock = get_block_from_instruction('w', true);
	double *ddata = dblock->get_data();
	size_t size = dblock->size();
	for(size_t i = 0; i != size; ++i){
		*(ddata++) = *(ldata++)  + *(rdata++);
	}
}

void Interpreter::handle_block_subtract(int pc){
	Block::BlockPtr rblock = get_block_from_selector_stack('r',  true);
	double *rdata  = rblock->get_data();
	Block::BlockPtr lblock = get_block_from_selector_stack('r',  true);
	double *ldata = lblock->get_data();
	Block::BlockPtr dblock = get_block_from_instruction('w', true);
	double *ddata = dblock->get_data();
	size_t size = dblock->size();
	for(size_t i = 0; i != size; ++i){
		*(ddata++) = *(ldata++)  - *(rdata++);
	}
}

void Interpreter::contiguous_blocks_post_op() {

	// Write back all contiguous slices
	//	while (!write_back_list_.empty()) {
	//		sip::WriteBack * ptr = write_back_list_.front();
	//#ifdef HAVE_CUDA
	//		if (gpu_enabled_) {
	//			sip::Block::BlockPtr cblock = ptr->get_block();
	//			data_manager_.block_manager_.lazy_gpu_read_on_host(cblock);
	//		}
	//#endif
	//		ptr->do_write_back();
	//		write_back_list_.erase(write_back_list_.begin());
	//		delete ptr;
	//	}
	//
	//	// Free up contiguous slices only needed for read.
	//	while (!read_block_list_.empty()){
	//		// TODO FIXME GPU ?????????????????
	//		Block::BlockPtr bptr = read_block_list_.front();
	//		read_block_list_.erase(read_block_list_.begin());
	//		delete bptr;
	//	}

	for (WriteBackList::iterator it = write_back_list_.begin();
			it != write_back_list_.end(); ++it) {
		WriteBack* wb = *it;
#ifdef HAVE_CUDA
		if (gpu_enabled_) {
			// Read from GPU back into host to do write back.
			sip::Block::BlockPtr cblock = (*it)->get_block();
			data_manager_.block_manager_.lazy_gpu_read_on_host(cblock);
		}
#endif
		wb->do_write_back();
		delete wb;
		*it = NULL;
	}
	write_back_list_.clear();

	for (ReadBlockList::iterator it = read_block_list_.begin();
			it != read_block_list_.end(); ++it) {
		Block::BlockPtr bptr = *it;
		delete bptr;
		*it = NULL;
	}
	read_block_list_.clear();

}


}/* namespace sip */





//void Interpreter::handle_slice_op(int pc) {
//	int lhs = op_table_.arg2(pc); //slot in array table for lhs
//	int rank = sip_tables_.array_table_.rank(lhs);
//	bool is_scope_extent = sip_tables_.is_scope_extent(lhs);
//
//	//get rhs block, offsets, and extents
//	//this is a special case, because we need to get the superblock
//	int rhs = op_table_.arg0(pc);    //slot in array table for rhs
//	sip::BlockSelector rhs_selector = block_selector_stack_.top();
//	block_selector_stack_.pop();
//	sip::check(data_manager_.is_subblock(rhs_selector), //at least one index is subindex
//	"Compiler or sip bug:  rhs should be subblock");
//	sip::BlockId rhs_super_block_id = data_manager_.super_block_id(
//			rhs_selector);
//	sip::Block::BlockPtr super_block = sial_ops_.get_block_for_reading(
//			rhs_super_block_id);  //for this
//	sip::offset_array_t offsets;
//	sip::BlockShape subblock_shape;
//	data_manager_.get_subblock_offsets_and_shape(super_block, rhs_selector,
//			offsets, subblock_shape);
//
//	//get lhs block
//	sip::Block::BlockPtr lhs_block = get_block_from_selector_stack('w', false);
//	//calls Dmitry's routine (via the Block method)
//	super_block->extract_slice(rank, offsets, lhs_block);
//}
//
//void Interpreter::handle_insert_op(int pc) {
//	int lhs = op_table_.arg2(pc); //slot in array table for lhs
//	int rank = sip_tables_.array_table_.rank(lhs);
//	bool is_scope_extent = sip_tables_.is_scope_extent(lhs);
//
//	//get rhs block
//	sip::Block::BlockPtr rhs_block = get_block_from_selector_stack('r', false);
//
//	//get lhs block.  This requires getting the superblock
//	sip::BlockSelector lhs_selector = block_selector_stack_.top();
//	block_selector_stack_.pop();
//	sip::BlockId lhs_super_block_id = data_manager_.super_block_id(
//			lhs_selector);
//	sip::Block::BlockPtr super_block =
//			data_manager_.block_manager_.get_block_for_writing(
//					lhs_super_block_id, is_scope_extent);
//	sip::offset_array_t offsets;
//	sip::BlockShape subblock_shape;
//	data_manager_.get_subblock_offsets_and_shape(super_block, lhs_selector,
//			offsets, subblock_shape);
//
//	super_block->insert_slice(rank, offsets, rhs_block);
//}

//void Interpreter::handle_assignment_op(int pc) {
//	int lhs = op_table_.arg2(pc); //slot in array table for lhs
//	int rhs = op_table_.arg0(pc);    //slot in array table for rhs
//	if (is_scalar(lhs) && is_scalar(rhs)) { //both scalars
//		set_scalar_value(lhs, scalar_value(rhs));
//		return;
//	}
//	if (is_scalar(lhs)) { //rhs is a data block, indices should be simple
//		sip::BlockSelector rhs_selector = block_selector_stack_.top();
//		sip::Block::BlockPtr rhs_block = get_block_from_selector_stack('r');
//		double value = rhs_block->get_data()[0];
//		set_scalar_value(lhs, value);
//		return;
//	}
//	bool is_scope_extent = sip_tables_.is_scope_extent(lhs);
//	if (is_scalar(rhs)) { //fill block with scalar value
//#ifdef HAVE_CUDA
//			if (gpu_enabled_) {
//				sip::Block::BlockPtr g_lhs_block = get_gpu_block('w');
//				g_lhs_block->gpu_fill(scalar_value(rhs));
//				return;
//			}
//#endif
//		sip::Block::BlockPtr lhs_block = get_block_from_selector_stack('w');
//		lhs_block->fill(scalar_value(rhs));
//		return;
//	} else { //both sides are arrays, get blocks, save selectors
//
//		int lhs_rank = sip_tables_.array_rank(lhs);
//		int rhs_rank = sip_tables_.array_rank(rhs);
//
//		sip::BlockSelector rhs_selector = block_selector_stack_.top();
//		block_selector_stack_.pop();
//		sip::BlockSelector lhs_selector = block_selector_stack_.top();
//		block_selector_stack_.pop();
//
//		sip::Block::BlockPtr rhs_block = NULL;
//		sip::Block::BlockPtr lhs_block = NULL;
//
//		sip::BlockId rhs_blockid;
//		sip::BlockId lhs_blockid;
//
//#ifdef HAVE_CUDA
//		if (gpu_enabled_) {
//			rhs_block = get_gpu_block('r', rhs_selector, rhs_blockid);
//			lhs_block = get_gpu_block('w', lhs_selector, lhs_blockid);
//		} else
//#endif
//		{
//			rhs_block = get_block('r', rhs_selector, rhs_blockid);
//			lhs_block = get_block('w', lhs_selector, lhs_blockid);
//		}
//
//		//check for self assignment
//		if (lhs_block->get_data() == rhs_block->get_data()) {
//			return;
//		}
//
//		if (lhs_rank <= rhs_rank) {
//			//Do indices match?  If so, this is an ordinary (possibly dimension reducing) copy
//			if (std::equal(lhs_selector.index_ids_ + 0,
//					lhs_selector.index_ids_ + lhs_rank,
//					rhs_selector.index_ids_ + 0)) {
//
//#ifdef HAVE_CUDA
//				if (gpu_enabled_) {
//					lhs_block->gpu_copy_data(rhs_block);
//					return;
//				}
//#endif
//				//indices match, this is an ordinary (possibly dimension reducing) copy
//				lhs_block->copy_data_(rhs_block);
//				return;
//			}
//			if (lhs_rank == rhs_rank) {
//				//this is a transpose.  calculate permutation
//				int permutation[MAX_RANK];
//				std::fill(permutation + 0, permutation + MAX_RANK, 0);
//
//				for (int i = 0; i < lhs_rank; ++i) {
//					int lhs_index = lhs_selector.index_ids_[i];
//					int j;
//					for (j = 0;
//							j < MAX_RANK
//									&& rhs_selector.index_ids_[j] != lhs_index;
//							++j) {/* keep looking until matching index found */
//					}
//					sip::check(j < lhs_rank, "illegal transpose");
//					permutation[j] = i;
//				}
//				for (int i = lhs_rank; i < MAX_RANK; ++i) {
//					//fill in unused dims with -1 to cause failure if accessed
//					permutation[i] = -1;
//				}
//				//DEBUG
//				//VFLstd::cout << "permutation matrix: ";
//				//VFLfor (int i = 0; i < MAX_RANK; i++)
//				//VFL	std::cout << permutation[i] << " ";
//				//VFLstd::cout << std::endl;
//#ifdef HAVE_CUDA
//				if (gpu_enabled_) {
//					_gpu_permute(lhs_block->get_gpu_data(), lhs_rank, lhs_block->shape().segment_sizes_, lhs_selector.index_ids_,
//							rhs_block->get_gpu_data(), lhs_rank, rhs_block->shape().segment_sizes_ , rhs_selector.index_ids_);
//					return;
//				}
//#endif
//				lhs_block->transpose_copy(rhs_block, lhs_rank, permutation);
//				return;
//			}
//
//			check(false, "illegal indices in assignment", line_number());
//		} else { //lhs_rank > rhs_rank, the compiler should have checked that extra indices are simple
//			lhs_block->copy_data_(rhs_block);
//			return;
//		}
//	}
//	check(false, "illegal indices in assignment");
//}

//void Interpreter::handle_contraction_op(int pc) {
//	//using dmitry's notation, i.e. d = l * r
//	//determine rank of arguments
//	int darray = op_table_.arg2(pc);
//	int drank = sip_tables_.array_rank(darray);
//	int larray = op_table_.arg0(pc);
//	int lrank = sip_tables_.array_rank(larray);
//	int rarray = op_table_.arg1(pc);
//	int rrank = sip_tables_.array_rank(rarray);
//
//	if (lrank == 0 && rrank == 0) {	//rhs is scalar * scalar
//		set_scalar_value(darray, scalar_value(larray) * scalar_value(rarray));
//		return;
//	}
//
//	if (lrank == 0) {  //rhs is scalar * block
//		double lvalue = scalar_value(larray);
//		sip::BlockId rid;
//		sip::Block::BlockPtr rblock = get_block_from_selector_stack('r', rid);
//		sip::BlockId did;
//		sip::Block::BlockPtr dblock = get_block_from_selector_stack('w', did);
//		double * ddata = dblock->get_data();
//		double * rdata = rblock->get_data();
//		for (int i = 0; i < dblock->size(); ++i) {
//			ddata[i] = lvalue * rdata[i];
//		}
//		return;
//	}
//	if (rrank == 0) {  //rhs is block * scalar
//		double rvalue = scalar_value(rarray);
//		sip::BlockId lid;
//		sip::Block::BlockPtr lblock = get_block_from_selector_stack('r', lid);
//		sip::BlockId did;
//		sip::Block::BlockPtr dblock = get_block_from_selector_stack('w', did);
//		double * ddata = dblock->get_data();
//		double * rdata = lblock->get_data();
//		for (int i = 0; i < dblock->size(); ++i) {
//			ddata[i] = rdata[i] * rvalue;
//		}
//		return;
//	}
//
//	//get destination operand info
//	bool d_is_scalar = is_scalar(op_table_.arg2(pc));
//
//#ifdef HAVE_CUDA
//	if (gpu_enabled_) {
//		std::cout<<"Contraction on the GPU at line "<<line_number()<<std::endl;
//
//		sip::BlockSelector rselector = block_selector_stack_.top();
//		sip::Block::BlockPtr g_rblock = get_gpu_block('r');
//
//		sip::BlockSelector lselector = block_selector_stack_.top();
//		sip::Block::BlockPtr g_lblock = get_gpu_block('r');
//
//		sip::Block::BlockPtr g_dblock;
//		sip::index_selector_t g_dselected_index_ids;
//		double *g_d;
//		if (d_is_scalar) {
//			double h_d = 0.0;
//			sip::BlockShape scalar_shape;
//			g_dblock = new sip::Block(scalar_shape);
//			//g_dblock->gpu_data_ = _gpu_allocate(1);
//			g_dblock->allocate_gpu_data();
//			std::fill(g_dselected_index_ids + 0, g_dselected_index_ids + MAX_RANK, sip::unused_index_slot);
//		} else {
//			sip::BlockSelector dselector = block_selector_stack_.top();
//			std::copy(dselector.index_ids_ + 0, dselector.index_ids_ + MAX_RANK, g_dselected_index_ids + 0);
//			g_dblock = get_gpu_block('w');
//		}
//
//		_gpu_contract(g_dblock->get_gpu_data(), drank, g_dblock->shape().segment_sizes_, g_dselected_index_ids,
//				g_lblock->get_gpu_data(), lrank, g_lblock->shape().segment_sizes_, lselector.index_ids_,
//				g_rblock->get_gpu_data(), rrank, g_rblock->shape().segment_sizes_, rselector.index_ids_);
//
//		if (d_is_scalar) {
//			double h_dbl;
//			_gpu_device_to_host(&h_dbl, g_dblock->get_gpu_data(), 1);
//			set_scalar_value(op_table_.arg2(pc), h_dbl);
//		}
//
//		return;
//	}
//#endif
//	//std::cout<<"Contraction at line "<<line_number()<<std::endl;
//
//	//get right operand info
//	sip::BlockSelector rselector = block_selector_stack_.top();
//	sip::check(rrank == sip_tables_.array_rank(rselector.array_id_),
//			"SIP or compiler bug, inconsistent for r arg in contract op");
//	sip::BlockId rid;
//	sip::Block::BlockPtr rblock = get_block_from_selector_stack('r', rid);
//	sip::check(rarray == rid.array_id(),
//			"SIP or compiler bug:  inconsistent array ids in contract op");
//
//	//get left operand info
//	sip::BlockSelector lselector = block_selector_stack_.top();
//	sip::check(lrank == sip_tables_.array_rank(lselector.array_id_),
//			"SIP or compiler bug, inconsistent for l arg in contract op");
//	sip::BlockId lid;
//	sip::Block::BlockPtr lblock = get_block_from_selector_stack('r', lid);
//
//	//If result of contraction is scalar.
//	sip::Block::BlockPtr dblock = NULL;
//	sip::index_selector_t dselected_index_ids;
//	if (d_is_scalar) {
//		//		double *dvalPtr = data_manager_.scalar_address(op_table_.result_array(pc));
//		//		*dvalPtr = 0; // Initialize to 0 as required by Dmitry's Contraction routine
//		//		sip::BlockShape scalar_shape;
//		//		dblock = new sip::Block(scalar_shape, dvalPtr);
//		//		std::fill(dselected_index_ids + 0, dselected_index_ids + MAX_RANK,
//		//				sip::unused_index_slot);
//		dblock = data_manager_.scalar_blocks_[op_table_.arg2(pc)];
//	} else {
//		sip::BlockSelector dselector = block_selector_stack_.top();
//		std::copy(dselector.index_ids_ + 0, dselector.index_ids_ + MAX_RANK,
//				dselected_index_ids + 0);
//		sip::check(drank == sip_tables_.array_rank(dselector.array_id_),
//				"SIP or compiler bug, inconsistent for d arg in contract op");
//		sip::BlockId did;
//		dblock = get_block_from_selector_stack('w', did);
//
//	}
//
//	//get contraction pattern
//	int pattern_size = drank + lrank + rrank;
//	std::vector<int> aces_pattern(pattern_size, 0);	// Initialize vector with pattern_size elements of value 0
//	std::vector<int>::iterator it;
//	it = aces_pattern.begin();
//
//	it = std::copy(dselected_index_ids + 0, dselected_index_ids + drank, it);
//	it = std::copy(lselector.index_ids_ + 0, lselector.index_ids_ + lrank, it);
//	it = std::copy(rselector.index_ids_ + 0, rselector.index_ids_ + rrank, it);
//
//	//	int contraction_pattern[lrank + rrank];
//	int contraction_pattern[MAX_RANK * 2];
//	int ierr;
//
//	get_contraction_ptrn_(drank, lrank, rrank, aces_pattern.data(),
//			contraction_pattern, ierr);
//	check(ierr == 0, std::string("error returned from get_contraction_ptrn_"),
//			line_number());
//	//    INPUT:
//	//    ! - nthreads - number of threads requested;
//	//    ! - contr_ptrn(1:left_rank+right_rank) - contraction pattern;
//	//    ! - ltens/ltens_num_dim/ltens_dim_extent - left tensor block / its rank / and dimension extents;
//	//    ! - rtens/rtens_num_dim/rtens_dim_extent - right tensor block / its rank / and dimension extents;
//	//    ! - dtens/dtens_num_dim/dtens_dim_extent - destination tensor block (initialized!) / its rank / and dimension extents;
//	//    !OUTPUT:
//	//    ! - dtens - modified destination tensor block;
//	//    ! - ierr - error code (0: success);
//	//     */
//	int nthreads = sip::MAX_OMP_THREADS;
//	tensor_block_contract__(nthreads, contraction_pattern, lblock->get_data(),
//			lrank,
//			const_cast<segment_size_array_t&>(lblock->shape().segment_sizes_),
//			rblock->get_data(), rrank,
//			const_cast<segment_size_array_t&>(rblock->shape().segment_sizes_),
//			dblock->get_data(), drank,
//			const_cast<segment_size_array_t&>(dblock->shape().segment_sizes_),
//			ierr);
//	//std::cout <<"scalar:" << dblock.get_data()[0] << std::endl;
//	//	if (d_is_scalar){
//	//		set_scalar_value(op_table_.result_array(pc), *(dblock->get_data()));
//	//		delete dblock;
//	//	}
//
//}


////the different treatment of where clauses and relational expressions
////is inherited from ACESIII.
//bool Interpreter::evaluate_int_relational_expr(int pc) {
//	int val2 = control_stack_.top();
//	control_stack_.pop();
//	int val1 = control_stack_.top();
//	control_stack_.pop();
//	bool result;
//	int opcode = op_table_.opcode(pc);
//	switch (opcode) {
//	case int_equal_op:
//		result = (val1 == val2);
//		break;
//	case int_nequal_op:
//		result = (val1 != val2);
//		break;
//	case int_ge_op:
//		result = (val1 >= val2);
//		break;
//	case int_le_op:
//		result = (val1 <= val2);
//		break;
//	case int_gt_op:
//		result = (val1 > val2);
//		break;
//	case int_lt_op:
//		result = (val1 < val2);
//		break;
//	default: {
//		std::cout << "int relational expression opcode =" << opcode;
//		check(false, " illegal operator code");
//	}
//	}
//	return result;
//}

//bool Interpreter::evaluate_double_relational_expr(int pc) {
//	double val2 = expression_stack_.top();
//	expression_stack_.pop();
//	double val1 = expression_stack_.top();
//	expression_stack_.pop();
//	bool result;
//	int opcode = op_table_.opcode(pc);
//	switch (opcode) {
//	case scalar_eq_op:
//		result = (val1 == val2);
//		break;
//	case scalar_ne_op:
//		result = (val1 != val2);
//		break;
//	case scalar_ge_op:
//		result = (val1 >= val2);
//		break;
//	case scalar_le_op:
//		result = (val1 <= val2);
//		break;
//	case scalar_gt_op:
//		result = (val1 > val2);
//		break;
//	case scalar_lt_op:
//		result = (val1 < val2);
//		break;
//	default: {
//		std::cout << "double relational expression opcode =" << opcode;
//		check(false, " illegal operator code");
//	}
//	}
//	return result;
//}








//void Interpreter::handle_self_multiply_op(int pc) {
//	check(is_scalar(op_table_.arg1(pc)), "*= only for scalar right hand side",
//			line_number());
//	bool d_is_scalar = is_scalar(op_table_.arg2(pc));
//	double lval = scalar_value(op_table_.arg1(pc));
//	if (d_is_scalar) {
//		double dval = scalar_value(op_table_.arg2(pc));
//		dval *= lval;
//		set_scalar_value(op_table_.arg2(pc), dval);
//	} else {
//		sip::BlockSelector dselector = block_selector_stack_.top();
//		int drank = sip_tables_.array_rank(dselector.array_id_);
//		check(dselector.array_id_ == op_table_.arg2(pc),
//				"compiler or sip bug:  inconsistent array values in self multiply and selector for d value",
//				line_number());
//
//#ifdef HAVE_CUDA
//		if (gpu_enabled_) {
//			sip::BlockId did;
//			sip::Block::BlockPtr dblock = get_gpu_block('u', did);
//			block_selector_stack_.pop();
//			dblock->gpu_scale(lval);
//			//************ check for write_back_contiguous
//		} else
//
//#endif
//		{
//			//TODO introduce debug level--or just delete?
//			sip::BlockId did;
//			sip::Block::BlockPtr dblock = get_block_from_selector_stack('u',
//					did);
//			//			block_selector_stack_.pop();
//			dblock->scale(lval);
//		}
//	}
//}

//void Interpreter::loop_start(LoopManager * loop) {
//	int enddo_pc = arg0();
//	if (loop->update()) { //there is at least one iteration of loop
//		loop_manager_stack_.push(loop);
//		control_stack_.push(pc + 1); //push pc of first instruction of loop body (including where)
//		control_stack_.push(enddo_pc); //used by where clauses
//		data_manager_.enter_scope();
//		++pc;
//	} else {
//		loop->finalize();
//		delete loop;
//		pc = enddo_pc + 1; //jump to statement following enddo
//	}
//}
//void Interpreter::loop_end() {
//	int own_pc = pc;
//	control_stack_.pop(); //remove own location
//	data_manager_.leave_scope();
//	bool more_iterations = loop_manager_stack_.top()->update();
//	if (more_iterations) {
//		data_manager_.enter_scope();
//		pc = control_stack_.top(); //get the place to jump back to
//		control_stack_.push(own_pc); //add own location for where clause in next iteration
//	} else {
//		LoopManager* loop = loop_manager_stack_.top();
//		loop->finalize();
//		loop_manager_stack_.pop(); //remove loop from stack
//		control_stack_.pop(); //pop address of first instruction in body
//		delete loop;
//		++pc;
//	}
//}
//
//Block::BlockPtr Interpreter::get_block_from_instruction(char intent,
//		bool contiguous_allowed) {
//	BlockSelector selector(arg0(), arg1(), index_selectors());
//	BlockId block_id;
//	Block::BlockPtr block = get_block(intent, selector, block_id, contiguous_allowed);
//	return block;
//}
//
//sip::BlockId Interpreter::get_block_id_from_selector_stack() {
//	sip::BlockSelector selector = block_selector_stack_.top();
//	block_selector_stack_.pop();
//	return block_id(selector);
//}
//
//sip::Block::BlockPtr Interpreter::get_block_from_selector_stack(char intent,
//		sip::BlockId& id, bool contiguous_allowed) {
//	sip::BlockSelector selector = block_selector_stack_.top();
//	block_selector_stack_.pop();
//	return get_block(intent, selector, id);
//}
//
//sip::Block::BlockPtr Interpreter::get_block_from_selector_stack(char intent,
//		bool contiguous_allowed) {
//	sip::BlockId block_id;
//	return get_block_from_selector_stack(intent, block_id, contiguous_allowed);
//}
//
//sip::Block::BlockPtr Interpreter::get_block(char intent,
//		sip::BlockSelector& selector, sip::BlockId& id,
//		bool contiguous_allowed) {
//	int array_id = selector.array_id_;
//	sip::Block::BlockPtr block;
//	if (sip_tables_.array_rank(selector.array_id_) == 0) { //this "array" was declared to be a scalar.  Nothing to remove from selector stack.
//		id = sip::BlockId(array_id);
//		block = data_manager_.get_scalar_block(array_id);
//		return block;
//	}
//	if (selector.rank_ == 0) { //this is a static array provided without a selector, block is entire array
//		block = data_manager_.contiguous_array_manager_.get_array(array_id);
//		id = sip::BlockId(array_id);
//		return block;
//	}
//	//argument is a block with an explicit selector
//	std::cout << " get_block selector, selecto rank, siptable rank " << selector << ", " << selector.rank_ << "," << sip_tables_.array_rank(selector.array_id_) << std::endl;
//	sip::check(selector.rank_ == sip_tables_.array_rank(selector.array_id_),
//			"SIP or Compiler bug: inconsistent ranks in sipTable and selector");
//	id = block_id(selector);
//	bool is_contiguous = sip_tables_.is_contiguous(selector.array_id_);
//	sip::check(!is_contiguous || contiguous_allowed,
//			"using contiguous block in a context that doesn't support it");
//	switch (intent) {
//	case 'r': {
//		block = is_contiguous ?
//				data_manager_.contiguous_array_manager_.get_block_for_reading(
//						id, read_block_list_) :
//				sial_ops_.get_block_for_reading(id);
//	}
//		break;
//	case 'w': {
//		bool is_scope_extent = sip_tables_.is_scope_extent(selector.array_id_);
//		block = is_contiguous ?
//				data_manager_.contiguous_array_manager_.get_block_for_updating( //w and u are treated identically for contiguous arrays
//						id, write_back_list_) :
//				sial_ops_.get_block_for_writing(id, is_scope_extent);
//	}
//		break;
//	case 'u': {
//		block = is_contiguous ?
//				data_manager_.contiguous_array_manager_.get_block_for_updating(
//						id, write_back_list_) :
//				sial_ops_.get_block_for_updating(id);
//	}
//		break;
//	default:
//		sip::check(false,
//				"SIP bug:  illegal or unsupported intent given to get_block");
//	}
//	return block;
//
//}
//
//void Interpreter::contiguous_blocks_post_op() {
//
//	// Write back all contiguous slices
//	//	while (!write_back_list_.empty()) {
//	//		sip::WriteBack * ptr = write_back_list_.front();
//	//#ifdef HAVE_CUDA
//	//		if (gpu_enabled_) {
//	//			sip::Block::BlockPtr cblock = ptr->get_block();
//	//			data_manager_.block_manager_.lazy_gpu_read_on_host(cblock);
//	//		}
//	//#endif
//	//		ptr->do_write_back();
//	//		write_back_list_.erase(write_back_list_.begin());
//	//		delete ptr;
//	//	}
//	//
//	//	// Free up contiguous slices only needed for read.
//	//	while (!read_block_list_.empty()){
//	//		// TODO FIXME GPU ?????????????????
//	//		Block::BlockPtr bptr = read_block_list_.front();
//	//		read_block_list_.erase(read_block_list_.begin());
//	//		delete bptr;
//	//	}
//
//	for (WriteBackList::iterator it = write_back_list_.begin();
//			it != write_back_list_.end(); ++it) {
//		WriteBack* wb = *it;
//#ifdef HAVE_CUDA
//		if (gpu_enabled_) {
//			// Read from GPU back into host to do write back.
//			sip::Block::BlockPtr cblock = (*it)->get_block();
//			data_manager_.block_manager_.lazy_gpu_read_on_host(cblock);
//		}
//#endif
//		wb->do_write_back();
//		delete wb;
//		*it = NULL;
//	}
//	write_back_list_.clear();
//
//	for (ReadBlockList::iterator it = read_block_list_.begin();
//			it != read_block_list_.end(); ++it) {
//		Block::BlockPtr bptr = *it;
//		delete bptr;
//		*it = NULL;
//	}
//	read_block_list_.clear();
//
//}
//
//#ifdef HAVE_CUDA
//sip::Block::BlockPtr Interpreter::get_gpu_block(char intent, sip::BlockId& id, bool contiguous_allowed) {
//	sip::BlockSelector selector = block_selector_stack_.top();
//	block_selector_stack_.pop();
//	return get_gpu_block(intent, selector, id, contiguous_allowed);
//}
//
//sip::Block::BlockPtr Interpreter::get_gpu_block(char intent, bool contiguous_allowed) {
//	sip::BlockId block_id;
//	return get_gpu_block(intent, block_id, contiguous_allowed);
//}
//
//sip::Block::BlockPtr Interpreter::get_gpu_block(char intent, sip::BlockSelector& selector, sip::BlockId& id,
//		bool contiguous_allowed) {
//	id = block_id(selector);
//	sip::Block::BlockPtr block;
//	bool is_contiguous = sip_tables_.is_contiguous(selector.array_id_);
//	sip::check(!is_contiguous || contiguous_allowed,
//			"using contiguous block in a context that doesn't support it");
//
//	switch (intent) {
//		case 'r': {
//			if (is_contiguous) {
//				block = data_manager_.contiguous_array_manager_.get_block_for_reading(id, read_block_list_);
//				data_manager_.block_manager_.lazy_gpu_read_on_device(block);
//			} else {
//				block = data_manager_.block_manager_.get_gpu_block_for_reading(id);
//			}
//		}
//		break;
//		case 'w': {
//			bool is_scope_extent = sip_tables_.is_scope_extent(selector.array_id_);
//			if (is_contiguous) {
//				block = data_manager_.contiguous_array_manager_.get_block_for_updating(
//						id, write_back_list_);
//				sip::BlockShape shape = sip_tables_.shape(id);
//				data_manager_.block_manager_.lazy_gpu_write_on_device(block, id, shape);
//			} else {
//				block = data_manager_.block_manager_.get_gpu_block_for_writing(id, is_scope_extent);
//			}
//		}
//		break;
//		case 'u': {
//			if (is_contiguous) {
//				block =
//				data_manager_.contiguous_array_manager_.get_block_for_updating(
//						id, write_back_list_);
//				sip::BlockShape shape = sip_tables_.shape(id);
//				data_manager_.block_manager_.lazy_gpu_update_on_device(block);
//			} else {
//				block = data_manager_.block_manager_.get_gpu_block_for_updating(id);
//			}
//		}
//		break;
//		default:
//		sip::check(false, "illegal or unsupported intent given to get_block");
//	}
//	return block;
//}
//
//sip::Block::BlockPtr Interpreter::get_gpu_block_from_selector_stack(char intent,
//		sip::BlockId& id, bool contiguous_allowed) {
//	sip::Block::BlockPtr block;
//	sip::BlockSelector selector = block_selector_stack_.top();
//	block_selector_stack_.pop();
//	int array_id = selector.array_id_;
//	if (sip_tables_.array_rank(selector.array_id_) == 0) { //this is a scalar
//		block = new sip::Block(data_manager_.scalar_address(array_id));
//		id = sip::BlockId(array_id);
//		return block;
//	}
//	sip::check(selector.rank_ != 0, "Contiguous arrays not supported for GPU", line_number());
//
//	//argument is a block with an explicit selector
//	sip::check(selector.rank_ == sip_tables_.array_rank(selector.array_id_),
//			"SIP or Compiler bug: inconsistent ranks in sipTable and selector");
//	id = block_id(selector);
//	bool is_contiguous = sip_tables_.is_contiguous(selector.array_id_);
//
//	switch (intent) {
//		case 'r': {
//			block = data_manager_.block_manager_.get_gpu_block_for_reading(id);
//		}
//		break;
//		case 'w': {
//			bool is_scope_extent = sip_tables_.is_scope_extent(selector.array_id_);
//			block = data_manager_.block_manager_.get_gpu_block_for_writing(id,
//					is_scope_extent);
//		}
//		break;
//		case 'u': {
//			block = data_manager_.block_manager_.get_gpu_block_for_updating(id); //w and u same for contig
//		}
//		break;
//		default:
//		sip::check(false, "illegal or unsupported intent given to get_block");
//	}
//
//	return block;
//}
//
//#endif
//
//}
///* namespace sip */



//DO NOT DELETE until reintegrated into handle_contract_op
//#ifdef HAVE_CUDA
//	if (gpu_enabled_) {
//		std::cout<<"Contraction on the GPU at line "<<line_number()<<std::endl;
//
//		sip::BlockSelector rselector = block_selector_stack_.top();
//		sip::Block::BlockPtr g_rblock = get_gpu_block('r');
//
//		sip::BlockSelector lselector = block_selector_stack_.top();
//		sip::Block::BlockPtr g_lblock = get_gpu_block('r');
//
//		sip::Block::BlockPtr g_dblock;
//		sip::index_selector_t g_dselected_index_ids;
//		double *g_d;
//		if (d_is_scalar) {
//			double h_d = 0.0;
//			sip::BlockShape scalar_shape;
//			g_dblock = new sip::Block(scalar_shape);
//			//g_dblock->gpu_data_ = _gpu_allocate(1);
//			g_dblock->allocate_gpu_data();
//			std::fill(g_dselected_index_ids + 0, g_dselected_index_ids + MAX_RANK, sip::unused_index_slot);
//		} else {
//			sip::BlockSelector dselector = block_selector_stack_.top();
//			std::copy(dselector.index_ids_ + 0, dselector.index_ids_ + MAX_RANK, g_dselected_index_ids + 0);
//			g_dblock = get_gpu_block('w');
//		}
//
//		_gpu_contract(g_dblock->get_gpu_data(), drank, g_dblock->shape().segment_sizes_, g_dselected_index_ids,
//				g_lblock->get_gpu_data(), lrank, g_lblock->shape().segment_sizes_, lselector.index_ids_,
//				g_rblock->get_gpu_data(), rrank, g_rblock->shape().segment_sizes_, rselector.index_ids_);
//
//		if (d_is_scalar) {
//			double h_dbl;
//			_gpu_device_to_host(&h_dbl, g_dblock->get_gpu_data(), 1);
//			set_scalar_value(op_table_.result_array(pc), h_dbl);
//		}
//
//		return;
//	}
//#endif
