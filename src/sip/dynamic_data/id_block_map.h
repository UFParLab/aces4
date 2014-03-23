/*
 * id_block_map.h
 *
 *  Created on: Mar 21, 2014
 *      Author: basbas
 */

#ifndef ID_BLOCK_MAP_H_
#define ID_BLOCK_MAP_H_

#include <map>
#include <vector>
#include <stack>
#include "blocks.h"

namespace sip {


template <typename BLOCK_TYPE>
class IdBlockMap {
public:

	typedef std::map<BlockId, BLOCK_TYPE*> PerArrayMap;
	typedef std::vector<PerArrayMap*> BlockMapVector;
	typedef typename std::vector<std::map<BlockId, BLOCK_TYPE* >* >::size_type size_type;

	/**
	 * Constructor takes size of array table in order to create a vector
	 * of the correct size.  Initially, there are no blocks and the maps
	 * for the individual arrays are NULL.  Map instances are created when
	 * needed.
	 *
	 * @param size
	 */
	explicit IdBlockMap(int array_table_size):block_map_(array_table_size)
	{
//		std::fill(block_map_.begin(), block_map_.end(), NULL);

	}


	/**
	 * destructor, deletes all blocks, and all maps
	 */
	~IdBlockMap(){
		int num_arrays = block_map_.size();
		for (int array_id=0; array_id< num_arrays; ++array_id) {
			delete_per_array_map(array_id);
		}
		block_map_.clear();
	}

	/** Obtains requested block
	 *
	 * @param id  BlockId of of desired block
	 * @return pointer to the requested block, or NULL if it is not in the map.
	 */
	BLOCK_TYPE* block(const BlockId& block_id) {
		int array_id = block_id.array_id();
		PerArrayMap* map_ptr = block_map_[array_id];
		if (map_ptr == NULL) return NULL;
		typename PerArrayMap::iterator it = map_ptr->find(block_id);
		return it != map_ptr->end() ? it->second : NULL;  //return NULL if not found
	}

	/** Obtains requested block, creating if it doesn't exist.
	 *
	 * @param [in] block_id
	 * @param [out] size
	 * @returns pointer to existing or new block
	 */
	BLOCK_TYPE* get_or_create_block(const BlockId& block_id, size_type size){
		BLOCK_TYPE* block = block(block_id);
		if (block == NULL) {
			block = new BLOCK_TYPE(size);
			block_map_.insert_block(block_id,block);
		}
	}

	/** inserts given block in the IdBlockMap.
	 *
	 * It is an error to insert a block that already exists.
	 *
	 * @param id BlockId of block to insert
	 * @param block_ptr pointer to Block to insert
	 */
	void insert_block(const BlockId& block_id, BLOCK_TYPE* block_ptr) {
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

	/** Obtains requested block and remove it from the map.
	 * Requires the requested block is in the map
	 *
	 * @param id block id
	 * @return pointer to requested block
	 */
	BLOCK_TYPE* get_and_remove_block(const BlockId& block_id){
		int array_id = block_id.array_id();
		PerArrayMap* map_ptr = block_map_[array_id];
		check (map_ptr != NULL, "attempting get_and_remove_block when given array doesn't have a map");
		typename PerArrayMap::iterator it = map_ptr->find(block_id);
		BLOCK_TYPE* block_ptr = (it != map_ptr->end() ? it->second : NULL);
		map_ptr->erase(it);
		return block_ptr;
    }

	/** removes given block from the map and deletes it
	 *
	 * @param id BlockId of block to delete
	 */
	void delete_block(const BlockId& id){
		BLOCK_TYPE* block_ptr = get_and_remove_block(id);
		delete(block_ptr);
	}

	/**
	 * per_array_map returns a pointer to the map containing blocks of the indicated array.
	 * If the map doesn't exist, one is created and added to the map.
	 * This routine never returns NULL
	 *
	 * @param array_id
	 * @return pointer to PerArrayMap holding the blocks of the indicated array.
	 */
	PerArrayMap* per_array_map(int array_id){
		PerArrayMap* map_ptr = block_map_[array_id];
		if (map_ptr == NULL) {
			map_ptr = new PerArrayMap(); //Will be deleted along with all of its Blocks in the IdBlockMap destructor.
			                             //Alternatively, may be transferred to PersistentArrayManager, and replaced with
			                             //NULL in the block_map_ entry.
			block_map_[array_id] = map_ptr;
		}
		return map_ptr;
	}

	/**
	 *deletes the map containing blocks of the indicated array
	 *alongs with the blocks in the map
	 *
	 * @param array_id
	 */
	void delete_per_array_map(int array_id){
		PerArrayMap* map_ptr = block_map_[array_id];
		if (map_ptr != NULL) {
			for (typename PerArrayMap::iterator it = map_ptr->begin(); it != map_ptr->end(); ++it) {
				if (it->second != NULL) delete it->second;// Delete the block being pointed to.
			}
			delete block_map_[array_id];
			block_map_[array_id] = NULL;
		}
	}

	/**
	 * returns a pointer to the PerArrayMap of the given array,
	 * and removes it from the map.  Contents of the map are preserved.
	 * If no PerArrayMap exists, one is created.
	 * This routine never returns NULL.
	 *
	 * @param array_id
	 * @return
	 */
	PerArrayMap* get_and_remove_per_array_map(int array_id){
		PerArrayMap* map_ptr = per_array_map(array_id); //will create one if doesn't exist.
		block_map_[array_id] = NULL;
		return map_ptr;
	}

	/**
	 * inserts the given PerArrayMap, updating the array_id in each block's ID to the given array_id.
	 *
	 * This is used to restore persistent arrays.  The array_id is not consistent across sial programs
	 * so must be updated here.
	 *
	 * @param array_id
	 * @param PerArrayMap*
	 */
	void insert_per_array_map(int array_id, PerArrayMap* map_ptr){
		//get current map and warn if it contains blocks.  Delete any map that exists
	    PerArrayMap* current_map = block_map_[array_id];
	    check_and_warn(current_map == NULL || current_map->empty(),"replacing non-empty array in insert_per_array_map");
	    delete_per_array_map(array_id);
		block_map_[array_id] =map_ptr;
	}

	int size() const {return block_map_.size();}

	PerArrayMap* operator[](unsigned i){ return block_map_[i]; }
    const PerArrayMap* operator[](unsigned i) const { return block_map_[i]; }

	template <class BLOCK_TYPE>
	friend std::ostream& operator<<(std::ostream&, const IdBlockMap<BLOCK_TYPE>&);

	template <class BLOCK_TYPE>
	friend std::ostream& operator<<(std::ostream&, const typename IdBlockMap<BLOCK_TYPE>::PerArrayMap &);

private:

	BlockMapVector block_map_;

	DISALLOW_COPY_AND_ASSIGN(IdBlockMap<BLOCK_TYPE>);
};

template <class BLOCK_TYPE>
std::ostream& operator<<(std::ostream& os, const IdBlockMap<BLOCK_TYPE>& obj){
//	IdBlockMap<BLOCK_TYPE>::BlockMapVector::size_type size = obj.size();
	typename IdBlockMap<BLOCK_TYPE>::size_type size = obj.size();
	for (unsigned i = 0; i < size; ++i){
		const typename IdBlockMap<BLOCK_TYPE>::PerArrayMap* map_ptr = obj[i];
		if (map_ptr != NULL && !map_ptr->empty()){
			os << map_ptr;
		}
	}
	return os;
}

/**
 * returns an os stream with the block ids of blocks stored in the map, obj
 * @param os
 * @param obj PerArrayMap
 * @return
 */
template <class BLOCK_TYPE>
std::ostream& operator<<(std::ostream& os, const typename IdBlockMap<BLOCK_TYPE>::PerArrayMap* & obj){
	typename IdBlockMap<BLOCK_TYPE>::PerArrayMap::iterator it, it_end;
	for (it = obj->begin(); it != obj->end(); ++it){
		os << (it = obj->begin()?"":", ") << it->first;
	}
	os << std::endl;
	return os;
}

}//namespace sip



#endif /* ID_BLOCK_MAP_H_ */
