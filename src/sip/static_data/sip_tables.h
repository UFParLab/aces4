/*! sip_tables.h
 * 
 * Instances of this class hold the static sip state needed by workers for a particular sial program.
 * The static data may be initialized by reading it from setup and .sio files using a setup::SioxReader object (if master)
 * or by receiving the tables from the master (if a worker).
 *
 * Note that this ScalarTable is only used for initialization, the dynamic values
 * are managed by the DataManager.
 *
 *  Created on: Jun 6, 2013
 *      Author: Beverly Sanders
 */

#ifndef SIP_SIP_TABLES_H_
#define SIP_SIP_TABLES_H_



#include <vector>
#include <map>
#include "sip.h"

#include "op_table.h"
#include "int_table.h"
#include "array_table.h"
#include "block_id.h"
#include "block_shape.h"
#include "index_table.h"
#include "special_instructions.h"
#include "setup_reader.h"
#include "siox_reader.h"
//#include "segment_table.h"

//forward references
class TestController;
namespace sip {
class SegmentTable;
class ContiguousArrayManager;
class DiskBackedArraysIO;
class Tracer;
}

namespace master {
class SioxReader;
}

namespace sip {

class DataManager;
class Interpreter;

class SipTables {
public:
	SipTables(setup::SetupReader&, setup::InputStream&);
	~SipTables();

// Convenience method
	/**
	 * @return Maximum number of slots to initialize in the timer
	 */
	int max_timer_slots() const;

	/**
	 * Returns a vector that contain super instruction names
	 * @return
	 */
	std::vector<std::string> line_num_to_name() const;

//scalars and arrays
	//int array_id(std::string name);
	std::string array_name(int array_table_slot) const;
	std::string scalar_name(int array_table_slot) const;
	int array_rank(int array_table_slot) const;
	int array_rank(const BlockId&) const;
	bool is_scalar(int array_table_slot) const;
	bool is_auto_allocate(int array_table_slot) const;
	bool is_scope_extent(int array_table_slot) const;
	bool is_contiguous(int array_table_slot) const;
	bool is_predefined(int array_table_slot) const;
	bool is_distributed(int array_table_slot) const;
	bool is_served(int array_table_slot) const;
	int num_arrays() const;

//int (symbolic constants)
	int int_value(int int_table_slot) const;
	std::string int_name(int int_table_slot){return int_table_.name(int_table_slot);}

// strint literals
	std::string string_literal(int slot) const;

//indices and blocks
	int index_id(std::string name) const;
	std::string index_name(int index_table_slot) const;
	IndexType_t index_type(int index_table_slot) const;
	BlockShape shape(const BlockId&) const;

	/**
	 * Calculates segment sizes.
	 * @param array_table_slot [in]
	 * @param index_vals [in]
	 * @param seg_sizes [out]
	 */
	void calculate_seq_sizes(const int array_table_slot,
			const index_value_array_t& index_vals, int seg_sizes[MAX_RANK]) const;
	BlockShape contiguous_array_shape(int array_id) const;
	int offset_into_contiguous(int selector, int value) const;
	const index_selector_t& selectors(int array_id) const;

	/**
	 * Returns number of elements in a given block
	 * @param
	 * @return
	 */
	int block_size(const BlockId&) const;

	/**
	 * Returns number of elements in a given block specified by the
	 * array id & index values
	 * @param array_id
	 * @param index_vals
	 * @return
	 */
	int block_size(const int array_id, const index_value_array_t& index_vals) const;

	/**
	 * Returns the offset of a block in its array
	 * The offset is in the number of double precision numbers.
	 * @param bid
	 * @return
	 */
	long block_offset_in_array(const BlockId& bid) const;

	/**
	 * Number of elements in array
	 * @param array_id
	 * @return
	 */
	long array_num_elems(const int array_id) const;

	int lower_seg(int index_table_slot) const;
	int num_segments(int index_table_slot) const;
	int num_subsegments(int index_slot, int parent_segment_value) const;
	bool is_subindex(int index_table_slot) const;
	int parent_index(int subindex_slot) const;

//special instructions
	std::string special_instruction_name(int func_slot);
	SpecialInstructionManager::fp0 zero_arg_special_instruction(int func_slot) const;
	SpecialInstructionManager::fp1 one_arg_special_instruction(int func_slot) const;
	SpecialInstructionManager::fp2 two_arg_special_instruction(int func_slot) const;

    void print() const;
	friend std::ostream& operator<<(std::ostream&, const SipTables &);

	setup::SetupReader& setup_reader() const { return setup_reader_; }


private:


    bool increment_indices(int rank, index_value_array_t& upper, index_value_array_t& lower, index_value_array_t& current) const;
     

	/**
	 * Helper method to returns the offset of a block in its array
	 * @param array_id
	 * @param index_vals
	 */
	long block_indices_offset_in_array(const int array_id, const index_value_array_t& index_vals) const;


	std::vector<int> blocks_in_array();

	OpTable op_table_;
	ArrayTable array_table_;
	IndexTable index_table_;
	ScalarTable scalar_table_;  //only used for initialization, dynamic value held in data manager
	IntTable int_table_;  //currently these are all predefined
	StringLiteralTable string_literal_table_;
	SpecialInstructionManager special_instruction_manager_;
	SioxReader siox_reader_;

	setup::SetupReader& setup_reader_;

	mutable int sialx_lines_;

	friend class SioxReader;  //initializes the SipTable
	friend class Interpreter;
	friend class DataManager;
	friend class ContiguousArrayManager;
	friend class DiskBackedArraysIO;
	friend class ::TestController;
	friend class Tracer;

	DISALLOW_COPY_AND_ASSIGN(SipTables);
};

} /* namespace sip */

#endif /* SIP_SIP_TABLES_H_ */


