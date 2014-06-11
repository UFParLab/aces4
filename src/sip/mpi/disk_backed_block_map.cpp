/*
 * disk_backed_block_map.cpp
 *
 *  Created on: Apr 17, 2014
 *      Author: njindal
 */

#include <disk_backed_block_map.h>
#include "server_block.h"
#include "block_id.h"
#include <iostream>
#include <string>
#include <sstream>

namespace sip {

DiskBackedBlockMap::DiskBackedBlockMap(const SipTables& sip_tables,
		const SIPMPIAttr& sip_mpi_attr, const DataDistribution& data_distribution) :
		sip_tables_(sip_tables), sip_mpi_attr_(sip_mpi_attr), data_distribution_(data_distribution),
		block_map_(sip_tables.num_arrays()),
		disk_backed_arrays_io_(sip_tables, sip_mpi_attr, data_distribution),
        policy_(block_map_){
}

DiskBackedBlockMap::~DiskBackedBlockMap(){}

void DiskBackedBlockMap::read_block_from_disk(ServerBlock*& block, const BlockId& block_id, size_t block_size){
    /** Allocates space for a block, reads block from disk
     * into newly allocated space, sets the in_memory flag,
     * inserts into block_map_ if needed
     */
    //bool was_in_map = true;
    //ServerBlock* block = block_map_.block(block_id);
    //if (block == NULL)
    //    was_in_map = false;

	block = allocate_block(block, block_size);
    //if (!was_in_map)
    //	block_map_.insert_block(block_id, block);

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
	std::size_t remaining_mem = ServerBlock::remaining_memory();

	while (block_size > remaining_mem){
        BlockId bid = policy_.get_next_block_for_removal();
        ServerBlock* blk = block_map_.block(bid);
        if(blk->is_dirty()){
            write_block_to_disk(bid, blk);
        }
        blk->free_in_memory_data();
		remaining_mem = ServerBlock::remaining_memory();
	}

	std::stringstream ss;
	ss << "S " << sip_mpi_attr_.company_rank() << " : Could not allocate memory for block of size "
			<< block_size << ", Remaining Memory :" << ServerBlock::remaining_memory() << std::endl;
	sip :: check (block_size <= ServerBlock::remaining_memory(), ss.str());
   
    if (block == NULL)
	    block = new ServerBlock(block_size, initialize);
    else
        block->allocate_in_memory_data();
	return block;

}


//ServerBlock* DiskBackedBlockMap::get_or_create_block(const BlockId& block_id,
//		size_t block_size, bool initialize) {
//	// TODO ===============================================
//	// TODO Complete this method.
//	// TODO ===============================================
//	ServerBlock* block = block_map_.block(block_id);
//	if (block == NULL){
//		block = read_block_from_disk(block_id, block_size);
//	}
//	//ServerBlock* block = block_map_.get_or_create_block(block_id, block_size, initialize);
//	//return block;
//	return block;
//}


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

IdBlockMap<ServerBlock>::PerArrayMap* DiskBackedBlockMap::get_and_remove_per_array_map(int array_id){
	return block_map_.get_and_remove_per_array_map(array_id);
}

void DiskBackedBlockMap::insert_per_array_map(int array_id, IdBlockMap<ServerBlock>::PerArrayMap* map_ptr){
	block_map_.insert_per_array_map(array_id, map_ptr);
}


void DiskBackedBlockMap::delete_per_array_map_and_blocks(int array_id){
    policy_.remove_all_blocks_for_array(array_id);          // remove from block replacement policy
	disk_backed_arrays_io_.delete_array(array_id);			// remove from disk
	block_map_.delete_per_array_map_and_blocks(array_id);	// remove from memory
}

void DiskBackedBlockMap::restore_persistent_array(int array_id, std::string& label){
    /** Restore file on disk by delegating to disk interface
     * For each block of the array that this server owns, 
     * make an entry with an empty block in the block_map_.
     */ 
	disk_backed_arrays_io_.restore_persistent_array(array_id, label);

    std::list<BlockId> all_blocks;
    generate_all_blocks_list(array_id, all_blocks);
    
    // Clear out all existing blocks of array in memory
    block_map_.delete_per_array_map_and_blocks(array_id);

    std::list<BlockId>::const_iterator it;
    for (it = all_blocks.begin(); it != all_blocks.end(); ++it){
        int size = sip_tables_.block_size(*it);         // Number of FP numbers
        double *data = NULL;
        ServerBlock *sb = new ServerBlock(size, data);   // ServerBlock which doesn't "take up any memory"
        sb->set_on_disk();
        block_map_.insert_block(*it, sb);
    }

}

void DiskBackedBlockMap::generate_all_blocks_list(int array_id, std::list<BlockId>& all_blocks){
    index_value_array_t upper;
    const index_selector_t& selector = sip_tables_.selectors(array_id);
	int rank = sip_tables_.array_rank(array_id);
	index_value_array_t upper_index_vals;
	index_value_array_t lower_index_vals;
	index_value_array_t current_index_vals;
	std::fill(upper_index_vals, upper_index_vals + MAX_RANK, unused_index_value);
	std::fill(lower_index_vals, lower_index_vals + MAX_RANK, unused_index_value);
	std::fill(current_index_vals, current_index_vals + MAX_RANK, unused_index_value);

    bool at_least_one_iter = false;

	// Initialize
	for (int i = 0; i < rank; i++) {
		int selector_index = selector[i];
		int lower = sip_tables_.lower_seg(selector_index);
		int upper = lower + sip_tables_.num_segments(selector_index);
		lower_index_vals[i] = lower;
		current_index_vals[i] = lower_index_vals[i];
		upper_index_vals[i] = upper;
        if (upper - lower >= 1)
            at_least_one_iter = true;
	}

    BlockId upper_bid(array_id, upper_index_vals);
    BlockId lower_bid(array_id, lower_index_vals);
    //std::cout << " Upper : " << upper_bid << std::endl;
    //std::cout << " Lower : " << lower_bid << std::endl;

        
   if (at_least_one_iter){
        // Save a BlockId into the output list
        BlockId bid(array_id, current_index_vals);
        all_blocks.push_back(bid);
//std::cout << " Generated : " << bid << " of " << sip_tables_.array_name(bid.array_id()) << std::endl;
   }

   while (increment_indices(rank, upper_index_vals, lower_index_vals, current_index_vals)){
        BlockId bid(array_id, current_index_vals);
        all_blocks.push_back(bid);
//std::cout << " Generated : " << bid << " of " << sip_tables_.array_name(bid.array_id()) << std::endl;
   }
}

bool DiskBackedBlockMap::increment_indices(int rank, index_value_array_t& upper, index_value_array_t& lower, index_value_array_t& current){
    int pos = 0;
    while (pos < rank){
        // Increment indices.
        if (current[pos] >= upper[pos] - 1) {
            current[pos] = lower[pos];
            pos++;
        } else {
            current[pos]++;
            return true;
        }
    }
    return false;
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


std::ostream& operator<<(std::ostream& os, const DiskBackedBlockMap& obj){
	os << "block map : " << std::endl;
	os << obj.block_map_;

	os << "disk_backed_arrays_io : " << std::endl;
	os << obj.disk_backed_arrays_io_;

	os << std::endl;

	return os;
}

} /* namespace sip */
