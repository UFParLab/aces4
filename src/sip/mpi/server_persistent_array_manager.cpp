/*
 * server_persistent_array_manager.cpp
 *
 *  Created on: Apr 21, 2014
 *      Author: njindal
 */

#include <server_persistent_array_manager.h>

#include "sip_server.h"
#include "id_block_map.h"

namespace sip {

	ServerPersistentArrayManager::ServerPersistentArrayManager() {}

	ServerPersistentArrayManager::~ServerPersistentArrayManager() {
		ServerPersistentArrayManager::LabelDistributedArrayMap::iterator dit;
		for (dit = distributed_array_map_.begin(); dit != distributed_array_map_.end(); ++dit){
			IdBlockMap<ServerBlock>::delete_blocks_from_per_array_map(dit -> second);
			delete dit -> second;
			dit -> second = NULL;
		}
	}


	void ServerPersistentArrayManager::set_persistent(SIPServer* runner, int array_id, int string_slot) {
		SIP_LOG(std::cout << "set_persistent: array= "
				<< runner->sip_tables()->array_name(array_id)
				<< ", label=" << runner->sip_tables()->string_literal(string_slot) << std::endl);
		std::pair<ArrayIdLabelMap::iterator, bool> ret = persistent_array_map_.insert(std::pair<int, int>(array_id, string_slot));
		CHECK(ret.second, "duplicate save of array in same sial program ");
	}


	void ServerPersistentArrayManager::save_marked_arrays(SIPServer* runner) {
		ArrayIdLabelMap::iterator it;
		for (it = persistent_array_map_.begin();
				it != persistent_array_map_.end(); ++it) {
			int array_id = it->first;
			int string_slot = it->second;
			const std::string label = runner->sip_tables()->string_literal(string_slot);
			CHECK( !runner->sip_tables()->is_scalar(array_id) && !runner->sip_tables()->is_contiguous(array_id),
					" Tried to save a scalar or contiguous array. Something went very wrong in the server.");

			IdBlockMap<ServerBlock>::PerArrayMap* per_array_map = runner->get_and_remove_per_array_map(array_id);
			save_distributed(runner, array_id, label, per_array_map);
		}
		persistent_array_map_.clear();
	}


	void ServerPersistentArrayManager::restore_persistent(SIPServer* runner, int array_id, int string_slot){
		SIP_LOG(std::cout << "restore_persistent: array= " <<
				runner->array_name(array_id) << ", label=" <<
				runner->string_literal(string_slot) << std::endl;)

		CHECK ( !runner->sip_tables()->is_scalar(array_id) && !runner->sip_tables()->is_contiguous(array_id),
							" Tried to restore a scalar or contiguous array. Something went very wrong in the server.");

		restore_persistent_distributed(runner, array_id, string_slot);
	}


	void ServerPersistentArrayManager::restore_persistent_distributed(SIPServer* runner,
			int array_id, int string_slot) {
		std::string label = runner->sip_tables()->string_literal(string_slot);
		runner->block_map_.restore_persistent_array(array_id, label);

		LabelDistributedArrayMap::iterator it = distributed_array_map_.find(label);
		CHECK(it != distributed_array_map_.end(), "distributed/served array to restore with label " + label + " not found");
	    // Clear out all existing blocks of array in memory
		runner->get_and_remove_per_array_map(array_id);
		runner->set_per_array_map(array_id, it->second);
		delete it->second;
		it->second = NULL;
		distributed_array_map_.erase(it);
	}


	void ServerPersistentArrayManager::save_distributed(SIPServer* runner, const int array_id, const std::string& label, IdBlockMap<ServerBlock>::PerArrayMap* array_blocks) {

		runner->block_map_.save_persistent_array(array_id, label, array_blocks);
		const std::pair<LabelDistributedArrayMap::iterator, bool> ret =
					distributed_array_map_.insert(std::pair<std::string, IdBlockMap<ServerBlock>::PerArrayMap*>(label, array_blocks));
		if (!check_and_warn(ret.second,
				"Label " + label + "already used for distributed/served array.  Overwriting previously saved array.")) {
					distributed_array_map_.erase(ret.first);
					distributed_array_map_.insert(std::pair<std::string, IdBlockMap<ServerBlock>::PerArrayMap*>(label,array_blocks));
		}
	}


	std::ostream& operator<<(std::ostream& os, const ServerPersistentArrayManager& obj){
		os << "********SERVER PERSISTENT ARRAY MANAGER********" << std::endl;
		os << "Marked arrays: size=" << obj.persistent_array_map_.size() << std::endl;
		ServerPersistentArrayManager::ArrayIdLabelMap::const_iterator mit;
		for (mit = obj.persistent_array_map_.begin(); mit != obj.persistent_array_map_.end(); ++mit){
			os << mit -> first << ": " << mit -> second << std::endl;
		}
		os<< "*********END OF SERVER PERSISTENT ARRAY MANAGER******" << std::endl;
		return os;
	}

} /* namespace sip */
