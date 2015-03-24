/*
 * interpreter.h
 *
 *  Created on: Sep 28, 2014
 *      Author: jindal
 */

#ifndef INTERPRETER_H_
#define INTERPRETER_H_

#include "block.h"
#include "id_block_map.h"
#include "setup_reader.h"
#include <vector>

#ifdef _OPENMP
#include <omp.h>
#endif

namespace sip {

class SialPrinter;

/**
 * Base class for all (Abstract) Interpreters.
 */
class Interpreter {
public:

	/** main interpret function */
	void interpret() { do_interpret(); }

	/**
	 *  Should be called after the main interpret loop terminates to
	 *  cleanly terminate this program.  This is the responsibility of
	 *  the main program to allow more convenient testing of parallel
	 *  programs that don't use servers.
	 */
	void post_sial_program() { do_post_sial_program(); }

	/**
	 * Returns the line number in the SIAL program corresponding to the
	 * instruction at op_table[pc].
	 * @return -1 if pc is out of range of the op_table.
	 */
	int line_number() { return get_line_number(); }

	int current_pc() { return get_pc(); }

	/** Allows superinstructions to use the same ostream as sial. */
	SialPrinter* printer() { return get_printer(); }

	std::string string_literal(int slot) { return string_literal_impl(slot); }

	double scalar_value(int array_table_slot) { return scalar_value_impl(array_table_slot); }
	double scalar_value(const std::string& name) { return scalar_value_impl(name); }
	void set_scalar_value(int array_table_slot, double value) { set_scalar_value_impl(array_table_slot, value); }
	void set_scalar_value(const std::string& name, double value) { 	set_scalar_value_impl(name, value); }
	bool is_scalar(int array_table_slot) { return is_scalar_impl(array_table_slot); }

	int int_value(int int_table_slot) { return int_value_impl(int_table_slot);}
	void set_int_value(int int_table_slot, int value){ set_int_value_impl(int_table_slot, value);	}
	int int_value(const std::string& name) { return int_value_impl(name);}
	void set_int_value(const std::string& name, int value){ set_int_value_impl(name, value);	}

	double predefined_scalar(const std::string& name) { return predefined_scalar_impl(name); }
	int predefined_int(const std::string& name) { return predefined_int_impl(name); }
	setup::PredefContigArray predefined_contiguous_array(const std::string& name) { return predefined_contiguous_array_impl(name); }
	setup::PredefIntArray predefined_integer_array(const std::string& name) { return predefined_integer_array_impl(name); }

	int index_value(int index_table_slot) { return index_value_impl(index_table_slot); }
	std::string index_value_to_string(int index_table_slot) { return index_value_to_string_impl(index_table_slot); }

	int num_subsegments(int subindex_slot, int super_index_value) { return num_subsegments_impl(subindex_slot, super_index_value);}
	int segment_extent(int index_slot, int segment) { return segment_extent_impl(index_slot, segment); }

	Block* get_and_remove_contiguous_array(int array_id) { return get_and_remove_contiguous_array_impl(array_id); }
	void set_contiguous_array(int array_id, Block* contig) { set_contiguous_array_impl(array_id, contig); }
	IdBlockMap<Block>::PerArrayMap* get_and_remove_per_array_map(int array_id) {return get_and_remove_per_array_map_impl(array_id);}
	void set_per_array_map(int array_id, IdBlockMap<Block>::PerArrayMap* map_ptr) { set_per_array_map_impl(array_id, map_ptr);	}
	std::string array_name(int array_table_slot) { return array_name_impl(array_table_slot); }

	bool is_contiguous(int array_table_slot) { return is_contiguous_impl(array_table_slot); }
	bool is_distributed_or_served(int array_slot) { return is_distributed_or_served_impl(array_slot); }


	/** For testing.
	 * Determine whether any data is left in the interpreter's data structures.
	 * This method should return true immediately after completion of a sial program.
	 * @return
	 */
	virtual bool all_stacks_empty() = 0;

	/** For testing12
	 * Determines the total number of blocks in the block map, including both the
	 * cache and the active map.  This means that the value may be non-deterministic,
	 * or at least difficult to predict, except at certain points where it is know to
	 * be zero.
	 * @return
	 */
	virtual std::size_t num_blocks_in_blockmap() = 0;

	/**
	 * For testing
	 * @param block_id
	 * @return
	 */
	virtual Block::BlockPtr get_block_for_reading(const BlockId& block_id) = 0;

	/**
	 * For testing
	 * @param array_name
	 * @return
	 */
	virtual int array_slot(const std::string& array_name) = 0;

	/**
	 * For testing
	 * @param array_id
	 * @return
	 */
	virtual Block* get_static(int array_id) = 0;

#ifdef _OPENMP
	virtual ~Interpreter(){
		int this_thread = omp_get_thread_num(), num_threads = omp_get_num_threads();
		global_interpreters_.at(this_thread) = NULL;
	}
	static Interpreter* global_interpreter() {
		int this_thread = omp_get_thread_num(), num_threads = omp_get_num_threads();
		if (global_interpreters_.empty())
			return NULL;
		return global_interpreters_.at(this_thread);
	}
#else
	virtual ~Interpreter(){  global_interpreters_.at(0) = NULL; global_interpreters_.clear(); }
	static Interpreter* global_interpreter() {
		if (global_interpreters_.empty())
					return NULL;
		return global_interpreters_.at(0);
	}
#endif

protected:

	/** Static pointer to the current Interpreter.  This is
	 * initialized in the Interpreter constructor and reset to NULL
	 * in its destructor.  There should be at most on Interpreter instance
	 * at any given time.
	 */
	static std::vector<Interpreter*> global_interpreters_;
	static bool done_once;
#ifdef _OPENMP
	Interpreter() {
	    int this_thread = omp_get_thread_num(), num_threads = omp_get_num_threads();
#pragma omp critical
	    {
	    	if (!done_once){
	    		done_once = true;
	    		global_interpreters_.resize(num_threads, NULL);
	    	}
	    	global_interpreters_.at(this_thread) = this;
	    }
	}
#else
	Interpreter() { global_interpreters_.push_back(this); }
#endif


	virtual SialPrinter* get_printer() = 0;
	virtual int get_line_number() = 0;
	virtual int get_pc() = 0;
	virtual void do_post_sial_program() = 0;
	virtual void do_interpret() = 0;


	// Data access methods
	virtual std::string string_literal_impl(int slot) = 0;

	virtual double scalar_value_impl(const std::string& name) = 0;
	virtual double scalar_value_impl(int array_table_slot) = 0;
	virtual void set_scalar_value_impl(const std::string& name, double value) = 0;
	virtual void set_scalar_value_impl(int array_table_slot, double value) = 0;
	virtual bool is_scalar_impl(int array_table_slot) = 0;

	virtual int int_value_impl(int int_table_slot) = 0;
	virtual void set_int_value_impl(int int_table_slot, int value) = 0;
	virtual int int_value_impl(const std::string& name) = 0;
	virtual void set_int_value_impl(const std::string&, int value) = 0;

	virtual double predefined_scalar_impl(const std::string& name) = 0;
	virtual int predefined_int_impl (const std::string& name) = 0;
	virtual setup::PredefContigArray predefined_contiguous_array_impl(const std::string& name) = 0;
	virtual setup::PredefIntArray predefined_integer_array_impl(const std::string& name) = 0;

	virtual int index_value_impl(int index_table_slot) = 0;
	virtual std::string index_value_to_string_impl(int index_table_slot) = 0;

	virtual int num_subsegments_impl(int subindex_slot, int super_index_value) = 0;
	virtual int segment_extent_impl(int index_slot, int segment) = 0;

	virtual Block* get_and_remove_contiguous_array_impl(int array_id) = 0;
	virtual void set_contiguous_array_impl(int array_id, Block* contig) = 0;
	virtual IdBlockMap<Block>::PerArrayMap* get_and_remove_per_array_map_impl(int array_id) = 0;
	virtual void set_per_array_map_impl(int array_id, IdBlockMap<Block>::PerArrayMap* map_ptr) = 0;
	virtual std::string array_name_impl(int array_table_slot) = 0;

	virtual bool is_contiguous_impl(int array_table_slot) = 0;
	virtual bool is_distributed_or_served_impl(int array_slot) = 0;


};

} /* namespace sip */

#endif /* INTERPRETER_H_ */
