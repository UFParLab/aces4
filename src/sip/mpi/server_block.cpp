/*
 * server_block.cpp
 *
 *  Created on: Mar 25, 2014
 *      Author: njindal
 */

#include <server_block.h>
#include <cstdlib>
#include <iostream>
#include <exception>
#include <sstream>

#include "sip.h"

namespace sip {

const std::size_t ServerBlock::field_members_size_ = sizeof(int) + sizeof(int) + sizeof(dataPtr);
std::size_t ServerBlock::max_allocated_bytes_ = 2147483648;	// 2 GB
std::size_t ServerBlock::allocated_bytes_ = 0;

ServerBlock::ServerBlock(int size, bool initialize): size_(size){
	if (initialize){
		data_ = new double[size_]();
    } else {
		data_ = new double[size_];
    }

	status_[ServerBlock::inMemory] = true;
	status_[ServerBlock::onDisk] = false;
	status_[ServerBlock::dirtyInMemory] = false;

	// Only Count number of bytes allocated for actual data.
//	const std::size_t bytes_in_block = field_members_size_ + size_ * sizeof(dataPtr);
//	allocated_bytes_ += bytes_in_block;
	allocated_bytes_ += size_ * sizeof(double);
}

ServerBlock::ServerBlock(int size, dataPtr data): size_(size), data_(data) {
	status_[ServerBlock::inMemory] = data_ == NULL ? false : true;
	status_[ServerBlock::onDisk] = false;
	status_[ServerBlock::dirtyInMemory] = false;

	// Only Count number of bytes allocated for actual data.
//	const std::size_t bytes_in_block = field_members_size_ + size_ * sizeof(dataPtr);
//	allocated_bytes_ += bytes_in_block;
    if (data_ != NULL)
    	allocated_bytes_ += size_ * sizeof(double);
}

ServerBlock::~ServerBlock(){
	const std::size_t bytes_in_block = size_ * sizeof(double);
	std::stringstream ss;
	ss << "Allocated bytes [ " << allocated_bytes_ <<" ] less than size of block being destroyed "
			<< " [ " << bytes_in_block << " ] ";
	sip::check(allocated_bytes_ >= bytes_in_block, ss.str());
	allocated_bytes_ -= bytes_in_block;

	if (data_){
		sip::check(status_[ServerBlock::inMemory], "ServerBlock not in memory yet data_ is not NULL");
		delete [] data_;
	}
}

ServerBlock::dataPtr ServerBlock::accumulate_data(size_t size, dataPtr to_add){
	check(size_ == size, "accumulating blocks of unequal size");
	for (unsigned i = 0; i < size; ++i){
			data_[i] += to_add[i];
	}
	return data_;
}


void ServerBlock::free_in_memory_data() {
   	delete [] data_; data_ = NULL;
   	status_[ServerBlock::inMemory] = false;
   	status_[ServerBlock::dirtyInMemory] = false;
   	allocated_bytes_ -= size_ * sizeof(double);
}

void ServerBlock::allocate_in_memory_data(bool initialize) {
   	sip::check(data_ == NULL, "data_ was not NULL, allocating memory in allocate_in_memory_data!");
   	if (initialize) 
        data_ = new double[size_]();
   	else 
        data_ = new double[size_];

   	status_[ServerBlock::inMemory] = true;
   	status_[ServerBlock::dirtyInMemory] = false;
   	allocated_bytes_ += size_ * sizeof(double);
}

std::ostream& operator<<(std::ostream& os, const ServerBlock& block) {
	os << "SIZE=" << block.size_ << std::endl;
	int i = 0;
	const int MAX_TO_PRINT = 800;
	const int MAX_IN_ROW = 80;
	int size = block.size_;
	if (block.data_ == NULL) {
		os << " data_ = NULL";
	} else {
		while (i < size && i < MAX_TO_PRINT) {
			for (int j = 0; j < MAX_IN_ROW && i < size; ++j) {
				os << block.data_[i++] << "  ";
			}
			os << '\n';
		}
	}
	os << "END OF BLOCK" << std::endl;
	return os;
}

bool ServerBlock::limit_reached(){
	return !(allocated_bytes_ < max_allocated_bytes_);
}

std::size_t ServerBlock::remaining_memory(){
	return max_allocated_bytes_ - allocated_bytes_;
}

void ServerBlock::set_memory_limit(std::size_t size){
    static bool done_once = false;
    if (!done_once){
        done_once = true;
        max_allocated_bytes_ = size;
    } else {
        sip::fail("Already set memory limit once !");
    }
}

} /* namespace sip */
