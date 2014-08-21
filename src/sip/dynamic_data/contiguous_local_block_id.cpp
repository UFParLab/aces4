/*
 * contiguous_local_block_id.cpp
 *
 *  Created on: Aug 20, 2014
 *      Author: basbas
 */

#include "contiguous_local_block_id.h"
#include <sstream>
#include "sip.h"
#include "sip_tables.h"

using namespace std::rel_ops;
namespace sip {


ContiguousLocalBlockId::ContiguousLocalBlockId() :
		 array_id_(-1) {
}

ContiguousLocalBlockId::ContiguousLocalBlockId(int array_id,
		const index_value_array_t& lower_index_values,
		const index_value_array_t& upper_index_values):
		array_id_(array_id) {
	std::copy(lower_index_values + 0, lower_index_values + MAX_RANK, lower_index_values_ + 0);
	std::copy(upper_index_values + 0, upper_index_values + MAX_RANK, upper_index_values_ + 0);
	sial_check(well_formed(),std::string("invalid range for contiguous local array"), current_line());
}


ContiguousLocalBlockId::ContiguousLocalBlockId(int array_id, int rank,
		const std::vector<int>& lower_index_values_vector,
		const std::vector<int>& upper_index_values_vector) :
		array_id_(array_id){
	sip::check(rank == lower_index_values_vector.size(), "ContiguousLocalBlockId constructor", current_line());
	sip::check(rank == upper_index_values_vector.size(), "ContiguousLocalBlockId constructor", current_line());
	std::copy(lower_index_values_vector.begin(), lower_index_values_vector.end(), lower_index_values_ + 0); //copy values from vector into index_values_ array
	std::fill(lower_index_values_ + rank, lower_index_values_ + MAX_RANK,
			unused_index_value); //fill in unused slots
	std::copy(upper_index_values_vector.begin(), upper_index_values_vector.end(), upper_index_values_ + 0); //copy values from vector into index_values_ array
	std::fill(upper_index_values_ + rank, upper_index_values_ + MAX_RANK,
			unused_index_value); //fill in unused slots
	sial_check(well_formed(),std::string("invalid range for contiguous local array"), current_line());
}

ContiguousLocalBlockId::ContiguousLocalBlockId(const ContiguousLocalBlockId& rhs) {
	array_id_ = rhs.array_id_;
	std::copy(rhs.lower_index_values_ + 0, rhs.lower_index_values_ + MAX_RANK,
			lower_index_values_ + 0);
	std::copy(rhs.upper_index_values_ + 0, rhs.upper_index_values_ + MAX_RANK,
			upper_index_values_ + 0);
}

ContiguousLocalBlockId ContiguousLocalBlockId::operator=(ContiguousLocalBlockId tmp) { //param passed by value, makes a copy.
	//C++ note:  this is the recommended way to make an exception safe assign constructor.
	//if an exception is thrown, it will happen in the (implicit) copy.
	std::swap(array_id_, tmp.array_id_);
	std::swap(lower_index_values_, tmp.lower_index_values_);
	std::swap(upper_index_values_, tmp.upper_index_values_);
	return *this;
}


ContiguousLocalBlockId::~ContiguousLocalBlockId() {}

bool ContiguousLocalBlockId::operator==(const ContiguousLocalBlockId& rhs) const {
//	bool is_equal = (array_id_ == rhs.array_id_);
//	is_equal &= std::equal(lower_index_values_ + 0, lower_index_values_ + MAX_RANK,
//			rhs.lower_index_values_);
//	is_equal &= std::equal(upper_index_values_ + 0, upper_index_values_ + MAX_RANK,
//			rhs.upper_index_values_);
//	return is_equal;
	return array_id_==rhs.array_id_   &&  equal_ranges(rhs);
}


bool ContiguousLocalBlockId::operator<(const ContiguousLocalBlockId& rhs) const {
	if (this == &rhs)
		return false;  //self comparison
	if (array_id_ != rhs.array_id_) {  //compare by arrays
		return (array_id_ < rhs.array_id_);
	}
	if (!disjoint(rhs)) return false;
	return std::lexicographical_compare(upper_index_values_ + 0,
				upper_index_values_ + MAX_RANK, rhs.lower_index_values_ + 0,
				rhs.lower_index_values_ + MAX_RANK);
	}


std::string ContiguousLocalBlockId::str(const SipTables& sip_tables) const{
	std::stringstream ss;
	int rank = sip_tables.array_rank(array_id_);
	ss  <<sip_tables.array_name(array_id_); // << " : " << array_id_ << " : ";
	ss << '[';
	int i;
	for (i = 0; i < rank; ++i) {
		ss << (i == 0 ? "" : ",") << lower_index_values_[i] << ":" << upper_index_values_[i];
	}
	ss << ']';
	return ss.str();
}

std::ostream& operator<<(std::ostream& os, const ContiguousLocalBlockId& id) {
	os << id.array_id_ << ':';
	os << '[';
	int i;
	for (i = 0; i < MAX_RANK; ++i) {
		os << (i == 0 ? "" : ",") << id.lower_index_values_[i] << ":" << id.upper_index_values_[i];
	}
	os << ']';
	return os;
}

bool ContiguousLocalBlockId::well_formed(){
//	bool lesseq = true;
//	for (int i = 0; i < MAX_RANK; ++i ){
//		lesseq = lesseq && lower_index_values_[i] <= upper_index_values_[i];
//	}
//	return lesseq;
	for (int i = 0; i < MAX_RANK; ++i ){
		if (lower_index_values_[i] > upper_index_values_[i]) return false;
	}
	return true;
}

bool ContiguousLocalBlockId::equal_ranges(const ContiguousLocalBlockId& rhs) const {
	bool is_equal = true;
	is_equal &= std::equal(lower_index_values_ + 0, lower_index_values_ + MAX_RANK,
			rhs.lower_index_values_);
	is_equal &= std::equal(upper_index_values_ + 0, upper_index_values_ + MAX_RANK,
			rhs.upper_index_values_);
	return is_equal;
}

bool ContiguousLocalBlockId::encloses(const ContiguousLocalBlockId& id) const{
	for (int i = 0; i < MAX_RANK; ++i){
		if(! (lower_index_values_[i] <= id.lower_index_values_[i] && id.upper_index_values_[i] <= upper_index_values_[i])) return false;
	}
	return true;
}

bool ContiguousLocalBlockId::disjoint(const ContiguousLocalBlockId& id) const{
	for (int i = 0; i < MAX_RANK; ++i){
		if(id.upper_index_values_[i] < lower_index_values_[i] || upper_index_values_[i] < id.lower_index_values_[i]) return true;
	}
	return false;
}


} /* namespace sip */
