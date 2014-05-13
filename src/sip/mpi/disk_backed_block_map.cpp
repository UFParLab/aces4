/*
 * disk_backed_block_map.cpp
 *
 *  Created on: Apr 17, 2014
 *      Author: njindal
 */

#include <disk_backed_block_map.h>
#include <iostream>
#include <string>
#include <sstream>

namespace sip {

DiskBackedBlockMap::DiskBackedBlockMap(const SipTables& sip_tables,
		const SIPMPIAttr& sip_mpi_attr, const DataDistribution& data_distribution) :
		sip_tables_(sip_tables), sip_mpi_attr_(sip_mpi_attr), data_distribution_(data_distribution),
		block_map_(sip_tables.num_arrays()),
		disk_backed_arrays_io_(sip_tables, sip_mpi_attr, data_distribution){
}

ServerBlock* DiskBackedBlockMap::read_block_from_disk(const BlockId& block_id, size_t block_size){
	ServerBlock* block = new ServerBlock(block_size);
	disk_backed_arrays_io_.read_block_from_disk(block_id, block);
	block->set_dirty();
	block_map_.insert_block(block_id, block);
	return block;
}

ServerBlock* DiskBackedBlockMap::get_or_create_block(const BlockId& block_id,
		size_t block_size, bool initialize) {
	// TODO ===============================================
	// TODO Complete this method.
	// TODO ===============================================
	ServerBlock* block = block_map_.block(block_id);
	if (block == NULL){
		block = read_block_from_disk(block_id, block_size);
	}
	//ServerBlock* block = block_map_.get_or_create_block(block_id, block_size, initialize);
	//return block;
	return block;
}

ServerBlock* DiskBackedBlockMap::get_block_for_reading(const BlockId& block_id){
	ServerBlock* block = block_map_.block(block_id);
	size_t block_size = sip_tables_.block_size(block_id);
	if (block == NULL) {
		std::stringstream msg;
		msg << " W " << sip_mpi_attr_.global_rank();
		msg << " : getting uninitialized block ";
		msg << block_id.str();
		msg << ".  Creating zero block ";
		msg << std::endl;

		//do this instead of check_and_warn so goes to std::out intead of std::err
		SIP_LOG(std::cout << msg.str() << std::flush);
		block = read_block_from_disk(block_id, block_size);
	}
	return block;
}

IdBlockMap<ServerBlock>::PerArrayMap* DiskBackedBlockMap::get_and_remove_per_array_map(int array_id){
	return block_map_.get_and_remove_per_array_map(array_id);
}

void DiskBackedBlockMap::insert_per_array_map(int array_id, IdBlockMap<ServerBlock>::PerArrayMap* map_ptr){
	block_map_.insert_per_array_map(array_id, map_ptr);
}


void DiskBackedBlockMap::delete_per_array_map_and_blocks(int array_id){
	block_map_.delete_per_array_map_and_blocks(array_id);
}

void DiskBackedBlockMap::restore_persistent_array(int array_id, std::string & label){
	disk_backed_arrays_io_.restore_persistent_array(array_id, label);
}


void DiskBackedBlockMap::save_persistent_array(const int array_id,
		const std::string& array_label,
		IdBlockMap<ServerBlock>::PerArrayMap* array_blocks) {
	disk_backed_arrays_io_.save_persistent_array(array_id, array_label, array_blocks);
}


std::ostream& operator<<(std::ostream& os, const DiskBackedBlockMap& obj){
	os << "block map : " << std::endl;
	os << obj.block_map_;

	os << "disk_backed_arrays_io : " << std::endl;
	os << obj.disk_backed_arrays_io_;

	os << std::endl;

	return os;
}

} /* namespace sip */
