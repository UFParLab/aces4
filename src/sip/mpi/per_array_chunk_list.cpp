/*
 * per_array_chunk_list.cpp
 *
 *  Created on: Jul 21, 2015
 *      Author: basbas
 */

#include "per_array_chunk_list.h"

namespace sip {

std::ostream& operator<<(std::ostream& os, const Chunk& obj){
	os << "data_: " << obj.data_;
	os << ", file_offset_: " << obj.file_offset_;
	os << " num_allocated_doubles_: "<< obj.num_allocated_doubles_;
	os << " valid_on_disk_: " << obj. valid_on_disk_;
	os << std::endl;
	return os;
}


} /* namespace sip */
