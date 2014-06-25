/*
 * disk_backed_block_map.h
 *
 *  Created on: Apr 17, 2014
 *      Author: njindal
 */

#ifndef DISK_BACKED_BLOCK_MAP_H_
#define DISK_BACKED_BLOCK_MAP_H_

#include "disk_backed_block_map.h"
#include "id_block_map.h"
#include "disk_backed_arrays_io.h"
#include "lru_array_policy.h"

namespace sip {
class BlockId;
class ServerPersistentArrayManager;

/**
 * Wrapper over block map for servers.
 * Support spilling over to disk and other operations.
 */
class DiskBackedBlockMap {
public:
	DiskBackedBlockMap(const SipTables&, const SIPMPIAttr&, const DataDistribution&);
    ~DiskBackedBlockMap();

	// Get blocks for reading, writing, updating
	ServerBlock* get_block_for_reading(const BlockId& block_id);
	ServerBlock* get_block_for_writing(const BlockId& block_id);
	ServerBlock* get_block_for_updating(const BlockId& block_id);

	// Get entire arrays for save, restore operations
	IdBlockMap<ServerBlock>::PerArrayMap* get_and_remove_per_array_map(int array_id);
	void insert_per_array_map(int array_id, IdBlockMap<ServerBlock>::PerArrayMap* map_ptr);
	void delete_per_array_map_and_blocks(int array_id);
	IdBlockMap<ServerBlock>::PerArrayMap* per_array_map(int array_id);

	/*! Restores a persistent array from disk. Delegates to internal DiskBackedArraysIO object.*/
	void restore_persistent_array(int array_id, std::string & label);

	/*! Saves a persistent array to disk. Delegates to internal DiskBackedArraysIO object.*/
	void save_persistent_array(const int array_id,const std::string& array_label,
			IdBlockMap<ServerBlock>::PerArrayMap* array_blocks);

	void reset_consistency_status_for_all_blocks();

	friend std::ostream& operator<<(std::ostream& os, const DiskBackedBlockMap& obj);

private:

	void read_block_from_disk(ServerBlock*& block, const BlockId& block_id, size_t block_size);
    void write_block_to_disk(const BlockId& block_id, ServerBlock* block);
	ServerBlock* allocate_block(ServerBlock* block, size_t block_size, bool initialze=true);

	//ServerBlock* get_or_create_block(const BlockId& block_id, size_t block_size, bool initialize);



    const SipTables &sip_tables_;
	const SIPMPIAttr & sip_mpi_attr_;
	const DataDistribution &data_distribution_;

    // Since the policy_ can be constructed only
    // after the block_map_ has been constructed, 
    // policy_ must appear after the declaration
    // of block_map_ here. 
	DiskBackedArraysIO disk_backed_arrays_io_;	/*! interface to disk io for blocks */
	IdBlockMap<ServerBlock> block_map_;			/*! interface to memory block map */
    LRUArrayPolicy policy_;                     /*! block replacement policy */

};

} /* namespace sip */

#endif /* DISK_BACKED_BLOCK_MAP_H_ */
