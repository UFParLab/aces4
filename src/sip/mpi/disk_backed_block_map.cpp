/*
 * disk_backed_block_map.cpp
 *
 *  Created on: Apr 17, 2014
 *      Author: njindal
 */

#include <disk_backed_block_map.h>
#include "server_block.h"
#include "block_id.h"
#include "global_state.h"
#include <iostream>
#include <string>
#include <sstream>

namespace sip {

DiskBackedBlockMap::DiskBackedBlockMap(const SipTables& sip_tables,
		const SIPMPIAttr& sip_mpi_attr, const DataDistribution& data_distribution) :
		sip_tables_(sip_tables), sip_mpi_attr_(sip_mpi_attr), data_distribution_(data_distribution),
		block_map_(sip_tables.num_arrays()),
		disk_backed_arrays_io_(sip_tables, sip_mpi_attr, data_distribution),
        policy_(block_map_),
        max_allocatable_bytes_(sip::GlobalState::get_max_data_memory_usage()) {
}

DiskBackedBlockMap::~DiskBackedBlockMap(){}

void DiskBackedBlockMap::read_block_from_disk(ServerBlock*& block, const BlockId& block_id, size_t block_size){
    /** Allocates space for a block, reads block from disk
     * into newly allocated space, sets the in_memory flag,
     * inserts into block_map_ if needed
     */

	block = allocate_block(block, block_size);
	disk_backed_arrays_io_.read_block_from_disk(block_id, block);
	block->set_in_memory();
}

void DiskBackedBlockMap::write_block_to_disk(const BlockId& block_id, ServerBlock* block){
    disk_backed_arrays_io_.write_block_to_disk(block_id, block);
    block->unset_dirty();
    block->set_on_disk();
}

ServerBlock* DiskBackedBlockMap::allocate_block(ServerBlock* block, size_t block_size, bool initialize){
    /** If enough memory remains, allocates block and returns.
     * Otherwise, frees up memory by writing out dirty blocks
     * till enough memory has been obtained, then allocates
     * and returns block.
     */
	std::size_t remaining_mem = max_allocatable_bytes_ - ServerBlock::allocated_bytes();

	while (block_size > remaining_mem){
		try{
			BlockId bid = policy_.get_next_block_for_removal();
			ServerBlock* blk = block_map_.block(bid);
			SIP_LOG(std::cout << "S " << sip_mpi_attr_.company_rank()
									<< " : Freeing block " << bid
									<< " and writing to disk to make space for new block"
									<< std::endl);
			if(blk->is_dirty()){
				write_block_to_disk(bid, blk);
			}
			blk->free_in_memory_data();
			remaining_mem = max_allocatable_bytes_ - ServerBlock::allocated_bytes();
		} catch (const std::out_of_range& oor){
			std::cerr << " In DiskBackedBlockMap::allocate_block" << std::endl;
			std::cerr << oor.what() << std::endl;
			std::cerr << *this << std::endl;
			fail(" Something got messed up in the internal data structures of the Server", current_line());
		} catch(const std::bad_alloc& ba){
			std::cerr << " In DiskBackedBlockMap::allocate_block" << std::endl;
			std::cerr << ba.what() << std::endl;
			std::cerr << *this << std::endl;
			fail(" Could not allocate ServerBlock, out of memory", current_line());
		}
	}

	std::stringstream ss;
	ss << "S " << sip_mpi_attr_.company_rank() << " : Could not allocate memory for block of size "
			<< block_size << ", Memory being used :" << ServerBlock::allocated_bytes() << std::endl;
	sip :: check (block_size <= max_allocatable_bytes_ - ServerBlock::allocated_bytes(), ss.str());
   
    if (block == NULL) {
	    block = new ServerBlock(block_size, initialize);
    } else {
        block->allocate_in_memory_data();
    }

	return block;

}


ServerBlock* DiskBackedBlockMap::get_block_for_updating(const BlockId& block_id){
	/** If block is not in block map, allocate space for it
	 * Otherwise, if the block is in memory, read and return
	 * if it is only on disk, read it in, store in block map and return.
	 * set in_memory and dirty_flag
	 */
	ServerBlock* block = block_map_.block(block_id);
	size_t block_size = sip_tables_.block_size(block_id);
	if (block == NULL) {
		std::stringstream msg;
		msg << "S " << sip_mpi_attr_.global_rank();
		msg << " : getting uninitialized block " << block_id << ".  Creating zero block for updating "<< std::endl;
		SIP_LOG(std::cout << msg.str() << std::flush);
		block = allocate_block(NULL, block_size);
	    block_map_.insert_block(block_id, block);
	} else {
		if(!block->is_in_memory()){
			if (block->is_on_disk()){
				read_block_from_disk(block, block_id, block_size);
            } else {
				block->allocate_in_memory_data();
            }
        }
	}

	block->set_in_memory();
	block->set_dirty();

	return block;
}

ServerBlock* DiskBackedBlockMap::get_block_for_writing(const BlockId& block_id){
	/** If block is not in block map, allocate space for it
	 * Otherwise, if the block is not in memory, allocate space for it.
	 * Set in_memory and dirty_flag
	 */
	ServerBlock* block = block_map_.block(block_id);
	size_t block_size = sip_tables_.block_size(block_id);
	if (block == NULL) {
		std::stringstream msg;
		msg << "S " << sip_mpi_attr_.global_rank();
		msg << " : getting uninitialized block " << block_id << ".  Creating zero block for writing"<< std::endl;
		SIP_LOG(std::cout << msg.str() << std::flush);
		block = allocate_block(NULL, block_size);
	    block_map_.insert_block(block_id, block);
	} else {
		if (!block->is_in_memory())
			block->allocate_in_memory_data();
	}

	block->set_in_memory();
	block->set_dirty();

	return block;
}

ServerBlock* DiskBackedBlockMap::get_block_for_reading(const BlockId& block_id){
	/** If block is not in block map, there is an error !!
	 * Otherwise, if the block is in memory, read and return or
	 * if it is only on disk, read it in, store in block map and return.
	 * Set in_memory flag.
	 */
	ServerBlock* block = block_map_.block(block_id);
	size_t block_size = sip_tables_.block_size(block_id);
	if (block == NULL) {
		// Error !

		std::stringstream errmsg;
		errmsg << " S " << sip_mpi_attr_.global_rank();
		errmsg << " : Asking for block " << block_id << ". It has not been put/prepared before !"<< std::endl;
		std::cout << errmsg.str() << std::flush;
		
		sip::fail(errmsg.str());

		// WARNING DISABLED !
		if (false){
			std::stringstream msg;
			msg << "S " << sip_mpi_attr_.global_rank();
			msg << " : getting uninitialized block " << block_id << ".  Creating zero block "<< std::endl;
			std::cout << msg.str() << std::flush;
			block = allocate_block(NULL, block_size);
			block_map_.insert_block(block_id, block);
		}


	} else {
		if(!block->is_in_memory())
			if (block->is_on_disk()){
				read_block_from_disk(block, block_id, block_size);
            } else {
				sip::fail("get_block_for_reading : ServerBlock neither on memory or on disk !");
            }
	}

	block->set_in_memory();

	sip::check(block != NULL, "Block is NULL in Server get_block_for_reading, should not happen !");
	return block;
}

IdBlockMap<ServerBlock>::PerArrayMap* DiskBackedBlockMap::per_array_map(int array_id){
	return block_map_.per_array_map(array_id);
}

IdBlockMap<ServerBlock>::PerArrayMap* DiskBackedBlockMap::get_and_remove_per_array_map(int array_id){
	return block_map_.get_and_remove_per_array_map(array_id);
}

void DiskBackedBlockMap::insert_per_array_map(int array_id, IdBlockMap<ServerBlock>::PerArrayMap* map_ptr){
	block_map_.insert_per_array_map(array_id, map_ptr);
}


void DiskBackedBlockMap::delete_per_array_map_and_blocks(int array_id){
    policy_.remove_all_blocks_for_array(array_id);          // remove from block replacement policy
    SIP_LOG(std::cout << "S " << sip_mpi_attr_.global_rank() << " : Removed blocks from block replacement policy" << std::endl;);

    IdBlockMap<ServerBlock>::PerArrayMap *per_array_map = block_map_.per_array_map(array_id);
	disk_backed_arrays_io_.delete_array(array_id, per_array_map);			// remove from disk
	SIP_LOG(std::cout << "S " << sip_mpi_attr_.global_rank() << " : Deleted blocks from disk" << std::endl;);

	block_map_.delete_per_array_map_and_blocks(array_id);	// remove from memory
	SIP_LOG(std::cout << "S " << sip_mpi_attr_.global_rank() << " : Removed blocks from memory" << std::endl;);
}

void DiskBackedBlockMap::restore_persistent_array(int array_id, std::string& label){
    /** Restore file on disk by delegating to disk interface
     * For each block of the array that this server owns, 
     * make an entry with an empty block in the block_map_.
     */ 
	disk_backed_arrays_io_.restore_persistent_array(array_id, label);

    std::list<BlockId> my_blocks;
    int this_server_rank = sip_mpi_attr_.global_rank();
    data_distribution_.generate_server_blocks_list(this_server_rank, array_id, my_blocks, sip_tables_);
    
    // Clear out all existing blocks of array in memory
    block_map_.delete_per_array_map_and_blocks(array_id);

    std::list<BlockId>::const_iterator it;
    for (it = my_blocks.begin(); it != my_blocks.end(); ++it){
        int size = sip_tables_.block_size(*it);         // Number of FP numbers
        double *data = NULL;
        ServerBlock *sb = new ServerBlock(size, data);   // ServerBlock which doesn't "take up any memory"
        sb->set_on_disk();
        block_map_.insert_block(*it, sb);
    }

}


void DiskBackedBlockMap::save_persistent_array(const int array_id,
		const std::string& array_label,
		IdBlockMap<ServerBlock>::PerArrayMap* array_blocks) {
	disk_backed_arrays_io_.save_persistent_array(array_id, array_label, array_blocks);
}


void DiskBackedBlockMap::reset_consistency_status_for_all_blocks(){
	int num_arrays = block_map_.size();
	for (int i=0; i<num_arrays; i++){
		typedef IdBlockMap<ServerBlock>::PerArrayMap PerArrayMap;
		typedef IdBlockMap<ServerBlock>::PerArrayMap::iterator PerArrayMapIterator;
		PerArrayMap *map = block_map_.per_array_map(i);
		PerArrayMapIterator it = map->begin();
		for (; it != map->end(); ++it){
			it->second->reset_consistency_status();
		}
	}
}

void DiskBackedBlockMap::set_max_allocatable_bytes(std::size_t size){
    static bool done_once = false;
    if (!done_once){
        done_once = true;
        max_allocatable_bytes_ = size;
    } else {
        sip::fail("Already set memory limit once !");
    }
}

std::ostream& operator<<(std::ostream& os, const DiskBackedBlockMap& obj){
	os << "block map : " << std::endl;
	os << obj.block_map_;

	os << "disk_backed_arrays_io : " << std::endl;
	os << obj.disk_backed_arrays_io_;

	os << "policy_ : " << std::endl;
	os << obj.policy_;

	os << std::endl;

	return os;
}

} /* namespace sip */
