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
				sip_mpi_attr), persistent_array_manager_(
				persistent_array_manager), terminated_(false), server_timer_(
				server_timer), last_seen_worker_(0) {
	mpi_type_.initialize_mpi_scalar_op_type();
	SIPServer::global_sipserver = this;
}

SIPServer::~SIPServer() {
	SIPServer::global_sipserver = NULL;
}

void SIPServer::run() {

	int my_rank = sip_mpi_attr_.global_rank();

//	{//for gdb
//	    int i = 0;
//	    char hostname[256];
//	    gethostname(hostname, sizeof(hostname));
//	    printf("PID %d on %s ready for attach\n", getpid(), hostname);
//	    fflush(stdout);
//	    while (0 == i)
//	        sleep(5);
//	}

	while (!terminated_) {

		MPI_Status status;

			int flag = 0;
			while (flag==0 & async_ops_.may_have_pending()){
				//check for a new short message without blocking
				SIPMPIUtils::check_err(
				MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD,
				&flag, &status), __LINE__, __FILE__);
				//if not found, try to handle a pending async message
			    if (!flag){
			    	async_ops_.try_pending();
			    }
			    //TODO this is a spin loop, do we want to sleep between checks?
			}
			//if here, a short message has arrived (flag!=0) or no more pending messages
			if (!flag){//no short message, thus no pending msgs, so block
				SIPMPIUtils::check_err(
						MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD,
								&status), __LINE__, __FILE__);
			}

		//handle the short message
		int mpi_tag = status.MPI_TAG;
		int mpi_source = status.MPI_SOURCE;

		SIPMPIConstants::MessageType_t message_type;
		int section_number;
		int transaction_number;

		try {
			state_.decode_tag(mpi_tag, message_type, transaction_number);
		} catch (const std::exception& e) {
			std::cerr << "exception thrown in server: " << e.what()
					<< std::endl;
			std::cerr << status << std::endl << std::flush;
			check(false, "exception thrown decoding tag");
		}

		SIP_LOG(std::cout << "S " << my_rank << " : has message from " << mpi_source
				<< " with tag " << mpi_tag << " of type "
				<< SIPMPIConstants::messageTypeToName(message_type)
				<< " transaction_number=" << transaction_number << std::endl
				<< std::flush;);

		// TODO bool section_number_changed = handle_section_number_change(section_number_changed);

		//this switch statement only handles message that arrive without a posted recieve.
		//Message with a posted receive, such as put_accumulate_data and put_data
		//are handled by creating an async op.
		switch (message_type) {
		//case SIPMPIData::CREATE:
		//	// NO OP - Create blocks on the fly.
		//	break;
		case SIPMPIConstants::GET: {
			handle_GET(mpi_source, mpi_tag);
		}
			break;
		case SIPMPIConstants::PUT: {
			handle_PUT(mpi_source, mpi_tag, transaction_number);
		}
			break;
		case SIPMPIConstants::PUT_ACCUMULATE: {

			handle_PUT_ACCUMULATE(mpi_source, mpi_tag, transaction_number);
		}
			break;
		case SIPMPIConstants::PUT_INITIALIZE: {
			handle_PUT_INITIALIZE(mpi_source, mpi_tag);
		}
			break;
		case SIPMPIConstants::PUT_INCREMENT: {
			handle_PUT_INCREMENT(mpi_source, mpi_tag);
		}
			break;
		case SIPMPIConstants::PUT_SCALE: {
			handle_PUT_SCALE(mpi_source, mpi_tag);
		}
			break;
		case SIPMPIConstants::DELETE: {
			handle_DELETE(mpi_source, mpi_tag);
		}
			break;
		case SIPMPIConstants::SET_PERSISTENT: {
			handle_SET_PERSISTENT(mpi_source, mpi_tag);
		}
			break;
		case SIPMPIConstants::RESTORE_PERSISTENT: {
			handle_RESTORE_PERSISTENT(mpi_source, mpi_tag);
		}
			break;
		case SIPMPIConstants::END_PROGRAM: {
			check(!async_ops_.may_have_pending(), "end_program with possibly pending req");
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

	//receive and decode the message
	int buffer[SIPMPIUtils::BLOCKID_BUFF_ELEMS];
	MPI_Status status;
	SIPMPIUtils::check_err(
			MPI_Recv(buffer, SIPMPIUtils::BLOCKID_BUFF_ELEMS, MPI_INT, mpi_source, get_tag,
					MPI_COMM_WORLD, &status));
	check_int_count(status, SIPMPIUtils::BLOCKID_BUFF_ELEMS);
	BlockId block_id;
	int section;
	int pc;
	SIPMPIUtils::decode_BlockID_buff(buffer, block_id, pc, section);
	last_seen_worker_ = mpi_source;
	server_timer_.start_timer(pc, ServerTimer::TOTALTIME);
    //retrieve the block
	server_timer_.start_timer(pc, ServerTimer::BLOCKWAITTIME);
	server_timer_.start_timer(pc, ServerTimer::BLOCKWAITTIME);
	ServerBlock* block = disk_backed_block_map_.get_block_for_reading(block_id,
			pc);
	server_timer_.pause_timer(pc, ServerTimer::BLOCKWAITTIME);

	//create async op to handle the reply
	async_ops_.add_get_reply(mpi_source, get_tag, block_id, block, pc);


	//handle section number updates
	SIP_LOG(
			std::cout << "S " << sip_mpi_attr_.global_rank() << " : get for block " << block_id.str(sip_tables_) << ", size = " << block_size << ", sent from = " << mpi_source << ", at line = " << line_number(pc) << std::endl;)

	if(section < state_.section_number_){
		std::cout << "illegal section number "<< section
				<< " where state_.section_number_ = "<< state_.section_number_
				<< " at block "<< block_id << " line" << line_number(pc) << " from worker "
				<< last_seen_worker_ << std::endl;
		std::cout << "This likely due to a \"get\" without subsequent use of block in the section" << std::flush;
	}
	state_.check_section_number_invariant(section);

    //check for races
	if (!block->update_and_check_consistency(SIPMPIConstants::GET,
			mpi_source, section)) {
		std::stringstream err_ss;
		err_ss << "Data race at server for block " << block_id
				<< "from worker " << mpi_source << ".  Probably a missing sip_barrier";
		sial_check(false, err_ss.str(), line_number(pc));
	}

	server_timer_.pause_timer(pc, ServerTimer::TOTALTIME);

}

void SIPServer::handle_PUT(int mpi_source, int put_tag,
		int transaction_number) {

	//receive and decode the message
	int buffer[SIPMPIUtils::BLOCKID_BUFF_ELEMS];
	MPI_Status status;
	SIPMPIUtils::check_err(
			MPI_Recv(buffer, SIPMPIUtils::BLOCKID_BUFF_ELEMS, MPI_INT, mpi_source, put_tag,
					MPI_COMM_WORLD, &status));
	check_int_count(status, SIPMPIUtils::BLOCKID_BUFF_ELEMS);
	BlockId block_id;
	int section;
	int pc;
	SIPMPIUtils::decode_BlockID_buff(buffer, block_id, pc, section);
	last_seen_worker_ = mpi_source;
	server_timer_.start_timer(pc, ServerTimer::TOTALTIME);

    //retrieve the block for writing
	server_timer_.start_timer(pc, ServerTimer::BLOCKWAITTIME);
	ServerBlock* block = disk_backed_block_map_.get_block_for_writing(block_id);
	server_timer_.pause_timer(pc, ServerTimer::BLOCKWAITTIME);

    //create async_op to handle message with the data
    int put_data_tag = BarrierSupport::make_mpi_tag(SIPMPIConstants::PUT_DATA,
			transaction_number);
	async_ops_.add_put_data_request(mpi_source, put_data_tag, block_id, block, pc);

	//send ack to worker, who is waiting for it
	SIPMPIUtils::check_err(
			MPI_Send(0, 0, MPI_INT, mpi_source, put_tag, MPI_COMM_WORLD),
			__LINE__, __FILE__);

	//handle section number updates
	SIP_LOG(
			std::cout << "S " << sip_mpi_attr_.global_rank() << " : get for block " << block_id.str(sip_tables_) << ", size = " << block_size << ", sent from = " << mpi_source << ", at line = " << last_seen_line_ << std::endl;)

	if(section < state_.section_number_){
		std::cout << "illegal section number "<< section
				<< " where state_.section_number_ = "<< state_.section_number_
				<< " at block "<< block_id << " line" << line_number(pc) << " from worker "
				<< last_seen_worker_ << std::endl;
		std::cout << "This likely due to a \"get\" without subsequent use of block in the section" << std::flush;
	}
	state_.check_section_number_invariant(
			section);

	//data race check
	if (!block->update_and_check_consistency(SIPMPIConstants::PUT,
			mpi_source, section)) {
		std::stringstream err_ss;
		err_ss << "Incorrect PUT block semantics (data race) for " << block_id
				<<  " from worker "
				<< mpi_source << ". Probably a missing sip_barrier";
		sial_check(false, err_ss.str(), line_number(pc));
	}
	server_timer_.pause_timer(pc, ServerTimer::TOTALTIME);



}


void SIPServer::handle_PUT_ACCUMULATE(int mpi_source, int put_accumulate_tag,
		int transaction_number) {

	//receive and decode the message
	int buffer[SIPMPIUtils::BLOCKID_BUFF_ELEMS];
	MPI_Status status;
	SIPMPIUtils::check_err(
			MPI_Recv(buffer, SIPMPIUtils::BLOCKID_BUFF_ELEMS, MPI_INT, mpi_source, put_accumulate_tag,
					MPI_COMM_WORLD, &status));
	check_int_count(status, SIPMPIUtils::BLOCKID_BUFF_ELEMS);
	BlockId block_id;
	int section;
	int pc;
	SIPMPIUtils::decode_BlockID_buff(buffer, block_id, pc, section);
	last_seen_worker_ = mpi_source;
	server_timer_.start_timer(pc, ServerTimer::TOTALTIME);

    //retrieve the block for accumulate
	server_timer_.start_timer(pc, ServerTimer::BLOCKWAITTIME);
	ServerBlock* block = disk_backed_block_map_.get_block_for_accumulate(
			block_id);
	server_timer_.pause_timer(pc, ServerTimer::BLOCKWAITTIME);

    //create async op to handle message with data
	int put_accumulate_data_tag;
	put_accumulate_data_tag = BarrierSupport::make_mpi_tag(
			SIPMPIConstants::PUT_ACCUMULATE_DATA, transaction_number);
	async_ops_.add_put_accumulate_data_request(mpi_source, put_accumulate_data_tag, block_id, block, pc);

	//send ack to worker, who is waiting for it
	SIPMPIUtils::check_err(
			MPI_Send(0, 0, MPI_INT, mpi_source, put_accumulate_tag, MPI_COMM_WORLD),
			__LINE__, __FILE__);

	//handle section number updates
	if(section < state_.section_number_){
		std::cout << "illegal section number "<< section
				<< " where state_.section_number_ = "<< state_.section_number_
				<< " at block "<< block_id << " line" << line_number(pc) << " from worker "
				<< last_seen_worker_ << std::endl;
		std::cout <<  std::flush;
	}
	bool section_number_changed = state_.check_section_number_invariant(
			section);

	//data race check
	if (!block->update_and_check_consistency(SIPMPIConstants::PUT_ACCUMULATE,
			mpi_source, section)) {
		std::stringstream err_ss;
		err_ss << "Incorrect PUT_ACCUMULATE block semantics (data race) for " << block_id
				<<  " from worker "
				<< mpi_source << ". Probably a missing sip_barrier";
		sial_check(false, err_ss.str(),line_number(pc));
	}

	SIP_LOG(
			std::cout << "S " << sip_mpi_attr_.global_rank() << " : put accumulate to receive block " << block_id.str(sip_tables_) << ", size = " << sip_tables_.block_size(block_id) << ", from = " << mpi_source << ", at line = " << last_seen_line_ << std::endl << std::flush;)

	server_timer_.pause_timer(pc, ServerTimer::TOTALTIME);

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
	int array_id = recv_buffer[0];

	int pc = recv_buffer[1];
	last_seen_worker_ = mpi_source;
	int section = recv_buffer[2];

	state_.check_section_number_invariant(
			section);

	server_timer_.start_timer(pc, ServerTimer::TOTALTIME);

	SIP_LOG(
			std::cout << "S " << sip_mpi_attr_.global_rank() << " : deleting array " << sip_tables_.array_name(array_id) << ", id = " << array_id << ", sent from = " << mpi_source << ", at line = " << last_seen_line_ << std::endl;)

	//send ack
	SIPMPIUtils::check_err(
			MPI_Send(0, 0, MPI_INT, mpi_source, delete_tag, MPI_COMM_WORLD),
			__LINE__, __FILE__);

	//delete the block and map for the indicated array
	async_ops_.remove_all_entries_for_array(array_id);
	disk_backed_block_map_.delete_per_array_map_and_blocks(array_id);
	server_timer_.pause_timer(pc, ServerTimer::TOTALTIME);

}

void SIPServer::handle_PUT_INITIALIZE(int mpi_source, int put_initialize_tag) {
	MPIScalarOpType::scalar_op_message_t message;
	MPI_Status status;
	SIPMPIUtils::check_err(MPI_Recv(&message, 1, mpi_type_.mpi_scalar_op_type_, mpi_source,
			put_initialize_tag, MPI_COMM_WORLD, &status));
	int section;
	int pc;
	BlockId block_id;
	SIPMPIUtils::decode_BlockID_buff(message.id_pc_section_buff_, block_id,
			pc, section);

	last_seen_worker_ = mpi_source;
	state_.check_section_number_invariant(
			section);

	ServerBlock* block = disk_backed_block_map_.get_block_for_writing(block_id);

    //send ack
	SIPMPIUtils::check_err(
			MPI_Send(0, 0, MPI_INT, mpi_source, put_initialize_tag,
					MPI_COMM_WORLD), __LINE__, __FILE__);

	//get the block size
	int block_size = sip_tables_.block_size(block_id);
	block->fill_data(message.value_);

	//data race check
	if (!block->update_and_check_consistency(SIPMPIConstants::PUT,
			mpi_source, section)) {
		std::stringstream err_ss;
		err_ss << "Incorrect PUT_INITIALIZE block semantics (data race) for " << block_id
				<<  " from worker "
				<< mpi_source << ". Probably a missing sip_barrier";
		sial_check(false, err_ss.str(), line_number(pc));
	}
}


void SIPServer::handle_PUT_INCREMENT(int mpi_source, int put_increment_tag) {
	MPIScalarOpType::scalar_op_message_t message;
	MPI_Status status;
	SIPMPIUtils::check_err(MPI_Recv(&message, 1, mpi_type_.mpi_scalar_op_type_, mpi_source,
			put_increment_tag, MPI_COMM_WORLD, &status));
	int section;
	int pc;
	BlockId block_id;
	SIPMPIUtils::decode_BlockID_buff(message.id_pc_section_buff_, block_id,
			pc, section);

	last_seen_worker_ = mpi_source;
	state_.check_section_number_invariant(
			section);

	ServerBlock* block = disk_backed_block_map_.get_block_for_accumulate(block_id);
	//accumulates need to be applied in order,
	//get_block_for_accumulate does not wait,
	// so wait for pending to complete
	block->wait();

    //send ack
	SIPMPIUtils::check_err(
			MPI_Send(0, 0, MPI_INT, mpi_source, put_increment_tag,
					MPI_COMM_WORLD), __LINE__, __FILE__);

	//get the block size and perform the operation
	int block_size = sip_tables_.block_size(block_id);
	block->increment_data(message.value_);

	//data race check, same effect as PUT_ACCUMULATE
	if (!block->update_and_check_consistency(SIPMPIConstants::PUT_ACCUMULATE,
			mpi_source, section)) {
		std::stringstream err_ss;
		err_ss << "Incorrect PUT_INCREMENT block semantics (data race) for " << block_id
				<<  " from worker "
				<< mpi_source << ". Probably a missing sip_barrier";
		sial_check(false, err_ss.str(), line_number(pc));
	}
}

void SIPServer::handle_PUT_SCALE(int mpi_source, int put_scale_tag) {

	MPIScalarOpType::scalar_op_message_t message;
	MPI_Status status;
	SIPMPIUtils::check_err(MPI_Recv(&message, 1, mpi_type_.mpi_scalar_op_type_, mpi_source,
			put_scale_tag, MPI_COMM_WORLD, &status));
	int section;
	int pc;
	BlockId block_id;
	SIPMPIUtils::decode_BlockID_buff(message.id_pc_section_buff_, block_id,
			pc, section);

	last_seen_worker_ = mpi_source;
	state_.check_section_number_invariant(section);


	ServerBlock* block = disk_backed_block_map_.get_block_for_writing(block_id);

    //send ack
	SIPMPIUtils::check_err(
			MPI_Send(0, 0, MPI_INT, mpi_source, put_scale_tag,
					MPI_COMM_WORLD), __LINE__, __FILE__);

	//get the block size and perform the operation
	int block_size = sip_tables_.block_size(block_id);
	block->scale_data(message.value_);

	//data race check, same effect as PUT
	if (!block->update_and_check_consistency(SIPMPIConstants::PUT,
			mpi_source, section)) {
		std::stringstream err_ss;
		err_ss << "Incorrect PUT_SCALE block semantics (data race) for " << block_id
				 << " from worker "
				<< mpi_source << ". Probably a missing sip_barrier";
		sial_check(false, err_ss.str(), line_number(pc));
	}
}

void SIPServer::handle_END_PROGRAM(int mpi_source, int end_program_tag) {
	SIP_LOG(
			std::cout << "S " << sip_mpi_attr_.global_rank()<< " : In END_PROGRAM at server " << std::endl);

	//receive the message (which is empty)
	MPI_Status status;
	SIPMPIUtils::check_err(
			MPI_Recv(0, 0, MPI_INT, mpi_source, end_program_tag, MPI_COMM_WORLD,
					&status), __LINE__, __FILE__);

//	last_seen_line_ = 0; 	// Special line number for end program
	int pc = sip_tables_.op_table_size();
	last_seen_worker_ = mpi_source;

	server_timer_.start_timer(pc, ServerTimer::TOTALTIME);

	SIP_LOG(
			std::cout << "S " << sip_mpi_attr_.global_rank() << " : ending program " << ", sent from = " << mpi_source << std::endl;)

	//handle any remaining async ops
	async_ops_.wait_all();

	//send ack
	SIPMPIUtils::check_err(
			MPI_Send(0, 0, MPI_INT, mpi_source, end_program_tag,
					MPI_COMM_WORLD), __LINE__, __FILE__);

	//set terminated flag;
	terminated_ = true;

	server_timer_.pause_timer(pc, ServerTimer::TOTALTIME);

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

	int pc  = buffer[2];
	last_seen_worker_ = mpi_source;

	int section = buffer[3];

	state_.check_section_number_invariant(section);


	server_timer_.start_timer(pc, ServerTimer::TOTALTIME);

	//send ack
	SIPMPIUtils::check_err(
			MPI_Send(0, 0, MPI_INT, mpi_source, set_persistent_tag,
					MPI_COMM_WORLD), __LINE__, __FILE__);


	//upcall to persistent_array_manager
	persistent_array_manager_->set_persistent(this, array_id, string_slot);

	SIP_LOG(
			std::cout << "S " << sip_mpi_attr_.global_rank() << " : set persistent array " << sip_tables_.array_name(array_id) << ", id = " << array_id << ", sent from = " << mpi_source << ", at line = " << last_seen_line_ << std::endl;);

	server_timer_.pause_timer(pc, ServerTimer::TOTALTIME);

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
	int pc = buffer[2];
	last_seen_worker_ = mpi_source;

	int section = buffer[3];
	state_.check_section_number_invariant(section);


	server_timer_.start_timer(pc, ServerTimer::TOTALTIME);

	//send ack
	SIPMPIUtils::check_err(
			MPI_Send(0, 0, MPI_INT, mpi_source, restore_persistent_tag,
					MPI_COMM_WORLD), __LINE__, __FILE__);

	//upcall
	persistent_array_manager_->restore_persistent(this, array_id, string_slot);

	SIP_LOG(
			std::cout << "S " << sip_mpi_attr_.global_rank() << " : restored persistent array " << sip_tables_.array_name(array_id) << ", id = " << array_id << ", sent from = " << mpi_source << ", at line = " << last_seen_line_ << std::endl;)

	server_timer_.pause_timer(pc, ServerTimer::TOTALTIME);

}

void SIPServer::check_int_count(MPI_Status& status, int expected_count) {
	int received_count;
	SIPMPIUtils::check_err(MPI_Get_count(&status, MPI_INT, &received_count),
			__LINE__, __FILE__);
	check(received_count == expected_count,
			"message int count different than expected");
}

void SIPServer::check_double_count(const MPI_Status& status,
		int expected_count) {
	int received_count;
	SIPMPIUtils::check_err(
			MPI_Get_count(&(const_cast<MPI_Status&>(status)), MPI_DOUBLE,
					&received_count), __LINE__, __FILE__);
	check(received_count == expected_count,
			"message double count different than expected");
}

std::ostream& operator<<(std::ostream& os, const SIPServer& obj) {
	os << "\nblock_map_:" << std::endl << obj.disk_backed_block_map_;
	os << "state_: " << obj.state_ << std::endl;
	os << "terminated=" << obj.terminated_ << std::endl;
	return os;
}

std::ostream& operator<<(std::ostream& os, const MPI_Status& status) {
	int mpi_tag = status.MPI_TAG;
	int mpi_source = status.MPI_SOURCE;
	os << "MPI_Status: ";
	os << " source=" << status.MPI_SOURCE;
	os << ", tag=" << status.MPI_TAG;
	int received_count;
	SIPMPIUtils::check_err(
			MPI_Get_count(&(const_cast<MPI_Status&>(status)), MPI_DOUBLE,
					&received_count), __LINE__, __FILE__);
	os << ", count=" << received_count;
	return os;
}

//int SIPServer::last_seen_line() {
//	return last_seen_line_;
//}

} /* namespace sip */
