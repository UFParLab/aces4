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
#include "block.h"
#include "cached_block_map.h"

void list_blocks_with_number();
void check_block_number_calculation(int& array_slot, int& rank,
			int* index_values, int& size, int* extents,  double* data, int& ierr);

namespace sip {
class SipTables;
class SialOpsParallel;
class ContiguousLocalArrayManager;
class SialOpsSequential;



class BlockManager {
public:

	typedef std::vector<BlockId> BlockList;
	typedef std::map<BlockId, int> BlockIdToIndexMap;

	BlockManager(const SipTables &sip_tables);
	~BlockManager();

	void allocate_local(const BlockId&);
	void deallocate_local(const BlockId&);

	/** basic block retrieval methods.*/

	/** Gets requested Block to be written to, allocating it if it doesn't exist.
	 * Newly allocated blocks ARE NOT initialized to 0.
	 *
	 * @param id  BlockId of requested Block
	 * @param is_scope_extent indicates whether a newly allocated block should be placed on the list of blocks to be deleted on a leave_scope.
	 * @return pointer to requested Block
	 */
	Block::BlockPtr get_block_for_writing(const BlockId& id, bool is_scope_extent = false);


	/** Gets requested Block, which is required to already exist, for read only access.
	 * It is a fatal error to ask for a block that does not exit.
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
	 * Newly allocated blocks ARE initialized to zero.
	 *
	 * @param id BlockId of requested Block
	 * @param is_scope_extent indicates whether a newly allocated block should be placed on the list of blocks to be deleted on a leave_scope.
	 * @return pointer to requested Block
	 */
	//TODO is this needed, or can it be replace with get_block_for_writing?
	Block::BlockPtr get_block_for_accumulate(const BlockId& id, bool is_scope_extent=false);

	/** Initializes a new scope for temp Blocks
	 */
	void enter_scope();

	/** Removes the temp Blocks that were allocated in the current scope,
	 * then deletes the scope's temp Block list
	 */
	void leave_scope();

	/**
	 * Deletes the map for the given array from the block map.  This is used by the
	 * persistent_array_manager. The call is simply delegated to the block_map_.
	 *
	 * @param array_id
	 */
	void delete_per_array_map_and_blocks(int array_id){
		block_map_.delete_per_array_map_and_blocks(array_id);
	}

	std::size_t total_blocks(){
		return block_map_.total_blocks();
	}

	void free_blocks(){
		for (int i = 0; i < sip_tables_.num_arrays(); ++i){
			if (sip_tables_.is_distributed(i)){
			block_map_.delete_per_array_map_and_blocks(i);
			}
		}
	}

#ifdef HAVE_CUDA
	// GPU
	Block::BlockPtr get_gpu_block_for_writing(const BlockId& id, bool is_scope_extent=false);
	Block::BlockPtr get_gpu_block_for_updating(const BlockId& id);
	Block::BlockPtr get_gpu_block_for_reading(const BlockId& id);
	Block::BlockPtr get_gpu_block_for_accumulate(const BlockId& id, bool is_scope_extent=false);

	/**
	 * Lazily copy block from host to device for reading if needed
	 * @param
	 * @param
	 * @return
	 */
	void lazy_gpu_read_on_device(const Block::BlockPtr& blk);
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
	void lazy_gpu_update_on_device(const Block::BlockPtr& blk);

	/**
 	 * Lazily copy block from device to host for reading if needed
	 * @param
	 * @param
	 * @return
	 */
	void lazy_gpu_read_on_host(const Block::BlockPtr& blk);
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
	void lazy_gpu_update_on_host(const Block::BlockPtr& blk);

#endif

	const CachedBlockMap& block_map() const {return block_map_;}

	friend std::ostream& operator<<(std::ostream&, const BlockManager&);

	friend class DataManager;
	friend class Interpreter;

private:
	/** Obtains block from BlockMap.
	 *
	 * If MPI and block is in transit, waits until it is available.
	 *
	 * @param BlockId of desired block
	 * @return pointer to given block, or NULL if it is not in the map.
	 */
	Block::BlockPtr block(const BlockId& id);

	/**
	 * Helper function to insert newly created block into block map.
	 * @param block_id
	 * @param block_ptr
	 */
     void insert_into_blockmap(const BlockId& block_id,	Block::BlockPtr block_ptr){block_map_.insert_block(block_id, block_ptr);}

     /**
	 * Removes the given Block from the map and caches it if possible. It is a fatal error
	 * to try to delete a block that doesn't exist.
	 *
	 * @param BlockId of block to remove
	 */
	void cached_delete_block(const BlockId& id){block_map_.cached_delete_block(id);}


	/**
	 * Removes the given Block from the map and frees its data. It is a fatal error
	 * to try to delete a block that doesn't exist.
	 * @param id
	 */
	void delete_block(const BlockId& id){block_map_.delete_block(id);}

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
	/** Pointer to static data */
	const SipTables& sip_tables_;

	/** Map from block id's to blocks */
	CachedBlockMap block_map_;

	/** Conceptually, a stack of lists of temp blocks.  Each list corresponds to a scope, and the entries in the
	 * list are blocks that should be deleted when that scope is exited.  The enter_scope and leave_scope methods
	 * create a new list and add it to the stack, and remove a list from the stack and delete its blocks, respectively.
	 */
	std::vector<BlockList*> temp_block_list_stack_; //this is a vector rather than a stack
													//to allow more convenient printing of
													//contents.


	friend class SialOpsSequential;
	friend class SialOpsParallel;
	friend class ContiguousLocalArrayManager;

	friend void ::list_blocks_with_number();
	friend void ::check_block_number_calculation(int& array_slot, int& rank,
			int* index_values, int& size, int* extents,  double* data, int& ierr);

	DISALLOW_COPY_AND_ASSIGN(BlockManager);

};

} /* namespace sip */

#endif /* BLOCK_MANAGER_H_ */
