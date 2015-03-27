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
 * The cleanup method may be called at any time to remove requests for acks that
 * have already arrived from the array of pending requests.  It will be
 * called if the array of pending requests becomes full.
 */
class AsyncAcks {
public:
	AsyncAcks();
   ~AsyncAcks();

	//TODO  figure out a good size for this (probably not as big as 2^16)
	static const size_t MAX_POSTED_ASYNC = 2048;

	/** called by sial_ops implementations that expect an ack from a server
	 *
	 * @param from_rank  server rank
	 * @param tag        tag from msg being acked
	 */
	void expect_ack_from(int from_rank, int tag);

	/**
	 * returns after the expected ack has been received.
	 * called during program termination.
	 *
	 * @param from_rank  server rank
	 * @param tag        tag from msg being acked
	 */
	void expect_sync_ack_from(int from_rank, int tag);

	/**
	 * called by sip_barrier to wait for all acks to be acknowledged.
	 * After return, this worker has no more message in transit
	 */
	void wait_all();

	/**
	 * Handles and receives complete acks.  It does not wait,
	 * just handles acks that have already arrived.
	 */
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
	 * satisfied.  This may be called by add_request to wait for a slot
	 * in the posted_async_ array.  A warning is printed since the situation
	 * where the entire poaster_async_ list is full of pending requests is
	 * unusual and should be investigated.
	 */
	void wait_and_cleanup();

	/**
	 * removes completed requests from the posted_async_ array.
	 * Called by cleanup and wait_and_cleanup
	 *
	 * @param outcount  the number of completed requests (obtained from
	 *                      MPI_Testsome or MPI_Waitsome
	 * @param array_of_indices  array containing outcount indices into
	 *                        posted_async_ array that are completed.
	 *                        This was also returned from MPI_Testsome
	 *                        or MPI_Waitsome
	 */
	void remove_completed_requests(int outcount,
			int array_of_indices[]);

	DISALLOW_COPY_AND_ASSIGN(AsyncAcks);

	friend std::ostream& operator<<(std::ostream&, const AsyncAcks &);

};

} /* namespace sip */

#endif /* ASYNC_ACKS_H_ */
