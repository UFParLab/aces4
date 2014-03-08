/*
 * block_manager.cpp
 *
 * This is a very simple implementation of the block manager interface for a single-node
 * version of aces4.
 *
 * Blocks are lazily instantiated when first written to.
 *
 * Temp blocks are removed when leave_scope is called for the scope in which they are created.
 * Scopes are tracked by a stack of lists, temp_block_list_stack_;  enter_scope adds a new,
 * empty block list to temp_block_list_stack_.  A pointer to each temp block that is created
 * is added to the list on top of the stack.  leave_scope removes all blocks in the list on
 * top of the stack, and pops the stack.
 *
 * Other blocks are removed when an explicit call is made to destroy_distributed, delete_served,
 * or deallocate_local.
 *
 *
 *
 *  Created on: Aug 10, 2013
 *      Author: Beverly Sanders
 */

#include "config.h"
#include "block_manager.h"
#include <assert.h>
#include <vector>
#include <set>
#include <utility>
#include <algorithm>
#include "sip_tables.h"
#include "array_constants.h"
#include "sip_interface.h"
#include "gpu_super_instructions.h"
#include "persistent_array_manager.h"

#ifdef HAVE_MPI
#include "sip_mpi_data.h"
#include "mpi.h"
#include "sip_mpi_utils.h"
#endif

namespace sip {

#ifdef HAVE_MPI
BlockManager::BlockManager(SipTables& sipTables, PersistentArrayManager& pbm_read, PersistentArrayManager& pbm_write,
		SIPMPIAttr& sip_mpi_attr, DataDistribution& data_distribution, int &section_number, int &message_number) :
		pbm_read_(pbm_read), pbm_write_(pbm_write),
		sip_tables_(sipTables), block_map_(sipTables.num_arrays()),
		sip_mpi_attr_(sip_mpi_attr), data_distribution_(data_distribution),
		section_number_(section_number), message_number_(message_number),
		num_posted_async_(0)
		{
	// Initialize the Block Map to all NULLs
	int num_arrs = block_map_.size();
	for (int i=0; i< num_arrs; i++){
		block_map_[i] = NULL;
	}

	std::fill(posted_async_ + 0, posted_async_+MAX_POSTED_ASYNC, MPI_REQUEST_NULL);
}


#else
BlockManager::BlockManager(SipTables& sipTables, PersistentArrayManager& pbm_read, PersistentArrayManager& pbm_write) :
		pbm_read_(pbm_read), pbm_write_(pbm_write),
		sip_tables_(sipTables), block_map_(sipTables.num_arrays()){
	// Initialize the Block Map to all NULLs
	int num_arrs = block_map_.size();
	for (int i=0; i< num_arrs; i++){
		block_map_[i] = NULL;
	}
}

#endif

/**
 * Delete blocks being managed by the block manager.
 */
BlockManager::~BlockManager() {
	check(temp_block_list_stack_.size() == 0, "temp_block_list_stack not empty when destroying data manager!");

	for (int i=0; i<block_map_.size(); i++){
		if (block_map_[i] != NULL){
			IdBlockMapPtr bid_map = block_map_[i];
				if (bid_map != NULL){
				for (IdBlockMap::iterator it2 = bid_map->begin(); it2 != bid_map->end(); ++it2) {
					if (it2->second != NULL)
						delete it2->second;	// Delete the block being pointed to.
				}
				delete block_map_[i];
				block_map_[i] = NULL;
			}
		}
	}
	block_map_.clear();

}

void BlockManager::barrier() {
	/*not actually needed for sequential version, but may
	 * want to enhance to detect data races.
	 */
#ifdef HAVE_MPI

	SIP_LOG(std::cout<< "W " << sip_mpi_attr_.global_rank() << " : Beginning BARRIER "<< std::endl);
	SIP_LOG(if(sip_mpi_attr_.is_company_master()) std::cout<<"W " << sip_mpi_attr_.global_rank() << " : I am company master sending BARRIER to server !" << std::endl);

	MPI_Status statuses[MAX_POSTED_ASYNC];
	sip::check(0 <= num_posted_async_ && num_posted_async_ <= MAX_POSTED_ASYNC, "Inconsistent value for number of posted asyncs !", current_line());
	if (num_posted_async_ > 0)
		SIPMPIUtils::check_err(MPI_Waitall(num_posted_async_, posted_async_, statuses));
	num_posted_async_ = 0;
	blocks_in_transit_.clear();

	std::fill(posted_async_ + 0, posted_async_+MAX_POSTED_ASYNC, MPI_REQUEST_NULL);

	// Clear out cached distributed & served blocks.
	for (int i=0; i<block_map_.size(); i++){
		if (sip_tables_.is_distributed(i) || sip_tables_.is_served(i)){
			if (block_map_[i] != NULL){
				IdBlockMap *bid_map = block_map_[i];
				for (IdBlockMap::iterator it2 = bid_map->begin(); it2 != bid_map->end(); ++it2) {
					if (it2->second != NULL)
						delete it2->second;	// Delete the block being pointed to.
				}
				delete block_map_[i];
				block_map_[i] = NULL;
			}
		}
	}

	// Workers do barrier amongst themselves
	SIPMPIUtils::check_err(MPI_Barrier(sip_mpi_attr_.company_communicator()));

	SIP_LOG(std::cout<< "W " << sip_mpi_attr_.global_rank() << " : Done with BARRIER "<< std::endl);

#endif
}

void BlockManager::create_distributed(int array_id) {
	/*For single-node version, this is a nop.  We will add blocks as needed.
	 * For the MPI version, the same behavior is implemented*/

	// NO op for multinode version
}

/**
 * Creates a distributed array from a given set of blocks.
 * Clones the block pointers.
 * @param array_id
 * @param bid_map
 */
void BlockManager::restore_distributed(int array_id, BlockManager::IdBlockMapPtr bid_map){

	// Clear blocks of array
	BlockManager::IdBlockMap *old_bid_map = block_map_[array_id];
	if (old_bid_map != NULL){
		for (BlockManager::IdBlockMap::iterator it = old_bid_map->begin(); it != old_bid_map->end(); ++it){
			delete it->second;
		}
		delete block_map_[array_id];
	}

	block_map_[array_id] = new IdBlockMap();

	// Put in new blocks.
	for (BlockManager::IdBlockMap::iterator it = bid_map->begin(); it != bid_map->end(); ++it){
		BlockId bid(it->first);
		bid.array_id_ = array_id;
		Block::BlockPtr bptr_clone = it->second->clone();
		block_map_[array_id]->insert(std::pair<BlockId, Block::BlockPtr>(bid, bptr_clone));
	}
}


void BlockManager::delete_distributed(int array_to_destroy) {
	/* Removes all the blocks associated with this array from the block map.
	 * In this implementation, removing it from the map will cause its destructor to
	 * be called, which will delete the data. Later, we may want to introduce smart
	 * pointers for the dataPtr.
	 */

	//BlockMap::iterator it = block_map_.find(array_to_destroy);
	BlockManager::IdBlockMapPtr bid_map = block_map_[array_to_destroy];

#ifndef HAVE_MPI
	SIP_LOG(check_and_warn(bid_map != NULL, "Could not find distributed array to destroy !"));
#endif

	if (bid_map != NULL){
		for (IdBlockMap::iterator it = bid_map->begin(); it != bid_map->end(); ++it){
			Block::BlockPtr bptr = it->second;
			delete bptr;
			it->second = NULL;
		}
		delete block_map_[array_to_destroy];
		block_map_[array_to_destroy] = NULL;
	}

#ifdef HAVE_MPI

	SIP_LOG(std::cout<< "W " << sip_mpi_attr_.global_rank() << " : Beginning DELETE "<< std::endl);

	// Delete remotely, only local servers are sent messages
	int global_rank = sip_mpi_attr_.global_rank();
	int global_size = sip_mpi_attr_.global_size();
	bool am_worker_to_communicate = RankDistribution::is_local_worker_to_communicate(global_rank, global_size);

	if (am_worker_to_communicate){
		int local_server_rank = RankDistribution::local_server_to_communicate(global_rank, global_size);
		if (local_server_rank > -1){
			SIP_LOG(if(sip_mpi_attr_.is_company_master()) std::cout<<"W " << sip_mpi_attr_.global_rank() << " : sending DELETE to server "<<  local_server_rank<< std::endl);

			// Send array_id to server master
			int delete_tag = SIPMPIUtils::make_mpi_tag(SIPMPIData::DELETE, section_number_, message_number_);
			SIPMPIUtils::check_err(MPI_Send(&array_to_destroy, 1, MPI_INT, local_server_rank, delete_tag, MPI_COMM_WORLD));
			int delete_ack_tag = SIPMPIUtils::make_mpi_tag(SIPMPIData::DELETE_ACK, section_number_, message_number_);
			SIPMPIUtils::expect_ack_from_rank(local_server_rank, SIPMPIData::DELETE_ACK, delete_ack_tag);
			message_number_++;
		}
	}



	SIP_LOG(std::cout<< "W " << sip_mpi_attr_.global_rank() << " : Done with DELETE "<< std::endl);

#endif


}

void BlockManager::get(const BlockId& block_id) {

#ifdef HAVE_MPI
	//get_block_for_reading(block_id);
	request_block_from_server(block_id);
#else
	get_block_for_writing(block_id, false);
#endif

}


void BlockManager::put_replace(const BlockId& target,
		const Block::BlockPtr source_ptr) {
	Block::BlockPtr target_ptr = get_block_for_writing(target, false);
	assert(target_ptr->shape_ == source_ptr->shape_);
	target_ptr->copy_data_(source_ptr);

#ifdef HAVE_MPI
	int server_rank = data_distribution_.get_server_rank(target);

	target_ptr->copy_data_(source_ptr);

	int put_block_info_tag = SIPMPIUtils::make_mpi_tag(SIPMPIData::PUT, section_number_, message_number_);
	int put_data_tag = SIPMPIUtils::make_mpi_tag(SIPMPIData::PUT_DATA, section_number_, message_number_);
	SIP_LOG(std::cout<< "W " << sip_mpi_attr_.global_rank() << " : Sending PUT for block with tags "<<put_block_info_tag<< " and "<< put_data_tag << ", "<< target << " to server rank " << server_rank << std::endl);

	wait_for_block_in_transit(target);
	// Free up a slot before posting any more requests
	if (num_posted_async_ == MAX_POSTED_ASYNC){
		free_any_posted_receive();
	}

	MPI_Request put_replace_request;
	SIPMPIUtils::isend_bid_and_bptr_to_rank(target, target_ptr, server_rank, put_block_info_tag, put_data_tag, &put_replace_request);

	int put_ack_tag = SIPMPIUtils::make_mpi_tag(SIPMPIData::PUT_DATA_ACK, section_number_, message_number_);
	MPI_Request put_ack_request;
	SIPMPIUtils::check_err(MPI_Irecv(&(posted_acks_[num_posted_async_]), 1, MPI_INT, server_rank, put_ack_tag, MPI_COMM_WORLD, &put_ack_request));


	//MPI_Status status;
	//SIPMPIUtils::check_err(MPI_Wait(&put_ack_request, &status));

	// Record this posted receive.
	posted_async_[num_posted_async_] = put_ack_request;
	blocks_in_transit_[target] = num_posted_async_;
	num_posted_async_++;

	//SIPMPIUtils::expect_ack_from_rank(server_rank, SIPMPIData::PUT_DATA_ACK, put_ack_tag);
	message_number_++;
	SIP_LOG(std::cout<< "W " << sip_mpi_attr_.global_rank() << " : Done with PUT for block " << target << " to server rank " << server_rank << std::endl);


#endif

}

void BlockManager::put_accumulate(const BlockId& lhs_id,
		const Block::BlockPtr source_ptr) {
	Block::BlockPtr target_ptr = get_block_for_accumulate(lhs_id, false);
	assert(target_ptr->shape_ == source_ptr->shape_);

#ifdef HAVE_MPI

	int server_rank = data_distribution_.get_server_rank(lhs_id);
	int to_send_message = SIPMPIData::PUT_ACCUMULATE;
	target_ptr->copy_data_(source_ptr);

	//send_block_to_server(server_rank, to_send_message, lhs_id, target_ptr);
	int put_acc_block_info_tag = SIPMPIUtils::make_mpi_tag(SIPMPIData::PUT_ACCUMULATE, section_number_, message_number_);
	int put_acc_data_tag = SIPMPIUtils::make_mpi_tag(SIPMPIData::PUT_ACCUMULATE_DATA, section_number_, message_number_);
	SIP_LOG(std::cout<< "W " << sip_mpi_attr_.global_rank() << " : Sending PUT for block with tags "<<put_acc_block_info_tag<< " and "<< put_acc_data_tag << ", "<< lhs_id << " to server rank " << server_rank << std::endl);

	wait_for_block_in_transit(lhs_id);
	// Free up a slot before posting any more requests
	if (num_posted_async_ == MAX_POSTED_ASYNC){
		free_any_posted_receive();
	}

	MPI_Request put_acc_request;
	SIPMPIUtils::isend_bid_and_bptr_to_rank(lhs_id, target_ptr, server_rank, put_acc_block_info_tag, put_acc_data_tag, &put_acc_request);

	int put_acc_ack_tag = SIPMPIUtils::make_mpi_tag(SIPMPIData::PUT_ACCUMULATE_DATA_ACK, section_number_, message_number_);
	MPI_Request put_acc_ack_request;
	SIPMPIUtils::check_err(MPI_Irecv(&(posted_acks_[num_posted_async_]), 1, MPI_INT, server_rank, put_acc_ack_tag, MPI_COMM_WORLD, &put_acc_ack_request));

	//MPI_Status status;
	//SIPMPIUtils::check_err(MPI_Wait(&put_acc_ack_request, &status));

	// Record this posted receive.
	posted_async_[num_posted_async_] = put_acc_ack_request;
	blocks_in_transit_[lhs_id] = num_posted_async_;
	num_posted_async_++;

	//SIPMPIUtils::expect_ack_from_rank(server_rank, SIPMPIData::PUT_ACCUMULATE_DATA_ACK, put_acc_ack_tag);

	message_number_++;
	SIP_LOG(std::cout<< "W " << sip_mpi_attr_.global_rank() << " : Done with PUT_ACCUMULATE for block " << lhs_id << " to server rank " << server_rank << std::endl);

#else
	// Accumulate locally
	target_ptr->accumulate_data(source_ptr);
#endif
}

void BlockManager::destroy_served(int array_id) {
	delete_distributed(array_id);
}

void BlockManager::request(const BlockId& block_id) {
	get(block_id);
}
void BlockManager::prequest(const BlockId&, const BlockId&) {
	fail("PREQUEST Not supported !");
}
void BlockManager::prepare(const BlockId& lhs_id,
		const Block::BlockPtr source_ptr) {
	put_replace(lhs_id, source_ptr);
}
void BlockManager::prepare_accumulate(const BlockId& lhs_id,
		const Block::BlockPtr source_ptr) {
	put_accumulate(lhs_id, source_ptr);
}


bool BlockManager::has_wild_value(const BlockId& id){
	int i = 0;
	while (i != MAX_RANK){
		if (id.index_values(i) == wild_card_value) return true;
		++i;
	}
	return false;
}
bool BlockManager::has_wild_slot(const index_selector_t& selector){
	int i = 0;
	while (i != MAX_RANK){
		if (selector[i] == wild_card_slot) return true;
		++i;
	}
	return false;
}

void BlockManager::allocate_local(const BlockId& id) {
	if (has_wild_value(id)) {
		std::vector<BlockId> list;
		generate_local_block_list(id, list);
		std::vector<BlockId>::iterator it;
		for (it = list.begin(); it != list.end(); ++it) {
			get_block_for_writing(*it, false);
		}
	} else {
		get_block_for_writing(id, false);
	}
}
void BlockManager::deallocate_local(const BlockId& id) {
	if (has_wild_value(id)) {
		std::vector<BlockId> list;
		generate_local_block_list(id, list);
		std::vector<BlockId>::iterator it;
		for (it = list.begin(); it != list.end(); ++it) {
			remove_block(*it);
		}
	} else {
		remove_block(id);
	}
}

//returns a pointer to the requested block, creating it if it doesn't exist
Block::BlockPtr BlockManager::get_block_for_writing(const BlockId& id,
		bool is_scope_extent) {
	Block::BlockPtr blk = block(id);
	BlockShape shape = sip_tables_.shape(id);
	if (blk == NULL) { //need to create it
		blk = create_block(id, shape);
		if (is_scope_extent) {
			temp_block_list_stack_.back()->push_back(id);
		}
	}
    //blk->fill(0);
#ifdef HAVE_CUDA
	// Lazy copying of data from gpu to host if needed.
	lazy_gpu_write_on_host(blk, id, shape);
#endif
	return blk;
}


#ifdef HAVE_MPI

void BlockManager::wait_for_block_in_transit(const BlockId& id) {
	BlockIdToIndexMap::iterator it = blocks_in_transit_.find(id);
	int i = -1;
	if (it != blocks_in_transit_.end()) {
		SIP_LOG(std::cout << "W " << sip_mpi_attr_.global_rank()
								<< " : Waiting for block " << id << std::endl);
		i = it->second;
		sip::check(0 <= i && i < MAX_POSTED_ASYNC, "Inconsistent value stored in blocks_in_transit_ map !", current_line());
		MPI_Request r = posted_async_[i];
		MPI_Status s;
		SIPMPIUtils::check_err(MPI_Wait(&r, &s));
		if (num_posted_async_ > 0) {
			posted_async_[i] = posted_async_[num_posted_async_ - 1];
			num_posted_async_--;
		}
		blocks_in_transit_.erase(id);
		SIP_LOG(std::cout << "W " << sip_mpi_attr_.global_rank()
							  << " : Got block " << id << ", was at index :"
							  << i << ", outstanding : "<< num_posted_async_
							  << std::endl);
	}
}

void BlockManager::free_any_posted_receive() {
	MPI_Status statuses[MAX_POSTED_ASYNC];
	int completed_index = -1;
	SIPMPIUtils::check_err(MPI_Waitany(num_posted_async_, posted_async_, &completed_index,
			statuses));
	posted_async_[completed_index] = posted_async_[num_posted_async_ - 1];
	num_posted_async_--;
}

void BlockManager::request_block_from_server(const BlockId& id) {
	Block::BlockPtr blk = block(id);
	int array_id = id.array_id();
	bool is_remote = sip_tables_.is_distributed(array_id) || sip_tables_.is_served(array_id);

	// Request block from server if not cached
	if (blk == NULL && is_remote) {

		// The request for this block is already in transit.
		// Complete that and launch a new one.
		//wait_for_block_in_transit(id); -> done in call to block()

		// Fetch remote block (not is cache).
		int server_rank = data_distribution_.get_server_rank(id);

		// Send block id to fetch
		int get_tag = SIPMPIUtils::make_mpi_tag(SIPMPIData::GET, section_number_, message_number_);
		SIP_LOG(std::cout << "W " << sip_mpi_attr_.global_rank()
						<< " : Sending GET with tag " << get_tag
						<< " for block " << id << " to server rank "
						<< server_rank << std::endl);
		SIPMPIUtils::send_block_id_to_rank(id, server_rank, get_tag);

		// Receive block id, shape & size
		BlockShape shape = sip_tables_.shape(id);
		int data_size = shape.num_elems();

		// Receive block double precision data
		blk = new Block(shape);
		int block_tag = SIPMPIUtils::make_mpi_tag(SIPMPIData::GET_DATA, section_number_, message_number_);
		//SIPMPIUtils::get_bptr_data_from_rank(server_rank, &block_tag, data_size, blk);

		check(blk != NULL, "Block Pointer into which data is to be received is NULL !", current_line());

		// Free up a slot before posting any more requests
		if (num_posted_async_ == MAX_POSTED_ASYNC){
			free_any_posted_receive();
		}

		MPI_Request request;
		SIPMPIUtils::check_err(MPI_Irecv(blk->data_, data_size, MPI_DOUBLE, server_rank, block_tag, MPI_COMM_WORLD, &request));

		MPI_Status status;
		SIPMPIUtils::check_err(MPI_Wait(&request, &status));

//		int msg_no = SIPMPIUtils::get_message_number(block_tag);
//		int sect_no = SIPMPIUtils::get_section_number(block_tag);
//		check(msg_no == message_number_, "Message number not consistent in GET !", current_line());
//		check(sect_no == section_number_,"Section number not consistent in GET !", current_line());

		insert_into_blockmap(id, blk);

		// Record this posted receive.
		posted_async_[num_posted_async_] = request;
		blocks_in_transit_[id] = num_posted_async_;
		num_posted_async_++;

		message_number_++;

		SIP_LOG(std::cout << "W " << sip_mpi_attr_.global_rank()
						<< " : Done with GET for block " << id
						<< " from server rank " << server_rank << std::endl);
	}
}

#endif

Block::BlockPtr BlockManager::get_block_for_reading(const BlockId& id) {
	Block::BlockPtr blk = block(id);

#ifdef HAVE_MPI

#endif
	check(blk != NULL, "attempting to read non-existent block", current_line());
#ifdef HAVE_CUDA
	// Lazy copying of data from gpu to host if needed.
	lazy_gpu_read_on_host(blk);
#endif

	return blk;
}

/* gets block for reading and writing.  The block should already exist.*/
Block::BlockPtr BlockManager::get_block_for_updating(const BlockId& id) {
	Block::BlockPtr blk = block(id);
	check(blk != NULL, "attempting to update non-existent block", current_line());
#ifdef HAVE_CUDA
	// Lazy copying of data from gpu to host if needed.
	lazy_gpu_update_on_host(blk);
#endif
	return blk;
}


Block::BlockPtr BlockManager::get_block_for_accumulate(const BlockId& id,
		bool is_scope_extent) {
	return get_block_for_writing(id, is_scope_extent);
}

void BlockManager::enter_scope() {
//	std::cout << "in enter_scope" << std::endl;
	BlockList* temps = new BlockList;
	temp_block_list_stack_.push_back(temps);
}
/*removes the temp blocks in the current scope, then delete the scope's TempBlockStack */
void BlockManager::leave_scope() {
//	std::cout << "in leave_scope" << std::endl;
	BlockList* temps = temp_block_list_stack_.back();
	BlockList::iterator it;
//std::cout<<*this<<std::endl;
	for (it = temps->begin(); it != temps->end(); ++it) {
//int array_id = (*it).array_id();
//std::string arrname = sipTables_.array_name(array_id);
//std::cout<<"Now freeing " <<arrname<<std::endl;
		remove_block(*it);
	}
	temp_block_list_stack_.pop_back();
	delete temps;
}


void BlockManager::save_persistent_dist_arrays(){

	int num_arrs = block_map_.size();
//	for (BlockMap::iterator it = block_map_.begin(); it != block_map_.end(); ++it){
//		if (pbm_write_.is_array_persistent(it->first)){
//
//			IdBlockMap &bid_map = it->second;
//			pbm_write_.save_dist_array(it->first, bid_map);
//		}
//	}
	for (int i=0; i<num_arrs; i++){
		bool is_remote = sip_tables_.is_distributed(i) || sip_tables_.is_served(i);
		if (is_remote && pbm_write_.is_array_persistent(i)){
			IdBlockMap *bid_map = block_map_[i];
			pbm_write_.save_dist_array(i, bid_map);
			block_map_[i] = NULL;
		}
	}
}


std::ostream& operator<<(std::ostream& os, const BlockManager& obj) {
	os << "block_map_:" << std::endl;
	{
		int num_arrs = obj.block_map_.size();
		for (int i=0; i<num_arrs; i++) {
			if (obj.block_map_[i] != NULL){
				os << "array [" << i << "]" << std::endl;  //print the array id
				const BlockManager::IdBlockMapPtr bid_map = obj.block_map_[i];
				BlockManager::IdBlockMap::const_iterator it2;
				for (it2 = bid_map->begin(); it2 != bid_map->end(); ++it2) {
					check(it2->second != NULL, "Trying to print NULL blockPtr !");
					SIP_LOG(if(it2->second == NULL) std::cout<<it2->first<<" is NULL!"<<std::endl;);
					os << it2 -> first << std::endl;
				}
				os << std::endl;
			}
		}
	}
	{
		os << "temp_block_list_stack_:" << std::endl;
		std::vector<BlockManager::BlockList*>::const_iterator it;
		for (it = obj.temp_block_list_stack_.begin();
				it != obj.temp_block_list_stack_.end(); ++it) {
			BlockManager::BlockList::iterator lit;
			for (lit = (*it)->begin(); lit != (*it)->end(); ++lit) {
				os << (lit == (*it)->begin() ? "" : ",") << *lit;
			}
			os << std::endl;
		}
	}
	return os;
}

Block::BlockPtr BlockManager::block(const BlockId& id) {
	int array_id = id.array_id_;

	if (block_map_[array_id] == NULL){
		block_map_[array_id] = new IdBlockMap();
		//check_and_warn(false, "Could not find distributed array, creating a new one...");
	}

	//check(&block_map_[array_id] != NULL, "map containing blocks for array is null!", current_line());
	IdBlockMap *bid_map = block_map_[array_id];
	IdBlockMap::iterator b = bid_map->find(id);
	if (b != bid_map->end()) {
#ifdef HAVE_MPI
		//Check if this block is in transit and wait for it.
		wait_for_block_in_transit(id);
#endif
		return b->second;
	}
	return NULL;
}

/**
 * It is an error to try to create a new block if a block with that id already exists
 * exists.
 */
void BlockManager::insert_into_blockmap(const BlockId& block_id, Block::BlockPtr block_ptr) {
	check(block_ptr != NULL, "Trying to insert NULL block into BlockMap !", current_line());
	std::pair<IdBlockMap::iterator, bool> ret;
	int array_id = block_id.array_id_;
	IdBlockMap *bid_map = block_map_[array_id];
	std::pair<BlockId, Block::BlockPtr> bid_pair (block_id, block_ptr);
	ret = bid_map->insert(bid_pair);
	check(ret.second, std::string("attempting to create block that already exists"));
}

/** creates a new block with the given Id and inserts it into the block map.
 * It is an error to try to create a new block if a block with that id already exists
 * exists.
 */
Block::BlockPtr BlockManager::create_block(const BlockId& block_id, const BlockShape& shape) {
	Block::BlockPtr block_ptr = new Block(shape);
	insert_into_blockmap(block_id, block_ptr);
	return block_ptr;
}

void BlockManager::remove_block(const BlockId& block_id) {
	int array_id = block_id.array_id_;

//	BlockMap::iterator it = block_map_.find(array_id);
//	check(it != block_map_.end(), "Could not find distributed array to remove block from !");


	IdBlockMap *bid_map = block_map_[array_id];
	check(bid_map != NULL,"Could not find distributed array " + sip_tables_.array_name(array_id) + " to remove block from !", current_line());
	Block::BlockPtr block_to_remove = bid_map->at(block_id);
	bid_map->erase(block_id);
	delete block_to_remove;
}

void BlockManager::generate_local_block_list(const BlockId& id,
		std::vector<BlockId>& list) {
	std::vector<int> prefix;
	int rank = sip_tables_.array_rank(id);
	int pos = 0;
	gen(id, rank, pos, prefix, 0, list);
}

void BlockManager::gen(const BlockId& id, int rank, const int pos,
		std::vector<int> prefix /*pass by value*/, int to_append, std::vector<BlockId>& list) {
    if (pos != 0) {
    	prefix.push_back(to_append);
    }
	if (pos < rank) {
		int curr_index = id.index_values(pos);
		if (curr_index == wild_card_value) {
			int index_slot = sip_tables_.selectors(id.array_id())[pos];
			int lower = sip_tables_.lower_seg(index_slot);
			int upper = lower + sip_tables_.num_segments(index_slot);
			for (int i = lower; i < upper; ++i){
				gen(id, rank, pos+1, prefix, i, list);
			}
		} else {
			gen(id, rank, pos + 1, prefix, curr_index, list);
		}

	} else {
		list.push_back(BlockId(id.array_id(), rank, prefix));
	}
}


#ifdef HAVE_CUDA
/*********************************************************************/
/**						GPU Specific methods						**/
/*********************************************************************/
Block::BlockPtr BlockManager::get_gpu_block_for_writing(const BlockId& id, bool is_scope_extent){
	Block::BlockPtr blk = block(id);
	BlockShape shape = sip_tables_.shape(id);
	if (blk == NULL) { //need to create it
		blk = create_gpu_block(id, shape);
		if (is_scope_extent) {
			temp_block_list_stack_.back()->push_back(id);
		}
	}
	// Lazy copying of data from host to gpu if needed.
	lazy_gpu_write_on_device(blk, id, shape);
    //blk->gpu_fill(0);
	return blk;
}
Block::BlockPtr BlockManager::get_gpu_block_for_updating(const BlockId& id){
	Block::BlockPtr blk = block(id);
	check(blk != NULL, "attempting to update non-existent block");

	// Lazy copying of data from host to gpu if needed.
	lazy_gpu_update_on_device(blk);

	return blk;
}
Block::BlockPtr BlockManager::get_gpu_block_for_reading(const BlockId& id){
	Block::BlockPtr blk = block(id);
	check(blk != NULL, "attempting to read non-existent gpu block", current_line());

	// Lazy copying of data from host to gpu if needed.
	lazy_gpu_read_on_device(blk);

	return blk;
}

Block::BlockPtr BlockManager::get_gpu_block_for_accumulate(const BlockId& id,
		bool is_scope_extent) {
	return get_gpu_block_for_writing(id, is_scope_extent);
}


/** creates a new block with the given Id and inserts it into the block map.
 * It is an error to try to create a new block if a block with that id already exists
 * exists.
 */
Block::BlockPtr BlockManager::create_gpu_block(const BlockId& block_id,
		const BlockShape& shape) {
	Block::BlockPtr block_ptr = Block::new_gpu_block(shape);
	insert_into_blockmap(block_id, block_ptr);
	return block_ptr;
}


void BlockManager::lazy_gpu_read_on_device(Block::BlockPtr& blk){
	if (!blk->is_on_gpu() && !blk->is_on_host()) {
		fail("block allocated neither on host or gpu", current_line());
	} else if (!blk->is_on_gpu()) {
		blk->allocate_gpu_data();
		_gpu_host_to_device(blk->get_data(), blk->get_gpu_data(), blk->size());
	} else if (blk->is_dirty_on_host()) {
		_gpu_host_to_device(blk->get_data(), blk->get_gpu_data(), blk->size());
	} else if (blk->is_dirty_on_all()) {
		fail("block dirty on host & gpu !", current_line());
	}
	blk->set_on_gpu();
	blk->unset_dirty_on_host();
}

void BlockManager::lazy_gpu_write_on_device(Block::BlockPtr& blk, const BlockId &id, const BlockShape& shape) {
	if (!blk->is_on_gpu() && !blk->is_on_host()) {
		remove_block(id); // Get rid of block, create a new one
		blk = create_gpu_block(id, shape);
//		if (is_scope_extent) {
//			temp_block_list_stack_.back()->push_back(id);
//		}
	} else if (!blk->is_on_gpu()) {
		blk->allocate_gpu_data();
		_gpu_host_to_device(blk->get_data(), blk->get_gpu_data(), blk->size());
	} else if (blk->is_dirty_on_host()) {
		_gpu_host_to_device(blk->get_data(), blk->get_gpu_data(), blk->size());
	} else if (blk->is_dirty_on_all()) {
		fail("block dirty on host & gpu !", current_line());
	}
	blk->set_on_gpu();
	blk->set_dirty_on_gpu();
	blk->unset_dirty_on_host();
}

void BlockManager::lazy_gpu_update_on_device(Block::BlockPtr& blk){
	if (!blk->is_on_gpu() && !blk->is_on_host()){
			fail("block allocated neither on host or gpu", current_line());
		} else if (!blk->is_on_gpu()){
			blk->allocate_gpu_data();
			_gpu_host_to_device(blk->get_data(), blk->get_gpu_data(), blk->size());
		} else if (blk->is_dirty_on_host()){
			_gpu_host_to_device(blk->get_data(), blk->get_gpu_data(), blk->size());
		} else if (blk->is_dirty_on_all()){
			fail("block dirty on host & gpu !", current_line());
		}
		blk->set_on_gpu();
		blk->unset_dirty_on_host();
		blk->set_dirty_on_gpu();
}

void BlockManager::lazy_gpu_read_on_host(Block::BlockPtr& blk){
	if (!blk->is_on_gpu() && !blk->is_on_host()) {
		fail("block allocated neither on host or gpu", current_line());
	} else if (!blk->is_on_host()) {
		blk->allocate_host_data();
		_gpu_device_to_host(blk->get_data(), blk->get_gpu_data(), blk->size());
	} else if (blk->is_dirty_on_gpu()) {
		_gpu_device_to_host(blk->get_data(), blk->get_gpu_data(), blk->size());
	} else if (blk->is_dirty_on_all()) {
		fail("block dirty on host & gpu !", current_line());
	}
	blk->set_on_host();
	blk->unset_dirty_on_gpu();
}

void BlockManager::lazy_gpu_write_on_host(Block::BlockPtr& blk, const BlockId &id, const BlockShape& shape){
	if (!blk->is_on_gpu() && !blk->is_on_host()) {
		remove_block(id); // Get rid of block, create a new one
		blk = create_block(id, shape);
//		if (is_scope_extent) {
//			temp_block_list_stack_.back()->push_back(id);
//		}
	} else if (!blk->is_on_host()) {
		blk->allocate_host_data();
		_gpu_device_to_host(blk->get_data(), blk->get_gpu_data(), blk->size());
	} else if (blk->is_dirty_on_gpu()) {
		_gpu_device_to_host(blk->get_data(), blk->get_gpu_data(), blk->size());
	} else if (blk->is_dirty_on_all()) {
		fail("block dirty on host & gpu !", current_line());
	}
	blk->set_on_host();
	blk->set_dirty_on_host();
	blk->unset_dirty_on_gpu();

}

void BlockManager::lazy_gpu_update_on_host(Block::BlockPtr& blk){
	if (!blk->is_on_gpu() && !blk->is_on_host()) {
		fail("block allocated neither on host or gpu", current_line());
	} else if (!blk->is_on_host()) {
		blk->allocate_host_data();
		_gpu_device_to_host(blk->get_data(), blk->get_gpu_data(), blk->size());
	} else if (blk->is_dirty_on_gpu()) {
		_gpu_device_to_host(blk->get_data(), blk->get_gpu_data(), blk->size());
	} else if (blk->is_dirty_on_all()) {
		fail("block dirty on host & gpu !", current_line());
	}
	blk->set_on_host();
	blk->unset_dirty_on_gpu();
	blk->set_dirty_on_host();
}


#endif


} //namespace sip
