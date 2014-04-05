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

#ifdef HAVE_MPI
#include "sip_mpi_data.h"
#include "mpi.h"
#include "sip_mpi_utils.h"
#endif

namespace sip {

#ifdef HAVE_MPI
BlockManager::BlockManager(SipTables& sipTables, SIPMPIAttr& sip_mpi_attr,
		DataDistribution& data_distribution) :
		sip_tables_(sipTables), block_map_(sipTables.num_arrays()), sip_mpi_attr_(
				sip_mpi_attr), data_distribution_(data_distribution), num_posted_async_(
				0) {
	std::fill(posted_async_ + 0, posted_async_ + MAX_POSTED_ASYNC,
			MPI_REQUEST_NULL);
}

#else
BlockManager::BlockManager(SipTables& sipTables) :
sip_tables_(sipTables),
block_map_(sipTables.num_arrays()) {
}
#endif //HAVE_MPI

/**
 * Delete blocks being managed by the block manager.
 */
BlockManager::~BlockManager() {
	check(temp_block_list_stack_.size() == 0,
			"temp_block_list_stack not empty when destroying block manager!");
}

void BlockManager::barrier() {
	/*not actually needed for sequential version, but may
	 * want to enhance to detect data races.
	 */
#ifdef HAVE_MPI

	SIP_LOG(
			std::cout<< "W " << sip_mpi_attr_.global_rank() << " : Beginning BARRIER "<< std::endl);SIP_LOG(
			if(sip_mpi_attr_.is_company_master()) std::cout<<"W " << sip_mpi_attr_.global_rank() << " : I am company master sending BARRIER to server !" << std::endl);

	MPI_Status statuses[MAX_POSTED_ASYNC];
	sip::check(0 <= num_posted_async_ && num_posted_async_ <= MAX_POSTED_ASYNC,
			"Inconsistent value for number of posted asyncs !", current_line());
	if (num_posted_async_ > 0)
		SIPMPIUtils::check_err(
				MPI_Waitall(num_posted_async_, posted_async_, statuses));
	num_posted_async_ = 0;
	blocks_in_transit_.clear();

	std::fill(posted_async_ + 0, posted_async_ + MAX_POSTED_ASYNC,
			MPI_REQUEST_NULL);

	sip_mpi_attr_.barrier_support_.barrier(); //increment section number, reset msg number

	// Remove and deallocate cached blocks of distributed and served arrays
	for (int i = 0; i < block_map_.size(); ++i) {
		if (sip_tables_.is_distributed(i) || sip_tables_.is_served(i))
			block_map_.delete_per_array_map_and_blocks(i);
	}

	// Workers do actual MPI_barrier among themselves
//	SIPMPIUtils::check_err(MPI_Barrier(sip_mpi_attr_.company_communicator()));

	SIP_LOG(
			std::cout<< "W " << sip_mpi_attr_.global_rank() << " : Done with BARRIER "<< std::endl);

#endif
}

/** Currently this is a no-op.  Map, and blocks are created lazily when needed */
void BlockManager::create_distributed(int array_id) {
}

void BlockManager::delete_distributed(int array_id) {
	/* Removes all the blocks associated with this array from the block map.
	 * In this implementation, removing it from the map will cause its destructor to
	 * be called, which will delete the data. Because of this, we must be very
	 * careful to remove blocks that should not be delete from the block_map_.
	 */
	//delete any blocks stored locally
	block_map_.delete_per_array_map_and_blocks(array_id);
#ifdef HAVE_MPI
	//send delete message to server if responsible worker
	if (int my_server = sip_mpi_attr_.my_server() > 0) {
		SIP_LOG(
				std::cout<<"Worker " << sip_mpi_attr_.global_rank() << " : sending DELETE to server "<< my_server << std::endl);
		int delete_tag =
				sip_mpi_attr_.barrier_support_.make_mpi_tag_for_DELETE();
		SIPMPIUtils::check_err(
				MPI_Send(&array_id, 1, MPI_INT, my_server, delete_tag,
						MPI_COMM_WORLD));
		MPI_Status status;
//			SIPMPIUtils::expect_ack_from_rank(my_server, delete_tag);
		SIPMPIUtils::check_err(
				MPI_Recv(0, 0, MPI_INT, my_server, delete_tag, MPI_COMM_WORLD,
						&status));
	}
#endif //HAVE_MPI
}

void BlockManager::check_double_count(MPI_Status& status, int expected_count) {
	int received_count;
	SIPMPIUtils::check_err(MPI_Get_count(&status, MPI_DOUBLE, &received_count));
	check(received_count == expected_count,
			"message double count different than expected");
}
void BlockManager::get(BlockId& block_id) {

#ifdef HAVE_MPI

	int server_rank = data_distribution_.get_server_rank(block_id);
	int get_tag;
	get_tag = sip_mpi_attr_.barrier_support_.make_mpi_tag_for_GET();
//	std::cout << "get_tag: section number "
//			<< sip_mpi_attr_.barrier_support_.extract_section_number(get_tag)
//			<< " transaction number "
//			<< sip_mpi_attr_.barrier_support_.extract_transaction_number(
//					get_tag) << " tag type "
//			<< sip_mpi_attr_.barrier_support_.extract_message_type(get_tag)
//			<< std::endl;
	SIPMPIUtils::check_err(
			MPI_Send(reinterpret_cast<int *>(&block_id), BlockId::mpi_count,
					MPI_INT, server_rank, get_tag, MPI_COMM_WORLD));
	Block::BlockPtr block = get_block_for_writing(block_id); //this buffer will be overwritten with the version from the server.
	MPI_Status status;
	SIPMPIUtils::check_err(
			MPI_Recv(block->data_, block->size_, MPI_DOUBLE, server_rank,
					get_tag, MPI_COMM_WORLD, &status));
	check_double_count(status, block->size_);

#else
	get_block_for_writing(block_id);
#endif

}

/** A put appears in a SIAL program as
 * put Target(i,j,k,l) = Source(i,j,k,l)
 *
 * To get this right, we copy the contents of the source block into a local
 * copy of the target block and "put" the target block.
 *
 * @param target
 * @param source_ptr
 */
void BlockManager::put_replace(BlockId& target_id,
		const Block::BlockPtr source_block) {
	Block::BlockPtr target_block = get_block_for_writing(target_id, false);
	assert(target_block->shape_ == source_block->shape_);
	target_block->copy_data_(source_block);
#ifdef HAVE_MPI
	int my_rank = sip_mpi_attr_.global_rank();
	int server_rank = data_distribution_.get_server_rank(target_id);
	int put_tag, put_data_tag;
	put_tag = sip_mpi_attr_.barrier_support_.make_mpi_tags_for_PUT(
			put_data_tag);

//	std::cout << "W" << my_rank << " put_tag: " << put_tag << " section number "
//			<< sip_mpi_attr_.barrier_support_.extract_section_number(put_tag)
//			<< " transaction number "
//			<< sip_mpi_attr_.barrier_support_.extract_transaction_number(
//					put_tag) << " tag type "
//			<< sip_mpi_attr_.barrier_support_.extract_message_type(put_tag)
//			<< std::endl;
//	std::cout << "W" << my_rank << " put_data_tag: " << put_data_tag
//			<< " section number "
//			<< sip_mpi_attr_.barrier_support_.extract_section_number(
//					put_data_tag) << " transaction number "
//			<< sip_mpi_attr_.barrier_support_.extract_transaction_number(
//					put_data_tag) << " tag type "
//			<< sip_mpi_attr_.barrier_support_.extract_message_type(put_data_tag)
//			<< std::endl;
////	SIP_LOG(std::cout<< "W " << sip_mpi_attr_.global_rank() << " : Sending PUT for block with tags "<<put__tag<< " and "<< put_data_tag << ", "<< target << " to server rank " << server_rank << std::endl);
//	std::cout << "W " << my_rank << " : Sending PUT for block with tags "
//			<< put_tag << " and " << put_data_tag << ", " << target_id
//			<< " to server rank " << server_rank << std::endl;

	//note that due to the structure of a blockId, we can just send the first MAX_RANK+1 int elements.
	SIPMPIUtils::check_err(
			MPI_Send(reinterpret_cast<int *>(&target_id), BlockId::mpi_count,
					MPI_INT, server_rank, put_tag, MPI_COMM_WORLD));
	//immediately follow with the data
	SIPMPIUtils::check_err(
			MPI_Send(target_block->data_, target_block->size_, MPI_DOUBLE,
					server_rank, put_data_tag, MPI_COMM_WORLD));
	//remove
	//wait for ack
	MPI_Status status;
	MPI_Recv(0, 0, MPI_INT, server_rank, put_data_tag, MPI_COMM_WORLD, &status);
//	SIP_LOG(
//			std::cout<< "W " << sip_mpi_attr_.global_rank() << " : Done with PUT for block " << lhs_id << " to server rank " << server_rank << std::endl);
//	std::cout << "W " << my_rank << " : Done with PUT for block " << target_id
//			<< " to server rank " << server_rank << std::endl;

	//remove target_block from map, it was used as a convenient buffer.
	//TODO perhaps just allocate a buffer instead?
	delete_block(target_id);

#endif //HAVE_MPI
}

void BlockManager::put_accumulate(BlockId& target_id,
		const Block::BlockPtr source_block) {
#ifdef HAVE_MPI
	int my_rank = sip_mpi_attr_.global_rank();
	Block::BlockPtr target_block = get_block_for_writing(target_id, false); //write locally by copying rhs, accumulate is done at server
	assert(target_block->shape_ == source_block->shape_);
	target_block->copy_data_(source_block);

	int server_rank = data_distribution_.get_server_rank(target_id);

	int put_accumulate_tag, put_accumulate_data_tag;
	put_accumulate_tag =
			sip_mpi_attr_.barrier_support_.make_mpi_tags_for_PUT_ACCUMULATE(
					put_accumulate_data_tag);

//	std::cout << "W" << my_rank << " put_accumulate_tag: " << put_accumulate_tag
//			<< " section number "
//			<< sip_mpi_attr_.barrier_support_.extract_section_number(
//					put_accumulate_tag) << " transaction number "
//			<< sip_mpi_attr_.barrier_support_.extract_transaction_number(
//					put_accumulate_tag) << " tag type "
//			<< sip_mpi_attr_.barrier_support_.extract_message_type(
//					put_accumulate_tag) << std::endl;
//
//	std::cout << "W" << my_rank << " put_accumulate_data_tag:"
//			<< " section number "
//			<< sip_mpi_attr_.barrier_support_.extract_section_number(
//					put_accumulate_data_tag) << " transaction number "
//			<< sip_mpi_attr_.barrier_support_.extract_transaction_number(
//					put_accumulate_data_tag) << " tag type "
//			<< sip_mpi_attr_.barrier_support_.extract_message_type(
//					put_accumulate_data_tag) << std::endl;
//
//	std::cout << "W " << my_rank
//			<< " : Sending PUT_ACCUMULATE for block with tags "
//			<< put_accumulate_tag << " and " << put_accumulate_data_tag << ", "
//			<< target_id << " to server rank " << server_rank;
//	std::cout << " size of block= " << target_block->size_ << std::endl;
//
//	SIP_LOG(
//			std::cout<< "W " << sip_mpi_attr_.global_rank() << " : Sending PUT_ACCUMULATE for block with tags "<< put_accumulate_tag << " and " << put_accumulate_data_tag << ", "<< target_id << " to server rank " << server_rank << std::endl);

	//note that due to the structure of a blockId, we can just send the first MAX_RANK+1 int elements.
	SIPMPIUtils::check_err(
			MPI_Send(reinterpret_cast<int *>(&target_id), BlockId::mpi_count,
					MPI_INT, server_rank, put_accumulate_tag, MPI_COMM_WORLD));
	//immediately follow with the data
	SIPMPIUtils::check_err(
			MPI_Send(target_block->data_, target_block->size_, MPI_DOUBLE,
					server_rank, put_accumulate_data_tag, MPI_COMM_WORLD));
	//wait for ack
//	std::cout << "worker waiting for put_accumulate tag";
	MPI_Status status;
	MPI_Recv(0, 0, MPI_INT, server_rank, put_accumulate_data_tag,
			MPI_COMM_WORLD, &status);

	SIP_LOG(
			std::cout<< "W " << sip_mpi_attr_.global_rank() << " : Done with PUT_ACCUMULATE for block " << lhs_id << " to server rank " << server_rank << std::endl);

	//remove target_block from map, it was used as a convenient buffer.
	//TODO perhaps just allocate a buffer instead?
	delete_block(target_id);
#else
	// Accumulate locally
	Block::BlockPtr target_block = get_block_for_accumulate(target_id, false);
	assert(target_block->shape_ == source_block->shape_);
	target_block->accumulate_data(source_block);
#endif
}

void BlockManager::destroy_served(int array_id) {
	delete_distributed(array_id);
}

void BlockManager::request(BlockId& block_id) {
	get(block_id);
}
void BlockManager::prequest(BlockId&, BlockId&) {
	fail("PREQUEST Not supported !");
}
void BlockManager::prepare(BlockId& lhs_id, Block::BlockPtr source_ptr) {
	put_replace(lhs_id, source_ptr);
}
void BlockManager::prepare_accumulate(BlockId& lhs_id,
		Block::BlockPtr source_ptr) {
	put_accumulate(lhs_id, source_ptr);
}

bool BlockManager::has_wild_value(const BlockId& id) {
	int i = 0;
	while (i != MAX_RANK) {
		if (id.index_values(i) == wild_card_value)
			return true;
		++i;
	}
	return false;
}
bool BlockManager::has_wild_slot(const index_selector_t& selector) {
	int i = 0;
	while (i != MAX_RANK) {
		if (selector[i] == wild_card_slot)
			return true;
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
			delete_block(*it);
		}
	} else {
		delete_block(id);
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
		SIP_LOG(
				std::cout << "W " << sip_mpi_attr_.global_rank() << " : Waiting for block " << id << std::endl);
		i = it->second;
		sip::check(0 <= i && i < MAX_POSTED_ASYNC,
				"Inconsistent value stored in blocks_in_transit_ map !",
				current_line());
		MPI_Request r = posted_async_[i];
		MPI_Status s;
		SIPMPIUtils::check_err(MPI_Wait(&r, &s));
		if (num_posted_async_ > 0) {
			posted_async_[i] = posted_async_[num_posted_async_ - 1];
			num_posted_async_--;
		}
		blocks_in_transit_.erase(id);
		SIP_LOG(
				std::cout << "W " << sip_mpi_attr_.global_rank() << " : Got block " << id << ", was at index :" << i << ", outstanding : "<< num_posted_async_ << std::endl);
	}
}

void BlockManager::free_any_posted_receive() {
	MPI_Status statuses[MAX_POSTED_ASYNC];
	int completed_index = -1;
	SIPMPIUtils::check_err(
			MPI_Waitany(num_posted_async_, posted_async_, &completed_index,
					statuses));
	posted_async_[completed_index] = posted_async_[num_posted_async_ - 1];
	num_posted_async_--;
}

void BlockManager::request_block_from_server(BlockId& id) {
	//get block, create if doesn't exist
	Block* block = get_block_for_writing(id, false);

	//check status--if valid copy of block return
	//if outstanding request, don't request again, just return.

//void BlockManager::request_block_from_server(const BlockId& id) {
//	Block::BlockPtr blk = block(id);
//	int array_id = id.array_id();
//	bool is_remote = sip_tables_.is_distributed(array_id) || sip_tables_.is_served(array_id);
//
//	// Request block from server if not cached
//	if (blk == NULL && is_remote) {
//
//		// The request for this block is already in transit.
//		// Complete that and launch a new one.
//		//wait_for_block_in_transit(id); -> done in call to block()
//
//		// Fetch remote block (not is cache).
//		int server_rank = data_distribution_.get_server_rank(id);
//
//		// Send block id to fetch
//		int get_tag = barrier_support_.make_mpi_tag_for_GET();
//		SIP_LOG(std::cout << "W " << sip_mpi_attr_.global_rank()
//						<< " : Sending GET with tag " << get_tag
//						<< " for block " << id << " to server rank "
//						<< server_rank << std::endl);
//		SIPMPIUtils::check_err(MPI_Send(reinterpret_cast<int *>(id),))
//
//		// Receive block id, shape & size
//		BlockShape shape = sip_tables_.shape(id);
//		int data_size = shape.num_elems();
//
//		// Receive block double precision data
//		blk = new Block(shape);
//		int block_tag = SIPMPIUtils::make_mpi_tag(SIPMPIData::GET_DATA, section_number_, message_number_);
//		//SIPMPIUtils::get_bptr_data_from_rank(server_rank, &block_tag, data_size, blk);
//
//		check(blk != NULL, "Block Pointer into which data is to be received is NULL !", current_line());
//
//		// Free up a slot before posting any more requests
//		if (num_posted_async_ == MAX_POSTED_ASYNC){
//			free_any_posted_receive();
//		}
//
//		MPI_Request request;
//		SIPMPIUtils::check_err(MPI_Irecv(blk->data_, data_size, MPI_DOUBLE, server_rank, block_tag, MPI_COMM_WORLD, &request));
//
//		MPI_Status status;
//		SIPMPIUtils::check_err(MPI_Wait(&request, &status));
//
////		int msg_no = SIPMPIUtils::get_message_number(block_tag);
////		int sect_no = SIPMPIUtils::get_section_number(block_tag);
////		check(msg_no == message_number_, "Message number not consistent in GET !", current_line());
////		check(sect_no == section_number_,"Section number not consistent in GET !", current_line());
//
//		insert_into_blockmap(id, blk);
//
//		// Record this posted receive.
//		posted_async_[num_posted_async_] = request;
//		blocks_in_transit_[id] = num_posted_async_;
//		num_posted_async_++;
//
//		message_number_++;
//
//		SIP_LOG(std::cout << "W " << sip_mpi_attr_.global_rank()
//						<< " : Done with GET for block " << id
//						<< " from server rank " << server_rank << std::endl);
//	}
}

#endif

Block::BlockPtr BlockManager::get_block_for_reading(const BlockId& id) {
	Block::BlockPtr blk = block(id);
	if (blk == NULL) {
		std::cout << "get_block_for_reading, block " << id << "    array "
				<< sip_tables_.array_name(id.array_id()) << " does not exist\n";
		fail("", current_line());
	}

#ifdef HAVE_CUDA
	// Lazy copying of data from gpu to host if needed.
	lazy_gpu_read_on_host(blk);
#endif //HAVE_CUDA
	return blk;
}

/* gets block for reading and writing.  The block should already exist.*/
Block::BlockPtr BlockManager::get_block_for_updating(const BlockId& id) {
	Block::BlockPtr blk = block(id);
	check(blk != NULL, "attempting to update non-existent block",
			current_line());
#ifdef HAVE_CUDA
	// Lazy copying of data from gpu to host if needed.
	lazy_gpu_update_on_host(blk);
#endif
	return blk;
}

/* gets block, creating it if it doesn't exist.  If new, initializes to zero.*/
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
		delete_block(*it);
	}
	temp_block_list_stack_.pop_back();
	delete temps;
}

//void BlockManager::save_persistent_dist_arrays(){
//
//	int num_arrs = block_map_.size();
////	for (BlockMap::iterator it = block_map_.begin(); it != block_map_.end(); ++it){
////		if (pbm_write_.is_array_persistent(it->first)){
////
////			IdBlockMap &bid_map = it->second;
////			pbm_write_.save_dist_array(it->first, bid_map);
////		}
////	}
////	for (int i=0; i<num_arrs; i++){
////		bool is_remote = sip_tables_.is_distributed(i) || sip_tables_.is_served(i);
////		if (is_remote && pbm_write_.is_array_persistent(i)){
////			IdBlockMap<Block>::PerArrayMap *bid_map = block_map_[i];
////			pbm_write_.save_dist_array(i, bid_map);
////			block_map_.delete_per_array_map(i);
////		}
////	}
//}

std::ostream& operator<<(std::ostream& os, const BlockManager& obj) {
	os << "block_map_:" << std::endl;
	os << obj.block_map_ << std::endl;
//	{
//		const int num_arrs = obj.block_map_.size();
//		for (int i=0; i<num_arrs; i++) {
//			if (obj.block_map_[i] != NULL){
//				os << "array [" << i << "]" << std::endl;  //print the array id
//				const IdBlockMap<Block>* bid_map = obj.block_map_[i];
//				IdBlockMap<Block>*::const_iterator it2;
//				for (it2 = bid_map->begin(); it2 != bid_map->end(); ++it2) {
//					check(it2->second != NULL, "Trying to print NULL blockPtr !");
//					SIP_LOG(if(it2->second == NULL) std::cout<<it2->first<<" is NULL!"<<std::endl;);
//					os << it2 -> first << std::endl;
//				}
//				os << std::endl;
//			}
//		}
//	}
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

//Block::BlockPtr BlockManager::block(const BlockId& id) {
//	int array_id = id.array_id_;
//
//	if (block_map_[array_id] == NULL){
//		block_map_[array_id] = new IdBlockMap();
//		//check_and_warn(false, "Could not find distributed array, creating a new one...");
//	}
//
//	//check(&block_map_[array_id] != NULL, "map containing blocks for array is null!", current_line());
//	IdBlockMap *bid_map = block_map_[array_id];
//	IdBlockMap::iterator b = bid_map->find(id);
//	if (b != bid_map->end()) {
//#ifdef HAVE_MPI
//		//Check if this block is in transit and wait for it.
//		wait_for_block_in_transit(id);
//#endif
//		return b->second;
//	}
//	return NULL;
//}

///**
// * It is an error to try to create a new block if a block with that id already exists
// * exists.
// */
//void BlockManager::insert_into_blockmap(const BlockId& block_id, Block::BlockPtr block_ptr) {
//	check(block_ptr != NULL, "Trying to insert NULL block into BlockMap !", current_line());
//	std::pair<IdBlockMap::iterator, bool> ret;
//	int array_id = block_id.array_id_;
//	IdBlockMap *bid_map = block_map_[array_id];
//	std::pair<BlockId, Block::BlockPtr> bid_pair (block_id, block_ptr);
//	ret = bid_map->insert(bid_pair);
//	check(ret.second, std::string("attempting to create block that already exists"));
//}

/** creates a new block with the given Id and inserts it into the block map.
 * It is an error to try to create a new block if a block with that id already exists
 * exists.
 */
Block::BlockPtr BlockManager::create_block(const BlockId& block_id,
		const BlockShape& shape) {
	Block::BlockPtr block_ptr = new Block(shape);
	insert_into_blockmap(block_id, block_ptr);
	return block_ptr;
}

//void BlockManager::remove_block(const BlockId& block_id) {
//	int array_id = block_id.array_id_;
//
////	BlockMap::iterator it = block_map_.find(array_id);
////	check(it != block_map_.end(), "Could not find distributed array to remove block from !");
//
//
//	IdBlockMap *bid_map = block_map_[array_id];
//	check(bid_map != NULL,"Could not find distributed array " + sip_tables_.array_name(array_id) + " to remove block from !", current_line());
//	Block::BlockPtr block_to_remove = bid_map->at(block_id);
//	bid_map->erase(block_id);
//	delete block_to_remove;
//}

void BlockManager::generate_local_block_list(const BlockId& id,
		std::vector<BlockId>& list) {
	std::vector<int> prefix;
	int rank = sip_tables_.array_rank(id);
	int pos = 0;
	gen(id, rank, pos, prefix, 0, list);
}

void BlockManager::gen(const BlockId& id, int rank, const int pos,
		std::vector<int> prefix /*pass by value*/, int to_append,
		std::vector<BlockId>& list) {
	if (pos != 0) {
		prefix.push_back(to_append);
	}
	if (pos < rank) {
		int curr_index = id.index_values(pos);
		if (curr_index == wild_card_value) {
			int index_slot = sip_tables_.selectors(id.array_id())[pos];
			int lower = sip_tables_.lower_seg(index_slot);
			int upper = lower + sip_tables_.num_segments(index_slot);
			for (int i = lower; i < upper; ++i) {
				gen(id, rank, pos + 1, prefix, i, list);
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
Block::BlockPtr BlockManager::get_gpu_block_for_writing(const BlockId& id, bool is_scope_extent) {
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
Block::BlockPtr BlockManager::get_gpu_block_for_updating(const BlockId& id) {
	Block::BlockPtr blk = block(id);
	check(blk != NULL, "attempting to update non-existent block");

	// Lazy copying of data from host to gpu if needed.
	lazy_gpu_update_on_device(blk);

	return blk;
}
Block::BlockPtr BlockManager::get_gpu_block_for_reading(const BlockId& id) {
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

void BlockManager::lazy_gpu_read_on_device(const Block::BlockPtr& blk) {
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

void BlockManager::lazy_gpu_update_on_device(const Block::BlockPtr& blk) {
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
	blk->set_dirty_on_gpu();
}

void BlockManager::lazy_gpu_read_on_host(const Block::BlockPtr& blk) {
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

void BlockManager::lazy_gpu_write_on_host(Block::BlockPtr& blk, const BlockId &id, const BlockShape& shape) {
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

void BlockManager::lazy_gpu_update_on_host(const Block::BlockPtr& blk) {
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

}
 //namespace sip
