/*
 * disk_backed_block_map.h
 *
 *  Created on: Apr 17, 2014
 *      Author: njindal
 */

#ifndef DISK_BACKED_BLOCK_MAP_H_
#define DISK_BACKED_BLOCK_MAP_H_

#include "id_block_map.h"
#include "lru_array_policy.h"
#include "server_timer.h"
#include "counter.h"
//
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
	typedef size_t block_num_t;
	typedef ArrayFile::offset_val_t offset_val_t;

	static const int MIN_BLOCKS_PER_CHUNK=4;
	static const offset_val_t ABSENT_BLOCK_OFFSET;

	DiskBackedBlockMap(const SipTables&, const SIPMPIAttr&, const DataDistribution&);
    ~DiskBackedBlockMap();

	// Get blocks for reading, writing, updating
	ServerBlock* get_block_for_reading(const BlockId& block_id, int pc);
	ServerBlock* get_block_for_writing(const BlockId& block_id);
//	ServerBlock* get_block_for_updating(const BlockId& block_id);
	ServerBlock* get_block_for_accumulate(const BlockId& block_id);
	ServerBlock* get_block_for_lazy_restore(const BlockId& block_id);

	// Allocate and deallocate the given number of double precision values.
	// This is used to allocate temporary buffers for asynchronous operations
	// These should be used instead of new and delete in order so they are included in memory accounting
	double* allocate_data(size_t size, bool initialize);//
	//Sets the pointer to NULL
	void  free_data(double*& data, size_t size);

	// Get entire arrays for save, restore operations
//	IdBlockMap<ServerBlock>::PerArrayMap* get_and_remove_per_array_map(int array_id);
	void insert_per_array_map(int array_id, IdBlockMap<ServerBlock>::PerArrayMap* map_ptr);
	void delete_per_array_map_and_blocks(int array_id);
	IdBlockMap<ServerBlock>::PerArrayMap* per_array_map(int array_id);

	/*!
	 * Restores the indicated persistent array from the file whose name was formed from the indicated label.
	 *
	 * This is a collective operation.
	 *
	 * @param array_id
	 * @param label
	 */
	void restore_persistent_array(int array_id, std::string & label){

		ArrayFile* old_file = array_files_.at(array_id);  //this is the file created based on array name
		size_t num_blocks = sip_tables_.num_blocks(array_id);
		ArrayFile::header_val_t expected_chunk_size = (sip_tables_.max_block_size(array_id)) * MIN_BLOCKS_PER_CHUNK;

		//ArrayFile constructor checks that chunk size is as expected.

		ArrayFile* file = new ArrayFile(num_blocks, expected_chunk_size, label, SIPMPIAttr::get_instance().company_communicator(), false /*new_file*/);
		array_files_.at(array_id) = file;

		//TODO at the moment, this chunk_manager should exist with correct parameters and be empty.
		//It was created when a file was created for every distributed/served array.
		//If this is changed, we need to revisit this and create manager if it is NULL
		ChunkManager* manager = chunk_managers_.at(array_id);
		//switch to ArrayFile containing saved data, delete the old one below.
		manager->file_ = file;

		eager_restore_chunks_from_index(array_id, file, chunk_managers_.at(array_id), num_blocks, data_distribution_);


		if (old_file != NULL){
			sial_warn(! old_file->is_persistent_ , "restoring on top of persistent array"
					+ old_file->file_name() + "persistence lost");
			delete old_file;
		}
	}


	/*!
	 * Reads all of the given file's chunks which belong to this server.
	 *
	 * Precondition:  the number of servers is unchanged from when the file was written.
	 *
	 * @param file
	 * @param manager
	 * @param num_blocks
	 * @param distribution
	 */
	void eager_restore_chunks_from_index(int array_id, ArrayFile* file, ChunkManager* manager, ArrayFile::header_val_t num_blocks, const DataDistribution& distribution);




	void lazy_restore_chunks_from_index(int array_id, ArrayFile* file, ChunkManager* manager, ArrayFile::header_val_t num_blocks, const DataDistribution& distribution);
	/**
	 * writes all chunks of given array to disk.  This is a collective operation.
	 * @param array_id
	 */
	void flush_array(int array_id){
		chunk_managers_.at(array_id)->collective_flush();
	}

//	/**
//	 * Marks the given array's ArrayFile object as persistent with the given label.
//	 * @param array_id
//	 * @param array_label
//	 */
//	void set_persistent(const int array_id, const std::string& array_label){
//		array_files_.at(array_id)->set_persistent(array_label);
//	}

	/**
	 * Marks the given array's ArrayFile object as persistent with the given label and
	 * then writes the blocks of the given array to disk using a a collective flush.
	 * This routine is also responsible for writing the files index.
	 *
	 * The file is closed and renamed in the ArrayFile object's destructor
	 *
	 * @param array_id
	 * @param label
	 */
	void save_persistent_array(int array_id, const std::string& label){
		ArrayFile* file = array_files_.at(array_id);
		ChunkManager* manager = chunk_managers_.at(array_id);
		size_t num_blocks = sip_tables_.num_blocks(array_id);
		file->mark_persistent(label);
		manager->collective_flush();
		std::vector<ArrayFile::offset_val_t> index_vals(num_blocks,ABSENT_BLOCK_OFFSET);
		initialize_local_index(array_id, index_vals, num_blocks);
		file->write_index(index_vals.data(), num_blocks);
	}

	friend std::ostream& operator<<(std::ostream& os, const DiskBackedBlockMap& obj);
	friend class SIPServer;
	std::ostream& gather_statistics(std::ostream& out){
		return out;
	}
//unneeded, set in constructor
//	/** Sets max_allocatable_bytes_ */
//    void set_max_allocatable_bytes(std::size_t size);

private:

	/**
	 * initialize files_ and chunk_managers_
	 *
	 * collectively opens a file and creates a chunk_manager for each distributed or served array.
	 */
	void _init();


	/**
	 * closes files, and deletes ArrayFile objects
	 * and ChunkManager objects.  Persistent files should
	 * have been handled with save_persistent_array.
	 */
	void _finalize();

    void initialize_local_index(int array_id, std::vector<ArrayFile::offset_val_t>& index_vals, size_t num_blocks){
    	IdBlockMap<ServerBlock>::PerArrayMap* array_blocks = block_map_.per_array_map(array_id);
    	IdBlockMap<ServerBlock>::PerArrayMap::iterator it;
    	for (it = array_blocks->begin(); it!=array_blocks->end(); ++it){
    		BlockId block_id = it->first;
    		std::cout << "adding block_id " << block_id << " to local index." << std::endl << std::flush;
    		int block_num = sip_tables_.block_number(it->first);
    		ServerBlock* block = it->second;
    		ArrayFile::offset_val_t offset = block->block_data_.file_offset();
    		index_vals.at(block_num) = offset;
    	}
    }


    /**
     * Creates a new block belonging to the indicated array with the given size.
     * Updates the memory counters.  The blocks data array is allocated if necessary.
     *
     * @param array_id
     * @param block_size
     * @param initialize
     * @return
     */

    ServerBlock* create_block(int array_id, size_t block_size, bool initialize);


    /**
     * Creates a new block belonging to the indicated array with the given size.
     * Updates the memory counters.
     *
     * This block has no data, but its info is in the data structures
     *
     * @param array_id
     * @param block_size
     * @param initialize
     * @return
     */

     ServerBlock* create_block_for_lazy_restore(int array_id, size_t block_size);

    size_t read_chunk_from_disk(ServerBlock* block, const BlockId& block_id);



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

    // Since the policy_ can be constructed only
    // after the block_map_ has been constructed, 
    // policy_ must appear after the declaration
    // of block_map_ here. 
//	DiskBackedArraysIO disk_backed_arrays_io_;	/*! interface to disk io for blocks */

	IdBlockMap<ServerBlock> block_map_;			/*! interface to memory block map */
    LRUArrayPolicy<ServerBlock> policy_;		/*! block replacement policy */
	std::vector<ArrayFile*> array_files_;
	std::vector<ChunkManager*> chunk_managers_;


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
	MPIMaxCounter allocated_doubles_; //init to mem size, and subtract to record min.
	MPICounter blocks_to_disk_;


};

} /* namespace sip */

#endif /* DISK_BACKED_BLOCK_MAP_H_ */
