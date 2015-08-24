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
		const DataDistribution& data_distribution) :
		sip_tables_(sip_tables), sip_mpi_attr_(sip_mpi_attr), data_distribution_(
				data_distribution), block_map_(sip_tables.num_arrays()), policy_(
				block_map_), max_allocatable_bytes_(
				sip::GlobalState::get_max_server_data_memory_usage()), max_allocatable_doubles_(
				max_allocatable_bytes_ / sizeof(double))
, remaining_doubles_(max_allocatable_doubles_)
,allocated_doubles_(sip_mpi_attr.company_communicator())
,blocks_to_disk_(sip_mpi_attr.company_communicator())
{
}

DiskBackedBlockMap::~DiskBackedBlockMap() {
	//TODO check this
}


//void DiskBackedBlockMap::read_block_from_disk(ServerBlock* block,
//		const BlockId& block_id, size_t block_size) {
//	/** Allocates space for a block, reads block from disk
//	 * into newly allocated space, sets the in_memory flag,
//	 * inserts into block_map_ if needed
//	 */
////	block = allocate_block(block_size, false);
//	block->set_data(allocate_data(block->size(), false));
//
////	server_timer_.start_timer(current_line(), ServerTimer::READTIME);
//	disk_backed_arrays_io_.read_block_from_disk(block_id, block);
////	server_timer_.pause_timer(current_line(), ServerTimer::READTIME);
//	block->disk_state_.set_in_memory();
//}
//
//void DiskBackedBlockMap::write_block_to_disk(const BlockId& block_id,
//		ServerBlock* block) {
////	server_timer_.start_timer(current_line(), ServerTimer::WRITETIME);
//	disk_backed_arrays_io_.write_block_to_disk(block_id, block);
////	server_timer_.pause_timer(current_line(), ServerTimer::WRITETIME);
//	block->disk_state_.set_valid_on_disk();
//	block->disk_state_.set_on_disk();
//}


////returns the number of doubles actually freed
//size_t DiskBackedBlockMap::backup_and_free_doubles(size_t requested_doubles_to_free) {
//	size_t freed_count = 0;
//	try {
//		while (freed_count < requested_doubles_to_free) { //if requested_doubles_to_free <= 0, no iterations performed.
//			ServerBlock* block;
//			BlockId bid = policy_.get_next_block_for_removal(block);
//			block->wait();
//			Chunk& chunk = block->get_chunk();
//
////			check(blk == block, "bug in free_doubles");
////			SIP_LOG(
////					std::cout << "S " << sip_mpi_attr_.company_rank() << " : Freeing block " << bid << " and writing to disk to make space for new block" << std::endl);
////			if (!blk->disk_state_.is_valid_on_disk()) {
////				write_block_to_disk(bid, blk);
////				blocks_to_disk_.inc();
////				blk->disk_state_.set_valid_on_disk();
////				blk->disk_state_.unset_in_memory();
////			}
////			double* data_to_free = blk->get_data();
////			free_data(data_to_free, blk->size()); //this method updates remaining_doubles_
////			blk->data_ = NULL;
////			freed_count += blk->size();
//		}
//	} catch (const std::out_of_range& oor) {
//		//ran out of blocks, just return what was freed.
//	}
//	return freed_count;
//}

size_t DiskBackedBlockMap::backup_and_free_doubles(size_t requested_doubles_to_free)
{
	size_t freed_count = 0;
	try {
		while (freed_count < requested_doubles_to_free) { //if requested_doubles_to_free <= 0, no iterations performed.

	//get a block to remove.  Also, need its containing chunk and array.
			ServerBlock* block;
			BlockId block_id = policy_.get_next_block_for_removal(block);
			int array_id = block_id.array_id();
			Chunk* chunk = block->get_chunk();

			//if the chunk doesn't have any data, just try the next block.  Otherwise
			//wait for pending operations.  If it is not valid on disk, write to disk.
			//Then free the data.
			if (chunk->get_data(0) != NULL){  //get the data for entire chunk--offset is 0
				chunk->wait_all();
				if (!chunk->valid_on_disk_){
					ArrayFile* file = array_files_.at(array_id);
					file->chunk_write_(*chunk);
					chunk->valid_on_disk_=true;
				}
				freed_count += chunk_managers_.at(array_id)->delete_chunk_data(chunk);
			}
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
	size_t to_free = (remaining_doubles_ <= size) ?  size : 0;
	size_t freed = 0;
	double* data = NULL;
	bool allocated = false;
	while (!allocated) {
		freed = backup_and_free_doubles(to_free); //free_doubles simply returns 0 if to_free <= 0
		try {
			if (initialize){
			data = new double[size]();
			}
			else {
				data = new double[size];
			}
			remaining_doubles_ -= size;
			allocated_doubles_.inc(size);
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
	allocated_doubles_.inc(-size);
}


ServerBlock* DiskBackedBlockMap::create_block(int array_id, size_t block_size, bool initialize) {
		int chunk_number;
		Chunk::offset_val_t offset;
		ChunkManager* manager = chunk_managers_.at(array_id);
		size_t allocated = manager->assign_block_data_from_chunk(block_size, initialize, chunk_number, offset);
		ServerBlock* block = new ServerBlock(block_size, manager, chunk_number, offset);
		block->get_chunk()->add_server_block(block);
		remaining_doubles_ -+ allocated;
		allocated_doubles_.inc(allocated);
		return block;
}

ServerBlock* DiskBackedBlockMap::create_block_for_lazy_restore(int array_id, size_t block_size) {
		int chunk_number;
		Chunk::offset_val_t offset;
		ChunkManager* manager = chunk_managers_.at(array_id);
		manager->lazy_assign_block_data_from_chunk(block_size, chunk_number, offset);
		ServerBlock* block = new ServerBlock(block_size, manager, chunk_number, offset);
		block->get_chunk()->add_server_block(block);
		return block;
}

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
		block = create_block(block_id.array_id(), block_size,  true); //blocks for accumulate must be initialize to zero
		block_map_.insert_block(block_id, block);

	} else {
		//note that we do not wait for pending ops to complete when block is to be used in accumulate
		//if here, the block exists.  If not in memory, read data from disk
		bool in_memory = (block->block_data_.get_data() != NULL);
		bool valid_on_disk = block->block_data_.chunk_->valid_on_disk_;
		check(in_memory || valid_on_disk,
				"existing block is neither in memory or on disk");
		if (!in_memory ){
				read_chunk_from_disk(block, block_id);
		}
	}
	block->block_data_.chunk_->valid_on_disk_ = false;
	policy_.touch(block_id);
	return block;
}

//ServerBlock* DiskBackedBlockMap::get_block_for_updating(
//		const BlockId& block_id) {
//	ServerBlock* block = get_block_for_reading(block_id, 0);
//	block->wait(); //get_block_for_reading only waits for pending writers to complete,
//				   //we need to wait for pending readers, too.
//	block->block_data_.chunk_.valid_on_disk_=false;
//	policy_.touch(block_id):
//	return block;
//}

ServerBlock* DiskBackedBlockMap::get_block_for_writing(
		const BlockId& block_id) {

	ServerBlock* block = block_map_.block(block_id);

	if (block == NULL) {
		//create new uninitialized block
		SIP_LOG(
				std::stringstream msg; msg << "S "; msg << sip_mpi_attr_.global_rank(); msg << " : getting uninitialized block " << block_id << ".  Creating zero block for writing"<< std::endl; std::cout << msg.str() << std::flush);
		size_t block_size = sip_tables_.block_size(block_id);
		block = create_block(block_id.array_id(), block_size, false);
		block_map_.insert_block(block_id, block);
	} else {
		bool in_memory = (block->block_data_.get_data() != NULL);
		if (in_memory){
			block->wait();
		}
		else {
			bool valid_on_disk = block->block_data_.chunk_->valid_on_disk_;
			check(valid_on_disk,
					"existing block is neither in memory or on disk");
			read_chunk_from_disk(block, block_id);
		}
	}
	block->block_data_.chunk_->valid_on_disk_=false;
    policy_.touch(block_id);

	return block;
}

ServerBlock* DiskBackedBlockMap::get_block_for_lazy_restore(const BlockId& block_id){
	ServerBlock* block = block_map_.block(block_id);
	check(block == NULL, "attempting to restore block that already exists");
	size_t block_size = sip_tables_.block_size(block_id);
	block = create_block_for_lazy_restore(block_id.array_id(), block_size);
	block_map_.insert_block(block_id, block);
	block->block_data_.chunk_->valid_on_disk_=true;
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

	}
	else {
		bool in_memory = (block->block_data_.get_data() != NULL);
		if (in_memory){
			block->wait_for_writes();
		}
		else {
			bool valid_on_disk = block->block_data_.chunk_->valid_on_disk_;
			check(valid_on_disk,
					"existing block is neither in memory or on disk");
			read_chunk_from_disk(block, block_id);
		}
	}
	policy_.touch(block_id);
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

//IdBlockMap<ServerBlock>::PerArrayMap* DiskBackedBlockMap::get_and_remove_per_array_map(
//		int array_id) {
//	return block_map_.get_and_remove_per_array_map(array_id);
//}

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
//	disk_backed_arrays_io_.delete_array(array_id, per_array_map);// remove from disk
	SIP_LOG(
			std::cout << "S " << sip_mpi_attr_.global_rank() << " : Deleted blocks from disk" << std::endl;);

	size_t bytes_deleted = block_map_.delete_per_array_map_and_blocks(array_id);// remove from memory
	remaining_doubles_ += bytes_deleted/ sizeof(double);
	allocated_doubles_.inc(-(bytes_deleted/sizeof(double)));
	SIP_LOG(
			std::cout << "S " << sip_mpi_attr_.global_rank() << " : Removed blocks from memory" << std::endl;);
}

//void DiskBackedBlockMap::restore_persistent_array(int array_id,
//		std::string& label) {
//	/** Restore file on disk by delegating to disk interface
//	 * For each block of the array that this server owns,
//	 * make an entry with an empty block in the block_map_.
//	 */
//	disk_backed_arrays_io_.restore_persistent_array(array_id, label);
//
//	std::list<BlockId> my_blocks;
//	int this_server_rank = sip_mpi_attr_.global_rank();
//	data_distribution_.generate_server_blocks_list(this_server_rank, array_id,
//			my_blocks, sip_tables_);
//
//	// Clear out all existing blocks of array in memory
//	block_map_.delete_per_array_map_and_blocks(array_id);
//
//	std::list<BlockId>::const_iterator it;
//	for (it = my_blocks.begin(); it != my_blocks.end(); ++it) {
//		int size = sip_tables_.block_size(*it);         // Number of FP numbers
//		double *data = NULL;
//		ServerBlock *sb = new ServerBlock(size, data);// ServerBlock which doesn't "take up any memory"
//		sb->disk_state_.set_on_disk();
//		sb->disk_state_.set_valid_on_disk();
//		block_map_.insert_block(*it, sb);
//	}
//
//}

//void DiskBackedBlockMap::save_persistent_array(int array_id, const std::string& array_label, IdBlockMap<ServerBlock>::PerArrayMap* array_blocks){
//////	IDBlockMap<ServerBlock>::PerArrayMap* array_blocks){
////		ArrayFile* file = array_files_.at(array_id);
////		check(file != NULL, "NULL ArrayFile* in DiskBackedBlockMap::save_persistent_array)";
////		ChunkManager* manager = chunk_managers_.at(array_id);
////		check(manager!= NULL, "NULL ChunkManager* in DiskBackedBlockMap::save_persistent_array)";
////		//collective operation to write chunks to file
////		manager->flush();
////		//construct the local version of the file index
////		size_t num_blocks = sip_tables_.num_blocks(array_id);
////		std::vector<ArrayFile::offset_val_t> index_vals(num_blocks);
////		initialize_local_index(array_blocks, index_vals, num_blocks);
////		//combine local indices from all server, master server writes blocks to file
////		file->write_index(index_vals.data(), num_blocks);
////	}
//}


size_t DiskBackedBlockMap::read_chunk_from_disk(ServerBlock* block,
		const BlockId& block_id){
	size_t newly_allocated = 0;
	ArrayFile* file = array_files_.at(block_id.array_id());
	ChunkManager* manager = chunk_managers_.at(block_id.array_id());
	Chunk* chunk = block->get_chunk();
	ArrayFile::offset_val_t offset = 0;
	if (chunk->get_data(offset)==NULL){
		newly_allocated += manager->reallocate_chunk_data(chunk);
	}
	file->chunk_read(*chunk);
	return newly_allocated;
}

//// Writes chunk containing block to disk and frees in memory data, updates memory accounting chunk disk_back_state
//void DiskBackedBlockMap::write_chunk_to_disk(const BlockId& block_id, ServerBlock* block){
//
//}


/**
 * Precondition:  current number of servers is the same as when the file was written.
 *
 * @param num_blocks
 * @param distribution
 */
void DiskBackedBlockMap::eager_restore_chunks_from_index(int array_id, ArrayFile* file, ChunkManager* manager, ArrayFile::header_val_t num_blocks, const DataDistribution& distribution){
	//read the index from the array file
	ArrayFile::offset_val_t index[num_blocks];
	file->read_index(index, num_blocks);
	ChunkManager::chunk_size_t chunk_size = manager->chunk_size();

	//create a map of blocks in file that belong to this server.  Count the ones that
	// correspond to the beginning of a chunk
	std::map<ArrayFile::offset_val_t, block_num_t> offset_block_map;
	int num_chunks = 0;
	for (int i = 0; i < num_blocks; i++){
	   offset_val_t offset = index[i];
	   if (offset != 0 && distribution.is_my_block(i)){  //block number i has data and belongs to this server
		   offset_block_map[offset]=i;
		   if (offset % chunk_size == 0){ //block is start of chunk
			   num_chunks++;
		   }
	   }
	}

	//determine how many collective reads to perform.
	int max_num_chunks;
	MPI_Allreduce(&num_chunks, &max_num_chunks, 1, MPI_INT, MPI_MAX, file->comm_);


	std::map<offset_val_t, block_num_t>::iterator it;
	int read_count = 0;
	size_t allocated_doubles = 0;
	for (it = offset_block_map.begin(); it != offset_block_map.end(); ++it){
		offset_val_t block_offset = it->first;
		size_t block_num = it->second;
		BlockId block_id = sip_tables_.block_id(array_id, block_num);
		ServerBlock* block = get_block_for_writing(block_id);
		Chunk* chunk = block->get_chunk();
		if (chunk->file_offset_ % chunk_size == 0){
			 file->chunk_read_all(*chunk);
			 chunk->valid_on_disk_=true;
		     read_count++;
		}
	}
	//noops in case other servers still need to read
	for (int j = read_count; j != max_num_chunks; ++j){
		file->chunk_read_all_nop();
	}
}



/**
 * Precondition:  current number of servers is the same as when the file was written.
 *
 * @param num_blocks
 * @param distribution
 */
void DiskBackedBlockMap::lazy_restore_chunks_from_index(int array_id, ArrayFile* file, ChunkManager* manager, ArrayFile::header_val_t num_blocks, const DataDistribution& distribution){
	//read the index from the array file
	ArrayFile::offset_val_t index[num_blocks];
	file->read_index(index, num_blocks);
	size_t chunk_size = manager->chunk_size();

	//create two maps offset to block number for blocks belonging to this server
	//the first map contains offsets that correspond to the beginning of a chunk
	//the second contains offsets that are within a chunk

	std::map<ArrayFile::offset_val_t, block_num_t> offset_block_map;
	int num_chunks = 0;
	for (int block_num = 0; block_num < num_blocks; block_num++){
	   offset_val_t offset = index[block_num];
	   if (offset != 0 && distribution.is_my_block(block_num)){
		   BlockId block_id = sip_tables_.block_id(array_id, block_num);
		   ServerBlock* block = get_block_for_lazy_restore(block_id);
	   }
	}
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

//	os << "disk_backed_arrays_io : " << std::endl;
//	os << obj.disk_backed_arrays_io_;

	os << "policy_ : " << std::endl;
	os << obj.policy_;

	os << std::endl;

	return os;
}

} /* namespace sip */
