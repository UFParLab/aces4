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
#include "block_manager.h"
#include "contiguous_array_manager.h"
#include "special_instructions.h"


namespace sip {
class PersistentArrayManager;
}

namespace sip {

class Interpreter;

class DataManager {
public:

	typedef std::vector<sip::BlockId> BlockIdList;
	typedef std::vector<sip::Block::BlockPtr>  ScalarBlockTable;

#ifdef HAVE_MPI
	DataManager(SipTables&, sip::PersistentArrayManager&, sip::PersistentArrayManager&, sip::SIPMPIAttr&, sip::DataDistribution&);
#else
	DataManager(SipTables&, sip::PersistentArrayManager&, sip::PersistentArrayManager&);
#endif


	~DataManager();

	//for scalars
	double scalar_value(int array_table_slot);
	double scalar_value(const std::string& name);
	double *scalar_address(int array_table_slot);
	void set_scalar_value(int array_table_slot, double value);
	void set_scalar_value(const std::string& name, double value);
	sip::Block::BlockPtr get_scalar_block(int array_table_slot);

	//for ints (aces symbolic constants)
	int int_value(int int_table_slot);

    //for indices
	int index_value(int index_table_slot);
	int index_value(const std::string index_name);
	std::string index_value_to_string(int index_table_slot);
	bool is_valid(int index_table_slot, int value);
	void set_index_value(int index_table_slot, int value);
	int  inc_index(int index_table_slot);
	void set_index_undefined(int index_table_slot);
	static const int undefined_index_value;
	static int scope_count; //DEBUG

	/** returns the block ID for the given selector */
	sip::BlockId block_id(const sip::BlockSelector& selector);

	/** determines whether the "block" indicated by the selector is a complete contiguous (static) array */
	bool is_complete_contiguous_array(const sip::BlockSelector& selector);

		/** determines whether the block indicated by the selector is a subblock */
	bool is_subblock(const sip::BlockSelector& selector);

	/** determines the superblock associated with the subblock indicated by the given selector */
	sip::BlockId super_block_id(const sip::BlockSelector& subblock_selector);


    void get_subblock_offsets_and_shape(const sip::Block::BlockPtr block, const sip::BlockSelector& subblock_selector,
    		sip::offset_array_t& offsets, sip::BlockShape& subblock_shape);

	/**scalar collective sum.  Creates dependency on MPI for parallel versions*/
	void collective_sum(int source_array_slot,int dest_array_slot);

	void enter_scope();
	void leave_scope();

	sip::BlockManager block_manager_;  //this should probably be private
	sip::ContiguousArrayManager contiguous_array_manager_;

	/**
	 * Sets an array to be persistent upto the next SIAL Program
	 * @param array_id
	 * @param name
	 * @param slot
	 */
	void set_persistent_array(int array_id, std::string name, int slot);

	/**
	 * Restores a previously saved persistent array.
	 * @param array_id
	 * @param name
	 * @param slot
	 */
	void restore_persistent_array(int array_id, std::string name, int slot);

	/**
	 * Saves persistent arrays to the persistent block manager.
	 */
	void save_persistent_arrays();

private:
	//dynamic state
	std::vector<int> index_values_; //maps index_table id's into current value of the represented index.  undefined_index_value if not defined.
	ScalarTable scalar_values_; //scalar values, initialized from "scalar table" read from siox file
	ScalarBlockTable scalar_blocks_; //blocks wrapped around pointers to scalars.


	//static data for convenience
	SipTables& sipTables_;

	/**
	 * Read and write persistent data between programs.
	 */
	sip::PersistentArrayManager & pbm_read_;
	sip::PersistentArrayManager & pbm_write_;


#ifdef HAVE_MPI
	/**
	 * MPI Attributes of the SIP for this rank
	 */
	sip::SIPMPIAttr & sip_mpi_attr_;

	/**
	 * Data distribution scheme
	 */
	sip::DataDistribution &data_distribution_;
#endif

	friend class Interpreter;

	DISALLOW_COPY_AND_ASSIGN(DataManager);
};

} /* namespace sip */
#endif /* DATA_MANAGER_H_ */
