/*
 * per_array_chunk_list.h
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

namespace sip {

class ArrayFile;

/** Entries in ChunkMap.  Each entry represents a chunk
 *
 *This is a passive container, its state is controlled by the containing ChunkList class.
 *In particular, the lifetime of the data array is managed elsewhere.
 */
class Chunk {

public:

	typedef double* data_ptr_t;
	/**
	 *
	 * @param data
	 * @param size
	 * @param valid_on_disk
	 * @param file_offset
	 */
	Chunk(data_ptr_t data, size_t size, MPI_Offset file_offset, bool valid_on_disk) :
			data_(data), num_assigned_doubles_(0), file_offset_(file_offset), valid_on_disk_(
					valid_on_disk) {
	}

	~Chunk() { //don't delete data here.  Data is owned by the owning ChunkManager
	}
private:
	data_ptr_t data_;  //pointer to beginning of chunk
	ArrayFile::offset_val_t file_offset_; //offset is in units of doubles starting at 0
	//the file view should have been set with a displacement
	//to skip the header and index.
	size_t num_assigned_doubles_; //number of doubles allocated  (remaining = chunk size - num_allocated_doubles)
	bool valid_on_disk_;

	friend class ChunkList;
	friend class ArrayFile;
	friend std::ostream& operator<<(std::ostream&, const Chunk& obj);

	DISALLOW_COPY_AND_ASSIGN(Chunk);
};

/**
 * This class maintains the set of Chunks for each array.
 *
 * It owns and is responsible for allocating and deallocating chunk data arrays.
 */
class ChunkManager {
public:
	typedef std::vector<Chunk> chunks_t;
	typedef size_t chunk_size_t;
	typedef ArrayFile::offset_val_t offset_val_t;
	typedef Chunk::data_ptr_t data_ptr_t;

	ChunkManager(chunk_size_t chunk_size) :
	chunk_size_(chunk_size), {
	}

	/**
	 * Deletes chunks managed by this list and
	 *
	 * Precondition:  All of the data arrays belonging to chunks belonging to this manager have already
	 * been deleted.  This is checked.  The reason for this decision is to allow the DiskBackedBlockMap
	 * to learn how much memory has been released.
	 *
	 * Precondition:  All of the blocks belonging to this array have already been deleted.  This guarantees that
	 * no pending operations are in progress.  This cannot be checked here.
	 */
	~ChunkManager() {
		chunks_t::iterator it;
		for (it = chunks_.begin(); it != chunks_.end(); ++it) {
			check(it->data_ == NULL, "Memory leak: attempting to delete chunk with allocated data");
		}
	}

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
	 * Assigns num_doubles from the given chunk, and returns the offset.
	 * Will try to create a new chunk if not enough space in the current one.
	 *
	 * (We will use allocate to refer to asking the OS to allocate more memory with new, and
	 * assign for allocations we manage.)
	 *
	 * Throws an exception (propagated from new_chunk) if allocating a new chunk fails.
	 * Returns number of doubles newly allocated by this call.  This is zero if there
	 * was enough space in the current chunk to satisyf the request.
	 *
	 * @param [in] num_doubles
	 * @param [in]initialize
	 * @param [out] chunk   number of chunk that holds data
	 * @param [out] offset
	 * @return number of newly allocated doubles
	 */
	size_t assign_block_data_from_chunk(size_t num_doubles, bool initialize,
			int& chunk_number, size_t& offset);


	/**
	 * Deletes data array for the given chunk (if one was allocated) and returns the amount
	 * of memory involved
	 *
	 * @param chunk
	 * @return
	 */
	size_t delete_chunk_data(Chunk& chunk);


	/**
	 * Deletes the data array for all chunks belonging to this manager.  Establishes the
	 * precondition of the destructor.
	 * @return doubles deallocated.
	 */
	size_t delete_chunk_data_all();

	//returns null if no data allocated for this chunk
	data_ptr_t get_data(int chunk_num, offset_val_t offset) const{
		Chunk& chunk = chunks_.at(chunk_num);
		if (chunk.data_ != NULL){
			return chunk.data_ + offset;
		}
		return NULL;
	}



private:
	chunks_t chunks_;
	const chunk_size_t chunk_size_;

};

} /* namespace sip */

#endif /* PER_ARRAY_CHUNK_LIST_H_ */
///**
// * Called to free chunk data, for example after chunk has been written to disk.
// * The num_allocated_doubles member is not modified.
// *
// * @param chunk
// * @return
// */
//size_t free_chunk_data(Chunk* chunk){
//	if (chunk->data_ == NULL) return 0;
//	delete [] chunk->data_;
//	chunk->data_=NULL;
//	return chunk_size_;
//}
//
//	double * get_data(Chunk* entry, size_t offset){
//		return entry->data_ + offset;
//	}
//
//	/**
//	 * Allocates num_doubles from the given chunk, and returns the offset.
//	 * Will try to allocate another chunk if not enough space in the current one.
//	 *
//	 * Throws an exception (propagated from new_chunk) if allocation fails.
//	 *
//	 *
//	 * @param num_doubles
//	 * @param initialize
//	 * @param chunk
//	 * @param offset
//	 * @return pointer to allocated data, or null if allocation failed
//	 */
//	double* allocate_data_from_chunk(size_t num_doubles, bool initialize,
//			Chunk* chunk, size_t& offset);
//
//	size_t free_chunk_data(int chunk_pos){
//		return free_chunk_data(chunk_list_.at(chunk_pos));
//	}
//
//	/**
//	 * Called to free chunk data, for example after chunk has been written to disk.
//	 * The num_allocated_doubles member is not modified.
//	 *
//	 * @param chunk
//	 * @return
//	 */
//	size_t free_chunk_data(Chunk* chunk){
//		if (chunk->data_ == NULL) return 0;
//		delete [] chunk->data_;
//		chunk->data_=NULL;
//		return chunk_size_;
//	}
//
//	/**
//	 * Called when restoring a chunk from disk.  The chunk should already
//	 * exist.
//	 *
//	 * Throws a bad alloc exception if space cannot be allocated.
//	 */
//	size_t reallocate_chunk_data(Chunk* chunk){
//		check(chunk->data_ == NULL & chunk->valid_on_disk_, "calling reallocate_chunk_data in invalid state");
//		//allocate data
//		double * chunk_data = new double[chunk_size_];
//		return chunk_size_;
//	}
//
//	void set_valid_on_disk(int chunk_pos, bool value){
//		set_valid_on_disk(chunk_list_.at(chunk_pos), value);
//	}
//	void set_valid_on_disk(Chunk* chunk, bool value){
//		chunk->valid_on_disk_=value;
//	}
//
//	/**
//	 * This procedure deletes all data for all chunks belonging to this
//	 * array, along with the list of ChunkEntry objects. This is typically
//	 * only called to free resources when an array is deleted permanently.
//	 *
//	 * @return number of doubles freed (but does not count memory used for
//	 * containers and bookkeeping.)
//	 */
//	size_t delete_all_chunks(){
//		size_t freed = chunk_size_ * chunk_list_.size();
//		ChunkList_t::iterator it = chunk_list_.begin();
//		for(;it != chunk_list_.end(); ++it){
//			free_chunk_data(*it);
//		}
//		ChunkList_t().swap(chunk_list_);   //clears chunk_list_ (clear method doesn't guarantee size is reduced)
//		check(chunk_list_.size() == 0 && chunk_list_.capacity == 0, "swap didn't clear vector");
//		return freed;
//	}
//
//	friend class DiskBackedBlockMap;
//
//private:
//	ChunkList_t chunk_list_;
//	const size_t chunk_size_;
//
//
//	/**
//	 * Allocates memory for a new chunk and updates the list data structures.
//	 * Other bookkeeping must be done by the caller.
//	 *
//	 * Throws a bad alloc exception if memory request cannot be satisfied..
//	 *
//	 * @param initialize if true, allocated data initialized to 0.
//	 * @return number of bytes allocated
//	 */
//	size_t new_chunk();
//}
//;
//} /* namespace sip */
//
//#endif /* PER_ARRAY_CHUNK_LIST_H_ */

///**
// * Delete all chunks in the list and return number of doubles freed.
// * @return
// */
//size_t delete_all_chunks() {
//	size_t doubles_freed = 0;
//	std::vector<Chunk>::iterator it = chunk_list_.begin();
//	while (it != chunk_list_.end()) {
//		if (it->data_ != NULL) {
//			doubles_freed += chunk_size_;
//		}
//		it = chunk_list_.erase(it);
//	}
//	return doubles_freed;
//}
//	//frees memory
//	//to be used after chunk is written to disk
//	void free_and_set_valid_on_disk(int chunk){
//		Chunk& entry = chunk_list_.at(current_chunk_);
//		delete [] entry.data_;
//		entry.data_ = NULL;
//		entry.next_ptr_ = NULL;
//		entry.num_allocated_doubles_ = 0;
//		entry.valid_on_disk_ = true;
//	}
//	//returns the number of total number of doubles in allocated chunks
//	size_t used_doubles(){
//		size_t num_doubles = 0;
//		std::vector<Chunk>::iterator it;
//		for (it = chunk_list_.begin(); it != chunk_list_.end(); ++it){
//			if(it->data_ != NULL){ num_doubles += chunk_size_; }
//		}
//		return num_doubles;
//	}
////called when array is deleted, return number of doubles freed
//size_t delete_all_chunks() {
//	size_t doubles_freed = 0;
//	std::vector<Chunk>::iterator it = chunk_list_.begin();
//	while (it != chunk_list_.end()) {
//		if (it->data_ != NULL) {
//			doubles_freed += chunk_size_;
//		}
//		it = chunk_list_.erase(it);
//	}
//	return doubles_freed;
//}
//
////returns pointer to the data at the given chunk and offset
//double* data(int chunk_num, size_t chunk_offset) {
//	return chunk_list_.at(chunk_num).data_ + chunk_offset;
//}
////allocates memory for a new chunk and updates its own data structures.  Other bookkeeping
//// must be done by caller.  This will throw a bad alloc exception if memory request
////cannot be satisfied, so the call should be surrounded by try-catch.
////return the number of blocks allocated.
//size_t new_chunk(bool initialize) {
//	double* chunk_data;
//	if (initialize) {
//		chunk_data = new double[chunk_size_]();
//	} else {
//		chunk_data = new double[chunk_size_];
//	}
//	int list_pos = chunk_list_.size();
//	SIPMPIAttr& mpi_attr = SIPMPIAttr::get_instance();
//	MPI_Offset mpi_offset = (mpi_attr.company_size() * list_pos
//			+ mpi_attr.company_rank()) * chunk_size_;
//	Chunk entry(chunk_data, chunk_size_, false, initialize,
//			mpi_offset);
//	chunk_list_.push_back(entry);
//	return chunk_size_;
//}
//used to reallocate data before reading from disk
//memory is not initialized to zero
//precondition:  chunk.data_ is NULL
//returns size of chunk
//size_t reallocate_chunk_data(Chunk& chunk) {
//try {
//	check(chunk.data_ == NULL,
//			"memory leak--reallocating non-NULL chunk data");
//	chunk.data_ = new double[chunk_size_];
//	chunk.num_allocated_doubles_ = 0;
//	return chunk_size_;
//}
//catch ( catch (const std::bad_alloc& ba)) {
//	//TODO fix this
//	check(false, "out of memory in reallocate_chunk_data");
//}
//}
//
//size_t free_chunk_data(Chunk& chunk) {
//if (chunk.data_ != NULL) {
//	delete[] chunk.data_;
//	chunk.data_ = NULL;
//	chunk.num_allocated_doubles_ = 0;
//	return chunk_size_;
//}
//return 0;
//}
//
//size_t free_all_chunks() {
//size_t doubles_freed = 0;
//std::vector<Chunk>::iterator it = chunk_list_.begin();
//while (it != chunk_list_.end()) {
//	doubles_freed += free_chunk_data(*it);
//	++it;
//}
//return doubles_freed;
//}
