/*
 * sip_mpi_utils.cpp
 *
 *  Created on: Jan 17, 2014
 *      Author: njindal
 */

#include <sip_mpi_utils.h>

#include "mpi.h"
#include "sip_mpi_data.h"
#include "sip_mpi_attr.h"

#include <memory>

namespace sip {


void SIPMPIUtils::set_error_handler(){
	MPI_Errhandler_set(MPI_COMM_WORLD, MPI_ERRORS_RETURN);
	SIPMPIAttr & mpi_attr = SIPMPIAttr::get_instance();
	MPI_Errhandler_set(mpi_attr.company_communicator(), MPI_ERRORS_RETURN);
}

void SIPMPIUtils::check_err(int err){

	if (err == MPI_SUCCESS){
		return;
	} else {
		int len, eclass;
	    char estring[MPI_MAX_ERROR_STRING];
	    MPI_Error_class(err, &eclass);
	    MPI_Error_string(err, estring, &len);
	    printf("Error %d: %s\n", eclass, estring);
	    fflush(stdout);
        sip::fail("MPI Error !\n");
	}
}

void SIPMPIUtils::get_block_params(const int rank, const int size_tag, sip::BlockId& bid, sip::BlockShape& shape, int &data_size) {

	// Get BlockId, shape, size
	MPI_Status blk_size_status;
	int int_size = 1 + MAX_RANK + MAX_RANK + 1;
	int *to_send = new int[int_size];

	check_err(MPI_Recv(to_send, int_size, MPI_INT, rank, size_tag, MPI_COMM_WORLD,
					&blk_size_status));
	int array_id_;
	sip::index_value_array_t index_values_;
	sip::segment_size_array_t segment_sizes_;
	int size_;

	memcpy(&array_id_, to_send, sizeof(array_id_));
	memcpy(&index_values_, to_send + 1, sizeof(index_values_));
	memcpy(&segment_sizes_, to_send + 1 + MAX_RANK, sizeof(segment_sizes_));
	memcpy(&size_, to_send + + 1 + MAX_RANK + MAX_RANK, sizeof(size_));

//	std::copy(to_send, to_send + 1, &array_id_);
//	std::copy(to_send + 1, to_send + MAX_RANK, &index_values_);
//	std::copy(to_send + 1 + MAX_RANK, to_send + MAX_RANK, &segment_sizes_);
//	std::copy(to_send + 1 + MAX_RANK + MAX_RANK, to_send + 1, &size_);

	bid.array_id_ = array_id_;
	std::copy(index_values_ + 0, index_values_ + MAX_RANK, bid.index_values_);
	std::copy(segment_sizes_ + 0, segment_sizes_ + MAX_RANK, shape.segment_sizes_);
	data_size = size_;
}

void SIPMPIUtils::get_bptr_from_rank(int rank, int data_tag, int size, sip::Block::BlockPtr bptr) {

	sip::check(bptr != NULL, "Block Pointer into which data is to be received is NULL !", current_line());
	MPI_Status blk_data_status;
	check_err(MPI_Recv(bptr->data_, size, MPI_DOUBLE, rank, data_tag, MPI_COMM_WORLD, &blk_data_status));
	//SIP_LOG(std::cout<<"Got block ptr : " << *bptr << std::endl);
}

void SIPMPIUtils::send_block_id_to_rank(const sip::BlockId& id, int rank, int tag) {
	int size = -1;
	int* send_bytes = sip::BlockId::serialize(id, size);
	sip::SIPMPIUtils::check_err(MPI_Send(send_bytes, size, MPI_INT, rank, tag, MPI_COMM_WORLD));
	delete [] send_bytes;
}

sip::BlockId SIPMPIUtils::get_block_id_from_rank(int rank, int block_id_tag) {
	// Receive a message with the block being requested - blockid
	int bid_int_size = sip::BlockId::serialized_size();
	int* recv  = new int[bid_int_size];
	MPI_Status get_status;
	check_err(MPI_Recv(recv, bid_int_size, MPI_INT, rank, block_id_tag, MPI_COMM_WORLD, &get_status));
	int blkid_msg_size_;
	check_err(MPI_Get_count(&get_status, MPI_INT, &blkid_msg_size_));
	sip::check(bid_int_size == blkid_msg_size_, "Expected block Id size not correct !");

	sip::BlockId bid = sip::BlockId::deserialize(recv);

	SIP_LOG(std::cout<<sip::SIPMPIAttr::get_instance().global_rank()<<" : Got block id : " << bid << std::endl);

	delete [] recv;

	return bid;
}


void SIPMPIUtils::send_bptr_to_rank(const sip::BlockId& bid, sip::Block::BlockPtr bptr, int rank, int size_tag, int data_tag) {
	int blk_size;
	//char* send_bytes = array::Block::serialize(bptr, blk_size);

	//check_err(MPI_Send(&blk_size, 1, MPI_INT, rank, size_tag, MPI_COMM_WORLD));
	//check_err(MPI_Send(send_bytes, blk_size, MPI_BYTE, rank, data_tag, MPI_COMM_WORLD));
	//delete [] send_bytes;

	// Send BlockID, data size, shape
	sip::check(bptr != NULL, "Block to send is NULL!", current_line());
	//int size_bytes = sizeof(int) + sizeof(array::index_value_array_t) + sizeof(array::segment_size_array_t) + sizeof(int);
	//int size = size_bytes / sizeof(int);

	int int_size = 1 + MAX_RANK + MAX_RANK + 1;
	int *to_send = new int[int_size];
	memcpy(to_send, &(bid.array_id_), sizeof(bid.array_id_));
	memcpy(to_send + 1 , &(bid.index_values_), sizeof(bid.index_values_));
	memcpy(to_send + 1 + MAX_RANK, &(bptr->shape_.segment_sizes_), sizeof(bptr->shape_.segment_sizes_));
	memcpy(to_send + 1 + MAX_RANK + MAX_RANK, &(bptr->size_), sizeof(bptr->size_));
//	int * _ts = std::copy(&(bid.array_id_), &(bid.array_id_) + 1, to_send);
//	_ts = std::copy(&(bid.index_values_), &(bid.index_values_) + MAX_RANK, _ts);
//	_ts = std::copy(&(bptr->shape_.segment_sizes_), &(bptr->shape_.segment_sizes_) + MAX_RANK, _ts);
//	_ts = std::copy(&(bptr->size_), &(bptr->size_) + 1, _ts);

	check_err(MPI_Send(to_send, int_size, MPI_INT, rank, size_tag, MPI_COMM_WORLD));

	// Send double precision data
	double * ddata = bptr->data_;
	check_err(MPI_Send(ddata, bptr->size_, MPI_DOUBLE, rank, data_tag, MPI_COMM_WORLD));

	delete [] to_send;
	//SIP_LOG(std::cout<<"Sent block ptr : " << *bptr << std::endl);
}

void SIPMPIUtils::send_str_to_rank(const int rank, char *str, const int len, const int tag) {
	sip::check(len < SIPMPIData::MAX_STRING, "Trying to send a very large string to other servers !");
	check_err(MPI_Send(str, len, MPI_CHAR, rank, tag, MPI_COMM_WORLD));

}


void SIPMPIUtils::send_ack_to_rank(const int rank, int ack, const int tag){
	check_err(MPI_Send(&ack, 1, MPI_INT, rank, tag, MPI_COMM_WORLD));
}

void SIPMPIUtils::expect_ack_from_rank(const int rank, int ack, const int tag){
	int recvd_ack;
	MPI_Status status;
	check_err(MPI_Recv(&recvd_ack, 1, MPI_INT, rank, tag, MPI_COMM_WORLD, &status));
	sip::check(recvd_ack == ack, "Did not receive expected ACK !", current_line());
}


} /* namespace sip */
