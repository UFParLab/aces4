/*
 * block_id.cpp
 *
 *  Created on: Apr 6, 2014
 *      Author: basbas
 */

#include "block_id.h"
#include <sstream>
#include "sip_tables.h"

using namespace std::rel_ops;
namespace sip {



const int BlockId::MPI_BLOCK_ID_COUNT;

BlockId::BlockId() :
		parent_id_ptr_(NULL), array_id_(-1) {
}

BlockId::BlockId(int array_id, const index_value_array_t& index_values) :
		array_id_(array_id), parent_id_ptr_(NULL) {
	std::copy(index_values + 0, index_values + MAX_RANK, index_values_ + 0);
}

BlockId::BlockId(const mpi_block_id_t buffer) :
		array_id_(buffer[0]), parent_id_ptr_(NULL) {
	std::copy(buffer + 1, buffer + MAX_RANK + 1, index_values_);
}

BlockId::BlockId(int array_id, const BlockId& old_block) :
		array_id_(array_id), parent_id_ptr_(NULL) {
	std::copy(old_block.index_values_ + 0, old_block.index_values_ + MAX_RANK,
			index_values_ + 0);
}

BlockId::BlockId(int array_id, const index_value_array_t& index_values,
		const BlockId& parent_id_ptr) :
		array_id_(array_id), parent_id_ptr_(new BlockId(parent_id_ptr)) {
	std::copy(index_values + 0, index_values + MAX_RANK, index_values_ + 0);
}

BlockId::BlockId(int array_id, int rank, const std::vector<int>& index_values) :
		array_id_(array_id), parent_id_ptr_(NULL) {
	sip::check(rank == index_values.size(), "SIP BUG: BlockId constructor");
	std::copy(index_values.begin(), index_values.end(), index_values_ + 0); //copy values from vector into index_values_ array
	std::fill(index_values_ + rank, index_values_ + MAX_RANK,
			unused_index_value); //fill in unused slots
}

BlockId::BlockId(int array_id, const index_value_array_t& lower, const index_value_array_t& upper):
	array_id_(array_id){
	std::copy(lower + 0, lower + MAX_RANK, index_values_ + 0);
	parent_id_ptr_ = new BlockId(array_id, upper);

	sial_check(this->is_well_formed(), "block is not well formed, invalid ranges", current_line());
}

BlockId::BlockId(const BlockId& rhs) {
	array_id_ = rhs.array_id_;
	std::copy(rhs.index_values_ + 0, rhs.index_values_ + MAX_RANK,
			index_values_ + 0);
	if (rhs.parent_id_ptr_ != NULL) {
		parent_id_ptr_ = new BlockId(*rhs.parent_id_ptr_);
	} else {
		parent_id_ptr_ = NULL;
	}
}

BlockId::BlockId(int array_id) :
		array_id_(array_id), parent_id_ptr_(NULL) {
	std::fill(index_values_ + 0, index_values_ + MAX_RANK, 1);
}

BlockId BlockId::operator=(BlockId tmp) { //param passed by value, makes a copy.
	//C++ note:  this is the recommended way to make an exception safe assign constructor.
	//if an exception is thrown, it will happen in the (implicit) copy.
	std::swap(array_id_, tmp.array_id_);
	std::swap(index_values_, tmp.index_values_);
	std::swap(parent_id_ptr_, tmp.parent_id_ptr_);
	return *this;
}

//TODO subindices may not be implemented properly any more:
BlockId::~BlockId() {
	if (parent_id_ptr_ != NULL)
		delete parent_id_ptr_;  //this should be a copy
}

bool BlockId::operator==(const BlockId& rhs) const {
//	bool is_equal = (array_id_ == rhs.array_id_);
//	is_equal &= std::equal(index_values_ + 0, index_values_ + MAX_RANK,
//			rhs.index_values_);
//	is_equal &= (parent_id_ptr_ == NULL && rhs.parent_id_ptr_ == NULL)
//			|| (parent_id_ptr_ != NULL && rhs.parent_id_ptr_ != NULL
//					&& *parent_id_ptr_ == *rhs.parent_id_ptr_);
//	return is_equal;
	return (array_id_ == rhs.array_id_) && std::equal(index_values_+0, index_values_+MAX_RANK, rhs.index_values_)
	     && ((parent_id_ptr_ == NULL && rhs.parent_id_ptr_ == NULL)
	     			|| (parent_id_ptr_ != NULL && rhs.parent_id_ptr_ != NULL
	    					&& *parent_id_ptr_ == *rhs.parent_id_ptr_));
}

//Since the blockId is used as a key in the BlockMap, which is currently
//the binary tree, this operator must be defined.
// a < b if the a.array_id_ < b.array_id_ or a.array_id_ = b.array_id_  && a.index_values_ < b.index_values
//where the latter uses lexicographic ordering.
//TODO this has only been tested with one level of nesting.
bool BlockId::operator<(const BlockId& rhs) const {
//	std::cout << "BlockId::operator<(const BlockId& rhs)\n this: " << *this << '@'<<this <<
//			"\n rhs " << rhs << '@'<< &rhs << std::endl;

	if (this == &rhs)
		return false;  //self comparison
	if (array_id_ != rhs.array_id_) {  //compare by arrays
		return (array_id_ < rhs.array_id_);
	}
	//same arrays, neither is a subblock nor contiguous local
	if (parent_id_ptr_ == NULL && rhs.parent_id_ptr_ == NULL) {
		return std::lexicographical_compare(index_values_ + 0,
				index_values_ + MAX_RANK, rhs.index_values_ + 0,
				rhs.index_values_ + MAX_RANK);
	}
	//same arrays, both are contiguous local (compiler ensures that all Ids for arrays declared contig local
	// are given with a[l0:u0, l1:u1...] so either both have non-null parents with same array in parent or neither do.
	if (is_contiguous_local()) {
		//can remove this check after debugging
		check(rhs.is_contiguous_local(), "comparing contiguous local with non-contiguous local", current_line());
		if (std::lexicographical_compare(index_values_ + 0,
				index_values_ + MAX_RANK, rhs.index_values_ + 0,
				rhs.index_values_ + MAX_RANK))
		return true;
		if (std::equal(index_values_ + 0,
				index_values_ + MAX_RANK, rhs.index_values_ + 0)) //the lower bounds are equal, compare upper
		{  return (*parent_id_ptr_ < *(rhs.parent_id_ptr_ ));
	    }
		return false;
	}

	//lhs is a subblock, rhs isn't, compare lhs parent with rhs
	if (parent_id_ptr_ != NULL && rhs.parent_id_ptr_ == NULL) {
		if (*parent_id_ptr_ < rhs)
			return true;
		if (*parent_id_ptr_ == rhs)
			return true;  //this is subblock of rhs, subblock < own parent block
		return false;
	}
	//rhs is subblock, lhs isn't, compare lhs with rhs parent
	if (parent_id_ptr_ == NULL && rhs.parent_id_ptr_ != NULL) {
		if (*this < *rhs.parent_id_ptr_)
			return true;
		if (*this == *rhs.parent_id_ptr_)
			return false;  //rhs is subblock of lhs, subblock < own parent block
		return false;
	}
	//both are subblocks, compare their parents
	if (*parent_id_ptr_ < *rhs.parent_id_ptr_)
		return true;
	if (*parent_id_ptr_ == *rhs.parent_id_ptr_) {
		return std::lexicographical_compare(index_values_ + 0,
				index_values_ + MAX_RANK, rhs.index_values_ + 0,
				rhs.index_values_ + MAX_RANK);
	}
	return false;
}


//two blocks overlap if they are from the same array, and they overlap in all dimensions.
//If they are not contiguous local, then overlap is the same as ==.
bool BlockId::overlaps(const BlockId& other) const{
	if (array_id_ != other.array_id_) return false;
	if (! is_contiguous_local()) return *this == other;
	//now we're left with two contiguous local blocks of same array.
	for (int i = 0; i < MAX_RANK; ++i){
		if( !(index_values_[i] <= other.parent_id_ptr_->index_values_[i] &&
				other.index_values_[i] <= parent_id_ptr_->index_values_[i])) return false;
	}
	return true;
}



bool BlockId::encloses(const BlockId& other) const{
if (is_contiguous_local()){
	if (array_id_ != other.array_id_) return false;
	for (int i = 0; i < MAX_RANK; ++i){
		if(! (index_values_[i] <= other.index_values_[i] &&
				other.parent_id_ptr_->index_values_[i] <= parent_id_ptr_->index_values_[i])) return false;
	}
	return true;
}
fail("applying encloses on noncontiguous locals.  We may want to allow this later");
return false;
}






/**
 * if this is a contiguous array, then checks whether, in each dimension, lower <= upper.
 * Returns true if not a contiguous array.
 * @return
 */
bool BlockId::is_well_formed(){
	if (!is_contiguous_local()) return true;
	for (int i = 0; i < MAX_RANK; ++i ){
		if (index_values_[i] > parent_id_ptr_->index_values_[i]) return false;
	}
	return true;
}

//std::string BlockId::str(const SipTables& sip_tables) const{
//	std::stringstream ss;
////	SipTables& tables = SipTables::instance();
////	int rank = tables.array_rank(array_id_);
////	ss << (tables.array_name(array_id_));
//	int rank = sip_tables.array_rank(array_id_);
//	ss  <<sip_tables.array_name(array_id_); // << " : " << array_id_ << " : ";
//	ss << '[';
//	int i;
//	for (i = 0; i < rank; ++i) {
//		ss << (i == 0 ? "" : ",") << index_values_[i];
//	}
//	ss << ']';
//	return ss.str();
//}

std::string BlockId::str(const SipTables& sip_tables) const{
std::stringstream ss;
bool contiguous_local = sip_tables.is_contiguous_local(array_id_);
if (contiguous_local) ss << "contiguous local ";
int rank = sip_tables.array_rank(array_id_);
ss << sip_tables.array_name(array_id_) ;
ss << '[';
int i;
for (i = 0; i < rank; ++i) {
	ss << (i == 0 ? "" : ",") << index_values(i);
	if (contiguous_local) ss << ":" << upper_index_values(i);
}
ss << ']';
return ss.str();
}

std::ostream& operator<<(std::ostream& os, const BlockId& id) {
	os << id.array_id_ << ':';
	os << "[" << id.index_values_[0];
	for (int i = 1; i < MAX_RANK; ++i) {
		os << "," << id.index_values_[i];
	}
	os << ']';
	if (id.parent_id_ptr_ != NULL) {
		os << "[" << id.parent_id_ptr_->index_values_[0];
		for (int i = 1; i < MAX_RANK; ++i) {
			os << "," << id.parent_id_ptr_->index_values_[i];
		}
		os << ']';
	}
	return os;
}

} /* namespace sip */
