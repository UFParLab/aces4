/*
 * cached_block_map.cpp
 *
 *  Created on: Jul 5, 2014
 *      Author: njindal
 */

#include <cached_block_map.h>
#include "global_state.h"
namespace sip {

CachedBlockMap::CachedBlockMap(int num_arrays)
	: block_map_(num_arrays), cache_(num_arrays), policy_(cache_),
	  max_allocatable_bytes_(sip::GlobalState::get_max_data_memory_usage()),
	  allocated_bytes_(0), pending_delete_bytes_(0){
}

CachedBlockMap::~CachedBlockMap() {
/** waits for blocks pending delete to be deleted*/
	wait_and_clean_pending();
	check_and_warn(pending_delete_bytes_==0, "pending_delete_bytes != 0 after wait_and_clean_pending in ~CachedBlockMap");
	check_and_warn(pending_delete_.size()==0, "pending_delete_ not empty in ~CachedBlockMap");
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




void CachedBlockMap::free_up_bytes_in_cache(std::size_t bytes_in_block) {
	while (max_allocatable_bytes_ - (allocated_bytes_ + pending_delete_bytes_) <= bytes_in_block) {
		clean_pending();
		BlockId block_id = policy_.get_next_block_for_removal();
		Block* tmp_block_ptr = cache_.get_and_remove_block(block_id);
		allocated_bytes_ -= tmp_block_ptr->size() * sizeof(double);
	}
}

void CachedBlockMap::clean_pending(){
	std::list<Block*>::iterator it;
	for (it = pending_delete_.begin(); it != pending_delete_.end(); ){
		if ((**it).test()){
			pending_delete_bytes_ -= (**it).size() * sizeof(double);
			pending_delete_.erase(it++);
		}
		else ++it;
	}
}

void CachedBlockMap::wait_and_clean_pending(){
	std::list<Block*>::iterator it;
	for (it = pending_delete_.begin(); it != pending_delete_.end(); ){
		    (**it).wait();
			pending_delete_bytes_ -= (**it).size() * sizeof(double);
			pending_delete_.erase(it++);
		}
}


//int CachedBlockMap::pending_list_size(){
//	return pending_delete_.size();
//}

void CachedBlockMap::insert_block(const BlockId& block_id, Block* block_ptr){
	/* Free up space to insert the new block in the block map
	 * Method will fail if it cannot find blocks to eject to free up space.
	 */
	std::size_t block_size = block_ptr->size() * sizeof(double);
	free_up_bytes_in_cache(block_size);

	block_map_.insert_block(block_id, block_ptr);
	allocated_bytes_ += block_size;
}

void CachedBlockMap::cached_delete_block(const BlockId& block_id){
	/* Remove block from block map and put in cache */
	Block* block_ptr = block_map_.get_and_remove_block(block_id);
	std::size_t bytes_in_block = block_ptr->size() * sizeof(double);
	free_up_bytes_in_cache(bytes_in_block);
	cache_.insert_block(block_id, block_ptr);
	policy_.touch(block_id);

//	block_map_.delete_block(block_id);

}

void CachedBlockMap::delete_block(const BlockId& block_id){
	Block* tmp_block_ptr = block_map_.get_and_remove_block(block_id);
	allocated_bytes_ -= tmp_block_ptr->size() * sizeof(double);
	if (!tmp_block_ptr->test()){
		pending_delete_bytes_ += tmp_block_ptr->size() * sizeof(double);
		pending_delete_.push_back(tmp_block_ptr);
	}
	else delete tmp_block_ptr;

}

void CachedBlockMap::insert_per_array_map(int array_id, IdBlockMap<Block>::PerArrayMap* map_ptr){
	allocated_bytes_ += block_map_.insert_per_array_map(array_id, map_ptr);
}

void CachedBlockMap::delete_per_array_map_and_blocks(int array_id){
	std::size_t tot_bytes_deleted = block_map_.delete_per_array_map_and_blocks(array_id);
	tot_bytes_deleted += cache_.delete_per_array_map_and_blocks(array_id);
	allocated_bytes_ -= tot_bytes_deleted;
}

IdBlockMap<Block>::PerArrayMap* CachedBlockMap::get_and_remove_per_array_map(int array_id){
	IdBlockMap<Block>::PerArrayMap* map_ptr = block_map_.get_and_remove_per_array_map(array_id);
	allocated_bytes_ -= IdBlockMap<Block>::total_bytes_per_array_map(map_ptr);
	return map_ptr;
}

void CachedBlockMap::set_max_allocatable_bytes(std::size_t size){
    static bool done_once = false;
    if (!done_once){
        done_once = true;
        max_allocatable_bytes_ = size;
    } else {
        sip::fail("Already set memory limit once !");
    }
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
