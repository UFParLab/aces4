/*
 * sip_server.h
 *
 *  Created on: Jan 14, 2014
 *      Author: njindal
 */

#ifndef SIP_SERVER_H_
#define SIP_SERVER_H_
#include <map>
#include <vector>
#include <utility>
#include <iomanip>
#include <ios>

#include "sip_tables.h"
#include "server_block.h"
#include "data_distribution.h"
#include "barrier_support.h"
#include "server_persistent_array_manager.h"
#include "disk_backed_block_map.h"
//#include "server_timer.h"
#include "counter.h"




namespace sip {

std::ostream& operator<<(std::ostream& os, const MPI_Status& obj);

class SIPServer;



/**
 * Manages server-wide pending blocks by keeping a list of (pointers to) blocks that
 * may have pending asynchronous operations.
 *
 * The server loop should check occasionally (may_have_pending)
 * and handle pending requests (try_pending). An iterator maintains the
 * next block to look at, so on each call of try_pending, the next block will
 * be chosen.
 *
 * When a block no longer has any pending requests, it should be removed from the list.
 *
 * The system should maintain the
 * invariant that if a block has a pending request, it is in the list.  The
 * converse does not necessarily hold. It is also the case that a block may
 * appear more than once in the list, but only one entry will be removed each time.
 *
 * Invariants:
 * if a block is in the pending list, it is in memory
 * (thus it must be deleted from the pending list on delete block, and as
 *    part of disk backing)
 */
class PendingAsyncManager{
public:
	PendingAsyncManager():next_block_iter_(pending_.begin())
	, pending_counter_(SIPMPIAttr::get_instance().company_communicator())
{}
	~PendingAsyncManager(){
		check(pending_.empty(),"destructing non-empty PendingDataRequestManager");
	}

	void add_put_data_request(int mpi_source, int put_data_tag, BlockId id, ServerBlock* block, int pc){
		block->async_state_.add_put_data_request(mpi_source, put_data_tag, block, pc);
		pending_.push_back(std::pair<BlockId,ServerBlock*>(id,block));
		pending_counter_.inc();
	}

	void add_put_accumulate_data_request(int mpi_source, int put_accumulate_data_tag, BlockId id, ServerBlock* block, int pc){
		block->async_state_.add_put_accumulate_data_request(mpi_source, put_accumulate_data_tag, block, pc);
		pending_.push_back(std::pair<BlockId,ServerBlock*>(id,block));
		pending_counter_.inc();
	}

	void add_get_reply(int mpi_source, int get_tag, BlockId id, ServerBlock* block, int pc){
		block->async_state_.add_get_reply(mpi_source, get_tag, block, pc);
		pending_.push_back(std::pair<BlockId,ServerBlock*>(id,block));
		pending_counter_.inc();
	}

	/**
	 * Returns true if there may be pending async ops.
	 * It is possible that there are blocks in the pending list that have no pending ops,
	 * so we only have false => no pending ops
	 * @return
	 */
	bool may_have_pending(){return !pending_.empty();}

	/**
	 * Tries to handle the pending requests for the next block in the pending list.
	 * If all requests for that block have been handled, then it is removed from the list.
	 *
	 */
	void try_pending() { //tries to handle a block
		if (pending_.empty())
			return;
		if (next_block_iter_ == pending_.end()) { //wrap around list
			next_block_iter_ = pending_.begin();
		}
		bool block_complete =
//				(next_block_iter_->second)->async_state_.try_handle_test_none_pending(); //this could be replace with try_handle_all_test_none_pending
		                                                        //for less overhead, but
		                                                        //longer latency for short messages.
		(next_block_iter_->second)->async_state_.try_handle_all_test_none_pending();
		if (block_complete) {
			next_block_iter_ = pending_.erase(next_block_iter_);
		} else {
			++next_block_iter_;
		}
		pending_counter_.set(pending_.size());
	}

	void wait_all() {
		while (may_have_pending()) {
			try_pending();
		}
		pending_counter_.set(pending_.size());
	}


	//call when an array is deleted.
	//leaves next_block_iter_ pointing to a valid
	//iterator (possibly end) for pending_
	//next_block_iter_ is unchanged if nothing was deleted
	void remove_all_entries_for_array(int array_id){
		std::list<std::pair<BlockId,ServerBlock*> >::iterator iter = pending_.begin();
		while (iter != pending_.end()){
			if(iter->first.array_id() == array_id){
				bool block_complete = iter->second->async_state_.try_handle_test_none_pending();
				check(block_complete, "attempting to delete array with pending async ops");
				iter = pending_.erase(iter);
				next_block_iter_ = iter;
			}
			else ++iter;
		}
		pending_counter_.set(pending_.size());
	}

//	std::ostream& gather_and_print_statistics(std::ostream& out, const std::string& title=""){
//		pending_counter_.gather();
//		if (SIPMPIAttr::get_instance().is_company_master()){
//			out << title << std::endl;
//			out << pending_counter_ << std::endl << std::flush;
//		}
//		return out;
//	}
	friend class SIPServer;

private:
	std::list<std::pair<BlockId,ServerBlock*> > pending_;
	std::list<std::pair<BlockId,ServerBlock*> >::iterator next_block_iter_;  //points to next block in list to try
	MPIMaxCounter pending_counter_;
//	NoopCounter<long> pending_counter_;
    DISALLOW_COPY_AND_ASSIGN(PendingAsyncManager);
};

/** SIPServer manages distributed/served arrays.
 *
 * The server executes a loop, run, that repeatedly polls for messages from workers and handles them.
 * The loop terminates when an END_PROGRAM message is received.
 *
 * The current sip_barrier implementation requires each "transaction" initiated by a worker be acknowledged by the server.
 * Also, workers and servers need to be able to match messages that belong to a single transaction.
 * For example, a GET transaction has a request and a reply.  PUT and PUT_ACCUMULATE
 * involve two messages from the worker to the server, one containing the block meta-data (id, size)
 * and another containing the data itself; followed by an ack from the server to the worker.
 * Each transaction is given a unique number.  To allow some error checking of the barrier routines,
 * this number is formed by combining the section number (each barrier increases the section number)
 * and message number within the barrier.  In a message, the transaction number is combined with
 * a constant indicating the message type and sent in the tag of the mpi message.
 *
 * The BarrierSupport class provides routines for constructing tags and extracting the components;
 * it also maintains the section and message numbers and provides a routine for checking the
 * section number invariant at server.
 *
 * The server supports the following "transactions" with workers:
 * GET: server receives GET from worker, replies with requested block.  It is a fatal error for a worker to request a block that doesn't exist
 * PUT: server receives PUT from worker with block id, server receives matching PUT_DATA from worker.  replies with PUT_DATA_ACK.
 * PUT_ACCUMULATE: server receives PUT_ACCUMLATE from worker with block id, server receives matching PUT_ACCUMULATE_DATA from worker.  replies with PUT_ACCUMULATE_DATA_ACK.
 * PUT_INITIALIZE: server receives PUT_INITIALIZE from  worker with block id and double value and replies with ack containing same tag.  Server initializes each element of the
 *     block to the value. If the block does not exist, it is created.
 * PUT_INCREMENT: server receives PUT_INCREMENT from worker with block id and double value and replies with ack containing the same tag.  Server increments each element
 *     of the block with the given value.
 * PUT_SCALE:  server receives PUT_SCALE from worker with block id and double value and replies with ack containing the same tag.  Server multiplies each element
 *     of the block with the given value.
 * DELETE:  deletes all the blocks of the indicated array
 *
 * The communication between servers and workers is designed to bypass MPI buffers for large messages.
 * Unanticipated  messages handled by the server loop are short.  Messages containing blocks are announced
 * first with a short message, then the server posts an MPI_Irecv and sends an acknowledgment.  On
 * receipt of the ack, the worker send the anticipated large message containing the data.
 *
 */

class SIPServer {

public:
	SIPServer(SipTables&, DataDistribution&, SIPMPIAttr&, ServerPersistentArrayManager*);
	~SIPServer();


	/** Static pointer to the current SIPServer.  This is
	 * initialized in the SIPServer constructor and reset to NULL
	 * in its destructor.  There should be at most on SIPServer instance
	 * at any given time.
	 */
	static SIPServer* global_sipserver;

	/**
	 * Main server loop
	 */
	void run();


	IdBlockMap<ServerBlock>::PerArrayMap* per_array_map(int array_id){
		return disk_backed_block_map_.per_array_map(array_id);
	}

//	/**last_
//	 * Called by persistent_array_manager. Delegates to block_map_.
//	 */
//	IdBlockMap<ServerBlock>::PerArrayMap* get_and_remove_per_array_map(int array_id){
//			return disk_backed_block_map_.get_and_remove_per_array_map(array_id);
//	}

	/**
	 * Called by persistent_array_manager during restore.  delegates
	 * to the block map
	 *
	 * @param array_id
	 * @param map_ptr
	 */
	void set_per_array_map(int array_id, IdBlockMap<ServerBlock>::PerArrayMap* map_ptr){
		disk_backed_block_map_.insert_per_array_map(array_id, map_ptr);
	}


	const SipTables* sip_tables() { return &sip_tables_;}


	/** The following methods are called by the PersistentArrayManager.
	 * If the functionality is not required on the server, they are empty--
	 * or rather cause a runtime error.
	 * @param slot
	 * @return
	 */
	std::string string_literal(int slot) {
		return sip_tables_.string_literal(slot);
	}
	std::string array_name(int array_id) {
		return sip_tables_.array_name(array_id);
	}

	/**returns the line in the sials code corresponding to the given pc
	 * or -1 if the pc value is past the end of the program, or program end
	 *
	 * @param pc
	 * @return
	 */
	int line_number(int pc) const {
		return sip_tables_.line_number(pc);
	}

	void gather_and_print_statistics(std::ostream& os){
//		op_timer_.gather();
		op_timer_.reduce();
//		get_block_timer_.gather(); //indexed by pc
		get_block_timer_.reduce();
//		idle_timer_.gather();
		idle_timer_.reduce();
//		total_timer_.gather();
		total_timer_.reduce();
		pending_timer_.gather();
		pending_timer_.reduce();
		handle_op_timer_.reduce();
		num_ops_.gather();
		async_ops_.pending_counter_.gather();
		disk_backed_block_map_.allocated_doubles_.gather();
		disk_backed_block_map_.blocks_to_disk_.gather();
//
		if (sip_mpi_attr_.is_company_master()){
			os << "\n\nServer statistics"<< std::endl << std::endl;
			std::ios saved_format(NULL);
			saved_format.copyfmt(os);
			os << "Server Utilization Summary (approximate percent time)" << std::endl;
			os << "directly processing ops,"<< std::setiosflags(std::ios::fixed) << std::setprecision(0) <<(handle_op_timer_.get_mean()/total_timer_.get_mean())*100  << std::endl;
			os << "handling async ops,"<< std::setiosflags(std::ios::fixed) << std::setprecision(0)  << (pending_timer_.get_mean()/total_timer_.get_mean())*100 << std::endl;
			os << "idle," << std::setiosflags(std::ios::fixed) << std::setprecision(0) << (idle_timer_.get_mean()/total_timer_.get_mean())*100 <<  std::endl;
			os.copyfmt(saved_format);
			os << std::endl;
			os << "Server op_timer_" << std::endl;
			//os << op_timer_ << std::endl;
			op_timer_.print_op_table_stats(os, sip_tables_);
			os << std::endl << "Server get_block_timer_" << std::endl;
			//os << get_block_timer_ << std::endl;
			get_block_timer_.print_op_table_stats(os, sip_tables_);
			os << std::endl << "total_timer_" << std::endl;
			os << total_timer_ ;
			os << std::endl << "handle_op_timer_" << std::endl;
			os << handle_op_timer_;
			os << std::endl << "idle_timer_" << std::endl;
			os << idle_timer_ ;
			os << std::endl << "pending_timer_" << std::endl;
			os << pending_timer_ ;
			os << std::endl << "num_ops_" << std::endl;
			os << num_ops_ ;
			os << std::endl << "async_ops_pending_" << std::endl;
			os << async_ops_.pending_counter_ ;
			os << std::endl << "allocated_doubles_" << std::endl;
			os << disk_backed_block_map_.allocated_doubles_ ;
			os << std::endl << "blocks_to_disk_" << std::endl;
			os << disk_backed_block_map_.blocks_to_disk_ ;
     		os << std::endl << std::flush;
		}

	}

	friend std::ostream& operator<<(std::ostream& os, const SIPServer& obj);


//	double scalar_value(int){ fail("scalar_value should not be invoked by a server"); return -.1;}
//	void set_scalar_value(int, double) { fail("set_scalar_value should not be invoked by a server");}
//    void set_contiguous_array(int, Block* ) {fail("set_contiguous_array should not be invoked by a server");}
//    Block* get_and_remove_contiguous_array(int) {fail("get_and_remove_contiguous_aray should not be invoked by a server"); return NULL;}







private:
    const SipTables &sip_tables_;
	const SIPMPIAttr & sip_mpi_attr_;
	const DataDistribution &data_distribution_;
	MPIScalarOpType mpi_type_;


	int pc_;

//	int last_seen_line_;
	int last_seen_worker_;

	/** maintains message, section number, etc for barrier.  This
	 * object's check_section_number_invariant should be invoked
	 * for each transaction.
	 */
	BarrierSupport state_;

	/**
	 * Set to true on receipt of an end program message.  Causes
	 * the run loop to terminate.
	 */
	bool terminated_;

	ServerPersistentArrayManager* persistent_array_manager_;
	DiskBackedBlockMap disk_backed_block_map_;
	PendingAsyncManager async_ops_;

//	/** Timers and counters */
	MPITimerList op_timer_;  //indexed by pc
	MPITimerList get_block_timer_; //indexed by pc
//  NoopTimerList<double> op_timer_;
//	NoopTimerList<double> get_block_timer_;
	MPITimer idle_timer_;
	MPITimer pending_timer_;
	MPITimer total_timer_;
	MPICounter num_ops_;
	MPITimer handle_op_timer_;

	/**
	 * Get
	 *
	 * invoked by server loop.
	 *
	 * Retrieves block_id and block size from the message and replies with the block
	 * The program fails if a block is requested that doesn't exist.
	 *
	 * @param mpi_source  requesting worker
	 * @param tag         tag, which includes message type, section number, and message number
	 *
	 *
	 */
	void handle_GET(int mpi_source, int tag);

	/**
	 * Put
	 *
	 * invoked by server loop.
	 *
	 * Creates the tag for the data message
	 * Receives the message and obtains the block_id and block size.
	 * Get the block, creating it if
	 *    it doesn't exist.
	 * Creates an async op to manage the receive
	 * Sends an ack to the source
	 *
	 * @param mpi_source
	 * @param put_tag
	 * @param put_data_tag
	 */
	void handle_PUT(int mpi_source, int put_tag, int transaction_number);


	/**
	 * put_accumulate
	 *
	 * invoked by server loop.
	 *
	 * Receives the message and obtains the block_id and block size.
	 * Gets the block to accumulate into, creating and initializing it if
	 *    it doesn't exist
	 * Creates an async op which manages the asynchronous receive which completes the transaction,
	 * including allocating a temp buffer, and performing the accumulate operation.
	 *
	 * @param [in] mpi_source
	 * @param [in] put_accumulate_tag
	 * @param [in] put_accumulate_data_tag
	 */
	void handle_PUT_ACCUMULATE(int mpi_source, int put_accumulate_tag, int put_accumulate_data_tag);

/**
 * put_initialize
 *
 * Receives the message and obtains block_id, and value from the message.
 * Initializes each element of the block to the given value.
 *
 * @param [in] mpi_source
 * @param [in] put_initialize_tag
 */
	void handle_PUT_INITIALIZE(int mpi_source, int put_initialize_tag);

	/**
	 * put_increment
	 *
	 * Receives the message and obtains block_id, and value from the message.
	 * Increments each element of the block by the given value.
	 *
	 * @param [in] mpi_source
	 * @param [in] put_increment_tag
	 */
	void handle_PUT_INCREMENT(int mpi_source, int put_increment_tag);


	/**
	 * put_scale
	 *
	 * Receives the message and obtains block_id, and value from the message.
	 * Multiplies each element of the block by the given value.
	 *
	 * @param [in] mpi_source
	 * @param [in] put_increment_tag
	 */
	void handle_PUT_SCALE(int mpi_source, int put_scale_tag);

	/**
	 * delete
	 *
	 * invoked by server loop.
	 *
	 * gets array_id from message and deletes all blocks
	 * belonging to that array.
	 *
	 * @param mpi_source
	 * @param tag
	 */
	void handle_DELETE(int mpi_source, int tag);

	/**
	 * end program
	 *
	 * invoked by server loop.
	 *
	 * causes server loop to exit.
	 *
	 * @param mpi_source
	 * @param tag
	 */
	void handle_END_PROGRAM(int mpi_source, int tag);

	/**
	 * set persistent
	 *
	 * invoked by server loop.
	 *
	 * Makes an upcall to the persistent_array_manager_ to
	 * mark the indicated array as persistent.  After server loop
	 * termination, the persistent array manager will move the
	 * marked arrays to its own data structures to preserver between
	 * SIAL programs
	 *
	 * @param mpi_source
	 * @param tag
	 */
	void handle_SET_PERSISTENT(int mpi_source, int tag);

	/**
	 * restore persistent
	 *
	 * invoked by server loop.
	 *
	 * Makes an upcall to the persistent_array_manager_ to
	 * restore the blocks of the indicated array from the
	 * persistent_array_manager to the block_manager_.
	 * The persistent_array_manager is responsible for
	 * updating the array_id.
	 *
	 * @param mpi_source
	 * @param tag
	 */
	void handle_RESTORE_PERSISTENT(int mpi_source, int tag);

	/**
	 * Utility method checks whether the message with the given MPI_Status object has the expected number of MPI_INT elements.
	 * If not, it is a fatal error.
	 *
	 * @param status
	 * @param expected_count
	 */
	void check_int_count(MPI_Status& status, int expected_count);

	/**
	 * Utility method checks whether the message with the given MPI_Status object has the expected number of MPI_DOUBLE elements.
	 * If not, it is a fatal error.
	 *
	 * @param status
	 * @param expected_count
	 */
	void check_double_count(const MPI_Status& status, int expected_count);


    friend ServerPersistentArrayManager;

	DISALLOW_COPY_AND_ASSIGN(SIPServer);
};

} /* namespace sip */

#endif /* SIP_SERVER_H_ */
