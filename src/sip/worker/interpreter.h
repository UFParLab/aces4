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

#ifndef INTERPRETER_H_
#define INTERPRETER_H_

#include <string>
#include <stack>
#include <sstream>
#include "block_manager.h"
#include "contiguous_array_manager.h"
#include "data_manager.h"
#include "sialx_timer.h"
#include "config.h"
#include "worker_persistent_array_manager.h"

#ifdef HAVE_MPI
#include "sial_ops_parallel.h"
#else
#include "sial_ops_sequential.h"
#endif //HAVE_MPI

namespace sip {

class LoopManager;

class Interpreter {
public:


	Interpreter(SipTables&, SialxTimer&, WorkerPersistentArrayManager* wpm = NULL);

	~Interpreter();

	/** Static pointer to the current Interpreter.  This is
	 * initialized in the Interpreter constructor and reset to NULL
	 * in its destructor.  There should be at most on Interpreter instance
	 * at any given time.
	 */
	static Interpreter* global_interpreter;

	friend class DoLoop;
	friend class SequentialPardoLoop;

	//data_access routines.  These are just for convenience and just call the
	//appropriate routine in the sipTable or dataManager.  The definitions are given
	//here to allow inlining.

	SipTables* sip_tables(){return &sip_tables_;}

	std::string string_literal(int slot) {
		return sip_tables_.string_literal(slot);
	}
	double scalar_value(int array_table_slot) {
		return data_manager_.scalar_value(array_table_slot);
	}
	double scalar_value(const std::string& name) {
		return data_manager_.scalar_value(name);
	}
	void set_scalar_value(int array_table_slot, double value) {
		data_manager_.set_scalar_value(array_table_slot, value);
	}
	void set_scalar_value(const std::string& name, double value) {
		data_manager_.set_scalar_value(name, value);
	}
	bool is_scalar(int array_table_slot) {
		return sip_tables_.is_scalar(array_table_slot);
	}
	int int_value(int int_table_slot) {
		return sip_tables_.int_value(int_table_slot);
	}
	int index_value(int index_table_slot) {
		return data_manager_.index_value(index_table_slot);
	}
	std::string index_value_to_string(int index_table_slot) {
		std::stringstream ss;
		ss << sip_tables_.index_name(index_table_slot) << '='
				<< index_value(index_table_slot);
		return ss.str();
	}
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
	int num_subsegments(int subindex_slot, int super_index_value) {
		return sip_tables_.num_subsegments(subindex_slot, super_index_value);
	}
	int segment_extent(int index_slot, int segment) {
		return sip_tables_.index_table_.segment_extent(index_slot, segment);
	}
	sip::BlockId block_id(const sip::BlockSelector& selector) {
		return data_manager_.block_id(selector);
	}
	std::string array_name(int array_table_slot) {
		return sip_tables_.array_name(array_table_slot);
	}
	int array_slot(std::string array_name) {
		return sip_tables_.array_table_.array_slot(array_name);
	}
	bool is_contiguous(int array_table_slot) {
		return sip_tables_.is_contiguous(array_table_slot);
	}
//	sip::Block::BlockPtr get_block_for_reading(sip::BlockId& id){return data_manager_.block_manager_.get_block_for_reading(id);}
	bool is_distributed_or_served(int array_slot) {
		return sip_tables_.is_distributed(array_slot)
				|| sip_tables_.is_served(array_slot);
	}
	Block* get_and_remove_contiguous_array(int array_id) {
		return data_manager_.contiguous_array_manager_.get_and_remove_array(
				array_id);
	}
	void set_contiguous_array(int array_id, Block* contig) {
		data_manager_.contiguous_array_manager_.insert_contiguous_array(
				array_id, contig);
	}
	IdBlockMap<Block>::PerArrayMap* get_and_remove_per_array_map(int array_id) {
		return data_manager_.block_manager_.block_map_.get_and_remove_per_array_map(
				array_id);
	}
	void set_per_array_map(int array_id,
			IdBlockMap<Block>::PerArrayMap* map_ptr) {
		data_manager_.block_manager_.block_map_.insert_per_array_map(array_id,
				map_ptr);
	}

	Block::BlockPtr get_block_for_reading(const BlockId& block_id){
		return data_manager_.block_manager_.get_block_for_reading(block_id);
	}
	/**
	 * main interpret function
	 */
	void interpret();

	/**
	 * Returns the line number in the SIAL program corresponding to the
	 * instruction at op_table[pc].
	 *
	 * Returns -1 if pc is out of range of the op_table.
	 *
	 * @return
	 *
	 */
	int line_number() {
		if (pc < op_table_.size())
			return op_table_.line_number(pc);
		else
			return -1;// Past the end of the program. Probably being called by a test.
	}

//TODO these should be private.  Made public as expedient way to implement list_block_map super instruction.

	//static data
	SipTables& sip_tables_;
	//dynamic data
	DataManager data_manager_;
private:

	/**
	 * Called by constructor
	 * @param
	 */
	void _init(SipTables&);

	/** main interpreter procedure */
	void interpret(int pc_start, int pc_end);

	/**
	 *  Called after the main interpret loop terminates to
	 *  cleanly terminate this program.
	 */
	void post_sial_program();


	OpTable & op_table_;  //owned by sipTables_, pointer copied for convenience
	/**
	 * Timer manager
	 */
	sip::SialxTimer& sialx_timers_;

	/**
	 * Owned by main program
	 * The reference is needed for upcalls to handle set_persistent and restore_persistent instructions
	 */
	WorkerPersistentArrayManager* persistent_array_manager_;

#ifdef HAVE_MPI
	SialOpsParallel sial_ops_;  //todo make this a template param
#else
	SialOpsSequential sial_ops_;
#endif
//	SIAL_OPS_TYPE sial_ops_;

	/** the "program counter". Actually, the current location in the op_table_.
	 */
	int pc; //technically, this should be pc_, but I'm going to leave it this way for convenience

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

	/** The expression stack is used for evaluating floating point expressions.
	 */
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

	/**
	 * Handles post processing for slices of contiguous arrays.
	 * -Frees up contiguous array slices that were obtained for intent "read".
	 * -Writes back the blocks with intent "write/update" in the write_back_list_
	 *  into their containing contiguous array.
	 * -This should be called after each instruction that may have updated a block.
	 * TODO let the compiler indicate if this needs to be called.
	 */
	void contiguous_blocks_post_op();


	/**
	 * Records whether or not we are executing in a section of code where gpu_on has been invoked
	 */
	bool gpu_enabled_;


	/**The next set of routines are helper routines in the interpreter whose function should be obvious from the name */
	void handle_user_sub_op(int pc);
	void handle_assignment_op(int pc);
	void handle_contraction_op(int pc);
	void handle_where_op(int pc);
	bool evaluate_where_clause(int pc);
	bool evaluate_double_relational_expr(int pc);
	bool evaluate_int_relational_expr(int pc);
	void handle_collective_sum_op(int source_array_slot, int dest_array_slot);
	void handle_sum_op(int pc, double factor);
	void handle_self_multiply_op(int pc);
	void handle_slice_op(int pc);
	void handle_insert_op(int pc);

	void loop_start(LoopManager * loop);
	void loop_end();

	/** Pops the selector off the top of the block_selector_stack_, and obtains the BlockId of the selected block.
	 * Used when only the ID, but not the block itself is needed, such as the left hand side of a put, or prepare.
	 */
	sip::BlockId get_block_id_from_selector_stack();

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
	sip::Block::BlockPtr get_block_from_selector_stack(char intent,
			sip::BlockId&, bool contiguous_allowed = true);
	/**
	 * Returns a pointer to the block on top of the selector stack and removes the selector from the stack.
	 * This just invokes the get_block_from_selector_stack version with a BlockId parameter and throws it away.
	 *
	 * @param intent
	 * @param contiguous_allowed
	 * @return pointer to selected Block
	 */
	sip::Block::BlockPtr get_block_from_selector_stack(char intent,
			bool contiguous_allowed = true);
	/**
	 * Returns the block identified by the given b
	 *
	 * @param[in] intent, either 'r' for read,  'w' for write,  or 'u' for update.
	 * @param[in] a BlockSelector
	 * @param[out] the Id of the selected block
	 * @param[in] contiguous_allowed indicates whether or not contiguous blocks are allowed.  Subblocks of contiguous arrays will be automatically extracted and/or written back depending on the intent.
	 * @return pointer to selected Block
	 */
	sip::Block::BlockPtr get_block(char intent, sip::BlockSelector&,
			sip::BlockId&, bool contiguous_allowed = true);

	// GPU
	sip::Block::BlockPtr get_gpu_block(char intent, sip::BlockId&,
			bool contiguous_allowed = true);
	sip::Block::BlockPtr get_gpu_block(char intent, sip::BlockSelector&,
			sip::BlockId&, bool contiguous_allowed = true);
	sip::Block::BlockPtr get_gpu_block(char intent, bool contiguous_allowed =
			true);
	sip::Block::BlockPtr get_gpu_block_from_selector_stack(char intent,
			sip::BlockId& id, bool contiguous_allowed = true);

	DISALLOW_COPY_AND_ASSIGN(Interpreter);
};
/* class Interpreter */

} /* namespace sip */

#endif /* INTERPRETER_H_ */
