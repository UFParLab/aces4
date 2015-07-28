/*
 * disk_backed_block_map.h
 *
 *  Created on: Apr 17, 2014
 *      Author: njindal
 */

#ifndef DISK_BACKED_BLOCK_MAP_H_
#define DISK_BACKED_BLOCK_MAP_H_

#include "id_block_map.h"
#include "disk_backed_arrays_io.h"
#include "lru_array_policy.h"
#include "server_timer.h"

namespace sip {
class BlockId;
class ServerPersistentArrayManager;

/**
 * Wrapper over block map for servers.
 * Support spilling over to disk and other operations.
 *
 * This class is responsible for maintaining the measures of remaining memory, and
 * for updating the status of individual blocks as they are modified, written and read from
 * disks, etc.
 *
 * All block-sized memory allocation (including, for example, temp buffers for asynchronous
 * ops), should happen via this class.
 */
class DiskBackedBlockMap {
public:
	DiskBackedBlockMap(const SipTables&, const SIPMPIAttr&, const DataDistribution&, ServerTimer&);
    ~DiskBackedBlockMap();

	// Get blocks for reading, writing, updating
	ServerBlock* get_block_for_reading(const BlockId& block_id, int line);
	ServerBlock* get_block_for_writing(const BlockId& block_id);
	ServerBlock* get_block_for_updating(const BlockId& block_id);
	ServerBlock* get_block_for_accumulate(const BlockId& block_id);

	// Allocate and deallocate the given number of double precision values.
	// This is used internally in this class, and to allocate temporary buffers for asynchronous operations
	// These should be used instead of new and delete in order so they are included in memory accounting
	double* allocate_data(size_t size, bool initialize);
	//Sets the pointer to NULL
	void  free_data(double*& data, size_t size);

	// Get entire arrays for save, restore operations
//	IdBlockMap<ServerBlock>::PerArrayMap* get_and_remove_per_array_map(int array_id);
	void insert_per_array_map(int array_id, IdBlockMap<ServerBlock>::PerArrayMap* map_ptr);
	void delete_per_array_map_and_blocks(int array_id);
	IdBlockMap<ServerBlock>::PerArrayMap* per_array_map(int array_id);

	/*! Restores a persistent array from disk. Delegates to internal DiskBackedArraysIO object.*/
	void restore_persistent_array(int array_id, std::string & label);

	/*! Saves a persistent array to disk. Delegates to internal DiskBackedArraysIO object.*/
	void save_persistent_array(const int array_id,const std::string& array_label,
			IdBlockMap<ServerBlock>::PerArrayMap* array_blocks);

	friend std::ostream& operator<<(std::ostream& os, const DiskBackedBlockMap& obj);

//unneeded, set in constructor
//	/** Sets max_allocatable_bytes_ */
//    void set_max_allocatable_bytes(std::size_t size);

private:

	// Reads block data from disk, updates memory accounting and block dis_back_state
	void read_block_from_disk(ServerBlock*& block, const BlockId& block_id, size_t block_size);

	// Writes block data to disk and frees in memory data, updates memory accounting and block disk_back_state
    void write_block_to_disk(const BlockId& block_id, ServerBlock* block);


    ServerBlock* allocate_block(size_t block_size, bool initialize);

    // Attempts to free the requested number of doubles (i.e. increase remaining
    // by at least requested_doubles_to_free
    // by writing block data to disk.  Returns the actual size freed.
    // The caller is responsible for using the returned value to update remaining_doubles_
    // This method updates the state of blocks that are freed on disk
	size_t backup_and_free_doubles(size_t requested_doubles_to_free);

	//ServerBlock* get_or_create_block(const BlockId& block_id, size_t block_size, bool initialize);

    const SipTables &sip_tables_;
	const SIPMPIAttr & sip_mpi_attr_;
	const DataDistribution &data_distribution_;

	ServerTimer &server_timer_;

    // Since the policy_ can be constructed only
    // after the block_map_ has been constructed, 
    // policy_ must appear after the declaration
    // of block_map_ here. 
	DiskBackedArraysIO disk_backed_arrays_io_;	/*! interface to disk io for blocks */
	IdBlockMap<ServerBlock> block_map_;			/*! interface to memory block map */
    LRUArrayPolicy<ServerBlock> policy_;		/*! block replacement policy */

    /** Maximum number of bytes before spilling over to disk */
	std::size_t max_allocatable_bytes_;
	std::size_t max_allocatable_doubles_;

	/** Remaining number of double precision numbers that can be allocated
	 * without exceeding max_allocatable_bytes_
	 *
	 * This value is updated whenever memory is allocated or released in allocating the
	 * data portion of a block, or the temp buffer for an asynchronous operation.
	 */
	std::size_t remaining_doubles_;

};

} /* namespace sip */

#endif /* DISK_BACKED_BLOCK_MAP_H_ */
