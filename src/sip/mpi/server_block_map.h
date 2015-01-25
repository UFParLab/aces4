/*
 * server_block_map.h
 *
 *  Created on: Jan 24, 2015
 *      Author: njindal
 */

#ifndef SERVER_BLOCK_MAP_H_
#define SERVER_BLOCK_MAP_H_

#include "server_timer.h"
#include "id_block_map.h"

namespace sip {
class SipTables;
class SIPMPIAttr;
class DataDistribution;
class ServerTimer;
class BlockId;
class ServerBlock;

class ServerBlockMap {
public:
	ServerBlockMap(const SipTables&, const SIPMPIAttr&, const DataDistribution&, ServerTimer&);
	~ServerBlockMap();

	// Get blocks for reading, writing, updating
	ServerBlock* get_block_for_reading(const BlockId& block_id);
	ServerBlock* get_block_for_writing(const BlockId& block_id);
	ServerBlock* get_block_for_updating(const BlockId& block_id);

	// Get entire arrays for save, restore operations
	IdBlockMap<ServerBlock>::PerArrayMap* get_and_remove_per_array_map(int array_id);
	void insert_per_array_map(int array_id, IdBlockMap<ServerBlock>::PerArrayMap* map_ptr);
	void delete_per_array_map_and_blocks(int array_id);
	IdBlockMap<ServerBlock>::PerArrayMap* per_array_map(int array_id);

	/*! Restores a persistent array from disk. Delegates to internal DiskBackedArraysIO object.*/
	void restore_persistent_array(int array_id, std::string & label);

	/*! Saves a persistent array to disk. Delegates to internal DiskBackedArraysIO object.*/
	void save_persistent_array(const int array_id,const std::string& array_label,
			IdBlockMap<ServerBlock>::PerArrayMap* array_blocks);

	void reset_consistency_status_for_all_blocks();

	/** Sets max_allocatable_bytes_ */
    void set_max_allocatable_bytes(std::size_t size);

	friend std::ostream& operator<<(std::ostream& os, const ServerBlockMap& obj);


private:

	ServerBlock* allocate_block(size_t block_size, bool initialze=true);


    const SipTables &sip_tables_;
	const SIPMPIAttr & sip_mpi_attr_;
	const DataDistribution &data_distribution_;

	ServerTimer &server_timer_;

	IdBlockMap<ServerBlock> block_map_;			/*! interface to memory block map */

	 /** Maximum number of bytes before spilling over to disk */
	std::size_t max_allocatable_bytes_;

};

} /* namespace sip */

#endif /* SERVER_BLOCK_MAP_H_ */
