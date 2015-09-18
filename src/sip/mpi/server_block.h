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

	/**
	 *
	 * @return offset in file (relative to beginning of data) of this block
	 */
	Chunk::offset_val_t file_offset(){
		return chunk_->file_offset() + offset_;
	}

	friend std::ostream& operator<< (std::ostream& os, const BlockData& block);

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
 * Represents a SIAL block at the server.  Also provides basic arithmetic operations on blocks.
 *
 * Invariants:
 *
 *  Block has been assigned data in a chunk.
 *  A pointer to this block is stored in that chunk.
 *  These invariants must be established by the caller of the constructor.
 *
 *  All blocks should be deleted from a chunk before the chunk is deleted.
 *
 *  Once created, a block is never deleted unless the entire array it belongs to is deleted.
 *  (However, the data associated with a block via its chunk may be deleted if the
 *  block has been backed up on disk.)
 */

class ServerBlock {
public:
	typedef double * dataPtr;
	typedef ServerBlock* ServerBlockPtr;

	~ServerBlock(); //remove from chunk's block list

	/**
	 * Updates and checks the consistency of a server block.  This method
	 * implements the check for races caused by conflicting accesses by different
	 * servers without an intervening barrier
	 *
	 * @param type
	 * @param worker
	 * @return false if brought into inconsistent state by operation
	 */
	bool update_and_check_consistency(SIPMPIConstants::MessageType_t operation, int worker, int section);

	/**
	 * Returns a pointer to the data of this block
	 * @return
	 */
    dataPtr get_data();

    /**
     * Returns the number of doubles in this block
     * @return
     */
	size_t size() const;

	/**
	 * Returns a pointer to the chunk that manages this block's data
	 * @return
	 */
	Chunk* get_chunk();

	/**
	 * Element-wise increment of this blocks data with the given array
	 * Precondition:  size  is number of elements in the to_add array
	 * Precondition:  size == this.size()
	 *
	 * @param size  number of elements
	 * @param to_add
	 * @return
	 */
    dataPtr accumulate_data(size_t size, dataPtr to_add); /*! for all elements, this->data += to_add->data */

    /**
     * Assigns value to each element of this block's data array
     * @param value
     * @return
     */
    dataPtr fill_data(double value);

    /**
     * Increments each element of this block's data array with the given delta value
     *
     * @param delta
     * @return
     */
    dataPtr increment_data(double delta);

    /**
     * Multiplies each element of this block's data array with the given factor
     * @param factor
     * @return
     */
    dataPtr scale_data(double factor);

    /**
     * Copy the values from the source blocks data into this block's data array.
     *
     * @param source_block
     * @return
     */
    dataPtr copy_data(ServerBlock* source_block);


    /**
     * Wait for all pending asynchronous operations on this block to complete
     */
	void wait();

	/**
	 * Wait for all pending write operations on this block to complete.
	 */
	void wait_for_writes();

	friend std::ostream& operator<< (std::ostream& os, const ServerBlock& block);

private:

    BlockData block_data_;
	ServerBlockAsyncManager async_state_;
    DistributedBlockConsistency race_state_;

    /**
     * Constructor for a ServerBlock.  It is private because blocks should only be
     * created by the create_block method in DiskBackedBlockMap.  That method establishes
     * the necessary interclass invariants.
     *
     * @param size
     * @param manager
     * @param chunk_number
     * @param offset
     */
    ServerBlock(size_t size, ChunkManager* manager,
    		int chunk_number, Chunk::offset_val_t offset):
    			block_data_(size, manager->chunk(chunk_number), offset){}

	friend IdBlockMap<ServerBlock>;
	friend class PendingAsyncManager;
	friend class DiskBackedBlockMap;

	DISALLOW_COPY_AND_ASSIGN(ServerBlock);
};

/** Inlined procedures
 */

inline ServerBlock::dataPtr ServerBlock::get_data() { return block_data_.get_data(); }
inline size_t ServerBlock::size() const { return block_data_.size_; }
inline Chunk* ServerBlock::get_chunk(){ return block_data_.chunk_;}
inline 	bool ServerBlock::update_and_check_consistency(SIPMPIConstants::MessageType_t operation, int worker, int section){
	return race_state_.update_and_check_consistency(operation, worker, section);
}
inline void ServerBlock::wait(){ async_state_.wait_all();}
inline void ServerBlock::wait_for_writes(){ async_state_.wait_for_writes();}

} /* namespace sip */

#endif /* SERVER_BLOCK_H_ */
