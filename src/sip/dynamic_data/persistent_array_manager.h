/*
 * persistent_array_manager.h
 *
 *  Created on: Jan 8, 2014
 *      Author: njindal
 */

#ifndef PERSISTENT_ARRAY_MANAGER_H_
#define PERSISTENT_ARRAY_MANAGER_H_

#include "blocks.h"
#include "block_manager.h"

#include <vector>
#include <set>

namespace sip {

/**
 * Maintains arrays and scalars that are persistent between SIAL programs
 */
template <typename BLOCK_TYPE>
class PersistentArrayManager {

public:
	typedef std::map<std::string, Block::BlockPtr> ContigLabelArrayMap;// Contiguous Array
	typedef typename std::map<std::string, typename IdBlockMap<BLOCK_TYPE>::PerArrayMap * > DistributedLabelArrayMap; // Distributed / Served Array
	typedef std::map<int, std::string> ArrIdLabelMap;	// Map of arrays marked for persistence


	/**
	 * creates empty PersistentArrayManager
	 */
	PersistentArrayManager<BLOCK_TYPE>() {}

	/**
	 * Destructor deletes all owned blocks and contiguous arrays.
	 */
	~PersistentArrayManager<BLOCK_TYPE>() {
		// Clear distributed arrays.
//		for(DistributedLabelArrayMap::iterator it = distributed_array_map_.begin(); it != distributed_array_map_.end(); ++it){
////			SIP_LOG(std::cout<<"Now deleting distributed array with label \""<<it->first<<"\" from Persistent Block Manager");
////			IdBlockMap<BLOCK_TYPE>* bid_map = it->second;
////			sip::check(bid_map != NULL, "Block map for dist. array " + it->first + " is NULL");
////			for (BlockManager::IdBlockMap::iterator it2 = bid_map->begin(); it2 != bid_map->end(); ++it2){
////				sip::check(it2->second != NULL, "Block of array " + it->first + " is NULL");
////				delete it2->second;
////			}
////			delete it->second;
//		}
//
//		// Clear contiguous arrays.
//		for (ContigLabelArrayMap::iterator it = contiguous_array_map_.begin(); it != contiguous_array_map_.end(); ++it){
//			SIP_LOG(std::cout<<"Now deleting contiguous array with label \""<<it->first<<"\" from Persistent Block Manager");
//			sip::check(it->second != NULL, "Block of array " + it->first + " is NULL");
//
//			delete it->second;
//		}

	}

	/**
	 * Marks an array persistent
	 * @param array_id
	 * @param label
	 */
	void mark_persistent_array(int array_id, std::string label) {
		persistent_array_map_.insert(std::pair<int, std::string>(array_id, label));
	}

	/**
	 * Returns true if an array has been marked persistent
	 * @param array_id
	 * @return
	 */
	bool is_array_persistent(int array_id){
		ArrIdLabelMap::iterator it = persistent_array_map_.find(array_id);
		if (it != persistent_array_map_.end())
			return true;
		return false;
	}

	/**
	 * Get previously saved contiguous array
	 * @param label
	 * @return
	 */
	Block::BlockPtr get_saved_contiguous_array(std::string label){
		ContigLabelArrayMap::iterator it = contiguous_array_map_.find(label);
		sip::check (it != contiguous_array_map_.end(), "Could not get previously saved contiguous array named " + label, current_line());
		Block::BlockPtr bptr = it->second;
		return bptr;
	}

	/**
	 * Get previously saved distributed array
	 * @param label
	 * @return
	 */
	IdBlockMap<BLOCK_TYPE>* get_saved_dist_array(std::string label){
		DistributedLabelArrayMap::iterator it = distributed_array_map_.find(label);
		sip::check(it != distributed_array_map_.end(), "Could not get previously saved distributed array named " + label, current_line());
		IdBlockMap<BLOCK_TYPE>* bid_map = it->second;
		return bid_map;
	}


	/**
	 * Save a contiguous array.
	 * If there already exists one with the same name, delete it.
	 * @param array_id
	 * @param bptr
	 */
	void save_contiguous_array(int array_id, Block::BlockPtr bptr){
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

	/**
	 * Save a distributed array.
	 * If there already exists one with the same name, delete it.
	 * @param array_id
	 * @param bid_map
	 */
	void save_dist_array(int array_id, typename IdBlockMap<BLOCK_TYPE>::PerArrayMap* bid_map){
		ArrIdLabelMap::iterator it = persistent_array_map_.find(array_id);
		sip::check(it != persistent_array_map_.end(), "Array wasn't marked as persistent", current_line());
		std::string &name = it->second;
		sip::check(bid_map != NULL, "Trying to save a NULL map for distributed array " + name + "!");

		typename DistributedLabelArrayMap::iterator it2 = distributed_array_map_.find(name);
		if (it2 != distributed_array_map_.end()){
			typename IdBlockMap<BLOCK_TYPE>::PerArrayMap* bid_map2 = it2->second;
			sip::check(bid_map2 != NULL, "entry for " + it2->first + " has a NULL map");
			if (bid_map != bid_map2){
				for (typename IdBlockMap<BLOCK_TYPE>::PerArrayMap::iterator it2 = bid_map2->begin(); it2 != bid_map2->end(); ++it2){
					delete it2->second;
					it2->second = NULL;
				}
				bid_map2->clear();
				distributed_array_map_.erase(name);
			}
		}

		distributed_array_map_.insert(std::pair<std::string, typename IdBlockMap<BLOCK_TYPE>::PerArrayMap*>(name, bid_map));

	}



	/**
	 * Returns label of an array marked previously for persistence
	 * @param array_id
	 * @return
	 */
	std::string get_label_of_marked_array(int array_id);

	/**
	 * Clears the persistent marked arrays.
	 */
	void clear_marked_arrays();

	template <class BLOCK_TYPE>
	friend std::ostream& operator<<(std::ostream&, const PersistentArrayManager<BLOCK_TYPE>&);


private:
	ContigLabelArrayMap contiguous_array_map_;
	DistributedLabelArrayMap distributed_array_map_;

	ArrIdLabelMap persistent_array_map_;

};

template <typename BLOCK_TYPE>
std::ostream& operator<<(std::ostream& os, const PersistentArrayManager<BLOCK_TYPE>& obj) {
	os << "persistent_array_map_:" << std::endl;
	{
		typename PersistentArrayManager<BLOCK_TYPE>::ArrIdLabelMap::const_iterator it;
		for (it = obj.persistent_array_map_.begin(); it != obj.persistent_array_map_.end(); ++it) {
			os << it->first << "\t" << it->second << std::endl;  //print the array id & associated string
		}
	}

	os << "contiguous_array_map_:" << std::endl;
	{
		typename PersistentArrayManager<BLOCK_TYPE>::ContigLabelArrayMap::const_iterator it;
		for (it = obj.contiguous_array_map_.begin(); it != obj.contiguous_array_map_.end(); ++it){
//			os << it->first << "\t" << *(it->second) << std::endl;
			os << it->first << std::endl;
		}
	}

	os << "distributed_array_map_:" << std::endl;
	{
		typename PersistentArrayManager<BLOCK_TYPE>::DistributedLabelArrayMap::const_iterator it;
		for (it = obj.distributed_array_map_.begin(); it != obj.distributed_array_map_.end(); ++it){
			os << it->first << std::endl;
			const IdBlockMap<BLOCK_TYPE> *bid_map = it -> second;
			IdBlockMap<BLOCK_TYPE>::const_iterator it2;
			for (it2 = bid_map->begin(); it2 != bid_map->end(); ++it2) {
				os << "\t" << it2 -> first << std::endl;
			}
			os << std::endl;
		}
	}
	return os;
}
} /* namespace sip */



#endif /* PERSISTENT_ARRAY_MANAGER_H_ */
