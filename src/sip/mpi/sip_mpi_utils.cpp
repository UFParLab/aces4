/*
 * sip_mpi_utils.cpp
 *
 *  Created on: Jan 17, 2014
 *      Author: njindal
 */

#include <sip_mpi_utils.h>

#include "mpi.h"
#include "sip_mpi_attr.h"

#include <memory>
#include <cstdio>
#include <cstring>

namespace sip {


void SIPMPIUtils::set_error_handler(){
//	std::cout << "  in SIPMPIUtils::set_error_handler" << std::endl << std::flush;
	MPI_Errhandler_set(MPI_COMM_WORLD, MPI_ERRORS_RETURN);
	SIPMPIAttr & mpi_attr = SIPMPIAttr::get_instance();
//	std::cout << "Rank " << mpi_attr.global_rank()  << "  in SIPMPIUtils::set_error_handler:  mpi_attr.company_communicator()!=NULL " << (mpi_attr.company_communicator() != NULL) << std::endl << std::flush;
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
	    printf("Error %d:%s\n", eclass, estring);
	    fflush(stdout);
        fail("MPI Error !\n");
	}
}
void SIPMPIUtils::check_err(int err, int line, char * file){

	if (err == MPI_SUCCESS){
		return;
	} else {
		int len, eclass;
	    char estring[MPI_MAX_ERROR_STRING];
	    MPI_Error_class(err, &eclass);
	    MPI_Error_string(err, estring, &len);
	    printf("Error %d: %s at line %d in file %s\n", eclass, estring, line, file);
	    fflush(stdout);
        fail("MPI Error !\n");
	}
}
/*
void SIPMPIUtils::get_block_params(const int rank, int tag, BlockId* bid, BlockShape* shape, int *data_size) {

	// Get BlockId, shape, size
	MPI_Status status;
	int size = 1 + MAX_RANK + MAX_RANK + 1;
	int *to_send = new int[size];

	check_err(MPI_Recv(to_send, size, MPI_INT, rank, tag, MPI_COMM_WORLD,
					&status));
	int array_id_;
	index_value_array_t index_values_;
	segment_size_array_t segment_sizes_;
	int size_;

	memcpy(&array_id_, to_send, sizeof(array_id_));
	memcpy(&index_values_, to_send + 1, sizeof(index_values_));
	memcpy(&segment_sizes_, to_send + 1 + MAX_RANK, sizeof(segment_sizes_));
	memcpy(&size_, to_send + + 1 + MAX_RANK + MAX_RANK, sizeof(size_));

	bid->array_id_ = array_id_;
	std::copy(index_values_ + 0, index_values_ + MAX_RANK, bid->index_values_);
	std::copy(segment_sizes_ + 0, segment_sizes_ + MAX_RANK, shape->segment_sizes_);
	*data_size = size_;

	delete [] to_send;
}

void SIPMPIUtils::get_data_from_rank(int rank, int tag, int size, double* data) {

	check(data != NULL, "Block Pointer into which data is to be received is NULL !", current_line());
	MPI_Status status;
	check_err(MPI_Recv(data, size, MPI_DOUBLE, rank, tag, MPI_COMM_WORLD, &status));
	SIP_LOG(std::cout<< "W " << SIPMPIAttr::get_instance().global_rank() << " : Got Block Data with tag : "<< tag << " from rank " << rank << std::endl);
	//SIP_LOG(std::cout<<"Got block ptr : " << *data << std::endl);
}

BlockId SIPMPIUtils::get_block_id_from_rank(int rank, int tag) {
	// Receive a message with the block being requested - blockid
	int bid_int_size = BlockId::serialized_size();
	int* recv  = new int[bid_int_size];
	MPI_Status get_status;
	check_err(MPI_Recv(recv, bid_int_size, MPI_INT, rank, tag, MPI_COMM_WORLD, &get_status));

	int blkid_msg_size_;
	check_err(MPI_Get_count(&get_status, MPI_INT, &blkid_msg_size_));
	check(bid_int_size == blkid_msg_size_, "Expected block Id size not correct !");

	BlockId bid = BlockId::deserialize(recv);
	SIP_LOG(std::cout<<SIPMPIAttr::get_instance().global_rank()<<" : Got block id : " << bid << std::endl);

	delete [] recv;

	return bid;
}


void SIPMPIUtils::send_block_id_to_rank(const BlockId& id, int rank, int tag) {
	int size = -1;
	int* send_bytes = BlockId::serialize(id, size);
	SIPMPIUtils::check_err(MPI_Send(send_bytes, size, MPI_INT, rank, tag, MPI_COMM_WORLD));
	delete [] send_bytes;
}


void SIPMPIUtils::isend_bid_and_bptr_to_rank(const BlockId& bid, Block::BlockPtr bptr, int rank, int size_tag, int data_tag, MPI_Request *request) {
	int blk_size;

	// Send BlockID, data size, shape
	check(bptr != NULL, "Block to send is NULL!", current_line());

	int int_size = 1 + MAX_RANK + MAX_RANK + 1;
	int *to_send = new int[int_size];
	memcpy(to_send, &(bid.array_id_), sizeof(bid.array_id_));
	memcpy(to_send + 1 , &(bid.index_values_), sizeof(bid.index_values_));
	memcpy(to_send + 1 + MAX_RANK, &(bptr->shape_.segment_sizes_), sizeof(bptr->shape_.segment_sizes_));
	memcpy(to_send + 1 + MAX_RANK + MAX_RANK, &(bptr->size_), sizeof(bptr->size_));

	SIP_LOG(std::cout<< SIPMPIAttr::get_instance().global_rank() << " : Sending Block Info with tag : "<< size_tag << " to rank " << rank << std::endl);
	MPI_Request block_info_request;
	check_err(MPI_Isend(to_send, int_size, MPI_INT, rank, size_tag, MPI_COMM_WORLD, &block_info_request));

	// Send double precision data
	double * ddata = bptr->data_;
	SIP_LOG(std::cout<< SIPMPIAttr::get_instance().global_rank() << " : Sending Block Data with tag : "<< data_tag << " and size " << bptr->size_ <<  " to rank " << rank << std::endl);
	check_err(MPI_Isend(ddata, bptr->size_, MPI_DOUBLE, rank, data_tag, MPI_COMM_WORLD, request));

	delete [] to_send;
	//SIP_LOG(std::cout<<"Sent block ptr : " << *bptr << std::endl);
}


MPI_Request SIPMPIUtils::isend_block_data_to_rank(Block::dataPtr data, int size, int rank, int tag){
	MPI_Request request;
	check_err(MPI_Isend(data, size, MPI_DOUBLE, rank, tag, MPI_COMM_WORLD, &request));
	//check_err(MPI_Send(data, size, MPI_DOUBLE, rank, tag, MPI_COMM_WORLD));
	return request;
}


//void SIPMPIUtils::send_str_to_rank(const int rank, char *str, const int len, const int tag) {
//	check(len < SIPMPIData::MAX_STRING, "Trying to send a very large string to other servers !");
//	check_err(MPI_Send(str, len, MPI_CHAR, rank, tag, MPI_COMM_WORLD));
//
//}


void SIPMPIUtils::send_ack_to_rank(const int rank, int ack, const int tag){
	SIP_LOG(std::cout<< SIPMPIAttr::get_instance().global_rank() << " : Sending Ack with tag : "<< tag << " to rank " << rank << std::endl);
	check_err(MPI_Send(&ack, 1, MPI_INT, rank, tag, MPI_COMM_WORLD));
}

void SIPMPIUtils::expect_ack_from_rank(const int rank, int ack, const int tag){
	int recvd_ack;
	MPI_Status status;
	SIP_LOG(std::cout<< SIPMPIAttr::get_instance().global_rank() << " : Expecting Ack with tag : "<< tag << " from rank " << rank << std::endl);
	check_err(MPI_Recv(&recvd_ack, 1, MPI_INT, rank, MPI_ANY_TAG, MPI_COMM_WORLD, &status));
	int recvd_tag = status.MPI_TAG;
	check(recvd_ack == ack, "Did not receive expected ACK !", current_line());
	check(recvd_tag == tag, "Did not receive expected Tag !", current_line());
}

void SIPMPIUtils::expect_async_ack_from_rank(const int rank, int ack, const int tag, MPI_Request *request){
	int recvd_ack;
	SIP_LOG(std::cout<< SIPMPIAttr::get_instance().global_rank() << " : Posting irecv for Ack with tag : "<< tag << " from rank " << rank << std::endl);
	check_err(MPI_Irecv(&recvd_ack, 1, MPI_INT, rank, tag, MPI_COMM_WORLD, request));
}



SIPMPIData::MessageType_t SIPMPIUtils::get_message_type(int mpi_tag){
	SIPMPITagBitFieldConverter bc;
	bc.i = mpi_tag;
	return SIPMPIData::intToMessageType(bc.bf.message_type);
}

int SIPMPIUtils::get_section_number(int mpi_tag){
	SIPMPITagBitFieldConverter bc;
	bc.i = mpi_tag;
	return bc.bf.section_number;
}

int SIPMPIUtils::get_message_number(int mpi_tag){
	SIPMPITagBitFieldConverter bc;
	bc.i = mpi_tag;
	return bc.bf.message_number;
}

int SIPMPIUtils::make_mpi_tag(SIPMPIData::MessageType_t message_type, int section_number, int message_number){
	SIPMPITagBitFieldConverter bc;
	bc.bf.message_type = message_type;
	bc.bf.section_number = section_number;
	bc.bf.message_number = message_number;
	return bc.i;
}
*/

} /* namespace sip */
