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

#include "counter.h"
#include "array_file.h"
#include "chunk_manager.h"

namespace sip {
class BlockId;
class ServerPersistentArrayManager;

/**
 * Wrapper over block map for servers.
 * Supports backing on disk
 *
 * This class is responsible for maintaining the measures of remaining memory, and
 * for updating the status of individual blocks as they are modified, written and read from
 * disks, etc.
 *
 * All block-sized memory allocation including, for example, temp buffers for asynchronous
 * ops, should happen via this class.
 *
 * Invariants:  each distributed/served array has exactly one open file associated with it,
 * whose ArrayFile object is in the array_files_ vector.  The ArrayFile objects are created
 * in this classe's constructor (which opens the file) and destroyed (which closes and
 * possibly deletes the file) in this class's destructor.  The ArrayFile object is replaced
 * with a new one, and the old one destroyed, when an array is restored.
 */
class DiskBackedBlockMap {
public:
	typedef size_t block_num_t;
	typedef ArrayFile::offset_val_t offset_val_t;

	static const int BLOCKS_PER_CHUNK;


	DiskBackedBlockMap(const SipTables&, const SIPMPIAttr&,
			const DataDistribution&);
	~DiskBackedBlockMap();

	/**
	 * Interface with server loop.
	 * Returns block for operations that will not modify the block
	 * Precondition:  Block exists in map and in memory or on disk.
	 * If necessary, the block is retrieved from disk.
	 * This routine waits for pending write operations on block to complete.
	 *
	 * @param block_id
	 * @param pc
	 * @return  pointer to block with initialized memory.
	 */
	ServerBlock* get_block_for_reading(const BlockId& block_id, int pc);


	/**
	 * Interface with server loop.
	 * Returns block for operations that will overwrite block.
	 * If the block does not exist, it will be created and data memory assigned and allocated.
	 * Newly allocated memory is not initialized.
	 * Waits for all pending asynchronous operations.
	 * Invalidates disk copy
	 *
	 * @param block_id
	 * @param pc
	 * @return  pointer to block with possibly uninitialized memory.
	 */

	ServerBlock* get_block_for_writing(const BlockId& block_id);

	/**
	 * Interface with server loop.
	 * Returns block for operations that accumulate into a block
	 * If the block does not exist, it will be created and data memory assigned and allocated.
	 * Newly allocated memory is initialized to 0
	 * Does NOT wait for pending asynchronous operations.
	 * Invalidates disk copy
	 *
	 * @param block_id
	 * @return  pointer to block with initialized memory
	 */
	ServerBlock* get_block_for_accumulate(const BlockId& block_id);

	/**
	 * Allocate the given number of double precision values and update memory usage accounting.
	 * This should be called instead of new to allocate temporary data for asynchronous operations.
	 *
	 * If not enough memory is available, the routine will backup blocks on disk and free their
	 * memory.
	 *
	 * @param size
	 * @param initialize   if true, allocated data will be initialized to 0
	 * @return
	 */
	double* allocate_data(size_t size, bool initialize); //

	/**
	 * Frees data that was allocated using allocate_data.
	 * Sets the caller's data pointer to null.
	 * Updates memory accounting.
	 *
	 * @param data
	 * @param size
	 */
	void free_data(double*& data, size_t size);

	/**
	 * Manages the entries for entire arrays in the block map.
	 */

	/**
	 * Inserts the given map into the delegate block map at the slot belonging to the
	 * indicated array.  Delegates to IdBlockMap.
	 *
	 * NOTE:  Currently unused.  But could be used if functionality that keeps persistent arrays
	 * in memory between sial programs is restored.
	 *
	 * @param array_id
	 * @param map_ptr
	 */
	void insert_per_array_map(int array_id,
			IdBlockMap<ServerBlock>::PerArrayMap* map_ptr);

	/**
	 * Returns a pointer to the block map for the indicated array.  If a
	 * map for the array does not exist, a new one will be created.  This routine
	 * never returns null
	 *
	 * Delegates to IdBlockMap
	 *
	 * @param array_id
	 * @return
	 */
	IdBlockMap<ServerBlock>::PerArrayMap* per_array_map(int array_id);


	/**
	 * Returns a pointer to the block map for the indicated array, or null if the
	 * map for the array does not exist
	 *
	 * Delegates to IdBlockMap
	 *
	 * @param array_id
	 * @return
	 */
	IdBlockMap<ServerBlock>::PerArrayMap* per_array_map_or_null(int array_id);


	/**
	 * Deletes the indicated array.
	 *
	 * All blocks for array are removed from the policy object, all blocks are deleted and
	 * their map is removed from the block map (via a call to the delegate IdBlockMap).
	 * Memory accounting is updated.    The backing disk file is neither modified nor deleted.
	 * @param array_id
	 */
	void delete_per_array_map_and_blocks(int array_id);

	/** Routines for persistent arrays
	 *
	 */

	/**
	 * Marks the given array's ArrayFile object as persistent with the given label and
	 * then writes the blocks of the given array to disk using a a collective flush.
	 * This routine is also responsible for writing the file's index.
	 *
	 * This routine is called by the persistent array manager on completion of the server loop.
	 *
	 * The file is closed and renamed with a filename derived from the label
	 *
	 * @param array_id
	 * @param label
	 */
	void save_persistent_array(int array_id, const std::string& label);

	/**
	 * Restores the indicated persistent array from the
	 * file whose name was formed from the indicated label.
	 *
	 * If restoration is eager, a new file is created and the old file is deleted.
	 * If the array has undergone disk backing, traverse blocks and restore those first.
	 *
	 * TODO   Currently, this routine requires that the number of servers when the file
	 * was save is the same as the number of servers now.
	 *
	 * TODO  Only eager restoration is currently implemented.
	 *
	 * @param array_id  "destination" array
	 * @param label     label used to save array in previous sial program
	 * @param eager     if true, all blocks of array are collectively restored.  Otherwise,
	 *                     chunks are read in when accessed.
	 */
	void restore_persistent_array(int array_id, const std::string & label, bool eager,
			int pc);


	/**
	 * writes all chunks of given array to disk.  This is a collective operation.
	 * @param array_id
	 */
	void flush_array(int array_id);

	friend std::ostream& operator<<(std::ostream& os,
			const DiskBackedBlockMap& obj);
	friend class SIPServer;

	/**
	 * Encapsulates timers and printers for this class.
	 *
	 * If owned classes have their own timers,
	 * this gather_and_print_statistics should invoke theirs.
	 */
	struct Stats {
		MPIMaxCounter allocated_doubles_;
		MPICounter blocks_to_disk_;
		MPICounter num_restored_arrays_with_disk_backing_;
		MPICounterList per_array_local_blocks_;

		explicit Stats(const MPI_Comm& comm, DiskBackedBlockMap* parent) :
				allocated_doubles_(comm), blocks_to_disk_(comm), num_restored_arrays_with_disk_backing_(
						comm), per_array_local_blocks_(comm, parent->sip_tables_.num_arrays()) {
		}

		void finalize(DiskBackedBlockMap* parent){
			for (int i = 0; i < parent->sip_tables_.num_arrays(); ++i){
				IdBlockMap<ServerBlock>::PerArrayMap* map =
						parent->per_array_map(i);
				if (map != NULL) {
					per_array_local_blocks_.inc(i, map->size());
				}
			}
		}

		std::ostream& gather_and_print_statistics(std::ostream& os,
				DiskBackedBlockMap* parent) {
			allocated_doubles_.gather();
			blocks_to_disk_.gather();
			num_restored_arrays_with_disk_backing_.reduce();
			per_array_local_blocks_.gather();

			if (SIPMPIAttr::get_instance().is_company_master()) {
				os << std::endl << "allocated_doubles_" << std::endl;
				os << allocated_doubles_;
				os << std::endl << "blocks_to_disk_" << std::endl;
				os << blocks_to_disk_;
				os << std::endl << "num_restored_arrays_with_disk_backing_"
						<< std::endl;
				os << num_restored_arrays_with_disk_backing_;
				os << std::endl << "per_array_local_blocks_" << std::endl;
				os << per_array_local_blocks_;
			}

			for (int i = 0; i < parent->sip_tables_.num_arrays(); ++i) {
				IdBlockMap<ServerBlock>::PerArrayMap* map =
						parent->per_array_map(i);
				if (map != NULL) {
					ArrayFile* array_file = parent->array_files_.at(i);
					if (array_file != NULL) {
						os << "Array " << i << ":"
								<< parent->sip_tables_.array_name(i) << std::endl;
						array_file->stats_.gather_and_print_statistics(os,
								array_file);
					}
				}
			}
			return os;
		}
	};

	/**
	 * Called by DiskBackedBlockMap destructor
	 *
	 * Deletes ArrayFile and ChunkManager objects.  The former closes the file.  Non-persistent
	 * files are deleted, persistent files are renamed.
	 *
	 * Precondition:  save_persistent_array has been called for each persistent array.
	 * Precondition to avoid memory leaks:  all blocks have been deleted
	 */
	void _finalize();

private:


	/**
	 * Called by DiskBackedBlockMap constructor
	 *
	 * initializes files_ and chunk_managers_
	 *
	 * collectively opens a file and creates a chunk_manager for each distributed or served array.
	 */
	void _init();



	/**
	 * Creates a new block belonging to the indicated array with the given size,
	 * and adds it to the block map.
	 *
	 * Assigns memory for data from a chunk, and ensures that the memory has been allocated.
	 * This may require reading from disk if blocks in same chunk exist and chunk has been
	 * written to disk.
	 *
	 * If initialize, the memory is initialized to 0
	 *
	 * Memory usage counters are updated.
	 *
	 * Establishes the invariants between ServerBlock and Chunk
	 *
	 * @param array_id
	 * @param block_size
	 * @param initialize
	 * @return
	 */


	ServerBlock* create_block(int array_id, size_t block_size, bool initialize);

	/**
	 * Gets block for restore.  If the block is not already in the map, it is created
	 * and added to the map.
	 *
	 * @param block_id
	 * @param array_id
	 * @param block_size
	 * @return
	 */
	ServerBlock* get_block_for_restore(BlockId block_id, int array_id,
			size_t block_size);

	/**
	 * Reads chunk containing the indicated block from disk.
	 *
	 * Returns the number of doubles allocated.
	 *
	 * @param block
	 * @param block_id
	 * @return
	 */
	size_t read_chunk_from_disk(ServerBlock* block, const BlockId& block_id);

	/**
	 * Frees the requested number of doubles by writing chunk data to disk and updating
	 * the "valid_on_disk" flag for those chunks.
	 *
	 * Returns the actual number of doubles freed.  The caller is responsible for using
	 * the returned value for memory accounting.
	 *
	 * @param requested_doubles_to_free
	 * @return
	 */
	size_t backup_and_free_doubles(size_t requested_doubles_to_free);

	/**
	 * Traverse the map for the given array and construct an index of block offsets.
	 * For blocks owned by this server, the value is the offset (relative to the beginning of
	 * the data part) of the file.  For unformed blocks, or blocks not owned by this server,
	 * the value is ABSENT_BLOCK_OFFSET
	 *
	 * @param array_id
	 * @param index_vals
	 * @param num_blocks
	 */
	void initialize_local_index(int array_id,
			std::vector<ArrayFile::offset_val_t>& index_vals,
			size_t num_blocks);

	void initialize_local_sparse_index(int array_id, std::vector<ArrayFile::offset_val_t>& index_vals);


	/*!
	 * Collectively reads all of the given file's chunks which belong to this server.
	 *
	 * Precondition:  the number of servers is unchanged from when the file was written.
	 * TODO relax this requirement
	 *
	 * @param file
	 * @param manager
	 * @param num_blocks
	 * @param distribution
	 */
	void eager_restore_chunks_from_index(int array_id, ArrayFile* file, std::vector<ArrayFile::offset_val_t>& index,
			ChunkManager* manager, ArrayFile::header_val_t num_blocks,
			const DataDistribution& distribution);


	void eager_restore_chunks_from_sparse_index(int array_id, ArrayFile* file, std::vector<ArrayFile::offset_val_t>& index,
			ChunkManager* manager,
			const DataDistribution& distribution);
	/**
	 * UNTESTED!!!!
	 * Creates an entry in the block map and assigns chunk data for each block, which is marked,
	 * valid on disk.  The chunk is read only when one of its blocks is accessed.
	 *
	 * @param array_id
	 * @param file
	 * @param manager
	 * @param num_blocks
	 * @param distribution
	 */
	void lazy_restore_chunks_from_index(int array_id, ArrayFile* file,
			ChunkManager* manager, ArrayFile::header_val_t num_blocks,
			const DataDistribution& distribution);
	/**
	 * UNTESTED!!!
	 * Like get_block_for_writing
	 * If block does not exist, it will be created and
	 * data memory will be assigned, but not allocated.
	 * @param block_id
	 * @return
	 */
	ServerBlock* get_block_for_lazy_restore(const BlockId& block_id);

	/**
	 * UNTESTED!!!
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

	const SipTables &sip_tables_;
	const SIPMPIAttr & sip_mpi_attr_;
	const DataDistribution &data_distribution_;

	// Since the policy_ can be constructed only
	// after the block_map_ has been constructed,
	// policy_ must appear after the declaration
	// of block_map_ here.

	IdBlockMap<ServerBlock> block_map_;
	LRUArrayPolicy<ServerBlock> policy_;
	std::vector<ArrayFile*> array_files_;
	std::vector<ChunkManager*> chunk_managers_;
	std::vector<bool> disk_backing_; //indicates whether array has been involved in disk backing

	/** Maximum number of bytes before spilling over to disk */
	std::size_t max_allocatable_bytes_;
	std::size_t max_allocatable_doubles_;

	/** Remaining number of double precision numbers that can be allocated
	 * without exceeding max_allocatable_bytes_
	 *
	 * There are a few places where this value can be negative.
	 *
	 * This value is updated whenever memory is allocated or released in allocating the
	 * data portion of a block, or the temp buffer for an asynchronous operation.
	 */

	long remaining_doubles_;


	Stats stats_;

};

//Inlined functions

inline void DiskBackedBlockMap::insert_per_array_map(int array_id,
		IdBlockMap<ServerBlock>::PerArrayMap* map_ptr) {
	block_map_.insert_per_array_map(array_id, map_ptr);
}

inline IdBlockMap<ServerBlock>::PerArrayMap* DiskBackedBlockMap::per_array_map(
		int array_id) {
	return block_map_.per_array_map(array_id);
}


inline void DiskBackedBlockMap::flush_array(int array_id) {
	chunk_managers_.at(array_id)->collective_flush();
}

} /* namespace sip */

#endif /* DISK_BACKED_BLOCK_MAP_H_ */
