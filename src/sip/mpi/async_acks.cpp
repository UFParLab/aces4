/*
 * async_acks.cpp
 *
 */

#include "async_acks.h"
#include "sip_mpi_utils.h"
#include "barrier_support.h"

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
			MPI_Irecv(0, 0, MPI_INT, from_rank, tag, MPI_COMM_WORLD, &request));
	add_request(request);
//	cleanup();
}

void AsyncAcks::expect_sync_ack_from(int from_rank, int tag) {
	SIPMPIUtils::check_err(
			MPI_Recv(0, 0, MPI_INT, from_rank, tag, MPI_COMM_WORLD,
					MPI_STATUS_IGNORE));
}

void AsyncAcks::wait_all() {
	if (next_slot_ == 0)
		return;
	SIPMPIUtils::check_err(
			MPI_Waitall(next_slot_, posted_async_, MPI_STATUSES_IGNORE));
//	std::cout << " in wait_all, up to " << next_slot_ << " acks released" << std::endl << std::flush;
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

//	std::cout << " in cleanup, " << outcount << " acks released" << std::endl << std::flush;
	remove_completed_requests(outcount, array_of_indices);
}

void AsyncAcks::remove_completed_requests(int outcount,
		int array_of_indices[]) {
	if (outcount == 0)
		return;
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
	//if no requests, then nothing to do
	if (next_slot_ == 0)
		return;

	//check for satisfied requests
	int outcount;
	int array_of_indices[next_slot_];
	SIPMPIUtils::check_err(
			MPI_Waitsome(next_slot_, posted_async_, &outcount, array_of_indices,
					MPI_STATUSES_IGNORE));
//	std::cout << " in wait_and_cleanup, "<< outcount << "  acks released" << std::endl << std::flush;

	remove_completed_requests(outcount, array_of_indices);
}

void AsyncAcks::add_request(MPI_Request request) {
	if (next_slot_ == MAX_POSTED_ASYNC) {
//		std::cout << "AsyncAcks posted_async_ array is full. SIAL line number "
//				<< current_line() << std::endl << std::flush
		wait_and_cleanup();
	}
	posted_async_[next_slot_++] = request;
}

std::ostream& operator<<(std::ostream& os, const AsyncAcks& acks) {
	os << "next_slot_=" << acks.next_slot_ << std::endl;
	for (int i = 0; i < acks.next_slot_; ++i) {
		MPI_Request r = acks.posted_async_[i];
		if (r == MPI_REQUEST_NULL) {
			os << "__,";
		} else {
			os << r << ',';
		}
	}
	os << std::endl;

	return os;
}
} /* namespace sip */
