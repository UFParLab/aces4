/*
 * async_acks.h
 *
 *  Created on: Apr 5, 2014
 *      Author: basbas
 */

#ifndef ASYNC_ACKS_H_
#define ASYNC_ACKS_H_

#include <mpi.h>
#include "aces_defs.h"
#include "sip.h"

namespace sip {

/** This class encapsulates the handling of acks from servers that
 * are required for the current sip_barrier implementation.
 *
 * After a worker sends a message that should be acked, it invokes
 * expect_ack_from with the rank of the server and the tag.
 * This posts an asynchronous receive for the ack and saves the
 * request internally.
 *
 * At a barrier, the wait_all method returns when all of the acks have
 * been received.
 *
 * The cleanup method may be called to remove requests for acks that
 * have already arrived from the array of pending requests.
 */
class AsyncAcks {
public:
	AsyncAcks();
   ~AsyncAcks();

	//TODO  this is excessive
	static const size_t MAX_POSTED_ASYNC = 65536;  // = 2^16. 16 bits for message number in mpi tag (sip_mpi_utils).

	void expect_ack_from(int from_rank, int tag);

	void wait_all();

	void cleanup();

private:
	MPI_Request posted_async_[MAX_POSTED_ASYNC]; // posted asynchronous receives/sends
	size_t next_slot_; // number of posted asynchronous receives/sends

	/**
	 * called by expect_ack_from to add the given request to the list
	 * of pending requests.  If the list is full, this will attempt to
	 * create space, if necessary, it will block until space is available.
	 *
	 * @param request
	 */
	void add_request(MPI_Request request);


	/**
	 * like cleanup, but blocks until at least one pending request has been
	 * satisfied.  This may be caleed by add_request to wait for a slot
	 * in the posted_async_ array.  A warning is printed since the situation
	 * where the entire poaster_async_ list is full of pending requests is
	 * unusual and should be investigated.
	 */
	void wait_and_cleanup();

	DISALLOW_COPY_AND_ASSIGN(AsyncAcks);


};

} /* namespace sip */

#endif /* ASYNC_ACKS_H_ */
