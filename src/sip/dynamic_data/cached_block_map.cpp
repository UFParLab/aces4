/*
 * cached_block_map.cpp
 *
 *  Created on: Jul 5, 2014
 *      Author: njindal
 */

#include <sstream>
#include <cached_block_map.h>
#include "job_control.h"
#include "sip_mpi_attr.h"
#include "memory_tracker.h"

namespace sip {

CachedBlockMap::CachedBlockMap(int num_arrays)
	: block_map_(num_arrays), cache_(num_arrays), policy_(cache_),

	  max_allocatable_bytes_(sip::JobControl::global->get_max_worker_data_memory_usage()),
//	  pending_delete_bytes_(0),
	  set_mem_limit_once_(false){

}

CachedBlockMap::~CachedBlockMap() {
/** waits for blocks pending delete to be deleted*/
//	WARN(pending_delete_bytes_==0, "pending_delete_bytes != 0 after wait_and_clean_pending in ~CachedBlockMap");
	WARN(pending_delete_.size()==0, "pending_delete_ not empty in ~CachedBlockMap");
}

Block* CachedBlockMap::block(const BlockId& block_id){
	/* If not present in block map,
	 * Check cache; if present, remove and put into block map.
	 */
	Block* block_ptr = block_map_.block(block_id);
	if (block_ptr == NULL){
		block_ptr = cache_.block(block_id);
		if (block_ptr != NULL){
			Block * dont_delete_block = cache_.get_and_remove_block(block_id);
			block_map_.insert_block(block_id, block_ptr);
		}
	}
	return block_ptr;
}



size_t CachedBlockMap::free_up_bytes_in_cache(std::size_t requested_bytes_to_free) {
	size_t freed_bytes = 0;
    while (freed_bytes < requested_bytes_to_free) {
        /** While not enough space is available, check to see if blocks in communication (pending block)
         * have been freed up. If so, check if space has been freed up (continue).
         * If not, check if there is any block to clear from the lru cache.
         * If no blocks in the LRU cache, wait on any pending block. If there are no pending blocks,
         * we have run out of space - Crash !
         */

        size_t cleared_some_pending = test_and_clean_pending();
        freed_bytes += cleared_some_pending;
        if (cleared_some_pending>0)
            continue;
        bool any_blocks_for_removal = policy_.any_blocks_for_removal();
        if (!any_blocks_for_removal){
            size_t  cleared_any_pending = wait_and_clean_any_pending();
            freed_bytes += cleared_any_pending;
//            if (cleared_any_pending == 0){
//                throw std::out_of_range("No blocks to remove from cache or pending deletes");
//            }
            if(cleared_any_pending == 0) {
            	std::stringstream ss;
            	ss << "Worker at rank " << SIPMPIAttr::get_instance().global_rank() << "Sial program "
            			<< JobControl::global->get_program_name() << ":" << current_line() << std::endl;
            	ss << " Out of memory.  There are no pending or cached blocks to delete." << std::endl;
            	ss << " allocated_bytes = " << MemoryTracker::global->allocated_bytes_ << std::endl;
            	ss << " requested_bytes_to_free " << requested_bytes_to_free << std::endl;
            	ss << " max_allocatable_bytes_" << max_allocatable_bytes_ << std::endl;
            	ss << " available " << (max_allocatable_bytes_ - MemoryTracker::global->allocated_bytes_) << std::endl << std::flush;
            	return freed_bytes;
            }
        } else {
        	Block* scratch;
            BlockId block_id = policy_.get_next_block_for_removal(scratch);
            Block* tmp_block_ptr = cache_.get_and_remove_block(block_id);
            size_t block_bytes = tmp_block_ptr->size() * sizeof(double);
            freed_bytes += block_bytes;
            delete tmp_block_ptr;
        }
    }
    return freed_bytes;
}


double* CachedBlockMap::allocate_data(std::size_t size, bool initialize){
//	double* DiskBackedBlockMap::allocate_data(size_t size, bool initialize){
//		size_t to_free = (remaining_doubles_ <= size) ?  size : 0;
//	{//for gdb
//	    volatile int i = 0;
//	    char hostname[256];
//	    gethostname(hostname, sizeof(hostname));
//	    printf("PID %d on %s ready for attach\n", getpid(), hostname);
//	    fflush(stdout);
//	    while (0 == i)
//	        sleep(5);
//	}
		size_t bytes_to_allocate = size*sizeof(double);
		size_t allocated_bytes = MemoryTracker::global->get_allocated_bytes();
//		std::cerr << "size, bytes_to_allocate, allocated_bytes = " <<
//				size << "," << bytes_to_allocate << "," << allocated_bytes <<
//				std::endl << std::flush;
		size_t bytes_to_free = 0;
		if ((allocated_bytes + bytes_to_allocate) > max_allocatable_bytes_){
			bytes_to_free = bytes_to_allocate;
		}
		size_t freed = 0;
		double* data = NULL;
		bool allocated = false;
//		std::cerr << "entering loop in allocate_data" << std::endl << std::flush;
//		std::cerr << "size, bytes_to_allocate, allocated_bytes, bytes_to_free, freed" <<
//				size << "," << bytes_to_allocate << "," << allocated_bytes << "," << bytes_to_free<< "," <<freed  <<
//				std::endl << std::flush;
		while (!allocated) {
//			std::cerr << "in loop in allocate_data" << std::endl << std::flush;
			freed = free_up_bytes_in_cache(bytes_to_free); //returns 0 if to_free <= 0
			try {
				if (initialize){
				data = new double[size]();
				}
				else {
					data = new double[size];
				}
				MemoryTracker::global->inc_allocated(size);
//				WARN(MemoryTracker::global->get_allocated_bytes() <= max_allocatable_bytes_ ,
//						"Memory allocated at worker exceeds max_allocatable_bytes_");
				allocated = true;
			}
			catch (const std::bad_alloc& ba) {
				//memory allocation failed.  If the requested amount was freed, then free more blocks and try again
				//However, if freed < to_free, then just give up
				if (freed < bytes_to_free){
					fail(" Could not free requested amount of memory.  CachedBlockMap::allocate_data failed");
				}
				bytes_to_free = bytes_to_allocate; //in case bytes_to_free was 0, we want to try again.
			}
		}
		return data;

}



size_t CachedBlockMap::test_and_clean_pending(){
// Cleaning pending blocks is relevant only for the MPI version
    size_t cleared_any_pending = 0;
#ifdef HAVE_MPI
    std::list<Block*>::iterator it;
    for (it = pending_delete_.begin(); it != pending_delete_.end(); ){
        Block* bptr = *it;
        if (bptr->test()){
        	size_t bytes_freed = bptr->size() * sizeof(double);
 //           pending_delete_bytes_ -= bytes_freed;
            cleared_any_pending += bytes_freed;
            delete *it;
            pending_delete_.erase(it++);
        }
        else ++it;
    }
#endif // HAVE_MPI
    return cleared_any_pending;
}

size_t CachedBlockMap::wait_and_clean_pending(){
// Cleaning of pending blocks only relevant for the MPI version
	size_t cleaned_bytes = 0;
#ifdef HAVE_MPI
	std::list<Block*>::iterator it;
	for (it = pending_delete_.begin(); it != pending_delete_.end(); ){
		Block* bptr = *it;
		bptr->wait();
		size_t bytes_freed = bptr->size() * sizeof(double);
//		pending_delete_bytes_ -= bytes_freed;
		cleaned_bytes += bytes_freed;
		delete *it;
		pending_delete_.erase(it++);
	}
#endif // HAVE_MPI
	return cleaned_bytes;
}


//int CachedBlockMap::pending_list_size(){
//	return pending_delete_.size();
//}

size_t CachedBlockMap::wait_and_clean_any_pending() {
	size_t cleaned_bytes = 0;
#ifdef HAVE_MPI
	std::list<Block*>::iterator it = pending_delete_.begin();
	if (it != pending_delete_.end()){
		Block* bptr = *it;
		bptr->wait();
		size_t bytes_freed = bptr->size() * sizeof(double);
//		pending_delete_bytes_ -= bytes_freed;
		cleaned_bytes += bytes_freed;
		delete *it;
		pending_delete_.erase(it);
        return cleaned_bytes;
	}
#endif
    return cleaned_bytes;
}

void CachedBlockMap::insert_block(const BlockId& block_id, Block* block_ptr){
//	/* Free up space to insert the new block in the block map
//	 * Method will fail if it cannot find blocks to eject to free up space.
//	 */
//	std::size_t block_size = block_ptr->size() * sizeof(double);
//	free_up_bytes_in_cache(block_size);
//
	block_map_.insert_block(block_id, block_ptr);
//	allocated_bytes_ += block_size;
}

void CachedBlockMap::cached_delete_block(const BlockId& block_id){
	/* Remove block from block map and put in cache */
	Block* block_ptr = block_map_.get_and_remove_block(block_id);
//	std::size_t bytes_in_block = block_ptr->size() * sizeof(double);
//	free_up_bytes_in_cache(bytes_in_block);
	cache_.insert_block(block_id, block_ptr);
	policy_.touch(block_id);

//	block_map_.delete_block(block_id);

}

void CachedBlockMap::delete_block(const BlockId& block_id){
	Block* tmp_block_ptr = block_map_.get_and_remove_block(block_id);
#ifdef HAVE_MPI
	if (!tmp_block_ptr->test()){
//		pending_delete_bytes_ += tmp_block_ptr->size() * sizeof(double);
		pending_delete_.push_back(tmp_block_ptr);
	} else 
	    delete tmp_block_ptr;
#else // HAVE_MPI
    delete tmp_block_ptr;
#endif // HAVE_MPI

}

void CachedBlockMap::insert_per_array_map(int array_id, IdBlockMap<Block>::PerArrayMap* map_ptr){
	block_map_.insert_per_array_map(array_id, map_ptr);
}

void CachedBlockMap::delete_per_array_map_and_blocks(int array_id){
 	 block_map_.delete_per_array_map_and_blocks(array_id);
	 cache_.delete_per_array_map_and_blocks(array_id);
}

IdBlockMap<Block>::PerArrayMap* CachedBlockMap::get_and_remove_per_array_map(int array_id){
	IdBlockMap<Block>::PerArrayMap* map_ptr = block_map_.get_and_remove_per_array_map(array_id);
	return map_ptr;
}

void CachedBlockMap::set_max_allocatable_bytes(std::size_t bytes){
        max_allocatable_bytes_ = bytes;
}

std::ostream& operator<<(std::ostream& os, const CachedBlockMap& obj){
	os << "Block Map : " << std::endl;
	os << obj.block_map_;
	os << std::endl;

	os << "Cache : " << std::endl;
	os << obj.cache_;
	os << std::endl;

	return os;
}

} /* namespace sip */
