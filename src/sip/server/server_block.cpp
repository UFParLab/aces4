/*
 * server_block.cpp
 *
 *  Created on: Apr 7, 2014
 *      Author: basbas
 */

#include "server_block.h"
#include <iostream>

namespace sip {

/**
 * ROUTINES FOR SERVER Blocks
 *
 */
std::ostream& operator<<(std::ostream& os, const ServerBlock& block) {
	os << "SIZE=" << block.size_ << std::endl;
	int i = 0;
	int MAX_TO_PRINT = 800;
	int size = block.size_;
	int output_row_size = 30;  //size of first dimension (really a column, but displayed as a row)
	if (block.data_ == NULL) {
		os << " data_ = NULL";
	} else {
		while (i < size && i < MAX_TO_PRINT) {
			for (int j = 0; j < output_row_size && i < size; ++j) {
				os << block.data_[i++] << "  ";

			}
			os << '\n';
		}
	}
	os << "END OF SERVER BLOCK" << std::endl;
	return os;
}

} /* namespace sip */
