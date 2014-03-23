/*
 * id_block_map.cpp
 *
 *  Created on: Mar 21, 2014
 *      Author: basbas
 */



namespace sip {

template <class BLOCK_TYPE>
IdBlockMap<BLOCK_TYPE>::IdBlockMap(int size):block_map_(size)
{
	std::fill(block_map_ + 0, block_map_ + size, NULL);

}

template <class BLOCK_TYPE>
IdBlockMap<BLOCK_TYPE>::~IdBlockMap(){
	BlockMapVector::num_arrays = block_map_.size();
	for (int array_id=0; array_id< num_arrays; ++array_id) {
		delete_per_array_map(array_id);
	}
	block_map_.clear();
}

template <class BLOCK_TYPE>
BLOCK_TYPE* IdBlockMap<BLOCK_TYPE>::block(const BlockId& block_id) {
	int array_id = block_id.array_id();
	PerArrayMap* map_ptr = block_map_[array_id];
	if (map_ptr == NULL) return NULL;
	typename PerArrayMap::iterator it = map_ptr->find(block_id);
	return it != map_ptr->end() ? it->second : NULL;  //return NULL if not found
}

template <class BLOCK_TYPE>
void IdBlockMap<BLOCK_TYPE>::insert_block(const BlockId& block_id, BLOCK_TYPE* block_ptr) {
	int array_id = block_id.array_id();
	PerArrayMap* map_ptr = block_map_[array_id];
	if (map_ptr == NULL) {
		map_ptr = new PerArrayMap(); //Will be deleted along with all of its Blocks in the IdBlockMap destructor.
		                             //Alternatively, may be transferred to PersistentArrayManager, and replaced with
		                             //NULL in the block_map_ entry.
		block_map_[array_id] = map_ptr;
	}
	std::pair<BlockId, BLOCK_TYPE*> bid_pair (block_id, block_ptr);
	std::pair<typename PerArrayMap::iterator, bool> ret;
	ret = map_ptr->insert(bid_pair);
	check(ret.second, std::string("attempting to insert block that already exists"));
}


template <class BLOCK_TYPE>
BLOCK_TYPE* IdBlockMap<BLOCK_TYPE>::get_and_remove_block(const BlockId& block_id){
		int array_id = block_id.array_id();
		PerArrayMap* map_ptr = block_map_[array_id];
		check (map_ptr != NULL, "attempting get_and_remove_block when given array doesn't have a map");
		typename PerArrayMap::iterator it = map_ptr->find(block_id);
		BLOCK_TYPE* block_ptr = (it != map_ptr->end() ? it->second : NULL);
		map_ptr->erase(it);
		return block_ptr;
}

template <class BLOCK_TYPE>
void IdBlockMap<BLOCK_TYPE>::delete_block(const BlockId& id){
	BLOCK_TYPE* block_ptr = get_and_remove_block(id);
	delete(block_ptr);
}

template <class BLOCK_TYPE>
IdBlockMap<BLOCK_TYPE>::PerArrayMap* IdBlockMap<BLOCK_TYPE>::per_array_map(int array_id){
	PerArrayMap* map_ptr = block_map_[array_id];
	if (map_ptr == NULL) {
		map_ptr = new PerArrayMap(); //Will be deleted along with all of its Blocks in the IdBlockMap destructor.
		                             //Alternatively, may be transferred to PersistentArrayManager, and replaced with
		                             //NULL in the block_map_ entry.
		block_map_[array_id] = map_ptr;
	}
	return map_ptr;
}


template <class BLOCK_TYPE>
void IdBlockMap<BLOCK_TYPE>::delete_per_array_map(int array_id){
	PerArrayMap* map_ptr = block_map_[array_id];
	if (map_ptr != NULL) {
		for (PerArrayMap::iterator it = map_ptr->begin(); it != map_ptr->end(); ++it) {
			if (it->second != NULL) delete it->second;// Delete the block being pointed to.
		}
		delete block_map_[array_id];
		block_map_[array_id] = NULL;
	}
}

template <class BLOCK_TYPE>
IdBlockMap<BLOCK_TYPE>::PerArrayMap* IdBlockMap<BLOCK_TYPE>::get_and_remove_per_array_map(int array_id){
	PerArrayMap* map_ptr = per_array_map(array_id); //will create one if doesn't exist.
	block_map_[array_id] = NULL;
	return map_ptr;
}

template <class BLOCK_TYPE>
void IdBlockMap<BLOCK_TYPE>:: insert_per_array_map(int array_id, PerArrayMap* map_ptr){
	//get current map and warn if it contains blocks.  Delete any map that exists
    PerArrayMap* current_map = block_map_[array_id];
    check_and_warn(current_map == NULL || current_map->empty(),"replacing non-empty array in insert_per_array_map");
    delete_per_array_map(array_id);
	block_map_[array_id] =map_ptr;
}


template <class BLOCK_TYPE>
std::ostream& operator<<(std::ostream& os, const IdBlockMap<BLOCK_TYPE>& obj){
	IdBlockMap<BLOCK_TYPE>::size_type size = obj.size();
	for (unsigned i = 0; i < size; ++i){
		IdBlockMap<BLOCK_TYPE>::PerArrayMap* map = obj[i];
		if (map != NULL && !map->empty()){
			os << *map;
		}
	}
}

}//namespace sip








typename
