/*
 * block_selector.cpp
 *
 *  Created on: Apr 6, 2014
 *      Author: basbas
 */

#include <block_selector.h>
#include <iostream>

namespace sip {

BlockSelector::BlockSelector(int array_id, int rank,
		const index_selector_t& index_ids) :
		array_id_(array_id),
		rank_(rank){
	for (int i = 0; i != rank; ++i) {
		index_ids_[i] = index_ids[i];
	}
	for (int i = rank; i != MAX_RANK; ++i) {
		index_ids_[i] = unused_index_slot;
	}
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
