/*
 * disk_backed_block_map.h
 *
 *  Created on: Apr 17, 2014
 *      Author: njindal
 */

#ifndef DISK_BACKED_BLOCK_MAP_H_
#define DISK_BACKED_BLOCK_MAP_H_

#include "disk_backed_block_map.h"
#include "id_block_map.h"
#include "disk_backed_arrays_io.h"

namespace sip {
class BlockId;
class ServerPersistentArrayManager;

/**
 * Wrapper over block map for servers.
 * Support spilling over to disk and other operations.
 */
class DiskBackedBlockMap {
public:
	DiskBackedBlockMap(const SipTables&, const SIPMPIAttr&, const DataDistribution&);

	ServerBlock* get_block_for_reading(const BlockId& block_id);

	IdBlockMap<ServerBlock>::PerArrayMap* get_and_remove_per_array_map(int array_id);

	void insert_per_array_map(int array_id, IdBlockMap<ServerBlock>::PerArrayMap* map_ptr);

	ServerBlock* get_or_create_block(const BlockId& block_id, size_t block_size, bool initialize);

	void delete_per_array_map_and_blocks(int array_id);

	/**
	 * Restores a persistent array from disk.
	 * Delegates to internal DiskBackedArraysIO object.
	 * @param array_id
	 * @param label
	 */
	void restore_persistent_array(int array_id, std::string & label);

	/**
	 * Saves a persistent array to disk.
	 * Delegates to internal DiskBackedArraysIO object.
	 * @param array_id
	 * @param array_label
	 * @param array_blocks
	 */
	void save_persistent_array(const int array_id,
			const std::string& array_label,
			IdBlockMap<ServerBlock>::PerArrayMap* array_blocks);

	friend std::ostream& operator<<(std::ostream& os, const DiskBackedBlockMap& obj);

private:

	ServerBlock* read_block_from_disk(const BlockId& block_id, size_t block_size);

    const SipTables &sip_tables_;
	const SIPMPIAttr & sip_mpi_attr_;
	const DataDistribution &data_distribution_;

	/**
	 * Interface to read and write blocks of distributed arrays to disk.
	 */
	DiskBackedArraysIO disk_backed_arrays_io_;

	/**
	 * Data structure to store blocks of distributed/served arrays
	 */
	IdBlockMap<ServerBlock> block_map_;

};

} /* namespace sip */

#endif /* DISK_BACKED_BLOCK_MAP_H_ */
