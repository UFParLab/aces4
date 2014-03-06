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
	// Only delete if the block map contains this array.
	IdBlockMapPtr bid_map = block_map_[array_id];
	if (bid_map != NULL){
		for (IdBlockMap::iterator it = bid_map->begin(); it != bid_map->end(); ++it) {
			sip::Block::BlockPtr bptr = it->second;
			delete bptr;
		}
		delete bid_map;
		block_map_[array_id] = NULL;
	}
}

int SIPServer::get_int_from_rank(const int rank) {
	int data = -1;
	MPI_Status status;
	sip::SIPMPIUtils::check_err(MPI_Recv(&data, 1, MPI_INT, rank, MPI_ANY_TAG, MPI_COMM_WORLD, &status));
	int msg_size_;
	sip::SIPMPIUtils::check_err(MPI_Get_count(&status, MPI_INT, &msg_size_));
	sip::check(1 == msg_size_, "Received more than 1 byte at SIP Server !");
	return data;
}

//std::string SIPServer::get_string_from(int rank, int &size) {
//	MPI_Status str_status;
//	char str[SIPMPIData::MAX_STRING];
//	sip::SIPMPIUtils::check_err(MPI_Recv(str, SIPMPIData::MAX_STRING, MPI_CHAR, rank, MPI_ANY_TAG,
//			MPI_COMM_WORLD, &str_status));
//	int len = 0;
//	sip::SIPMPIUtils::check_err(MPI_Get_count(&str_status, MPI_CHAR, &len));
//	size = len;
//	std::string recvd_string = std::string(str);
//	SIP_LOG(std::cout << sip_mpi_attr_.global_rank() << " : Got String "<<" from rank "<<str_status.MPI_SOURCE<<std::endl);
//	return recvd_string;
//}

void SIPServer::post_program_processing() {

	// Save all the arrays marked as persistent
	int num_arrs = block_map_.size();
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

		std::pair<sip::BlockId, sip::Block::BlockPtr> bid_pair = std::pair<sip::BlockId, sip::Block::BlockPtr>(bid, bptr);
		std::pair<IdBlockMap::iterator, bool> ret = bid_map->insert(bid_pair);
		sip::check(ret.second, "attempting to create an empty block that already exists");
	} else { // Block requested exists.
		bptr = it2->second;
	}
	// Send the size, then the block.
	SIPMPIUtils::send_bptr_to_rank(bid, bptr, mpi_source, SIPMPIData::SERVER_TO_WORKER_BLOCK_SIZE, SIPMPIData::SERVER_TO_WORKER_BLOCK_DATA);
	SIP_LOG(std::cout << sip_mpi_attr_.global_rank() << " : Done GET for rank "	<< mpi_source << std::endl);
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
	// Message sent to all servers from their corresponding local workers
	SIP_LOG(std::cout << sip_mpi_attr_.global_rank()<< " : In DELETE at server " << std::endl);

	// From the servers master
	// Receieve the ID of the array to delete
	// Inform other servers
	int array_id = get_int_from_rank(mpi_source);

	// Get rid of blocks from given array
	delete_array(array_id);

	// Send Ack
	SIPMPIUtils::send_ack_to_rank(mpi_source, SIPMPIData::DELETE_ACK, SIPMPIData::SERVER_TO_WORKER_DELETE_ACK);

	SIP_LOG(std::cout << sip_mpi_attr_.global_rank() << " : Done with DELETE "<< std::endl);
}


void SIPServer::handle_END_PROGRAM() {
	// Message sent to all servers from their corresponding local workers
	// All servers return from this method.
	SIP_LOG(std::cout << sip_mpi_attr_.global_rank()<< " : In END_PROGRAM at server " << std::endl);
	post_program_processing();
	SIP_LOG(std::cout << sip_mpi_attr_.global_rank()<< " : Done with END_PROGRAM " << std::endl);
}


void SIPServer::handle_SAVE_PERSISTENT(int mpi_source) {
	// Message sent to all servers from their corresponding local workers
	// All servers save given array as persistent array.
	SIP_LOG(std::cout << sip_mpi_attr_.global_rank()<< " : Got SAVE_PERSISTENT " << std::endl);
	int array_id = get_int_from_rank(mpi_source);
	int label_slot = get_int_from_rank(mpi_source);
	std::string array_label = sip_tables_.string_literal(label_slot);

	// mark persistent array
	pbm_write_.mark_persistent_array(array_id, array_label);

	// Send Ack
	SIPMPIUtils::send_ack_to_rank(mpi_source, SIPMPIData::SAVE_PERSISTENT_ACK, SIPMPIData::SERVER_TO_WORKER_SAVE_PERSISTENT_ACK);

	SIP_LOG(std::cout << sip_mpi_attr_.global_rank()<< " : Done with SAVE_PERSISTENT for array "<<array_id<<" with label "  <<array_label << std::endl);
}

void SIPServer::handle_RESTORE_PERSISTENT(int mpi_source) {
	// Message sent to all servers from their corresponding local workers
	SIP_LOG(std::cout << sip_mpi_attr_.global_rank()<< " : Got RESTORE_PERSISTENT " << std::endl);
	int array_id = get_int_from_rank(mpi_source);
	int label_slot = get_int_from_rank(mpi_source);
	std::string array_label = sip_tables_.string_literal(label_slot);
	restore_persistent_array(array_label, array_id);

	// Send Ack
	SIPMPIUtils::send_ack_to_rank(mpi_source, SIPMPIData::RESTORE_PERSISTENT_ACK, SIPMPIData::SERVER_TO_WORKER_RESTORE_PERSISTENT_ACK);

	SIP_LOG(std::cout << sip_mpi_attr_.global_rank()<< " : Done with RESTORE_PERSISTENT for array "<<array_label << std::endl);
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

		bool correct_tag = status.MPI_TAG == SIPMPIData::WORKER_TO_SERVER_MESSAGE;
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
			handle_DELETE(mpi_source);
		}
			break;

		case SIPMPIData::END_PROGRAM: {
			handle_END_PROGRAM();
			return;
		}
			break;

		case SIPMPIData::SAVE_PERSISTENT:{
			handle_SAVE_PERSISTENT(mpi_source);
		}
			break;

		case SIPMPIData::RESTORE_PERSISTENT:{
			handle_RESTORE_PERSISTENT(mpi_source);
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
