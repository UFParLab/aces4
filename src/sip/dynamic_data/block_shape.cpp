/*
 * block_shape.cpp
 *
 *  Created on: Apr 6, 2014
 *      Author: basbas
 */

#include <block_shape.h>
#include <iostream>

using namespace std::rel_ops;

namespace sip {



BlockShape::BlockShape() {
	std::fill(segment_sizes_ + 0, segment_sizes_ + MAX_RANK, 1);
}


BlockShape::BlockShape(const BlockShape& rhs){
	std::copy(rhs.segment_sizes_ + 0, rhs.segment_sizes_ + MAX_RANK, segment_sizes_+0);
}

//BlockShape::BlockShape(const segment_size_array_t& segment_sizes) {
//	std::copy(segment_sizes + 0, segment_sizes + MAX_RANK, segment_sizes_ + 0);
//}

BlockShape::BlockShape(const segment_size_array_t& segment_sizes, int rank){
	std::copy(segment_sizes + 0, segment_sizes + rank, segment_sizes_+0);
	std::fill(segment_sizes_ + rank, segment_sizes_ + MAX_RANK, 1);
}

BlockShape::~BlockShape() {
}

std::ostream& operator<<(std::ostream& os, const BlockShape & shape) {
	os << "[";
	for (int i = 0; i < MAX_RANK; ++i) {
		os << (i == 0 ? "" : ",") << shape.segment_sizes_[i];
	}
	os << ']';

	return os;
}

int BlockShape::num_elems() const{
	int num_elems = 1;
	for (int i = 0; i < MAX_RANK; i++) {
		num_elems *= segment_sizes_[i];
	}
	return num_elems;
}

BlockShape& BlockShape::operator=(const BlockShape& rhs) {
	std::copy(rhs.segment_sizes_ + 0, rhs.segment_sizes_ + MAX_RANK, segment_sizes_+0);
	return *this;
}

bool BlockShape::operator==(const BlockShape& rhs) const {
	bool is_equal = true;
	for (int i = 0; is_equal && i < MAX_RANK; ++i) {
		is_equal = (segment_sizes_[i] == rhs.segment_sizes_[i]);
	}
	return is_equal;
}

//bool BlockShape::operator<(const BlockShape& rhs) const {
//	bool is_eq = true;
//	bool is_leq = true;
//	for (int i = 0; is_leq && i < MAX_RANK; ++i) {
//		is_leq = (segment_sizes_[i] <= rhs.segment_sizes_[i]);
//		is_eq = is_eq && (segment_sizes_[i] == rhs.segment_sizes_[i]);
//	}
//	return (is_leq && !is_eq);
//}

} /* namespace sip */
