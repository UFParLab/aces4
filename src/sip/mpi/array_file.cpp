/*
 * array_file.cpp
 *
 *  Created on: Aug 10, 2015
 *      Author: basbas
 */

#include "array_file.h"

namespace sip {

std::ostream& operator<<(std::ostream& os, const ArrayFile& obj){
	os << obj.name_ << ": "
			<< obj.num_blocks_ << ","
			<< obj.chunk_size_ << ","
			<< obj.is_persistent_ << ","
			<< obj.label_;
	os << std::endl;
	return os;
}
} /* namespace sip */
