/*
 * sip_server.h
 *
 *  Created on: Jan 14, 2014
 *      Author: njindal
 */

#ifndef SIP_SERVER_H_
#define SIP_SERVER_H_

#include "sip_tables.h"
#include "server_block.h"
#include "data_distribution.h"
#include "barrier_support.h"
#include "server_persistent_array_manager.h"
#include "disk_backed_block_map.h"
#include "server_interpreter.h"
#include "server_timer.h"

namespace sip {

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
 * RESTORE_PERSISTENT:
 * SET_PERSISTENT:
 * DELETE:
 *
 * TODO:  In the current implementation, on receipt of a PUT or PUT_ACCUMULATE, the server waits for the
 * following PUT_DATA or PUT_ACCUMULATE_DATA message.
 * We need to investigate the performance implications.
 */

class SIPServer {

public:
	SIPServer(SipTables&, DataDistribution&, SIPMPIAttr&, ServerPersistentArrayManager*, ServerTimer&);
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

	/**
	 * Called by persistent_array_manager. Delegates to block_map_.
	 */
	IdBlockMap<ServerBlock>::PerArrayMap* get_and_remove_per_array_map(int array_id){
			return disk_backed_block_map_.get_and_remove_per_array_map(array_id);
	}

	/**
	 * Called by persistent_array_manager during restore.  elegates
	 * to block_map_
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

	friend std::ostream& operator<<(std::ostream& os, const SIPServer& obj);

	double scalar_value(int){ fail("scalar_value should not be invoked by a server"); return -.1;}
	void set_scalar_value(int, double) { fail("set_scalar_value should not be invoked by a server");}
    void set_contiguous_array(int, Block* ) {fail("set_contiguous_array should not be invoked by a server");}
    Block* get_and_remove_contiguous_array(int) {fail("get_and_remove_contiguous_aray should not be invoked by a server"); return NULL;}


    /**
     * Gets the last seen sialx line from which a worker
     * sent a message. Line 0 is seen for PROGRAM_END.
     * @return
     */
    int last_seen_line();

private:
    const SipTables &sip_tables_;
	const SIPMPIAttr & sip_mpi_attr_;
	const DataDistribution &data_distribution_;

	ServerTimer& server_timer_;

	int last_seen_line_;
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

	/**
	 * Interface to disk backed block manager.
	 */
	DiskBackedBlockMap disk_backed_block_map_;

    ServerInterpreter server_interpreter_;
	
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
	void handle_GET(int mpi_source, int tag, int mpi_count);

	/**
	 * Put
	 *
	 * invoked by server loop.
	 *
	 * Receives the message and obtains the block_id and block size.
	 * Get the block, creating it if
	 *    it doesn't exist.
	 * Posts recvieve with tag put_dat_tag
	 * Sends an ack to the source
	 *
	 * @param mpi_source
	 * @param put_tag
	 * @param put_data_tag
	 */
	void handle_PUT(int mpi_source, int put_tag, int put_data_tag);


	/**
	 * put_accumulate
	 *
	 * invoked by server loop.
	 *
	 * Receives the message and obtains the block_id and block size.
	 * Creates a temp buffer to receive the data, and posts receive
	 *    for message with the put_accumulate_data_tag.
	 *    This tag should have the same message number and section number as
	 *    the put_accumulate_tag.
	 * Get the block to accumulate into, creating and initializing it if
	 *    it doesn't exist.
	 * Accumulates received data into block
	 * Sends an ack to the source
	 *
	 * @param [in] mpi_source
	 * @param [in] put_accumulate_tag
	 * @param [in] put_accumulate_data_tag
	 */
	void handle_PUT_ACCUMULATE(int mpi_source, int put_accumulate_tag, int put_accumulate_data_tag, int mpi_count);

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
	void check_double_count(MPI_Status& status, int expected_count);


	void handle_section_number_change(bool section_number_changed);


    friend ServerPersistentArrayManager;

};

} /* namespace sip */

#endif /* SIP_SERVER_H_ */