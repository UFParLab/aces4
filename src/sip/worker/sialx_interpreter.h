/*
 * interpreter.h
 *
 * This is the SIAL interpreter.
 *
 * The global_interpreter is a static global pointer to the current interpreter instance.  This is needed
 * for super instructions written in fortran.
 *
 * The interpreter contains pointers to the SipTable and DataManager, which contain the static and dynamic data
 * for the current program, respectively.  A set of inline functions is given that simply provide convenient
 * access to the contents of these data structures.
 *
 *
 *
 *      Author: Beverly Sanders
 */

#ifndef SIALX_INTERPRETER_H_
#define SIALX_INTERPRETER_H_

#include <string>
#include <stack>
#include <set>
#include <sstream>

#include "config.h"
#include "block_manager.h"
#include "contiguous_array_manager.h"
#include "data_manager.h"
#include "sialx_timer.h"
#include "worker_persistent_array_manager.h"
#include "sial_math.h"
#include "interpreter.h"


#ifdef HAVE_MPI
#include "sial_ops_parallel.h"
#else
#include "sial_ops_sequential.h"
#endif //HAVE_MPI

class TestControllerParallel;
class TestController;

namespace sip {

class LoopManager;
class SialPrinter;
class BalancedTaskAllocParallelPardoLoop;

/**
 * Interprets Sialx code.
 */
class SialxInterpreter:public Interpreter {
public:


	SialxInterpreter(const SipTables&, SialxTimer* timers, SialPrinter* printer, WorkerPersistentArrayManager* wpm);
	SialxInterpreter(const SipTables&, SialxTimer* timers, SialPrinter* printer);
	virtual ~SialxInterpreter();


	/** main interpret function */
	virtual void do_interpret();

	/**
	 *  Should be called after the main interpret loop terminates to
	 *  cleanly terminate this program.  This is the responsibility of
	 *  the main program to allow more convenient testing of parallel
	 *  programs that don't use servers.
	 */
	virtual void do_post_sial_program();

	/**
	 * Returns the line number in the SIAL program corresponding to the
	 * instruction at op_table[pc].
	 * @return -1 if pc is out of range of the op_table.
	 */
	virtual int get_line_number() {
		if (pc_ < op_table_.size())
			return op_table_.line_number(pc_);
		else
			return -1;// Past the end of the program. Probably being called by a test.
	}

	/**
	 * @return the currently executing pc_, -1 if past the end of program
	 */
	virtual int get_pc() {
		return pc_ < op_table_.size() ? pc_ : -1;
	}

	/** Allows superinstructions to use the same ostream as sial. */
	virtual SialPrinter* get_printer(){
		return printer_;
	}


    /** Called by BalancedTaskAllocParallelPardoLoop to correctly schedule tasks */
    bool interpret_where(int num_where_clauses);


	virtual double scalar_value_impl(const std::string& name) {
		return data_manager_.scalar_value(name);
	}

	virtual void set_scalar_value_impl(const std::string& name, double value) {
		data_manager_.set_scalar_value(name, value);
	}

	virtual double predefined_scalar_impl(const std::string& name) {
		return sip_tables().setup_reader().predefined_scalar(name);
	}

	virtual std::string index_value_to_string_impl(int index_table_slot) {
		std::stringstream ss;
		ss << sip_tables_.index_name(index_table_slot) << '='
				<< index_value(index_table_slot);
		return ss.str();
	}

	virtual int predefined_int_impl (const std::string& name) {
		return sip_tables().setup_reader().predefined_int(name);
	}

	virtual setup::PredefContigArray predefined_contiguous_array_impl(const std::string& name){
		return sip_tables().setup_reader().predefined_contiguous_array(name);
	}

	virtual setup::PredefIntArray predefined_integer_array_impl(const std::string& name) {
		return sip_tables().setup_reader().predefined_integer_array(name);
	}

	virtual int num_subsegments_impl(int subindex_slot, int super_index_value) {
		return sip_tables_.num_subsegments(subindex_slot, super_index_value);
	}
	virtual int segment_extent_impl(int index_slot, int segment) {
		return sip_tables_.index_table_.segment_extent(index_slot, segment);
	}

	virtual Block* get_and_remove_contiguous_array_impl(int array_id) {
		return data_manager_.contiguous_array_manager_.get_and_remove_array(array_id);
	}
	virtual void set_contiguous_array_impl(int array_id, Block* contig){
		data_manager_.contiguous_array_manager_.insert_contiguous_array(array_id, contig);
	}

	virtual IdBlockMap<Block>::PerArrayMap* get_and_remove_per_array_map_impl(int array_id){
		return data_manager_.block_manager_.block_map_.get_and_remove_per_array_map(array_id);
	}

	virtual void set_per_array_map_impl(int array_id, IdBlockMap<Block>::PerArrayMap* map_ptr) {
		data_manager_.block_manager_.block_map_.insert_per_array_map(array_id, map_ptr);
	}

	virtual std::string array_name_impl(int array_table_slot){
		return sip_tables_.array_name(array_table_slot);
	}

	virtual int index_value_impl(int index_table_slot) {
		return data_manager_.index_value(index_table_slot);
	}

	virtual std::string string_literal_impl(int slot) {
		return sip_tables_.string_literal(slot);
	}
	virtual double scalar_value_impl(int array_table_slot) {
		return data_manager_.scalar_value(array_table_slot);
	}

	virtual void set_scalar_value_impl(int array_table_slot, double value) {
		data_manager_.set_scalar_value(array_table_slot, value);
	}

	virtual bool is_scalar_impl(int array_table_slot) {
		return sip_tables_.is_scalar(array_table_slot);
	}

	virtual bool is_contiguous_impl(int array_table_slot) {
			return sip_tables_.is_contiguous(array_table_slot);
	}
//	sip::Block::BlockPtr get_block_for_reading(sip::BlockId& id){return data_manager_.block_manager_.get_block_for_reading(id);}
	virtual bool is_distributed_or_served_impl(int array_slot) {
		return sip_tables_.is_distributed(array_slot) || sip_tables_.is_served(array_slot);
	}

	virtual int int_value_impl(int int_table_slot) {
		return data_manager_.int_table_.value(int_table_slot);
	}

	virtual int int_value_impl(const std::string& name) {
		return data_manager_.int_value(name);
	}

	virtual void set_int_value_impl(const std::string& name, int value){
		data_manager_.set_int_value(name, value);
	}

	virtual void set_int_value_impl(int int_table_slot, int value){
		data_manager_.set_int_value(int_table_slot, value);
	}


	/**
	 * For testing.
	 * @param array_id
	 * @return
	 */
	virtual Block* get_static(int array_id){
		return data_manager_.contiguous_array_manager_.get_array(array_id);
	}

	/**
	 * For testing
	 * @param array_name
	 * @return
	 */
	virtual int array_slot(const std::string& array_name) {
		return sip_tables_.array_table_.array_slot(array_name);
	}


	/**
	 * For testing
	 * @param block_id
	 * @return
	 */
	virtual Block::BlockPtr get_block_for_reading(const BlockId& block_id){
		return data_manager_.block_manager_.get_block_for_reading(block_id);
	}


	/** For testing.
	 * Determine whether any data is left in the interpreter's data structures.
	 * This method should return true immediately after completion of a sial program.
	 * @return
	 */
	virtual bool all_stacks_empty(){
		return loop_manager_stack_.size()==0
				&& block_selector_stack_.size()==0 && control_stack_.size()==0
				&& expression_stack_.size()==0
				&& write_back_list_.size()==0
				&& read_block_list_.size()==0;
	}

	/** For testing12
	 * Determines the total number of blocks in the block map, including both the
	 * cache and the active map.  This means that the value may be non-deterministic,
	 * or at least difficult to predict, except at certain points where it is know to
	 * be zero.
	 * @return
	 */
	virtual std::size_t num_blocks_in_blockmap(){
    	return data_manager_.block_manager_.total_blocks();
    }



	// Methods for convenience

	void set_index_value(int index_table_slot, int value) {
		data_manager_.set_index_value(index_table_slot, value);
	}
	bool is_subindex(int index_table_slot) {
		return sip_tables_.is_subindex(index_table_slot);
	}
	//TODO fix inconsistent name.  parent_index or super_index??
	int super_index(int subindex_slot) {
		return sip_tables_.parent_index(subindex_slot);
	}

	sip::BlockId block_id(const sip::BlockSelector& selector) {
		return data_manager_.block_id(selector);
	}
	std::string int_name(int int_table_slot){
		return sip_tables_.int_name(int_table_slot);
	}
	std::string index_name(int index_table_slot){
		return sip_tables_.index_name(index_table_slot);
	}


	/**
	 * Helper method to get line number for a given pc
	 * @param pc
	 * @return
	 */
	int get_line_number(int pc){
		if (pc < op_table_.size())
			return op_table_.line_number(pc);
		else
			return -1;// Past the end of the program. Probably being called by a test.
	}

	/**
	 * Helper method to get a set of PCs for a given line number
	 * @param line
	 * @return
	 */
	std::set<int> line_to_pc_set(int line){

		for (LineToPCMap::iterator it = line_to_pc_map_.begin(); it!= line_to_pc_map_.end(); ++it){
			std::cout << it->first << "\t";
			std::set<int>& s = it->second;
			for (std::set<int>::iterator it2 = s.begin(); it2 != s.end(); ++it2){
				std::cout << *it2 << ", ";
			}
			std::cout << std::endl;
		}

		LineToPCMap::iterator it = line_to_pc_map_.find(line);
		if (it == line_to_pc_map_.end()) fail("Invalid line number !", get_line_number());
		return it->second;
	}


	// Convenience methods to access data members.
	const DataManager& data_manager() const { return data_manager_; }
	const SipTables& sip_tables() const { return sip_tables_; }
	SialPrinter& printer() const { return *printer_; }


private:
	int pc_; 			/*! the "program counter". Actually, the current location in the op_table_.	 */
	int timer_pc_; 		/** Auxillary field needed by timers. initialize to value < 0 **/

protected:
	const SipTables& sip_tables_; /*! static data */
	DataManager data_manager_;    /*! dynamic data */

	/** Object that manages and formats output from SIAL print statements.
	 * A subclass with desired properties is passed to the constructor.
	 * One such subclass is used for tests that compare sial output to
	 * an expected string.
	 */
	SialPrinter* printer_;

	/**
	 * Valid for when BalancedTaskAllocParallelPardoLoop is used.
	 * In a pardo section, the iteration is counted across all pardos. This variable is passed to
	 * the BalancedTaskAllocParallelPardoLoop instance.
	 */
	long iteration_;

	const OpTable & op_table_;  /*! owned by sipTables_, pointer copied for convenience */
	SialxTimer* sialx_timers_; /*! Data structure to hold timers. Owned by calling program. Maybe NULL */

	/**
	 * The reference is needed for upcalls to handle set_persistent and restore_persistent instructions
	 * Owned by main program
	 */
	WorkerPersistentArrayManager* persistent_array_manager_;

	/**
	 * Implements sial operations. Mostly communication related.
	 */
#ifdef HAVE_MPI
	SialOpsParallel sial_ops_;  //todo make this a template param
#else
	SialOpsSequential sial_ops_;
#endif


	/** contains a stack of pointers to LoopManager objects.
	 * A LoopManager is created and pushed onto the stack when a loop is entered,
	 * and popped when a loop is exited.  The LoopManager is the
	 * base class--each type of loop, do loop, sequential pardo, (later, static pardo), has its own subclass.
	 * This is one of the few places where subclass polymorphism is used in aces4.
	 */
	std::stack<LoopManager*> loop_manager_stack_;

	/** instructions that access blocks are usually preceded by a reindex op that pushes the selector of the
	 * desired block onto the block_selector_stack.  The selectors are then popped in the instruction.
	 * For example, a(i,j) = b(i,j) would generate push_block_selector_op instructions to put selectors for  a(i,j), and
	 * b(i,j) onto the stack.  Then, the handle_assignment_op would pop both selectors and use them to retrieve
	 * the blocks.
	 */
	std::stack<sip::BlockSelector> block_selector_stack_;

	/** the control_stack contains integers.  It is used for holding various int values needed by the interpreter.
	 * In particular, loop instructions use this stack to record optable offsets such as the offset of first instruction in the loop body, etc.
	 * The control stack is also used for evaluating relational expressions that have integer arguments
	 */
	std::stack<int> control_stack_;

	/** The expression stack is used for evaluating floating point expressions. */
	std::stack<double> expression_stack_;

	/** The write_back_list keeps track of which blocks that have been extracted from a contiguous array have been
	 * modified, and thus need to be inserted back into the array when an operation is complete.
	 * For example, if a is a static array, a(i,j) = b(i,j,k,l) * c(k,l), then a buffer the size of a(i,j) would
	 * be obtained to receive the value of b(i,j,k,l) * c(k,l).  Once the block has been computed, it will
	 * be copied back into the contiguous array.
	 */
	sip::WriteBackList write_back_list_;


	/** Maintains the list of sliced out blocks from contiguous arrays.
	 * Garbage collected after the operation that uses it.
	 */
	sip::ReadBlockList read_block_list_;

	/** Records whether or not we are executing in a section of code where gpu_on has been invoked
	 * TODO  is nesting of gpu sections allowed?  If so, this needs to be an int.  If not, need check.
	 */
	bool gpu_enabled_;

	/** sialx line number -> {set of program counters} */
	typedef std::map<int, std::set<int> > LineToPCMap;
	LineToPCMap line_to_pc_map_;


	// Convenience methods for getting data from the current pc's optable entry
	int arg0(int pc){ return op_table_.arg0(pc); }
	int arg1(int pc){ return op_table_.arg1(pc); }
	int arg2(int pc){ return op_table_.arg2(pc); }
	const sip::index_selector_t& index_selectors(int pc){return op_table_.index_selectors(pc);}

	/**
	 * Handles post processing for slices of contiguous arrays.
	 * -Frees up contiguous array slices that were obtained for intent "read".
	 * -Writes back the blocks with intent "write/update" in the write_back_list_
	 *  into their containing contiguous array.
	 * -This should be called after each instruction that may have updated a block.
	 * TODO let the compiler indicate if this needs to be called.
	 */
	void contiguous_blocks_post_op();


	/** Called by constructor */
	void _init(const SipTables&);

	/** main interpreter procedure */
	void do_interpret(int pc_start, int pc_end);

	/** Manages per-line timers.
	 *
	 * It should be invoked before execution of each opcode.
	 * It should also be invoked with opcode = invalid_opcode and a negative value for line
	 * the interpreter loop has completed to ensure all timers are turned off.
	 * **/
	void timer_trace(int pc, opcode_t opcode, int line);

	/**The next set of routines are helper routines in the interpreter whose function should be obvious from the name */
	void handle_user_sub_op(int pc);
	void handle_contraction(int drank, const index_selector_t& dselected_index_ids, Block::BlockPtr dblock);
	void handle_contraction(int drank, const index_selector_t& dselected_index_ids, double* ddata, segment_size_array_t& dshapeget);
	void handle_block_add(int pc);
	void handle_block_subtract(int pc);
	void loop_start(int &pc, LoopManager * loop);
	void loop_end(int &pc);

//	void handle_assignment_op(int pc);
//	void handle_contraction_op(int pc);
//	void handle_where_op(int pc);
//	bool evaluate_where_clause(int pc);
//	bool evaluate_double_relational_expr(int pc);
//	bool evaluate_int_relational_expr(int pc);
//	void handle_collective_sum_op(int source_array_slot, int dest_array_slot);
//	void handle_sum_op(int pc, double factor);
//	void handle_self_multiply_op(int pc);
//	void handle_slice_op(int pc);
//	void handle_insert_op(int pc);

	/** Gets the rank and array Id index_selector array from the instruction.
	 * @param
	 * @return
	 */
	BlockId get_block_id_from_instruction(int pc);

	/** Pops the selector off the top of the block_selector_stack, reads the bound off the control_stack
	 * and returns the ContiguousLocalBlockId.
	 *
	 * @return
	 */
	BlockId get_contiguous_local_id_from_selector_stack();
	/** Pops the selector off the top of the block_selector_stack_, and obtains the BlockId of the selected block.
	 * Used when only the ID, but not the block itself is needed, such as the left hand side of a put, or prepare.
	 */
	sip::BlockId get_block_id_from_selector_stack();

	/**
	 * Constructs a block id from a given block selector.
	 * Reads values from control_stack_, modifying it.
	 * @param selector [in]
	 * @return
	 */
	sip::BlockId get_block_id_from_selector(const BlockSelector& selector);

	/** get_block
	 *
	 * Returns a pointer to the block on top of the selector stack, and removes the selector from the stack.
	 * It also returns the BlockId of the selected block in the out parameter.
	 *
	 * If the intent is 'r' (read), the block must exist or a runtime error
	 * will be thrown.  If the intent is 'w' (write), the block will be created if it doesn't exist.  If the intent is 'u', the block must exist.
	 * If the intent is 'w' and the caller does not write all the elements, an unwritten element will either have the default value (zero) if the
	 * block did not previously exist, or the previous value.
	 *
	 * TODO  discuss whether intent 'w' should cause the block to be initialized to zero.
	 *
	 * If the block belongs to a contiguous array, but contiguous_allowed = false, a runtime error will be given.
	 * If the block belongs to a contiguous array, and the intent is r or u, the elements from that block will be extracted.  If the
	 * intent is 'w' or 'u', the block will be added to the write_back_list.
	 *
	 * @param[in] intent, either 'r' for read,  'w' for write,  or 'u' for update.
	 * @param[out] the blockId
	 * @param[in]  contiguous_allowed indicates whether or not contiguous blocks are allowed.  Subblocks of contiguous arrays will be automatically extracted and/or written back depending on the intent.
	 * @return  pointer to selected Block
	 */
	sip::Block::BlockPtr get_block_from_selector_stack(char intent, sip::BlockId&, bool contiguous_allowed = true);
	/**
	 * Returns a pointer to the block on top of the selector stack and removes the selector from the stack.
	 * This just invokes the get_block_from_selector_stack version with a BlockId parameter and throws it away.
	 *
	 * @param intent
	 * @param contiguous_allowed
	 * @return pointer to selected Block
	 */
	sip::Block::BlockPtr get_block_from_selector_stack(char intent, bool contiguous_allowed = true);


	/**
	 * Returns a pointer to the block indicated by the arg0 (rank), arg1 (array slot), and index selector
	 * taken from the current instruction  This is the same as get_block_from_selector_stack except that
	 * the information is in the instruction instead of on the block_selector_stack.
	 *
	 * @param intent
	 * @param contiguous_allowed
	 * @return
	 */
	Block::BlockPtr get_block_from_instruction(int pc, char intent, bool contiguous_allowed = true);


	/**
	 * Returns the block identified by the given block selector
	 *
	 * @param[in] intent, either 'r' for read,  'w' for write,  or 'u' for update.
	 * @param[in] a BlockSelector
	 * @param[out] the Id of the selected block
	 * @param[in] contiguous_allowed indicates whether or not contiguous blocks are allowed.  Subblocks of contiguous arrays will be automatically extracted and/or written back depending on the intent.
	 * @return pointer to selected Block
	 */
	sip::Block::BlockPtr get_block(char intent, sip::BlockSelector&, sip::BlockId&, bool contiguous_allowed = true);

	// GPU
	sip::Block::BlockPtr get_gpu_block(char intent, sip::BlockId&, bool contiguous_allowed = true);
	sip::Block::BlockPtr get_gpu_block(char intent, sip::BlockSelector&, sip::BlockId&, bool contiguous_allowed = true);
	sip::Block::BlockPtr get_gpu_block(char intent, bool contiguous_allowed = true);
	sip::Block::BlockPtr get_gpu_block_from_selector_stack(char intent, sip::BlockId& id, bool contiguous_allowed = true);


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
	virtual void handle_loop_end(int &pc);
	virtual void handle_exit_op(int &pc);
	virtual void handle_where_op(int &pc);
	virtual void handle_pardo_op(int &pc);
	virtual void handle_endpardo_op(int &pc);
	virtual void handle_call_op(int &pc);

	// Functions that don't modify the program counter.
	virtual void handle_execute_op(int pc);
	virtual void handle_sip_barrier_op(int pc);
	virtual void handle_broadcast_static_op(int pc);
	virtual void handle_push_block_selector_op(int pc);
	virtual void handle_allocate_op(int pc);
	virtual void handle_deallocate_op(int pc);
	virtual void handle_allocate_contiguous_op(int pc);
	virtual void handle_deallocate_contiguous_op(int pc);
	virtual void handle_get_op(int pc);
	virtual void handle_put_accumulate_op(int pc);
	virtual void handle_put_replace_op(int pc);
	virtual void handle_create_op(int pc);
	virtual void handle_delete_op(int pc);
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
	virtual void handle_cast_to_scalar_op(int pc);
	virtual void handle_collective_sum_op(int pc);
	virtual void handle_assert_same_op(int pc);
	virtual void handle_block_copy_op(int pc);
	virtual void handle_block_permute_op(int pc);
	virtual void handle_block_fill_op(int pc);
	virtual void handle_block_scale_op(int pc);
	virtual void handle_block_scale_assign_op(int pc);
	virtual void handle_block_accumulate_scalar_op(int pc);
	virtual void handle_block_add_op(int pc);
	virtual void handle_block_subtract_op(int pc);
	virtual void handle_block_contract_op(int pc);
	virtual void handle_block_contract_to_scalar_op(int pc);
	virtual void handle_block_load_scalar_op(int pc);
	virtual void handle_print_string_op(int pc);
	virtual void handle_print_scalar_op(int pc);
	virtual void handle_print_int_op(int pc);
	virtual void handle_print_index_op(int pc);
	virtual void handle_print_block_op(int pc);
	virtual void handle_println_op(int pc);
	virtual void handle_gpu_on_op(int pc);
	virtual void handle_gpu_off_op(int pc);
	virtual void handle_set_persistent_op(int pc);
	virtual void handle_restore_persistent_op(int pc);
	virtual void handle_idup_op(int pc);
	virtual void handle_iswap_op(int pc);
	virtual void handle_sswap_op(int pc);


	/**
	 * Steps to be performed after interpreting opcode at old_pc.
	 * @param old_pc
	 * @param new_pc program counter changed to new pc as a result of interpreting old_pc
	 */
	virtual void post_interpret(int old_pc, int new_pc);

	/**
	 * Steps to be peformed before interpreting opcode at pc.
	 * The timer trace happens here.
	 * Override this method with a blank implementation to do nothing.
	 * @param pc
	 */
	virtual void pre_interpret(int pc);


	// ================================================================
	//		End of Methods to be overriden by abstract interpreters
	// ================================================================

	friend class DoLoop;
	friend class SequentialPardoLoop;
	friend class BalancedTaskAllocParallelPardoLoop;
	friend class Tracer;
	friend class ::TestControllerParallel;
	friend class ::TestController;

	DISALLOW_COPY_AND_ASSIGN(SialxInterpreter);

};
/* class Interpreter */

} /* namespace sip */

#endif /* SIALX_INTERPRETER_H_ */
