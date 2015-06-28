/*
 * async_state.cpp
 *
 *  Created on: Jun 22, 2015
 *      Author: basbas
 */

#include "async_ops.h"
#include "server_block.h"

namespace sip {




/************* GetAsync ************/

GetAsync::GetAsync(int mpi_source, int get_tag, ServerBlock* block, int pc) :
		AsyncBase(pc), mpi_request_() {
	//send block
	SIPMPIUtils::check_err(
			MPI_Isend(block->get_data(), block->size(), MPI_DOUBLE, mpi_source,
					get_tag, MPI_COMM_WORLD, &mpi_request_), __LINE__,
			__FILE__);
}



/**  PutAccumulateAsync *******************/

PutAccumulateDataAsync::PutAccumulateDataAsync(int mpi_source,
		int put_accumulate_data_tag, ServerBlock* block, int pc) :
		AsyncBase(pc), mpi_source_(mpi_source), tag_(put_accumulate_data_tag),  block_(
				block), mpi_request_(MPI_REQUEST_NULL) {
	//allocate temp buffer
	temp_ = new double[block->size()];
	//post receive
	SIPMPIUtils::check_err(
			MPI_Irecv(temp_, block->size(), MPI_DOUBLE, mpi_source,
					put_accumulate_data_tag, MPI_COMM_WORLD, &mpi_request_), __LINE__,
			__FILE__);
	//reply should be sent in calling routing AFTER this object has been constructed,
	//so the Irecv will already be posted.
}

bool PutAccumulateDataAsync::do_test() {
	int flag = 0;
	MPI_Status status;
	MPI_Test(&mpi_request_, &flag, &status);
	if (flag) {
		//check that received message was expected size
		int count;
		MPI_Get_count(&status, MPI_DOUBLE, &count);
		check(count == block_->size(), "count != block_->size()");
	}
	return flag;
}

void PutAccumulateDataAsync::do_handle() {
	//send ack to worker
	SIPMPIUtils::check_err(
			MPI_Send(0, 0, MPI_INT, mpi_source_, tag_, MPI_COMM_WORLD),
			__LINE__, __FILE__);
	//accumulate received data into block
	block_->accumulate_data(block_->size(), temp_);
}

void PutAccumulateDataAsync::do_wait() {
	MPI_Status status;
	MPI_Wait(&mpi_request_, &status);
	//check that received message was expected size
	int count;
	MPI_Get_count(&status, MPI_DOUBLE, &count);
	check(count == block_->size(), "count != block_->size()");
}

std::string PutAccumulateDataAsync::to_string() const {
	std::stringstream ss;
	ss << "PutAccumulateDataAsync";
	ss << AsyncBase::to_string();
	ss << " source=" << mpi_source_ << " tag=" << tag_;
	int transaction;
	SIPMPIConstants::MessageType_t message_type;
	BarrierSupport::decode_tag(tag_, message_type, transaction);
	ss << " message type="
			<< SIPMPIConstants::messageTypeToName(
					SIPMPIConstants::intToMessageType(message_type))
			<< " transaction=" << transaction << std::cout;
	return ss.str();
}


/******************* PutDataAsync **********/

PutDataAsync::PutDataAsync(int mpi_source, int put_data_tag, ServerBlock* block,
		int pc) :
		AsyncBase(pc), mpi_source_(mpi_source), tag_(put_data_tag), block_(
				block), mpi_request_(MPI_REQUEST_NULL) {
	//post receive
	SIPMPIUtils::check_err(
			MPI_Irecv(block->get_data(), block->size(), MPI_DOUBLE, mpi_source,
					put_data_tag, MPI_COMM_WORLD, &mpi_request_), __LINE__,
			__FILE__);
}

bool PutDataAsync::do_test() {
	int flag = 0;
	MPI_Status status;
	MPI_Test(&mpi_request_, &flag, &status);
	if (flag) {
		int count;
		MPI_Get_count(&status, MPI_DOUBLE, &count);
		check(count == block_->size(), "count != block->size()");
	}
	return flag;
}

void PutDataAsync::do_wait() {
	MPI_Status status;
	MPI_Wait(&mpi_request_, &status);
	//check that received message was expected size
	int count;
	MPI_Get_count(&status, MPI_DOUBLE, &count);
	check(count == block_->size(), "count != block_->size()");
}

void PutDataAsync::do_handle() {
	//send ack to worker
	SIPMPIUtils::check_err(
			MPI_Send(0, 0, MPI_INT, mpi_source_, tag_, MPI_COMM_WORLD),
			__LINE__, __FILE__);
}

std::string PutDataAsync::to_string() const {
	std::stringstream ss;
	ss << "PutDataAsync";
	ss << AsyncBase::to_string();
	ss << " source=" << mpi_source_ << " tag=" << tag_;
	int transaction;
	SIPMPIConstants::MessageType_t message_type;
	BarrierSupport::decode_tag(tag_, message_type, transaction);
	ss << " message type="
			<< SIPMPIConstants::messageTypeToName(
					SIPMPIConstants::intToMessageType(message_type))
			<< " transaction=" << transaction << std::cout;
	return ss.str();
}















void ServerBlockAsyncManager::add_get_reply(int mpi_source, int get_tag,
		ServerBlock* block, int pc) {
pending_.push_back(new GetAsync(mpi_source, get_tag, block,  pc));

}

void ServerBlockAsyncManager::add_put_data_request(int mpi_source, int put_data_tag,
	ServerBlock* block, int pc) {
//create async op (which posts irecv
pending_.push_back(
		new PutDataAsync(mpi_source, put_data_tag, block, pc));

}

void ServerBlockAsyncManager::add_put_accumulate_data_request(int mpi_source,
	int put_accumulate_data_tag, ServerBlock* block, int pc) {
//create async op (which posts irecv

pending_.push_back(
		new PutAccumulateDataAsync(mpi_source, put_accumulate_data_tag, block, pc));


}

} /* namespace sip */
