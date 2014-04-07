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
namespace sip {
class SegmentTable;
class ContiguousArrayManager;
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

	static SipTables* get_instance();

// Convenience method
	/**
	 * @return Maximum number of slots to initialize in the timer
	 */
	int max_timer_slots();

	/**
	 * Returns a vector that contain super instruction names
	 * @return
	 */
	std::vector<std::string> line_num_to_name();

//scalars and arrays
	//int array_id(std::string name);
	std::string array_name(int array_table_slot);
	std::string scalar_name(int array_table_slot);
	int array_rank(int array_table_slot);
	int array_rank(const sip::BlockId&);
	bool is_scalar(int array_table_slot);
	bool is_auto_allocate(int array_table_slot);
	bool is_scope_extent(int array_table_slot);
	bool is_contiguous(int array_table_slot);
	bool is_predefined(int array_table_slot);
	bool is_distributed(int array_table_slot);
	bool is_served(int array_table_slot);
	int num_arrays();

//int (symbolic constants)
	int int_value(int int_table_slot);

// strint literals
	std::string string_literal(int slot);

//indices and blocks
	//int index_id(std::string name);
	std::string index_name(int index_table_slot);
	sip::IndexType_t index_type(int index_table_slot);
	sip::BlockShape shape(const sip::BlockId&);
	sip::BlockShape contiguous_array_shape(int array_id);
	int offset_into_contiguous(int selector, int value);
	sip::index_selector_t& selectors(int array_id);
	int block_size(const sip::BlockId&);
	int lower_seg(int index_table_slot);
	int num_segments(int index_table_slot);
	int num_subsegments(int index_slot, int parent_segment_value);
	bool is_subindex(int index_table_slot);
	int parent_index(int subindex_slot);

//special instructions
	std::string special_instruction_name(int func_slot);
	SpecialInstructionManager::fp0 zero_arg_special_instruction(int func_slot);
	SpecialInstructionManager::fp1 one_arg_special_instruction(int func_slot);
	SpecialInstructionManager::fp2 two_arg_special_instruction(int func_slot);

    void print();
	friend std::ostream& operator<<(std::ostream&, const SipTables &);

	setup::SetupReader& setup_reader() const { return setup_reader_; }
private:

static SipTables* global_sip_tables;

	OpTable op_table_;
	sip::ArrayTable array_table_;
	sip::IndexTable index_table_;
	ScalarTable scalar_table_;  //only used for initialization, dynamic value held in data manager
	sip::IntTable int_table_;  //currently these are all predefined
	StringLiteralTable string_literal_table_;
	sip::SpecialInstructionManager special_instruction_manager_;
	sip::SioxReader siox_reader_;

	setup::SetupReader& setup_reader_;

	int sialx_lines_;

	friend class SioxReader;  //initializes the SipTable
	friend class Interpreter;
	friend class DataManager;
	friend class ContiguousArrayManager;

	DISALLOW_COPY_AND_ASSIGN(SipTables);
};

} /* namespace sip */

#endif /* SIP_SIP_TABLES_H_ */


