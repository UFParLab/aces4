/*
 * block_manager.cpp
 *
 * This is a very simple implementation of the block manager interface for a single-node
 * version of aces4.
 *
 * Blocks are lazily instantiated when first written to.
 *
 * Temp blocks are removed when leave_scope is called for the scope in which they are created.
 * Scopes are tracked by a stack of lists, temp_block_list_stack_;  enter_scope adds a new,
 * empty block list to temp_block_list_stack_.  A pointer to each temp block that is created
 * is added to the list on top of the stack.  leave_scope removes all blocks in the list on
 * top of the stack, and pops the stack.
 *
 * Other blocks are removed when an explicit call is made to destroy_distributed, delete_served,
 * or deallocate_local.
 *
 *
 *
 *  Created on: Aug 10, 2013
 *      Author: Beverly Sanders
 */

#include "config.h"
#include "block_manager.h"
#include <assert.h>
#include <vector>
#include <set>
#include <utility>
#include <algorithm>
#include "sip_tables.h"
#include "array_constants.h"
#include "sip_interface.h"
#include "gpu_super_instructions.h"
#ifdef HAVE_MPI
#include "sip_mpi_attr.h"
#endif


namespace sip {


BlockManager::BlockManager(const SipTables &sip_tables) :
sip_tables_(sip_tables),
block_map_(sip_tables.num_arrays()),
blocks_added_to_pending_delete_list_("blocks added to pending delete list", true),
blocks_created_("total blocks created", true),
max_blocks_("max blocks", true),
wait_pending_delete_("waiting_pending_delete", true){
}



/**
 * Delete blocks being managed by the block manager.
 */
BlockManager::~BlockManager() {
	check_and_warn(temp_block_list_stack_.size() == 0,
			"temp_block_list_stack not empty when destroying block manager!");
	// Free up all blocks managed by this block manager.
	for (int i = 0; i < sip_tables_.num_arrays(); ++i)
		delete_per_array_map_and_blocks(i);

//	//DEBUG print
//#ifdef HAVE_MPI
//	blocks_added_to_pending_delete_list_.gather_from_workers(std::cout);
//	blocks_created_.gather_from_workers(std::cout);
//	max_blocks_.gather_from_workers(std::cout);
//	std::cout << std::endl << std::flush;
//#endif
}


bool BlockManager::has_wild_value(const BlockId& id) {
	int i = 0;
	while (i != MAX_RANK) {
		if (id.index_values(i) == wild_card_value)
			return true;
		++i;
	}
	return false;
}
bool BlockManager::has_wild_slot(const index_selector_t& selector) {
	int i = 0;
	while (i != MAX_RANK) {
		if (selector[i] == wild_card_slot)
			return true;
		++i;
	}
	return false;
}

void BlockManager::allocate_local(const BlockId& id) {
	if (has_wild_value(id)) {
		std::vector<BlockId> list;
		generate_local_block_list(id, list);
		std::vector<BlockId>::iterator it;
		for (it = list.begin(); it != list.end(); ++it) {
			Block* blk = get_block_for_writing(*it, false);
		}
	} else {
		Block* blk = get_block_for_writing(id, false);
	}
}
void BlockManager::deallocate_local(const BlockId& id) {
	if (has_wild_value(id)) {
		std::vector<BlockId> list;
		generate_local_block_list(id, list);
		std::vector<BlockId>::iterator it;
		for (it = list.begin(); it != list.end(); ++it) {
			delete_block(*it);
		}
	} else {
		delete_block(id);
	}
}

//returns a pointer to the requested block, creating it if it doesn't exist.
//The block is not initialized to zero
Block::BlockPtr BlockManager::get_block_for_writing(const BlockId& id,
		bool is_scope_extent) {
	Block::BlockPtr blk = block(id);
	BlockShape shape = sip_tables_.shape(id);
	if (blk == NULL) { //need to create it
		blk = create_block(id, shape);
		if (is_scope_extent) {
			temp_block_list_stack_.back()->push_back(id);
		}
	}
#ifdef HAVE_CUDA
	// Lazy copying of data from gpu to host if needed.
	lazy_gpu_write_on_host(blk, id, shape);
#endif
	return blk;
}

////returns a pointer to the requested block, creating it if it doesn't exist.
////The block is not initialized to zero
//Block::BlockPtr BlockManager::get_block_for_get(const BlockId& id,
//		bool is_scope_extent) {
//	Block::BlockPtr blk = block(id);
//	BlockShape shape = sip_tables_.shape(id);
//	if (blk == NULL) { //need to create it
//		blk = create_block(id, shape);
//	}
//#ifdef HAVE_CUDA
//	// Lazy copying of data from gpu to host if needed.
//	lazy_gpu_write_on_host(blk, id, shape);
//#endif
//	return blk;
//}


//TODO TEMPORARY FIX WHILE SEMANTICS BEING WORKED OUT
Block::BlockPtr BlockManager::get_block_for_reading(const BlockId& id) {
	Block::BlockPtr blk = block(id);
	sial_check(blk != NULL, "Attempting to read non-existent block " + id.str(sip_tables_), current_line());
	////
	//#ifdef HAVE_CUDA
	//	// Lazy copying of data from gpu to host if needed.
	//	lazy_gpu_read_on_host(blk);
	//#endif //HAVE_CUDA
    return blk;
}





/* gets block for reading and writing.  The block should already exist.*/
Block::BlockPtr BlockManager::get_block_for_updating(const BlockId& id) {
//	std::cout << "calling get_block_for_updating for " << id << current_line()<<std::endl << std::flush;
	Block::BlockPtr blk = block(id);
	sial_check(blk != NULL, "attempting to update non-existent block",
			current_line());
#ifdef HAVE_CUDA
	// Lazy copying of data from gpu to host if needed.
	lazy_gpu_update_on_host(blk);
#endif
	return blk;
}

/* gets block, creating it if it doesn't exist.  If new, initializes to zero.*/
Block::BlockPtr BlockManager::get_block_for_accumulate(const BlockId& id,
		bool is_scope_extent) {
	return get_block_for_writing(id, is_scope_extent);
}

void BlockManager::enter_scope() {
	BlockList* temps = new BlockList;
	temp_block_list_stack_.push_back(temps);
}

/*removes the temp blocks in the current scope, then delete the scope's TempBlockStack */
void BlockManager::leave_scope() {
	BlockList* temps = temp_block_list_stack_.back();
	BlockList::iterator it;
	for (it = temps->begin(); it != temps->end(); ++it) {
		BlockId &block_id = *it;
		int array_id = block_id.array_id();

		//CACHED BLOCK MAP
//		// Cached delete for distributed/served arrays.
//		// Regular delete for temp blocks.
//
//		if (sip_tables_.is_distributed(array_id) || sip_tables_.is_served(array_id))
//			cached_delete_block(*it);
//		else
//			delete_block(*it);
		delete_block(*it);
	}
	temp_block_list_stack_.pop_back();
	delete temps;

	test_and_clean_pending(); //empty proc if no mpi
}

//empty proc in serial version
void BlockManager::test_and_clean_pending(){
#ifdef HAVE_MPI
	std::list<Block*>::iterator it;
	for (it = pending_delete_.begin(); it != pending_delete_.end(); ){
		if ((**it).test()){
			delete *it;
			pending_delete_.erase(it++);
		}
		else ++it;
	}
#endif
}

//empty proc in serial version
void BlockManager::wait_and_clean_pending(){
#ifdef HAVE_MPI
	std::list<Block*>::iterator it;
	for (it = pending_delete_.begin(); it != pending_delete_.end(); ){
		wait_pending_delete_.start();
		    (**it).wait();
		wait_pending_delete_.pause();
			delete *it;
			pending_delete_.erase(it++);
		}
#endif
}

std::ostream& operator<<(std::ostream& os, const BlockManager& obj){
	os << "block_map_:" << std::endl;
	os << obj.block_map_ << std::endl;

		os << "temp_block_list_stack_:" << std::endl;
		std::vector<BlockManager::BlockList*>::const_iterator it;
		for (it = obj.temp_block_list_stack_.begin();
				it != obj.temp_block_list_stack_.end(); ++it) {
			BlockManager::BlockList::iterator lit;
			for (lit = (*it)->begin(); lit != (*it)->end(); ++lit) {
				os << (lit == (*it)->begin() ? "" : ",") << *lit;
			}
			os << std::endl;
		}

	return os;
}


Block::BlockPtr BlockManager::block(const BlockId& id){
	return block_map_.block(id);
}


/** creates a new block with the given Id and inserts it into the block map.
 * It is an error to try to create a new block if a block with that id already exists
 * exists.
 */
Block::BlockPtr BlockManager::create_block(const BlockId& block_id,
		const BlockShape& shape) {
	try {
		Block::BlockPtr block_ptr = new Block(shape);
		blocks_created_.inc();
		max_blocks_.inc();
//		current_blocks_++;
//		max_blocks_ = current_blocks_>max_blocks_?current_blocks_:max_blocks_;
//		insert_into_blockmap(block_id, block_ptr);
		block_map_.insert_block(block_id, block_ptr);
		return block_ptr;
	} catch (const std::out_of_range& oor){
		std::cerr << " In BlockManager::create_block" << std::endl;
		std::cerr << *this << std::endl;
		fail(" Could not create block, out of memory", current_line());
		return NULL;
	}
}

//ASYNCH  without this, original is in .h file
void BlockManager::delete_block(const BlockId& id){
	Block::BlockPtr b = block_map_.get_and_remove_block(id);
	if (!(b->test())){
		pending_delete_.push_back(b);
//		num_pending_delete_++;
		blocks_added_to_pending_delete_list_.inc();
	}

	else delete b;
//	current_blocks_--;
	max_blocks_.dec();
}

void BlockManager::generate_local_block_list(const BlockId& id,
		std::vector<BlockId>& list) {
	std::vector<int> prefix;
	int rank = sip_tables_.array_rank(id);
	int pos = 0;
	gen(id, rank, pos, prefix, 0, list);
}

void BlockManager::gen(const BlockId& id, int rank, const int pos,
		std::vector<int> prefix /*pass by value*/, int to_append,
		std::vector<BlockId>& list) {
	if (pos != 0) {
		prefix.push_back(to_append);
	}
	if (pos < rank) {
		int curr_index = id.index_values(pos);
		if (curr_index == wild_card_value) {
			int index_slot = sip_tables_.selectors(id.array_id())[pos];
			int lower = sip_tables_.lower_seg(index_slot);
			int upper = lower + sip_tables_.num_segments(index_slot);
			for (int i = lower; i < upper; ++i) {
				gen(id, rank, pos + 1, prefix, i, list);
			}
		} else {
			gen(id, rank, pos + 1, prefix, curr_index, list);
		}

	} else {
		list.push_back(BlockId(id.array_id(), rank, prefix));
	}
}

#ifdef HAVE_CUDA
/*********************************************************************/
/**						GPU Specific methods						**/
/*********************************************************************/
Block::BlockPtr BlockManager::get_gpu_block_for_writing(const BlockId& id, bool is_scope_extent) {
	Block::BlockPtr blk = block(id);
	BlockShape shape = sip_tables_.shape(id);
	if (blk == NULL) { //need to create it
		blk = create_gpu_block(id, shape);
		if (is_scope_extent) {
			temp_block_list_stack_.back()->push_back(id);
		}
	}
	// Lazy copying of data from host to gpu if needed.
	lazy_gpu_write_on_device(blk, id, shape);
	//blk->gpu_fill(0);
	return blk;
}
Block::BlockPtr BlockManager::get_gpu_block_for_updating(const BlockId& id) {
	Block::BlockPtr blk = block(id);
	check(blk != NULL, "attempting to update non-existent block");

	// Lazy copying of data from host to gpu if needed.
	lazy_gpu_update_on_device(blk);

	return blk;
}
Block::BlockPtr BlockManager::get_gpu_block_for_reading(const BlockId& id) {
	Block::BlockPtr blk = block(id);
	check(blk != NULL, "attempting to read non-existent gpu block", current_line());

	// Lazy copying of data from host to gpu if needed.
	lazy_gpu_read_on_device(blk);

	return blk;
}

Block::BlockPtr BlockManager::get_gpu_block_for_accumulate(const BlockId& id,
		bool is_scope_extent) {
	return get_gpu_block_for_writing(id, is_scope_extent);
}

/** creates a new block with the given Id and inserts it into the block map.
 * It is an error to try to create a new block if a block with that id already exists
 * exists.
 */
Block::BlockPtr BlockManager::create_gpu_block(const BlockId& block_id,
		const BlockShape& shape) {
	Block::BlockPtr block_ptr = Block::new_gpu_block(shape);
	insert_into_blockmap(block_id, block_ptr);
	return block_ptr;
}

void BlockManager::lazy_gpu_read_on_device(const Block::BlockPtr& blk) {
	if (!blk->is_on_gpu() && !blk->is_on_host()) {
		fail("block allocated neither on host or gpu", current_line());
	} else if (!blk->is_on_gpu()) {
		blk->allocate_gpu_data();
		_gpu_host_to_device(blk->get_data(), blk->get_gpu_data(), blk->size());
	} else if (blk->is_dirty_on_host()) {
		_gpu_host_to_device(blk->get_data(), blk->get_gpu_data(), blk->size());
	} else if (blk->is_dirty_on_all()) {
		fail("block dirty on host & gpu !", current_line());
	}
	blk->set_on_gpu();
	blk->unset_dirty_on_host();
}

void BlockManager::lazy_gpu_write_on_device(Block::BlockPtr& blk, const BlockId &id, const BlockShape& shape) {
	if (!blk->is_on_gpu() && !blk->is_on_host()) {
		block_map_.cached_delete_block(id); // Get rid of block, create a new one
		blk = create_gpu_block(id, shape);
//		if (is_scope_extent) {
//			temp_block_list_stack_.back()->push_back(id);
//		}
	} else if (!blk->is_on_gpu()) {
		blk->allocate_gpu_data();
		_gpu_host_to_device(blk->get_data(), blk->get_gpu_data(), blk->size());
	} else if (blk->is_dirty_on_host()) {
		_gpu_host_to_device(blk->get_data(), blk->get_gpu_data(), blk->size());
	} else if (blk->is_dirty_on_all()) {
		fail("block dirty on host & gpu !", current_line());
	}
	blk->set_on_gpu();
	blk->set_dirty_on_gpu();
	blk->unset_dirty_on_host();
}

void BlockManager::lazy_gpu_update_on_device(const Block::BlockPtr& blk) {
	if (!blk->is_on_gpu() && !blk->is_on_host()) {
		fail("block allocated neither on host or gpu", current_line());
	} else if (!blk->is_on_gpu()) {
		blk->allocate_gpu_data();
		_gpu_host_to_device(blk->get_data(), blk->get_gpu_data(), blk->size());
	} else if (blk->is_dirty_on_host()) {
		_gpu_host_to_device(blk->get_data(), blk->get_gpu_data(), blk->size());
	} else if (blk->is_dirty_on_all()) {
		fail("block dirty on host & gpu !", current_line());
	}
	blk->set_on_gpu();
	blk->unset_dirty_on_host();
	blk->set_dirty_on_gpu();
}

void BlockManager::lazy_gpu_read_on_host(const Block::BlockPtr& blk) {
	if (!blk->is_on_gpu() && !blk->is_on_host()) {
		fail("block allocated neither on host or gpu", current_line());
	} else if (!blk->is_on_host()) {
		blk->allocate_host_data();
		_gpu_device_to_host(blk->get_data(), blk->get_gpu_data(), blk->size());
	} else if (blk->is_dirty_on_gpu()) {
		_gpu_device_to_host(blk->get_data(), blk->get_gpu_data(), blk->size());
	} else if (blk->is_dirty_on_all()) {
		fail("block dirty on host & gpu !", current_line());
	}
	blk->set_on_host();
	blk->unset_dirty_on_gpu();
}

void BlockManager::lazy_gpu_write_on_host(Block::BlockPtr& blk, const BlockId &id, const BlockShape& shape) {
	if (!blk->is_on_gpu() && !blk->is_on_host()) {
		block_map_.cached_delete_block(id); // Get rid of block, create a new one
		blk = create_block(id, shape);
//		if (is_scope_extent) {
//			temp_block_list_stack_.back()->push_back(id);
//		}
	} else if (!blk->is_on_host()) {
		blk->allocate_host_data();
		_gpu_device_to_host(blk->get_data(), blk->get_gpu_data(), blk->size());
	} else if (blk->is_dirty_on_gpu()) {
		_gpu_device_to_host(blk->get_data(), blk->get_gpu_data(), blk->size());
	} else if (blk->is_dirty_on_all()) {
		fail("block dirty on host & gpu !", current_line());
	}
	blk->set_on_host();
	blk->set_dirty_on_host();
	blk->unset_dirty_on_gpu();

}

void BlockManager::lazy_gpu_update_on_host(const Block::BlockPtr& blk) {
	if (!blk->is_on_gpu() && !blk->is_on_host()) {
		fail("block allocated neither on host or gpu", current_line());
	} else if (!blk->is_on_host()) {
		blk->allocate_host_data();
		_gpu_device_to_host(blk->get_data(), blk->get_gpu_data(), blk->size());
	} else if (blk->is_dirty_on_gpu()) {
		_gpu_device_to_host(blk->get_data(), blk->get_gpu_data(), blk->size());
	} else if (blk->is_dirty_on_all()) {
		fail("block dirty on host & gpu !", current_line());
	}
	blk->set_on_host();
	blk->unset_dirty_on_gpu();
	blk->set_dirty_on_host();
}

#endif

}
 //namespace sip
