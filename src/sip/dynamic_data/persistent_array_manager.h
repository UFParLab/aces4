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
 * Maintains set of blocks that are persistent between SIAL programs
 */
class PersistentArrayManager {

public:
	typedef std::map<std::string, Block::BlockPtr> ContigLabelArrayMap;// Contiguous Array
	typedef std::map<std::string, BlockManager::IdBlockMapPtr> DistributedLabelArrayMap; // Distributed / Served Array

	typedef std::map<int, std::string> ArrIdLabelMap;	// Map of arrays marked for persistence


	PersistentArrayManager();
	~PersistentArrayManager();

	/**
	 * Marks an array persistent
	 * @param array_id
	 * @param label
	 */
	void mark_persistent_array(int array_id, std::string label);

	/**
	 * Returns true if an array has been marked persistent
	 * @param array_id
	 * @return
	 */
	bool is_array_persistent(int array_id);

	/**
	 * Get previously saved contiguous array
	 * @param label
	 * @return
	 */
	Block::BlockPtr get_saved_contiguous_array(std::string label);

	/**
	 * Get previously saved distributed array
	 * @param label
	 * @return
	 */
	BlockManager::IdBlockMapPtr get_saved_dist_array(std::string label);


	/**
	 * Save a contiguous array for the next SIAL program
	 * @param
	 * @param
	 */
	void save_contiguous_array(int array_id, Block::BlockPtr bptr);

	/**
	 * Save a distributed array for the next SIAL program
	 * @param
	 * @param
	 */
	void save_dist_array(int array_id, BlockManager::IdBlockMapPtr bid_map);


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

	friend std::ostream& operator<<(std::ostream&, const PersistentArrayManager&);


private:
	ContigLabelArrayMap contiguous_array_map_;
	DistributedLabelArrayMap distributed_array_map_;

	ArrIdLabelMap persistent_array_map_;

};

} /* namespace sip */

#endif /* PERSISTENT_ARRAY_MANAGER_H_ */
