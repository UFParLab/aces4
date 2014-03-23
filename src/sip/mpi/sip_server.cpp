/*
 * sip_server.cpp
 *
 */

#include <sip_server.h>
#include "mpi.h"
#include "sip.h"
#include "blocks.h"
#include "sip_mpi_utils.h"
#include <sstream>


namespace sip {

SIPServer::SIPServer(SipTables& sip_tables, DataDistribution &data_distribution, SIPMPIAttr & sip_mpi_attr,
		PersistentArrayManager<ServerBlock>& persistent_array_manager):
		sip_tables_(sip_tables), data_distribution_(data_distribution), sip_mpi_attr_(sip_mpi_attr),
		persistent_array_manager_(persistent_array_manager),
		terminated_(false){
}

SIPServer::~SIPServer() {}

void SIPServer::run(){

	while (!terminated_){

		MPI_Status status;

		// Check to see if a request has arrived from any worker.
		SIPMPIUtils::check_err(MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status));

		int mpi_tag = status.MPI_TAG;
		int mpi_source = status.MPI_SOURCE;

		SIPMPIData::MessageType_t message_type;
		int section_number;
		int message_number;

		state_.decode_tag_and_check_invariant(mpi_tag, message_type, section_number, message_number);

		switch (message_type) {
		//case SIPMPIData::CREATE:
		//	// NO OP - Create blocks on the fly.
		//	break;
		case SIPMPIData::GET :
			handle_GET(mpi_source, mpi_tag);
			break;
		case SIPMPIData::PUT :
			handle_PUT(mpi_source, mpi_tag);
			break;
		case SIPMPIData::PUT_ACCUMULATE:
			handle_PUT_ACCUMULATE(mpi_source, mpi_tag);
			break;
		case SIPMPIData::DELETE:
			handle_DELETE(mpi_source, mpi_tag);
			break;
		case SIPMPIData::SET_PERSISTENT:
			handle_SET_PERSISTENT(mpi_source, mpi_tag);
			break;
		case SIPMPIData::RESTORE_PERSISTENT:
			handle_RESTORE_PERSISTENT(mpi_source, mpi_tag);
			break;
		case SIPMPIData::END_PROGRAM:
			handle_END_PROGRAM(mpi_source, mpi_tag);
			return;
		default:
			fail("Received unexpected message in SIP Server !");
			break;
		}

	}

}

//TODO make reply isend
//TODO Investigate whether buffer can be replaced with BlockId object.
void SIPServer::handle_GET(int mpi_source, int get_tag){
	int count = BlockId::mpi_count;
	BlockId::mpi_block_id_t buffer;
	MPI_Status status;
	//receive the GET message
	check_err(MPI_Recv(buffer, count, MPI_INT, mpi_source, get_tag, MPI_COMM_WORLD, &status));
	check_count(status, count);
	//construct a BlockId object from the message contents and retrieve the block.
	BlockId block_id(buffer);
	ServerBlock* block = block_map_.block(block_id);  //method ensures block exists.
	//send block to worker using same tag as GET
	check_err(MPI_Send(block->get_data()), block->size(), MPI_DOUBLE, mpi_source, get_tag, MPI_COMM_WORLD));
}

//TODO make second rec asynch
void SIPServer::handle_PUT(int mpi_source, int put_tag, int put_data_tag){
	SIP_LOG(std::cout << sip_mpi_attr_.global_rank()<< " : In PUT at server from rank " << mpi_source<< std::endl);
	int count = BlockId::mpi_count;
	BlockId::mpi_block_id_t buffer;
	MPI_Status status;
	//receive the GET message
	check_err(MPI_Recv(buffer, count, MPI_INT, mpi_source, mpi_tag, MPI_COMM_WORLD, &status));
	check_int_count(status, count);
	//construct a BlockId object from the message contents and retrieve the block.
	BlockId block_id(buffer);
	//get the block and its size, constructing it if it doesn't exist
	int block_size = sip_tables_.block_size(block_id);
	ServerBlock* block = block_map_.get_or_create_block(block_id, block_size, false);
	//receive data
	check_err(MPI_Recv(block->data_, block_size, MPI_DOUBLE, mpi_source, put_data_tag, MPI_COMM_WORLD, &status));
	//send ack
	check_err(MPI_Send(0,0,MPI_INT, mpi_source, put_data_tag, MPI_COMM_WORLD));
	SIP_LOG(std::cout << sip_mpi_attr_.global_rank() << " : Done PUT for rank "<< mpi_source << std::endl);
}

void SIPServer::handle_PUT_ACCUMULATE(int mpi_source, int put_accumulate_tag, int put_accumulate_data_tag){
	SIP_LOG(std::cout << sip_mpi_attr_.global_rank()<< " : In PUT_Accumulate at server from rank " << mpi_source<< std::endl);
	int count = BlockId::mpi_count;
	BlockId::mpi_block_id_t buffer;
	MPI_Status status;
	//receive the first PUT message
	check_err(MPI_Recv(buffer, count, MPI_INT, mpi_source, put_accumulate_tagg, MPI_COMM_WORLD, &status));
	check_int_count(status, count);
	//construct a BlockId object from the message contents .
	BlockId block_id(buffer);
	//get the size, create a temporary buffer, and post irecv.
	int block_size = sip_tables_.block_size(block_id);
	ServerBlock::dataPtr temp = new double[block_size];
	MPI_Request request;
	check_err(MPI_Irecv(temp, block_size, MPI_DOUBLE, mpi_source, put_accumulate_data_tag, MPI_COMM_WORLD, &request));

	//get the block from the block_map, constructing and initializing to 0 fi it doesn't exist.
	ServerBlock* block = block_map_.get_or_create_block(block_id, block_size, true);

	//wait for data to arrive
	MPI_Wait(request, status);
	check_double_count(status, block_size);

	//accumulate into block
	block->accumulate_data(block_size, temp);

	//send ack
	check_err(MPI_Send(0,0,MPI_INT, mpi_source, put_accumulate_data_tag, MPI_COMM_WORLD));
	SIP_LOG(std::cout << sip_mpi_attr_.global_rank() << " : Done PUT_accumulate for rank "<< mpi_source << std::endl);

	//free the temp buffer
	delete [] temp;
}



void SIPServer::handle_DELETE(int mpi_source, int delete_tag) {
	SIP_LOG(std::cout << sip_mpi_attr_.global_rank()<< " : In DELETE at server " << std::endl);
	//receive and check the message
	int array_id;
	check_err(MPI_Recv(&array_id, 1, MPI_INT, mpi_source, delete_tag, MPI_COMM_WORLD, &status));
	check_int_count(status, 1);
	//delete the block and map for the indicated array
	block_map_.delete_per_array_map(array_id);
	//send ack
	check_err(MPI_Send(0,0,MPI_INT, mpi_source, delete_tag, MPI_COMM_WORLD));
	SIP_LOG(std::cout << sip_mpi_attr_.global_rank() << " : Done DELETE for rank "<< mpi_source << std::endl);
}


void SIPServer::handle_END_PROGRAM(int mpi_source, int end_program_tag) {
	SIP_LOG(std::cout << sip_mpi_attr_.global_rank()<< " : In END_PROGRAM at server " << std::endl);
	//receive the empty message
	check_err(MPI_Recv(0, 0, MPI_INT, mpi_source, end_program_tag, MPI_COMM_WORLD, &status));

	//set terminated flag;
	terminated = true;

	//send ack
	check_err(MPI_Send(0,0,MPI_INT, mpi_source, end_program_tag, MPI_COMM_WORLD));
	SIP_LOG(std::cout << sip_mpi_attr_.global_rank() << " : Done END_PROGRAM for rank "<< mpi_source << std::endl);
}


void SIPServer::handle_SET_PERSISTENT(int mpi_source, int set_persistent_tag) {
	SIP_LOG(std::cout << sip_mpi_attr_.global_rank()<< " : In SET_PERSISTENT at server " << std::endl);
	//receive and check the message
	int buffer[2];  //array_id, string_slot
	check_err(MPI_Recv(buffer, 2, MPI_INT, mpi_source, set_persistent_tag, MPI_COMM_WORLD, &status));
	check_int_count(status, 2);

	//upcall
	persistent_block_manager_.set_persistent(array_id, string_slot);

	//send ack
	check_err(MPI_Send(0,0,MPI_INT, mpi_source, set_persistent_tag, MPI_COMM_WORLD));
	SIP_LOG(std::cout << sip_mpi_attr_.global_rank() << " : Done SET_PERSISTENT at server " << std::endl);
}



void SIPServer::handle_RESTORE_PERSISTENT(int mpi_source, int restore_persistent_tag) {
	SIP_LOG(std::cout << sip_mpi_attr_.global_rank()<< " : Got RESTORE_PERSISTENT " << std::endl);
	//receive and check the message
	int buffer[2];  //array_id, string_slot
	check_err(MPI_Recv(buffer, 2, MPI_INT, mpi_source, srestore_persistent_tag, MPI_COMM_WORLD, &status));
	check_int_count(status, 2);

	//upcall
	persistent_block_manager_.set_persistent(array_id, string_slot,data_manager_);

	//send ack
	check_err(MPI_Send(0,0,MPI_INT, mpi_source, restore_persistent_tag, MPI_COMM_WORLD));
	SIP_LOG(std::cout << sip_mpi_attr_.global_rank() << " : Done RESTORE_PERSISTENT at server " << std::endl);
}



void SIPServer::check_int_count(const MPI_Status& status, int expected_count){
	int received_count;
	check_err(MPI_Get_count(status, MPI_INT, &received_count));
	check(received_count == expected_count, "message count different than expected");
}

void SIPServer::check_int_count(const MPI_Status& status, int expected_count){
	int received_count;
	check_err(MPI_Get_count(status, MPI_DOUBLE, &received_count));
	check(received_count == expected_count, "message count different than expected");
}

std::ostream& operator<<(std::ostream& os, const SIPServer& obj) {
	os << "block_map_:" << std::endl << obj.block_map_ << std::endl;
    os << "state_: " << obj.state_ << std::endl;
    os << "terminated=" << obj.terminated_ << std::endl;
	return os;
}

} /* namespace sip */
