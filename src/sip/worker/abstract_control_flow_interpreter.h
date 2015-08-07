/*
 * abstract_control_flow_interpreter.h
 *
 *  Created on: Jun 16, 2015
 *      Author: njindal
 */

#ifndef SIP_WORKER_ABSTRACT_CONTROL_FLOW_INTERPRETER_H_
#define SIP_WORKER_ABSTRACT_CONTROL_FLOW_INTERPRETER_H_

#include <sstream>
#include "config.h"
#include "interpreter.h"
#include "block_manager.h"
#include "contiguous_array_manager.h"
#include "data_manager.h"
#include "sip_tables.h"
#include "loop_manager.h"
#include "cached_block_map.h"


namespace sip {

/**
 * Abstract Class for other interpreters to inherit from.
 * Interprets the optable, only doing scalar, index & int operations.
 * For other operations, makes sure that the stacks are correctly operated on.
 */
class AbstractControlFlowInterpreter: public Interpreter {
public:
	AbstractControlFlowInterpreter(int worker_rank, int num_workers, const SipTables& sipTables);
	virtual ~AbstractControlFlowInterpreter();

	virtual void do_interpret();

	virtual void do_post_sial_program() = 0;

	/**
	 * @return Line number currently being interpreted. -1 if pc is out of range.
	 */
	virtual int get_line_number() {
		if (pc_ < op_table_.size())
			return op_table_.line_number(pc_);
		else
			return -1;// Past the end of the program. Probably being called by a test.
	}

	/**
	 * @return Currently being interpreted program counter or -1 if past the end of the program
	 */
	virtual int get_pc() { return pc_ < op_table_.size() ? pc_ : -1; }

	virtual SialPrinter* get_printer() = 0;

	virtual double scalar_value_impl(const std::string& name) { return data_manager_.scalar_value(name); }
	virtual void set_scalar_value_impl(const std::string& name, double value) { data_manager_.set_scalar_value(name, value); }
	virtual double predefined_scalar_impl(const std::string& name) { return sip_tables().setup_reader().predefined_scalar(name); }
	virtual std::string index_value_to_string_impl(int index_table_slot) {
		std::stringstream ss;
		ss << sip_tables_.index_name(index_table_slot) << '=' << index_value(index_table_slot);
		return ss.str();
	}
	virtual int predefined_int_impl (const std::string& name) { return sip_tables().setup_reader().predefined_int(name); }
	virtual setup::PredefContigArray predefined_contiguous_array_impl(const std::string& name){ return sip_tables().setup_reader().predefined_contiguous_array(name); }
	virtual setup::PredefIntArray predefined_integer_array_impl(const std::string& name) { return sip_tables().setup_reader().predefined_integer_array(name); }
	virtual int num_subsegments_impl(int subindex_slot, int super_index_value) { return sip_tables_.num_subsegments(subindex_slot, super_index_value); }
	virtual int segment_extent_impl(int index_slot, int segment) { return sip_tables_.index_table_.segment_extent(index_slot, segment); }
	virtual Block* get_and_remove_contiguous_array_impl(int array_id) { return data_manager_.contiguous_array_manager_.get_and_remove_array(array_id); }
	virtual void set_contiguous_array_impl(int array_id, Block* contig){ data_manager_.contiguous_array_manager_.insert_contiguous_array(array_id, contig); }
	virtual IdBlockMap<Block>::PerArrayMap* get_and_remove_per_array_map_impl(int array_id){ return data_manager_.block_manager_.block_map_.get_and_remove_per_array_map(array_id); }
	virtual void set_per_array_map_impl(int array_id, IdBlockMap<Block>::PerArrayMap* map_ptr) { data_manager_.block_manager_.block_map_.insert_per_array_map(array_id, map_ptr); }
	virtual std::string array_name_impl(int array_table_slot){ return sip_tables_.array_name(array_table_slot); }
	virtual int index_value_impl(int index_table_slot) { return data_manager_.index_value(index_table_slot); }
	virtual std::string string_literal_impl(int slot) { return sip_tables_.string_literal(slot); }
	virtual double scalar_value_impl(int array_table_slot) { return data_manager_.scalar_value(array_table_slot); }
	virtual void set_scalar_value_impl(int array_table_slot, double value) { data_manager_.set_scalar_value(array_table_slot, value); }
	virtual bool is_scalar_impl(int array_table_slot) { return sip_tables_.is_scalar(array_table_slot); }
	virtual bool is_contiguous_impl(int array_table_slot) { return sip_tables_.is_contiguous(array_table_slot); }
//	Block::BlockPtr get_block_for_reading(BlockId& id){return data_manager_.block_manager_.get_block_for_reading(id);}
	virtual bool is_distributed_or_served_impl(int array_slot) { return sip_tables_.is_distributed(array_slot) || sip_tables_.is_served(array_slot); }
	virtual int int_value_impl(int int_table_slot) { return data_manager_.int_table_.value(int_table_slot); }
	virtual int int_value_impl(const std::string& name) { return data_manager_.int_value(name); }
	virtual void set_int_value_impl(const std::string& name, int value){ data_manager_.set_int_value(name, value); }
	virtual void set_int_value_impl(int int_table_slot, int value){ data_manager_.set_int_value(int_table_slot, value); }


	// --------------------------------------------------------------
	//                     Methods for Testing
	// --------------------------------------------------------------

	virtual Block* get_static(int array_id){ return data_manager_.contiguous_array_manager_.get_array(array_id); }
	virtual int array_slot(const std::string& array_name) { return sip_tables_.array_table_.array_slot(array_name); }
	virtual Block::BlockPtr get_block_for_reading(const BlockId& block_id){ return data_manager_.block_manager_.get_block_for_reading(block_id); }
	virtual bool all_stacks_empty(){
		return loop_manager_stack_.size()==0
				&& block_selector_stack_.size()==0 && control_stack_.size()==0
				&& expression_stack_.size()==0
				&& write_back_list_.size()==0
				&& read_block_list_.size()==0;
	}
	virtual std::size_t num_blocks_in_blockmap(){ return data_manager_.block_manager_.total_blocks(); }

	// --------------------------------------------------------------
	//               End of Methods for Testing
	// --------------------------------------------------------------


	// Convenience methods
	const DataManager& data_manager() const { return data_manager_; }
	const SipTables& sip_tables() const { return sip_tables_; }

	bool interpret_where(int num_where_clauses);

protected:

	const int num_workers_;		/*! Total number of workers to emulate */
	const int worker_rank_;		/*! Worker to emulate */

	int pc_; 						/*! the "program counter". Actually, the current location in the op_table_.	 */
	const SipTables& sip_tables_; 	/*! static data */
	const OpTable & op_table_;  	/*! owned by sipTables_, pointer copied for convenience */
	DataManager data_manager_;    	/*! dynamic data */

	long iteration_;
	std::stack<LoopManager*> loop_manager_stack_;
	std::stack<BlockSelector> block_selector_stack_;
	std::stack<int> control_stack_;
	std::stack<double> expression_stack_;
	WriteBackList write_back_list_;
	ReadBlockList read_block_list_;

	// Convenience methods for getting data from the current pc's optable entry
	int arg0(int pc){ return op_table_.arg0(pc); }
	int arg1(int pc){ return op_table_.arg1(pc); }
	int arg2(int pc){ return op_table_.arg2(pc); }
	const index_selector_t& index_selectors(int pc){return op_table_.index_selectors(pc);}
	BlockId block_id(const sip::BlockSelector& selector) { return data_manager_.block_id(selector); }


	void do_interpret(int pc_start, int pc_end);

	void handle_user_sub_op(int pc);
	void contiguous_blocks_post_op();


	/**The next set of routines are helper routines in the interpreter whose function should be obvious from the name */
	void loop_start(int &pc, LoopManager * loop);
	void loop_end(int &pc);



	BlockId get_block_id_from_instruction(int pc);
	BlockId get_contiguous_local_id_from_selector_stack();
	BlockId get_block_id_from_selector_stack();
	BlockId get_block_id_from_selector(const BlockSelector& selector);
	Block::BlockPtr get_block_from_selector_stack(char intent, BlockId&, bool contiguous_allowed = true);
	Block::BlockPtr get_block_from_selector_stack(char intent, bool contiguous_allowed = true);
	Block::BlockPtr get_block_from_instruction(int pc, char intent, bool contiguous_allowed = true);
	Block::BlockPtr get_block(char intent, sip::BlockSelector&, sip::BlockId&, bool contiguous_allowed = true);



	// ================================================================
	//		Methods to be overriden by abstract interpreters
	//		READ THE IMPLEMENTATION BEFORE OVERRIDING
	// ================================================================


	// The following functions handle each operation in the
	// optable of a compiled sialx program. There are 2 sets
	// of super instructions - ones that modify the program counter
	// on account of being jump or procedure calls and others
	// that dont. The ones that cause the program counter to
	// change take a reference to the program counter which is to
	// be changed.

	// Functions that modify the program counter
	// are passed in a reference to it.
	virtual void handle_goto_op(int &pc);
	virtual void handle_jump_if_zero_op(int &pc);
	virtual void handle_stop_op(int &pc);
	virtual void handle_return_op(int &pc);
	virtual void handle_do_op(int &pc);
	virtual void handle_enddo_op(int &pc);
	virtual void handle_exit_op(int &pc);
	virtual void handle_where_op(int &pc);
	virtual void handle_pardo_op(int &pc);
	virtual void handle_endpardo_op(int &pc);
	virtual void handle_call_op(int &pc);

	// Functions that calculate scalar and integer values & modify the stack
	virtual void handle_execute_op(int pc);
	virtual void handle_string_load_literal_op(int pc);
	virtual void handle_int_load_value_op(int pc);
	virtual void handle_int_load_literal_op(int pc);
	virtual void handle_int_store_op(int pc);
	virtual void handle_index_load_value_op(int pc);
	virtual void handle_int_add_op(int pc);
	virtual void handle_int_subtract_op(int pc);
	virtual void handle_int_multiply_op(int pc);
	virtual void handle_int_divide_op(int pc);
	virtual void handle_int_equal_op(int pc);
	virtual void handle_int_nequal_op(int pc);
	virtual void handle_int_ge_op(int pc);
	virtual void handle_int_le_op(int pc);
	virtual void handle_int_gt_op(int pc);
	virtual void handle_int_lt_op(int pc);
	virtual void handle_int_neg_op(int pc);
	virtual void handle_cast_to_int_op(int pc);
	virtual void handle_cast_to_scalar_op(int pc);
	virtual void handle_scalar_load_value_op(int pc);
	virtual void handle_scalar_store_op(int pc);
	virtual void handle_scalar_add_op(int pc);
	virtual void handle_scalar_subtract_op(int pc);
	virtual void handle_scalar_multiply_op(int pc);
	virtual void handle_scalar_divide_op(int pc);
	virtual void handle_scalar_exp_op(int pc);
	virtual void handle_scalar_eq_op(int pc);
	virtual void handle_scalar_ne_op(int pc);
	virtual void handle_scalar_ge_op(int pc);
	virtual void handle_scalar_le_op(int pc);
	virtual void handle_scalar_gt_op(int pc);
	virtual void handle_scalar_lt_op(int pc);
	virtual void handle_scalar_neg_op(int pc);
	virtual void handle_scalar_sqrt_op(int pc);
	virtual void handle_idup_op(int pc);
	virtual void handle_iswap_op(int pc);
	virtual void handle_sswap_op(int pc);



	void blk_instr() {
		get_block_id_from_instruction(pc_);
	}

	void blk_sstack(){
		get_block_id_from_selector_stack();
	}


	// Methods for block operations and misc operations
	// An implementation that does the correct thing with
	// the stacks is provided.
	virtual void handle_broadcast_static_op(int pc) { 	control_stack_.pop(); }
	virtual void handle_allocate_op(int pc) {}
	virtual void handle_deallocate_op(int pc) {}
	virtual void handle_allocate_contiguous_op(int pc) { get_block_id_from_instruction(pc);}
	virtual void handle_deallocate_contiguous_op(int pc) { get_block_id_from_instruction(pc);}
	virtual void handle_collective_sum_op(int pc) {  expression_stack_.pop(); }
	virtual void handle_assert_same_op(int pc) { /* TODO Time for MPI_BCast */ }
	virtual void handle_print_string_op(int pc) { control_stack_.pop();}
	virtual void handle_print_scalar_op(int pc) {  expression_stack_.pop();}
	virtual void handle_print_int_op(int pc) { control_stack_.pop();}
	virtual void handle_print_index_op(int pc) { control_stack_.pop();}
	virtual void handle_print_block_op(int pc) { blk_sstack(); }
	virtual void handle_println_op(int pc) {}
	virtual void handle_gpu_on_op(int pc) {}
	virtual void handle_gpu_off_op(int pc) {}
	virtual void handle_set_persistent_op(int pc) {}
	virtual void handle_restore_persistent_op(int pc) {}

	virtual void handle_block_copy_op(int pc) 		{ blk_instr(); blk_sstack(); }
	virtual void handle_block_permute_op(int pc) 	{ blk_sstack(); blk_sstack();}
	virtual void handle_block_fill_op(int pc) 		{ blk_instr(); expression_stack_.pop(); }
	virtual void handle_block_scale_op(int pc) 		{ blk_instr(); expression_stack_.pop(); }
	virtual void handle_block_scale_assign_op(int pc){ blk_instr(); blk_sstack(); expression_stack_.pop();}
	virtual void handle_block_accumulate_scalar_op(int pc){ blk_instr(); expression_stack_.pop(); }
	virtual void handle_block_add_op(int pc)		{ blk_instr(); blk_sstack(); blk_sstack();}
	virtual void handle_block_subtract_op(int pc)	{ blk_instr(); blk_sstack(); blk_sstack();}
	virtual void handle_block_contract_op(int pc)	{ blk_instr(); blk_sstack(); blk_sstack();}
	virtual void handle_block_contract_to_scalar_op(int pc) { blk_sstack(); blk_sstack(); expression_stack_.push(0.0);}
	virtual void handle_block_load_scalar_op(int pc){ blk_sstack(); expression_stack_.push(0.0);}
	virtual void handle_push_block_selector_op(int pc) { block_selector_stack_.push(BlockSelector(arg0(pc), arg1(pc), index_selectors(pc)));}

	// Methods that potentially cause a communication with the server
	virtual void handle_sip_barrier_op(int pc) {};

	virtual void handle_get_op(int pc) {blk_sstack();}
	virtual void handle_put_accumulate_op(int pc) 	{block_selector_stack_.pop(); blk_sstack();}
	virtual void handle_put_replace_op(int pc) 		{block_selector_stack_.pop(); blk_sstack();}
	virtual void handle_put_initialize_op(int pc) 	{blk_sstack(); expression_stack_.pop(); }
	virtual void handle_put_increment_op(int pc)  	{blk_sstack(); expression_stack_.pop(); }
	virtual void handle_put_scale_op(int pc) 	  	{blk_sstack(); expression_stack_.pop(); }

	virtual void handle_create_op(int pc) {};
	virtual void handle_delete_op(int pc) {};
	virtual void handle_program_end(int &pc) {}

	/**
	 * Steps to be performed after interpreting opcode at old_pc.
	 * @param old_pc
	 * @param new_pc program counter changed to new pc as a result of interpreting old_pc
	 */
	virtual void post_interpret(int old_pc, int new_pc) = 0;

	/**
	 * Steps to be peformed before interpreting opcode at pc.
	 * The timer trace happens here.
	 * Override this method with a blank implementation to do nothing.
	 * @param pc
	 */
	virtual void pre_interpret(int pc) = 0;


	// ================================================================
	// End of Methods to be overriden / implemented by abstract interpreters
	// ================================================================


};

} /* namespace sip */

#endif /* SIP_WORKER_ABSTRACT_CONTROL_FLOW_INTERPRETER_H_ */
