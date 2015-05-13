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
#include "server_timer.h"

#include "server_interpreter.h"

#include <set>

//#include <unordered_map>
#include <map>

#include <limits>
#include <utility>

namespace sip {
class BlockId;
class ServerPersistentArrayManager;
class ServerInterpreter;

/**
 * Wrapper over block map for servers.
 * Support spilling over to disk and other operations.
 */
class DiskBackedBlockMap {
public:
	DiskBackedBlockMap(const SipTables&, const SIPMPIAttr&, const DataDistribution&, 
                       ServerTimer&, ServerInterpreter&);
    ~DiskBackedBlockMap();

	// Get blocks for reading, writing, updating
	ServerBlock* get_block_for_reading(const BlockId& block_id, int line);
	ServerBlock* get_block_for_writing(const BlockId& block_id);
	ServerBlock* get_block_for_updating(const BlockId& block_id);
	ServerBlock* get_block_for_accumulate(const BlockId& block_id);
    
    
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


	/** Sets max_allocatable_bytes_ */
    void set_max_allocatable_bytes(std::size_t size);

    void get_array_blocks(int array_id, std::set<BlockId> &blocks_set);
    void get_dead_blocks(int array_id, std::set<BlockId> &blocks_set);
    
    void prefetch_array(int array_id);
    void free_array(int array_id);
    
    bool prefetch_block(const BlockId &block_id);
    bool write_block(const BlockId &block_id);
    void free_block(const BlockId &block_id);
    
    bool is_block_prefetchable(const BlockId &block_id);
    bool is_block_in_memory(const BlockId &block_id);
    bool is_block_dirty(const BlockId &block_id);
    
    void update_array_distance(std::map<int, int> &new_array_distance);
    
    void select_blocks(int array_id, int index[6], std::set<BlockId> &blocks);
private:

    void create_blocks_bucket();

	void read_block_from_disk(ServerBlock*& block, const BlockId& block_id, size_t block_size);
    void write_block_to_disk(const BlockId& block_id, ServerBlock* block);
	ServerBlock* allocate_block(const BlockId &block_id, ServerBlock* block, size_t block_size, bool initialze=true);

    void remove_block_from_memory(std::size_t needed_bytes);
    
	//ServerBlock* get_or_create_block(const BlockId& block_id, size_t block_size, bool initialize);

    std::multimap<int,int> array_distance_;
    std::set<int> array_distance_mask_;
    std::map<int, int> new_array_distance_;

    long long block_access_counter;
    long long data_volume_counter;
    long long disk_volume_counter;
    long long disk_read_counter;
    long long disk_write_counter;
    long long total_stay_in_memory_counter;
    long long max_allocated_bytes;
    long      max_block_size;

    std::map<int, std::map<int, std::map<int, std::set<BlockId> > > > blocks_bucket;

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

	ServerInterpreter &server_interpreter_;
    
    /** Maximum number of bytes before spilling over to disk */
	std::size_t max_allocatable_bytes_;

	friend ServerInterpreter;
	friend DiskBackedArraysIO;
};

} /* namespace sip */

#endif /* DISK_BACKED_BLOCK_MAP_H_ */
