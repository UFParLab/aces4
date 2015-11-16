/*
 * worker_persistent_array_manager.cpp
 *
 *  Created on: Apr 21, 2014
 *      Author: njindal
 */

#include <worker_persistent_array_manager.h>
#include <algorithm>
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
		SIP_LOG(std::cout << "set_persistent: array= " << runner->array_name(array_id) << ", label=" << runner->string_literal(string_slot) << std::endl);
		std::pair<ArrayIdLabelMap::iterator, bool> ret = persistent_array_map_.insert(std::pair<int, int>(array_id, string_slot));
		//CHECK(ret.second, "duplicate save of array in same sial program ");
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
			SIP_LOG(std::cout << "\nsave marked: array= " << runner->array_name(array_id) << ", label=" << runner->string_literal(string_slot) << std::endl);
			const std::string label = runner->string_literal(string_slot);
			if (runner->is_scalar(array_id)) {
				double value = runner->scalar_value(array_id);
				SIP_LOG(std::cout << "saving scalar with label " << label << " value is " << value << std::endl);
				save_scalar(label, value);
			} else if (runner->is_contiguous(array_id)) {
				Block* contiguous_array = runner->get_and_remove_contiguous_array(array_id);
				SIP_LOG(std::cout << "saving contiguous array with label  "<<  label << " with contents "<< std::endl << *contiguous_array << std::endl);
				save_contiguous(label, contiguous_array);
			} else {
				//in parallel implementation, there won't be any of these on worker.
				IdBlockMap<Block>::PerArrayMap* per_array_map = runner->get_and_remove_per_array_map(array_id);
				SIP_LOG(std::cout << " saving distributed array  with label " << label << " and map with " << per_array_map->size() << " blocks" << std::endl);
				save_distributed(label, per_array_map);
			}
		}

		old_persistent_array_map_.clear();
		old_persistent_array_map_.insert(persistent_array_map_.begin(), persistent_array_map_.end());
		persistent_array_map_.clear();
	}


	void WorkerPersistentArrayManager::restore_persistent(Interpreter* runner, int array_id, int string_slot){
//		std::cerr << "restore_persistent: array= " <<
//				runner->array_name(array_id) << ", label=" <<
//				runner->string_literal(string_slot) << std::endl << std::flush;

		if (runner->is_scalar(array_id))
			restore_persistent_scalar(runner, array_id, string_slot);
		else if (runner->is_contiguous(array_id))
			restore_persistent_contiguous(runner, array_id, string_slot);
		else
			restore_persistent_distributed(runner, array_id, string_slot);
	}


	void WorkerPersistentArrayManager::restore_persistent_scalar(Interpreter* worker, int array_id,
			int string_slot) {
		std::string label = worker->string_literal(string_slot);
		LabelScalarValueMap::iterator it = scalar_value_map_.find(label);
		//CHECK(it != scalar_value_map_.end(),
		//		"scalar to restore with label " + label + " not found");
//		std::cerr<< "restoring scalar " << worker -> array_name(array_id) << "=" << it -> second << std::endl;
		worker->set_scalar_value(array_id, it->second);
		scalar_value_map_.erase(it);
	}


	void WorkerPersistentArrayManager::restore_persistent_contiguous(Interpreter* worker, int array_id, int string_slot) {
		std::string label = worker->string_literal(string_slot);
		LabelContiguousArrayMap::iterator it = contiguous_array_map_.find(
				label);
		//CHECK(it != contiguous_array_map_.end(),
		//		"contiguous array to restore with label " + label
		//				+ " not found");
//		std::cerr<< "restoring contiguous " << worker -> array_name(array_id) << "=" << *(it -> second) << std::endl;
		worker->set_contiguous_array(array_id, it->second);
		contiguous_array_map_.erase(it);
	}

	void WorkerPersistentArrayManager::restore_persistent_distributed(Interpreter* runner,
			int array_id, int string_slot) {
		std::string label = runner->string_literal(string_slot);
		LabelDistributedArrayMap::iterator it = distributed_array_map_.find(label);
		//CHECK(it != distributed_array_map_.end(),
		//		"distributed/served array to restore with label " + label
		//				+ " not found");
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
		//CHECK_WITH_LINE(contig != NULL, "attempting to save nonexistent contiguous array", current_line());
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


	/*
	 * This implementation borrows from code originally implemented to write and read .dat files.
	 */
	void WorkerPersistentArrayManager::checkpoint_persistent(const std::string& filename){
		SIPMPIAttr& attr = SIPMPIAttr::get_instance();
		if( ! (attr.is_company_master() && attr.is_worker()) ) return; //only worker master does this
//		std::cerr << "checkpointing worker in file " << filename << std::endl << std::flush;

		setup::OutputStream * file;
		   setup::BinaryOutputFile *bfile = new setup::BinaryOutputFile(filename);  //checkpoint file opened here
		   file = bfile;
		   //write persistent scalars
		   int nscalars = scalar_value_map_.size();
			file->write_int(nscalars);
//			std::cerr << "\nnscalars=" << nscalars << std::endl << std::flush;
			for (LabelScalarValueMap::iterator it=scalar_value_map_.begin();
					it!= scalar_value_map_.end(); ++it){
				 file -> write_string(it->first);
				 file -> write_double(it->second);
//				 std::cerr << it->first << "=" <<it->second;
			}
			//write contiguous arrays
			int narrays = contiguous_array_map_.size();
			file->write_int(narrays);
//			std::cerr << "\nnarrays=" << narrays << std::endl << std::flush;
			for (LabelContiguousArrayMap::iterator it = contiguous_array_map_.begin(); it != contiguous_array_map_.end(); ++it){
				// Array Name
				file->write_string(it->first);
//				std::cerr << it->first;
				Block::BlockPtr block = it->second;
				// Array Rank
				int array_rank = MAX_RANK;
				file->write_int(array_rank);
//				std::cerr << " rank=" << array_rank ;
				// Array Dimensions
				const int * array_dims = block->shape().segment_sizes_;
				file->write_int_array(array_rank, const_cast<int *>(array_dims));
//				std::cerr << "checkpointing array " << it->first << " with rank= "<< array_rank << "["
//						<< array_dims[0] << ","
//						<< array_dims[1] << ","
//						<< array_dims[2] << ","
//						<< array_dims[3] << ","
//						<< array_dims[4] << ","
//						<< array_dims[5] << "]" << std::endl << std::flush;
				// Array Data
				int num_data_elems = 1;
				for (int i=0; i<array_rank; i++){
					num_data_elems *= array_dims[i];
				}
				double * array_data = it->second->get_data();
				file->write_double_array(num_data_elems, array_data);
//				std::cerr << std::endl << *block;
			}
			delete file; //checkpoint file closed here

	}


	void WorkerPersistentArrayManager::init_from_checkpoint(const std::string& filename){
		if(! SIPMPIAttr::get_instance().is_worker()) return;  //only workers do this

//		std::cerr<< "WorkerPersistentArrayManager::init_from_checkpoint(" << filename << ")" << std::endl << std::flush;
		check(contiguous_array_map_.empty(), "initializing nonempty persistent contiguous_array_map_ from checkpoint");
		check(scalar_value_map_.empty(), "initializing nonempty persistent scalar_value_map_ from checkpoint");
			setup::InputStream * file;
			setup::BinaryInputFile *bfile = new setup::BinaryInputFile(filename);  //checkpoint file opened here
			file = bfile;
			//restore scalars
//			std::cerr<< "WorkerPersistentArrayManager opened the file " << filename << std::endl << std::flush;
			int num_scalars = file->read_int();
//			std::cerr << "\nnum_scalars=" << num_scalars << std::endl << std::flush;
			for (int i=0; i < num_scalars; ++i){
				std::string name = file->read_string();
				double value = file->read_double();
				scalar_value_map_[name] = value;
//				std::cerr << "initializing persistent scalar "<< name << "=" << value << " from restart" << std::endl << std::flush;
			}
			//restore contiguous arrays
			int num_arrays = file->read_int();
//			std::cerr << "\n num_arrays=" << num_arrays << std::endl << std::flush;
			for (int i = 0; i < num_arrays; i++) {
				// Name of array
				std::string name = file->read_string();
				// Rank of array
				int rank = file->read_int();
				// Dimensions
				int * dims = file->read_int_array(&rank);
				// Number of elements
				int num_data_elems = 1;
				for (int i=0; i<rank; i++){
					num_data_elems *= dims[i];
				}
//				std::cerr << "initializing persistent array" << name  << " from restart with rank= "<< rank << "["
//						<< dims[0] << ","
//						<< dims[1] << ","
//						<< dims[2] << ","
//						<< dims[3] << ","
//						<< dims[4] << ","
//						<< dims[5] << "]" << std::endl << std::flush;

				double * data = file->read_double_array(&num_data_elems);
        		sip::segment_size_array_t dim_sizes;
				std::copy(dims+0, dims+rank,dim_sizes);
				if (rank < MAX_RANK) std::fill(dim_sizes+rank, dim_sizes+MAX_RANK, 1);
				sip::BlockShape shape(dim_sizes, rank);
				delete [] dims;

				Block::BlockPtr block = new Block(shape,data);
				contiguous_array_map_[name]=block;
//				std::cerr << *block << std::endl << std::flush;

			}

//			std::cerr << "dumping worker's persistent array map " << std::endl << *this << std::endl << std::flush;

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
