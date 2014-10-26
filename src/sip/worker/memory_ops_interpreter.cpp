/*
 * memory_ops_interpreter.cpp
 *
 *  Created on: Oct 23, 2014
 *      Author: njindal
 */

#include "memory_ops_interpreter.h"

#ifdef HAVE_TAU
#include "TAU.h"
#endif

namespace sip {

MemoryOpsInterpreter::MemoryOpsInterpreter(const SipTables& sipTables, SialxTimer* sialx_timer):
	SialxInterpreter(sipTables, sialx_timer, NULL, NULL){
}

MemoryOpsInterpreter::~MemoryOpsInterpreter() {}

void MemoryOpsInterpreter::pre_interpret(int pc){
#ifdef HAVE_TAU
    TAU_TRACK_MEMORY_HERE();
#endif
    SialxInterpreter::pre_interpret(pc);
}

void MemoryOpsInterpreter::handle_execute_op(int pc){
	int num_args = arg1(pc);
	int func_slot = arg0(pc);

	// Do the super instruction if the number of args is > 1
	// and if the parameters are only written to (all 'w')
	// and if all the arguments are either static or scalar
	if (num_args >= 1){
		bool all_write = true;
		const std::string signature(sip_tables_.special_instruction_manager().get_signature(func_slot));
		for (int i=0; i<signature.size(); i++){
			if (signature[i] != 'w'){
				all_write = false;
				break;
			}
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

		// Push back blocks on stack and execute the super instruction.
		while (!bs_list.empty()){
			BlockSelector bs = bs_list.back();
			block_selector_stack_.push(bs);
			bs_list.pop_back();
		}
		if (all_write && all_scalar_or_static){ // Do the super instruction
			std::cout << "Executing Super Instruction "
					<< sip_tables_.special_instruction_manager().name(func_slot)
					<< " at line " << line_number() << std::endl;
			SialxInterpreter::handle_execute_op(pc);
		} else { // Just emulate block allocation & deallocation operations
			for (int i=0; i<num_args; ++i){
				BlockId block_id;
				char intent = signature[i];
				Block::BlockPtr block = get_block_from_selector_stack(intent,	block_id, true);
			}
		}
	}
	contiguous_blocks_post_op();
}

void MemoryOpsInterpreter::handle_get_op(int pc){
	sip::BlockId id = get_block_id_from_selector_stack();
	Block::BlockPtr block = data_manager_.block_manager_.get_block_for_writing(id, true);
}

void MemoryOpsInterpreter::handle_put_accumulate_op(int pc){
	sip::Block::BlockPtr rhs_block = get_block_from_selector_stack('r', true);
	sip::BlockId lhs_id = get_block_id_from_selector_stack();
}

void MemoryOpsInterpreter::handle_put_replace_op(int pc){
	sip::Block::BlockPtr rhs_block = get_block_from_selector_stack('r', true);
	sip::BlockId lhs_id = get_block_id_from_selector_stack();
}

void MemoryOpsInterpreter::handle_delete_op(int pc) {
	data_manager_.block_manager_.delete_per_array_map_and_blocks(arg0(pc));
}

void MemoryOpsInterpreter::handle_block_copy_op(int pc){
	sip::Block::BlockPtr rhs_block = get_block_from_selector_stack('r', true);
	sip::Block::BlockPtr lhs_block = get_block_from_instruction(pc, 'w', true);
}

void MemoryOpsInterpreter::handle_block_permute_op(int pc){
	sip::Block::BlockPtr lhs_block = get_block_from_selector_stack('w', true);
	sip::Block::BlockPtr rhs_block = get_block_from_selector_stack('r', true); //TODO, check contiguous allowed.
}

void MemoryOpsInterpreter::handle_block_fill_op(int pc){
	sip::Block::BlockPtr lhs_block = get_block_from_instruction(pc, 'w', true);
	expression_stack_.pop();
}

void MemoryOpsInterpreter::handle_block_scale_op(int pc){
	sip::Block::BlockPtr lhs_block = get_block_from_instruction(pc, 'u', true);
	expression_stack_.pop();
}

void MemoryOpsInterpreter::handle_block_scale_assign_op(int pc) {
	Block::BlockPtr lhs_block = get_block_from_instruction(pc, 'w', true);
	Block::BlockPtr rhs_block = get_block_from_selector_stack('r', true);
	expression_stack_.pop();
}

void MemoryOpsInterpreter::handle_block_accumulate_scalar_op(int pc){
	sip::Block::BlockPtr lhs_block = get_block_from_instruction(pc, 'u', true);
	expression_stack_.pop();
}

void MemoryOpsInterpreter::handle_block_add_op(int pc){
	BlockId did, lid, rid;
	Block::BlockPtr rblock = get_block_from_selector_stack('r', rid, true);
	Block::BlockPtr lblock = get_block_from_selector_stack('r', lid, true);
	Block::BlockPtr dblock = get_block_from_instruction(pc, 'w',  true);
}

void MemoryOpsInterpreter::handle_block_subtract_op(int pc){
	BlockId did, lid, rid;
	Block::BlockPtr rblock = get_block_from_selector_stack('r', rid, true);
	Block::BlockPtr lblock = get_block_from_selector_stack('r', lid, true);
	Block::BlockPtr dblock = get_block_from_instruction(pc, 'w',  true);
}

void MemoryOpsInterpreter::handle_block_contract_op(int pc){
	BlockId lid, rid;
	Block::BlockPtr dblock = get_block_from_instruction(pc, 'w', true);
	Block::BlockPtr rblock = get_block_from_selector_stack('r', rid, true);
	Block::BlockPtr lblock = get_block_from_selector_stack('r', lid, true);
}

void MemoryOpsInterpreter::handle_block_contract_to_scalar_op(int pc){
	BlockId lid, rid;
	Block::BlockPtr rblock = get_block_from_selector_stack('r', rid, true);
	Block::BlockPtr lblock = get_block_from_selector_stack('r', lid, true);
	expression_stack_.push(0.0);
}

void MemoryOpsInterpreter::handle_block_load_scalar_op(int pc){
	BlockSelector &bs = block_selector_stack_.top();
	if (sip_tables_.is_contiguous(bs.array_id_)){
		SialxInterpreter::handle_block_load_scalar_op(pc);
	} else {
		sip::Block::BlockPtr block = get_block_from_selector_stack('r', true);
		expression_stack_.push(0.0);
	}
}

} /* namespace sip */
