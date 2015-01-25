/*
 * server_block_map.cpp
 *
 *  Created on: Jan 24, 2015
 *      Author: njindal
 */

#include <server_block_map.h>

#include <sstream>
#include "server_block.h"
#include "global_state.h"
#include "sip_interface.h"
#include "id_block_map.h"
#include "block_id.h"
#include "server_block.h"
#include "sip_mpi_attr.h"
#include "data_distribution.h"

namespace sip {

ServerBlockMap::ServerBlockMap(const SipTables& sip_tables,
		const SIPMPIAttr& sip_mpi_attr, const DataDistribution& data_distribution,
		ServerTimer& server_timer) :
		sip_tables_(sip_tables), sip_mpi_attr_(sip_mpi_attr), data_distribution_(data_distribution),
		block_map_(sip_tables.num_arrays()),
        max_allocatable_bytes_(sip::GlobalState::get_max_data_memory_usage()),
        server_timer_(server_timer){
}

ServerBlockMap::~ServerBlockMap() {}

ServerBlock* ServerBlockMap::get_block_for_reading(const BlockId& block_id){
	ServerBlock* block = block_map_.block(block_id);
	size_t block_size = sip_tables_.block_size(block_id);
	if (block == NULL) {
		// Error !
		std::stringstream errmsg;
		errmsg << " S " << sip_mpi_attr_.global_rank();
		errmsg << " : Asking for block " << block_id << ". It has not been put/prepared before !"<< std::endl;
		std::cout << errmsg.str() << std::flush;

		sip::fail(errmsg.str());

	}
	return block;

}

ServerBlock* ServerBlockMap::get_block_for_writing(const BlockId& block_id){
	ServerBlock* block = block_map_.block(block_id);
	size_t block_size = sip_tables_.block_size(block_id);
	if (block == NULL) {
		SIP_LOG(std::stringstream msg;
			msg << "S " << sip_mpi_attr_.global_rank();
			msg << " : getting uninitialized block " << block_id << ".  Creating zero block for writing"<< std::endl;
			std::cout << msg.str() << std::flush);
		block = allocate_block(block_size);
	    block_map_.insert_block(block_id, block);
	}
	return block;
}

ServerBlock* ServerBlockMap::get_block_for_updating(const BlockId& block_id){
	ServerBlock* block = block_map_.block(block_id);
	size_t block_size = sip_tables_.block_size(block_id);
	if (block == NULL) {
		SIP_LOG(std::stringstream msg;
			msg << "S " << sip_mpi_attr_.global_rank();
			msg << " : getting uninitialized block " << block_id << ".  Creating zero block for updating "<< std::endl;
			std::cout << msg.str() << std::flush);
		block = allocate_block(block_size);
		block_map_.insert_block(block_id, block);
	}
	return block;
}

ServerBlock* ServerBlockMap::allocate_block(size_t block_size, bool initialze){
	std::size_t remaining_mem = max_allocatable_bytes_ - ServerBlock::allocated_bytes();
	if (remaining_mem < block_size * sizeof(double)){
		sip::fail("Out of memory on server");
	}
	ServerBlock* block = new ServerBlock(block_size, initialze);
	return block;

}

void ServerBlockMap::reset_consistency_status_for_all_blocks(){
	int num_arrays = block_map_.size();
	for (int i=0; i<num_arrays; i++){
		typedef IdBlockMap<ServerBlock>::PerArrayMap PerArrayMap;
		typedef IdBlockMap<ServerBlock>::PerArrayMap::iterator PerArrayMapIterator;
		PerArrayMap *map = block_map_.per_array_map(i);
		PerArrayMapIterator it = map->begin();
		for (; it != map->end(); ++it){
			it->second->reset_consistency_status();
		}
	}
}

IdBlockMap<ServerBlock>::PerArrayMap* ServerBlockMap::get_and_remove_per_array_map(int array_id){
	return block_map_.get_and_remove_per_array_map(array_id);
}
void ServerBlockMap::insert_per_array_map(int array_id, IdBlockMap<ServerBlock>::PerArrayMap* map_ptr){
	block_map_.insert_per_array_map(array_id, map_ptr);
}
void ServerBlockMap::delete_per_array_map_and_blocks(int array_id){
	block_map_.delete_per_array_map_and_blocks(array_id);	// remove from memory
	SIP_LOG(std::cout << "S " << sip_mpi_attr_.global_rank() << " : Removed blocks from memory" << std::endl;);
}
IdBlockMap<ServerBlock>::PerArrayMap* ServerBlockMap::per_array_map(int array_id){
	return block_map_.per_array_map(array_id);
}

void ServerBlockMap::restore_persistent_array(int array_id, std::string & label){
	// Do Nothing
}

void ServerBlockMap::save_persistent_array(const int array_id,const std::string& array_label,
		IdBlockMap<ServerBlock>::PerArrayMap* array_blocks){
	 // Do Nothing
}

std::ostream& operator<<(std::ostream& os, const ServerBlockMap& obj){
	os << "block map : " << std::endl;
	os << obj.block_map_;
	os << std::endl;

	return os;
}



} /* namespace sip */
