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
#include <utility>

#include "sip.h"
#include "sip_c_blas.h"
#include "lru_array_policy.h"

namespace sip {

const std::size_t ServerBlock::field_members_size_ = sizeof(int) + sizeof(int) + sizeof(dataPtr);
std::size_t ServerBlock::allocated_bytes_ = 0;

ServerBlock::ServerBlock(int size, bool initialize): size_(size) {
	if (initialize){
		data_ = new double[size_]();
    } else {
		data_ = new double[size_];
    }

	disk_status_[ServerBlock::IN_MEMORY] = true;
	disk_status_[ServerBlock::ON_DISK] = false;
	disk_status_[ServerBlock::DIRTY_IN_MEMORY] = false;

	// Only Count number of bytes allocated for actual data.
//	const std::size_t bytes_in_block = field_members_size_ + size_ * sizeof(dataPtr);
//	allocated_bytes_ += bytes_in_block;
	allocated_bytes_ += size_ * sizeof(double);
}

ServerBlock::ServerBlock(int size, dataPtr data): size_(size), data_(data){
	disk_status_[ServerBlock::IN_MEMORY] = (data_ == NULL) ? false : true;
	disk_status_[ServerBlock::ON_DISK] = false;
	disk_status_[ServerBlock::DIRTY_IN_MEMORY] = false;

	// Only Count number of bytes allocated for actual data.
//	const std::size_t bytes_in_block = field_members_size_ + size_ * sizeof(dataPtr);
//	allocated_bytes_ += bytes_in_block;
    if (data_ != NULL)
    	allocated_bytes_ += size_ * sizeof(double);
}

ServerBlock::~ServerBlock(){
	const std::size_t bytes_in_block = size_ * sizeof(double);
	if (data_ != NULL) {
        state_.wait();
		if (!(allocated_bytes_ >= bytes_in_block)){
			std::stringstream ss;
			ss << "Allocated bytes [ " << allocated_bytes_ <<" ] less than size of block being destroyed "
					<< " [ " << bytes_in_block << " ] ";
			sip::fail(ss.str());
		}
		allocated_bytes_ -= bytes_in_block;

		CHECK(disk_status_[ServerBlock::IN_MEMORY], "ServerBlock not in memory yet data_ is not NULL");
		delete [] data_;
	}
}

ServerBlock::dataPtr ServerBlock::accumulate_data(size_t size, dataPtr to_add){
	CHECK(size_ == size, "accumulating blocks of unequal size");
	// USE BLAS DAXPY or Loop
	//for (unsigned i = 0; i < size; ++i){
	//		data_[i] += to_add[i];
	//}
	int i_one = 1;
	double d_one = 1.0;
	int size__ = size;
	sip_blas_daxpy(size__, d_one, to_add, i_one, data_, i_one);
	return data_;
}


bool ServerBlock::free_in_memory_data() {
	bool deleted_block = false;
	if (data_ != NULL) {
        state_.wait();
		delete [] data_; data_ = NULL;
		disk_status_[ServerBlock::IN_MEMORY] = false;
		disk_status_[ServerBlock::DIRTY_IN_MEMORY] = false;
		allocated_bytes_ -= size_ * sizeof(double);
		deleted_block = true;
	}
	return deleted_block;
}

void ServerBlock::allocate_in_memory_data(bool initialize) {
	CHECK(data_ == NULL, "data_ was not NULL, allocating memory in allocate_in_memory_data!");
   	if (initialize) 
        data_ = new double[size_]();
   	else 
        data_ = new double[size_];

   	disk_status_[ServerBlock::IN_MEMORY] = true;
   	disk_status_[ServerBlock::DIRTY_IN_MEMORY] = false;
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


std::size_t ServerBlock::allocated_bytes(){
	return allocated_bytes_;
}


} /* namespace sip */
