/*
 * persistent_array_manager.cpp
 *
 *  Created on: Jan 8, 2014
 *      Author: njindal
 */

#include "persistent_array_manager.h"
#include "blocks.h"

namespace sip {

PersistentArrayManager::PersistentArrayManager() {}

PersistentArrayManager::~PersistentArrayManager() {
	// Clear distributed arrays.
	for(DistributedLabelArrayMap::iterator it = distributed_array_map_.begin(); it != distributed_array_map_.end(); ++it){
		SIP_LOG(std::cout<<"Now deleting distributed array with label \""<<it->first<<"\" from Persistent Block Manager");
		BlockManager::IdBlockMapPtr bid_map = it->second;
		sip::check(bid_map != NULL, "Block map for dist. array " + it->first + " is NULL");
		for (BlockManager::IdBlockMap::iterator it2 = bid_map->begin(); it2 != bid_map->end(); ++it2){
			sip::check(it2->second != NULL, "Block of array " + it->first + " is NULL");
			delete it2->second;
		}
		delete it->second;
	}

	// Clear contiguous arrays.
	for (ContigLabelArrayMap::iterator it = contiguous_array_map_.begin(); it != contiguous_array_map_.end(); ++it){
		SIP_LOG(std::cout<<"Now deleting contiguous array with label \""<<it->first<<"\" from Persistent Block Manager");
		sip::check(it->second != NULL, "Block of array " + it->first + " is NULL");

		delete it->second;
	}

}

void PersistentArrayManager::mark_persistent_array(int array_id, std::string label) {
	persistent_array_map_.insert(std::pair<int, std::string>(array_id, label));
}

bool PersistentArrayManager::is_array_persistent(int array_id){
	ArrIdLabelMap::iterator it = persistent_array_map_.find(array_id);
	if (it != persistent_array_map_.end())
		return true;
	return false;
}

Block::BlockPtr PersistentArrayManager::get_saved_contiguous_array(std::string label){
	ContigLabelArrayMap::iterator it = contiguous_array_map_.find(label);
	sip::check (it != contiguous_array_map_.end(), "Could not get previously saved contiguous array named " + label, current_line());
	Block::BlockPtr bptr = it->second;
	return bptr;
}

BlockManager::IdBlockMapPtr PersistentArrayManager::get_saved_dist_array(std::string label){
	DistributedLabelArrayMap::iterator it = distributed_array_map_.find(label);
	sip::check(it != distributed_array_map_.end(), "Could not get previously saved distributed array named " + label, current_line());
	BlockManager::IdBlockMapPtr bid_map = it->second;
	return bid_map;
}

/**
 * Save a contiguous array.
 * If there already exists one with the same name, delete it.
 * @param array_id
 * @param bptr
 */
void PersistentArrayManager::save_contiguous_array(int array_id, Block::BlockPtr bptr){
	ArrIdLabelMap::iterator it = persistent_array_map_.find(array_id);
	sip::check(it != persistent_array_map_.end(), "Array wasn't marked as persistent", current_line());
	std::string &name = it->second;
	const std::pair<ContigLabelArrayMap::iterator, bool> &ret = contiguous_array_map_.insert(std::pair<std::string, Block::BlockPtr>(name, bptr));
	if (!ret.second){ // Already Exists
		delete ret.first->second;
		ret.first->second = NULL;
		contiguous_array_map_.erase(name);
		contiguous_array_map_.insert(std::pair<std::string, Block::BlockPtr>(name, bptr));
	}

}

std::string PersistentArrayManager::get_label_of_marked_array(int array_id){
	return persistent_array_map_.at(array_id);
}


void PersistentArrayManager::clear_marked_arrays(){
	persistent_array_map_.clear();
}

/**
 * Save a distributed array.
 * If there already exists one with the same name, delete it.
 * @param array_id
 * @param bid_map
 */
void PersistentArrayManager::save_dist_array(int array_id, BlockManager::IdBlockMapPtr bid_map){
	ArrIdLabelMap::iterator it = persistent_array_map_.find(array_id);
	sip::check(it != persistent_array_map_.end(), "Array wasn't marked as persistent", current_line());
	std::string &name = it->second;
	sip::check(bid_map != NULL, "Trying to save a NULL map for distributed array " + name + "!");

	DistributedLabelArrayMap::iterator it2 = distributed_array_map_.find(name);
	if (it2 != distributed_array_map_.end()){
		BlockManager::IdBlockMapPtr bid_map2 = it2->second;
		sip::check(bid_map2 != NULL, "entry for " + it2->first + " has a NULL map");
		if (bid_map != bid_map2){
			for (BlockManager::IdBlockMap::iterator it2 = bid_map2->begin(); it2 != bid_map2->end(); ++it2){
				delete it2->second;
				it2->second = NULL;
			}
			bid_map2->clear();
			distributed_array_map_.erase(name);
		}
	}

	distributed_array_map_.insert(std::pair<std::string, BlockManager::IdBlockMapPtr>(name, bid_map));

}

std::ostream& operator<<(std::ostream& os, const PersistentArrayManager& obj) {
	os << "persistent_array_map_:" << std::endl;
	{
		sip::PersistentArrayManager::ArrIdLabelMap::const_iterator it;
		for (it = obj.persistent_array_map_.begin(); it != obj.persistent_array_map_.end(); ++it) {
			os << it->first << "\t" << it->second << std::endl;  //print the array id & associated string
		}
	}

	os << "contiguous_array_map_:" << std::endl;
	{
		sip::PersistentArrayManager::ContigLabelArrayMap::const_iterator it;
		for (it = obj.contiguous_array_map_.begin(); it != obj.contiguous_array_map_.end(); ++it){
//			os << it->first << "\t" << *(it->second) << std::endl;
			os << it->first << std::endl;
		}
	}

	os << "distributed_array_map_:" << std::endl;
	{
		sip::PersistentArrayManager::DistributedLabelArrayMap::const_iterator it;
		for (it = obj.distributed_array_map_.begin(); it != obj.distributed_array_map_.end(); ++it){
			os << it->first << std::endl;
			const BlockManager::IdBlockMap *bid_map = it -> second;
			BlockManager::IdBlockMap::const_iterator it2;
			for (it2 = bid_map->begin(); it2 != bid_map->end(); ++it2) {
				os << "\t" << it2 -> first << std::endl;
			}
			os << std::endl;
		}
	}
	return os;
}

} /* namespace sip */
