/*
 * sial_ops_parallel.cpp
 *
 */

#include <sial_ops_parallel.h>
#include <iostream>
#include <cmath>
#include <cfloat>
#include "sip_mpi_utils.h"
#include "interpreter.h"


namespace sip {


#ifdef HAVE_MPI //only compile if parallel
SialOpsParallel::SialOpsParallel(DataManager& data_manager,
		WorkerPersistentArrayManager* persistent_array_manager,
		SipTables& sip_tables) :
		sip_tables_(sip_tables), sip_mpi_attr_(
				SIPMPIAttr::get_instance()), data_manager_(data_manager), block_manager_(
				data_manager.block_manager_), data_distribution_(sip_tables_,
				sip_mpi_attr_), persistent_array_manager_(
				persistent_array_manager), mode_(sip_tables_.num_arrays(), NONE) {
}

SialOpsParallel::~SialOpsParallel() {
}

void SialOpsParallel::sip_barrier() {
	//wait for all expected acks,
	ack_handler_.wait_all();

	//do an MPI barrier among all the workers
	MPI_Comm worker_comm = sip_mpi_attr_.company_communicator();
	int num_workers;
	MPI_Comm_size(worker_comm, &num_workers);
	if (num_workers > 1) {
		SIPMPIUtils::check_err(MPI_Barrier(worker_comm));
	}

	//update the local sip_barrier state;
	//increment section number, reset msg number
	barrier_support_.update_state_at_barrier();

	// Remove and deallocate cached blocks of distributed and served arrays
	//TODO optimize this--if only read and needed in the future, don't need to delete.
	for (int i = 0; i < sip_tables_.num_arrays(); ++i) {
		if (sip_tables_.is_distributed(i) || sip_tables_.is_served(i))
			block_manager_.delete_per_array_map_and_blocks(i);
	}
	reset_mode();

	SIP_LOG(std::cout<< "W " << sip_mpi_attr_.global_rank() << " : Done with BARRIER "<< std::endl);

}

/** Currently this is a no-op.  Map, and blocks are created lazily when needed */
void SialOpsParallel::create_distributed(int array_id) {
}

/** Removes all the blocks associated with the given array from the block map.
 * Removing the array from the map will cause its destructor to
 * be called, which will delete the data. Because of this, we must be very
 * careful to remove blocks that should not be delete from the block_map_.
 */
void SialOpsParallel::delete_distributed(int array_id) {

	//delete any blocks stored locally along with the map
	block_manager_.block_map_.delete_per_array_map_and_blocks(array_id);

	//send delete message to server if responsible worker
	int server_rank = sip_mpi_attr_.my_server();
	if (server_rank > 0) {
		SIP_LOG(std::cout<<"W " << sip_mpi_attr_.global_rank() << " : sending DELETE for array " << array_id << " to server "<< server_rank << std::endl);
		std::cout<<"W " << sip_mpi_attr_.global_rank() << " : sending DELETE for array " << array_id << " to server "<< server_rank << std::endl;  //DEBUG
		int delete_tag = barrier_support_.make_mpi_tag_for_DELETE();
		SIPMPIUtils::check_err(
				MPI_Send(&array_id, 1, MPI_INT, server_rank, delete_tag,
						MPI_COMM_WORLD));
		ack_handler_.expect_ack_from(server_rank, delete_tag);
	}
}

//TODO optimize this.  Can reduce searches in block map.
void SialOpsParallel::get(BlockId& block_id) {

	//check for "data race"
	check_and_set_mode(block_id, READ);

	//if block already exists, or has pending request, just return
	Block::BlockPtr block = block_manager_.block(block_id);
	if (block != NULL)
		return;

	//send get message to block's server, and post receive
	int server_rank = data_distribution_.get_server_rank(block_id);
	int get_tag;
	get_tag = barrier_support_.make_mpi_tag_for_GET();

    sip::check(server_rank>=0&&server_rank<sip_mpi_attr_.global_size(), "invalid server rank",current_line()); 

    SIP_LOG(std::cout<<"W " << sip_mpi_attr_.global_rank()
    		<< " : sending GET for block " << block_id
    		<< " to server "<< server_rank << std::endl);

	SIPMPIUtils::check_err(
			MPI_Send(block_id.to_mpi_array(), BlockId::MPI_BLOCK_ID_COUNT, MPI_INT,
					server_rank, get_tag, MPI_COMM_WORLD));

	//allocate block, and insert in block map, using block data as buffer
	block = block_manager_.get_block_for_writing(block_id, true);

	//post an asynchronous receive and store the request in the
	//block's state
	MPI_Request request;
	SIPMPIUtils::check_err(
			MPI_Irecv(block->get_data(), block->size(), MPI_DOUBLE, server_rank,
					get_tag, MPI_COMM_WORLD, &request));
	block->state().mpi_request_ = request;
}

//NOTE:  I can't remember why the source block was copied.
//DO NOT DELETE until we are sure that the version without copy is OK
//** A put appears in a SIAL program as
// * put target(i,j,k,l) = source(i,j,k,l)
// *
// * To get this right, we copy the contents of the source block into a local
// * instance of the target block and "put" the target block.
// *
// * TODO  optimize this to eliminate the copy
// *
// * @param target
// * @param source_ptr
// */
//void SialOpsParallel::put_replace(BlockId& target_id,
//		const Block::BlockPtr source_block) {
//	//check for data races
//	check_and_set_mode(target_id, WRITE);
//
//	Block::BlockPtr target_block = block_manager_.get_block_for_writing(
//			target_id, false);
//	target_block->copy_data_(source_block);
//	int my_rank = sip_mpi_attr_.global_rank();
//	int server_rank = data_distribution_.get_server_rank(target_id);
//	int put_tag, put_data_tag;
//	put_tag = barrier_support_.make_mpi_tags_for_PUT(put_data_tag);
//	//send message with array_id to server
//	SIPMPIUtils::check_err(
//			MPI_Send(target_id.to_mpi_array(), BlockId::MPI_COUNT, MPI_INT,
//					server_rank, put_tag, MPI_COMM_WORLD));
//	//immediately follow with the data
//	//TODO  should we wait for ack before sending data???
//	SIPMPIUtils::check_err(
//			MPI_Send(target_block->data_, target_block->size_, MPI_DOUBLE,
//					server_rank, put_data_tag, MPI_COMM_WORLD));
//
//	//the data message should be acked
//	ack_handler_.expect_ack_from(server_rank, put_data_tag);
//	SIP_LOG(
//			std::cout << "W " << my_rank << " : Done with PUT for block " << target_id << " to server rank " << server_rank << std::endl;)
//
//	//can go ahead and remove since MPI_Send is synchronous.  If not, it
//	//will be removed at the next update_state_at_barrier, anyway.  The data race check should
//	//prevent readers.  This is not checked, but a correct program will not write
//	//the same block twice without an intervening update_state_at_barrier.
//	block_manager_.delete_block(target_id);
//
//}

/**
 * A put appears in a SIAL program as
 * put target(i,j,k,l) = source(i,j,k,l)
 * So we need the target block id, but the source block data.
 *
 * The implementation will be more complicated if asynchronous send is
 * used
 *
 * @param target
 * @param source_ptr
 */
void SialOpsParallel::put_replace(BlockId& target_id,
		const Block::BlockPtr source_block) {
	//partial check for data races
	check_and_set_mode(target_id, WRITE);

	//send message with array_id to server
	int my_rank = sip_mpi_attr_.global_rank();
	int server_rank = data_distribution_.get_server_rank(target_id);
	int put_tag, put_data_tag;
	put_tag = barrier_support_.make_mpi_tags_for_PUT(put_data_tag);

    sip::check(server_rank>=0&&server_rank<sip_mpi_attr_.global_size(), "invalid server rank",current_line()); 

    SIP_LOG(std::cout<<"W " << sip_mpi_attr_.global_rank()
    		<< " : sending PUT for block " << target_id
    		<< " to server "<< server_rank << std::endl);

	SIPMPIUtils::check_err(
			MPI_Send(target_id.to_mpi_array(), BlockId::MPI_BLOCK_ID_COUNT, MPI_INT,
					server_rank, put_tag, MPI_COMM_WORLD));

	//immediately follow with the data
	//TODO  should we wait for ack before sending data???
	SIPMPIUtils::check_err(
			MPI_Send(source_block->get_data(), source_block->size(), MPI_DOUBLE,
					server_rank, put_data_tag, MPI_COMM_WORLD));

	//the data message should be acked
	ack_handler_.expect_ack_from(server_rank, put_data_tag);

	SIP_LOG(std::cout << "W " << my_rank << " : Done with PUT for block " << target_id << " to server rank " << server_rank << std::endl;)

}

//NOTE:  I can't remember why the source block was copied.
//DO NOT DELETE until we are sure that the version without copy is OK
// void SialOpsParallel::put_accumulate(BlockId& target_id,
//		const Block::BlockPtr source_block) {
//
//	//check for data races
//	check_and_set_mode(target_id, WRITE);
//
//	int my_rank = sip_mpi_attr_.global_rank();
//	Block::BlockPtr target_block = block_manager_.get_block_for_writing(
//			target_id, false); //write locally by copying rhs, accumulate is done at server
//	target_block->copy_data_(source_block);
//
//	int server_rank = data_distribution_.get_server_rank(target_id);
//
//	int put_accumulate_tag, put_accumulate_data_tag;
//	put_accumulate_tag = barrier_support_.make_mpi_tags_for_PUT_ACCUMULATE(
//			put_accumulate_data_tag);
//	//send block id
//	SIPMPIUtils::check_err(
//			MPI_Send(target_id.to_mpi_array(), BlockId::MPI_COUNT, MPI_INT,
//					server_rank, put_accumulate_tag, MPI_COMM_WORLD));
//	//immediately follow with the data
//	SIPMPIUtils::check_err(
//			MPI_Send(target_block->data_, target_block->size_, MPI_DOUBLE,
//					server_rank, put_accumulate_data_tag, MPI_COMM_WORLD));
//
//	//ack
//	ack_handler_.expect_ack_from(server_rank, put_accumulate_data_tag);
//
//	SIP_LOG(
//			std::cout<< "W " << sip_mpi_attr_.global_rank() << " : Done with PUT_ACCUMULATE for block " << lhs_id << " to server rank " << server_rank << std::endl);
//size_
//	//it is legal to accumulate into the block multiple so keep the block
//	//around until the update_state_at_barrier.
//}

/**
 * A put appears in a SIAL program as
 * put target(i,j,k,l) += source(i,j,k,l)
 * So we need the target block id, but the source block data.
 * Accumulation is done by the server
 *
 * The implementation will be more complicated if asynchronous send is
 * used
 *
 * @param target
 * @param source_ptr
 */
void SialOpsParallel::put_accumulate(BlockId& target_id,
		const Block::BlockPtr source_block) {

	//partial check for data races
	check_and_set_mode(target_id, WRITE);

	//send message with target block's id to server
	int my_rank = sip_mpi_attr_.global_rank();
	int server_rank = data_distribution_.get_server_rank(target_id);
	int put_accumulate_tag, put_accumulate_data_tag;
	put_accumulate_tag = barrier_support_.make_mpi_tags_for_PUT_ACCUMULATE(
			put_accumulate_data_tag);

    sip::check(server_rank>=0&&server_rank<sip_mpi_attr_.global_size(), "invalid server rank",current_line()); 

    SIP_LOG(std::cout<<"W " << sip_mpi_attr_.global_rank()
       		<< " : sending PUT_ACCUMULATE for block " << target_id
       		<< " to server "<< server_rank << std::endl);

	//send block id
	SIPMPIUtils::check_err(
			MPI_Send(target_id.to_mpi_array(), BlockId::MPI_BLOCK_ID_COUNT, MPI_INT,
					server_rank, put_accumulate_tag, MPI_COMM_WORLD));
	//immediately follow with the data
	//like put--maybe we should wait for ack from server
	SIPMPIUtils::check_err(
			MPI_Send(source_block->get_data(), source_block->size(), MPI_DOUBLE,
					server_rank, put_accumulate_data_tag, MPI_COMM_WORLD));

	//ack
	ack_handler_.expect_ack_from(server_rank, put_accumulate_data_tag);

	SIP_LOG(
			std::cout<< "W " << sip_mpi_attr_.global_rank() << " : Done with PUT_ACCUMULATE for block " << target_id << " to server rank " << server_rank << std::endl);

}

void SialOpsParallel::destroy_served(int array_id) {
	delete_distributed(array_id);
}

void SialOpsParallel::request(BlockId& block_id) {
	get(block_id);
}
void SialOpsParallel::prequest(BlockId&, BlockId&) {
	fail("PREQUEST Not supported !");
}
void SialOpsParallel::prepare(BlockId& lhs_id, Block::BlockPtr source_ptr) {
	put_replace(lhs_id, source_ptr);
}
void SialOpsParallel::prepare_accumulate(BlockId& lhs_id,
		Block::BlockPtr source_ptr) {
	put_accumulate(lhs_id, source_ptr);
}

void SialOpsParallel::collective_sum(double rhs_value,
		int dest_array_slot) {

	double lhs_value = data_manager_.scalar_value(dest_array_slot);

	double reduced_value = 0.0;
	if (sip_mpi_attr_.num_workers() > 1) {
		const MPI_Comm& worker_comm = sip_mpi_attr_.company_communicator();
		SIPMPIUtils::check_err(
				MPI_Allreduce(&rhs_value, &reduced_value, 1, MPI_DOUBLE, MPI_SUM,
						worker_comm));
	} else {
		reduced_value = rhs_value;
	}

	data_manager_.set_scalar_value(dest_array_slot,lhs_value + reduced_value);
}




bool SialOpsParallel::assert_same(int source_array_slot){
	if (sip_mpi_attr_.num_workers() <= 1) return true;
	int rank = sip_mpi_attr_.company_rank_;
	const MPI_Comm& worker_comm = sip_mpi_attr_.company_communicator();
	double value;
	if (rank == 0){
       value = data_manager_.scalar_value(source_array_slot);
	   SIPMPIUtils::check_err(MPI_Bcast(&value, 1, MPI_DOUBLE, 0, worker_comm));
	   return true;
	}
		value = 0.0;
		double EPSILON = .00005;
		SIPMPIUtils::check_err(MPI_Bcast(&value, 1, MPI_DOUBLE, 0, worker_comm));
		bool close = nearlyEqual(value, data_manager_.scalar_value(source_array_slot), EPSILON);
		check (close, "values are too far apart");
		//replace old value with new value from master.
		data_manager_.set_scalar_value(source_array_slot, value);
		return close;
}


/* QUICK SOLUTION.  NEEDS IMPROVEMENT!
 */
bool SialOpsParallel::nearlyEqual(double a, double  b, double epsilon) {
	double absA = std::abs(a);
	double absB = std::abs(b);
	double diff = std::abs(a - b);

	if (a == b) { // shortcut, handles infinities
		return true;
	} else if (a == 0 || b == 0 || diff < DBL_MIN) {
		//DBL_MIN is from <cfloat>  and is the smallest representable pos value, 1E-37 or smaller
		// a or b is zero or both are extremely close to it
		// relative error is less meaningful here
		return diff < (epsilon * DBL_MIN);
	} else { // use relative error
		return diff / (absA + absB) < epsilon;
	}
}


void SialOpsParallel::broadcast_static(int source_array_slot, int source_worker){
	if (sip_mpi_attr_.num_servers() )
		std::cout<< "placeholder for broadcast_static";
}



/**
 * action depends on whether array is local or remote.  If remote,
 * a message is sent to my_server (if responsible). Otherwise,
 * a upcall is made to the local persistent_array_manager.
 *
 * @param worker
 * @param array_slot
 * @param string_slot
 */
void SialOpsParallel::set_persistent(Interpreter * worker, int array_slot,
		int string_slot) {
	if (sip_tables_.is_distributed(array_slot)
			|| sip_tables_.is_served(array_slot)) {
		int my_server = sip_mpi_attr_.my_server();
		if (my_server > 0) {
			int set_persistent_tag;
			set_persistent_tag =
					barrier_support_.make_mpi_tag_for_SET_PERSISTENT();
			int buffer[2] = { array_slot, string_slot };
			SIPMPIUtils::check_err(
					MPI_Send(buffer, 2, MPI_INT, my_server, set_persistent_tag,
							MPI_COMM_WORLD));

			//ack
			ack_handler_.expect_ack_from(my_server, set_persistent_tag);
		}
	} else {
		persistent_array_manager_->set_persistent(worker, array_slot,
				string_slot);
	}
}

/**
 * action depends on whether array is local or remote.  If remote,
 * a message is sent to my_server (if responsible). Otherwise,
 * a upcall is made to the local persistent_array_manager.
 * @param worker
 * @param array_slot
 * @param string_slot
 */

void SialOpsParallel::restore_persistent(Interpreter* worker, int array_slot,
		int string_slot) {
	SIP_LOG(std::cout << "restore_persistent with array " << sip_tables_.array_name(array_slot) << " in slot " << array_slot << " and string \"" << sip_tables_.string_literal(string_slot) << "\"" << std::endl);

	if (sip_tables_.is_distributed(array_slot)
			|| sip_tables_.is_served(array_slot)) {
		int my_server = sip_mpi_attr_.my_server();
		if (my_server > 0) {
			int restore_persistent_tag;
			restore_persistent_tag =
					barrier_support_.make_mpi_tag_for_RESTORE_PERSISTENT();
			int buffer[2] = { array_slot, string_slot };
			SIPMPIUtils::check_err(
					MPI_Send(buffer, 2, MPI_INT, my_server,
							restore_persistent_tag, MPI_COMM_WORLD));
			//expect ack
			ack_handler_.expect_ack_from(my_server, restore_persistent_tag);
		}
	} else {
		persistent_array_manager_->restore_persistent(worker, array_slot,
				string_slot);
		SIP_LOG(std::cout << "returned from restore_persistent" << std::endl << std::flush);
	}

}

void SialOpsParallel::end_program() {
	//implicit sip_barrier
	//this is required to ensure that there are no pending messages
	//at the server when the end_program message arrives.
	sip_barrier();
	int my_server = sip_mpi_attr_.my_server();
//	std::cout << "in end_program, attr =\n" << sip_mpi_attr_ << std::endl << std::flush;
	SIP_LOG(std::cout << "I'm a worker and my server is " << my_server << std::endl << std::flush);
	//send end_program message to server, if designated worker and wait for ack.
	if (my_server > 0) {
		int end_program_tag;
		end_program_tag = barrier_support_.make_mpi_tag_for_END_PROGRAM();
		SIPMPIUtils::check_err(
				MPI_Send(0, 0, MPI_INT, my_server, end_program_tag,
						MPI_COMM_WORLD));
		ack_handler_.expect_sync_ack_from(my_server, end_program_tag);
	}
	//the program is done and the servers know it.
	SIP_LOG(std::cout << "leaving end_program" << std::endl << std::flush);
}

//void SialOpsParallel::print_to_ostream(std::ostream& out, const std::string& to_print){
//	/** If all ranks should print, do that,
//	 * Otherwise just print from company master.
//	 */
//	if (sip::should_all_ranks_print()){
//		out << "W " << sip_mpi_attr_.global_rank() << " : " << to_print << std::flush;
//	} else {
//		if (sip_mpi_attr_.is_company_master()){
//			out << to_print << std::flush;
//		}
//	}
//}

//enum array_mode {NONE, READ, WRITE};
bool SialOpsParallel::check_and_set_mode(int array_id, array_mode mode) {
	array_mode current = mode_.at(array_id);
	if (current == NONE) {
		mode_[array_id] = mode;
		return true;
	}
	return (current == mode);
}

bool SialOpsParallel::check_and_set_mode(const BlockId& id, array_mode mode) {
	int array_id = id.array_id();
	return check_and_set_mode(array_id, mode);
}
/**resets the mode of all arrays to NONE.  This should be invoked
 * in SIP_barriers.
 */
void SialOpsParallel::reset_mode() {
	std::fill(mode_.begin(), mode_.end(), NONE);
}

void SialOpsParallel::log_statement(opcode_t type, int line){
	SIP_LOG(
			std::cout<< "W " << sip_mpi_attr_.global_rank()
					 << " : Line "<<line << ", type: " << opcodeToName(type)<<std::endl);
}

Block::BlockPtr SialOpsParallel::get_block_for_reading(const BlockId& id) {
	int array_id = id.array_id();
	if (sip_tables_.is_distributed(array_id)
			|| sip_tables_.is_served(array_id)) {
		check_and_set_mode(array_id, READ);
		return wait_and_check(block_manager_.get_block_for_reading(id));
	}
	return block_manager_.get_block_for_reading(id);
}

Block::BlockPtr SialOpsParallel::get_block_for_writing(const BlockId& id,
		bool is_scope_extent) {
	int array_id = id.array_id();
	if (sip_tables_.is_distributed(array_id)
			|| sip_tables_.is_served(array_id)) {
		check(!is_scope_extent,
				"sip bug: asking for scope-extend dist or served block");
		check_and_set_mode(array_id, WRITE);
	}
	return block_manager_.get_block_for_writing(id, is_scope_extent);
}

Block::BlockPtr SialOpsParallel::get_block_for_updating(const BlockId& id) {
	int array_id = id.array_id();
	check(
			!(sip_tables_.is_distributed(array_id)
					|| sip_tables_.is_served(array_id)),
			"attempting to update distributed or served block", current_line());

	return block_manager_.get_block_for_updating(id);
}

Block::BlockPtr SialOpsParallel::wait_and_check(Block::BlockPtr b) {
	if (b->state().pending()) {
		b->state().wait(b->size());
	}
	return b;
}

#endif //HAVE_MPI

} /* namespace sip */
