/*
 * contiguous_local_id_block_map.cpp
 *
 *  Created on: Aug 20, 2014
 *      Author: basbas
 */

#include "contiguous_local_id_block_map.h"

namespace sip {


ContiguousLocalIdBlockMap::ContiguousLocalIdBlockMap(int array_table_size):block_map_(array_table_size, NULL){
}

ContiguousLocalIdBlockMap::~ContiguousLocalIdBlockMap(){
	int num_arrays = block_map_.size();
	for (int array_id=0; array_id< num_arrays; ++array_id) {
		delete_per_array_map_and_blocks(array_id);
	}
	block_map_.clear();
}

Block* ContiguousLocalIdBlockMap::enclosing_block(const ContiguousLocalBlockId& block_id, ContiguousLocalBlockId& enclosing_block_id){
	int array_id = block_id.array_id();
	ContiguousLocalPerArrayMap* map_ptr = block_map_.at(array_id);
	if (map_ptr == NULL) return NULL;
//	ContiguousLocalPerArrayMap::iterator it = map_ptr->find(block_id);
//	if ( it != map_ptr->end() ) {
//		contained_block_id = block_id;
//		return it->second;
//	}
	ContiguousLocalPerArrayMap::iterator it = map_ptr->lower_bound(block_id);
	if (it->first == block_id){ //found an exact match
		enclosing_block_id = block_id;
		return it->second;
	}
	if (it->first.encloses(block_id)){
		enclosing_block_id = it->first;
		return it->second;
	}
	//not found, return NULL;
	return NULL;
}


bool ContiguousLocalIdBlockMap::disjoint(const ContiguousLocalBlockId& block_id){
	int array_id = block_id.array_id();
	ContiguousLocalPerArrayMap* map_ptr = block_map_.at(array_id);
	if (map_ptr == NULL) return true;
	ContiguousLocalPerArrayMap::iterator it = map_ptr->lower_bound(block_id); //this points to the largest element < block_id
	++it;
	return it == map_ptr->end() || block_id < it->first; 	//it is invariant that all elements in the map are totally ordered,
	                                                         //so we only need to look at one.
}


//Block* ContiguousLocalIdBlockMap::get_or_create_block(const ContiguousLocalBlockId& block_id, size_type size, bool initialize){
//	sial_check(disjoint(block_id),std::string("trying to create  local contiguous region overlaps with existing region",current_line()));
//	ContiguousLocalBlockId enclosing_id;
//	Block* b = block(block_id, enclosing_id);
//	if (b == NULL) {
//
//		b = new Block(size, initialize);
//		insert_block(block_id,b);
//	}
//	return b;
//}

/** inserts given block in the IdBlockMap.
 *
 * It is a fatal error to insert a block that already exists or overlaps an existing block.
 *
 * @param id BlockId of block to insert
 * @param block_ptr pointer to Block to insert
 */
void ContiguousLocalIdBlockMap::insert_block(const ContiguousLocalBlockId& block_id, Block* block_ptr) {
	sial_check(disjoint(block_id), std::string("contiguous local region overlaps with existing region"),current_line());
	int array_id = block_id.array_id();
	ContiguousLocalPerArrayMap* map_ptr = block_map_.at(array_id);
	if (map_ptr == NULL) {
		map_ptr = new ContiguousLocalPerArrayMap(); //Will be deleted along with all of its Blocks in the IdBlockMap destructor.
		                             //Alternatively, ownership may be transferred to PersistentArrayManager, and replaced with
		                             //NULL in the block_map_ entry.
		block_map_.at(array_id) = map_ptr;
	}
	std::pair<ContiguousLocalBlockId, Block*> bid_pair (block_id, block_ptr);
	std::pair<ContiguousLocalPerArrayMap::iterator, bool> ret;
	ret = map_ptr->insert(bid_pair);
	check(ret.second, std::string("problem inserting ContiguousLocal block into the map"));
}

void ContiguousLocalIdBlockMap::delete_block(const ContiguousLocalBlockId& id){
	Block* block_ptr = get_and_remove_block(id);
	delete(block_ptr);
}

Block* ContiguousLocalIdBlockMap::get_and_remove_block(const ContiguousLocalBlockId& block_id){
	int array_id = block_id.array_id();
	ContiguousLocalPerArrayMap* map_ptr = block_map_.at(array_id);
	check (map_ptr != NULL, "attempting get_and_remove_block when given array doesn't have a map");
	ContiguousLocalPerArrayMap::iterator it = map_ptr->find(block_id);
	sial_check(it != map_ptr->end(),
			"attempting to remove a non-existent block ",
			current_line() );
	Block* block_ptr = it->second;
	map_ptr->erase(it);
	return block_ptr;
}

ContiguousLocalIdBlockMap::ContiguousLocalPerArrayMap* ContiguousLocalIdBlockMap::per_array_map(int array_id){
	ContiguousLocalPerArrayMap* map_ptr = block_map_.at(array_id);
	if (map_ptr == NULL) {
		map_ptr = new ContiguousLocalPerArrayMap(); //Will be deleted along with all of its Blocks in the ContiguousIdBlockMap destructor.
		block_map_.at(array_id) = map_ptr;
	}
	return map_ptr;
}

void ContiguousLocalIdBlockMap::delete_blocks_from_per_array_map(ContiguousLocalPerArrayMap* map_ptr) {
	for (ContiguousLocalPerArrayMap::iterator it = map_ptr->begin(); it != map_ptr->end(); ++it) {
		if (it->second != NULL) {
			delete it->second; // Delete the block being pointed to.
			it->second = NULL;
		}
	}
}

void ContiguousLocalIdBlockMap::delete_per_array_map_and_blocks(int array_id){
	ContiguousLocalPerArrayMap* map_ptr = block_map_.at(array_id);
	if (map_ptr != NULL) {
		delete_blocks_from_per_array_map(map_ptr);
		delete block_map_.at(array_id);
		block_map_.at(array_id) = NULL;
	}
}



ContiguousLocalIdBlockMap::size_type ContiguousLocalIdBlockMap::size() const {return block_map_.size();}

ContiguousLocalIdBlockMap::ContiguousLocalPerArrayMap* ContiguousLocalIdBlockMap::operator[](unsigned i){ return block_map_.at(i); }
const ContiguousLocalIdBlockMap::ContiguousLocalPerArrayMap* ContiguousLocalIdBlockMap::operator[](unsigned i) const { return block_map_.at(i); }

std::ostream& operator<<(std::ostream& os, const ContiguousLocalIdBlockMap& obj){
	ContiguousLocalIdBlockMap::size_type size = obj.size();
	for (unsigned i = 0; i < size; ++i){
		const ContiguousLocalIdBlockMap::ContiguousLocalPerArrayMap* map_ptr = obj.block_map_.at(i);
		if (map_ptr != NULL && !map_ptr->empty()){
			ContiguousLocalIdBlockMap::ContiguousLocalPerArrayMap::const_iterator it;
			for (it = map_ptr->begin(); it != map_ptr->end(); ++it){
				ContiguousLocalBlockId id = it->first;
				Block* block = it->second;
				if (block == NULL) {
					os << " NULL block ";
				} else {
					double * data = block->get_data();
					if (data == NULL) {
						os << " NULL data ";
					} else {
						int size = block->size();
						os << id << " size=" << size << " ";
						os << '[' << data[0];
						if (size > 1) {
							os << "..." << data[size - 1];
						}
						os << ']';
					}
				}
				os << std::endl;
			}
			os << std::endl;
		}
	}
	return os;
}




} /* namespace sip */
