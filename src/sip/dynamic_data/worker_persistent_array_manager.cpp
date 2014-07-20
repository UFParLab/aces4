/*
 * worker_persistent_array_manager.cpp
 *
 *  Created on: Apr 21, 2014
 *      Author: njindal
 */

#include <worker_persistent_array_manager.h>
#include "interpreter.h"
#include "id_block_map.h"

namespace sip {

	WorkerPersistentArrayManager::WorkerPersistentArrayManager() {}

	WorkerPersistentArrayManager::~WorkerPersistentArrayManager() {

		WorkerPersistentArrayManager::LabelContiguousArrayMap::iterator cit;
		for (cit = contiguous_array_map_.begin(); cit != contiguous_array_map_.end(); ++cit){
			delete cit -> second;
			cit -> second = NULL;
		}

		WorkerPersistentArrayManager::LabelDistributedArrayMap::iterator dit;
		for (dit = distributed_array_map_.begin(); dit != distributed_array_map_.end(); ++dit){
			IdBlockMap<Block>::delete_blocks_from_per_array_map(dit -> second);
			delete dit -> second;
			dit -> second = NULL;
		}
	}


	void WorkerPersistentArrayManager::set_persistent(Interpreter* runner, int array_id, int string_slot) {
//		std::cout << "set_persistent: array= " << runner->sip_tables()->array_name(array_id) << ", label=" << runner->sip_tables()->string_literal(string_slot) << std::endl;
		std::pair<ArrayIdLabelMap::iterator, bool> ret = persistent_array_map_.insert(std::pair<int, int>(array_id, string_slot));
		check(ret.second, "duplicate save of array in same sial program ");
		//check(ret.second, "duplicate save of array in same sial program " + SipTables::instance().array_name(array_id));
		//note that duplicate label for same type of object will
		//be detected during the save process so we don't
		//check for unique labels here.
	}


	void WorkerPersistentArrayManager::save_marked_arrays(Interpreter* runner) {
		ArrayIdLabelMap::iterator it;
		for (it = persistent_array_map_.begin();
				it != persistent_array_map_.end(); ++it) {
			int array_id = it->first;
			int string_slot = it->second;
			//DEBUG
	//			std::cout << "\nsave marked: array= " << runner->array_name(array_id) << ", label=" << runner->string_literal(string_slot) << std::endl;
			const std::string label = runner->sip_tables()->string_literal(string_slot);
			if (runner->sip_tables()->is_scalar(array_id)) {
				double value = runner->scalar_value(array_id);
	//				std::cout << "saving scalar with label " << label << " value is " << value << std::endl;
				save_scalar(label, value);
			} else if (runner->sip_tables()->is_contiguous(array_id)) {
				Block* contiguous_array = runner->get_and_remove_contiguous_array(array_id);
	//				std::cout << "saving contiguous array with label  "<<  label << " with contents "<< std::endl << *contiguous_array << std::endl;
				save_contiguous(label, contiguous_array);
			} else {
				//in parallel implementation, there won't be any of these on worker.
				IdBlockMap<Block>::PerArrayMap* per_array_map = runner->get_and_remove_per_array_map(array_id);
	//				std::cout << " saving distributed array  with label " << label << " and map with " << per_array_map->size() << " blocks" << std::endl;
				save_distributed(label, per_array_map);
			}
		}
		persistent_array_map_.clear();
	}


	void WorkerPersistentArrayManager::restore_persistent(Interpreter* runner, int array_id, int string_slot){
		SIP_LOG(std::cout << "restore_persistent: array= " <<
				runner->array_name(array_id) << ", label=" <<
				runner->string_literal(string_slot) << std::endl;)

		if (runner->sip_tables()->is_scalar(array_id))
			restore_persistent_scalar(runner, array_id, string_slot);
		else if (runner->sip_tables()->is_contiguous(array_id))
			restore_persistent_contiguous(runner, array_id, string_slot);
		else
			restore_persistent_distributed(runner, array_id, string_slot);
	}


	void WorkerPersistentArrayManager::restore_persistent_scalar(Interpreter* worker, int array_id,
			int string_slot) {
		std::string label = worker->sip_tables()->string_literal(string_slot);
		LabelScalarValueMap::iterator it = scalar_value_map_.find(label);
		check(it != scalar_value_map_.end(),
				"scalar to restore with label " + label + " not found");
		SIP_LOG(std::cout<< "restoring scalar " << worker -> array_name(array_id) << "=" << it -> second << std::endl);
		worker->set_scalar_value(array_id, it->second);
		scalar_value_map_.erase(it);
	}


	void WorkerPersistentArrayManager::restore_persistent_contiguous(Interpreter* worker, int array_id, int string_slot) {
		std::string label = worker->sip_tables()->string_literal(string_slot);
		LabelContiguousArrayMap::iterator it = contiguous_array_map_.find(
				label);
		check(it != contiguous_array_map_.end(),
				"contiguous array to restore with label " + label
						+ " not found");
		worker->set_contiguous_array(array_id, it->second);
		contiguous_array_map_.erase(it);
	}

	void WorkerPersistentArrayManager::restore_persistent_distributed(Interpreter* runner,
			int array_id, int string_slot) {
		std::string label = runner->sip_tables()->string_literal(string_slot);
		LabelDistributedArrayMap::iterator it = distributed_array_map_.find(label);
		check(it != distributed_array_map_.end(),
				"distributed/served array to restore with label " + label
						+ " not found");
		runner->set_per_array_map(array_id, it->second);
		distributed_array_map_.erase(it);
	}

	void WorkerPersistentArrayManager::save_scalar(const std::string label, double value) {
		const std::pair<LabelScalarValueMap::iterator, bool> ret = scalar_value_map_.insert(std::pair<std::string, double>(label, value));
		if (!check_and_warn(ret.second, "Label " + label + "already used for scalar.  Overwriting previously saved value.")) {
			scalar_value_map_.erase(ret.first);
			scalar_value_map_.insert(std::pair<std::string,double>(label,value));
		}
	}

	void WorkerPersistentArrayManager::save_contiguous(const std::string label, Block* contig) {
		check(contig != NULL, "attempting to save nonexistent contiguous array", current_line());
		const std::pair<LabelContiguousArrayMap::iterator, bool> ret = contiguous_array_map_.insert(std::pair<std::string, Block*>(label, contig));
		SIP_LOG(std::cout << "save_contiguous: ret= " << ret.second);
		if (!check_and_warn(ret.second, "Label " + label + "already used for contiguous array.  Overwriting previously saved array.")) {
			contiguous_array_map_.erase(ret.first);
			contiguous_array_map_.insert(std::pair<std::string, Block*>(label, contig));
		}

	}

	void WorkerPersistentArrayManager::save_distributed(const std::string label, IdBlockMap<Block>::PerArrayMap* map) {
			const std::pair<LabelDistributedArrayMap::iterator, bool> ret =
					distributed_array_map_.insert(std::pair<std::string, IdBlockMap<Block>::PerArrayMap*>(label, map));
		if (!check_and_warn(ret.second,
				"Label " + label + "already used for distributed/served array.  Overwriting previously saved array.")) {
					distributed_array_map_.erase(ret.first);
					distributed_array_map_.insert(std::pair<std::string, IdBlockMap<Block>::PerArrayMap*>(label,map));
		}
	}


	std::ostream& operator<<(std::ostream& os, const WorkerPersistentArrayManager& obj){
		os << "********WORKER PERSISTENT ARRAY MANAGER********" << std::endl;
		os << "Marked arrays: size=" << obj.persistent_array_map_.size() << std::endl;
		WorkerPersistentArrayManager::ArrayIdLabelMap::const_iterator mit;
		for (mit = obj.persistent_array_map_.begin(); mit != obj.persistent_array_map_.end(); ++mit){
			os << mit -> first << ": " << mit -> second << std::endl;
		}
		os << "ScalarValueMap: size=" << obj.scalar_value_map_.size()<<std::endl;
		WorkerPersistentArrayManager::LabelScalarValueMap::const_iterator it;
		for (it = obj.scalar_value_map_.begin(); it != obj.scalar_value_map_.end(); ++it){
			os << it->first << "=" << it->second << std::endl;
		}
		os << "ContiguousArrayMap: size=" << obj.contiguous_array_map_.size() << std::endl;
		WorkerPersistentArrayManager::LabelContiguousArrayMap::const_iterator cit;
		for (cit = obj.contiguous_array_map_.begin(); cit != obj.contiguous_array_map_.end(); ++cit){
			os << cit -> first << std::endl;
		}
		os << "Distributed/ServedArrayMap: size=" << obj.distributed_array_map_.size() << std::endl;
		WorkerPersistentArrayManager::LabelDistributedArrayMap::const_iterator dit;
		for (dit = obj.distributed_array_map_.begin(); dit != obj.distributed_array_map_.end(); ++dit){
			os << dit -> first << std::endl;
			//os << dit -> second << std::endl;
		}
		os<< "*********END OF WORKER PERSISTENT ARRAY MANAGER******" << std::endl;
		return os;
	}

} /* namespace sip */
