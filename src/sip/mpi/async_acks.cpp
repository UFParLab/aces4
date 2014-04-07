/*
 * async_acks.cpp
 *
 */

#include "async_acks.h"
#include "sip_mpi_utils.h"

namespace sip {

AsyncAcks::AsyncAcks() :
		next_slot_(0) {
	std::fill(posted_async_, posted_async_ + MAX_POSTED_ASYNC,
			MPI_REQUEST_NULL);
}

AsyncAcks::~AsyncAcks() {
}

void AsyncAcks::expect_ack_from(int from_rank, int tag) {
	MPI_Request request;
	SIPMPIUtils::check_err(
			MPI_Irecv(0, 0, MPI_INT, from_rank, tag, MPI_COMM_WORLD,
					&request));
	add_request(request);
}

void AsyncAcks::wait_all() {
	if (next_slot_==0)
		return;

	SIPMPIUtils::check_err(
			MPI_Waitall(next_slot_, posted_async_, MPI_STATUSES_IGNORE));

	//at this point, all outstanding requests have been satisfied and their corresponding
	//value in array set to MPI_REQUEST_NULL by MPI
	next_slot_ = 0;
}


void AsyncAcks::cleanup() {
	//if no requests, then nothing to do
	if (next_slot_ == 0)
		return;

	//check for satisfied requests
	int outcount;
	int array_of_indices[next_slot_];
	SIPMPIUtils::check_err(
			MPI_Testsome(next_slot_, posted_async_, &outcount, array_of_indices,
					MPI_STATUSES_IGNORE));
	//if none, return
	if (outcount == 0 || outcount == MPI_UNDEFINED)
		return;

	//otherwise, compact the array
	int array_of_indices_index, to_replace, top;
	array_of_indices_index = 0;
	for (array_of_indices_index = 0; array_of_indices_index < outcount;
			array_of_indices_index++) {
		top = next_slot_ - 1;
		to_replace = array_of_indices[array_of_indices_index];
		if (top > to_replace) {
			posted_async_[to_replace] = posted_async_[top];
		}
		next_slot_--;
	}

}

void AsyncAcks::wait_and_cleanup() {
	//check for satisfied requests
	int outcount;
	int array_of_indices[next_slot_];
	SIPMPIUtils::check_err(
			MPI_Waitsome(next_slot_, posted_async_, &outcount, array_of_indices,
					MPI_STATUSES_IGNORE));
	//if none, return
	if (outcount == 0 || outcount == MPI_UNDEFINED)
		return;

	//otherwise, compact the array
	int array_of_indices_index, to_replace, top;
	array_of_indices_index = 0;
	for (array_of_indices_index = 0; array_of_indices_index < outcount;
			array_of_indices_index++) {
		top = next_slot_ - 1;
		to_replace = array_of_indices[array_of_indices_index];
		if (top > to_replace) {
			posted_async_[to_replace] = posted_async_[top];
		}
		next_slot_--;
	}
}

void AsyncAcks::add_request(MPI_Request request) {
	if(next_slot_ == MAX_POSTED_ASYNC) {
		cleanup();
		if (next_slot_ == MAX_POSTED_ASYNC){
			check_and_warn(false,"AsyncAcks posted_async_ array is full of unsatisfied requests");
			wait_and_cleanup();
		}
	}
	posted_async_[next_slot_++] = request;
}


} /* namespace sip */
