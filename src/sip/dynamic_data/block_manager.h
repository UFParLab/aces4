/*
 * block_manager.h
 *
 * Manages blocks and index values for the sip interpreter.  This public interface
 * should suffice for standard aces.  Needs to be enhanced to support GPUs.
 *
 *  Created on: Aug 10, 2013
 *      Author: Beverly Sanders
 */

#ifndef BLOCK_MANAGER_H_
#define BLOCK_MANAGER_H_

#include "config.h"
#include <map>
#include <utility>
#include <vector>
#include <stack>
#include "blocks.h"

#ifdef HAVE_MPI
#include "sip_mpi_attr.h"
#include "data_distribution.h"
#endif

namespace sip {
class SipTables;
}

namespace sip {

class PersistentArrayManager;

class BlockManager {
public:


	typedef std::map<BlockId, Block::BlockPtr> IdBlockMap;
	typedef IdBlockMap* IdBlockMapPtr;
	typedef std::vector<IdBlockMapPtr> BlockMap;
	typedef std::vector<BlockId> BlockList;

#ifdef HAVE_MPI
	BlockManager(sip::SipTables&, PersistentArrayManager&, PersistentArrayManager&, sip::SIPMPIAttr&, sip::DataDistribution&);
#else
	BlockManager(sip::SipTables&, PersistentArrayManager&, PersistentArrayManager&);
#endif
	~BlockManager();

	/*! implements a global SIAL barrier */
	void barrier();

	/*! SIAL operations on arrays */
	void create_distributed(int array_id);
	void restore_distributed(int array_id, BlockManager::IdBlockMapPtr bid_map);
	void delete_distributed(int array_id);
	void get(const BlockId&);
	void put_replace(const BlockId&, const Block::BlockPtr);
	void put_accumulate(const BlockId&, const Block::BlockPtr);

	void destroy_served(int array_id);
	void request(const BlockId&);
	void prequest(const BlockId&, const BlockId&);
	void prepare(const BlockId&, const Block::BlockPtr);
	void prepare_accumulate(const BlockId&, const Block::BlockPtr);

	void allocate_local(const BlockId&);
	void deallocate_local(const BlockId&);

	/** basic block retrieval methods.*/


	/** Gets requested Block to be written to, allocating it if it doesn't exist. Newly allocated blocks are initialized to 0.
	 *
	 * @param id  BlockId of requested Block
	 * @param is_scope_extent indicates whether a newly allocated block should be placed on the list of blocks to be deleted on a leave_scope.
	 * @return pointer to requested Block
	 */
	Block::BlockPtr get_block_for_writing(const BlockId& id, bool is_scope_extent);


	/** Gets requested Block, which is required to already exist, for read only access.
	 *
	 * @param id BlockId of requested Block
	 * @return pointer to requested Block
	 */
	Block::BlockPtr get_block_for_reading(const BlockId& id);

	/** Gets requested Block for reading and writing.  Requires that the Block already exist
	 *
	 * @param id BlockId of requested Block
	 * @return pointer to requested Block
	 */
	Block::BlockPtr get_block_for_updating(const BlockId& id);


	/** Gets block for accumulating into,  allocating it if it doesn't exists.
	 * Newly allocated blocks are initialized to zero.
	 *
	 * @param id BlockId of requested Block
	 * @param is_scope_extent indicates whether a newly allocated block should be placed on the list of blocks to be deleted on a leave_scope.
	 * @return pointer to requested Block
	 */
	//TODO is this needed, or can it be replace with get_block_for_writing?
	Block::BlockPtr get_block_for_accumulate(const BlockId& id, bool is_scope_extent);

	/** Initializes a new scope for temp Blocks
	 */
	void enter_scope();

	/** Removes the temp Blocks that were allocated in the current scope,
	 * then deletes the scope's temp Block list
	 */
	void leave_scope();

	/**
	 * Saves the persistent distributed arrays to the persistent block manager.
	 */
	void save_persistent_dist_arrays();

#ifdef HAVE_CUDA
	// GPU
	Block::BlockPtr get_gpu_block_for_writing(const BlockId& id, bool is_scope_extent);
	Block::BlockPtr get_gpu_block_for_updating(const BlockId& id);
	Block::BlockPtr get_gpu_block_for_reading(const BlockId& id);
	Block::BlockPtr get_gpu_block_for_accumulate(const BlockId& id, bool is_scope_extent);

	/**
	 * Lazily copy block from host to device for reading if needed
	 * @param
	 * @param
	 * @return
	 */
	void lazy_gpu_read_on_device(Block::BlockPtr& blk);
	/**
	 * Lazily copy block from host to device for writing to if needed
	 * @param
	 * @param
	 * @return
	 */
	void lazy_gpu_write_on_device(Block::BlockPtr& blk, const BlockId &id, const BlockShape& shape);
	/**
	 * Lazily copy block from host to device for updating if needed
	 * @param
	 * @param
	 * @return
	 */
	void lazy_gpu_update_on_device(Block::BlockPtr& blk);

	/**
 	 * Lazily copy block from device to host for reading if needed
	 * @param
	 * @param
	 * @return
	 */
	void lazy_gpu_read_on_host(Block::BlockPtr& blk);
	/**
	 * Lazily copy block from device to host for writing to if needed
	 * @param
	 * @param
	 * @return
	 */
	void lazy_gpu_write_on_host(Block::BlockPtr& blk, const BlockId &id, const BlockShape& shape);
	/**
	 * Lazily copy block from device to host for updating if needed
	 * @param
	 * @param
	 * @return
	 */
	void lazy_gpu_update_on_host(Block::BlockPtr& blk);

#endif


	friend std::ostream& operator<<(std::ostream&, const BlockManager&);

	friend class DataManager;

private:
	/** Obtains block from BlockMap
	 *
	 * @param BlockId of desired block
	 * @return pointer to given block, or NULL if it is not in the map.
	 */
	Block::BlockPtr block(const BlockId&);

	/** Creates and returns a new block with the given shape and records it in the block_map_ with the given BlockId.
	 * Requires the block with given id does not already exist.
	 *
	 * @param BlockId of given block
	 * @param shape of block to create
	 * @return pointer to newly created block.
	 */
	Block::BlockPtr create_block(const BlockId&, const BlockShape& shape);

	/**
	 * Creates and returns a new block on the gpu with the given shape and records it
	 * in the block_map_ with the given BlockId.
	 * Requires the block with given id does not already exist.
	 *
	 * @param BlockId of given block
	 * @param shape of block to create
	 * @return pointer to newly created block.
	 */
	Block::BlockPtr create_gpu_block(const BlockId&, const BlockShape& shape);

	/**
	 * Removes the given Block from the map and frees its data. It is a fatal error
	 * to try to delete a block that doesn't exist.
	 *
	 * @param BlockId of block to remove
	 */
	void remove_block(const BlockId&);

	/**
	 * Creates a list of concrete BlockIds from the given BlockId, which may contain wild card slots
	 * @param id BlockId of the possibly wild-card-containing argument to allocate statement
	 * @param list list of BlockIds implied by the wild cards in id.
	 */
	void generate_local_block_list(const BlockId& id, std::vector<BlockId>& list);
	/**
	 * Recursive helper function used by generate_local_block_list
	 * @param [in] id BlockId of the possibly wild-card-containing argument to allocate statement
	 * @param [in] rank
	 * @param [in] pos current position in selector
	 * @param [in] prefix previously determined indices in selector.  Passed by value
	 * @param [in] to_append  index value to be added to selector
	 * @param [inout] list of generated BlockIds
	 */
	void gen(const BlockId& id, int rank, const int pos,  std::vector<int> prefix /*pass by value*/,  int to_append, std::vector<BlockId>& list);

	/**
	 *
	 * @param BlockId
	 * @return whether the BlockId was obtained from a selector with a wild card
	 */
	bool has_wild_value(const BlockId&);

	/**
	 *
	 * @param selector
	 * @return whether the selector contains a wild card
	 */
	bool has_wild_slot(const index_selector_t& selector);

	/** Vector of map of blocks> */
	BlockMap block_map_;

	/** Conceptually, a stack of lists of temp blocks.  Each list corresponds to a scope, and the entries in the
	 * list are blocks that should be deleted when that scope is exited.  The enter_scope and leave_scope methods
	 * create a new list and add it to the stack, and remove a list from the stack and delete its blocks, respectively.
	 */
	std::vector<BlockList*> temp_block_list_stack_; //this is a vector rather than a stack
													//to allow more convenient printing of
													//contents.

	/** Pointer to static data */
	sip::SipTables& sip_tables_;  //TODO only needed to look up shape. Perhaps refactor to eliminate
	                             //this dependency?

	/**
	 * Read and write persistent data between programs.
	 */
	sip::PersistentArrayManager & pbm_read_;
	sip::PersistentArrayManager & pbm_write_;

#ifdef HAVE_MPI
	/**
	 * MPI Attributes of the SIP for this rank
	 */
	sip::SIPMPIAttr & sip_mpi_attr_;

	/**
	 * Data distribution scheme
	 */
	sip::DataDistribution &data_distribution_;

	/**
	 * Helper function to send a block to the server that "owns" it.
	 */
	void send_block_to_server(int server_rank, int to_send_message, const BlockId& bid,
			Block::BlockPtr bptr);
#endif

	DISALLOW_COPY_AND_ASSIGN(BlockManager);

	/**
	 * Helper function to insert newly created block into block map.
	 * @param block_id
	 * @param block_ptr
	 */
	void insert_into_blockmap(const BlockId& block_id,
			Block::BlockPtr block_ptr);

};

} /* namespace sip */

#endif /* BLOCK_MANAGER_H_ */
