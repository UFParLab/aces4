/*
 * sip_server.cpp
 *
 */

#include <sip_server.h>
#include "mpi.h"
#include "sip.h"
#include "sip_mpi_utils.h"
#include <sstream>

namespace sip {

SIPServer::SIPServer(SipTables& sip_tables, DataDistribution& data_distribution,
		SIPMPIAttr& sip_mpi_attr,
		ServerPersistentArrayManager* persistent_array_manager) :
		sip_tables_(sip_tables), data_distribution_(data_distribution), disk_backed_block_map_(
				sip_tables, sip_mpi_attr, data_distribution), sip_mpi_attr_(
				sip_mpi_attr), persistent_array_manager_(persistent_array_manager),
				terminated_(false){
}

SIPServer::~SIPServer() {
}

void SIPServer::run() {

	int my_rank = sip_mpi_attr_.global_rank();

	while (!terminated_) {

		MPI_Status status;

		// Check to see if a request has arrived from any worker.
		SIPMPIUtils::check_err(
				MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD,
						&status));

		//extract info about the message
		int mpi_tag = status.MPI_TAG;
		int mpi_source = status.MPI_SOURCE;

		SIPMPIConstants::MessageType_t message_type;
		int section_number;
		int transaction_number;

		bool section_number_changed = state_.decode_tag_and_check_invariant(mpi_tag, message_type,
				section_number, transaction_number);
		SIP_LOG(
				std::cout<< "S " << my_rank << " : has message with tag " << mpi_tag << " of type " << SIPMPIConstants::messageTypeToName(message_type) << " section_number=" << section_number << " transaction_number=" << transaction_number << std::endl << std::flush);

		handle_section_number_change(section_number_changed);

		//handle the message
		switch (message_type) {
		//case SIPMPIData::CREATE:
		//	// NO OP - Create blocks on the fly.
		//	break;
		case SIPMPIConstants::GET:
			handle_GET(mpi_source, mpi_tag);
			break;
		case SIPMPIConstants::PUT:
			int put_data_tag;
			put_data_tag = BarrierSupport::make_mpi_tag(SIPMPIConstants::PUT_DATA,
					section_number, transaction_number);
			handle_PUT(mpi_source, mpi_tag, put_data_tag);
			break;
		case SIPMPIConstants::PUT_ACCUMULATE:
			int put_accumulate_data_tag;
			put_accumulate_data_tag = BarrierSupport::make_mpi_tag(
					SIPMPIConstants::PUT_ACCUMULATE_DATA, section_number,
					transaction_number);
			handle_PUT_ACCUMULATE(mpi_source, mpi_tag, put_accumulate_data_tag);
			break;
		case SIPMPIConstants::DELETE:
			handle_DELETE(mpi_source, mpi_tag);
			break;
		case SIPMPIConstants::SET_PERSISTENT:
			handle_SET_PERSISTENT(mpi_source, mpi_tag);
			break;
		case SIPMPIConstants::RESTORE_PERSISTENT:
			handle_RESTORE_PERSISTENT(mpi_source, mpi_tag);
			break;
		case SIPMPIConstants::END_PROGRAM:
			handle_END_PROGRAM(mpi_source, mpi_tag);
			break;
		default:
			fail("Received unexpected message in SIP Server !");
			break;
		}
	}
	//after loop.  Could cleanup here, but will not receive more messages.
}

void SIPServer::handle_GET(int mpi_source, int get_tag) {
	int block_id_count = BlockId::MPI_COUNT;
	BlockId::mpi_block_id_t buffer;
	MPI_Status status;

	//receive the GET message
	SIPMPIUtils::check_err(
			MPI_Recv(buffer, block_id_count, MPI_INT, mpi_source, get_tag,
					MPI_COMM_WORLD, &status));
	check_int_count(status, block_id_count);

	//construct a BlockId object from the message contents and retrieve the block.
	BlockId block_id(buffer);

	size_t block_size = sip_tables_.block_size(block_id);

	ServerBlock* block = disk_backed_block_map_.get_block_for_reading(block_id);

	SIP_LOG(std::cout << "S " << sip_mpi_attr_.global_rank()
			<< " : get for block " << block_id.str(sip_tables_)
			<< ", size = " << block_size
			<< ", sent from = " << mpi_source
			<< std::endl;)

//	ServerBlock* block = block_map_.block(block_id);
//	if (block == NULL) {
//		std::string msg(" getting uninitialized block ");
//		msg.append(block_id.str());
//		msg.append(".  Creating zero block ");
//		std::cout <<"worker " << mpi_source << msg << std::endl << std::flush; //do this instead of check_and_warn so goes to std::out intead of std::err
//		block = block_map_.get_or_create_block(block_id, block_size, true);
//	}

	//send block to worker using same tag as GET
	SIPMPIUtils::check_err(
			MPI_Send(block->get_data(), block_size, MPI_DOUBLE, mpi_source,
					get_tag, MPI_COMM_WORLD));

	check(block->update_and_check_consistency(SIPMPIConstants::GET, mpi_source),
			"Incorrect block semantics !");

}

void SIPServer::handle_PUT(int mpi_source, int put_tag, int put_data_tag) {
	int block_id_count = BlockId::MPI_COUNT;
	BlockId::mpi_block_id_t buffer;
	MPI_Status status;

	//receive the PUT message
	SIPMPIUtils::check_err(
			MPI_Recv(buffer, block_id_count, MPI_INT, mpi_source, put_tag,
					MPI_COMM_WORLD, &status));
	check_int_count(status, block_id_count);

	//construct a BlockId object from the message contents
	BlockId block_id(buffer);
//	check_array_mode(block_id.array_id(), WRITE);

	//get the block and its size, constructing it if it doesn't exist
	int block_size;
	block_size = sip_tables_.block_size(block_id);
	SIP_LOG(std::cout << "S " << sip_mpi_attr_.global_rank()
					<< " : put to receive block " << block_id.str(sip_tables_)
					<< ", size = " << block_size
					<< ", from = " << mpi_source
					<<std::endl;)
	ServerBlock* block = disk_backed_block_map_.get_block_for_writing(block_id);

	//receive data
	SIPMPIUtils::check_err(
			MPI_Recv(block->get_data(), block_size, MPI_DOUBLE, mpi_source,
					put_data_tag, MPI_COMM_WORLD, &status));
	check_double_count(status, block_size);

	check(block->update_and_check_consistency(SIPMPIConstants::PUT, mpi_source),
			"Incorrect block semantics !");

	//send ack
	SIPMPIUtils::check_err(
			MPI_Send(0, 0, MPI_INT, mpi_source, put_data_tag, MPI_COMM_WORLD));

}

void SIPServer::handle_PUT_ACCUMULATE(int mpi_source, int put_accumulate_tag,
		int put_accumulate_data_tag) {
	int block_id_count = BlockId::MPI_COUNT;
	BlockId::mpi_block_id_t buffer;
	MPI_Status status;

	//receive the PUT_ACCUMULATE message
	SIPMPIUtils::check_err(
			MPI_Recv(buffer, block_id_count, MPI_INT, mpi_source,
					put_accumulate_tag, MPI_COMM_WORLD, &status));
	check_int_count(status, block_id_count);

	//construct a BlockId object from the message contents
	BlockId block_id(buffer);
//	check_array_mode(block_id.array_id(), UPDATE);

	//get the block size
	int block_size;
	block_size = sip_tables_.block_size(block_id);
	SIP_LOG(std::cout << "S " << sip_mpi_attr_.global_rank()
			<< " : put accumulate to receive block " << block_id.str(sip_tables_)
			<< ", size = " << block_size
			<< ", from = " << mpi_source
			<< std::endl << std::flush;)

	//allocate a temporary buffer and post irecv.
	ServerBlock::dataPtr temp = new double[block_size];
	MPI_Request request;
	MPI_Status status2;
	MPI_Irecv(temp, block_size, MPI_DOUBLE, mpi_source, put_accumulate_data_tag,
			MPI_COMM_WORLD, &request);

	//now get the block itself, constructing it if it doesn't exist.  If creating new block, initialize to zero.
	ServerBlock* block = disk_backed_block_map_.get_block_for_updating(block_id);

	//wait for data to arrive
	MPI_Wait(&request, &status2);
	check_double_count(status2, block_size);

	check(block->update_and_check_consistency(SIPMPIConstants::PUT_ACCUMULATE, mpi_source),
			"Incorrect block semantics !");

	//send ack
	SIPMPIUtils::check_err(
			MPI_Send(0, 0, MPI_INT, mpi_source, put_accumulate_data_tag,
					MPI_COMM_WORLD));

	//accumulate into block
	block->accumulate_data(block_size, temp);

	//delete the temporary buffer
	delete[] temp;
}

void SIPServer::handle_DELETE(int mpi_source, int delete_tag) {
	SIP_LOG(
			std::cout << "S " << sip_mpi_attr_.global_rank()<< " : In DELETE at server " << std::endl);

	//receive and check the message
	int array_id;
	MPI_Status status;
	SIPMPIUtils::check_err(
			MPI_Recv(&array_id, 1, MPI_INT, mpi_source, delete_tag,
					MPI_COMM_WORLD, &status));
	check_int_count(status, 1);

	SIP_LOG(std::cout << "S " << sip_mpi_attr_.global_rank()
			<< " : deleting array " << sip_tables_.array_name(array_id) << ", id = " << array_id
			<< ", sent from = " << mpi_source << std::endl;)

	//send ack
	SIPMPIUtils::check_err(
			MPI_Send(0, 0, MPI_INT, mpi_source, delete_tag, MPI_COMM_WORLD));

	//delete the block and map for the indicated array
	disk_backed_block_map_.delete_per_array_map_and_blocks(array_id);

}

void SIPServer::handle_END_PROGRAM(int mpi_source, int end_program_tag) {
	SIP_LOG(
			std::cout << "S " << sip_mpi_attr_.global_rank()<< " : In END_PROGRAM at server " << std::endl);

	//receive the message (which is empty)
	MPI_Status status;
	SIPMPIUtils::check_err(
			MPI_Recv(0, 0, MPI_INT, mpi_source, end_program_tag, MPI_COMM_WORLD,
					&status));

	SIP_LOG(std::cout << "S " << sip_mpi_attr_.global_rank()
				<< " : ending program "
				<< ", sent from = " << mpi_source << std::endl;)

	//send ack
	SIPMPIUtils::check_err(
			MPI_Send(0, 0, MPI_INT, mpi_source, end_program_tag,
					MPI_COMM_WORLD));

	//set terminated flag;
	terminated_ = true;
}

void SIPServer::handle_SET_PERSISTENT(int mpi_source, int set_persistent_tag) {
	SIP_LOG(
			std::cout <<"S " << sip_mpi_attr_.global_rank()<< " : In SET_PERSISTENT at server " << std::endl);

	//receive and check the message
	MPI_Status status;
	int buffer[2];  //array_id, string_slot
	SIPMPIUtils::check_err(
			MPI_Recv(buffer, 2, MPI_INT, mpi_source, set_persistent_tag,
					MPI_COMM_WORLD, &status));
	check_int_count(status, 2);


	//send ack
	SIPMPIUtils::check_err(
			MPI_Send(0, 0, MPI_INT, mpi_source, set_persistent_tag,
					MPI_COMM_WORLD));

	//upcall to persistent_array_manager
	int array_id = buffer[0];
	int string_slot = buffer[1];
//	std::cout << "calling spam set_persistent with array " << array_id << " and string " << string_slot << std::endl;
	persistent_array_manager_->set_persistent(this, array_id, string_slot);

	SIP_LOG(std::cout << "S " << sip_mpi_attr_.global_rank()
				<< " : set persistent array " << sip_tables_.array_name(array_id) << ", id = " << array_id
				<< ", sent from = " << mpi_source << std::endl;);

}

void SIPServer::handle_RESTORE_PERSISTENT(int mpi_source,
		int restore_persistent_tag) {
	SIP_LOG(
			std::cout << "S " << sip_mpi_attr_.global_rank()<< " : Got RESTORE_PERSISTENT " << std::endl);
	//receive and check the message
	int buffer[2];  //array_id, string_slot
	MPI_Status status;
	SIPMPIUtils::check_err(
			MPI_Recv(buffer, 2, MPI_INT, mpi_source, restore_persistent_tag,
					MPI_COMM_WORLD, &status));
	check_int_count(status, 2);

	//send ack
	SIPMPIUtils::check_err(
			MPI_Send(0, 0, MPI_INT, mpi_source, restore_persistent_tag,
					MPI_COMM_WORLD));

	//upcall
	int array_id = buffer[0];
	int string_slot = buffer[1];
	persistent_array_manager_->restore_persistent(this, array_id, string_slot);

	SIP_LOG(std::cout << "S " << sip_mpi_attr_.global_rank()
				<< " : restored persistent array " << sip_tables_.array_name(array_id) << ", id = " << array_id
				<< ", sent from = " << mpi_source << std::endl;)

}

void SIPServer::check_int_count(MPI_Status& status, int expected_count) {
	int received_count;
	SIPMPIUtils::check_err(MPI_Get_count(&status, MPI_INT, &received_count));
	check(received_count == expected_count,
			"message int count different than expected");
}

void SIPServer::check_double_count(MPI_Status& status, int expected_count) {
	int received_count;
	SIPMPIUtils::check_err(MPI_Get_count(&status, MPI_DOUBLE, &received_count));
	check(received_count == expected_count,
			"message double count different than expected");
}


void SIPServer::handle_section_number_change(bool section_number_changed) {
	// If section number is changed, reset the "mode" of all blocks to "OPEN, NONE".
	if (section_number_changed) {
		disk_backed_block_map_.reset_consistency_status_for_all_blocks();
	}
}


std::ostream& operator<<(std::ostream& os, const SIPServer& obj) {
	os << "\nblock_map_:" << std::endl << obj.disk_backed_block_map_;
	os << "state_: " << obj.state_ << std::endl;
	os << "terminated=" << obj.terminated_ << std::endl;
	return os;
}

} /* namespace sip */
