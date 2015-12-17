/*
 * chunk_manager.cpp
 *
 *  Created on: Jul 21, 2015
 *      Author: basbas
 */

#include "chunk_manager.h"
#include "server_block.h"

namespace sip {

ChunkManager::ChunkManager(chunk_size_t chunk_size, ArrayFile* file):
	chunk_size_(chunk_size)
,   file_(file){}

ChunkManager::~ChunkManager() {
	chunks_t::iterator it;
	for (it = chunks_.begin(); it != chunks_.end(); ++it) {
		CHECK((*it)->data_ == NULL, "Memory leak: attempting to delete chunk with allocated data");
	}
}
size_t ChunkManager::new_chunk() {
	double* chunk_data = new double[chunk_size_];
    int chunk_number = chunks_.size();
	offset_val_t offset = chunk_offset(chunk_number);
	//if c++11, change to emplace version
	chunks_.push_back(new Chunk(chunk_data, offset, false));
	return chunk_size_;
}

void ChunkManager::new_chunk_for_restore(offset_val_t offset){
	int chunk_number = chunks_.size();
	offset_val_t calculated_offset = chunk_offset(chunk_number);
	check (offset == calculated_offset, "calculated chunk offset and offset from file index not the same");
	Chunk* chunk = new Chunk(NULL, offset, true);
	chunks_.push_back(chunk);
}

size_t ChunkManager::reallocate_chunk_data(Chunk* chunk) {
	CHECK(chunk->data_ == NULL, "reallocating chunk data that exists");
	chunk->data_ = new double[chunk_size_];
	return chunk_size_;
}

size_t ChunkManager::assign_block_data_from_chunk(ServerBlock* block, size_t num_doubles, bool initialize, Chunk*& chunk,
		int& chunk_number, offset_val_t& offset){


	size_t num_newly_allocated = 0;
	//get current chunk
	chunks_t::reverse_iterator chunk_it = chunks_.rbegin();  //iterater points to last element, if there is one.
	//if not enough space in current chunk, or no chunks yet, allocate a new chunk
	if (chunk_it == chunks_.rend() || (chunk_size_ - (*chunk_it)->num_assigned_doubles_) < num_doubles){
		num_newly_allocated += new_chunk();
		chunk_it = chunks_.rbegin();
	}
	//number of current chunk
	chunk_number = chunks_.size()-1;  //chunks_.size > 0 here.
	//offset relative to beginning of chunk
	Chunk* chunk_ptr = *chunk_it;
	//if necessary, restore chunk from disk
	if (chunk_ptr->data_ == NULL){
		num_newly_allocated += reallocate_chunk_data(chunk_ptr);
		file_->chunk_read(*chunk_ptr);
	}
	offset = chunk_ptr->num_assigned_doubles_;
	chunk_ptr->num_assigned_doubles_ += num_doubles;
	if (initialize){
		std::fill(chunk_ptr->data_+offset, chunk_ptr->data_+offset + num_doubles, 0);
	}
	if (block != NULL) {chunk_ptr->blocks_.push_back(block);}
	chunk = chunk_ptr;
	return num_newly_allocated;
}

void ChunkManager::lazy_assign_block_data_from_chunk(size_t num_doubles,
		int& chunk_number, offset_val_t& offset){
	chunks_t::reverse_iterator chunk_it = chunks_.rbegin();
	if (chunk_it == chunks_.rend() || (chunk_size_ - (*chunk_it)->num_assigned_doubles_) < num_doubles){
		new_chunk_for_restore(offset);
		chunk_it = chunks_.rbegin();
	}
	//number of current chunk
	chunk_number = chunks_.size()-1;  //chunks_.size > 0 here.
	//offset relative to beginning of chunk
	Chunk* chunk_ptr = *chunk_it;
	offset = chunk_ptr->num_assigned_doubles_;
	chunk_ptr->num_assigned_doubles_ += num_doubles;
}

size_t ChunkManager::assign_block_data_from_chunk(size_t num_doubles, bool initialize,
		int& chunk_number, offset_val_t& offset){
	size_t num_newly_allocated = 0;
	if (chunks_.size()==0)
		num_newly_allocated += new_chunk();
	//get current chunk
	chunks_t::reverse_iterator chunk_it = chunks_.rbegin();  //last iterator
	//if not enough space, allocate a new chunk and update chunk_it
	if ((chunk_size_ - (*chunk_it)->num_assigned_doubles_) < num_doubles){
		num_newly_allocated += new_chunk();
		chunk_it = chunks_.rbegin();
	}
	//number of current chunk
	chunk_number = chunks_.size()-1;  //chunks_.size > 0 here.
	Chunk* chunk_ptr = *chunk_it;
	//offset relative to beginning of chunk
	offset = chunk_ptr->num_assigned_doubles_;
	chunk_ptr->num_assigned_doubles_ += num_doubles;
	if (initialize){
		std::fill(chunk_ptr->data_+offset, chunk_ptr->data_+offset + num_doubles, 0);
	}
//	chunk = *chunk_it;
	return num_newly_allocated;
}


size_t ChunkManager::delete_chunk_data(Chunk* chunk){
	if(chunk->data_ != NULL){
		delete chunk->data_;
		chunk->data_ = NULL;
		return chunk_size_;
	}
	return 0;
}

size_t ChunkManager::delete_chunk_data_all(){
	size_t num_deleted = 0;
	chunks_t::iterator it;
	for(it = chunks_.begin(); it != chunks_.end(); ++it){
		wait_all(*it);
		num_deleted += delete_chunk_data(*it);
	}
	return num_deleted;
}


void ChunkManager::wait_all(Chunk* chunk){
	for (std::vector<ServerBlock*>::iterator it = chunk->blocks_.begin();
			it != chunk->blocks_.end(); ++it){
//			    ServerBlock* block = *it;
//			    block->wait();
			(*it)->wait();
	}
}

void ChunkManager::collective_flush(){
	//determine max number of chunks to write by any server
	chunks_t::const_iterator it;
	int num_chunks = 0;
	for (it = chunks_.begin(); it != chunks_.end(); ++it){
		if(  ! (*it)->valid_on_disk_ ){
			num_chunks ++;
		}
	}
	int max;
	MPI_Allreduce(&num_chunks, &max, 1, MPI_INT, MPI_MAX, file_->comm_);

	//write own chunks that need writing
	chunks_t::iterator wit;
	int written = 0;
	for (wit = chunks_.begin(); wit != chunks_.end(); ++wit){
		if ( ! (*wit)->valid_on_disk_){
			file_->chunk_write_all(**wit);
			(*wit)->valid_on_disk_=true;
			written ++;
		}
	}

	//call noop collective write for remaining chunks at other servers.
	for (int i = written; i < max; ++i){
		file_->chunk_write_all_nop();
	}
}

size_t ChunkManager::collective_restore(){
//determine max number of chunks to read by any server
chunks_t::const_iterator it;
int num_chunks = 0;
for (it = chunks_.begin(); it != chunks_.end(); ++it){
	if( (*it)->data_== NULL){
		num_chunks ++;
	}
}
int max;
MPI_Allreduce(&num_chunks, &max, 1, MPI_INT, MPI_MAX, file_->comm_);

size_t doubles_allocated = 0;
//read own chunks that need writing
chunks_t::iterator rit;
int chunks_read = 0;
for (rit = chunks_.begin(); rit != chunks_.end(); ++rit){
	if ( (*rit)->data_ == NULL){
		doubles_allocated += reallocate_chunk_data(*rit);
		file_->chunk_read_all(**rit);
		chunks_read ++;
	}
}

//call noop collective read for remaining chunks at other servers.
for (int i = chunks_read; i < max; ++i){
	file_->chunk_read_all_nop();
}

return doubles_allocated;
}

size_t ChunkManager::restore(){
size_t doubles_allocated = 0;
chunks_t::iterator rit;
int chunks_read = 0;
for (rit = chunks_.begin(); rit != chunks_.end(); ++rit){
	if ( (*rit)->data_ == NULL){
		doubles_allocated += reallocate_chunk_data(*rit);
		file_->chunk_read(**rit);
		chunks_read ++;
	}
}
return doubles_allocated;
}


//This is a collective operation
int ChunkManager::max_num_chunks() const{
	int  num = chunks_.size();
	int max;
	MPI_Allreduce(&num, &max, 1, MPI_INT, MPI_MAX, file_->comm_);
	return max;
}


std::ostream& operator<<(std::ostream& os, const ChunkManager& obj){
	int chunk_number = 0;
	os << "Chunk Manager" << std::endl;
	ChunkManager::chunks_t::const_iterator it;
	for (it = obj.chunks_.begin(); it != obj.chunks_.end(); ++it) {
		os << chunk_number << ':' << **it << std::endl;
		chunk_number++;
	}
	return os;
}

} /* namespace sip */
