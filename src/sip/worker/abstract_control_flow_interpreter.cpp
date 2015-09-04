/*
 * abstract_control_flow_interpreter.cpp
 *
 *  Created on: Jun 16, 2015
 *      Author: njindal
 */

#include <abstract_control_flow_interpreter.h>
#include "sial_math.h"

namespace sip {

AbstractControlFlowInterpreter::AbstractControlFlowInterpreter(int worker_rank,
				int num_workers,
				const SipTables& sipTables) :
				worker_rank_(worker_rank),
				num_workers_(num_workers),
				sip_tables_(sipTables),
				op_table_(sip_tables_.op_table_),
				data_manager_(sipTables),
				iteration_(0),
				pc_(0){
}

AbstractControlFlowInterpreter::~AbstractControlFlowInterpreter() {}

void AbstractControlFlowInterpreter::do_interpret() {
	int nops = op_table_.size();
	do_interpret(0, nops);
}

void AbstractControlFlowInterpreter::do_interpret(int pc_start, int pc_end) {
	pc_ = pc_start;

restart:
	while (pc_ < pc_end) {
		opcode_t opcode = op_table_.opcode(pc_);

		pre_interpret(pc_);
		int old_pc = pc_;

		switch (opcode) {
		// Opcodes that modify the program counter (pc_)
		case goto_op: 					handle_goto_op(pc_); 					break;
		case jump_if_zero_op: 			handle_jump_if_zero_op(pc_); 			break;
		case stop_op: 					handle_stop_op(pc_);					break;
		case call_op: 					handle_call_op(pc_);					break;
		case return_op: 				handle_return_op(pc_);					break;
		case do_op: 					handle_do_op(pc_);						break;
		case enddo_op: 					handle_enddo_op(pc_);					break;
		case exit_op: 					handle_exit_op(pc_);					break;
		case where_op: 					handle_where_op(pc_);					break;
		case pardo_op: 					handle_pardo_op(pc_);					break;
		case endpardo_op: 				handle_endpardo_op(pc_);				break;

		// Opcodes that don't modify the program counter
		case execute_op: 				handle_execute_op(pc_);					++pc_; break;
		case sip_barrier_op: 			handle_sip_barrier_op(pc_);				iteration_ = 0;++pc_; break;
		case broadcast_static_op: 		handle_broadcast_static_op(pc_);		++pc_; break;
		case push_block_selector_op: 	handle_push_block_selector_op(pc_);		++pc_; break;
		case allocate_op: 				handle_allocate_op(pc_);				++pc_; break;
		case deallocate_op: 			handle_deallocate_op(pc_);				++pc_; break;
		case allocate_contiguous_op: 	handle_allocate_contiguous_op(pc_);		++pc_; break;
		case deallocate_contiguous_op: 	handle_deallocate_contiguous_op(pc_);	++pc_; break;
		case get_op: 					handle_get_op(pc_);						++pc_; break;
		case put_accumulate_op: 		handle_put_accumulate_op(pc_);			++pc_; break;
		case put_replace_op: 			handle_put_replace_op(pc_);				++pc_; break;
		case put_initialize_op: 		handle_put_initialize_op(pc_);			++pc_; break;
		case put_increment_op: 			handle_put_increment_op(pc_);			++pc_; break;
		case put_scale_op: 				handle_put_scale_op(pc_);				++pc_; break;
		case create_op: 				handle_create_op(pc_);					++pc_; break;
		case delete_op: 				handle_delete_op(pc_);					++pc_; break;
		case string_load_literal_op: 	handle_string_load_literal_op(pc_);		++pc_; break;
		case int_load_value_op: 		handle_int_load_value_op(pc_);			++pc_; break;
		case int_load_literal_op: 		handle_int_load_literal_op(pc_);		++pc_; break;
		case int_store_op: 				handle_int_store_op(pc_);				++pc_; break;
		case index_load_value_op: 		handle_index_load_value_op(pc_);		++pc_; break;
		case int_add_op: 				handle_int_add_op(pc_);					++pc_; break;
		case int_subtract_op: 			handle_int_subtract_op(pc_);			++pc_; break;
		case int_multiply_op: 			handle_int_multiply_op(pc_);			++pc_; break;
		case int_divide_op: 			handle_int_divide_op(pc_);				++pc_; break;
		case int_equal_op: 				handle_int_equal_op(pc_);				++pc_; break;
		case int_nequal_op: 			handle_int_nequal_op(pc_);				++pc_; break;
		case int_ge_op: 				handle_int_ge_op(pc_);					++pc_; break;
		case int_le_op: 				handle_int_le_op(pc_);					++pc_; break;
		case int_gt_op: 				handle_int_gt_op(pc_);					++pc_; break;
		case int_lt_op: 				handle_int_lt_op(pc_);					++pc_; break;
		case int_neg_op: 				handle_int_neg_op(pc_);					++pc_; break;
		case cast_to_int_op: 			handle_cast_to_int_op(pc_);				++pc_; break;
		case scalar_load_value_op: 		handle_scalar_load_value_op(pc_);		++pc_; break;
		case scalar_store_op: 			handle_scalar_store_op(pc_);			++pc_; break;
		case scalar_add_op: 			handle_scalar_add_op(pc_);				++pc_; break;
		case scalar_subtract_op: 		handle_scalar_subtract_op(pc_);			++pc_; break;
		case scalar_multiply_op: 		handle_scalar_multiply_op(pc_);			++pc_; break;
		case scalar_divide_op: 			handle_scalar_divide_op(pc_);			++pc_; break;
		case scalar_exp_op: 			handle_scalar_exp_op(pc_);				++pc_; break;
		case scalar_eq_op: 				handle_scalar_eq_op(pc_);				++pc_; break;
		case scalar_ne_op: 				handle_scalar_ne_op(pc_);				++pc_; break;
		case scalar_ge_op: 				handle_scalar_ge_op(pc_);				++pc_; break;
		case scalar_le_op: 				handle_scalar_le_op(pc_);				++pc_; break;
		case scalar_gt_op: 				handle_scalar_gt_op(pc_);				++pc_; break;
		case scalar_lt_op: 				handle_scalar_lt_op(pc_);				++pc_; break;
		case scalar_neg_op: 			handle_scalar_neg_op(pc_);				++pc_; break;
		case scalar_sqrt_op: 			handle_scalar_sqrt_op(pc_);				++pc_; break;
		case cast_to_scalar_op: 		handle_cast_to_scalar_op(pc_);			++pc_; break;
		case collective_sum_op: 		handle_collective_sum_op(pc_);			++pc_; break;
		case assert_same_op: 			handle_assert_same_op(pc_);				++pc_; break;
		case block_copy_op: 			handle_block_copy_op(pc_);				++pc_; break;
		case block_permute_op: 			handle_block_permute_op(pc_);			++pc_; break;
		case block_fill_op: 			handle_block_fill_op(pc_);				++pc_; break;
		case block_scale_op: 			handle_block_scale_op(pc_);				++pc_; break;
		case block_scale_assign_op: 	handle_block_scale_assign_op(pc_);		++pc_; break;
		case block_accumulate_scalar_op:handle_block_accumulate_scalar_op(pc_);	++pc_; break;
		case block_add_op: 				handle_block_add_op(pc_);				++pc_; break;
		case block_subtract_op: 		handle_block_subtract_op(pc_);			++pc_; break;
		case block_contract_op: 		handle_block_contract_op(pc_);			++pc_; break;
		case block_contract_to_scalar_op:handle_block_contract_to_scalar_op(pc_);++pc_; break;
		case block_load_scalar_op: 		handle_block_load_scalar_op(pc_);		++pc_; break;
		case print_string_op: 			handle_print_string_op(pc_);			++pc_; break;
		case print_scalar_op: 			handle_print_scalar_op(pc_);			++pc_; break;
		case print_int_op: 				handle_print_int_op(pc_);				++pc_; break;
		case print_index_op: 			handle_print_index_op(pc_);				++pc_; break;
		case print_block_op: 			handle_print_block_op(pc_);				++pc_; break;
		case println_op: 				handle_println_op(pc_);					++pc_; break;
		case gpu_on_op:					handle_gpu_on_op(pc_);					++pc_; break;
		case gpu_off_op:				handle_gpu_off_op(pc_);					++pc_; break;
		case set_persistent_op: 		handle_set_persistent_op(pc_);			++pc_; break;
		case restore_persistent_op: 	handle_restore_persistent_op(pc_);		++pc_; break;
		case idup_op: 					handle_idup_op(pc_);					++pc_; break;
		case iswap_op: 					handle_iswap_op(pc_);					++pc_; break;
		case sswap_op: 					handle_sswap_op(pc_);					++pc_; break;
		default:
			fail(opcodeToName(opcode) + " not yet implemented ", line_number());
		}// switch

		post_interpret(old_pc, pc_);

	}// while


	// If the end of program causes the value of pc to change to something
	// between pc_start and pc_end, go through the interpretation again.
	// Needed by BlockConsistencyInterpreter.
	handle_program_end(pc_);
	if (pc_ >= pc_start && pc_ < pc_end)
		goto restart;

} //interpret



void AbstractControlFlowInterpreter::contiguous_blocks_post_op() {
	for (WriteBackList::iterator it = write_back_list_.begin();
			it != write_back_list_.end(); ++it) {
		WriteBack* wb = *it;
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

void AbstractControlFlowInterpreter::handle_user_sub_op(int pc) {
	int num_args = arg1(pc);
	int func_slot = arg0(pc);
	int ierr = 0;
	if (num_args == 0) {
		SpecialInstructionManager::fp0 func =
				sip_tables_.special_instruction_manager().get_no_arg_special_instruction_ptr(
						func_slot);
		func(ierr);
		CHECK(ierr == 0,
				"error returned from special super instruction "
						+ sip_tables_.special_instruction_manager().name(
								func_slot));
		return;
	}
	// num_args >= 1.  Set up first argument
	const std::string signature(
			sip_tables_.special_instruction_manager().get_signature(func_slot));
	sip::BlockId block_id0;
	char intent0 = signature[0];
	sip::BlockSelector arg_selector0 = block_selector_stack_.top();
	int array_id0 = arg_selector0.array_id_;
	int rank0 = sip_tables_.array_rank(array_id0);
	sip::Block::BlockPtr block0 = get_block_from_selector_stack(intent0,
			block_id0, true);
    if (intent0 == 'w') block0->fill(0.0);
	int block0_size = block0->size();
	segment_size_array_t& seg_sizes0 =
			const_cast<segment_size_array_t&>(block0->shape().segment_sizes_);
	Block::dataPtr data0 = block0->get_data();
	if (num_args == 1) {
		SpecialInstructionManager::fp1 func =
				sip_tables_.special_instruction_manager().get_one_arg_special_instruction_ptr(
						func_slot);
		//	typedef void(*fp1)(int& array_slot, int& rank, int * index_values, int& size, int * extents, double * block_data, int& ierr
		func(array_id0, rank0, block_id0.index_values(), block0_size, seg_sizes0,
				data0, ierr);
		sip::check(ierr == 0,
				"error returned from special super instruction "
						+ sip_tables_.special_instruction_manager().name(
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
    if (intent1 == 'w') block1->fill(0.0);
	int block1_size = block1->size();
	segment_size_array_t& seg_sizes1 =
			const_cast<segment_size_array_t&>(block1->shape().segment_sizes_);
	Block::dataPtr data1 = block1->get_data();
	if (num_args == 2) {
		SpecialInstructionManager::fp2 func =
				sip_tables_.special_instruction_manager().get_two_arg_special_instruction_ptr(
						func_slot);

		func(array_id0, rank0, block_id0.index_values(), block0_size, seg_sizes0,
				data0, array_id1, rank1, block_id1.index_values(), block1_size,
				seg_sizes1, data1, ierr);
		sip::check(ierr == 0,
				"error returned from special super instruction "
						+ sip_tables_.special_instruction_manager().name(
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
    if (intent2 == 'w') block2->fill(0.0);
	int block2_size = block2->size();
	segment_size_array_t& seg_sizes2 =
			const_cast<segment_size_array_t&>(block2->shape().segment_sizes_);
	Block::dataPtr data2 = block2->get_data();
	if (num_args == 3) {
		SpecialInstructionManager::fp3 func =
				sip_tables_.special_instruction_manager().get_three_arg_special_instruction_ptr(
						func_slot);

		func(array_id0, rank0, block_id0.index_values(), block0_size, seg_sizes0,
				data0, array_id1, rank1, block_id1.index_values(), block1_size,
				seg_sizes1, data1, array_id2, rank2, block_id2.index_values(),
				block2_size, seg_sizes2, data2, ierr);
		sip::check(ierr == 0,
				"error returned from special super instruction "
						+ sip_tables_.special_instruction_manager().name(
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
    if (intent3 == 'w') block3->fill(0.0);
	int block3_size = block3->size();
	segment_size_array_t& seg_sizes3 =
			const_cast<segment_size_array_t&>(block3->shape().segment_sizes_);
	Block::dataPtr data3 = block3->get_data();
	if (num_args == 4) {
		SpecialInstructionManager::fp4 func =
				sip_tables_.special_instruction_manager().get_four_arg_special_instruction_ptr(
						func_slot);
		func(array_id0, rank0, block_id0.index_values(), block0_size, seg_sizes0,
				data0, array_id1, rank1, block_id1.index_values(), block1_size,
				seg_sizes1, data1, array_id2, rank2, block_id2.index_values(),
				block2_size, seg_sizes2, data2, array_id3, rank3,
				block_id3.index_values(), block3_size, seg_sizes3, data3, ierr);
		sip::check(ierr == 0,
				"error returned from special super instruction "
						+ sip_tables_.special_instruction_manager().name(
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
    if (intent4 == 'w') block4->fill(0.0);
	int block4_size = block4->size();
	segment_size_array_t& seg_sizes4 =
			const_cast<segment_size_array_t&>(block4->shape().segment_sizes_);
	Block::dataPtr data4 = block4->get_data();
	if (num_args == 5) {
		SpecialInstructionManager::fp5 func =
				sip_tables_.special_instruction_manager().get_five_arg_special_instruction_ptr(
						func_slot);
		func(array_id0, rank0, block_id0.index_values(), block0_size, seg_sizes0,
				data0, array_id1, rank1, block_id1.index_values(), block1_size,
				seg_sizes1, data1, array_id2, rank2, block_id2.index_values(),
				block2_size, seg_sizes2, data2, array_id3, rank3,
				block_id3.index_values(), block3_size, seg_sizes3, data3,
				array_id4, rank4, block_id4.index_values(), block4_size,
				seg_sizes4, data4, ierr);
		sip::check(ierr == 0,
				"error returned from special super instruction "
						+ sip_tables_.special_instruction_manager().name(
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
    if (intent5 == 'w') block5->fill(0.0);
	int block5_size = block5->size();
	segment_size_array_t& seg_sizes5 =
			const_cast<segment_size_array_t&>(block5->shape().segment_sizes_);
	Block::dataPtr data5 = block5->get_data();
	if (num_args == 6) {
		SpecialInstructionManager::fp6 func =
				sip_tables_.special_instruction_manager().get_six_arg_special_instruction_ptr(
						func_slot);
		func(array_id0, rank0, block_id0.index_values(), block0_size, seg_sizes0,
				data0, array_id1, rank1, block_id1.index_values(), block1_size,
				seg_sizes1, data1, array_id2, rank2, block_id2.index_values(),
				block2_size, seg_sizes2, data2, array_id3, rank3,
				block_id3.index_values(), block3_size, seg_sizes3, data3,
				array_id4, rank4, block_id4.index_values(), block4_size,
				seg_sizes4, data4, array_id5, rank5, block_id5.index_values(),
				block5_size, seg_sizes5, data5, ierr);
		sip::check(ierr == 0,
				"error returned from special super instruction "
						+ sip_tables_.special_instruction_manager().name(
								func_slot));
		return;
	}

	sip::fail("Implementation restriction:  At most 6 arguments to a super instruction supported.  This can be increased if necessary");
}



void AbstractControlFlowInterpreter::handle_execute_op(int pc) {

	int num_args = arg1(pc) ;
	int func_slot = arg0(pc);

	try {

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

	if (all_write && all_scalar_or_static){
		SIP_LOG(std::cout << "Executing Super Instruction "
				<< sip_tables_.special_instruction_manager().name(func_slot)
				<< " at line " << line_number() << std::endl);
		// Push back blocks on stack and execute the super instruction.
		while (!bs_list.empty()){
			BlockSelector bs = bs_list.back();
			block_selector_stack_.push(bs);
			bs_list.pop_back();
		}
		if (opcode_name == "get_my_rank"){
			sip::BlockId block_id0;
			sip::Block::BlockPtr block0 = get_block_from_selector_stack('w', block_id0, true);
			double* data = block0->get_data();
			data[0] = worker_rank_;
		} else {
			handle_user_sub_op(pc);
		}
	}

	} catch (const std::out_of_range& oor){
		std::string opcode_name = sip_tables_.special_instruction_manager().name(func_slot);
		std::cerr << "****************************************************************" << std::endl;
		std::cerr << "SUPER INSTRUCTION " << opcode_name << " NOT FOUND, SKIPPING !!!" << std::endl;
		std::cerr << oor.what() << std::endl;
		std::cerr << "****************************************************************" << std::endl;
	}
}



Block::BlockPtr AbstractControlFlowInterpreter::get_block_from_instruction(int pc, char intent,
		bool contiguous_allowed) {
	BlockSelector selector(arg0(pc), arg1(pc), index_selectors(pc));
	BlockId block_id;
	Block::BlockPtr block = get_block(intent, selector, block_id, contiguous_allowed);
	return block;
}

BlockId AbstractControlFlowInterpreter::get_block_id_from_instruction(int pc){
	int rank = arg0(pc);
	int array_id = arg1(pc);
	if (sip_tables_.is_contiguous_local(array_id)){
		int upper[MAX_RANK];
		int lower[MAX_RANK];
		for (int j = rank-1; j >=0; --j){
			upper[j] = control_stack_.top();
			control_stack_.pop();
			lower[j] = control_stack_.top();
			control_stack_.pop();
		}
		for (int j = rank; j < MAX_RANK; ++j){
			upper[j] = unused_index_value;
			lower[j] = unused_index_value;
		}
		BlockId tmp(array_id, lower, upper);
		return tmp;
	}
	return BlockId(array_id, index_selectors(pc));
}


sip::BlockId AbstractControlFlowInterpreter::get_block_id_from_selector_stack() {
	BlockSelector selector = block_selector_stack_.top();
	block_selector_stack_.pop();
	return get_block_id_from_selector(selector);
}

sip::BlockId AbstractControlFlowInterpreter::get_block_id_from_selector(const BlockSelector &selector) {

	int array_id = selector.array_id_;
	int rank = sip_tables_.array_rank(array_id);
	if (sip_tables_.is_contiguous_local(array_id)){
		CHECK_WITH_LINE (selector.rank_ == rank,
				"SIP or Compiler bug: inconsistent ranks in sipTable and selector for contiguous local",line_number() );
		int upper[MAX_RANK];
		int lower[MAX_RANK];
		for (int j = rank-1; j >=0; --j){
			upper[j] = control_stack_.top();
			control_stack_.pop();
			lower[j] = control_stack_.top();
			control_stack_.pop();
		}
		for (int j = rank; j < MAX_RANK; ++j){
			upper[j] = unused_index_value;
			lower[j] = unused_index_value;
		}
		BlockId tmp(array_id, lower, upper);
		return tmp;
	}
	return block_id(selector);
}

sip::Block::BlockPtr AbstractControlFlowInterpreter::get_block_from_selector_stack(char intent,
		sip::BlockId& id, bool contiguous_allowed) {
	sip::BlockSelector selector = block_selector_stack_.top();
	block_selector_stack_.pop();
	return get_block(intent, selector, id, contiguous_allowed);
}

sip::Block::BlockPtr AbstractControlFlowInterpreter::get_block_from_selector_stack(char intent,
		bool contiguous_allowed) {
	sip::BlockId block_id;
	return get_block_from_selector_stack(intent, block_id, contiguous_allowed);
}

sip::Block::BlockPtr AbstractControlFlowInterpreter::get_block(char intent,
		sip::BlockSelector& selector, sip::BlockId& id,
		bool contiguous_allowed) {
	int array_id = selector.array_id_;
	Block::BlockPtr block;
	if (sip_tables_.is_contiguous_local(array_id)){
		int rank = sip_tables_.array_rank(selector.array_id_);
		CHECK_WITH_LINE (selector.rank_ == rank,
				"SIP or Compiler bug: inconsistent ranks in sipTable and selector for contiguous local",line_number() );
		int upper[MAX_RANK];
		int lower[MAX_RANK];
		for (int j = rank-1; j >=0; --j){
			upper[j] = control_stack_.top();
			control_stack_.pop();
			lower[j] = control_stack_.top();
			control_stack_.pop();
		}
		for (int j = rank; j < MAX_RANK; ++j){
			upper[j] = unused_index_value;
			lower[j] = unused_index_value;
		}
		BlockId tmp_id(array_id, lower, upper);
		id = tmp_id;
		Block::BlockPtr block;
		switch(intent){
		case 'w': {
		block = data_manager_.contiguous_local_array_manager_.get_block_for_writing(id, write_back_list_);
		}
		break;
		case 'r': {
			block = data_manager_.contiguous_local_array_manager_.get_block_for_reading(id, read_block_list_);
		}
		break;
		case 'u': {
			block = data_manager_.contiguous_local_array_manager_.get_block_for_updating(id, write_back_list_);
		}
		break;
		default:
			fail("SIP bug:  illegal or unsupported intent given to get_block");

	}
		return block;
	}
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
	SIAL_CHECK(!is_contiguous || contiguous_allowed,
			"using contiguous block in a context that doesn't support it", line_number());
	switch (intent) {
	case 'r': {
		block = is_contiguous ?
				data_manager_.contiguous_array_manager_.get_block_for_reading(
						id, read_block_list_) :
				data_manager_.block_manager_.get_block_for_reading(id);
	}
		break;
	case 'w': {
		bool is_scope_extent = sip_tables_.is_scope_extent(selector.array_id_);
		block = is_contiguous ?
				data_manager_.contiguous_array_manager_.get_block_for_updating( //w and u are treated identically for contiguous arrays
						id, write_back_list_) :
				data_manager_.block_manager_.get_block_for_writing(id, is_scope_extent);
	}
		break;
	case 'u': {
		bool is_scope_extent = sip_tables_.is_scope_extent(selector.array_id_);
		block = is_contiguous ?
				data_manager_.contiguous_array_manager_.get_block_for_updating(
						id, write_back_list_) :
				data_manager_.block_manager_.get_block_for_updating(id);
	}
		break;
	default:
		sip::check(false,
				"SIP bug:  illegal or unsupported intent given to get_block");
	}
	return block;
}

void AbstractControlFlowInterpreter::loop_start(int &pc, LoopManager * loop) {
    int enddo_pc = arg0(pc);
    int loop_body_pc = pc+1;
    pc = loop_body_pc;
    control_stack_.push(loop_body_pc);
    control_stack_.push(enddo_pc);
    if (loop->update()) { //there is at least one iteration of loop
        loop_manager_stack_.push(loop);
        data_manager_.enter_scope();
    } else {
        loop->finalize();
        delete loop;
        control_stack_.pop();
        control_stack_.pop();
        pc = enddo_pc + 1; //jump to statement following enddo
    }
}

void AbstractControlFlowInterpreter::loop_end(int &pc) {
    int own_pc = pc;  //save pc of enddo instruction
    int own_pc_from_stack = control_stack_.top();
    control_stack_.pop(); //remove own location
    pc = control_stack_.top();
    control_stack_.push(own_pc_from_stack);
    data_manager_.leave_scope();
    bool more_iterations = loop_manager_stack_.top()->update();
    if (more_iterations) {
        data_manager_.enter_scope();
    } else {
        LoopManager* loop = loop_manager_stack_.top();
        loop->finalize();
        loop_manager_stack_.pop(); //remove loop from stack
        int loop_end_pc = control_stack_.top();
        control_stack_.pop(); //pop pc of loop_end
        control_stack_.pop(); //pop pc of first instruction in body
        delete loop;
        pc = loop_end_pc + 1; //address of instruction following the loop
    }
}

bool AbstractControlFlowInterpreter::interpret_where(int num_where_clauses){
    //get address of top of loop and restore control stack
    int loop_end_pc = control_stack_.top();
    control_stack_.pop();
    int loop_body_pc = control_stack_.top();
    control_stack_.push(loop_end_pc);
    pc_ = loop_body_pc;

    //for each where expression (until one is false)
	for (int i = num_where_clauses; i > 0; --i) {
		//evaluate the where expression and leave the value on top of the control stack
		opcode_t opcode = op_table_.opcode(pc_);
		while (opcode != where_op) {
			switch (opcode) {
			case int_load_literal_op: {
				control_stack_.push(arg0(pc_));
				++pc_;
			}
				break;
			case index_load_value_op: {
				control_stack_.push(index_value(arg0(pc_)));
				++pc_;
			}
				break;
			case int_equal_op: {
				int i1 = control_stack_.top();
				control_stack_.pop();
				int i0 = control_stack_.top();
				control_stack_.pop();
				control_stack_.push(i0 == i1);
				++pc_;
			}
				break;
			case int_nequal_op: {
				int i1 = control_stack_.top();
				control_stack_.pop();
				int i0 = control_stack_.top();
				control_stack_.pop();
				control_stack_.push(i0 != i1);
				++pc_;
			}
				break;
			case int_ge_op: {
				int i1 = control_stack_.top();
				control_stack_.pop();
				int i0 = control_stack_.top();
				control_stack_.pop();
				control_stack_.push(i0 >= i1);
				++pc_;
			}
				break;
			case int_le_op: {
				int i1 = control_stack_.top();
				control_stack_.pop();
				int i0 = control_stack_.top();
				control_stack_.pop();
				control_stack_.push(i0 <= i1);
				++pc_;
			}
				break;
			case int_gt_op: {
				int i1 = control_stack_.top();
				control_stack_.pop();
				int i0 = control_stack_.top();
				control_stack_.pop();
				control_stack_.push(i0 > i1);
				++pc_;
			}
				break;
			case int_lt_op: {
				int i1 = control_stack_.top();
				control_stack_.pop();
				int i0 = control_stack_.top();
				control_stack_.pop();
				control_stack_.push(i0 < i1);
				++pc_;
			}
				break;
			case push_block_selector_op: {
				block_selector_stack_.push(sip::BlockSelector(arg0(pc_), arg1(pc_),	index_selectors(pc_)));
				++pc_;
			}
				break;
			case cast_to_int_op: {
				double e = expression_stack_.top();
				expression_stack_.pop();
				int int_val = sial_math::cast_double_to_int(e);
				control_stack_.push(int_val);
				++pc_;
			}
				break;
			case scalar_load_value_op: {
				expression_stack_.push(scalar_value(arg0(pc_)));
				++pc_;
			}
				break;
			case block_load_scalar_op: {
				//This instruction pushes the value of a block with all simple indices onto the instruction stack.
				//If this is a frequent occurrance, we should introduce a new block type for this
				sip::Block::BlockPtr block = get_block_from_selector_stack('r', true);
				expression_stack_.push(block->get_data()[0]);
				++pc_;
			}
				break;
			default:
				fail("unexpected opcode, " + opcodeToName(opcode) + ", in where evaluation ", current_line());
			}
			opcode = op_table_.opcode(pc_);
		}

		//the current instruction is a where
		CHECK_WITH_LINE(opcode == where_op, "expected where_op, actual " + opcodeToName(opcode), line_number());

		//get the value of the expression
		//  std::cout << "handling where (should be >=3): control_stack_.size() = " << control_stack_.size()<< std::endl << std::flush; //DEBUG
		int where_clause_value = control_stack_.top();
		control_stack_.pop();

		//if false, we are done
		if (!where_clause_value) {
			return false;
		} else { //otherwise increment pc and go back to evaluate the next where clause
			++pc_;
		}
	}
    //all the where clauses are true and pc is the next instruction after the where clauses.
    return true;
}


void AbstractControlFlowInterpreter::handle_goto_op(int &pc) {
	pc = arg0(pc);
}

void AbstractControlFlowInterpreter::handle_stop_op(int &pc) {
	fail("sial stop command caused program abort");
}

void AbstractControlFlowInterpreter::handle_jump_if_zero_op(int &pc){
	int i = control_stack_.top();
	control_stack_.pop();
	if (i == 0)
		pc = arg0(pc);
	else
		++pc;
}

void AbstractControlFlowInterpreter::handle_return_op(int &pc) {
	pc = control_stack_.top();
	control_stack_.pop();
}


void AbstractControlFlowInterpreter::handle_do_op(int &pc) {
	int index_slot = index_selectors(pc)[0];
	LoopManager* loop = new DoLoop(index_slot, data_manager_, sip_tables_);
	loop_start(pc, loop);
}

void AbstractControlFlowInterpreter::handle_enddo_op(int &pc) {
	loop_end(pc);
}

void AbstractControlFlowInterpreter::handle_exit_op(int &pc) {
	loop_manager_stack_.top()->set_to_exit();
	pc = control_stack_.top();
}

void AbstractControlFlowInterpreter::handle_where_op(int &pc) {
	int where_clause_value = control_stack_.top();
	control_stack_.pop();
	if (where_clause_value) {
		++pc;
	} else
		pc = control_stack_.top(); //note that this clause must be in a loop and the enddo (or other endloop instruction will pop the value
}

void AbstractControlFlowInterpreter::handle_pardo_op(int &pc){
	/** What a particular worker would do */
//	LoopManager* loop = new StaticTaskAllocParallelPardoLoop(arg1(pc),
//					index_selectors(pc), data_manager_, sip_tables_,
//					worker_rank_, num_workers_);
	int num_where_clauses = arg2(pc);
	int num_indices = arg1(pc);
	LoopManager* loop = new BalancedTaskAllocAbstractControlFlowPardoLoop(num_indices,
				index_selectors(pc), data_manager_, sip_tables_, num_where_clauses,
				this, worker_rank_, num_workers_, iteration_);
	loop_start(pc, loop);
}

void AbstractControlFlowInterpreter::handle_endpardo_op(int &pc) {
	loop_end(pc);
}

void AbstractControlFlowInterpreter::handle_call_op(int &pc) {
	control_stack_.push(pc + 1);
	pc = arg0(pc);
}


void AbstractControlFlowInterpreter::handle_string_load_literal_op(int pc) {
	control_stack_.push(arg0(pc));
}


void AbstractControlFlowInterpreter::handle_int_load_value_op(int pc) {
	control_stack_.push(int_value(arg0(pc)));
}

void AbstractControlFlowInterpreter::handle_int_load_literal_op(int pc) {
	control_stack_.push(arg0(pc));
}

void AbstractControlFlowInterpreter::handle_int_store_op(int pc) {
	int op = arg1(pc);
	if (op == int_store_op) {
		int value = control_stack_.top();
		set_int_value(arg0(pc), value);
		control_stack_.pop();
	} else if (op == int_add_op) {
		int val = int_value(arg0(pc));
		set_int_value(arg0(pc), val + control_stack_.top());
		control_stack_.pop();
	} else if (op == int_subtract_op) {
		int val = int_value(arg0(pc));
		set_int_value(arg0(pc), val - control_stack_.top());
		control_stack_.pop();
	} else if (op == int_multiply_op) {
		int val = int_value(arg0(pc));
		set_int_value(arg0(pc), val * control_stack_.top());
		control_stack_.pop();
	} else
		fail("unexpected operator", line_number());
}

void AbstractControlFlowInterpreter::handle_index_load_value_op(int pc) {
	control_stack_.push(index_value(arg0(pc)));
}

void AbstractControlFlowInterpreter::handle_int_add_op(int pc) {
	int i1 = control_stack_.top();
	control_stack_.pop();
	int i0 = control_stack_.top();
	control_stack_.pop();
	control_stack_.push(i0 + i1);
}

void AbstractControlFlowInterpreter::handle_int_subtract_op(int pc) {
	int i1 = control_stack_.top();
	control_stack_.pop();
	int i0 = control_stack_.top();
	control_stack_.pop();
	control_stack_.push(i0 - i1);
}

void AbstractControlFlowInterpreter::handle_int_multiply_op(int pc) {
	int i1 = control_stack_.top();
	control_stack_.pop();
	int i0 = control_stack_.top();
	control_stack_.pop();
	control_stack_.push(i0 * i1);
}

void AbstractControlFlowInterpreter::handle_int_divide_op(int pc) {
	int i1 = control_stack_.top();
	control_stack_.pop();
	int i0 = control_stack_.top();
	control_stack_.pop();
	control_stack_.push(i0 / i1);
}

void AbstractControlFlowInterpreter::handle_int_equal_op(int pc) {
	int i1 = control_stack_.top();
	control_stack_.pop();
	int i0 = control_stack_.top();
	control_stack_.pop();
	control_stack_.push(i0 == i1);
}

void AbstractControlFlowInterpreter::handle_int_nequal_op(int pc) {
	int i1 = control_stack_.top();
	control_stack_.pop();
	int i0 = control_stack_.top();
	control_stack_.pop();
	control_stack_.push(i0 != i1);
}

void AbstractControlFlowInterpreter::handle_int_ge_op(int pc) {
	int i1 = control_stack_.top();
	control_stack_.pop();
	int i0 = control_stack_.top();
	control_stack_.pop();
	control_stack_.push(i0 >= i1);
}

void AbstractControlFlowInterpreter::handle_int_le_op(int pc) {
	int i1 = control_stack_.top();
	control_stack_.pop();
	int i0 = control_stack_.top();
	control_stack_.pop();
	control_stack_.push(i0 <= i1);
}

void AbstractControlFlowInterpreter::handle_int_gt_op(int pc) {
	int i1 = control_stack_.top();
	control_stack_.pop();
	int i0 = control_stack_.top();
	control_stack_.pop();
	control_stack_.push(i0 > i1);
}

void AbstractControlFlowInterpreter::handle_int_lt_op(int pc) {
	int i1 = control_stack_.top();
	control_stack_.pop();
	int i0 = control_stack_.top();
	control_stack_.pop();
	control_stack_.push(i0 < i1);
}

void AbstractControlFlowInterpreter::handle_int_neg_op(int pc) {
	int i = control_stack_.top();
	control_stack_.pop();
	control_stack_.push(-i);
}

void AbstractControlFlowInterpreter::handle_cast_to_int_op(int pc) {
	double e = expression_stack_.top();
	expression_stack_.pop();
	int int_val = sial_math::cast_double_to_int(e);
	control_stack_.push(int_val);
}

void AbstractControlFlowInterpreter::handle_scalar_load_value_op(int pc) {
	expression_stack_.push(scalar_value(arg0(pc)));
}

void AbstractControlFlowInterpreter::handle_scalar_store_op(int pc) {
	int op = arg1(pc);
	if (op == scalar_store_op) {
		set_scalar_value(arg0(pc), expression_stack_.top());
		expression_stack_.pop();
	} else if (op == scalar_add_op) {
		double val = scalar_value(arg0(pc));
		set_scalar_value(arg0(pc), val + expression_stack_.top());
		expression_stack_.pop();
	} else if (op == scalar_subtract_op) {
		double val = scalar_value(arg0(pc));
		set_scalar_value(arg0(pc), val - expression_stack_.top());
		expression_stack_.pop();
	} else if (op == scalar_multiply_op) {
		double val = scalar_value(arg0(pc));
		set_scalar_value(arg0(pc), val * expression_stack_.top());
		expression_stack_.pop();
	} else
		fail("unexpected operator");
}

void AbstractControlFlowInterpreter::handle_scalar_add_op(int pc) {
	double e1 = expression_stack_.top();
	expression_stack_.pop();
	double e0 = expression_stack_.top();
	expression_stack_.pop();
	expression_stack_.push(e0 + e1);
	++pc;
}

void AbstractControlFlowInterpreter::handle_scalar_subtract_op(int pc) {
	double e1 = expression_stack_.top();
	expression_stack_.pop();
	double e0 = expression_stack_.top();
	expression_stack_.pop();
	expression_stack_.push(e0 - e1);
}

void AbstractControlFlowInterpreter::handle_scalar_multiply_op(int pc) {
	double e1 = expression_stack_.top();
	expression_stack_.pop();
	double e0 = expression_stack_.top();
	expression_stack_.pop();
	expression_stack_.push(e0 * e1);
}

void AbstractControlFlowInterpreter::handle_scalar_divide_op(int pc) {
	double e1 = expression_stack_.top();
	expression_stack_.pop();
	double e0 = expression_stack_.top();
	expression_stack_.pop();
	expression_stack_.push(e0 / e1);
}

void AbstractControlFlowInterpreter::handle_scalar_exp_op(int pc) {
	double e1 = expression_stack_.top();
	expression_stack_.pop();
	double e0 = expression_stack_.top();
	expression_stack_.pop();
	double value = sial_math::pow(e0, e1);
	expression_stack_.push(value);
}

void AbstractControlFlowInterpreter::handle_scalar_eq_op(int pc) {
	double e1 = expression_stack_.top();
	expression_stack_.pop();
	double e0 = expression_stack_.top();
	expression_stack_.pop();
	int value = (e0 == e1);
	control_stack_.push(value);
}

void AbstractControlFlowInterpreter::handle_scalar_ne_op(int pc) {
	double e1 = expression_stack_.top();
	expression_stack_.pop();
	double e0 = expression_stack_.top();
	expression_stack_.pop();
	int value = (e0 != e1);
	control_stack_.push(value);
}

void AbstractControlFlowInterpreter::handle_scalar_ge_op(int pc) {
	double e1 = expression_stack_.top();
	expression_stack_.pop();
	double e0 = expression_stack_.top();
	expression_stack_.pop();
	int value = (e0 >= e1);
	control_stack_.push(value);
}

void AbstractControlFlowInterpreter::handle_scalar_le_op(int pc) {
	double e1 = expression_stack_.top();
	expression_stack_.pop();
	double e0 = expression_stack_.top();
	expression_stack_.pop();
	int value = (e0 <= e1);
	control_stack_.push(value);
}

void AbstractControlFlowInterpreter::handle_scalar_gt_op(int pc) {
	double e1 = expression_stack_.top();
	expression_stack_.pop();
	double e0 = expression_stack_.top();
	expression_stack_.pop();
	int value = (e0 > e1);
	control_stack_.push(value);
}

void AbstractControlFlowInterpreter::handle_scalar_lt_op(int pc) {
	double e1 = expression_stack_.top();
	expression_stack_.pop();
	double e0 = expression_stack_.top();
	expression_stack_.pop();
	int value = (e0 < e1);
	control_stack_.push(value);
}

void AbstractControlFlowInterpreter::handle_scalar_neg_op(int pc) {
	double e = expression_stack_.top();
	expression_stack_.pop();
	expression_stack_.push(-e);
}

void AbstractControlFlowInterpreter::handle_scalar_sqrt_op(int pc) {
	double e = expression_stack_.top();
	expression_stack_.pop();
	double value = sial_math::sqrt(e);
	expression_stack_.push(value);
}

void AbstractControlFlowInterpreter::handle_cast_to_scalar_op(int pc) {
	int i = control_stack_.top();
	control_stack_.pop();
	expression_stack_.push(double(i));
}


void AbstractControlFlowInterpreter::handle_idup_op(int pc) {
	control_stack_.push(control_stack_.top());
}

void AbstractControlFlowInterpreter::handle_iswap_op(int pc) {
	int i1 = control_stack_.top();
	control_stack_.pop();
	int i0 = control_stack_.top();
	control_stack_.pop();
	control_stack_.push(i1);
	control_stack_.push(i0);
}

void AbstractControlFlowInterpreter::handle_sswap_op(int pc) {
	double e1 = expression_stack_.top();
	expression_stack_.pop();
	double e0 = expression_stack_.top();
	expression_stack_.pop();
	expression_stack_.push(e1);
	expression_stack_.push(e0);
}

} /* namespace sip */
