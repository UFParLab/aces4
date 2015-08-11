/*
 * per_array_chunk_list.h
 *
 *  Created on: Jul 21, 2015
 *      Author: basbas
 */

#ifndef PER_ARRAY_CHUNK_LIST_H_
#define PER_ARRAY_CHUNK_LIST_H_

#include <limits>
#include "sip_mpi_attr.h"
#include "data_distribution.h"

namespace sip {


class ServerBlock;

/** Entries in ChunkMap.  Each entry represents a chunk
 *
 */
class ChunkListEntry {

private:
	ChunkListEntry(double* data, size_t size, bool valid_on_disk,
			bool initialized, MPI_Offset file_offset) :
			data_(data), next_ptr_(data), num_allocated_doubles_(size), valid_on_disk_(
					valid_on_disk), initialized_(initialized), file_offset_(
					file_offset) {
	}

	//while this frees the data, it doesn't do any additional accounting
	~ChunkListEntry() {
		if (data_ != NULL)
			delete[] data_;
		data_ = NULL;
	}

	void free_data() {
		delete[] data_;
		data_ = NULL;
	}

	double* data_;  //pointer to beginning of chunk
	double* next_ptr_;
	size_t num_allocated_doubles_; //number of doubles (including padding for alignment)
	bool valid_on_disk_;
	bool initialized_;  //if initialized to zero when chunk created.
	MPI_Offset file_offset_;  //offset is in units of doubles starting at 0
							  //the file view should have been set with a displacement
							  //to skip the header.

	friend class ChunkList;
	friend class DiskBackedBlockMap;
	friend class ServerBlock;
	friend class DiskBackedArraysIO;
	friend std::ostream& operator<<(std::ostream&, const ChunkListEntry &);
	friend std::ostream& operator<<(std::ostream& os, const ServerBlock& block);
};

/** per array list of chunks */
class ChunkList {
public:
	typedef std::vector<ChunkListEntry> ChunkList_t;

	ChunkList(size_t max_block_size, size_t chunk_size,
			const DataDistribution& data_distribution) :
			max_block_size_(max_block_size), chunk_size_(chunk_size), current_chunk_(
					0), data_distribution_(data_distribution) {
		check(max_block_size < (size_t)std::numeric_limits<int>::max(),
				"Chunk size exceeds implementation limit (which currently is the max value represented by int");
	}

	~ChunkList() {
		//this will delete the chunk_list_, which will also delete the data
	}

	bool has_space_in_current_chunk(size_t num_doubles) {
		ChunkListEntry& entry = chunk_list_.at(current_chunk_);
		return chunk_size_ - entry.num_allocated_doubles_ > num_doubles;
	}



	/**
	 * allocates num_doubles from the given chunk, and returns the offset.
	 * Precondition:
	 *
	 * @param num_doubles
	 * @param initialize
	 * @param chunk
	 * @param offset
	 * @return
	 */
	double* allocate_data_from_chunk(size_t num_doubles, bool initialize,
			ChunkListEntry& chunk, size_t& offset) {
		ChunkListEntry& entry = chunk_list_.at(current_chunk_);

		check(has_space_in_current_chunk(num_doubles),
				"not enough space in chunk");

		double * data = entry.next_ptr_;
		entry.next_ptr_ += num_doubles;
		entry.num_allocated_doubles_ += num_doubles;
		if (initialize && !entry.initialized_) {
			std::fill(data, data + num_doubles, 0);
		}
		return data;
	}



//	//frees memory
//	//to be used after chunk is written to disk
//	void free_and_set_valid_on_disk(int chunk){
//		ChunkListEntry& entry = chunk_list_.at(current_chunk_);
//		delete [] entry.data_;
//		entry.data_ = NULL;
//		entry.next_ptr_ = NULL;
//		entry.num_allocated_doubles_ = 0;
//		entry.valid_on_disk_ = true;
//	}

//	//returns the number of total number of doubles in allocated chunks
//	size_t used_doubles(){
//		size_t num_doubles = 0;
//		std::vector<ChunkListEntry>::iterator it;
//		for (it = chunk_list_.begin(); it != chunk_list_.end(); ++it){
//			if(it->data_ != NULL){ num_doubles += chunk_size_; }
//		}
//		return num_doubles;
//	}

	//called when array is deleted, return number of doubles freed
	size_t delete_all_chunks() {
		size_t doubles_freed = 0;
		std::vector<ChunkListEntry>::iterator it = chunk_list_.begin();
		while (it != chunk_list_.end()) {
			if (it->data_ != NULL) {
				doubles_freed += chunk_size_;
			}
			it = chunk_list_.erase(it);
		}
		return doubles_freed;
	}

	//returns pointer to the data at the given chunk and offset
	double* data(int chunk_num, size_t chunk_offset) {
		return chunk_list_.at(chunk_num).data_ + chunk_offset;
	}

	//allocates memory for a new chunk and updates its own data structures.  Other bookkeeping
	// must be done by caller.  This will throw a bad alloc exception if memory request
	//cannot be satisfied, so the call should be surrounded by try-catch.
	//return the number of blocks allocated.
	size_t new_chunk(bool initialize) {
		double* chunk_data;
		if (initialize) {
			chunk_data = new double[chunk_size_]();
		} else {
			chunk_data = new double[chunk_size_];
		}
		int list_pos = chunk_list_.size();
		SIPMPIAttr& mpi_attr = SIPMPIAttr::get_instance();
		MPI_Offset mpi_offset = (mpi_attr.company_size() * list_pos
				+ mpi_attr.company_rank()) * chunk_size_;
		ChunkListEntry entry(chunk_data, chunk_size_, false, initialize,
				mpi_offset);
		chunk_list_.push_back(entry);
		return chunk_size_;
	}

//	//reallocates memory for an existing chunk.  Throws a bad alloc exception if
//	//request cannot be satisfied.
//	void set_chunk_data(int chunk, double* chunk_data){
//		ChunkListEntry& entry = chunk_list_.at(chunk);
//		entry.data_ = chunk_data;
//		entry.next_ptr_ = chunk_data;
//	}

	//used to reallocate data before reading from disk
	//memory is not initialized to zero
	//precondition:  chunk.data_ is NULL
	//returns size of chunk
	size_t reallocate_chunk_data(ChunkListEntry& chunk) {
		check(chunk.data_ == NULL,
				"memory leak--reallocating non-NULL chunk data");
		chunk.data_ = new double[chunk_size_];
		return chunk_size_;
	}

//	size_t free_chunk_data(ChunkListEntry& chunk) {
//		delete[] chunk.data_;
//		chunk.data_ = NULL;
//		return chunk_size_;
//	}

	size_t free_all_chunk_data() {
		size_t doubles_freed = 0;
		std::vector<ChunkListEntry>::iterator it = chunk_list_.begin();
		while (it != chunk_list_.end()) {
			if (it->data_ != NULL) {
				doubles_freed += chunk_size_;
				delete[] it->data_;
				it->data_ = NULL;
			}
			++it;
		}
		return doubles_freed;
	}

	friend class DiskBackedBlockMap;

private:
	std::vector<ChunkListEntry> chunk_list_; //entries are never removed from this list
	//but the data belonging to individual entries may
	//be freed.
	int current_chunk_;
	const size_t chunk_size_;
	const size_t max_block_size_;
	const DataDistribution &data_distribution_;
};

class PerArrayChunkList {
};

} /* namespace sip */

#endif /* PER_ARRAY_CHUNK_LIST_H_ */
