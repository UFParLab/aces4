/*
 * chunk_manager.cpp
 *
 *  Created on: Jul 21, 2015
 *      Author: basbas
 */

#include "chunk_manager.h"
#include "server_block.h"

namespace sip {



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
	check(chunk->data_ == NULL, "reallocating chunk data that exists");
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
		num_newly_allocated = new_chunk();
		chunk_it = chunks_.rbegin();
	}
	//number of current chunk
	chunk_number = chunks_.size()-1;  //chunks_.size > 0 here.
	//offset relative to beginning of chunk
	Chunk* chunk_ptr = *chunk_it;
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
	//get current chunk
	chunks_t::reverse_iterator chunk_it = chunks_.rbegin();  //last iterator
	//if not enough space, allocate a new chunk and update chunk_it
	if (chunk_it == chunks_.rend() || (chunk_size_ - (*chunk_it)->num_assigned_doubles_) < num_doubles){
		num_newly_allocated = new_chunk();
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
