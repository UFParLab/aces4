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

#include "TAU.h"

namespace sip {

const std::size_t ServerBlock::field_members_size_ = sizeof(int) + sizeof(int) + sizeof(dataPtr);
std::size_t ServerBlock::allocated_bytes_ = 0;

ServerBlock::ServerBlock(int size, bool initialize)
        : size_(size)
        , consistency_status_(std::make_pair(NONE, OPEN))
        , isOnMPI(false)
        , MPI_operation_type(MPI_NONE)
        , mpi_request()
        {
    
	if (initialize){
        TAU_START("double allocate true");
		data_ = new double[size_]();
        TAU_STOP("double allocate true");
    } else {
        TAU_START("double allocate");
		data_ = new double[size_];
        TAU_STOP("double allocate");
    }

	disk_status_[ServerBlock::IN_MEMORY] = true;
	disk_status_[ServerBlock::ON_DISK] = false;
	disk_status_[ServerBlock::DIRTY_IN_MEMORY] = false;

	// Only Count number of bytes allocated for actual data.
//	const std::size_t bytes_in_block = field_members_size_ + size_ * sizeof(dataPtr);
//	allocated_bytes_ += bytes_in_block;
	allocated_bytes_ += size_ * sizeof(double);
}

ServerBlock::ServerBlock(int size, dataPtr data)
        : size_(size)
        , data_(data)
        , consistency_status_(std::make_pair(NONE, OPEN)) 
        , isOnMPI(false)
        , MPI_operation_type(MPI_NONE)
        , mpi_request()
        {
        
	disk_status_[ServerBlock::IN_MEMORY] = (data_ == NULL) ? false : true;
	disk_status_[ServerBlock::ON_DISK] = false;
	disk_status_[ServerBlock::DIRTY_IN_MEMORY] = false;

	// Only Count number of bytes allocated for actual data.
//	const std::size_t bytes_in_block = field_members_size_ + size_ * sizeof(dataPtr);
//	allocated_bytes_ += bytes_in_block;
    if (data_ != NULL)
    	allocated_bytes_ += size_ * sizeof(double);
}

ServerBlock::~ServerBlock(){
	const std::size_t bytes_in_block = size_ * sizeof(double);
	std::stringstream ss;
	if (data_ != NULL) {
		ss << "Allocated bytes [ " << allocated_bytes_ <<" ] less than size of block being destroyed "
				<< " [ " << bytes_in_block << " ] ";
		sip::check(allocated_bytes_ >= bytes_in_block, ss.str());
		allocated_bytes_ -= bytes_in_block;

		sip::check(disk_status_[ServerBlock::IN_MEMORY], "ServerBlock not in memory yet data_ is not NULL");
		delete [] data_;
	}
}

ServerBlock::dataPtr ServerBlock::accumulate_data(size_t size, dataPtr to_add){
	check(size_ == size, "accumulating blocks of unequal size");
	for (unsigned i = 0; i < size; ++i){
			data_[i] += to_add[i];
	}
	return data_;
}


void ServerBlock::free_in_memory_data() {
	if (data_ != NULL) {
        if (isOnMPI)
        {
            MPI_Wait(&mpi_request, MPI_STATUS_IGNORE);
            isOnMPI = false;
        }
    
		delete [] data_; data_ = NULL;
		disk_status_[ServerBlock::IN_MEMORY] = false;
		disk_status_[ServerBlock::DIRTY_IN_MEMORY] = false;
		allocated_bytes_ -= size_ * sizeof(double);
	}
}

void ServerBlock::allocate_in_memory_data(bool initialize) {
   	sip::check(data_ == NULL, "data_ was not NULL, allocating memory in allocate_in_memory_data!");
   	if (initialize) 
        data_ = new double[size_]();
   	else 
        data_ = new double[size_];

   	disk_status_[ServerBlock::IN_MEMORY] = true;
   	disk_status_[ServerBlock::DIRTY_IN_MEMORY] = false;
   	allocated_bytes_ += size_ * sizeof(double);
}

std::ostream& operator<<(std::ostream& os, const ServerBlock& block) {
	os << "SIZE=" << block.size_ << std::endl;
	int i = 0;
	const int MAX_TO_PRINT = 800;
	const int MAX_IN_ROW = 80;
	int size = block.size_;
	if (block.data_ == NULL) {
		os << " data_ = NULL";
	} else {
		while (i < size && i < MAX_TO_PRINT) {
			for (int j = 0; j < MAX_IN_ROW && i < size; ++j) {
				os << block.data_[i++] << "  ";
			}
			os << '\n';
		}
	}
	os << "END OF BLOCK" << std::endl;
	return os;
}

void ServerBlock::reset_consistency_status (){
	check(consistency_status_.first != INVALID_MODE &&
			consistency_status_.second != INVALID_WORKER,
			"Inconsistent block status !");
	consistency_status_.first = NONE;
	consistency_status_.second = OPEN;
}


bool ServerBlock::update_and_check_consistency(SIPMPIConstants::MessageType_t operation, int worker){
	/**
	 * Block Consistency Rules
	 *
	 *     		  GET,w    PUT,w    PUT_ACC,w   GET,w1     PUT,w1  PUT_ACC,w1
	 *		NO      Rw      Ww         Aw         Rw1        Ww1     Aw1
	 *		Rw      Rw      Sw         Sw         RM          X       X
	 *		RM      RM       X          X          RM         X       X
	 *		Ww      Sw      Sw         Sw          X          X       X
	 *		Aw      Sw      Sw         Aw          X          X       AM
	 *		AM       X       X         AM          X          X       AM
	 *		Sw      Sw      Sw         Sw          X          X       X
	 */

	ServerBlockMode mode = consistency_status_.first;
	int prev_worker = consistency_status_.second;

	// Check if block already in inconsistent state.
	if (mode == INVALID_MODE || prev_worker == INVALID_WORKER)
		return false;


	ServerBlockMode new_mode = INVALID_MODE;
	int new_worker = INVALID_WORKER;

	switch (mode){
	case NONE: {
		/*  		  GET,w    PUT,w    PUT_ACC,w   GET,w1     PUT,w1  PUT_ACC,w1
		 *		NO      Rw      Ww         Aw         Rw1        Ww1     Aw1
		 */
		if (OPEN == prev_worker){
			switch(operation){
			case SIPMPIConstants::GET : 			new_mode = READ; 		new_worker = worker; break;
			case SIPMPIConstants::PUT : 			new_mode = WRITE; 		new_worker = worker; break;
			case SIPMPIConstants::PUT_ACCUMULATE :	new_mode = ACCUMULATE; 	new_worker = worker; break;
			default : goto consistency_error;
			}
		} else {
			goto consistency_error;
		}
	}
		break;
	case READ: {
		/*  		  GET,w    PUT,w    PUT_ACC,w   GET,w1     PUT,w1  PUT_ACC,w1
		 *		Rw      Rw      Sw         Sw         RM          X       X
		 *		RM      RM       X          X          RM         X       X
		 */
		if (OPEN == prev_worker){
			goto consistency_error;
		} else if (MULTIPLE_WORKER == prev_worker){
			switch(operation){
			case SIPMPIConstants::GET :				new_mode = READ; new_worker = MULTIPLE_WORKER; break;
			case SIPMPIConstants::PUT : case SIPMPIConstants::PUT_ACCUMULATE : {	goto consistency_error; } break;
			default : goto consistency_error;
			}
		} else { 	// Single worker
			switch(operation){
			case SIPMPIConstants::GET :	new_mode = READ; new_worker = (worker == prev_worker ? worker : MULTIPLE_WORKER); break;
			case SIPMPIConstants::PUT : case SIPMPIConstants::PUT_ACCUMULATE :{
				if (worker != prev_worker)
					goto consistency_error;
				new_mode = SINGLE_WORKER;
				new_worker = worker;
			}
			break;
			default : goto consistency_error;
			}
		}
	}
		break;
	case WRITE:{
		/*  		  GET,w    PUT,w    PUT_ACC,w   GET,w1     PUT,w1  PUT_ACC,w1
		 *		Ww      Sw      Sw         Sw          X          X       X
		 */
		if (prev_worker == worker){
			new_mode = SINGLE_WORKER; new_worker = worker;
		} else {
			goto consistency_error;
		}
	}
	break;
	case ACCUMULATE:{
		/*  		  GET,w    PUT,w    PUT_ACC,w   GET,w1     PUT,w1  PUT_ACC,w1
		 *		Aw      Sw      Sw         Aw          X          X       AM
		 *		AM       X       X         AM          X          X       AM
		 */
		if (OPEN == prev_worker){
			goto consistency_error;
		} else if (MULTIPLE_WORKER == prev_worker){
			if (SIPMPIConstants::PUT_ACCUMULATE == operation){
				new_mode = ACCUMULATE; new_worker = MULTIPLE_WORKER;
			} else {
				goto consistency_error;
			}
		} else { // Single worker
			switch(operation){
			case SIPMPIConstants::GET : case SIPMPIConstants::PUT : {
				if (prev_worker != worker)
					goto consistency_error;
				new_worker = worker;
				new_mode = SINGLE_WORKER;
			}
			break;
			case SIPMPIConstants::PUT_ACCUMULATE :	{
				new_mode = ACCUMULATE;
				new_worker = (worker == prev_worker ? worker : MULTIPLE_WORKER);
			}
			break;
			default : goto consistency_error;
			}
		}
	}
	break;
	case SINGLE_WORKER:{
		/*  		  GET,w    PUT,w    PUT_ACC,w   GET,w1     PUT,w1  PUT_ACC,w1
		 *		Sw      Sw      Sw         Sw          X          X       X
		 */
		if (worker == prev_worker){
			new_mode = SINGLE_WORKER; new_worker = worker;
		} else {
			goto consistency_error;
		}
	}
	break;
	default:
		goto consistency_error;
	}

	consistency_status_.first = new_mode;
	consistency_status_.second = new_worker;
	return true;

consistency_error:
	SIP_LOG(std::cout << "Inconsistent block at server ")
	consistency_status_.first = INVALID_MODE;
	consistency_status_.second = INVALID_WORKER;

	return false;

}

std::size_t ServerBlock::allocated_bytes(){
	return allocated_bytes_;
}


} /* namespace sip */