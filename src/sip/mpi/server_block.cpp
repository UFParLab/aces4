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
const std::size_t ServerBlock::max_allocated_bytes_ = 2147483648;	// 2 GB
std::size_t ServerBlock::allocated_bytes_ = 0;

ServerBlock::ServerBlock(int size, bool initialize): is_dirty_(false), size_(size){
	if (initialize)
		data_ = new double[size_]();
	else
		data_ = new double[size_];

	const std::size_t bytes_in_block = field_members_size_ + size_ * sizeof(dataPtr);
	allocated_bytes_ += bytes_in_block;
}

ServerBlock::ServerBlock(int size, dataPtr data): is_dirty_(false), size_(size), data_(data) {
	const std::size_t bytes_in_block = field_members_size_ + size_ * sizeof(dataPtr);
	allocated_bytes_ += bytes_in_block;
}

ServerBlock::~ServerBlock(){
	const std::size_t bytes_in_block = field_members_size_ + size_ * sizeof(dataPtr);
	std::stringstream ss;
	ss << "Allocated bytes [ " << allocated_bytes_ <<" ] less than size of block being destroyed "
			<< " [ " << bytes_in_block << " ] ";
	sip::check(allocated_bytes_ >= bytes_in_block, ss.str());
	allocated_bytes_ -= bytes_in_block;
	if (!data_)
		delete [] data_;
}

ServerBlock::dataPtr ServerBlock::accumulate_data(size_t size, dataPtr to_add){
        check(size_ == size, "accumulating blocks of unequal size");
        for (unsigned i = 0; i < size; ++i){
                data_[i] += to_add[i];
        }
        return data_;
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

} /* namespace sip */
