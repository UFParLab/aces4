/*
 * sip_server.cpp
 *
 *  Created on: Jan 14, 2014
 *      Author: njindal
 */

#include <sip_server.h>
#include "sip_mpi_data.h"
#include "mpi.h"
#include "sip.h"
#include "blocks.h"
#include "sip_mpi_utils.h"

namespace sip {

SIPServer::SIPServer(SipTables& sip_tables, DataDistribution &data_distribution, SIPMPIAttr & sip_mpi_attr,
		sip::PersistentArrayManager& pbm_read, sip::PersistentArrayManager& pbm_write):
		sip_tables_(sip_tables), data_distribution_(data_distribution), sip_mpi_attr_(sip_mpi_attr),
		pbm_read_(pbm_read), pbm_write_(pbm_write), block_map_(sip_tables.num_arrays())		{
	int num_arrs = block_map_.size();
	for (int i=0; i< num_arrs; i++){
		block_map_[i] = NULL;
	}
}

SIPServer::~SIPServer() {
	int num_arrs = block_map_.size();
	for (int i=0; i<num_arrs; i++){
		if (!pbm_write_.is_array_persistent(i)){
			IdBlockMapPtr bid_map = block_map_[i];
			if (bid_map != NULL){
				IdBlockMap::iterator it;
				for (it = bid_map->begin(); it != bid_map->end(); ++it){
					if (it->second != NULL)
						delete it->second;	// Delete the block being pointed to.
				}
				delete block_map_[i];
				block_map_[i] = NULL;
			}
		}
	}
}


// Get rid of blocks from given array
void SIPServer::delete_array(int array_id) {
	//BlockMap::iterator it = block_map_.find(array_id);

	// Only delete if the block map contains this array.
	IdBlockMapPtr bid_map = block_map_[array_id];
	if (bid_map != NULL){
		for (IdBlockMap::iterator it = bid_map->begin(); it != bid_map->end(); ++it) {
			sip::Block::BlockPtr bptr = it->second;
			delete bptr;
		}
		//delete block_map_[array_id];
		delete bid_map;
		block_map_[array_id] = NULL;
	}
}

int SIPServer::get_array_id(const int rank) {
	// From the servers master
	// Receieve the ID of the array to delete
	// Inform other servers
	int array_id = -1;
	MPI_Status array_id_status;
	sip::SIPMPIUtils::check_err(MPI_Recv(&array_id, 1, MPI_INT, rank, MPI_ANY_TAG, MPI_COMM_WORLD, &array_id_status));
	int id_msg_size_;
	sip::SIPMPIUtils::check_err(MPI_Get_count(&array_id_status, MPI_INT, &id_msg_size_));
	sip::check(1 == id_msg_size_, "Received more than 1 byte at SIP Server !");
	return array_id;
}


void SIPServer::send_to_other_servers(int to_send, int tag) {
	const std::vector<int>& server_ranks = sip_mpi_attr_.server_ranks();
	for (std::vector<int>::const_iterator it = server_ranks.begin(); it != server_ranks.end(); ++it) {
		const int server_rank = *it;
		if (server_rank != sip_mpi_attr_.global_rank()) {
			SIP_LOG(std::cout << sip_mpi_attr_.global_rank() << " : Sending "<<to_send<<" to server "<<server_rank<<std::endl);
			sip::SIPMPIUtils::check_err(MPI_Send(&to_send, 1, MPI_INT, server_rank, tag, MPI_COMM_WORLD));
		}
	}
}

void SIPServer::send_to_other_servers(const char *str, int len, int tag) {
	sip::check(len < SIPMPIData::MAX_STRING, "Trying to send a very large string to other servers !");
	const std::vector<int>& server_ranks = sip_mpi_attr_.server_ranks();
	for (std::vector<int>::const_iterator it = server_ranks.begin(); it != server_ranks.end(); ++it) {
		const int server_rank = *it;
		if (server_rank != sip_mpi_attr_.global_rank()) {
			SIP_LOG(std::cout << sip_mpi_attr_.global_rank() << " : Sending String"<<std::string(str)<<" to server "<<server_rank<<std::endl);
			MPI_Send(const_cast<char*>(str), len, MPI_CHAR, server_rank, tag, MPI_COMM_WORLD);
		}
	}
}

std::string SIPServer::get_string_from(int rank, int &size) {
	MPI_Status str_status;
	char str[SIPMPIData::MAX_STRING];
	sip::SIPMPIUtils::check_err(MPI_Recv(str, SIPMPIData::MAX_STRING, MPI_CHAR, rank, MPI_ANY_TAG,
			MPI_COMM_WORLD, &str_status));
	int len = 0;
	sip::SIPMPIUtils::check_err(MPI_Get_count(&str_status, MPI_CHAR, &len));
	size = len;
	std::string recvd_string = std::string(str);
	SIP_LOG(std::cout << sip_mpi_attr_.global_rank() << " : Got String "<<" from rank "<<str_status.MPI_SOURCE<<std::endl);
	return recvd_string;
}

void SIPServer::post_program_processing() {

	// Save all the arrays marked as persistent
	int num_arrs = block_map_.size();
//	for (BlockMap::iterator it = block_map_begin(); it != block_map_.end(); ++it) {
	for(int i=0; i<num_arrs; i++){
		if (pbm_write_.is_array_persistent(i)) {
			IdBlockMapPtr bid_map = block_map_[i];
			if (bid_map == NULL){
				SIP_LOG(std::cout << sip_mpi_attr_.global_rank() << " : Trying to save NULL map with label : "<< pbm_write_.get_label_of_marked_array(i) <<std::endl);
				bid_map = new IdBlockMap();
			}
			pbm_write_.save_dist_array(i, bid_map);
			block_map_[i] = NULL;
		}
	}
}

void SIPServer::restore_persistent_array(const std::string& label, int array_id) {

	// Clear blocks of array
	sip::BlockManager::IdBlockMapPtr bid_map = pbm_read_.get_saved_dist_array(label);
	sip::BlockManager::IdBlockMap *old_bid_map = block_map_[array_id];
	if (old_bid_map != NULL){
		for (sip::BlockManager::IdBlockMap::iterator it = old_bid_map->begin(); it != old_bid_map->end(); ++it){
			delete it->second;
		}
		delete block_map_[array_id];
	}

	block_map_[array_id] = new IdBlockMap();

	// Put in new blocks.
	if (bid_map != NULL){
	for (sip::BlockManager::IdBlockMap::iterator it = bid_map->begin(); it != bid_map->end(); ++it){
		sip::BlockId bid(it->first);
		bid.array_id_ = array_id;
		sip::Block::BlockPtr bptr_clone = it->second->clone();
		block_map_[array_id]->insert(std::pair<sip::BlockId, sip::Block::BlockPtr>(bid, bptr_clone));
	}
	}else {
		SIP_LOG(std::cout << sip_mpi_attr_.global_rank() << " : Trying to restore empty map with label : "<<label<<std::endl);
	}
}

void SIPServer::handle_GET(int mpi_source) {
	SIP_LOG(std::cout << sip_mpi_attr_.global_rank()<< " : In GET at server from rank " << mpi_source<< std::endl);

	// Receive a message with the block being requested - blockid
	sip::BlockId bid = SIPMPIUtils::get_block_id_from_rank(mpi_source, SIPMPIData::WORKER_TO_SERVER_BLOCKID);
	int array_id = bid.array_id();
	SIP_LOG(std::cout << sip_mpi_attr_.global_rank() << " : Get block " << bid<< " of array " << sip_tables_.array_name(bid.array_id_)<< std::endl);

	IdBlockMapPtr bid_map = block_map_[array_id];
	if (bid_map == NULL) {
		bid_map = new IdBlockMap();
		block_map_[array_id] = bid_map;
	}
	IdBlockMap::iterator it2 = bid_map->find(bid);
	sip::Block::BlockPtr bptr = NULL;
	if (it2 == bid_map->end()) {	// Block requested doesn't exist
		SIP_LOG(std::cout << sip_mpi_attr_.global_rank()<< " : Block did not exist, Creating Empty Block !!"<< std::endl);
		// Initialize and return empty block
		sip::BlockShape shape = sip_tables_.shape(bid);
		bptr = new sip::Block(shape);
		IdBlockMapPtr bid_map = block_map_[array_id];

		std::pair<sip::BlockId, sip::Block::BlockPtr> bid_pair = std::pair<
				sip::BlockId, sip::Block::BlockPtr>(bid, bptr);
		std::pair<IdBlockMap::iterator, bool> ret = bid_map->insert(bid_pair);
		sip::check(ret.second,
				std::string(
						"attempting to create an empty block that already exists"));
	} else { // Block requested exists.
		bptr = it2->second;
	}
	// Send the size, then the block.
	SIPMPIUtils::send_bptr_to_rank(bid, bptr, mpi_source,
			SIPMPIData::SERVER_TO_WORKER_BLOCK_SIZE,
			SIPMPIData::SERVER_TO_WORKER_BLOCK_DATA);
	SIP_LOG(
			std::cout << sip_mpi_attr_.global_rank() << " : Done GET for rank "
					<< mpi_source << std::endl);
}

void SIPServer::handle_PUT(int mpi_source) {
	SIP_LOG(std::cout << sip_mpi_attr_.global_rank()<< " : In PUT at server from rank " << mpi_source<< std::endl);

	// Receive the blockid, size of the block, then the block itself
	sip::BlockId bid;
	sip::BlockShape shape;
	int data_size;
	sip::SIPMPIUtils::get_block_params(mpi_source, sip::SIPMPIData::WORKER_TO_SERVER_BLOCK_SIZE, bid, shape, data_size);

	int array_id = bid.array_id();
	IdBlockMapPtr bid_map = block_map_[array_id];
	if (bid_map == NULL) {
		bid_map = new IdBlockMap();
		block_map_[array_id] = bid_map;
		SIP_LOG(std::cout<<"First PUT into block of array number "<<array_id<<" called "<<sip_tables_.array_name(array_id)<<std::endl);
	}
	IdBlockMap::iterator it = bid_map->find(bid);
	sip::Block::BlockPtr bptr;
	if (it == bid_map->end()){
		bptr = new sip::Block(shape);
	} else {
		bptr = it->second;
	}

	SIPMPIUtils::get_bptr_from_rank(mpi_source, SIPMPIData::WORKER_TO_SERVER_BLOCK_DATA, data_size, bptr);

	// Write into block map
	SIP_LOG(std::cout << sip_mpi_attr_.global_rank() << " : Putting block " << bid << " of array " << sip_tables_.array_name(bid.array_id_) << std::endl);

	bid_map->insert(std::pair<sip::BlockId, sip::Block::BlockPtr>(bid, bptr));

	// Send Ack
	SIPMPIUtils::send_ack_to_rank(mpi_source, SIPMPIData::PUT_ACK, SIPMPIData::SERVER_TO_WORKER_PUT_ACK);

	SIP_LOG(std::cout << sip_mpi_attr_.global_rank() << " : Done PUT for rank "<< mpi_source << std::endl);
}

void SIPServer::handle_PUT_ACCUMULATE(int mpi_source) {
	SIP_LOG(std::cout << sip_mpi_attr_.global_rank()<< " : In PUT_ACCUMULATE at server from rank " << mpi_source<< std::endl);
	// Receive the blockid, size of the block, then the block itself

	// Receive the blockid, size of the block, then the block itself
	sip::BlockId bid;
	sip::BlockShape shape;
	int data_size;
	sip::SIPMPIUtils::get_block_params(mpi_source, sip::SIPMPIData::WORKER_TO_SERVER_BLOCK_SIZE, bid, shape, data_size);

	int array_id = bid.array_id();
	IdBlockMapPtr bid_map = block_map_[array_id];
	if (bid_map == NULL) {
		bid_map = new IdBlockMap();
		block_map_[array_id] = bid_map;
		SIP_LOG(std::cout<<"First PUT_ACCUMULATE into block of array number "<<array_id<<" called "<<sip_tables_.array_name(array_id)<<std::endl);

	}
	IdBlockMap::iterator it = bid_map->find(bid);
	sip::Block::BlockPtr bptr = new sip::Block(shape);

	SIPMPIUtils::get_bptr_from_rank(mpi_source, SIPMPIData::WORKER_TO_SERVER_BLOCK_DATA, data_size, bptr);

	IdBlockMap::iterator it2 = bid_map->find(bid);
	if (it2 == bid_map->end()) {
		bid_map->insert(std::pair<sip::BlockId, sip::Block::BlockPtr>(bid, bptr));
	} else {
		sip::Block::BlockPtr bptr_orig = it2->second;
		sip::check(bptr->size() == bptr_orig->size(), "Size of block being accumulated into doesnt match incoming block!");
		bptr_orig->accumulate_data(bptr);
		delete bptr;
	}

	// Send Ack
	SIPMPIUtils::send_ack_to_rank(mpi_source, SIPMPIData::PUT_ACCUMULATE_ACK, SIPMPIData::SERVER_TO_WORKER_PUT_ACCUMULATE_ACK);

	SIP_LOG(std::cout << sip_mpi_attr_.global_rank()<< " : Done PUT_ACCUMULATE for rank " << mpi_source<< std::endl);
}

void SIPServer::handle_DELETE(int mpi_source) {
	// Worker master sends a delete to server master. The server master sends it to other servers.
	SIP_LOG(
			std::cout << sip_mpi_attr_.global_rank()
					<< " : In DELETE at server " << std::endl);
	sip::check(sip_mpi_attr_.is_company_master(),
			" Delete sent to server other than a server master !");
	// From the servers master
	// Receieve the ID of the array to delete
	// Inform other servers
	int array_id = get_array_id(mpi_source);
	if (sip_mpi_attr_.num_servers() > 1) {
		int to_send1 = SIPMPIData::SERVER_DELETE;
		int tag1 = SIPMPIData::SERVER_TO_SERVER_DELETE;
		int to_send2 = array_id;
		int tag2 = SIPMPIData::SERVER_TO_SERVER_DELETE_ARRAY_ID;
		//Send SERVER_DELETE & array_id to other servers.
		send_to_other_servers(to_send1, tag1);
		send_to_other_servers(to_send2, tag2);
	}
	// Get rid of blocks from given array
	delete_array(array_id);

	// Send Ack
	SIPMPIUtils::send_ack_to_rank(mpi_source, SIPMPIData::DELETE_ACK, SIPMPIData::SERVER_TO_WORKER_DELETE_ACK);


	SIP_LOG(
			std::cout << sip_mpi_attr_.global_rank() << " : Done with DELETE "
					<< std::endl);
}

//void SIPServer::handle_BARRIER() {
//	SIP_LOG(std::cout << sip_mpi_attr_.global_rank() << " : In BARRIER at server " << std::endl);
//	// Worker master sends a barrier to server master. The server master sends it to other servers.
//	// Then all the workers and servers go into a global barrier.
//	sip::check(sip_mpi_attr_.is_company_master(),
//			" Barrier sent to server other than a server master !");
//	//Send SERVER_BARRIER to other servers.
//	if (sip_mpi_attr_.num_servers() > 1) {
//		int to_send = SIPMPIData::SERVER_BARRIER;
//		int tag = SIPMPIData::SERVER_TO_SERVER_BARRIER;
//		send_to_other_servers(to_send, tag);
//	}
//	sip::SIPMPIUtils::check_err(MPI_Barrier(sip_mpi_attr_.company_communicator()));
//
//	// Global Barrier
//	sip::SIPMPIUtils::check_err(MPI_Barrier(MPI_COMM_WORLD));
//	SIP_LOG(
//			std::cout << sip_mpi_attr_.global_rank() << " : Done with BARRIER "
//					<< std::endl);
//}

void SIPServer::handle_END_PROGRAM() {
	// Worker master sends a END_PROGRAM to server master. The server master sends it to other servers.
	// All servers return from this method.
	SIP_LOG(
			std::cout << sip_mpi_attr_.global_rank()
					<< " : In END_PROGRAM at server " << std::endl);
	sip::check(sip_mpi_attr_.is_company_master(),
			" End program sent to server other than a server master !");
	if (sip_mpi_attr_.num_servers() > 1) {
		int to_send = SIPMPIData::SERVER_END_PROGRAM;
		int tag = SIPMPIData::SERVER_TO_SERVER_END_PROGRAM;
		send_to_other_servers(to_send, tag);
	}
	post_program_processing();
	SIP_LOG(
			std::cout << sip_mpi_attr_.global_rank()
					<< " : Done with END_PROGRAM " << std::endl);
}

void SIPServer::handle_SERVER_DELETE(int mpi_source) {
	// Get rid of blocks from given array. Must not be the server master itself.
	sip::check(!sip_mpi_attr_.is_company_master(),
			" SERVER_DELETE sent to master server!");
	SIP_LOG(
			std::cout << sip_mpi_attr_.global_rank()
					<< " : Got SERVER_DELETE from server master " << std::endl);
	int array_id = get_array_id(mpi_source);
	delete_array(array_id);
	SIP_LOG(
			std::cout << sip_mpi_attr_.global_rank()
					<< " : Done with SERVER_DELETE " << std::endl);
}

void SIPServer::handle_SERVER_END_PROGRAM() {
	SIP_LOG(
			std::cout << sip_mpi_attr_.global_rank()
					<< " : Got SERVER_END_PROGRAM from server master "
					<< std::endl);
	post_program_processing();
	SIP_LOG(
			std::cout << sip_mpi_attr_.global_rank()
					<< " : Done with SERVER_END_PROGRAM " << std::endl);
}

void SIPServer::handle_SERVER_BARRIER() {
	// Global Barrier
	SIP_LOG(
			std::cout << sip_mpi_attr_.global_rank()
					<< " : Got SERVER_BARRIER from server master "
					<< std::endl);
	sip::SIPMPIUtils::check_err(MPI_Barrier(sip_mpi_attr_.company_communicator()));
	sip::SIPMPIUtils::check_err(MPI_Barrier(MPI_COMM_WORLD));
	SIP_LOG(
			std::cout << sip_mpi_attr_.global_rank()
					<< " : Done with SERVER_BARRIER " << std::endl);
}

void SIPServer::handle_SAVE_PERSISTENT(int mpi_source) {
	// Worker master sends SAVE_PERSISTENT to server master. Server master sends it to other servers.
	// All servers save given array as persistent array.
	sip::check(sip_mpi_attr_.is_company_master(),
			" SAVE_PERSISTENT sent to server other than a server master !");
	SIP_LOG(
			std::cout << sip_mpi_attr_.global_rank()
					<< " : Got SAVE_PERSISTENT " << std::endl);
	int to_send1 = SIPMPIData::SERVER_SAVE_PERSISTENT;
	int tag1 = SIPMPIData::SERVER_TO_SERVER_SAVE_PERSISTENT;
	int array_id = get_array_id(mpi_source);
	int to_send2 = array_id;
	int tag2 = SIPMPIData::SERVER_TO_SERVER_SAVE_PERSISTENT_ARRAY_ID;
	int label_len;
	std::string array_label = get_string_from(mpi_source, label_len);
	int tag3 = SIPMPIData::SERVER_TO_SERVER_SAVE_PERSISTENT_ARRAY_IDENT;
	// Send SAVE_PERSISTENT, array_id & array_ident to other servers.
	send_to_other_servers(to_send1, tag1);
	send_to_other_servers(to_send2, tag2);
	send_to_other_servers(array_label.c_str(), label_len, tag3);
	// mark persistent array
	pbm_write_.mark_persistent_array(array_id, array_label);

	// Send Ack
	SIPMPIUtils::send_ack_to_rank(mpi_source, SIPMPIData::SAVE_PERSISTENT_ACK, SIPMPIData::SERVER_TO_WORKER_SAVE_PERSISTENT_ACK);

	SIP_LOG(
			std::cout << sip_mpi_attr_.global_rank()
					<< " : Done with SAVE_PERSISTENT for array "<<array_id<<" with label "  <<array_label << std::endl);
}

void SIPServer::handle_RESTORE_PERSISTENT(int mpi_source) {
	sip::check(sip_mpi_attr_.is_company_master(),
			" RESTORE_PERSISTENT sent to server other than a server master !");
	SIP_LOG(
			std::cout << sip_mpi_attr_.global_rank()
					<< " : Got RESTORE_PERSISTENT " << std::endl);
	int array_id = get_array_id(mpi_source);
	int label_len;
	std::string array_label = get_string_from(mpi_source, label_len);
	// Send SAVE_PERSISTENT, array_id & array_ident to other servers.
	send_to_other_servers(SIPMPIData::SERVER_RESTORE_PERSISTENT,
			SIPMPIData::SERVER_TO_SERVER_RESTORE_PERSISTENT);
	send_to_other_servers(array_id,
			SIPMPIData::SERVER_TO_SERVER_RESTORE_PERSISTENT_ARRAY_ID);
	send_to_other_servers(array_label.c_str(), label_len,
			SIPMPIData::SERVER_TO_SERVER_RESTORE_PERSISTENT_ARRAY_IDENT);
	// restore persistent array
	restore_persistent_array(array_label, array_id);

	// Send Ack
	SIPMPIUtils::send_ack_to_rank(mpi_source, SIPMPIData::RESTORE_PERSISTENT_ACK, SIPMPIData::SERVER_TO_WORKER_RESTORE_PERSISTENT_ACK);

	SIP_LOG(
			std::cout << sip_mpi_attr_.global_rank()
					<< " : Done with RESTORE_PERSISTENT for array "<<array_label << std::endl);
}

void SIPServer::handle_SERVER_SAVE_PERSISTENT(int mpi_source) {
	SIP_LOG(
			std::cout << sip_mpi_attr_.global_rank()
					<< " : Got SERVER_SAVE_PERSISTENT from server master "
					<< std::endl);
	// Get array id & array_ident from SERVER master
	int array_id = get_array_id(mpi_source);
	int label_len;
	std::string array_ident_str = get_string_from(mpi_source, label_len);
	// mark persistent array
	pbm_write_.mark_persistent_array(array_id, array_ident_str);
	SIP_LOG(
			std::cout << sip_mpi_attr_.global_rank()
					<< " : Done with SERVER_SAVE_PERSISTENT for array "<< array_id << " with label "<<array_ident_str<< std::endl);
}

void SIPServer::handle_SERVER_RESTORE_PERSISTENT(int mpi_source) {
	SIP_LOG(
			std::cout << sip_mpi_attr_.global_rank()
					<< " : Got SERVER_RESTORE_PERSISTENT from server master "
					<< std::endl);
	// Get array id & array_ident from SERVER master
	int array_id = get_array_id(mpi_source);
	int label_len;
	std::string array_ident_str = get_string_from(mpi_source, label_len);
	// restore persistent array
	restore_persistent_array(array_ident_str, array_id);
	SIP_LOG(
			std::cout << sip_mpi_attr_.global_rank()
					<< " : Done with SERVER_RESTORE_PERSISTENT for array " << array_ident_str << std::endl);
}

void SIPServer::run(){

	while (true){

		// Receive a message from either a server or a worker.
		// The worker can ask to
		//		get a block,
		//		put a block,
		//		put accumulate a block,
		//		do a barrier,
		//		do a save array
		// 		do a restore array
		//		end the current program


		int received;
		MPI_Status status;
		sip::SIPMPIUtils::check_err(MPI_Recv(&received, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status));
		int tlevel_size_;
		sip::SIPMPIUtils::check_err(MPI_Get_count(&status, MPI_INT, &tlevel_size_));
		sip::check(1 == tlevel_size_, "Received more than 1 byte at SIP Server !");
		int mpi_source = status.MPI_SOURCE;

		bool correct_tag = status.MPI_TAG == SIPMPIData::WORKER_TO_SERVER_MESSAGE ||
				//status.MPI_TAG == SIPMPIData::SERVER_TO_SERVER_BARRIER ||
				status.MPI_TAG == SIPMPIData::SERVER_TO_SERVER_DELETE ||
				status.MPI_TAG == SIPMPIData::SERVER_TO_SERVER_END_PROGRAM ||
				status.MPI_TAG == SIPMPIData::SERVER_TO_SERVER_SAVE_PERSISTENT ||
				status.MPI_TAG == SIPMPIData::SERVER_TO_SERVER_RESTORE_PERSISTENT;
		sip::check(correct_tag, "Server got message with incorrect tag ! : " + status.MPI_TAG);

		switch (received) {

		case SIPMPIData::GET : {

			handle_GET(mpi_source);
		}
			break;
		case SIPMPIData::PUT : {

			handle_PUT(mpi_source);
		}
			break;
		case SIPMPIData::PUT_ACCUMULATE: {

			handle_PUT_ACCUMULATE(mpi_source);
		}
			break;
		case SIPMPIData::CREATE:{
			// NO OP - Create blocks on the fly.
		}
			break;
		case SIPMPIData::DELETE:{
			// Worker master sends a delete to server master. The server master sends it to other servers.

			handle_DELETE(mpi_source);
		}
			break;
//		case SIPMPIData::BARRIER:{
//
//			handle_BARRIER();
//		}
//			break;
		case SIPMPIData::END_PROGRAM: {
			// Worker master sends a END_PROGRAM to server master. The server master sends it to other servers.
			// All servers return from this method.

			handle_END_PROGRAM();
			return;
		}
			break;
		case SIPMPIData::SERVER_DELETE:{
			// Get rid of blocks from given array. Must not be the server master itself.
			handle_SERVER_DELETE(mpi_source);
		}
			break;
		case SIPMPIData::SERVER_END_PROGRAM:{
			handle_SERVER_END_PROGRAM();
			return;
		}
			break;
//		case SIPMPIData::SERVER_BARRIER:{
//			// Global Barrier
//			handle_SERVER_BARRIER();
//		}
//			break;

		case SIPMPIData::SAVE_PERSISTENT:{
			// Worker master sends SAVE_PERSISTENT to server master. Server master sends it to other servers.
			// All servers save given array as persistent array.
			handle_SAVE_PERSISTENT(mpi_source);
		}
			break;

		case SIPMPIData::RESTORE_PERSISTENT:{

			handle_RESTORE_PERSISTENT(mpi_source);
		}
			break;

		case SIPMPIData::SERVER_SAVE_PERSISTENT:{

			handle_SERVER_SAVE_PERSISTENT(mpi_source);
		}
			break;

		case SIPMPIData::SERVER_RESTORE_PERSISTENT:{

			handle_SERVER_RESTORE_PERSISTENT(mpi_source);
		}
			break;
		default:
			sip::fail("Received unexpected message in SIP Server !");
			break;
		}

	}

}

std::ostream& operator<<(std::ostream& os, const SIPServer& obj) {
	os << "block_map_:" << std::endl;
	{
		int num_arrs = obj.block_map_.size();
		for (int i = 0; i < num_arrs; i++) {
			if (obj.block_map_[i] != NULL) {
				os << "array [" << i << "]" << std::endl; //print the array id
				const SIPServer::IdBlockMapPtr bid_map = obj.block_map_[i];
				SIPServer::IdBlockMap::const_iterator it2;
				for (it2 = bid_map->begin(); it2 != bid_map->end(); ++it2) {
					sip::check(it2->second != NULL,
							"Trying to print NULL blockPtr !");
					SIP_LOG(if(it2->second == NULL) std::cout<<it2->first<<" is NULL!"<<std::endl;);
					os << it2->first << std::endl;
					os << *(it2->second) << std::endl;
				}
				os << std::endl;
			}
		}
	}
	return os;
}

} /* namespace sip */
