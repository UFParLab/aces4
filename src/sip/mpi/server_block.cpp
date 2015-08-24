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

bool BlockData::operator==(const BlockData& rhs) const {
	return chunk_->file_offset() == rhs.chunk_->file_offset() && offset_ == rhs.offset_;
}

bool BlockData::operator<(const BlockData& rhs) const{
	if (this == &rhs) return false;
	return (chunk_->file_offset() < rhs.chunk_->file_offset()) || ((chunk_->file_offset() == rhs.chunk_->file_offset()) && (offset_ < rhs.offset_));
}

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


//const std::size_t ServerBlock::field_members_size_ = sizeof(int) + sizeof(int) + sizeof(dataPtr);
//std::size_t ServerBlock::allocated_bytes_ = 0;

//caller is responsible for updating memory usage accounting in DiskBackedBlockMap
//ServerBlock::ServerBlock(size_t size, bool initialize):
//		size_(size){
//	if (initialize){
//		data_ = new double[size_]();
//    } else {
//		data_ = new double[size_];
//    }
//	disk_state_.disk_status_[DiskBackingState::IN_MEMORY] = true;
//	disk_state_.disk_status_[DiskBackingState::ON_DISK] = false;
//	disk_state_.disk_status_[DiskBackingState::DIRTY_IN_MEMORY] = false;

	// Only Count number of bytes allocated for actual data.
//	const std::size_t bytes_in_block = field_members_size_ + size_ * sizeof(dataPtr);
//	allocated_bytes_ += bytes_in_block;
//	allocated_bytes_ += size_ * sizeof(double);
//}


//ServerBlock::ServerBlock(size_t size, dataPtr data):
//		size_(size), data_(data) {
////	disk_state_.disk_status_[DiskBackingState::IN_MEMORY] = (data_ == NULL) ? false : true;
////	disk_state_.disk_status_[DiskBackingState::ON_DISK] = false;
////	disk_state_.disk_status_[DiskBackingState::DIRTY_IN_MEMORY] = false;
//
//	// Only Count number of bytes allocated for actual data.
////	const std::size_t bytes_in_block = field_members_size_ + size_ * sizeof(dataPtr);
////	allocated_bytes_ += bytes_in_block;
////    if (data_ != NULL)
////    	allocated_bytes_ += size_ * sizeof(double);
//}

ServerBlock::~ServerBlock(){
////	const std::size_t bytes_in_block = size_ * sizeof(double);
//	std::stringstream ss;
//	if (data_ != NULL) {
//        async_state_.wait_all();
//		delete [] data_;
//	}
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
//	std::cout << "at server, in scale_data, first element of block is " << data_[0] << std::endl<< std::flush;
	return data_;
}

ServerBlock::dataPtr ServerBlock::increment_data( double delta) {
	double* data_ = block_data_.get_data();
	for (int i = 0; i < size(); ++i) {
		data_[i] += delta;
	}
	return data_;
}

//void ServerBlock::free_in_memory_data() {
//	if (data_ != NULL) {
//        async_state_.wait_all();
//		delete [] data_; data_ = NULL;
//		disk_state_.disk_status_[IN_MEMORY] = false;
//		disk_state_.disk_status_[DIRTY_IN_MEMORY] = false;
//		allocated_bytes_ -= size_ * sizeof(double);
//	}
//}

//void ServerBlock::allocate_in_memory_data(bool initialize) {
//   	sip::check(data_ == NULL, "data_ was not NULL, allocating memory in allocate_in_memory_data!");
//   	if (initialize)
//        data_ = new double[size_]();
//   	else
//        data_ = new double[size_];
//
//   	disk_state_.disk_status_[IN_MEMORY] = true;
//   	disk_state_.disk_status_[DIRTY_IN_MEMORY] = false;
//   	allocated_bytes_ += size_ * sizeof(double);
//}

std::ostream& operator<<(std::ostream& os, const ServerBlock& block) {
	os << block.block_data_ << std::endl;
	os << "END OF BLOCK" << std::endl;
	return os;
}

//void ServerBlock::reset_consistency_status (){
//	check(consistency_status_.first != INVALID_MODE &&
//			consistency_status_.second != INVALID_WORKER,
//			"Inconsistent block status !");
//	consistency_status_.first = NONE;
//	consistency_status_.second = OPEN;
//}


//bool ServerBlock::update_and_check_consistency(SIPMPIConstants::MessageType_t operation, int worker){
//
//	/**
//	 * Block Consistency Rules
//	 *
//	 *     		  GET,w    PUT,w    PUT_ACC,w   GET,w1     PUT,w1  PUT_ACC,w1
//	 *		NO      Rw      Ww         Aw         Rw1        Ww1     Aw1
//	 *		Rw      Rw      Sw         Sw         RM          X       X
//	 *		RM      RM       X          X          RM         X       X
//	 *		Ww      Sw      Sw         Sw          X          X       X
//	 *		Aw      Sw      Sw         Aw          X          X       AM
//	 *		AM       X       X         AM          X          X       AM
//	 *		Sw      Sw      Sw         Sw          X          X       X
//	 */
//
//	ServerBlockMode mode = consistency_status_.first;
//	int prev_worker = consistency_status_.second;
//
//	// Check if block already in inconsistent state.
//	if (mode == INVALID_MODE || prev_worker == INVALID_WORKER){
//		std::cout << "block in inconsistent state on entry to update and check "<< std::endl << std::flush;
//		return false;
//	}
//
//
//	ServerBlockMode new_mode = INVALID_MODE;
//	int new_worker = INVALID_WORKER;
//
//	switch (mode){
//	case NONE: {
//		/*  		  GET,w    PUT,w    PUT_ACC,w   GET,w1     PUT,w1  PUT_ACC,w1
//		 *		NO      Rw      Ww         Aw         Rw1        Ww1     Aw1
//		 */
//		if (OPEN == prev_worker){
//			switch(operation){
//			case SIPMPIConstants::GET : new_mode = READ; new_worker = worker; break;
//			case SIPMPIConstants::PUT : case SIPMPIConstants::PUT_INITIALIZE	:		new_mode = WRITE; 		new_worker = worker; break;
//			case SIPMPIConstants::PUT_ACCUMULATE :	case SIPMPIConstants::PUT_INCREMENT: case SIPMPIConstants::PUT_SCALE: new_mode = ACCUMULATE; 	new_worker = worker; break;
//			default : goto consistency_error;
//			}
//		} else {
//			goto consistency_error;
//		}
//	}
//		break;
//	case READ: {
//		/*  		  GET,w    PUT,w    PUT_ACC,w   GET,w1     PUT,w1  PUT_ACC,w1
//		 *		Rw      Rw      Sw         Sw         RM          X       X
//		 *		RM      RM       X          X          RM         X       X
//		 */
//		if (OPEN == prev_worker){
//			goto consistency_error;
//		} else if (MULTIPLE_WORKER == prev_worker){
//			switch(operation){
//			case SIPMPIConstants::GET :				new_mode = READ; new_worker = MULTIPLE_WORKER; break;
//			case SIPMPIConstants::PUT : case SIPMPIConstants::PUT_INITIALIZE: case SIPMPIConstants::PUT_ACCUMULATE : case SIPMPIConstants::PUT_INCREMENT: case SIPMPIConstants::PUT_SCALE:{	goto consistency_error; } break;
//			default : goto consistency_error;
//			}
//		} else { 	// Single worker
//			switch(operation){
//			case SIPMPIConstants::GET :	new_mode = READ; new_worker = (worker == prev_worker ? worker : MULTIPLE_WORKER); break;
//			case SIPMPIConstants::PUT : case SIPMPIConstants::PUT_ACCUMULATE : case SIPMPIConstants::PUT_SCALE:{
//				if (worker != prev_worker)
//					goto consistency_error;
//				new_mode = SINGLE_WORKER;
//				new_worker = worker;
//			}
//			break;
//			default : goto consistency_error;
//			}
//		}
//	}
//		break;
//	case WRITE:{
//		/*  		  GET,w    PUT,w    PUT_ACC,w   GET,w1     PUT,w1  PUT_ACC,w1
//		 *		Ww      Sw      Sw         Sw          X          X       X
//		 */
//		if (prev_worker == worker){
//			new_mode = SINGLE_WORKER; new_worker = worker;
//		} else {
//			goto consistency_error;
//		}
//	}
//	break;
//	case ACCUMULATE:{
//		/*  		  GET,w    PUT,w    PUT_ACC,w   GET,w1     PUT,w1  PUT_ACC,w1
//		 *		Aw      Sw      Sw         Aw          X          X       AM
//		 *		AM       X       X         AM          X          X       AM
//		 */
//		if (OPEN == prev_worker){
//			goto consistency_error;
//		} else if (MULTIPLE_WORKER == prev_worker){
//			if (SIPMPIConstants::PUT_ACCUMULATE == operation || SIPMPIConstants::PUT_INCREMENT == operation || SIPMPIConstants::PUT_SCALE == operation){
//				new_mode = ACCUMULATE; new_worker = MULTIPLE_WORKER;
//			} else {
//				goto consistency_error;
//			}
//		} else { // Single worker
//			switch(operation){
//			case SIPMPIConstants::GET : case SIPMPIConstants::PUT : case SIPMPIConstants::PUT_INITIALIZE: {
//				if (prev_worker != worker)
//					goto consistency_error;
//				new_worker = worker;
//				new_mode = SINGLE_WORKER;
//			}
//			break;
//			case SIPMPIConstants::PUT_ACCUMULATE :	case SIPMPIConstants::PUT_SCALE : case SIPMPIConstants::PUT_INCREMENT : {
//				new_mode = ACCUMULATE;
//				new_worker = (worker == prev_worker ? worker : MULTIPLE_WORKER);
//			}
//			break;
//			default : goto consistency_error;
//			}
//		}
//	}
//	break;
//	case SINGLE_WORKER:{
//		/*  		  GET,w    PUT,w    PUT_ACC,w   GET,w1     PUT,w1  PUT_ACC,w1
//		 *		Sw      Sw      Sw         Sw          X          X       X
//		 */
//		if (worker == prev_worker){
//			new_mode = SINGLE_WORKER; new_worker = worker;
//		} else {
//			goto consistency_error;
//		}
//	}
//	break;
//	default:
//		goto consistency_error;
//	}
//
//	consistency_status_.first = new_mode;
//	consistency_status_.second = new_worker;
//	return true;
//
//consistency_error:
//std::cout << "Inconsistent block at server " << consistency_status_.first << "," << consistency_status_.second << std::endl << std::flush;
//	SIP_LOG(std::cout << "Inconsistent block at server ")
//	consistency_status_.first = INVALID_MODE;
//	consistency_status_.second = INVALID_WORKER;
//
//	return false;
//
//}

//std::size_t ServerBlock::allocated_bytes(){
//	return allocated_bytes_;
//}


} /* namespace sip */
