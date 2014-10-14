/* A singleton DataManager instance contains the dynamic data used in the sip.
 * In particular, it contains a BlockManager instance, a ContiguousArrayManager instance,
 * a table for index values, and a table for scalars.
 *
 * data_manager.h
 *
 *  Created on: Aug 14, 2013
 *      Author: basbas
 */

#ifndef DATA_MANAGER_H_
#define DATA_MANAGER_H_

#include "config.h"
#include "sip.h"
#include "sip_tables.h"
#include "block_selector.h"
#include "block.h"
#include "block_manager.h"
#include "contiguous_array_manager.h"
#include "contiguous_local_array_manager.h"
#include "special_instructions.h"

class TestControllerParallel;
class TestController;

namespace sip {
class SialOpsParallel;
class SialOpsSequential;
class SialxInterpreter;
class ProfileInterpreter;
class SIPMaPInterpreter;

class DataManager {
public:

	typedef std::vector<BlockId> BlockIdList;
	typedef std::vector<Block::BlockPtr>  ScalarBlockTable;


	DataManager(const SipTables &sip_tables);



	~DataManager();

	//for scalars
	double scalar_value(int array_table_slot) const;
	double scalar_value(const std::string& name) const;
	double *scalar_address(int array_table_slot);
	void set_scalar_value(int array_table_slot, double value);
	void set_scalar_value(const std::string& name, double value);
	sip::Block::BlockPtr get_scalar_block(int array_table_slot);

	//for ints
	int int_value(int int_table_slot) const;
	int int_value(const std::string& name) const;
	void set_int_value(std::string& name, int value);
	void set_int_value(int int_table_slot, int value);

    //for indices
	int index_value(int index_table_slot) const;
	int index_value(const std::string index_name) const;
	std::string index_value_to_string(int index_table_slot) const;
	bool is_valid(int index_table_slot, int value) const;
	void set_index_value(int index_table_slot, int value);
	int  inc_index(int index_table_slot);
	void set_index_undefined(int index_table_slot);
	static const int undefined_index_value;
	static int scope_count; //DEBUG

	/** returns the block ID for the given selector */
	sip::BlockId block_id(const sip::BlockSelector& selector) const;

	/** determines whether the "block" indicated by the selector is a complete contiguous (static) array */
	bool is_complete_contiguous_array(const sip::BlockSelector& selector) const;

		/** determines whether the block indicated by the selector is a subblock */
	bool is_subblock(const sip::BlockSelector& selector) const;

	/** determines the superblock associated with the subblock indicated by the given selector */
	sip::BlockId super_block_id(const sip::BlockSelector& subblock_selector) const;


    void get_subblock_offsets_and_shape(const sip::Block::BlockPtr block, const sip::BlockSelector& subblock_selector,
    		sip::offset_array_t& offsets, sip::BlockShape& subblock_shape) const;



	void enter_scope();
	void leave_scope();


	//immutable data for convenience
	const BlockManager& block_manager() const { return block_manager_; }	// For printing


private:


	//dynamic state
	std::vector<int> index_values_; //maps index_table id's into current value of the represented index.  undefined_index_value if not defined.
	ScalarTable scalar_values_; //scalar values, initialized from "scalar table" read from siox file
	ScalarBlockTable scalar_blocks_; //blocks wrapped around pointers to scalars.
	IntTable int_table_;	// int values, initialized from "int table" read from siox file.

	BlockManager block_manager_;
	ContiguousArrayManager contiguous_array_manager_;
	ContiguousLocalArrayManager contiguous_local_array_manager_;  //this shares the map with block_manager_

	//immutable data for convenience
	const SipTables& sip_tables_;

	friend class SialxInterpreter;
	friend class ProfileInterpreter;
	friend class SIPMaPInterpreter;
	friend class SialOpsParallel;
	friend class SialOpsSequential;
	friend class ::TestControllerParallel;
	friend class ::TestController;

	DISALLOW_COPY_AND_ASSIGN(DataManager);
};

} /* namespace sip */
#endif /* DATA_MANAGER_H_ */
