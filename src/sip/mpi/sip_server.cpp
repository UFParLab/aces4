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
#include <sstream>

namespace sip {

SIPServer::SIPServer(SipTables& sip_tables, DataDistribution &data_distribution, SIPMPIAttr & sip_mpi_attr,
		PersistentArrayManager& pbm_read, PersistentArrayManager& pbm_write):
		sip_tables_(sip_tables), data_distribution_(data_distribution), sip_mpi_attr_(sip_mpi_attr),
		pbm_read_(pbm_read), pbm_write_(pbm_write), block_map_(sip_tables.num_arrays()), section_number_(0)	{
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
			Block::BlockPtr bptr = it->second;
			delete bptr;
		}
		delete bid_map;
		block_map_[array_id] = NULL;
	}
}

int SIPServer::get_int_from_rank(const int rank, int tag) {
	int data = -1;
	MPI_Status status;
	SIPMPIUtils::check_err(MPI_Recv(&data, 1, MPI_INT, rank, tag, MPI_COMM_WORLD, &status));
	return data;
}

//std::string SIPServer::get_string_from(int rank, int &size) {
//	MPI_Status str_status;
//	char str[SIPMPIData::MAX_STRING];
//	SIPMPIUtils::check_err(MPI_Recv(str, SIPMPIData::MAX_STRING, MPI_CHAR, rank, MPI_ANY_TAG,
//			MPI_COMM_WORLD, &str_status));
//	int len = 0;
//	SIPMPIUtils::check_err(MPI_Get_count(&str_status, MPI_CHAR, &len));
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
	BlockManager::IdBlockMapPtr bid_map = pbm_read_.get_saved_dist_array(label);
	BlockManager::IdBlockMap *old_bid_map = block_map_[array_id];
	if (old_bid_map != NULL){
		for (BlockManager::IdBlockMap::iterator it = old_bid_map->begin(); it != old_bid_map->end(); ++it){
			delete it->second;
		}
		delete block_map_[array_id];
	}

	block_map_[array_id] = new IdBlockMap();

	// Put in new blocks.
	if (bid_map != NULL){
	for (BlockManager::IdBlockMap::iterator it = bid_map->begin(); it != bid_map->end(); ++it){
		BlockId bid(it->first);
		bid.array_id_ = array_id;
		Block::BlockPtr bptr_clone = it->second->clone();
		block_map_[array_id]->insert(std::pair<BlockId, Block::BlockPtr>(bid, bptr_clone));
	}
	}else {
		SIP_LOG(std::cout << sip_mpi_attr_.global_rank() << " : Trying to restore empty map with label : "<<label<<std::endl);
	}
}

void SIPServer::handle_GET(int rank, int tag) {
	SIP_LOG(std::cout << sip_mpi_attr_.global_rank()<< " : In GET at server from rank " << rank<< std::endl);

	// Receive a message with the block being requested - blockid
	BlockId bid = SIPMPIUtils::get_block_id_from_rank(rank, tag);
	int array_id = bid.array_id();
	SIP_LOG(std::cout << sip_mpi_attr_.global_rank() << " : Get block " << bid<< " of array " << sip_tables_.array_name(bid.array_id_)<< std::endl);

	int message_number = SIPMPIUtils::get_message_number(tag);
	int section_number = SIPMPIUtils::get_section_number(tag);
	check (section_number >= this->section_number_, "Section number invariant violated. Got request from an older section !");
	this->section_number_ = section_number;

	IdBlockMapPtr bid_map = block_map_[array_id];
	if (bid_map == NULL) {
		bid_map = new IdBlockMap();
		block_map_[array_id] = bid_map;
	}
	IdBlockMap::const_iterator it2 = bid_map->find(bid);
	Block::BlockPtr bptr = NULL;
	if (it2 == bid_map->end()) {	// Block requested doesn't exist
		SIP_LOG(std::cout << sip_mpi_attr_.global_rank()<< " : Block did not exist, Creating Empty Block !!"<< std::endl);
		// Initialize and return empty block
		BlockShape shape = sip_tables_.shape(bid);
		bptr = new Block(shape);
		IdBlockMapPtr bid_map = block_map_[array_id];

		std::pair<BlockId, Block::BlockPtr> bid_pair(bid, bptr);
		std::pair<IdBlockMap::iterator, bool> ret = bid_map->insert(bid_pair);
		check(ret.second, "attempting to create an empty block that already exists");
	} else { // Block requested exists.
		bptr = it2->second;
	}
	// Send the size & the block.
	int get_reply_tag = SIPMPIUtils::make_mpi_tag(SIPMPIData::GET_DATA, this->section_number_, message_number);
	MPI_Request request = SIPMPIUtils::isend_block_data_to_rank(bptr->data_, bptr->size(), rank, get_reply_tag);
	SIP_LOG(std::cout << sip_mpi_attr_.global_rank() << " : Done GET for rank "	<< rank << std::endl);

//	MPI_Status status;
//	SIPMPIUtils::check_err(MPI_Wait(&request, &status));

}

void SIPServer::handle_PUT(int mpi_source, int tag) {
	SIP_LOG(std::cout << sip_mpi_attr_.global_rank()<< " : In PUT at server from rank " << mpi_source<< std::endl);

	// Receive the blockid, size of the block, then the block itself
	BlockId bid;
	BlockShape shape;
	int data_size;
	SIPMPIUtils::get_block_params(mpi_source, tag, &bid, &shape, &data_size);
	int section_number = SIPMPIUtils::get_section_number(tag);
	int message_number = SIPMPIUtils::get_message_number(tag);
	check (section_number >= this->section_number_, "Section number invariant violated. Got request from an older section !");
	this->section_number_ = section_number;

	int array_id = bid.array_id();
	IdBlockMapPtr bid_map = block_map_[array_id];
	if (bid_map == NULL) {
		bid_map = new IdBlockMap();
		block_map_[array_id] = bid_map;
		SIP_LOG(std::cout<<"First PUT into block of array number "<<array_id<<" called "<<sip_tables_.array_name(array_id)<<std::endl);
	}
	IdBlockMap::const_iterator it = bid_map->find(bid);
	Block::BlockPtr bptr;
	if (it == bid_map->end()){
		bptr = new Block(shape);
	} else {
		bptr = it->second;
	}

	//SIPMPIUtils::get_bptr_from_rank(mpi_source, SIPMPIData::WORKER_TO_SERVER_BLOCK_DATA, data_size, bptr);

	// Write into block map
	//SIP_LOG(std::cout << sip_mpi_attr_.global_rank() << " : Putting block " << bid << " of array " << sip_tables_.array_name(bid.array_id_) << std::endl);

	SIP_LOG(std::cout << sip_mpi_attr_.global_rank() << " : Putting empty block " << bid << " of array " << sip_tables_.array_name(bid.array_id_) << std::endl);
	bid_map->insert(std::pair<BlockId, Block::BlockPtr>(bid, bptr));

	TagInfo ti(mpi_source, section_number_, message_number);
	std::pair<BlockId, Block::BlockPtr> id_bptr_pair (bid, bptr);
	std::pair<TagInfoIdBlockPairMap::iterator, bool> outstanding_insert;
	outstanding_insert = outstanding_put_data_map_.insert(std::pair<TagInfo, IdBlockPair>(ti, id_bptr_pair));
	sip::check(outstanding_insert.second, "Could not insert into outstanding asyncs map !");

	// Send Ack
	//SIPMPIUtils::send_ack_to_rank(mpi_source, SIPMPIData::PUT_ACK, SIPMPIData::SERVER_TO_WORKER_PUT_ACK);

	SIP_LOG(std::cout << sip_mpi_attr_.global_rank() << " : Done PUT for rank "<< mpi_source << std::endl);
}


void SIPServer::handle_PUT_DATA(int mpi_source, int size, int tag) {
	SIP_LOG(std::cout << sip_mpi_attr_.global_rank()<< " : In PUT_DATA at server from rank " << mpi_source<< std::endl);

	// Receive the block data
	int section_number = SIPMPIUtils::get_section_number(tag);
	int message_number = SIPMPIUtils::get_message_number(tag);
	check (section_number >= this->section_number_, "Section number invariant violated. Got request from an older section !");
	this->section_number_ = section_number;
//std::cout<<"\n section & message number : "<<section_number<<"\t"<<message_number<<"\n";

	TagInfo ti(mpi_source, section_number_, message_number);
	//==============//TagInfoIdBlockPairMap::const_iterator it = outstanding_put_data_map_.find(ti);
	TagInfoIdBlockPairMap::const_iterator it = outstanding_put_data_map_.begin();
	for (; it != outstanding_put_data_map_.end(); ++it){
		if (it->first == ti)
			break;
	}
	std::ostringstream err_no_id;
	err_no_id << "Could not find block in which to insert incoming block data, could not find "<< ti << " in map " << outstanding_put_data_map_<< " !";
	check(it != outstanding_put_data_map_.end(), err_no_id.str());
//check (outstanding_put_data_map_.size() < 2, "MATCHED GREATER THAN 2 !");

	BlockId bid = it->second.first;
	Block::BlockPtr bptr = it->second.second;

	std::ostringstream err_message;
	err_message << "Size of incoming block ("<<size<<") and that expected ("<<bptr->size()<< ")don't match up !";
	check(size == bptr->size(), err_message.str());
	//check(size == bptr->size(), "Size of incoming block and that expected don't match up !");
	SIPMPIUtils::get_bptr_data_from_rank(mpi_source, tag, size, bptr);
	SIP_LOG(std::cout << sip_mpi_attr_.global_rank() << " : Putting received block data" << bid << " of array " << sip_tables_.array_name(bid.array_id_) << std::endl);

	// Send Ack
	int put_data_ack_tag = SIPMPIUtils::make_mpi_tag(SIPMPIData::PUT_DATA_ACK, section_number, message_number);
	SIPMPIUtils::send_ack_to_rank(mpi_source, SIPMPIData::PUT_DATA_ACK, put_data_ack_tag);

	outstanding_put_data_map_.erase(ti);
	SIP_LOG(std::cout << sip_mpi_attr_.global_rank() << " : Done PUT_DATA for rank "<< mpi_source << std::endl);
}

void SIPServer::handle_PUT_ACCUMULATE(int mpi_source, int tag) {
	SIP_LOG(std::cout << sip_mpi_attr_.global_rank()<< " : In PUT_ACCUMULATE at server from rank " << mpi_source<< std::endl);

	// Receive the blockid, size of the block, then the block itself
	BlockId bid;
	BlockShape shape;
	int data_size;
	SIPMPIUtils::get_block_params(mpi_source, tag, &bid, &shape, &data_size);
	int section_number = SIPMPIUtils::get_section_number(tag);
	int message_number = SIPMPIUtils::get_message_number(tag);
	check (section_number >= this->section_number_, "Section number invariant violated. Got request from an older section !");
	this->section_number_ = section_number;

	int array_id = bid.array_id();
	IdBlockMapPtr bid_map = block_map_[array_id];
	if (bid_map == NULL) {
		bid_map = new IdBlockMap();
		block_map_[array_id] = bid_map;
		SIP_LOG(std::cout<<"First PUT_ACCUMULATE into block of array number "<<array_id<<" called "<<sip_tables_.array_name(array_id)<<std::endl);

	}
	IdBlockMap::const_iterator it = bid_map->find(bid);
	Block::BlockPtr bptr = new Block(shape);


	TagInfo ti(mpi_source, section_number_, message_number);
	std::pair<BlockId, Block::BlockPtr> id_bptr_pair(bid, bptr);
	std::pair<TagInfoIdBlockPairMap::iterator, bool> outstanding_insert;
	outstanding_insert = outstanding_put_data_map_.insert(std::pair<TagInfo, IdBlockPair>(ti, id_bptr_pair));
	sip::check(outstanding_insert.second, "Could not insert into outstanding asyncs !");

	SIP_LOG(std::cout << sip_mpi_attr_.global_rank()<< " : Done PUT_ACCUMULATE for rank " << mpi_source<< std::endl);
}

void SIPServer::handle_PUT_ACCUMULATE_DATA(int mpi_source, int size, int tag) {
	SIP_LOG(std::cout << sip_mpi_attr_.global_rank()<< " : In PUT_ACCUMULATE_DATA at server from rank " << mpi_source<< std::endl);

	// Receive the block data
	int section_number = SIPMPIUtils::get_section_number(tag);
	int message_number = SIPMPIUtils::get_message_number(tag);
	check (section_number >= this->section_number_, "Section number invariant violated. Got request from an older section !");
	this->section_number_ = section_number;

	TagInfo ti(mpi_source, section_number_, message_number);
	//==============//TagInfoIdBlockPairMap::const_iterator it = outstanding_put_data_map_.find(ti);
	TagInfoIdBlockPairMap::const_iterator it = outstanding_put_data_map_.begin();
	for (; it != outstanding_put_data_map_.end(); ++it){
		if (it->first == ti)
			break;
	}
	std::ostringstream err_no_id;
	err_no_id << "Could not find block in which to insert incoming block data, could not find "<< ti << " in map " << outstanding_put_data_map_<< " !";
	check(it != outstanding_put_data_map_.end(), err_no_id.str());
//check (outstanding_put_data_map_.size() < 2, "MATCHED GREATER THAN 2 !");
	BlockId bid = it->second.first;
	Block::BlockPtr bptr = it->second.second;

	std::ostringstream err_message;
	err_message << "Size of incoming block ("<<size<<") and that expected ("<<bptr->size()<< ")don't match up !";
	check(size == bptr->size(), err_message.str());
	//check(size == bptr->size(), "Size of incoming block and that expected don't match up !");
	SIPMPIUtils::get_bptr_data_from_rank(mpi_source, tag, size, bptr);

	int array_id = bid.array_id();
	IdBlockMapPtr bid_map = block_map_[array_id];
	check(bid_map != NULL, "Map for blocks of array is NULL !");
	IdBlockMap::const_iterator it2 = bid_map->find(bid);
	if (it2 == bid_map->end()) {
		bid_map->insert(std::pair<BlockId, Block::BlockPtr>(bid, bptr));
	} else {
		Block::BlockPtr bptr_orig = it2->second;
		check(bptr->size() == bptr_orig->size(), "Size of block being accumulated into doesnt match incoming block!");
		bptr_orig->accumulate_data(bptr);
		delete bptr;
	}

	SIP_LOG(std::cout << sip_mpi_attr_.global_rank() << " : Putting received block data" << bid << " of array " << sip_tables_.array_name(bid.array_id_) << std::endl);

	// Send Ack
	int put_data_ack_tag = SIPMPIUtils::make_mpi_tag(SIPMPIData::PUT_ACCUMULATE_DATA_ACK, section_number, message_number);
	SIPMPIUtils::send_ack_to_rank(mpi_source, SIPMPIData::PUT_ACCUMULATE_DATA_ACK, put_data_ack_tag);

	SIP_LOG(std::cout << sip_mpi_attr_.global_rank() << " : Done PUT_ACCUMULATE_DATA for rank "<< mpi_source << std::endl);

	outstanding_put_data_map_.erase(ti);
}

void SIPServer::handle_DELETE(int mpi_source, int tag) {
	// Message sent to all servers from their corresponding local workers
	SIP_LOG(std::cout << sip_mpi_attr_.global_rank()<< " : In DELETE at server " << std::endl);

	// From the servers master
	// Receieve the ID of the array to delete
	// Inform other servers
//	int tag;
	int array_id = get_int_from_rank(mpi_source, tag);
	int section_number = SIPMPIUtils::get_section_number(tag);
	int message_number = SIPMPIUtils::get_message_number(tag);
	check (section_number >= this->section_number_, "Section number invariant violated. Got request from an older section !");
	this->section_number_ = section_number;

	// Get rid of blocks from given array
	delete_array(array_id);

	// Send Ack
	int delete_data_ack_tag = SIPMPIUtils::make_mpi_tag(SIPMPIData::DELETE_ACK, section_number, message_number);
	SIPMPIUtils::send_ack_to_rank(mpi_source, SIPMPIData::DELETE_ACK, delete_data_ack_tag);

	SIP_LOG(std::cout << sip_mpi_attr_.global_rank() << " : Done with DELETE "<< std::endl);
}


void SIPServer::handle_END_PROGRAM(int mpi_source, int tag) {
	// Message sent to all servers from their corresponding local workers
	// All servers return from this method.
	int dummy = get_int_from_rank(mpi_source, tag);
	SIP_LOG(std::cout << sip_mpi_attr_.global_rank()<< " : In END_PROGRAM at server " << std::endl);
	post_program_processing();
	SIP_LOG(std::cout << sip_mpi_attr_.global_rank()<< " : Done with END_PROGRAM " << std::endl);
}


void SIPServer::handle_SAVE_PERSISTENT(int mpi_source, int tag) {
	// Message sent to all servers from their corresponding local workers
	// All servers save given array as persistent array.
	SIP_LOG(std::cout << sip_mpi_attr_.global_rank()<< " : Got SAVE_PERSISTENT " << std::endl);
	int recv_buffer[2];
	MPI_Status status;
	MPI_Recv(recv_buffer, 2, MPI_INT, mpi_source, tag, MPI_COMM_WORLD, &status);
	int array_id = recv_buffer[0];
	int label_slot = recv_buffer[1];
	std::string array_label = sip_tables_.string_literal(label_slot);
	int section_number = SIPMPIUtils::get_section_number(tag);
	int message_number = SIPMPIUtils::get_message_number(tag);
	check (section_number >= this->section_number_, "Section number invariant violated. Got request from an older section !");
	this->section_number_ = section_number;

	// mark persistent array
	pbm_write_.mark_persistent_array(array_id, array_label);

	// Send Ack
	int save_persistent_ack_tag = SIPMPIUtils::make_mpi_tag(SIPMPIData::SAVE_PERSISTENT_ACK, section_number, message_number);
	SIPMPIUtils::send_ack_to_rank(mpi_source, SIPMPIData::SAVE_PERSISTENT_ACK, save_persistent_ack_tag);

	SIP_LOG(std::cout << sip_mpi_attr_.global_rank()<< " : Done with SAVE_PERSISTENT for array "<<array_id<<" with label "  <<array_label << std::endl);
}

void SIPServer::handle_RESTORE_PERSISTENT(int mpi_source, int tag) {
	// Message sent to all servers from their corresponding local workers
	SIP_LOG(std::cout << sip_mpi_attr_.global_rank()<< " : Got RESTORE_PERSISTENT " << std::endl);
	int recv_buffer[2];
	MPI_Status status;
	MPI_Recv(recv_buffer, 2, MPI_INT, mpi_source, tag, MPI_COMM_WORLD, &status);
	int array_id = recv_buffer[0];
	int label_slot = recv_buffer[1];
	std::string array_label = sip_tables_.string_literal(label_slot);
	int section_number = SIPMPIUtils::get_section_number(tag);
	int message_number = SIPMPIUtils::get_message_number(tag);
	check (section_number >= this->section_number_, "Section number invariant violated. Got request from an older section !");
	this->section_number_ = section_number;
	restore_persistent_array(array_label, array_id);

	// Send Ack
	int retore_persistent_ack_tag = SIPMPIUtils::make_mpi_tag(SIPMPIData::RESTORE_PERSISTENT_ACK, section_number, message_number);
	SIPMPIUtils::send_ack_to_rank(mpi_source, SIPMPIData::RESTORE_PERSISTENT_ACK, retore_persistent_ack_tag);

	SIP_LOG(std::cout << sip_mpi_attr_.global_rank()<< " : Done with RESTORE_PERSISTENT for array "<<array_label << std::endl);
}

void SIPServer::run(){

	while (true){

		// Receive a message from either a server or a worker.
		// The worker can ask to
		//		get a block,
		//		put a block, (and send the data later)
		//		put accumulate a block, (and send the data later)
		//		do a barrier,
		//		do a save array
		// 		do a restore array
		//		end the current program


		MPI_Status status;

		// Check to see if a request has arrived from any worker.
		SIPMPIUtils::check_err(MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status));

		int mpi_tag = status.MPI_TAG;
		int mpi_source = status.MPI_SOURCE;

		SIPMPIData::MessageType_t message_type = SIPMPIUtils::get_message_type(mpi_tag);

		switch (message_type) {
		//case SIPMPIData::CREATE:
		//	// NO OP - Create blocks on the fly.
		//	break;
		case SIPMPIData::GET :
			handle_GET(mpi_source, mpi_tag);
			break;
		case SIPMPIData::PUT :
			handle_PUT(mpi_source, mpi_tag);
			break;
		case SIPMPIData::PUT_DATA :{
			int size;
			SIPMPIUtils::check_err(MPI_Get_count(&status, MPI_DOUBLE, &size));
			handle_PUT_DATA(mpi_source, size, mpi_tag);
		}
			break;
		case SIPMPIData::PUT_ACCUMULATE:
			handle_PUT_ACCUMULATE(mpi_source, mpi_tag);
			break;
		case SIPMPIData::PUT_ACCUMULATE_DATA :{
			int size;
			SIPMPIUtils::check_err(MPI_Get_count(&status, MPI_DOUBLE, &size));
			handle_PUT_ACCUMULATE_DATA(mpi_source, size, mpi_tag);
		}
			break;
		case SIPMPIData::DELETE:
			handle_DELETE(mpi_source, mpi_tag);
			break;
		case SIPMPIData::SAVE_PERSISTENT:
			handle_SAVE_PERSISTENT(mpi_source, mpi_tag);
			break;
		case SIPMPIData::RESTORE_PERSISTENT:
			handle_RESTORE_PERSISTENT(mpi_source, mpi_tag);
			break;
		case SIPMPIData::END_PROGRAM:
			handle_END_PROGRAM(mpi_source, mpi_tag);
			return;
		default:
			fail("Received unexpected message in SIP Server !");
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
					check(it2->second != NULL,
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

std::ostream& operator<<(std::ostream& os, const SIPServer::TagInfo& obj){
	os << "TagInfo[from:" << obj.from << ",msg : "<<obj.message_number
		<< ",sect : " << obj.section_number << "]";
	return os;
}

std::ostream& operator<<(std::ostream& os, const SIPServer::TagInfoIdBlockPairMap& obj){
	SIPServer::TagInfoIdBlockPairMap::const_iterator it = obj.begin();
	os << "TagInfoIdBlockPairMap[";
	for (; it != obj.end(); ++it){
		os << it->first << " : "<<it->second.first<<", ";
	}
	os << "]";
	return os;
}

} /* namespace sip */
