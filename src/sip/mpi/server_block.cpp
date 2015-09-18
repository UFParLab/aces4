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

#include "lru_array_policy.h"

using namespace std::rel_ops;
namespace sip {


std::ostream& operator<< (std::ostream& os, const BlockData& block_data){
	os << "BlockData" << std::endl;
	os << "SIZE=" << block_data.size_ << std::endl;
	os << "Chunk" << block_data.chunk_ << std::endl;
	os << "offset" << block_data.offset_ << std::endl;
	int i = 0;
	const int MAX_TO_PRINT = 800;
	const int MAX_IN_ROW = 80;
	int size = block_data.size_;
	double* data = block_data.get_data();
	if (data == NULL) {
		os << " data = NULL";
	} else {
		while (i < size && i < MAX_TO_PRINT) {
			for (int j = 0; j < MAX_IN_ROW && i < size; ++j) {
				os << data[i++] << "  ";
			}
			os << '\n';
		}
	}
	os << "END OF BLOCK" << std::endl;
	return os;
}


ServerBlock::~ServerBlock(){
	block_data_.chunk_->remove_server_block(this);
}

ServerBlock::dataPtr ServerBlock::accumulate_data(size_t asize, dataPtr to_add){
	double* data_ = block_data_.get_data();
	check(size() == asize, "accumulating blocks of unequal size");
	check(data_ != NULL, "attempting to accumulate into block with null data_");
	check(to_add != NULL, "attempting to accumulate from null dataPtr");
	for (size_t i = 0; i < asize; ++i){
			data_[i] += to_add[i];
	}
	return data_;
}

ServerBlock::dataPtr ServerBlock::fill_data(double value) {
	double* data_ = block_data_.get_data();
	std::fill(data_+0, data_+size(), value);
	return data_;
}

ServerBlock::dataPtr ServerBlock::scale_data(double factor) {
	double* data_ = block_data_.get_data();
	for (int i = 0; i < size(); ++i) {
		data_[i] *= factor;
	}
	return data_;
}

ServerBlock::dataPtr ServerBlock::copy_data(ServerBlock* source_block){
	double* data = block_data_.get_data();
	double* source_data = source_block->get_data();
	check(size() == source_block->size(), "mismatched sizes in server block copy");
	check(data != NULL, "attempting to copy into block with null data_");
	check(source_data != NULL, "attempting to accumulate from null dataPtr");
	for (size_t i = 0; i < size(); ++i){
			data[i] = source_data[i];
	}
	return data;
}

ServerBlock::dataPtr ServerBlock::increment_data( double delta) {
	double* data_ = block_data_.get_data();
	for (int i = 0; i < size(); ++i) {
		data_[i] += delta;
	}
	return data_;
}

std::ostream& operator<<(std::ostream& os, const ServerBlock& block) {
	os << block.block_data_ << std::endl;
	os << "END OF BLOCK" << std::endl;
	return os;
}




} /* namespace sip */
