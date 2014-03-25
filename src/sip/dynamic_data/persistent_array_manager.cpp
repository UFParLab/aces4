///*
// * persistent_array_manager.cpp
// *
// *  Created on: Jan 8, 2014
// *      Author: njindal
// */
//
//#include "persistent_array_manager.h"
//#ifdef HAVE_MPI
//#include "sip_mpi_data.h"
//#endif //HAVE_MPI
//namespace sip {
//
//
////PersistentArrayManager::PersistentArrayManager() {
////
////	sip_mpi_attr_ = sip::SIPMPIAttr::get_instance();
////
////}
//
////PersistentArrayManager::~PersistentArrayManager() {}
//
////	void PersistentArrayManager::set_persistent(int array_id, int string_slot, bool is_distributed){
////		const std::pair<ArrayIdLabelMap::iterator,bool> ret = persistent_array_map_.insert(std::pair<int, int>(array_id, string_slot));
////		check(ret.second, "duplicate save of array in same sial program " + array_name_value(array_id), current_line());
////		//duplicate label for same type of object will be detected during the save process so we don't check for that here.
////		//distributed arrays stored for error checking purposes for mpi implementations.  Workers should ignore when saving
////
////		if (sip_mpi_attr_.is_worker() && is_distributed){
////			//send array_id and string_slot to all servers
////			send_set_persistent(array_id, string_slot);
////		}
////	}
//
////	/**
////	 * Called during post processing. Values of marked scalars are copied into the scalar_value_map_;
////	 *  marked contiguous and distributed/served arrays are moved to the contiguous_array_map_ and
////	 *  distributed_array_map_ respectively.  In a parallel implementation, distributed arrays
////	 *  should only be marked at servers.
////	 */
////	void PersistentArrayManager::save_marked_arrays_on_worker(const Interpreter& worker){
//////		check(sip_mpi_attr_.is_worker(), "Error: calling save_marked_arrays_on_worker called by a server");
////		SipTables& sip_tables = worker.sip_tables_;
////		ArrayIdLabelMap::iterator it;
////		for (it = persistent_array_map_.begin(); it != persistent_array_map_.end(); ++it){
////			int array_id = it->first;
////			int string_slot = it->second;
////			const std::string label = sip_tables.string_literal(string_slot);
////			if (sip_tables.is_scalar(array_id)){
////				double value = worker.scalar_value(array_id);
////				const std::pair<LabelScalarValueMap::iterator,bool> ret = scalar_value_map_.insert(std::pair<std::string,double>(label,value));
////				if !(check_and_warn(ret->second && ret->first->second != value, "Overwriting label " + label +
////						 ": old scalar value=" + ret->first->second + "; new value=" + value), current_line()){
////					scalar_value_map_.erase(ret->first);
////					scalar_value_map_.insert(std::pair<std::string,double>(label,value));
////				}
////			}
////			else if (sip_tables.is_contiguous(array_id)){
////				Block::BlockPtr bptr = worker.data_manager_.contiguous_array_manager_.get_and_remove_array(array_id);
////				const std::pair<LabelContiguousArrayMap::iterator,bool> ret = contiguous_array_map_.insert(std::pair<std::string,Block::BlockPtr>(label,bptr));
////				if !(check_and_warn(ret->second && ret->first->second != bptr, "Overwriting label " + label + "with contiguous array", current_line())){
////					contiguous_array_map_.erase(ret->first);
////					contiguous_array_map_.insert(std::pair<std::string,double>(label,value));
////				}
////			}
////			else if (sip_tables.is_distributed){
////#ifdef HAVE_MPI
////				check(false,"saved array on worker was neither scalar nor contiguous");
////#else
////				BlockManager::IdBlockMapPtr bmptr = worker.data_manager_.block_manager_.get_and_remove_map(array_id);
////				const std::pair<LabelDistributedArrayMap::iterator,bool> ret = distributed_array_map_.insert(std::pair<std::string,Block::BlockPtr>(label,bmptr));
////				if !(check_and_warn(ret->second && ret->first->second != bmptr, "Overwriting label " + label + "with distributed array", current_line())){
////					distributed_array_map_.erase(ret->first);
////					distributed_array_map_.insert(std::pair<std::string,double>(label,value));
////#endif //HAVE_MPI
////				}
////		}
////	   persistent_array_map_.clear();
////	}
//
////    void PersistentArrayManager::save_marked_arrays_on_server(const SIPServer& server){
////		ArrayIdLabelMap::iterator it;
////		SipTables& sip_tables = server.sip_tables_;
////		for (it = persistent_array_map_.begin(); it != persistent_array_map_.end(); ++it){
////			int array_id = it->first;
////			int string_slot = it->second;
////			const std::string label = sip_tables.string_literal(string_slot);
////				IdBlockMap::PerArrayMap* bmptr = server.get_and_remove_map(array_id);
////				const std::pair<LabelDistributedArrayMap::iterator,bool> ret = distributed_array_map_.insert(std::pair<std::string,Block::BlockPtr>(label,bmptr));
////				if !(check_and_warn(ret->second && ret->first->second != bptr, "Overwriting label " + label + "with distributed array", current_line())){
////					distributed_array_map_.erase(ret->first);
////					distributed_array_map_.insert(std::pair<std::string,double>(label,value));
////				}
////		}
////	   persistent_array_map_.clear();
////	}
//
//
//	void PersistentArrayManager::restore_persistent_on_worker(Interpreter *worker, int array_id, int string_slot){
//		const std::string label = worker->string_literal(string_slot);
//		if (worker->is_scalar(array_id))
//			restore_persistent_scalar(worker, array_id, label);
//		else if (worker->is_contiguous(array_id))
//			restore_persistent_contiguous_array(array_id, label);
//		else
//#ifdef HAVE_MPI
//			send_restore_persistent(array_id, string_slot);
//#else
//			restore_persistent_distributed(array_id, label);
//#endif //HAVE_MPI
//		}
//	}
//
////#ifdef HAVE_MPI
//
//	int SET_PERSISTENT_SIZE = 2;
//
//	int RESTORE_PERSISTENT_SIZE=2;
//	/**
//	 *
//	 */
//	void PersistentArrayManager::send_set_persistent(int array_id, int string_slot){
//		int my_server = sip_mpi_attr_.my_server_to_control();
//		if( my_server >= 0){
//			int buff [SET_PERSISTENT_SIZE];
//			buff[0] = array_id;
//			buff[1] = string_slot;
//			MPI_Status status;
//		    MPI_Send(&buff, SET_PERSISTENT_SIZE, MPI_INT, my_server, SET_PERSISTENT, MPI_COMM_WORLD);
//		    MPI_Recv(0,0,MPI_INT, my_server, SET_PERSISTENT_ACK, MPI_COMM_WORLD, &status);
//		}
//	}
//
////	void PersistentArrayManager::receive_set_persistent(int source){
////		int buff [SET_PERSISTENT_SIZE];
////		MPI_Status status;
////		MPI_Recv(&buff,SET_PERSISTENT_SIZE,MPI_INT, source, SET_PERSISTENT, MPI_COMM_WORLD, &status);
////		MPI_Send(0,0,MPI_INT, source, SET_PERSISTENT_ACK, MPI_COMM_WORLD);
////		int array_id = buff[0];
////		int string_slot = buff[1];
////		set_persistent(array_id, string_slot, true);
////	}
//
//	void PersistentArrayManager::send_restore_persistent(int array_id, int string_slot){
//		SIPMPIAttr& mpi_attribute = SIPMPIAttr::get_instance();
//		int num_ranks = mpi_attribute.global_size();
//		int my_rank = mpi_attribute.global_rank();
//		if (RankDistribution::is_local_worker_to_communicate(my_rank, num_ranks)){
//
//		int my_server = RankDistribution::local_server_to_communicate(my_rank, num_ranks);
//		check(my_server>=0, "error in rank distribution");
//			int buff [RESTORE_PERSISTENT_SIZE];
//			buff[0] = array_id;
//			buff[1] = string_slot;
//			int tag =
//			MPI_Status status;
//		    MPI_Send(buff, RESTORE_PERSISTENT_SIZE, MPI_INT, my_server, RESTORE_PERSISTENT, MPI_COMM_WORLD);
//		    MPI_Recv(0,0,MPI_INT, my_server, RESTORE_PERSISTENT_ACK, MPI_COMM_WORLD, &status);
//		}
//	}
//
////	void PersistentArrayManager::receive_restore_persistent(int source){
////		int buff [RESTORE_PERSISTENT_SIZE];
////		MPI_Status status;
////		MPI_Recv(&buff,RESTORE_PERSISTENT_SIZE,MPI_INT, source, RESTORE_PERSISTENT, MPI_COMM_WORLD, &status);
////		MPI_Send(0,0,MPI_INT, source, RESTORE_PERSISTENT_ACK, MPI_COMM_WORLD);
////		int array_id = buff[0];
////		int string_slot = buff[1]
////		restore_persistent_distributed(array_id, string_slot);
////	}
////#endif //HAVE_MPI
//}
//std::ostream& operator<<(std::ostream& os, const PersistentArrayManager& obj){
//	os << "Persistent Array Manager" << std::endl;
//	os << "ScalarValueMap:" << std::endl;
//	LabelScalarValueMap::iterator it;
//	for (it = scalar_value_map_.begin(); it != scalar_value_map_.end(); ++it){
//		os << it->first << ":" << it->second << std::endl;
//	}
//	os << "ContiguousArrayMap:" << std::endl;
//	LabelContiguousArrayMap::iterator cit;
//	for (cit = contiguous_array_map_.begin(); cit != contiguous_array_map_.end(); ++cit){
//		os << it -> first << std::endl;
//	}
//	os << "Distributed/ServedArrayMap:" << std::endl;
//	LabelDistributedArrayMap::iterator dit;
//	for (dit = distributed_array_map_.begin(); dit != distributed_array_map_.end(); ++dit{
//		os << it -> first << std::endl;
//		os <<
//	}
//
//
//
//} /* namespace sip */
