/*
 * disk_backed_block_map.cpp
 *
 *  Created on: Apr 17, 2014
 *      Author: njindal
 */

#include "disk_backed_block_map.h"
#include <iostream>
#include <string>
#include <sstream>
#include "server_block.h"
#include "block_id.h"
#include "global_state.h"
#include "sip_server.h"

namespace sip {

/** Template specialization for ServerBlock.
 *
 * Regular LRU Array policy is to evict the
 * "first" block from the least recently used array.
 * For ServerBlocks however, since they are not
 * removed from the BlockMap, but are "emptied out",
 * the regular array policy won't work.
 *
 * This methods first chooses an array whose block
 * is to be evicted. It then finds one, skipping
 * over those blocks which have been "emptied" out.
 *
 * The pointer the the block is returned to avoid another search through the data structure
 */
template<> BlockId LRUArrayPolicy<ServerBlock>::get_next_block_for_removal(ServerBlock*& blockpp) {
	/** Return an arbitrary block from the least recently used array
	 */
	if (lru_list_.empty())
		throw std::out_of_range(
				"No blocks have been touched, yet block requested for flushing");
	while (!lru_list_.empty()) {
		int to_remove_array = lru_list_.back();
		IdBlockMap<ServerBlock>::PerArrayMap* array_map =
				block_map_.per_array_map(to_remove_array);
		IdBlockMap<ServerBlock>::PerArrayMap::iterator it = array_map->begin();
		for (; it != array_map->end(); it++) {
			ServerBlock *blk = it->second;
			if (blk != NULL && blk->get_data() != NULL) {
				blockpp = blk;
				return it->first;
			}
		}
		lru_list_.pop_back();
	}
	throw std::out_of_range(
			"No server blocks to remove - all empty or none present !");
}

DiskBackedBlockMap::DiskBackedBlockMap(const SipTables& sip_tables,
		const SIPMPIAttr& sip_mpi_attr,
		const DataDistribution& data_distribution, ServerTimer& server_timer) :
		sip_tables_(sip_tables), sip_mpi_attr_(sip_mpi_attr), data_distribution_(
				data_distribution), block_map_(sip_tables.num_arrays()), disk_backed_arrays_io_(
				sip_tables, sip_mpi_attr, data_distribution), policy_(
				block_map_), max_allocatable_bytes_(
				sip::GlobalState::get_max_data_memory_usage()), max_allocatable_doubles_(
				max_allocatable_bytes_ / sizeof(double)), remaining_doubles_(
				max_allocatable_doubles_), server_timer_(server_timer) {
}

DiskBackedBlockMap::~DiskBackedBlockMap() {
}

void DiskBackedBlockMap::read_block_from_disk(ServerBlock*& block,
		const BlockId& block_id, size_t block_size) {
	/** Allocates space for a block, reads block from disk
	 * into newly allocated space, sets the in_memory flag,
	 * inserts into block_map_ if needed
	 */
	block = allocate_block(block_size, false);
//	server_timer_.start_timer(current_line(), ServerTimer::READTIME);
	disk_backed_arrays_io_.read_block_from_disk(block_id, block);
//	server_timer_.pause_timer(current_line(), ServerTimer::READTIME);
	block->disk_state_.set_in_memory();
}

void DiskBackedBlockMap::write_block_to_disk(const BlockId& block_id,
		ServerBlock* block) {
//	server_timer_.start_timer(current_line(), ServerTimer::WRITETIME);
	disk_backed_arrays_io_.write_block_to_disk(block_id, block);
//	server_timer_.pause_timer(current_line(), ServerTimer::WRITETIME);
	block->disk_state_.set_valid_on_disk();
	block->disk_state_.set_on_disk();
}


//returns the number of doubles actually freed
size_t DiskBackedBlockMap::backup_and_free_doubles(size_t requested_doubles_to_free) {
	size_t freed_count = 0;
	try {
		while (freed_count < requested_doubles_to_free) { //if requested_doubles_to_free <= 0, no iterations performed.
			ServerBlock* block;
			BlockId bid = policy_.get_next_block_for_removal(block);
			ServerBlock* blk = block_map_.block(bid);
			check(blk == block, "bug in free_doubles");
			SIP_LOG(
					std::cout << "S " << sip_mpi_attr_.company_rank() << " : Freeing block " << bid << " and writing to disk to make space for new block" << std::endl);
			if (!blk->disk_state_.is_valid_on_disk()) {
				write_block_to_disk(bid, blk);
				blk->disk_state_.set_valid_on_disk();
			}
			double* data_to_free = blk->get_data();
			free_data(data_to_free, blk->size()); //this method updates remaining_doubles_
			freed_count += blk->size();
		}
	} catch (const std::out_of_range& oor) {
		//ran out of blocks, just return what was freed.
	}
	return freed_count;
}

//this allocates the requested number of double precision values,
//backing up to disk if necessary.
//updates remaining_doubles_
double* DiskBackedBlockMap::allocate_data(size_t size, bool initialize){

	size_t to_free = (remaining_doubles_ - size > 0) ? 0 : size;
	size_t freed = 0;
	double* data = NULL;
	bool allocated = false;
	while (!allocated) {
		freed = backup_and_free_doubles(to_free); //free_doubles simply returns if to_free <= 0
		try {
			if (initialize){
			data = new double[size]();
			}
			else {
				data = new double[size];
			}
			remaining_doubles_ -= size;
			allocated = true;
		}
		catch (const std::bad_alloc& ba) {
			//memory allocation failed, try to free more blocks and try again
			//unless freed < to_free, then just give up
			if (freed < to_free){
				fail(" Could not requested amount of memory and allocate failed");
			}
			to_free = size; //in case it was zero
		}
	}
	return data;
}
//this routine simply deallocates memory
//and properly updates the remaining memory count.
void DiskBackedBlockMap::free_data(double*& data, size_t size){
	delete[] data;
	data = NULL;
	remaining_doubles_ += size;
}


ServerBlock* DiskBackedBlockMap::allocate_block(size_t block_size, bool initialize) {
		double* data = allocate_data(block_size, initialize);
		return new ServerBlock(block_size, data);
}

//	bool allocated = false;
//	size_t to_free = block_size - remaining_doubles_;
//	size_t freed = 0;
//	while (!allocated) {
//		freed = free_doubles(to_free);
//		try {
//			if (block == NULL) {
//				block = new ServerBlock(block_size, initialize);
//				block->disk_state_.disk_status_[DiskBackingState::IN_MEMORY] = true;
//				block->disk_state_.disk_status_[DiskBackingState::ON_DISK] = false;
//				block->disk_state_.disk_status_[DiskBackingState::VALID_ON_DISK] = false;
//			} else {
//				check(block->get_data() == NULL, "memory leak, allocating data for block with existing");
//				double* data = allocate_block_data(block_size, initialize);
//				block->set_data(data);
//			}
//			remaining_doubles_ -= block_size;
//			allocated = true;
//		}
//		catch (const std::bad_alloc& ba) {
//			//memory allocation failed, try to free more blocks and try again
//			//unless freed < to_free, then just give up
//			if (freed < to_free){
//				fail(" Could not allocate ServerBlock, out of memory");
//			}
//		}
//	}
//	return block;
//}


//double* DiskBackedBlockMap::allocate_block_data(
//		size_t block_size, bool initialize) {
//	bool allocated = false;
//	size_t to_free = block_size - remaining_doubles_;
//	size_t freed = 0;
//	double* data = NULL;
//	while (!allocated) {
//		freed = free_doubles(to_free);
//		try {
//			if (initialize){
//			data = new double[block_size]();
//			}
//			else {
//				data = new double[block_size];
//			}
//			remaining_doubles_ -= block_size;
//			allocated = true;
//		}
//		catch (const std::bad_alloc& ba) {
//			//memory allocation failed, try to free more blocks and try again
//			//unless freed < to_free, then just give up
//			if (freed < to_free){
//				fail(" Could not allocate data for block, out of memory");
//			}
//		}
//	}
//	return data;
//}

//ServerBlock* DiskBackedBlockMap::allocate_block(ServerBlock* block,
//		size_t block_size, bool initialize) {
//	/** If enough memory remains, allocates block and returns.
//	 * Otherwise, frees up memory by writing out dirty blocks
//	 * till enough memory has been obtained, then allocates
//	 * and returns block.
//	 */
////	std::size_t remaining_mem = max_allocatable_bytes_
////			- ServerBlock::allocated_bytes();
////
////	while (block_size * sizeof(double) > remaining_mem) {
//	while (block_size > remaining_doubles_)
//		try {
//			BlockId bid = policy_.get_next_block_for_removal();
//			ServerBlock* blk = block_map_.block(bid);
//			SIP_LOG(
//					std::cout << "S " << sip_mpi_attr_.company_rank() << " : Freeing block " << bid << " and writing to disk to make space for new block" << std::endl);
//			if (blk->disk_state_.is_dirty()) {
//				write_block_to_disk(bid, blk);
//			}
//			blk->free_in_memory_data();
//			remaining_doubles_+= blk->size();
////			if (!(remaining_mem
////					< max_allocatable_bytes_ - ServerBlock::allocated_bytes())) {
////				throw std::out_of_range("Break now.");
////			}
////			remaining_mem = max_allocatable_bytes_
////					- ServerBlock::allocated_bytes();
//
////		} catch (const std::out_of_range& oor) {
////			std::cerr << " In DiskBackedBlockMap::allocate_block" << std::endl;
////			std::cerr << oor.what() << std::endl;
////			std::cerr << *this << std::endl;
////			fail(
////					" Something got messed up in the internal data structures of the Server",
////					current_line()
////					);
////			check(false, " Something got messed up in the internal data structures of the Server");
//		} catch (const std::bad_alloc& ba) {
//			std::cerr << " In DiskBackedBlockMap::allocate_block" << std::endl;
//			std::cerr << ba.what() << std::endl;
//			std::cerr << *this << std::endl;
//			check(false," Could not allocate ServerBlock, out of memory" );
////			fail(" Could not allocate ServerBlock, out of memory",
////					current_line());
//		}
//	}
//
//	if ((block_size <= remaining_doubles_) {
//		if (block == NULL) {
//			block = new ServerBlock(block_size, initialize);
//		} else {
//			block->allocate_in_memory_data(initialize);
//		}
//		remaining_doubles_ -= block_size;
//	} else { // can't allocate block
//		std::stringstream ss;
//		ss << "S ";
//		ss << sip_mpi_attr_.company_rank();
//		ss << " : Could not allocate memory for block of size ";
//		ss << block_size;
//		ss << ", Memory being used :";
//		ss << ServerBlock::allocated_bytes();
//		ss << std::endl;
//		sip::check(fail, ss.str());
//	}
//
//	return block;
//}



ServerBlock* DiskBackedBlockMap::get_block_for_accumulate(
		const BlockId& block_id) {
	/** If block is not in block map, allocate  it
	 * Otherwise, if the block is in memory, read and return
	 * if it is only on disk, read it in, store in block map and return.
	 * set in_memory and dirty_flag
	 */
	ServerBlock* block = block_map_.block(block_id);

	if (block == NULL) {
		//create block with data initialized to zero and insert in map
		SIP_LOG(
		std::stringstream msg;
		msg << "S " << sip_mpi_attr_.global_rank();
		msg << " : getting uninitialized block " << block_id
		<< ".  Creating zero block for accumulating " << std::endl;
		std::cout << msg.str() << std::flush;)

		size_t block_size = sip_tables_.block_size(block_id);
		block = allocate_block(block_size, true); //blocks for accumulate must be initialize to zero
		block_map_.insert_block(block_id, block);
		block->disk_state_.unset_on_disk();
	} else {
		//note that we do not wait for pending ops to complete when block is to be used in accumulate
		//if here, the block exists.  If not in memory, read data from disk
		check(block->disk_state_.is_in_memory() || block->disk_state_.is_valid_on_disk(),
				"existing block is neither in memory or on disk");
		if (!block->disk_state_.is_in_memory()) {
				read_block_from_disk(block, block_id, block->size());
		}
	}

	block->disk_state_.set_in_memory();
	block->disk_state_.unset_valid_on_disk();
	policy_.touch(block_id);
	return block;
}

ServerBlock* DiskBackedBlockMap::get_block_for_updating(
		const BlockId& block_id) {
	ServerBlock* block = get_block_for_reading(block_id, 0);
	block->wait(); //get_block_for_reading only waits for pending writers to complete,
				   //we need to wait for pending readers, too.
	block->disk_state_.unset_valid_on_disk();
	return block;
}

ServerBlock* DiskBackedBlockMap::get_block_for_writing(
		const BlockId& block_id) {

	ServerBlock* block = block_map_.block(block_id);

	if (block == NULL) {
		//create new uninitialized block
		SIP_LOG(
				std::stringstream msg; msg << "S "; msg << sip_mpi_attr_.global_rank(); msg << " : getting uninitialized block " << block_id << ".  Creating zero block for writing"<< std::endl; std::cout << msg.str() << std::flush);
		size_t block_size = sip_tables_.block_size(block_id);
		block = allocate_block(block_size, false);
		block_map_.insert_block(block_id, block);
		block->disk_state_.unset_on_disk();
	} else {
		block->wait(); //if involved in-progress asynchronous communication, wait for it to complete.
		if (!block->disk_state_.is_in_memory()){
			//allocate data for the block
			block->set_data(allocate_data(block->size(), false));
		}
	}

	block->disk_state_.set_in_memory();
	block->disk_state_.unset_valid_on_disk();

	policy_.touch(block_id);

	return block;
}

ServerBlock* DiskBackedBlockMap::get_block_for_reading(const BlockId& block_id,
		int pc) {
	/** If block is not in block map, there is an error !!
	 * Otherwise, if the block is in memory, read and return or
	 * if it is only on disk, read it in, store in block map and return.
	 * Set in_memory flag.
	 */
	ServerBlock* block = block_map_.block(block_id);
	if (block == NULL) {
		// Error !
		std::stringstream errmsg;
		errmsg << " S " << sip_mpi_attr_.global_rank();
		errmsg << " : Asking for block " << block_id
		<< ". It has not been put/prepared before !  ";

		sial_check(false, errmsg.str(),
				SIPServer::global_sipserver->line_number(pc));

//		// WARNING DISABLED !
//		if (false){
//			std::stringstream msg;
//			msg << "S " << sip_mpi_attr_.global_rank();
//			msg << " : getting uninitialized block " << block_id << ".  Creating zero block "<< std::endl;
//			std::cout << msg.str() << std::flush;
//			block = allocate_block(NULL, block_size,true);
//			block_map_.insert_block(block_id, block);
//		}

	} else {
		block->wait_for_writes(); //if involved in-progress asynchronous communication, wait for ops that may modify to complete.
		check(block->disk_state_.is_in_memory() || block->disk_state_.is_valid_on_disk(),
				"block is not valid in either memory or disk");
		if (!block->disk_state_.is_in_memory()) {
			read_block_from_disk(block, block_id, block->size());
		}
	}
    block->disk_state_.set_in_memory();
	policy_.touch(block_id);

	sip::check(block != NULL,
			"Block is NULL in Server get_block_for_reading, should not happen !");
	return block;
}

//double* DiskBackedBlockMap::get_temp_buffer_for_put_accumulate(const BlockId& block_id, size_t& block_size){
//	block_size = sip_tables_.block_size(block_id);
//	return new double[block_size];
//}

IdBlockMap<ServerBlock>::PerArrayMap* DiskBackedBlockMap::per_array_map(
		int array_id) {
	return block_map_.per_array_map(array_id);
}

IdBlockMap<ServerBlock>::PerArrayMap* DiskBackedBlockMap::get_and_remove_per_array_map(
		int array_id) {
	return block_map_.get_and_remove_per_array_map(array_id);
}

void DiskBackedBlockMap::insert_per_array_map(int array_id,
		IdBlockMap<ServerBlock>::PerArrayMap* map_ptr) {
	block_map_.insert_per_array_map(array_id, map_ptr);
}

void DiskBackedBlockMap::delete_per_array_map_and_blocks(int array_id) {
	policy_.remove_all_blocks_for_array(array_id); // remove from block replacement policy
	SIP_LOG(
			std::cout << "S " << sip_mpi_attr_.global_rank() << " : Removed blocks from block replacement policy" << std::endl;);

	IdBlockMap<ServerBlock>::PerArrayMap *per_array_map =
	block_map_.per_array_map(array_id);
	disk_backed_arrays_io_.delete_array(array_id, per_array_map);// remove from disk
	SIP_LOG(
			std::cout << "S " << sip_mpi_attr_.global_rank() << " : Deleted blocks from disk" << std::endl;);

	block_map_.delete_per_array_map_and_blocks(array_id);// remove from memory
	SIP_LOG(
			std::cout << "S " << sip_mpi_attr_.global_rank() << " : Removed blocks from memory" << std::endl;);
}

void DiskBackedBlockMap::restore_persistent_array(int array_id,
		std::string& label) {
	/** Restore file on disk by delegating to disk interface
	 * For each block of the array that this server owns,
	 * make an entry with an empty block in the block_map_.
	 */
	disk_backed_arrays_io_.restore_persistent_array(array_id, label);

	std::list<BlockId> my_blocks;
	int this_server_rank = sip_mpi_attr_.global_rank();
	data_distribution_.generate_server_blocks_list(this_server_rank, array_id,
			my_blocks, sip_tables_);

	// Clear out all existing blocks of array in memory
	block_map_.delete_per_array_map_and_blocks(array_id);

	std::list<BlockId>::const_iterator it;
	for (it = my_blocks.begin(); it != my_blocks.end(); ++it) {
		int size = sip_tables_.block_size(*it);         // Number of FP numbers
		double *data = NULL;
		ServerBlock *sb = new ServerBlock(size, data);// ServerBlock which doesn't "take up any memory"
		sb->disk_state_.set_on_disk();
		sb->disk_state_.set_valid_on_disk();
		block_map_.insert_block(*it, sb);
	}

}

void DiskBackedBlockMap::save_persistent_array(const int array_id,
		const std::string& array_label,
		IdBlockMap<ServerBlock>::PerArrayMap* array_blocks) {
	server_timer_.start_timer(current_line(), ServerTimer::WRITETIME);
	disk_backed_arrays_io_.save_persistent_array(array_id, array_label,
			array_blocks);
	server_timer_.pause_timer(current_line(), ServerTimer::WRITETIME);

}

//void DiskBackedBlockMap::set_max_allocatable_bytes(std::size_t size) {
//	static bool done_once = false;
//	if (!done_once) {
//		done_once = true;
//		max_allocatable_bytes_ = size;
//	} else {
//		sip::fail("Already set memory limit once !");
//	}
//}

std::ostream& operator<<(std::ostream& os, const DiskBackedBlockMap& obj) {
	os << "block map : " << std::endl;
	os << obj.block_map_;

	os << "disk_backed_arrays_io : " << std::endl;
	os << obj.disk_backed_arrays_io_;

	os << "policy_ : " << std::endl;
	os << obj.policy_;

	os << std::endl;

	return os;
}

} /* namespace sip */
