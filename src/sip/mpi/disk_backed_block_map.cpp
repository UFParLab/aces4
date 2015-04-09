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
//#include "TAU.h"

namespace sip {

/** Template specialization for ServerBlock.
 *
 * Regular LRU Array policy is to evict the
 * "first" block from the least recently used array.
 * For ServerBlocks however, since they are not
 * removed from the BlockMap, but are "emptied out",
 * the regular array policy won't work.
 *
 * This methods first chooses an array whose block
 * is to be evicted. It then finds one, skipping
 * over those blocks which have been "emptied" out.
 */
template<> BlockId LRUArrayPolicy<ServerBlock>::get_next_block_for_removal(){
    /** Return an arbitrary block from the least recently used array
     */
    if(lru_list_.empty())
        throw std::out_of_range("No blocks have been touched, yet block requested for flushing");
    while (!lru_list_.empty()) {
        int to_remove_array = lru_list_.back();
        IdBlockMap<ServerBlock>::PerArrayMap* array_map = block_map_.per_array_map(to_remove_array);
        IdBlockMap<ServerBlock>::PerArrayMap::iterator it = array_map->begin();
        for (; it != array_map->end(); it ++) {
            ServerBlock *blk = it->second;
            if (blk !=NULL && blk->get_data() != NULL) {
                return it->first;
            }
        }
        lru_list_.pop_back();
    }
    throw std::out_of_range("No server blocks to remove - all empty or none present !");
}

DiskBackedBlockMap::DiskBackedBlockMap(const SipTables& sip_tables, 
                                       const SIPMPIAttr& sip_mpi_attr, 
                                       const DataDistribution& data_distribution,
                                       ServerTimer& server_timer/*, 
                                       ServerInterpreter& server_interpreter*/)
                                       : sip_tables_(sip_tables), sip_mpi_attr_(sip_mpi_attr)
                                       , data_distribution_(data_distribution), block_map_(sip_tables.num_arrays())
                                       , disk_backed_arrays_io_(sip_tables, sip_mpi_attr, data_distribution)
                                       , policy_(block_map_), server_timer_(server_timer) 
                                       //, server_interpreter_(server_interpreter)
                                       , max_allocatable_bytes_(sip::GlobalState::get_max_data_memory_usage())
                                       , block_access_counter(0), data_volume_counter(0)
                                       , disk_volume_counter(0), disk_read_counter(0)
                                       , disk_write_counter(0), total_stay_in_memory_counter(0)
                                       , max_allocated_bytes(0), max_block_size(0) {
    
    //create_blocks_bucket();
    
}

DiskBackedBlockMap::~DiskBackedBlockMap(){
    std::stringstream ss;
    ss << "Server: " << sip_mpi_attr_.global_rank() << std::endl;
    ss << "Max allocatable bytes: " << max_allocatable_bytes_ << std::endl;
    ss << "Max allocated bytes: " << max_allocated_bytes << std::endl;
    ss << "Max block size: " << max_block_size << std::endl;
    ss << "Block access counter: " << block_access_counter << std::endl;
    ss << "Data volume counter: " << data_volume_counter << std::endl;
    ss << "Disk volume counter: " << disk_volume_counter << std::endl;
    ss << "Disk read counter: " << disk_read_counter << std::endl;
    ss << "Disk write counter: " << disk_write_counter << std::endl;
    ss << "Average stay in memory counter: " << total_stay_in_memory_counter / (disk_write_counter+1) << std::endl;
    //std::cout << ss.str() << std::endl;
}

void DiskBackedBlockMap::create_blocks_bucket() {
    int num_arrays = sip_tables_.num_arrays();

    for (int array_id = 0; array_id < num_arrays; array_id ++) {
        bool is_remote = sip_tables_.is_distributed(array_id) 
                      || sip_tables_.is_served(array_id);
        if (is_remote) {
            std::list<BlockId> &server_blocks = disk_backed_arrays_io_.my_blocks_[array_id];
            std::map<int, std::map<int, std::set<BlockId> > > &array_bucket = blocks_bucket[array_id];
            int array_rank = sip_tables_.array_rank(array_id);
            
            for (std::list<BlockId>::iterator iter = server_blocks.begin(); iter != server_blocks.end(); iter ++) {
                for (int array_index = 0; array_index < array_rank; array_index ++) {
                    array_bucket[array_index][iter->index_values(array_index)].insert(*iter);
                }
                //std::cout << *iter << std::endl;
            }
            /*
            std::cout << "Array: " << sip_tables_.array_name(array_id) << std::endl;
            for (int array_index = 0; array_index < array_rank; array_index ++) {
                std::cout << "Slot: " << array_index << std::endl;
                
                auto &block_bucket = blocks_bucket[array_id][array_index];
                for (auto iter = block_bucket.begin(); iter != block_bucket.end(); iter ++) {
                    std::cout << "Value: " << iter->first << ", Blocks: " << iter->second.size() << std::endl;
                    for (auto it = iter->second.begin(); it != iter->second.end(); it ++) {
                        std::cout << *it << std::endl;
                    }
                }
            }
            */
        }
    }
    //sip_abort();
}

void DiskBackedBlockMap::select_blocks(int array_id, int index[6], std::set<BlockId> &blocks) {
    int array_rank = sip_tables_.array_rank(array_id);
    
    int min_num = std::numeric_limits<int>::max();
    int min_num_rank = -1;

    for (int array_index = 0; array_index < array_rank; array_index ++) {
        if (index[array_index] != -1 && blocks_bucket[array_id][array_index][index[array_index]].size() < min_num) {
            min_num = blocks_bucket[array_id][array_index][index[array_index]].size();
            min_num_rank = array_index;
        }
    }
    
    if (min_num_rank == -1) {
        std::list<BlockId> &server_blocks = disk_backed_arrays_io_.my_blocks_[array_id];
        for (std::list<BlockId>::iterator iter = server_blocks.begin(); iter != server_blocks.end(); iter ++) {
            blocks.insert(*iter);
        }
        return ;
    }
    
    std::set<BlockId> &blocks_set = blocks_bucket[array_id][min_num_rank][index[min_num_rank]];
    
    for (std::set<BlockId>::iterator iter = blocks_set.begin(); iter != blocks_set.end(); iter ++) {
        bool filtered = false;
        for (int array_index = 0; array_index < array_rank; array_index ++) {
            if (index[array_index] != -1) {
                if (iter->index_values(array_index) != index[array_index]) {
                    filtered = true;
                    break;
                }
            }
        }
        if (filtered == false) {
            blocks.insert(*iter);
        }
    }
}


void DiskBackedBlockMap::read_block_from_disk(ServerBlock*& block, const BlockId& block_id, size_t block_size){
    /** Allocates space for a block, reads block from disk
     * into newly allocated space, sets the in_memory flag,
     * inserts into block_map_ if needed
     */
    block = allocate_block(block, block_size, false);
    server_timer_.start_timer(current_line(), ServerTimer::READTIME);
    disk_backed_arrays_io_.read_block_from_disk(block_id, block);
    server_timer_.pause_timer(current_line(), ServerTimer::READTIME);
    //block->set_in_memory();
    disk_read_counter++;
    //server_interpreter_.remove_read_block(block_id);
    //std::cout << "disk read: " << disk_read_counter << std::endl;
}

void DiskBackedBlockMap::get_array_blocks(int array_id, std::set<BlockId> &blocks_set) {

    std::list<BlockId> &server_blocks = disk_backed_arrays_io_.my_blocks_[array_id];
    //std::cout << "Array:" << array_id << std::endl;
    
    blocks_set.clear();
    
    for (std::list<BlockId>::iterator iter = server_blocks.begin(); iter != server_blocks.end(); iter ++) {
        BlockId &block_id = *iter;
        ServerBlock* block = block_map_.block(block_id);
        if (block == NULL || !block->is_in_memory()) {
            blocks_set.insert(block_id);
        }
    }
}

void DiskBackedBlockMap::prefetch_array(int array_id) {
    //TAU_START("prefetch");

    std::list<BlockId> server_blocks = disk_backed_arrays_io_.my_blocks_[array_id];
    //std::cout << "Array:" << array_id << std::endl;
    
    for (std::list<BlockId>::iterator iter = server_blocks.begin(); iter != server_blocks.end(); iter ++) {
        BlockId &block_id = *iter;
        
        ServerBlock* block = block_map_.block(block_id);
        size_t block_size = sip_tables_.block_size(block_id);
        
        if (block == NULL) { // This is a copy from code below.
            std::stringstream msg;
            msg << "S " << sip_mpi_attr_.global_rank();
            msg << " : uninitialized block " << block_id << " will be pre-fetched. "<< std::endl;
            SIP_LOG(std::cout << msg.str() << std::flush);
            //TAU_START("allocate block");
            block = allocate_block(NULL, block_size, false);
            //TAU_STOP("allocate block");
            block_map_.insert_block(block_id, block);
            disk_backed_arrays_io_.iread_block_from_disk(block_id, block);
            std::cout << "Read null block: " << block_id << std::endl;
        } else if(!block->is_in_memory()) {
            if (block->is_on_disk()){
                //TAU_START("allocate block");
                block = allocate_block(block, block_size, false);
                //TAU_STOP("allocate block");
                disk_backed_arrays_io_.iread_block_from_disk(block_id, block);
                std::cout << "Read block: " << block_id << std::endl;
            } else {
                sip::fail("get_block_for_reading : ServerBlock neither on memory or on disk !");
            }
        } else {
            std::cout << "Block is in memory:" << block_id << std::endl;
        }
        block->set_in_memory();
    }
    
    //TAU_STOP("prefetch");
}

bool DiskBackedBlockMap::is_block_prefetchable(const BlockId &block_id) {
    ServerBlock* block = block_map_.block(block_id);
    
    std::size_t block_size = sip_tables_.block_size(block_id);
    std::size_t remaining_mem = max_allocatable_bytes_ - ServerBlock::allocated_bytes();
    std::size_t block_size_in_bytes = block_size * sizeof(double);
    
    return (block == NULL || !block->is_in_memory()) && (block_size_in_bytes < remaining_mem);
}

bool DiskBackedBlockMap::is_block_in_memory(const BlockId &block_id) {
    ServerBlock* block = block_map_.block(block_id);
    
    return block != NULL && block->is_in_memory();
}

bool DiskBackedBlockMap::is_block_dirty(const BlockId &block_id) {
    ServerBlock* block = block_map_.block(block_id);
    
    return block == NULL || block->is_dirty();
}

bool DiskBackedBlockMap::prefetch_block(const BlockId &block_id) {
    //TAU_START("prefetch");
    //std::cout << "Prefetching block " << block_id << std::endl;
    
    bool ret = false;
    std::size_t block_size = sip_tables_.block_size(block_id);
    std::size_t remaining_mem = max_allocatable_bytes_ - ServerBlock::allocated_bytes();
    std::size_t block_size_in_bytes = block_size * sizeof(double);
    
    if (block_size_in_bytes < remaining_mem) {
        return ret;
    }
    
    ServerBlock* block = block_map_.block(block_id);
    
    if (block == NULL) { // This is a copy from code below.
        std::stringstream msg;
        msg << "S " << sip_mpi_attr_.global_rank();
        msg << " : uninitialized block " << block_id << " will be pre-fetched. "<< std::endl;
        SIP_LOG(std::cout << msg.str() << std::flush);
        block = allocate_block(NULL, block_size, false);
        block_map_.insert_block(block_id, block);
        disk_backed_arrays_io_.iread_block_from_disk(block_id, block);
        //std::cout << "Read null block: " << block_id << std::endl;
        ret = true;
    } else if(!block->is_in_memory()) {
        if (block->is_on_disk()){
            block = allocate_block(block, block_size, false);
            //block->allocate_in_memory_data(false);
            disk_backed_arrays_io_.iread_block_from_disk(block_id, block);
            //std::cout << "Read block: " << block_id << std::endl;
            ret = true;
        } else {
            sip::fail("get_block_for_reading : ServerBlock neither on memory or on disk !");
        }
    } else {
        //std::cout << "Block is in memory:" << block_id << std::endl;
    }
    
    //TAU_STOP("prefetch");
    
    return ret;
}

bool DiskBackedBlockMap::write_block(const BlockId &block_id) {
    ServerBlock* block = block_map_.block(block_id);
    
    if (block == NULL) { // This is a copy from code below.
        std::stringstream msg;
        msg << "S " << sip_mpi_attr_.global_rank();
        msg << " : uninitialized block " << block_id << " will not be written to disk. "<< std::endl;
        SIP_LOG(std::cout << msg.str() << std::flush);
        return false;
    } else if(block->is_in_memory()) {
        if (block->is_dirty()){
            disk_backed_arrays_io_.write_block_to_disk(block_id, block); //// iwrite
            return true;
        }
    }
    
    return false;
}

void DiskBackedBlockMap::get_dead_blocks(int array_id, std::set<BlockId> &blocks_set) {

    std::list<BlockId> server_blocks = disk_backed_arrays_io_.my_blocks_[array_id];
    //std::cout << "Array:" << array_id << std::endl;
    
    blocks_set.clear();
    
    for (std::list<BlockId>::iterator iter = server_blocks.begin(); iter != server_blocks.end(); iter ++) {
        BlockId &block_id = *iter;
        
        ServerBlock* block = block_map_.block(block_id);
        
        if (block == NULL) { // This is a copy from code below.
            std::stringstream msg;
            msg << "S " << sip_mpi_attr_.global_rank();
            msg << " : uninitialized block " << block_id << " will not be freed. "<< std::endl;
            SIP_LOG(std::cout << msg.str() << std::flush);
        } else if(block->is_in_memory()) {
            if (block->is_dirty()){
                blocks_set.insert(block_id);
            } else {
                block->free_in_memory_data();
                block->unset_in_memory();
                block->set_on_disk();
            }
        }
    }
}

void DiskBackedBlockMap::free_array(int array_id) {
    std::list<BlockId> server_blocks = disk_backed_arrays_io_.my_blocks_[array_id];
    
    for (std::list<BlockId>::iterator iter = server_blocks.begin(); iter != server_blocks.end(); iter ++) {
        BlockId &block_id = *iter;
        
        ServerBlock* block = block_map_.block(block_id);
        
        if (block == NULL) { // This is a copy from code below.
            std::stringstream msg;
            msg << "S " << sip_mpi_attr_.global_rank();
            msg << " : uninitialized block " << block_id << " will not be removed from memory. "<< std::endl;
            SIP_LOG(std::cout << msg.str() << std::flush);
        } else if(block->is_in_memory()) {
            if (block->is_dirty()){
                disk_backed_arrays_io_.iwrite_block_to_disk(block_id, block);
            }
            block->free_in_memory_data();
            block->unset_in_memory();
            block->set_on_disk();
        }
    }
}

void DiskBackedBlockMap::free_block(const BlockId &block_id) {
    ServerBlock* block = block_map_.block(block_id);
    
    if (block == NULL) { // This is a copy from code below.
        std::stringstream msg;
        msg << "S " << sip_mpi_attr_.global_rank();
        msg << " : uninitialized block " << block_id << " will not be removed from memory. "<< std::endl;
        SIP_LOG(std::cout << msg.str() << std::flush);
    } else if(block->is_in_memory()) {
        if (block->is_dirty()){
            disk_backed_arrays_io_.write_block_to_disk(block_id, block); //// iwrite
        }
        block->free_in_memory_data();
        block->unset_in_memory();
        block->set_on_disk();
    }
}

void DiskBackedBlockMap::write_block_to_disk(const BlockId& block_id, ServerBlock* block){
    server_timer_.start_timer(current_line(), ServerTimer::WRITETIME);
    disk_backed_arrays_io_.write_block_to_disk(block_id, block);
    server_timer_.pause_timer(current_line(), ServerTimer::WRITETIME);

    block->unset_dirty();
    block->set_on_disk();
    //block->leave_memory_counter = block_access_counter;
    //total_stay_in_memory_counter += block->leave_memory_counter - block->enter_memory_counter;
    disk_write_counter++;
    //std::cout << "disk write: " << disk_write_counter << std::endl;
}

void DiskBackedBlockMap::update_array_distance(std::map<int, int> &new_array_distance) {
    array_distance_.clear();
    
    for (std::map<int, int>::iterator iter = new_array_distance.begin(); iter != new_array_distance.end(); iter ++) {
        array_distance_.insert(std::pair<int,int>(iter->second,iter->first));
    }
}

void DiskBackedBlockMap::remove_block_from_memory(std::size_t needed_bytes) {
    std::size_t remaining_mem = max_allocatable_bytes_ - ServerBlock::allocated_bytes();
    
    if (needed_bytes <= remaining_mem) {
        return;
    }
    
    while (array_distance_.count(-1) != 0) {
        std::multimap<int,int>::iterator iter = array_distance_.find(-1);
        //std::cout << "Remove dead array: " << sip_tables_.array_name(iter->second) << std::endl;
        
        IdBlockMap<ServerBlock>::PerArrayMap* array_map = per_array_map(iter->second);
        for (IdBlockMap<ServerBlock>::PerArrayMap::iterator it = array_map->begin(); it != array_map->end(); it ++) {
            const BlockId &bid = it->first;
            ServerBlock* block = block_map_.block(bid);
            if (block->is_in_memory()) {
                if (block->is_dirty()) {
                    write_block_to_disk(bid, block);
                }
                block->free_in_memory_data();
                remaining_mem = max_allocatable_bytes_ - ServerBlock::allocated_bytes();
                if (needed_bytes <= remaining_mem) {
                    return;
                }
            }
        }
        array_distance_.erase(iter);
    }
    
    std::multimap<int,int>::reverse_iterator iter = array_distance_.rbegin();
    while (iter != array_distance_.rend()) {
        //std::cout << "Remove Array: " << sip_tables_.array_name(iter->second) << " with distance " << iter->first << std::endl;
        
        IdBlockMap<ServerBlock>::PerArrayMap* array_map = per_array_map(iter->second);
        for (IdBlockMap<ServerBlock>::PerArrayMap::iterator it = array_map->begin(); it != array_map->end(); it ++) {
            const BlockId &bid = it->first;
            ServerBlock* block = block_map_.block(bid);
            if (block->is_in_memory()) {
                if (block->is_dirty()) {
                    write_block_to_disk(bid, block);
                }
                block->free_in_memory_data();
                remaining_mem = max_allocatable_bytes_ - ServerBlock::allocated_bytes();
                if (needed_bytes <= remaining_mem) {
                    return;
                }
            }
        }
        //array_distance_.erase(std::next(iter).base() );;
        iter = array_distance_.rbegin();
    }
    
    throw std::out_of_range("No blocks to remove!");
}

ServerBlock* DiskBackedBlockMap::allocate_block(ServerBlock* block, size_t block_size, bool initialize){
    /** If enough memory remains, allocates block and returns.
     * Otherwise, frees up memory by writing out dirty blocks
     * till enough memory has been obtained, then allocates
     * and returns block.
     */
    std::size_t remaining_mem = max_allocatable_bytes_ - ServerBlock::allocated_bytes();
    std::size_t block_size_in_bytes = block_size * sizeof(double);
    //std::set<BlockId> &unused_blocks = server_interpreter_.get_unused_blocks();
    //std::set<BlockId> &dirty_blocks = server_interpreter_.get_dirty_blocks();
    
    while (!false && block_size_in_bytes > remaining_mem){
        try{
            /*
            if (unused_blocks.empty() && dirty_blocks.empty()) {
                throw std::out_of_range("All blocks have been removed and memory is still insufficient for new block.");
            }
            
            if (!unused_blocks.empty()) {
                std::set<BlockId>::iterator iter = unused_blocks.begin();
                ServerBlock* block = block_map_.block(*iter);
                block->free_in_memory_data();
                unused_blocks.erase(iter);
            } else if (!dirty_blocks.empty()) {
                std::set<BlockId>::iterator iter = dirty_blocks.begin();
                ServerBlock* block = block_map_.block(*iter);
                write_block_to_disk(*iter, block);
                block->free_in_memory_data();
                dirty_blocks.erase(iter);
            }
            remaining_mem = max_allocatable_bytes_ - ServerBlock::allocated_bytes();
            */
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

			// Junmin's fix :
			// As a result of freeing up block memory, the remaining memory should
			// have increased. Otherwise it will go into an infinite loop.
			if (!(remaining_mem < max_allocatable_bytes_ - ServerBlock::allocated_bytes())) {
				throw std::out_of_range("Break now.");
			}
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
        block->allocate_in_memory_data(initialize);
    }

    //block->enter_memory_counter = block_access_counter;

    if (max_block_size < block->size()) {
        max_block_size = block->size();
    }
    
    if (max_allocated_bytes < ServerBlock::allocated_bytes())
    {
        max_allocated_bytes = ServerBlock::allocated_bytes();
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
                disk_volume_counter+=block_size*sizeof(double);
            } else {
                block->allocate_in_memory_data();
            }
        }
    }

    block->set_in_memory();
    block->set_dirty();

    policy_.touch(block_id);
    block_access_counter++;
    data_volume_counter+=block_size*sizeof(double);

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

    policy_.touch(block_id);
        block_access_counter++;

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
        //block->free_in_memory_data();
        if(!block->is_in_memory())
            if (block->is_on_disk()){
                //std::cout << "Read block from disk " << block_id << std::endl;
                read_block_from_disk(block, block_id, block_size);
                disk_volume_counter+=block_size*sizeof(double);
                //server_interpreter_.remove_read_block(block_id);
            } else {
                sip::fail("get_block_for_reading : ServerBlock neither on memory or on disk !");
            }
    }
    
    block->set_in_memory();

    policy_.touch(block_id);
    block_access_counter++;
    data_volume_counter+=block_size*sizeof(double);

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
    disk_backed_arrays_io_.delete_array(array_id, per_array_map);            // remove from disk
    SIP_LOG(std::cout << "S " << sip_mpi_attr_.global_rank() << " : Deleted blocks from disk" << std::endl;);

    block_map_.delete_per_array_map_and_blocks(array_id);    // remove from memory
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

    unsigned int *array_bit_map_ = disk_backed_arrays_io_.get_array_bit_map(array_id);
    int block_index_ = 0;
    
    for (std::list<BlockId>::iterator iter = my_blocks.begin(); iter != my_blocks.end(); ++iter) {
        if (array_bit_map_[block_index_/32] & (1<<(block_index_%32))) {
            int size = sip_tables_.block_size(*iter);         // Number of FP numbers
            double *data = NULL;
            ServerBlock *sb = new ServerBlock(size, data);   // ServerBlock which doesn't "take up any memory"
            sb->set_on_disk();
            block_map_.insert_block(*iter, sb);
        }
        block_index_ ++;
    }
}


void DiskBackedBlockMap::save_persistent_array(const int array_id,
        const std::string& array_label,
        IdBlockMap<ServerBlock>::PerArrayMap* array_blocks) {
    server_timer_.start_timer(current_line(), ServerTimer::WRITETIME);
    disk_backed_arrays_io_.save_persistent_array(array_id, array_label, array_blocks);
    server_timer_.pause_timer(current_line(), ServerTimer::WRITETIME);

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
