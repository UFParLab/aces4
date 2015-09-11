/*
 * server_block.h
 *
 *  Created on: Mar 25, 2014
 *      Author: njindal
 */

#ifndef SERVER_BLOCK_H_
#define SERVER_BLOCK_H_

#include <bitset>
#include <new>

#include "id_block_map.h"
#include "sip_mpi_constants.h"
#include "async_ops.h"
#include "distributed_block_consistency.h"
#include "chunk_manager.h"
#include "chunk.h"


namespace sip {

class SIPServer;
class DiskBackedBlockMap;
class ServerBlock;

/**
 * Encapsulates the data array belonging to this block, and the information necessary to
 * retrieve it.  The data array may be NULL.
 */
class BlockData{
public:
	/**
	 *
	 * @return  pointer to this blocks data array, or NULL if not in memory.
	 */
	double* get_data() const{
		return chunk_->get_data(offset_);
	}

	Chunk::offset_val_t file_offset(){
		return chunk_->file_offset() + offset_;
	}

	friend std::ostream& operator<< (std::ostream& os, const BlockData& block);

	/** Overload ==  Only compares chunk and offset
	 * Overloading == and < is required because we are using BlockIds as
	 * keys in the block map, which is currently an STL map instance,
	 * where map is a binary tree.
	 *
	 * @param rhs
	 * @return
	 */
	bool operator==(const BlockData& rhs) const;

	/** Overload <.
	 *  a < b if the a.chunk_ < b.chunk_ or a.chunk_ = chunk_  &&  a.offset_ < b.offset
	 * @param rhs
	 * @return
	 */
	bool operator<(const BlockData& rhs) const;

	friend class DiskBackedBlockMap;

private:
	BlockData(size_t size, Chunk* chunk, Chunk::offset_val_t offset):
		size_(size), chunk_(chunk), offset_(offset){
	}

	const size_t size_;
	Chunk* chunk_;
	Chunk::offset_val_t offset_;  //offset, in units of double values within the chunk

	friend ServerBlock;

};

/**
 * TODO  Update these comments!!!
 *
 * These methods should be called by the DiskBackedBlockManager.
 *
 *
 *
 * Maintains a Block in the memory of a server.
 * Each block maintains its status. The status informs the client of a
 * ServerBlock instance of how to treat it.
 *
 * The "status" of a ServerBlock consists of :
 * a) Whether it is in memory (I)
 * b) Whether it is on disk (O)
 * c) Whether it is dirty in memory (D)
 *
 * The operations that can be performed are:
 * 1. Read Block (get_block_for_reading)
 * 2. Write Block (get_block_for_writing)
 * 3. Update Block (get_block_for_updating)
 * 4. Flush Block To Disk (in case of limit_reached() being true).
 *
 * For each operation, representing IOD as 3 bits as initial state,
 * the action and final state is specified for each operation.
 *
 * // In Memory Operations
 *
 * Operation : Read Block
 * 100 -> No Action 			-> 100
 * 010 -> Alloc, Read from disk	-> 110
 * 101 -> No Action 			-> 101
 * 110 -> No Action 			-> 101
 * 111 -> No Action 			-> 110
 *
 * Operation : Write Block
 * 100 -> No Action 			-> 101
 * 010 -> Alloc					-> 111
 * 101 -> No Action 			-> 101
 * 110 -> No Action 			-> 111
 * 111 -> No Action 			-> 111
 *
 * Operation : Update Block
 * 100 -> No Action 			-> 101
 * 010 -> Alloc, Read from disk	-> 111
 * 101 -> No Action				-> 111
 * 110 -> No Action				-> 111
 * 111 -> No Action				-> 111
 *
 * // On Disk Operations (Assumes memory available)
 *
 * Operation : Write Block to Disk
 * 100 -> Write to disk 		-> 110
 * 010 -> No Action				-> 010
 * 101 -> Write to disk			-> 110
 * 110 -> No Action				-> 110
 * 111 -> Write to disk			-> 110
 *
 * Operation : Read Block from Disk
 * 100 -> No Action				-> 100
 * 010 -> Read from disk		-> 110
 * 101 -> No Action				-> 101
 * 110 -> No Action				-> 110
 * 111 -> No Action				-> 111
 *
 */

/** Invariants:
 *
 *  Block has been assigned data in a chunk.
 *  The block pointer is stored in that chunk iff block exists.
 */

class ServerBlock {
public:
	typedef double * dataPtr;
	typedef ServerBlock* ServerBlockPtr;


	~ServerBlock(); //remove from chunk's block list

	/**
	 * Updates and checks the consistency of a server block.
	 * @param type
	 * @param worker
	 * @return false if brought into inconsistent state by operation
	 */
	bool update_and_check_consistency(SIPMPIConstants::MessageType_t operation, int worker, int section){
			return race_state_.update_and_check_consistency(operation, worker, section);
}

    dataPtr get_data() { return block_data_.get_data(); }
	size_t size() const { return block_data_.size_; }
	Chunk* get_chunk(){ return block_data_.chunk_;}

    dataPtr accumulate_data(size_t size, dataPtr to_add); /*! for all elements, this->data += to_add->data */
    dataPtr fill_data(double value);
    dataPtr increment_data(double delta);
    dataPtr scale_data(double factor);
    dataPtr copy_data(ServerBlock* source_block){
    	double* data = block_data_.get_data();
    	double* source_data = source_block->get_data();
    	check(size() == source_block->size(), "mismatched sizes in server block copy");
    	check(data != NULL, "attempting to copy into block with null data_");
    	check(source_data != NULL, "attempting to accumulate from null dataPtr");
    	for (size_t i = 0; i < size(); ++i){
    			data[i] = source_data[i];
    	}
    	return data;
    }

	void wait(){ async_state_.wait_all();}
	void wait_for_writes(){ async_state_.wait_for_writes();}

	friend std::ostream& operator<< (std::ostream& os, const ServerBlock& block);

private:

//    const size_t size_;/**< Number of elements in block */  //TODO get rid of this, is now in BlockData

    BlockData block_data_;

	ServerBlockAsyncManager async_state_; /** handles async communication operations */

    DistributedBlockConsistency race_state_;

    /*  this is private because blocks should ONLY be created by create_block
     * method in DiskBackedBlockMap;
     */
//	ServerBlock(size_t size, Chunk& chunk, Chunk::offset_val_t offset):
//	//	size_(size),
//		block_data_(size, chunk, offset){
//	}

    ServerBlock(size_t size, ChunkManager* manager,
    		int chunk_number, Chunk::offset_val_t offset):
    			block_data_(size, manager->chunk(chunk_number), offset){}

	friend IdBlockMap<ServerBlock>;
	friend class PendingAsyncManager;
	friend class DiskBackedBlockMap;


	DISALLOW_COPY_AND_ASSIGN(ServerBlock);
};

} /* namespace sip */

#endif /* SERVER_BLOCK_H_ */
