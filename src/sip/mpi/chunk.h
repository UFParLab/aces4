/*
 * chunk.h
 *
 *  Created on: Jul 21, 2015
 *      Author: basbas
 */

#ifndef CHUNK_H_
#define CHUNK_H_

#include <limits>
#include <vector>
#include "sip_mpi_attr.h"
#include "data_distribution.h"




namespace sip {

class ArrayFile;
class DiskBackedBlockMap;
class ServerBlock;

/** Entries in ChunkMap.  Each entry represents a chunk
 *
 *This is a fairly passive container, its state is controlled by the containing ChunkList class.
 *In particular, the lifetime of the data array is managed elsewhere.
 */
class Chunk {

public:

	typedef double* data_ptr_t;
	typedef MPI_Offset offset_val_t;  //This must be the same as in array_file.h
	                                  //TODO refactor this
	/**
	 *
	 * @param data
	 * @param size
	 * @param valid_on_disk
	 * @param file_offset
	 */
	Chunk(data_ptr_t data, MPI_Offset file_offset, bool valid_on_disk) :
			data_(data), num_assigned_doubles_(0), file_offset_(file_offset), valid_on_disk_(
					valid_on_disk) {
	}


	~Chunk() { //don't delete data here.  Data is owned by the owning ChunkManager
	}

	data_ptr_t get_data(offset_val_t offset){
		if (data_ == NULL) return NULL;
		return data_ + offset;
	}

	void add_server_block(ServerBlock* block){
		blocks_.push_back(block);
	}

	void remove_server_block(ServerBlock* block){
		std::vector<ServerBlock*>::iterator it;
		for (it = blocks_.begin(); it != blocks_.end(); ++it){
			if (block == *it){
				blocks_.erase(it);
				return;
			}
		}
		check(false, "attempting to remove block from chunk's blocks_ list but block not found");
	}


	void wait_all();

	offset_val_t file_offset(){
		return file_offset_;
	}

private:
	data_ptr_t data_;  //pointer to beginning of chunk
	offset_val_t file_offset_; //offset is in units of doubles starting at 0.
	                           //We can start at 0 because any accesses to chunk data on
	                           //disk should be in a file view with displacement set to
	                           //skip the header, and to count in units of doubles.
	size_t num_assigned_doubles_; //number of doubles allocated  (remaining = chunk size - num_allocated_doubles)
	bool valid_on_disk_;
	std::vector<ServerBlock*> blocks_;  //list of blocks that have been assigned data from this chunk.

	friend class ChunkManager;
	friend class ArrayFile;
	friend class DiskBackedBlockMap;
	friend std::ostream& operator<<(std::ostream&, const Chunk& obj);

	DISALLOW_COPY_AND_ASSIGN(Chunk);

};

} /* namespace sip */

#endif /* CHUNK_H_ */



