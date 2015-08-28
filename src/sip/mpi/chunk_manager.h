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
 * This class maintains the set of Chunks for each array.
 *
 * It owns and is responsible for allocating and deallocating chunk data arrays.
 *
 * It contains a reference to the associated file in order to obtain the rank and number of
 * procs in the file's communicator.  This is used for determining the file offsets, which allow
 * a chunk-sized region of the file to be exclusively associated with a particular chunk.  The
 * calculation needs the rank and number of processes in the file's communicator.
 */
class ChunkManager {
public:
	typedef std::vector<Chunk*> chunks_t;
	typedef size_t chunk_size_t;
	typedef MPI_Offset offset_val_t;  //This must be the same as in array_file.h
	                                  //Not sure why we can't use that definition here.
	typedef Chunk::data_ptr_t data_ptr_t;
	typedef size_t block_num_t;

	ChunkManager(chunk_size_t chunk_size, ArrayFile* file):
		chunk_size_(chunk_size)
	,   file_(file){}


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
			check((*it)->data_ == NULL, "Memory leak: attempting to delete chunk with allocated data");
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
	size_t reallocate_chunk_data(int chunk_number){
		return reallocate_chunk_data(chunks_.at(chunk_number));
	}


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
	 * TODO may be able to eliminate either chunk or chunk_number from this signature
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


	void lazy_assign_block_data_from_chunk(size_t num_doubles,
			int& chunk_number, offset_val_t& offset);
	/**
	 * Deletes data array for the given chunk (if one was allocated) and returns the amount
	 * of memory involved.
	 *
	 * Waits for all pending operations on blocks belonging to the chunk.
	 *
	 * @param chunk
	 * @return doubles deallocated
	 */
	size_t delete_chunk_data(Chunk* chunk);


	/**
	 * Deletes the data array for all chunks belonging to this manager.  Establishes the
	 * precondition of the destructor.
	 *
	 * Waits for all pending operations on blocks belonging to the chunk.
	 *
	 * @return doubles deallocated.
	 */
	size_t delete_chunk_data_all();

	//returns null if no data allocated for this chunk
	data_ptr_t get_data(int chunk_num, offset_val_t offset) const{
		const Chunk* chunk = chunks_.at(chunk_num);
		if (chunk->data_ != NULL){
			return chunk->data_ + offset;
		}
		return NULL;
	}

	void set_valid_on_disk(int chunk_number, bool val){
		chunks_.at(chunk_number)->valid_on_disk_ = val;
	}

	offset_val_t block_file_offset(int chunk_number, offset_val_t offset){
		return chunks_.at(chunk_number)->file_offset_ + offset;
	}


	Chunk*  chunk(int chunk_number) {
		return chunks_.at(chunk_number);
	}

	int num_chunks() const{
		return chunks_.size();
	}

	void wait_all(Chunk* chunk);


	/**
	 * Write all invalid chunks from this array to disk.
	 *
	 * This is a collective operation
	 */
	void collective_flush(){

		//determine max number of chunks to write by any server
		chunks_t::const_iterator it;
		int num_chunks = 0;
		for (it = chunks_.begin(); it != chunks_.end(); ++it){
			if(  ! (*it)->valid_on_disk_ ){
				num_chunks ++;
			}
		}
		int max;
		MPI_Allreduce(&num_chunks, &max, 1, MPI_INT, MPI_MAX, file_->comm_);

		//write own chunks that need writing
		chunks_t::iterator wit;
		int written = 0;
		for (wit = chunks_.begin(); wit != chunks_.end(); ++wit){
			if ( ! (*wit)->valid_on_disk_){
				file_->chunk_write_all(**wit);
				(*wit)->valid_on_disk_=true;
				written ++;
			}
		}

		//call noop collective write for remaining chunks at other servers.
		for (int i = written; i < max; ++i){
			file_->chunk_write_all_nop();
		}

	}


//	/**
//	 * This routine can be used when the number of servers is now the same as when the
//	 * file was written.
//	 *
//	 * @param num_blocks
//	 * @param distribution
//	 */
//	std::pair<ChunkManager*, size_t> eager_restore_chunks_from_index(ArrayFile::header_val_t num_blocks, const DataDistribution& distribution){
//		//read the index from the array file
//		ArrayFile::offset_val_t index1[num_blocks];
//		file_->read_index(index1, num_blocks);
//
//		//create two maps offset to block number for blocks belonging to this server
//		//the first map contains offsets that correspond to the beginning of a chunk
//		//the second contains offsets that are within a chunk
//
//		std::map<offset_val_t, block_num_t> chunk_begin_map;
//		std::map<offset_val_t, block_num_t> chunk_interior_map;
//
//		for (int i = 0; i < num_blocks; i++){
//			offset_val_t offset = index[i];
//		   if (offset != != 0 && distribution.is_my_block(i)){
//			   if (offset % chunk_size_ == 0){
//				   chunk_begin_map[offset]=i;
//			   }
//			   else {
//				   chunk_interior_map[offset]=i;
//			   }
//		   }
//		}
//		//determine number of iterations of collective read
//		int max_num_chunks;
//		int num_chunks = chunk_begin_map.size();
//		MPI_Allreduce(&num_chunks, &max_num_chunks, 1, MPI_INT, MPI_MAX, file_->comm_);
//
//		//create chunks and read their data
//
//		std::map<offset_val_t, size_t>::iterator it;
//		int read_count = 0;
//		size_t allocated_doubles = 0;
//		for (it = chunk_begin_map.begin(); it != chunk_begin_map.end(); ++it){
//			offset_val_t = it->first;
//			int block_num = it->second;
//			BlockId block_id(array_id, block_num);
//			ServerBlock* block = disk_backed_block_map.get_block_for_writing(block_id);
//			Chunk* chunk = block.get_chunk();
//			if (num_servers_match && this is start of chunk){
//   			 file_->chunk_read_all(*chunk);
//			 read_count++;
//			}
//			else {
//				file_->block_read_all(*chunk, block->offset, block_size);
//			}
//
//		}
//		//noops in case other servers still need to read
//		for (int j = read_count; j != max_num_chunks; ++j){
//			file_->chunk_read_all_nop();
//		}
//
//
//	}

	/**
	 * This routine is used when the number of servers is different from when the file was written
	 * It will create a new file and chunk_manager.
	 *
	 * @param num_blocks
	 * @param distribution
	 * @return
	 */
	ChunkManager* restore_blocks_from_index(ArrayFile::header_val_t num_blocks, const DataDistribution& distribution, ArrayFile** new_file){
		check(false, "restore_blocks_from_index not implemented");
		return NULL;
	}


//	void init_offset_block_map(std::map<offset_val_t, size_t> offset_block_map, const DataDistribution& distribution){
//		offset_val_t index1[num_blocks_];
//		std::fill(index1, index1 + num_blocks_,0);
//		read_index(index1, num_blocks_);
//		for (header_val_t i = 0; i < num_blocks_; i++){
//			if (distribution.is_my_block(i)){
//				offset_block_map[index1[i]] = i;
//			}
//		}
//	}

	//This is a collective operation
	int max_num_chunks() const{
		int  num = chunks_.size();
		int max;
		MPI_Allreduce(&num, &max, 1, MPI_INT, MPI_MAX, file_->comm_);
		return max;
	}

	chunk_size_t chunk_size(){ return chunk_size_;}

	friend std::ostream& operator<<(std::ostream& os, const ChunkManager& obj);
	friend class DiskBackedBlockMap;

private:
	chunks_t chunks_;
	const chunk_size_t chunk_size_;
	ArrayFile* file_;

	offset_val_t chunk_offset(int chunk_number) const{
		return (file_->comm_size() * chunk_number + file_->comm_rank()) * chunk_size_;
	}

	DISALLOW_COPY_AND_ASSIGN(ChunkManager);



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
