/*
 * server_persistent_array_manager.cpp
 *
 *  Created on: Apr 21, 2014
 *      Author: njindal
 */

#include <server_persistent_array_manager.h>

#include "sip_server.h"

namespace sip {

	ServerPersistentArrayManager::ServerPersistentArrayManager() {}

	ServerPersistentArrayManager::~ServerPersistentArrayManager() {}


	void ServerPersistentArrayManager::set_persistent(SIPServer* runner, int array_id, int string_slot) {
//		std::cout << "set_persistent: array= " << runner->sip_tables()->array_name(array_id) << ", label=" << runner->sip_tables()->string_literal(string_slot) << std::endl;
		std::pair<ArrayIdLabelMap::iterator, bool> ret = persistent_array_map_.insert(std::pair<int, int>(array_id, string_slot));
		check(ret.second, "duplicate save of array in same sial program ");
		//check(ret.second, "duplicate save of array in same sial program " + SipTables::instance().array_name(array_id));
		//note that duplicate label for same type of object will
		//be detected during the save process so we don't
		//check for unique labels here.
	}


	void ServerPersistentArrayManager::save_marked_arrays(SIPServer* runner) {
		ArrayIdLabelMap::iterator it;
		for (it = persistent_array_map_.begin();
				it != persistent_array_map_.end(); ++it) {
			int array_id = it->first;
			int string_slot = it->second;
			//DEBUG
	//			std::cout << "\nsave marked: array= " << runner->array_name(array_id) << ", label=" << runner->string_literal(string_slot) << std::endl;
			const std::string label = runner->sip_tables()->string_literal(string_slot);

			sip::check ( !runner->sip_tables()->is_scalar(array_id) && !runner->sip_tables()->is_contiguous(array_id),
					" Tried to save a scalar or contiguous array. Something went very wrong in the server.");

			//in parallel implementation, there won't be any of these on worker.
			IdBlockMap<ServerBlock>::PerArrayMap* per_array_map = runner->get_and_remove_per_array_map(array_id);
	//				std::cout << " saving distributed array  with label " << label << " and map with " << per_array_map->size() << " blocks" << std::endl;
			save_distributed(label, per_array_map);
		}
		persistent_array_map_.clear();
	}


	void ServerPersistentArrayManager::restore_persistent(SIPServer* runner, int array_id, int string_slot){
		SIP_LOG(std::cout << "restore_persistent: array= " <<
				runner->array_name(array_id) << ", label=" <<
				runner->string_literal(string_slot) << std::endl;)

		sip::check ( !runner->sip_tables()->is_scalar(array_id) && !runner->sip_tables()->is_contiguous(array_id),
							" Tried to restore a scalar or contiguous array. Something went very wrong in the server.");

		restore_persistent_distributed(runner, array_id, string_slot);
	}


	void ServerPersistentArrayManager::restore_persistent_distributed(SIPServer* runner,
			int array_id, int string_slot) {
		std::string label = runner->sip_tables()->string_literal(string_slot);
		LabelDistributedArrayMap::iterator it = distributed_array_map_.find(label);
		check(it != distributed_array_map_.end(),
				"distributed/served array to restore with label " + label
						+ " not found");
		runner->set_per_array_map(array_id, it->second);
		distributed_array_map_.erase(it);
	}


	void ServerPersistentArrayManager::save_distributed(const std::string& label, IdBlockMap<ServerBlock>::PerArrayMap* map) {
			const std::pair<LabelDistributedArrayMap::iterator, bool> ret =
					distributed_array_map_.insert(std::pair<std::string, IdBlockMap<ServerBlock>::PerArrayMap*>(label, map));
		if (!check_and_warn(ret.second,
				"Label " + label + "already used for distributed/served array.  Overwriting previously saved array.")) {
					distributed_array_map_.erase(ret.first);
					distributed_array_map_.insert(std::pair<std::string, IdBlockMap<ServerBlock>::PerArrayMap*>(label,map));
		}
	}


	std::ostream& operator<<(std::ostream& os, const ServerPersistentArrayManager& obj){
		os << "********SERVER PERSISTENT ARRAY MANAGER********" << std::endl;
		os << "Marked arrays: size=" << obj.persistent_array_map_.size() << std::endl;
		ServerPersistentArrayManager::ArrayIdLabelMap::const_iterator mit;
		for (mit = obj.persistent_array_map_.begin(); mit != obj.persistent_array_map_.end(); ++mit){
			os << mit -> first << ": " << mit -> second << std::endl;
		}
		os << "ScalarValueMap: size=" << obj.scalar_value_map_.size()<<std::endl;
		ServerPersistentArrayManager::LabelScalarValueMap::const_iterator it;
		for (it = obj.scalar_value_map_.begin(); it != obj.scalar_value_map_.end(); ++it){
			os << it->first << "=" << it->second << std::endl;
		}
		os << "Distributed/ServedArrayMap: size=" << obj.distributed_array_map_.size() << std::endl;
		ServerPersistentArrayManager::LabelDistributedArrayMap::const_iterator dit;
		for (dit = obj.distributed_array_map_.begin(); dit != obj.distributed_array_map_.end(); ++dit){
			os << dit -> first << std::endl;
			//os << dit -> second << std::endl;
		}
		os<< "*********END OF SERVER PERSISTENT ARRAY MANAGER******" << std::endl;
		return os;
	}

} /* namespace sip */
