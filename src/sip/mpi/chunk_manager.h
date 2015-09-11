/*
 * chunk_manager.h
 *
 *  Created on: Jul 21, 2015
 *      Author: basbas
 */

#ifndef PER_ARRAY_CHUNK_LIST_H_
#define PER_ARRAY_CHUNK_LIST_H_

#include <limits>
#include <vector>
#include "sip_mpi_attr.h"
#include "data_distribution.h"
#include "array_file.h"
#include "chunk.h"


namespace sip {

class DiskBackedBlockMap;

/**
 * This class maintains the set of Chunks for a single array.
 *
 * It owns and is responsible for allocating and deallocating chunk data arrays.
 *
 * A reference to the associated file is needed in order to obtain the rank and number of
 * procs in the file's communicator and to restore chunks when necessary to maintain certain
 * invariants.  The rank and number of servers are used for determining the file offsets, which allow
 * a chunk-sized region of the file to be exclusively associated with a particular chunk.
 *
 * The memory for an array is allocated in units of a Chunk.  Blocks are
 * assigned space in a chunk, with new chunks created as needed.
 *
 * Invariant:  All blocks belonging to a chunk have valid data
 * in memory or the chunk's data pointer is NULL.
 *
 * Invariant:  If the chunk's data pointer is NULL, then the data has been
 * backed up on disk and the chunk's valid_on_disk flag is set.  (A chunk
 * may be valid_on_disk and valid in memory at the same time)
 */
class ChunkManager {
public:
	typedef std::vector<Chunk*> chunks_t;
	typedef size_t chunk_size_t;
	typedef MPI_Offset offset_val_t;  //This must be the same as in array_file.h
	                                  //Not sure why we can't use that definition here.
	typedef Chunk::data_ptr_t data_ptr_t;
	typedef size_t block_num_t;

	ChunkManager(chunk_size_t chunk_size, ArrayFile* file);


	/**
	 * Deletes chunks managed by this list
	 *
	 * Precondition:  All of the data arrays belonging to chunks belonging to this manager have already
	 * been deleted.  This is checked.  The reason for this decision is to allow the DiskBackedBlockMap
	 * to learn how much memory has been released.
	 *
	 * Precondition:  All of the blocks belonging to this array have already been deleted.  This guarantees that
	 * no pending operations are in progress.  This cannot be checked here.
	 */
	~ChunkManager();

	/**
	 * Allocates a data array for a new chunk and updates the manager's state.
	 * Other bookkeeping must be done by the caller.
	 *
	 * The offset of the new chunk is determined from the number of servers, the chunk_size,
	 * and the number of chunks already created by this manager.  Chunks are stored in an
	 * ordered list so that the offsets increase with chunk numbers.
	 *
	 * Throws a bad alloc exception if memory request cannot be satisfied.
	 * Typically the caller may try to free some memory and try again.
	 *
	 * The data array is not initialized to zero.
	 *
	 * @return number of doubles allocated
	 */
	size_t new_chunk();

	/**
	 * Similar to new_chunk, but does not allocate data.  This is used for lazy
	 * restoration of persistent arrays
	 *
	 * @return
	 */
	void new_chunk_for_restore(offset_val_t offset);


	/** reallocates data array for an existing chunk for which it has been deleted.
	 * precondition:
	 *
	 * The allocated data is not initialized to zero
	 *
	 * @param chunk
	 * @return
	 */
	size_t reallocate_chunk_data(Chunk* chunk);
	size_t reallocate_chunk_data(int chunk_number);


	/**
	 *
	 * 	/**
	 * Assigns num_doubles from the given chunk, and returns the number of doubles created.
	 * Will try to create a new chunk if not enough space in the current one.
	 *
	 * (We will use allocate to refer to asking the OS to allocate more memory with new, and
	 * assign for allocations we manage.)
	 *
	 * Throws an exception (propagated from new_chunk) if allocating a new chunk fails.
	 * Returns number of doubles newly allocated by this call.  This is zero if there
	 * was enough space in the current chunk to satisfy the request.
	 *
	 * If the chunk is not in memory and is valid on disk, read it in.
	 *
	 * @param[in] block  pointer to block whose data is being assigned
	 * @param[in] num_doubles  amount of data required
	 * @param[in] initialize  whether newly assigned data should be initialized to zero
	 * @param[out] chunk  chunk that owns the assigned data
	 * @param[out]  chunk_number  number of chunk that owns the assigned data
	 * @param[out] offset   offset into the chunk
	 * @return  number of newly allocated doubles
	 */
	size_t assign_block_data_from_chunk(ServerBlock* block, size_t num_doubles, bool initialize, Chunk*& chunk,
			int& chunk_number, offset_val_t& offset);

	/**
	 * Calls above with NULL block, throws away the chunk
	 * @param num_doubles
	 * @param initialize
	 * @param chunk_number
	 * @param offset
	 * @return
	 */
	size_t assign_block_data_from_chunk(size_t num_doubles, bool initialize,
			int& chunk_number, offset_val_t& offset);

	/**
	 * Calls above with NULL block
	 * @param num_doubles
	 * @param initialize
	 * @param chunk_number
	 * @param offset
	 * @return
	 */
	size_t assign_block_data_from_chunk(size_t num_doubles, bool initialize,
			int& chunk_number, offset_val_t& offset, Chunk*& chunk);


	//UNTESTED
	void lazy_assign_block_data_from_chunk(size_t num_doubles,
			int& chunk_number, offset_val_t& offset);


	/**
	 * Deletes data array for the given chunk (if one was allocated) and returns the amount
	 * of memory involved.
	 *
	 * Precondition:  There are no pending operations on blocks of the chunk
	 *
	 * @param chunk
	 * @return doubles deallocated
	 */
	size_t delete_chunk_data(Chunk* chunk);


	/**
	 * Deletes the data array for all chunks belonging to this manager.  Establishes the
	 * precondition of the destructor.
	 *
	 * Waits for any pending operations on blocks of the array.
	 *
	 * @return doubles deallocated.
	 */
	size_t delete_chunk_data_all();

	/**
	 * Returns a pointer to the data at the given offset of the given
	 * chunk.
	 *
	 * Currently used only in tests.
	 *
	 * @param chunk_num
	 * @param offset
	 * @return
	 */
	data_ptr_t get_data(int chunk_num, offset_val_t offset) const;


	/**
	 * Sets the valid_on_disk flag of the given chunk to the given value
	 *
	 * @param chunk_number
	 * @param val
	 */
	void set_valid_on_disk(int chunk_number, bool val);

	offset_val_t block_file_offset(int chunk_number, offset_val_t offset);


	Chunk*  chunk(int chunk_number) ;

	int num_chunks() const;

	void wait_all(Chunk* chunk);


	/**
	 * Write all chunks from this array to disk.  Chunks that are already valid_on_disk are
	 * no rewritten.
	 *
	 * This is a collective operation
	 */
	void collective_flush();


	/**
	 * Restores all chunks that are not in memory from disk.
	 * Returns number of doubles that were allocated.
	 *
	 * This is a collective operation
	 */
	size_t collective_restore();

	/**
	 * Determines the maximum number of chunks on any server.
	 *
	 * This is a collective operation
	 *
	 * @return
	 */
	int max_num_chunks() const;

	/**
	 * Returns the number of doubles in each chunk
	 *
	 * @return
	 */
	chunk_size_t chunk_size(){ return chunk_size_;}

	friend std::ostream& operator<<(std::ostream& os, const ChunkManager& obj);
	friend class DiskBackedBlockMap;

private:
	chunks_t chunks_;
	const chunk_size_t chunk_size_;
	ArrayFile* file_;

	/**
	 * Returns the offset in the file of the indicated chunk.
	 *
	 * The offset is relative to the "data view" in the file, so chunk 0 has offset 0.
	 *
	 * @param chunk_number
	 * @return
	 */
	offset_val_t chunk_offset(int chunk_number) const;

	DISALLOW_COPY_AND_ASSIGN(ChunkManager);



};


/**Inlined functions*/

inline size_t ChunkManager::reallocate_chunk_data(int chunk_number){
	return reallocate_chunk_data(chunks_.at(chunk_number));
}

inline ChunkManager::data_ptr_t ChunkManager::get_data(int chunk_num, offset_val_t offset) const{
	const Chunk* chunk = chunks_.at(chunk_num);
	if (chunk->data_ != NULL){
		return chunk->data_ + offset;
	}
	return NULL;
}

inline void ChunkManager::set_valid_on_disk(int chunk_number, bool val){
	chunks_.at(chunk_number)->valid_on_disk_ = val;
}

inline ChunkManager::offset_val_t ChunkManager::block_file_offset(int chunk_number, offset_val_t offset){
	return chunks_.at(chunk_number)->file_offset_ + offset;
}


inline Chunk*  ChunkManager::chunk(int chunk_number) {
	return chunks_.at(chunk_number);
}

inline int ChunkManager::num_chunks() const{
	return chunks_.size();
}


inline ChunkManager::offset_val_t ChunkManager::chunk_offset(int chunk_number) const{
	return (file_->comm_size() * chunk_number + file_->comm_rank()) * chunk_size_;
}

} /* namespace sip */

#endif /* PER_ARRAY_CHUNK_LIST_H_ */


