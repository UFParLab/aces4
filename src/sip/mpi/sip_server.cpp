/*
 * sip_server.cpp
 *
 */

#include <sip_server.h>
#include "mpi.h"
#include "sip.h"
#include "sip_mpi_utils.h"
#include <sstream>
#include "sial_ops_parallel.h"

namespace sip {

SIPServer* SIPServer::global_sipserver = NULL;

SIPServer::SIPServer(SipTables& sip_tables, DataDistribution& data_distribution,
		SIPMPIAttr& sip_mpi_attr,
		ServerPersistentArrayManager* persistent_array_manager,
		ServerTimer& server_timer) :
		sip_tables_(sip_tables), data_distribution_(data_distribution), disk_backed_block_map_(
				sip_tables, sip_mpi_attr, data_distribution, server_timer), sip_mpi_attr_(
				sip_mpi_attr), persistent_array_manager_(persistent_array_manager),
				terminated_(false), server_timer_(server_timer),
				last_seen_line_(0), last_seen_worker_(0){
	initialize_mpi_type();
	SIPServer::global_sipserver = this;
}

SIPServer::~SIPServer() {
	SIPServer::global_sipserver = NULL;
}

void SIPServer::run() {

	int my_rank = sip_mpi_attr_.global_rank();

	while (!terminated_) {

		MPI_Status status;

		// Check to see if a request has arrived from any worker.
		SIPMPIUtils::check_err(
				MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD,
						&status), __LINE__, __FILE__);

		//extract info about the message
		int mpi_tag = status.MPI_TAG;
		int mpi_source = status.MPI_SOURCE;

		SIPMPIConstants::MessageType_t message_type;
		int section_number;
		int transaction_number;

		state_.decode_tag(mpi_tag, message_type, transaction_number);

		SIP_LOG(
				std::cout<< "S " << my_rank << " : has message with tag " << mpi_tag << " of type " << SIPMPIConstants::messageTypeToName(message_type) << " transaction_number=" << transaction_number << std::endl << std::flush);

		// TODO bool section_number_changed = handle_section_number_change(section_number_changed);

		//handle the message
		switch (message_type) {
		//case SIPMPIData::CREATE:
		//	// NO OP - Create blocks on the fly.
		//	break;
		case SIPMPIConstants::GET:{
			handle_GET(mpi_source, mpi_tag);
		}
			break;
		case SIPMPIConstants::PUT:{
			int put_data_tag;
			put_data_tag = BarrierSupport::make_mpi_tag(SIPMPIConstants::PUT_DATA, transaction_number);
			handle_PUT(mpi_source, mpi_tag, put_data_tag);
		}
			break;
		case SIPMPIConstants::PUT_ACCUMULATE:{
			int put_accumulate_data_tag;
			put_accumulate_data_tag = BarrierSupport::make_mpi_tag(
					SIPMPIConstants::PUT_ACCUMULATE_DATA, transaction_number);
			handle_PUT_ACCUMULATE(mpi_source, mpi_tag, put_accumulate_data_tag);
		}
			break;
		case SIPMPIConstants::PUT_INITIALIZE:{
			handle_PUT_INITIALIZE(mpi_source, mpi_tag);
		}
			break;
		case SIPMPIConstants::PUT_INCREMENT:{
			handle_PUT_INCREMENT(mpi_source, mpi_tag);
		}
			break;
		case SIPMPIConstants::PUT_SCALE:{
			handle_PUT_INCREMENT(mpi_source, mpi_tag);
		}
			break;
		case SIPMPIConstants::DELETE:{
			handle_DELETE(mpi_source, mpi_tag);
		}
			break;
		case SIPMPIConstants::SET_PERSISTENT:{
			handle_SET_PERSISTENT(mpi_source, mpi_tag);
		}
			break;
		case SIPMPIConstants::RESTORE_PERSISTENT:{
			handle_RESTORE_PERSISTENT(mpi_source, mpi_tag);
		}
			break;
		case SIPMPIConstants::END_PROGRAM:{
			handle_END_PROGRAM(mpi_source, mpi_tag);
		}
			break;
		default:
			fail("Received unexpected message in SIP Server !");
			break;
		}
	}
	//after loop.  Could cleanup here, but will not receive more messages.
}

void SIPServer::handle_GET(int mpi_source, int get_tag) {
	const int recv_data_count = BlockId::MPI_BLOCK_ID_COUNT + 2;
	const int line_num_offset = BlockId::MPI_BLOCK_ID_COUNT;
	const int section_num_offset = line_num_offset + 1;
	int recv_buffer[recv_data_count];
	MPI_Status status;

	//receive the GET message (Block ID & Line Number)
	SIPMPIUtils::check_err(MPI_Recv(recv_buffer, recv_data_count, MPI_INT, mpi_source, get_tag,
					MPI_COMM_WORLD, &status));
	check_int_count(status, recv_data_count);

	last_seen_line_ = recv_buffer[line_num_offset];
	last_seen_worker_ = mpi_source;

	int section = recv_buffer[section_num_offset];
	bool section_number_changed = state_.check_section_number_invariant(section);
	handle_section_number_change(section_number_changed);

	server_timer_.start_timer(last_seen_line_, ServerTimer::TOTALTIME);

	//construct a BlockId object from the message contents and retrieve the block.
	BlockId::mpi_block_id_t buffer;
	std::copy(recv_buffer, recv_buffer + BlockId::MPI_BLOCK_ID_COUNT, buffer);
	BlockId block_id(buffer);
	//DEBUG
	if(block_id.array_id_==137){std::cout << "get block " << block_id << " line "<< last_seen_line_ << std::endl << std::flush;}

	size_t block_size = sip_tables_.block_size(block_id);

	server_timer_.start_timer(last_seen_line_, ServerTimer::BLOCKWAITTIME);
	ServerBlock* block = disk_backed_block_map_.get_block_for_reading(block_id, last_seen_line_);
	server_timer_.pause_timer(last_seen_line_, ServerTimer::BLOCKWAITTIME);

	SIP_LOG(std::cout << "S " << sip_mpi_attr_.global_rank()
			<< " : get for block " << block_id.str(sip_tables_)
			<< ", size = " << block_size
			<< ", sent from = " << mpi_source
			<< ", at line = " << last_seen_line_
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
					get_tag, MPI_COMM_WORLD), __LINE__, __FILE__);

	if (!block->update_and_check_consistency(SIPMPIConstants::GET, mpi_source)){
		std::stringstream err_ss;
		err_ss << "Incorrect GET block semantics for " << block_id ;
		fail(err_ss.str());
	}

	server_timer_.pause_timer(last_seen_line_, ServerTimer::TOTALTIME);

}

void SIPServer::handle_PUT(int mpi_source, int put_tag, int put_data_tag) {
	const int recv_data_count = BlockId::MPI_BLOCK_ID_COUNT + 2;
	const int line_num_offset = BlockId::MPI_BLOCK_ID_COUNT;
	const int section_num_offset = line_num_offset + 1;
	int recv_buffer[recv_data_count];
	MPI_Status status;

	//receive the PUT message
	SIPMPIUtils::check_err(
			MPI_Recv(recv_buffer, recv_data_count, MPI_INT, mpi_source, put_tag,
					MPI_COMM_WORLD, &status));
	check_int_count(status, recv_data_count);

	last_seen_line_ = recv_buffer[line_num_offset];
	last_seen_worker_ = mpi_source;

	int section = recv_buffer[section_num_offset];
	bool section_number_changed = state_.check_section_number_invariant(section);
	handle_section_number_change(section_number_changed);

	server_timer_.start_timer(last_seen_line_, ServerTimer::TOTALTIME);

	//construct a BlockId object from the message contents and retrieve the block.
	BlockId::mpi_block_id_t buffer;
	std::copy(recv_buffer, recv_buffer + BlockId::MPI_BLOCK_ID_COUNT, buffer);
	BlockId block_id(buffer);
	//DEBUG
	if(block_id.array_id_==137){std::cout << "put block " << block_id << " line "<< last_seen_line_ << std::endl << std::flush;}

	//get the block and its size, constructing it if it doesn't exist
	int block_size;
	block_size = sip_tables_.block_size(block_id);
	SIP_LOG(std::cout << "S " << sip_mpi_attr_.global_rank()
					<< " : put to receive block " << block_id.str(sip_tables_)
					<< ", size = " << block_size
					<< ", from = " << mpi_source
					<< ", at line = " << last_seen_line_
					<<std::endl;)

	server_timer_.start_timer(last_seen_line_, ServerTimer::BLOCKWAITTIME);
	ServerBlock* block = disk_backed_block_map_.get_block_for_writing(block_id);
	server_timer_.pause_timer(last_seen_line_, ServerTimer::BLOCKWAITTIME);


	//receive data
	SIPMPIUtils::check_err(
			MPI_Recv(block->get_data(), block_size, MPI_DOUBLE, mpi_source,
					put_data_tag, MPI_COMM_WORLD, &status), __LINE__, __FILE__);
	check_double_count(status, block_size);

	if (!block->update_and_check_consistency(SIPMPIConstants::PUT, mpi_source)){
		std::stringstream err_ss;
		err_ss << "Incorrect PUT block semantics (data race) for " << block_id << " at line " << last_seen_line_ << " from worker " << mpi_source << ". Probably a missing sip_barrier";
		fail(err_ss.str());
	}

	//send ack
	SIPMPIUtils::check_err(
			MPI_Send(0, 0, MPI_INT, mpi_source, put_data_tag, MPI_COMM_WORLD), __LINE__, __FILE__);

	server_timer_.pause_timer(last_seen_line_, ServerTimer::TOTALTIME);

}

void SIPServer::handle_PUT_ACCUMULATE(int mpi_source, int put_accumulate_tag,
		int put_accumulate_data_tag) {
	const int recv_data_count = BlockId::MPI_BLOCK_ID_COUNT + 2;
	const int line_num_offset = BlockId::MPI_BLOCK_ID_COUNT;
	const int section_num_offset = line_num_offset + 1;
	int recv_buffer[recv_data_count];
	MPI_Status status;

	//receive the PUT_ACCUMULATE message
	SIPMPIUtils::check_err(
			MPI_Recv(recv_buffer, recv_data_count, MPI_INT, mpi_source,
					put_accumulate_tag, MPI_COMM_WORLD, &status));
	check_int_count(status, recv_data_count);

	last_seen_line_ = recv_buffer[line_num_offset];
	last_seen_worker_ = mpi_source;

	int section = recv_buffer[section_num_offset];
	bool section_number_changed = state_.check_section_number_invariant(section);
	handle_section_number_change(section_number_changed);

	server_timer_.start_timer(last_seen_line_, ServerTimer::TOTALTIME);


	//construct a BlockId object from the message contents and retrieve the block.
	BlockId::mpi_block_id_t buffer;
	std::copy(recv_buffer, recv_buffer + BlockId::MPI_BLOCK_ID_COUNT, buffer);
	BlockId block_id(buffer);
	//DEGBUG
	if(block_id.array_id_==137){std::cout << "put_acc block " << block_id << " line "<< last_seen_line_ << std::endl << std::flush;}
	//get the block size
	int block_size;
	block_size = sip_tables_.block_size(block_id);
	SIP_LOG(std::cout << "S " << sip_mpi_attr_.global_rank()
			<< " : put accumulate to receive block " << block_id.str(sip_tables_)
			<< ", size = " << block_size
			<< ", from = " << mpi_source
			<< ", at line = " << last_seen_line_
			<< std::endl << std::flush;)

	//allocate a temporary buffer and post irecv.
	ServerBlock::dataPtr temp = new double[block_size];
	MPI_Request request;
	MPI_Status status2;
	MPI_Irecv(temp, block_size, MPI_DOUBLE, mpi_source, put_accumulate_data_tag,
			MPI_COMM_WORLD, &request);

	//now get the block itself, constructing it if it doesn't exist.  If creating new block, initialize to zero.
	server_timer_.start_timer(last_seen_line_, ServerTimer::BLOCKWAITTIME);
	ServerBlock* block = disk_backed_block_map_.get_block_for_accumulate(block_id);
	server_timer_.pause_timer(last_seen_line_, ServerTimer::BLOCKWAITTIME);


	//wait for data to arrive
	MPI_Wait(&request, &status2);
	check_double_count(status2, block_size);

	if (!block->update_and_check_consistency(SIPMPIConstants::PUT_ACCUMULATE, mpi_source)){
		std::stringstream err_ss;
		err_ss << "Incorrect PUT_ACCUMULATE block semantics for " << block_id << " at line " << last_seen_line_ << " from worker " << mpi_source;;
		fail(err_ss.str());
	}

	//send ack
	SIPMPIUtils::check_err(
			MPI_Send(0, 0, MPI_INT, mpi_source, put_accumulate_data_tag,
					MPI_COMM_WORLD), __LINE__, __FILE__);

	//accumulate into block
	block->accumulate_data(block_size, temp);

	//delete the temporary buffer
	delete[] temp;

	server_timer_.pause_timer(last_seen_line_, ServerTimer::TOTALTIME);

}

void SIPServer::handle_DELETE(int mpi_source, int delete_tag) {
	SIP_LOG(
			std::cout << "S " << sip_mpi_attr_.global_rank()<< " : In DELETE at server " << std::endl);

	//receive and check the message
	int recv_buffer[3];
	MPI_Status status;
	SIPMPIUtils::check_err(
			MPI_Recv(recv_buffer, 3, MPI_INT, mpi_source, delete_tag,
					MPI_COMM_WORLD, &status));
	check_int_count(status, 3);
	int array_id	= recv_buffer[0];

	last_seen_line_ = recv_buffer[1];
	last_seen_worker_ = mpi_source;
	int section = recv_buffer[2];

	bool section_number_changed = state_.check_section_number_invariant(section);
	handle_section_number_change(section_number_changed);

	server_timer_.start_timer(last_seen_line_, ServerTimer::TOTALTIME);

	SIP_LOG(std::cout << "S " << sip_mpi_attr_.global_rank()
			<< " : deleting array " << sip_tables_.array_name(array_id) << ", id = " << array_id
			<< ", sent from = " << mpi_source
			<< ", at line = " << last_seen_line_ << std::endl;)

	//send ack
	SIPMPIUtils::check_err(
			MPI_Send(0, 0, MPI_INT, mpi_source, delete_tag, MPI_COMM_WORLD), __LINE__, __FILE__);

	//delete the block and map for the indicated array
	disk_backed_block_map_.delete_per_array_map_and_blocks(array_id);
	if(array_id==137){std::cout << "delete " << array_id << " line "<< last_seen_line_ << std::endl << std::flush;}

	server_timer_.pause_timer(last_seen_line_, ServerTimer::TOTALTIME);


}

void SIPServer::handle_PUT_INITIALIZE(int mpi_source, int put_initialize_tag){
	SialOpsParallel::Put_scalar_op_message_t message;
	MPI_Status status;
	MPI_Recv(&message, 1, mpi_put_scalar_op_type_, mpi_source, put_initialize_tag,
			MPI_COMM_WORLD, &status);




	std::cout << "put_initialize message.id_=" << message.id_ << " message.line="<< message.line_ << " message.section="<< message.section_ << " message.value_=" << message.value_ << std::endl << std::flush;
	ServerBlock* block =
			disk_backed_block_map_.get_block_for_writing(message.id_);
	last_seen_line_ = message.line_;
	last_seen_worker_ = mpi_source;
	int section = message.section_;

	bool section_number_changed = state_.check_section_number_invariant(section);
	handle_section_number_change(section_number_changed);

	//send ack
	SIPMPIUtils::check_err(
			MPI_Send(0, 0, MPI_INT, mpi_source, put_initialize_tag, MPI_COMM_WORLD), __LINE__, __FILE__);
	//get the block size
	int block_size;
	block_size = sip_tables_.block_size(message.id_);

	block->fill_data(block_size, message.value_);

	std::cout << *block << std::endl << std::flush;

}
void SIPServer::handle_PUT_INCREMENT(int mpi_source, int put_increment_tag){
	SialOpsParallel::Put_scalar_op_message_t message;
	MPI_Status status;
	MPI_Recv(&message, 1, mpi_put_scalar_op_type_, mpi_source, put_increment_tag,
			MPI_COMM_WORLD, &status);

	//send ack
	SIPMPIUtils::check_err(
			MPI_Send(0, 0, MPI_INT, mpi_source, put_increment_tag, MPI_COMM_WORLD), __LINE__, __FILE__);

	std::cout << "put_increment message.id_=" << message.id_ << " message.line="<< message.line_ << " message.section="<< message.section_ << " message.value_=" << message.value_ << std::endl << std::flush;
	ServerBlock* block =
			disk_backed_block_map_.get_block_for_accumulate(message.id_);
	last_seen_line_ = message.line_;
	last_seen_worker_ = mpi_source;

	bool section_number_changed = state_.check_section_number_invariant(message.section_);
	handle_section_number_change(section_number_changed);

	//get the block size
	int block_size;
	block_size = sip_tables_.block_size(message.id_);

	block->increment_data(block_size, message.value_);

}

void SIPServer::handle_PUT_SCALE(int mpi_source, int put_scale_tag){
	SialOpsParallel::Put_scalar_op_message_t message;
	MPI_Status status;
	MPI_Recv(&message, 1, mpi_put_scalar_op_type_, mpi_source, put_scale_tag,
			MPI_COMM_WORLD, &status);

	//send ack
	SIPMPIUtils::check_err(
			MPI_Send(0, 0, MPI_INT, mpi_source, put_scale_tag, MPI_COMM_WORLD), __LINE__, __FILE__);

	std::cout << "put_scale message.id_=" << message.id_ << " message.line="<< message.line_ << " message.section="<< message.section_ << " message.value_=" << message.value_ << std::endl << std::flush;
	ServerBlock* block =
			disk_backed_block_map_.get_block_for_accumulate(message.id_);
	last_seen_line_ = message.line_;
	last_seen_worker_ = mpi_source;

	bool section_number_changed = state_.check_section_number_invariant(message.section_);
	handle_section_number_change(section_number_changed);

	//get the block size
	int block_size;
	block_size = sip_tables_.block_size(message.id_);

	block->scale_data(block_size, message.value_);
}

void SIPServer::handle_END_PROGRAM(int mpi_source, int end_program_tag) {
	SIP_LOG(
			std::cout << "S " << sip_mpi_attr_.global_rank()<< " : In END_PROGRAM at server " << std::endl);

	//receive the message (which is empty)
	MPI_Status status;
	SIPMPIUtils::check_err(
			MPI_Recv(0, 0, MPI_INT, mpi_source, end_program_tag, MPI_COMM_WORLD,
					&status), __LINE__, __FILE__);

	last_seen_line_ = 0; 	// Special line number for end program
	last_seen_worker_ = mpi_source;

	server_timer_.start_timer(last_seen_line_, ServerTimer::TOTALTIME);

	SIP_LOG(std::cout << "S " << sip_mpi_attr_.global_rank()
				<< " : ending program "
				<< ", sent from = " << mpi_source << std::endl;)

	//send ack
	SIPMPIUtils::check_err(
			MPI_Send(0, 0, MPI_INT, mpi_source, end_program_tag,
					MPI_COMM_WORLD), __LINE__, __FILE__);

	//set terminated flag;
	terminated_ = true;

	server_timer_.pause_timer(last_seen_line_, ServerTimer::TOTALTIME);

}

void SIPServer::handle_SET_PERSISTENT(int mpi_source, int set_persistent_tag) {
	SIP_LOG(
			std::cout <<"S " << sip_mpi_attr_.global_rank()<< " : In SET_PERSISTENT at server " << std::endl);

	//receive and check the message
	MPI_Status status;
	int buffer[4];  //array_id, string_slot
	SIPMPIUtils::check_err(
			MPI_Recv(buffer, 4, MPI_INT, mpi_source, set_persistent_tag,
					MPI_COMM_WORLD, &status));
	check_int_count(status, 4);

	int array_id = buffer[0];
	int string_slot = buffer[1];

	last_seen_line_ = buffer[2];
	last_seen_worker_ = mpi_source;

	int section = buffer[3];

	bool section_number_changed = state_.check_section_number_invariant(section);
	handle_section_number_change(section_number_changed);


	server_timer_.start_timer(last_seen_line_, ServerTimer::TOTALTIME);

	//send ack
	SIPMPIUtils::check_err(
			MPI_Send(0, 0, MPI_INT, mpi_source, set_persistent_tag,
					MPI_COMM_WORLD), __LINE__, __FILE__);


//	std::cout << "calling spam set_persistent with array " << array_id << " and string " << string_slot << std::endl;
	//upcall to persistent_array_manager
	persistent_array_manager_->set_persistent(this, array_id, string_slot);

	SIP_LOG(std::cout << "S " << sip_mpi_attr_.global_rank()
				<< " : set persistent array " << sip_tables_.array_name(array_id) << ", id = " << array_id
				<< ", sent from = " << mpi_source
				<< ", at line = " << last_seen_line_ << std::endl;);

	server_timer_.pause_timer(last_seen_line_, ServerTimer::TOTALTIME);

}

void SIPServer::handle_RESTORE_PERSISTENT(int mpi_source,
		int restore_persistent_tag) {
	SIP_LOG(
			std::cout << "S " << sip_mpi_attr_.global_rank()<< " : Got RESTORE_PERSISTENT " << std::endl);
	//receive and check the message
	int buffer[4];  //array_id, string_slot
	MPI_Status status;
	SIPMPIUtils::check_err(
			MPI_Recv(buffer, 4, MPI_INT, mpi_source, restore_persistent_tag,
					MPI_COMM_WORLD, &status));
	check_int_count(status, 4);


	int array_id = buffer[0];
	int string_slot = buffer[1];
	last_seen_line_ = buffer[2];
	last_seen_worker_ = mpi_source;

	int section = buffer[3];
	bool section_number_changed = state_.check_section_number_invariant(section);
	handle_section_number_change(section_number_changed);

	server_timer_.start_timer(last_seen_line_, ServerTimer::TOTALTIME);

	//send ack
	SIPMPIUtils::check_err(
			MPI_Send(0, 0, MPI_INT, mpi_source, restore_persistent_tag,
					MPI_COMM_WORLD), __LINE__, __FILE__);

	//upcall
	persistent_array_manager_->restore_persistent(this, array_id, string_slot);

	SIP_LOG(std::cout << "S " << sip_mpi_attr_.global_rank()
				<< " : restored persistent array " << sip_tables_.array_name(array_id) << ", id = " << array_id
				<< ", sent from = " << mpi_source
				<< ", at line = " << last_seen_line_ << std::endl;)

	server_timer_.pause_timer(last_seen_line_, ServerTimer::TOTALTIME);

}

void SIPServer::check_int_count(MPI_Status& status, int expected_count) {
	int received_count;
	SIPMPIUtils::check_err(MPI_Get_count(&status, MPI_INT, &received_count), __LINE__, __FILE__);
	check(received_count == expected_count,
			"message int count different than expected");
}

void SIPServer::check_double_count(MPI_Status& status, int expected_count) {
	int received_count;
	SIPMPIUtils::check_err(MPI_Get_count(&status, MPI_DOUBLE, &received_count), __LINE__, __FILE__);
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

int SIPServer::last_seen_line(){
	return last_seen_line_;
}


//TODO refactor to avoid code duplication in sip_server and sial_ops_parallel
//TODO use MPI features to define the datatype in a more portable way
void SIPServer::initialize_mpi_type(){

	/*
	    //TODO change line to pc
		struct Put_scalar_op_message_t{
		    double value_;
		    int line_;
		    int section_;
			BlockId id_;
		};

		MPI_Datatype mpi_put_scalar_op_type;
		MPI_Datatype block_id_type;
		void initialize_mpi_type();
	 *
	 */


	    MPI_Aint  displacements[4], s_lower, s_extent;
	    MPI_Datatype  types[4];

	    Put_scalar_op_message_t tmp_struct;

	    /* create type for BlockId, skip the parent_id_ptr */
	     BlockId id;
	     MPI_Aint id_displacements[3], id_lb, id_extent;
	     MPI_Datatype id_types[3];
	     MPI_Get_address(&id.array_id_, &id_displacements[0]);
	     MPI_Get_address(&id.index_values_, &id_displacements[1]);
	     MPI_Get_address(&id.parent_id_ptr_, &id_displacements[2]);
	     id_types[0] = MPI_INT;
	     id_types[1] = MPI_INT;
	     id_types[2] = MPI_BYTE;
	     int id_counts[3]={1,MAX_RANK, sizeof(BlockId*)};
	     //convert addresses to displacement from beginning
	     id_displacements[2] -= id_displacements[0];
	     id_displacements[1] -= id_displacements[0];
	     id_displacements[0] = 0;
	     MPI_Type_create_struct(3,id_counts, id_displacements, id_types, &block_id_type_);
	     /*check that it has the correct extent*/
	     MPI_Type_get_extent(block_id_type_, &id_lb, &id_extent);
	     if(id_extent != sizeof(BlockId)){

	     std::cout << "id_extent, sizeof = " << id_extent << "," << sizeof(BlockId) << std::endl << std::flush;
	    	 MPI_Datatype id_type_old = block_id_type_;
	    	 MPI_Type_create_resized(id_type_old, 0, sizeof(BlockId), &block_id_type_);
	    	 MPI_Type_free(&id_type_old);
	     }
	     MPI_Type_commit(&block_id_type_);

	    /* Now get type  for the initialize whole struct */
	    MPI_Get_address(&tmp_struct.value_, &displacements[0]);
	    MPI_Get_address(&tmp_struct.line_, &displacements[1]);
	    MPI_Get_address(&tmp_struct.section_, &displacements[2]);
	    MPI_Get_address(&tmp_struct.id_, &displacements[3]);
	    types[0] = MPI_DOUBLE;
	    types[1] = MPI_INT;
	    types[2] = MPI_INT;
	    types[3] = block_id_type_;
	    int counts[4] = {1,1,1,1};
	    //convert addresses to displacement from beginning
	    displacements[3] -= displacements[0];
	    displacements[2] -= displacements[0];
	    displacements[1] -= displacements[0];
	    displacements[0] = 0;
	    MPI_Type_create_struct(4,counts, displacements, types, &mpi_put_scalar_op_type_);
	    MPI_Type_get_extent(mpi_put_scalar_op_type_, &s_lower, &s_extent);
	    if (s_extent != sizeof(Put_scalar_op_message_t)){
	        std::cout << "s_extent, sizeof = " << id_extent << "," << sizeof(Put_scalar_op_message_t) << std::endl << std::flush;
	       	 MPI_Datatype type_old = mpi_put_scalar_op_type_;
	       	 MPI_Type_create_resized(type_old, 0, sizeof(Put_scalar_op_message_t), &mpi_put_scalar_op_type_);
	       	 MPI_Type_free(&type_old);
	    }
	    MPI_Type_commit(&mpi_put_scalar_op_type_);

//    MPI_Aint offsets[3];
//    MPI_Aint int_size = sizeof(int);
//
//    offsets[0] = 0;  //block_id
//     offsets[1] = int_size * BlockId::MPI_BLOCK_ID_COUNT + /*padding*/ int_size;  //line, section
//     offsets[2] = offsets[1] + 2* int_size + /*padding*/ 2*int_size; //value
//
//
//
//
//
//	int counts[] = {BlockId::MPI_BLOCK_ID_COUNT, 2, 1};
//	MPI_Datatype types[] = {MPI_INT, MPI_INT, MPI_DOUBLE};
//
//	SIPMPIUtils::check_err(MPI_Type_create_struct(3, counts, offsets, types, &mpi_put_scalar_op_type));
//	SIPMPIUtils::check_err(MPI_Type_commit(&mpi_put_scalar_op_type));
};

} /* namespace sip */
