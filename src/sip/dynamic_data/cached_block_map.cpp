/*
 * cached_block_map.cpp
 *
 *  Created on: Jul 5, 2014
 *      Author: njindal
 */

#include <cached_block_map.h>

namespace sip {

CachedBlockMap::CachedBlockMap(int num_arrays)
	: block_map_(num_arrays), cache_(num_arrays), policy_(cache_) {
}

CachedBlockMap::~CachedBlockMap() {
	/** Just calls the destructors of the backing structures. */
}

Block* CachedBlockMap::block(const BlockId& block_id){
	// TODO check cache, if present, remove and put into cache.
	return block_map_.block(block_id);
}

void CachedBlockMap::insert_block(const BlockId& block_id, Block* block_ptr){
	// TODO Check if enough space in cache
	block_map_.insert_block(block_id, block_ptr);
}

void CachedBlockMap::delete_block(const BlockId& block_id){
	// TODO Put in cache
	block_map_.delete_block(block_id);
}

void CachedBlockMap::insert_per_array_map(int array_id, IdBlockMap<Block>::PerArrayMap* map_ptr){
	block_map_.insert_per_array_map(array_id, map_ptr);
}

void CachedBlockMap::delete_per_array_map_and_blocks(int array_id){
	block_map_.delete_per_array_map_and_blocks(array_id);
}

IdBlockMap<Block>::PerArrayMap* CachedBlockMap::get_and_remove_per_array_map(int array_id){
	return block_map_.get_and_remove_per_array_map(array_id);
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
