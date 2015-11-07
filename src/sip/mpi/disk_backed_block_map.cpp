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
#include "job_control.h"
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

const int DiskBackedBlockMap::BLOCKS_PER_CHUNK=4;


DiskBackedBlockMap::DiskBackedBlockMap(const SipTables& sip_tables,
		const SIPMPIAttr& sip_mpi_attr,
		const DataDistribution& data_distribution) :
		sip_tables_(sip_tables), sip_mpi_attr_(sip_mpi_attr), data_distribution_(
				data_distribution), block_map_(sip_tables.num_arrays()), policy_(
				block_map_), max_allocatable_bytes_(

				sip::JobControl::global->get_max_server_data_memory_usage()), max_allocatable_doubles_(
				max_allocatable_bytes_ / sizeof(double))
, remaining_doubles_(max_allocatable_doubles_)
,stats_(sip_mpi_attr.company_communicator(), this)
{
   _init();

}

DiskBackedBlockMap::~DiskBackedBlockMap() {
   _finalize();
}


ServerBlock* DiskBackedBlockMap::get_block_for_reading(const BlockId& block_id,
		int pc) {
	/** If block is not in block map, there is an error !!
	 * Otherwise, if the block is in memory, read and return.
	 * If it is only on disk, read it in, store in block map and return.
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
		else if (block->block_data_.chunk_->valid_on_disk_){
			read_chunk_from_disk(block, block_id);
		}
		else {
			check(false,
					"existing block is neither in memory or on disk" + block_id.str(sip_tables_));
		}
	}
	policy_.touch(block_id);
	return block;

}



ServerBlock* DiskBackedBlockMap::get_block_for_writing(
		const BlockId& block_id) {

	ServerBlock* block = block_map_.block(block_id);

	if (block == NULL) {
		//create new uninitialized block
		size_t block_size = sip_tables_.block_size(block_id);
		block = create_block(block_id.array_id(), block_size, false);
		block_map_.insert_block(block_id, block);
	} else {
		bool in_memory = (block->block_data_.get_data() != NULL);
		if (in_memory){
			block->wait();
		}
		else if (block->block_data_.chunk_->valid_on_disk_){
			    //block is on disk, we read the chunk it belongs to before invalidating the disk copy.
			    //we need to do this because of other blocks in the chunk
			read_chunk_from_disk(block, block_id);
		}
		else {
			check(false, "existing block is neither in memory or on disk " + block_id.str(sip_tables_));
		}
	}
	block->block_data_.chunk_->valid_on_disk_=false;
    policy_.touch(block_id);

	return block;
}

ServerBlock* DiskBackedBlockMap::get_block_for_accumulate(
		const BlockId& block_id) {
	/** If block is not in block map, allocate  it and initialize to 0
	 * Otherwise, if the block is in memory, read and return.
	 * If it is only on disk, read it in, store in block map and return.
	 * Invalidate disk copy
	 */
	ServerBlock* block = block_map_.block(block_id);

	if (block == NULL) {
		//create block with data initialized to zero and insert in map
		size_t block_size = sip_tables_.block_size(block_id);
		block = create_block(block_id.array_id(), block_size,  true); //blocks for accumulate must be initialize to zero
		block_map_.insert_block(block_id, block);


	} else {
		//note that we do not wait for pending ops to complete when block is to be used in accumulate
		//if here, the block exists.  If not in memory, read data from disk
		bool in_memory = (block->block_data_.get_data() != NULL);
		if (in_memory){
			block->wait();
		}
		else if (block->block_data_.chunk_->valid_on_disk_){
			    //block is on disk, we read the chunk it belongs to before invalidating the disk copy.
			    //we need to do this because of other blocks in the chunk
			read_chunk_from_disk(block, block_id);
		}
		else {
			check(false, "existing block is neither in memory or on disk " + block_id.str(sip_tables_));
		}
	}
	block->block_data_.chunk_->valid_on_disk_ = false;
	policy_.touch(block_id);
	return block;
}



double* DiskBackedBlockMap::allocate_data(size_t size, bool initialize){
	size_t to_free = (remaining_doubles_ <= size) ?  size : 0;

	size_t freed = 0;
	double* data = NULL;
	bool allocated = false;
	while (!allocated) {

		freed = backup_and_free_doubles(to_free); //returns 0 if to_free <= 0

		try {
			if (initialize){
			data = new double[size]();
			}
			else {
				data = new double[size];
			}
			remaining_doubles_ -= size;

			stats_.allocated_doubles_.inc(size);

			allocated = true;
		}
		catch (const std::bad_alloc& ba) {
			//memory allocation failed, even though there are apparently enough blocks
			// according to remaining_doubles_.
			//Just free more blocks and try again
			//However, if freed < to_free, then just give up
			if (freed < to_free){
				fail(" Could not free requested amount of memory and allocate failed");
			}
			to_free = size; //in case it was zero
		}
	}
	return data;
}


void DiskBackedBlockMap::free_data(double*& data, size_t size){
	delete[] data;
	data = NULL;
	remaining_doubles_ += size;

	stats_.allocated_doubles_.inc(-size);

}

IdBlockMap<ServerBlock>::PerArrayMap* DiskBackedBlockMap::per_array_map_or_null(int array_id){
	return block_map_.per_array_map_or_null(array_id);
}

void DiskBackedBlockMap::delete_per_array_map_and_blocks(int array_id) {
	 // remove from block replacement policy object
	policy_.remove_all_blocks_for_array(array_id);
	// remove from memory and block_map
	IdBlockMap<ServerBlock>::PerArrayMap *per_array_map =
	block_map_.per_array_map(array_id);
	size_t bytes_deleted = block_map_.delete_per_array_map_and_blocks(array_id);
	//update memory usage accounting
	remaining_doubles_ += bytes_deleted/ sizeof(double);
	stats_.allocated_doubles_.inc(-(bytes_deleted/sizeof(double)));
}



void DiskBackedBlockMap::save_persistent_array(int array_id, const std::string& label){
	ArrayFile* file = array_files_.at(array_id);
	if (file->was_restored_){
		check (label == file->label_, "marking restored array persistent with non-matching label");
		return;
	}

	ChunkManager* manager = chunk_managers_.at(array_id);
	size_t num_blocks = sip_tables_.num_blocks(array_id);
	//mark the file as persistent
	file->mark_persistent(label);
	//save blocks with a collective operation
	manager->collective_flush();
	bool is_dense = false;  //hard code this for now
	//construct the local index
	if(is_dense){
	std::vector<ArrayFile::offset_val_t> index_vals(num_blocks,ArrayFile::ABSENT_BLOCK_OFFSET);
	initialize_local_index(array_id, index_vals, num_blocks);
	//write index to file (reduce local indices, server master writes)
	file->write_dense_index(index_vals);
	}
	else {
		std::vector<ArrayFile::offset_val_t> index_vals;
	initialize_local_sparse_index(array_id, index_vals);
	file->write_sparse_index(index_vals);
	file->close_and_rename_persistent();
	}
}


void DiskBackedBlockMap::restore_persistent_array(int array_id, const std::string & label, bool eager, int pc){
    ArrayFile* file = array_files_.at(array_id);
	ArrayFile::header_val_t expected_chunk_size = (sip_tables_.max_block_size(array_id)) * BLOCKS_PER_CHUNK;
	ArrayFile::offset_val_t index_type;
	ArrayFile::offset_val_t num_blocks;
	std::vector<ArrayFile::offset_val_t> index_file_data;
	file->open_persistent_file(label, index_type, num_blocks, index_file_data);
	ChunkManager* manager = chunk_managers_.at(array_id);
	//const std::string& label, offset_val_t& index_type, offset_val_t& num_blocks, std::vector<offset_val_t>& index
	if(eager && index_type == ArrayFile::DENSE_INDEX){
		eager_restore_chunks_from_index(array_id, file, index_file_data, manager, num_blocks, data_distribution_);
	}
	else if (index_type == ArrayFile::DENSE_INDEX){
		check(false, "lazy restore persistent not yet implemented");
	}
	else if (eager && index_type == ArrayFile::SPARSE_INDEX){
		eager_restore_chunks_from_sparse_index(array_id, file, index_file_data, manager, data_distribution_);
	}
	else check(false, "illegal value for index_file_data type");
}

//	//get the existing file, map, and chunk_manager for this array.
//	ArrayFile* old_file = array_files_.at(array_id);  //this is the file that was created during initialization
//	IdBlockMap<ServerBlock>::PerArrayMap* old_map = block_map_.per_array_map_or_null(array_id);
//	ChunkManager* old_manager = chunk_managers_.at(array_id);
//	if (old_map != NULL && old_map->size() > 0){
//		//blocks have already been created for this array.
//		//If any have been backed to disk, using the old file and old manager,
//		//restore them now and update memory accounting.
//			stats_.num_restored_arrays_with_disk_backing_.inc();
//		    size_t allocated = old_manager->restore();
//	        remaining_doubles_ -= allocated;
//		    stats_.allocated_doubles_.inc(allocated);
//
//	}
//
//	//Create a new ArrayFile object, map, and chunk manager
//	size_t num_blocks = sip_tables_.num_blocks(array_id);
//	ArrayFile::header_val_t expected_chunk_size = (sip_tables_.max_block_size(array_id)) * BLOCKS_PER_CHUNK;
//	//ArrayFile constructor checks that chunk size is as expected.
//	ArrayFile* file = new ArrayFile(num_blocks, expected_chunk_size, label, SIPMPIAttr::get_instance().company_communicator(), false /*file must exist*/);
//	array_files_.at(array_id) = file;
//	ChunkManager* manager = new ChunkManager(expected_chunk_size, file);
//	chunk_managers_.at(array_id) = manager;
//	IdBlockMap<ServerBlock>::PerArrayMap* new_map = new std::map<BlockId, ServerBlock*>;
//	block_map_.update_per_array_map(array_id, new_map);
//
//	//restore chunks from the persistent file
//	if(eager){
//	eager_restore_chunks_from_index(array_id, file, manager, num_blocks, data_distribution_);
//	}
//	else {
//		check(false, "lazy restore not yet implemented");
//	}
//
//
//	//traverse old map.  For each block in old map, check whether it was restored from persistent file.
//	//if not, copy its data.
//	if (old_map != NULL && old_map->size() > 0){
//	  IdBlockMap<ServerBlock>::PerArrayMap::iterator it;
//		for (it = old_map->begin(); it != old_map->end(); ++it){
//			BlockId id = it->first;
//			ServerBlock* block = block_map_.block(id);
//			if (block == NULL){
//				//old existing block has not been overwritten
//				//create new block in new data structures and copy data from old
//				block = get_block_for_writing(id);
//				block->copy_data(it->second);
//			}
//		}
//	}
//
//	if (old_map != NULL) {
//		IdBlockMap<ServerBlock>::delete_blocks_from_per_array_map(old_map);
//		delete old_map;
//	}
//
//	if (old_manager != NULL) {
//		old_manager->delete_chunk_data_all();
//	    delete old_manager;
//	}
//	if (old_file != NULL) {
//		delete old_file;
//	}
//}

void DiskBackedBlockMap::_init(){
	//create manager and open file for each distributed or served array
	int num_arrays = sip_tables_.num_arrays();
	chunk_managers_.resize(num_arrays,NULL);
	array_files_.resize(num_arrays,NULL);
	disk_backing_.resize(num_arrays,false);

	const MPI_Comm& comm = SIPMPIAttr::get_instance().company_communicator();


	for( int i = 0; i < num_arrays; ++i){
		if (sip_tables_.is_distributed(i) || sip_tables_.is_served(i)){
			ArrayFile::header_val_t num_blocks = sip_tables_.num_blocks(i);
			size_t max_block_size = sip_tables_.max_block_size(i);
			ArrayFile::header_val_t chunk_size = max_block_size * BLOCKS_PER_CHUNK; // TODO handle this better
			std::string name = sip_tables_.array_name(i);
			array_files_[i] = new ArrayFile(chunk_size, name, comm);
			chunk_managers_[i] = new ChunkManager(chunk_size, array_files_[i]);
		}
	}
}

void DiskBackedBlockMap::_finalize(){
	for (int i = 0; i < sip_tables_.num_arrays(); ++i){
		if (chunk_managers_[i] != NULL){
			ChunkManager* manager = chunk_managers_[i];
			remaining_doubles_+= manager->delete_chunk_data_all();
			delete manager;
			delete array_files_[i];
		}
	}
}

ServerBlock* DiskBackedBlockMap::create_block(int array_id, size_t block_size, bool initialize) {
		int chunk_number;
		Chunk::offset_val_t offset;
		ChunkManager* manager = chunk_managers_.at(array_id);
		size_t allocated = manager->assign_block_data_from_chunk(block_size, initialize, chunk_number, offset);
		ServerBlock* block = new ServerBlock(block_size, manager, chunk_number, offset);
		block->get_chunk()->add_server_block(block);
		remaining_doubles_ -+ allocated;
		stats_.allocated_doubles_.inc(allocated);
		return block;
}

ServerBlock* DiskBackedBlockMap::get_block_for_restore(BlockId block_id, int array_id, size_t block_size){
	ServerBlock* block = block_map_.block(block_id);
	if (block == NULL){
	block = create_block(array_id, block_size, false);
	block_map_.insert_block(block_id, block);
	}
	else {
		block->wait();
	}
    policy_.touch(block_id);
	return block;
}

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


size_t DiskBackedBlockMap::backup_and_free_doubles(size_t requested_doubles_to_free)
{
	size_t freed_count = 0;
	try {
		while (freed_count < requested_doubles_to_free) { //if requested_doubles_to_free <= 0, no iterations performed.
	//get a block to remove.  Also, we need its containing chunk and array.
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
					file->chunk_write(*chunk);
					chunk->valid_on_disk_=true;
					disk_backing_[array_id]=true;
				}
				freed_count += chunk_managers_.at(array_id)->delete_chunk_data(chunk);
			}
		}
	} catch (const std::out_of_range& oor) {
		//ran out of blocks, just return the number of doubles freed.
	}
	return freed_count;
}


void DiskBackedBlockMap::initialize_local_index(int array_id, std::vector<ArrayFile::offset_val_t>& index_vals, size_t num_blocks){
//	std::cerr << "in initialize_local_index:  num_blocks=" << num_blocks << std::endl;
	IdBlockMap<ServerBlock>::PerArrayMap* array_blocks = block_map_.per_array_map(array_id);
	check (array_blocks->size()<=num_blocks, "mismatch between number of blocks in map and num_blocks");
	IdBlockMap<ServerBlock>::PerArrayMap::iterator it;
	//iterate over blocks in map
	for (it = array_blocks->begin(); it!=array_blocks->end(); ++it){
		//get id of block and linearize it
		BlockId block_id = it->first;
//		std::cerr << "adding block_id " << block_id << " to local index." << std::endl << std::flush;
		int block_num = sip_tables_.block_number(it->first);
//		std::cerr <<" adding offset for block " << block_num << std::endl;
		ServerBlock* block = it->second;
		ArrayFile::offset_val_t offset = block->block_data_.file_offset();
		index_vals.at(block_num) = offset;
	}
//	std::cerr << std::flush;
}

void DiskBackedBlockMap::initialize_local_sparse_index(int array_id, std::vector<ArrayFile::offset_val_t>& index_vals){
	IdBlockMap<ServerBlock>::PerArrayMap* array_blocks = block_map_.per_array_map(array_id);
	IdBlockMap<ServerBlock>::PerArrayMap::iterator it;
	//iterate over blocks in map
//	std::cerr << "CONSTRUCTING LOCAL SPARSE INDEX" << std::endl <<  std::endl;
	for (it = array_blocks->begin(); it!=array_blocks->end(); ++it){
		//get id of block and linearize it
		BlockId block_id = it->first;
		int block_num = sip_tables_.block_number(it->first);
		ServerBlock* block = it->second;
		ArrayFile::offset_val_t offset = block->block_data_.file_offset();
		index_vals.push_back(block_num);
		index_vals.push_back(offset);
//		std::cerr << block_id << ", " << block_num << ", " << offset << std::endl << std::flush;
	}
//	std::cerr << "PRINTING LOCAL SPARSE INDEX" << std::endl <<  std::endl;
//	std::vector<ArrayFile::offset_val_t>::iterator it2;
//	for (it2 = index_vals.begin(); it2 != index_vals.end(); ++it2){
//		std::cerr << *it2 << ",";
//	}
//	std::cerr << std::endl << std::flush;;
}


void DiskBackedBlockMap::eager_restore_chunks_from_index(int array_id, ArrayFile* file, std::vector<ArrayFile::offset_val_t>& index, ChunkManager* manager, ArrayFile::header_val_t num_blocks, const DataDistribution& distribution){
	ChunkManager::chunk_size_t chunk_size = manager->chunk_size();
	int rank = SIPMPIAttr::get_instance().company_rank();
	//create a map of blocks in file that belong to this server.  Count the ones that
	// correspond to the beginning of a chunk
	std::map<ArrayFile::offset_val_t, block_num_t> offset_block_map;
	int num_chunks = 0;
	for (int i = 0; i < num_blocks; i++){
	   offset_val_t offset = index[i];
	   if (offset != ArrayFile::ABSENT_BLOCK_OFFSET && distribution.is_my_block(i)){  //block number i has data and belongs to this server
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
		size_t block_size = sip_tables_.block_size(block_id);
   	    ServerBlock* block = get_block_for_restore(block_id, array_id,block_size);
		Chunk* chunk = block->get_chunk();
		if (block_offset % chunk_size == 0){
//			 file->chunk_read_all(*chunk);
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

void DiskBackedBlockMap::eager_restore_chunks_from_sparse_index(int array_id, ArrayFile* file, std::vector<ArrayFile::offset_val_t>& index,
		ChunkManager* manager,
		const DataDistribution& distribution){
	ChunkManager::chunk_size_t chunk_size = manager->chunk_size();
	int rank = SIPMPIAttr::get_instance().company_rank();
	//create a map of blocks in file that belong to this server.  Count the ones that
	// correspond to the beginning of a chunk
	std::map<ArrayFile::offset_val_t, block_num_t> offset_block_map;
	int num_chunks = 0;
	int i = 0;
	while (i < index.size()){
	   offset_val_t block_num = index[i];
	   offset_val_t offset = index[i+1];
	   if (distribution.is_my_block(block_num)){
		   offset_block_map[offset]=block_num;
//		   std::cerr << " added to my blockMap";
		   if (offset % chunk_size == 0){ //block is start of chunk
			   num_chunks++;
		   }
	   }
	   i += 2;
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
		size_t block_size = sip_tables_.block_size(block_id);
   	    ServerBlock* block = get_block_for_restore(block_id, array_id,block_size);
		Chunk* chunk = block->get_chunk();
		if (block_offset % chunk_size == 0){
//			 file->chunk_read_all(*chunk);
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


//UNTESTED
ServerBlock* DiskBackedBlockMap::create_block_for_lazy_restore(int array_id, size_t block_size) {
		int chunk_number;
		Chunk::offset_val_t offset;
		ChunkManager* manager = chunk_managers_.at(array_id);
		manager->lazy_assign_block_data_from_chunk(block_size, chunk_number, offset);
		ServerBlock* block = new ServerBlock(block_size, manager, chunk_number, offset);
		block->get_chunk()->add_server_block(block);
		return block;
}





//UNTESTED
ServerBlock* DiskBackedBlockMap::get_block_for_lazy_restore(const BlockId& block_id){
	ServerBlock* block = block_map_.block(block_id);
	check(block == NULL, "attempting to restore block that already exists");
	size_t block_size = sip_tables_.block_size(block_id);
	block = create_block_for_lazy_restore(block_id.array_id(), block_size);
	block_map_.insert_block(block_id, block);
	block->block_data_.chunk_->valid_on_disk_=true;
	return block;

}








////UNTESTED
//void DiskBackedBlockMap::lazy_restore_chunks_from_index(int array_id, ArrayFile* file, ChunkManager* manager, ArrayFile::header_val_t num_blocks, const DataDistribution& distribution){
//	//read the index from the array file
//	ArrayFile::offset_val_t index[num_blocks];
//	file->read_index(index, num_blocks);
//	size_t chunk_size = manager->chunk_size();
//
//	//create two maps offset to block number for blocks belonging to this server
//	//the first map contains offsets that correspond to the beginning of a chunk
//	//the second contains offsets that are within a chunk
//
//	std::map<ArrayFile::offset_val_t, block_num_t> offset_block_map;
//	int num_chunks = 0;
//	for (int block_num = 0; block_num < num_blocks; block_num++){
//	   offset_val_t offset = index[block_num];
//	   if (offset != 0 && distribution.is_my_block(block_num)){
//		   BlockId block_id = sip_tables_.block_id(array_id, block_num);
//		   ServerBlock* block = get_block_for_lazy_restore(block_id);
//	   }
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
