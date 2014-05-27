/*
 * lru_array_policy.cpp
 *
 *  Created on: May 15, 2014
 *      Author: njindal
 */

#include <lru_array_policy.h>

#include "block_id.h"

namespace sip {

LRUArrayPolicy::LRUArrayPolicy(IdBlockMap<ServerBlock>& block_map) : block_map_(block_map) {
    
}

void LRUArrayPolicy::touch(const BlockId& bid){
    // Search for array_id; if present, place at front of queue

    int array_id = bid.array_id();
    std::list<int>::iterator it;
    for (it = lru_list_.begin(); it != lru_list_.end(); ++it){
        if (array_id == *it)
            break;
    }

    if (it != lru_list_.end()) 
        lru_list_.erase(it);

    lru_list_.push_front(array_id);

}

void LRUArrayPolicy::remove_all_blocks_for_array(int array_id){
    lru_list_.remove(array_id);
}

BlockId LRUArrayPolicy::get_next_block_for_removal(){
    /** Return an arbitrary block from the least recently used array
     *
     */
    sip::check(!lru_list_.empty(), "No blocks have been touched, yet block requested for flushing");
    while (!lru_list_.empty()){
    	int to_remove_array = lru_list_.back();
    	IdBlockMap<ServerBlock>::PerArrayMap* array_map = block_map_.per_array_map(to_remove_array);
    	IdBlockMap<ServerBlock>::PerArrayMap::iterator it = array_map->begin();
    	if (it == array_map->end())
    		lru_list_.pop_back();
    	else
    		return it->first;
    }
    sip::fail("No blocks to remove !");
}

} /* namespace sip */
