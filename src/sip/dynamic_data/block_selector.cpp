/*
 * block_selector.cpp
 *
 *  Created on: Apr 6, 2014
 *      Author: basbas
 */

#include <block_selector.h>
#include <iostream>
#include <utility>

namespace sip {

BlockSelector::BlockSelector() :array_id_(-1), rank_(0) {
	std::fill(index_ids_ + 0, index_ids_ + MAX_RANK, -1);
}

BlockSelector::BlockSelector(int rank, int array_id,
		const index_selector_t& index_ids) :
		array_id_(array_id),
		rank_(rank){
	int i;
	for (i = 0; i < rank; ++i) {
		index_ids_[i] = index_ids[i];
	}
	for (; i < MAX_RANK; ++i) {
		index_ids_[i] = unused_index_slot;
	}
}

BlockSelector::BlockSelector(const BlockSelector& rhs):
	array_id_(rhs.array_id_), rank_(rhs.rank_){
	std::copy(rhs.index_ids_ + 0, rhs.index_ids_ + MAX_RANK, index_ids_);
}

BlockSelector& BlockSelector::operator=(const BlockSelector& rhs){
	this->array_id_ = rhs.array_id_;
	this->rank_ = rhs.rank_;
	std::copy (rhs.index_ids_ + 0, rhs.index_ids_ + MAX_RANK, this->index_ids_);
	return *this;
}

bool BlockSelector::operator==(const BlockSelector& rhs) const {
	if (rank_ != rhs.rank_) return false;
	for (int i = 0; i < MAX_RANK; ++i) {
		if  (index_ids_[i] != rhs.index_ids_[i]) return false;
	}
	return true;
}

std::ostream& operator<<(std::ostream& os, const BlockSelector & selector) {
	os << selector.array_id_;
	os << "[" << selector.index_ids_[0];
	for (int i = 1; i < MAX_RANK; ++i) {
		int id = selector.index_ids_[i];
		if (id != unused_index_slot)
			os << "," << id;
	}
	os << ']';
	return os;
}

} /* namespace sip */
