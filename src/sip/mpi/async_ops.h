/*
 * async_state.h
 *
 *  Created on: Jun 22, 2015
 *      Author: basbas
 */

#ifndef ASYNC_STATE_H_
#define ASYNC_STATE_H_

#include <list>
#include <mpi.h>
#include "sip.h"
#include "sip_mpi_constants.h"
#include "barrier_support.h"
#include "sip_mpi_utils.h"

namespace sip {

class ServerBlock;

/**
 * Base class for pending asynchronous operations on a block
 *
 * Objects are owned by a data structure belonging to the block and constructed when
 * inserted and destructed when removed.  These objects are not copyable--there should be
 * a single instance per operation.
 *
 * Coding standards related to asynchronous operations:
 * The constructor of each class should perform the asynchronous send or post the asynchronous receive
 * that derived objects of this class are handling.  The do_handle method should perform tasks necessary
 * to complete the operation after it has been enabled.
 *
 * Each subclass should maintain the following properties:
 * 1.  The constructor should perform the communication (async send, or receive) that this object is
 * 		managing the wait for
 * 2.  Closure:  the methods should be callable from anywhere
 * 3.  Nonblocking:  An enabled operations should be able to be executed to completion without blocking.
 *
 * This class is a wrapper for these operations which ensures that calls are idempotent.
 *
 *
 */
class AsyncBase {
public:
	enum AsyncStatus {
		WAITING = 1, READY = 2, DONE = 3,
	};

	AsyncBase(int pc) :
			async_status_(WAITING), pc_(pc) {
	}

	virtual ~AsyncBase() {
		check(async_status_ == DONE,
				"attempting to deconstruct unhandled request");
	}

	/** Tests the status of this operation
	 *
	 * @return  true if the operation is enabled or completed, and false otherwise
	 */
	bool test() {
		if (async_status_ != WAITING)
			return true;
		if (do_test()) {
			async_status_ = READY;
			return true;
		}
		return false;
	}

	/**
	 * Waits for this operation to to enabled or completed.
	 * On return, async_status_ is READY or DONE.
	 */
	void wait() {
		if (async_status_ != WAITING)
			return;
		do_wait();
		async_status_ = READY;
	}


	/**  Attempts to handle this operation.
	 *
	 * If the operation cannot be performed, then async_status remains WAITING and the method returns false
	 * If the operation can be performed, it is, asynch_status is set to DONE, and the method returns true
	 *
	 * @return  true if the operation has been completed and async_status_ set to DONE
	 */
	bool try_handle() {
		if (async_status_ == READY || (async_status_==WAITING && test())) {
			do_handle();  //this should always succeed if called when enabled
			async_status_ = DONE;
			return true;
		}
		return async_status_== DONE;
	}

	friend std::ostream& operator<<(std::ostream& os, const AsyncBase &obj) {
		os << obj.to_string();
		return os;
	}


protected:
	//subclasses should invoke AsyncStatus::to_string() in overriding methods
	virtual std::string to_string() const {
		std::stringstream ss;
		ss << "async_status_=" << async_status_;
		ss << " pc_=" << pc_ << " ";
		return ss.str();
	}

private:
	AsyncStatus async_status_; //initially WAITING, must be DONE before destruction
	int pc_;  //index into optable of the sialx instruction that generated this operation

	//precondition:  async_state == WAITNG
	//postcondition: returned value == "operation is enabled"
	virtual bool do_test()=0;

	//precondition:   async_state == WAITING
	//postcondition:  "operation is enabled"
	virtual void do_wait()=0;

	//precondition:  async_state == READY
	//postcondition:  op completed
	virtual void do_handle()=0;



	DISALLOW_COPY_AND_ASSIGN (AsyncBase);
};

/** Represents asynchronous send of block in response to get operation
 * Once the send operation is complete, no additional "handling" needs to be done.
 *
 * Constructor posts Isend
 *
 */
class GetAsync: public AsyncBase {
public:
	GetAsync(int mpi_source, int get_tag, ServerBlock* block, int pc);
	virtual ~GetAsync() {
	}

private:
	MPI_Request mpi_request_;

	bool do_test() {
		int flag = 0;
		MPI_Test(&mpi_request_, &flag, MPI_STATUS_IGNORE);
		return flag;
	}

	void do_wait() {
		MPI_Wait(&mpi_request_, MPI_STATUS_IGNORE);
	}

	void do_handle() {  //nothing to do here.
	}

	std::string to_string() const{
		std::stringstream ss;
		ss << "GetAsync";
		ss << AsyncBase::to_string();
		return ss.str();
	}

};

/** Represents asynchronous put_accumulate operation
 *
 * Will be created in response to put_accumulate message to handle the second message containing
 * the data and to perform the accumulate operation.  Instances of this class own
 * the temporary buffer used to receive the data.
 *
 * Constructor:
 *             allocates temporary buffer,
 *             posts Irecv for block data.
 *
 * do_handle: sends ack for data message to source
 *           performs the accumulate operation of temp data into block
 *
 * Destructor: deletes the temp data buffer.
 *
 *Note The constructor does not send the "reply" message to the source to let it know that
 *Note the data message can be sent.  This should be done AFTER this object is constructed
 *Note in order to ensure that the Irecv has been posted (and thus ensuring that the message
 *Note bypasses MPI buffers
 *
 *TODO:  reuse the temp buffers?
 *
 */
class PutAccumulateDataAsync: public AsyncBase {
public:
	PutAccumulateDataAsync(int mpi_source, int put_accumulate_data_tag,
			ServerBlock* block,  int pc);
	virtual ~PutAccumulateDataAsync() {
		check(temp_ != NULL,
				"destructor of PutAccumulateDataAsync invoked with null data_");
		delete[] temp_;
	}
private:
	ServerBlock* block_;
	int mpi_source_;
	int tag_;
	MPI_Request mpi_request_;
	double* temp_;  //buffer to recieve data.  created in constructor, deleted in destructor

	bool do_test();
	void do_wait();
	void do_handle();
	virtual std::string to_string() const;
};






/** Represents asynchronous put data operation
 *
 * Will be created in response to put message.
 * This class assumes that there is at most one unhandled put message per block at any time.
 *
 * This is guaranteed because the block should have been obtained by a call
 * to get_block_for_writing,  This method should call the blocks CommunicationState.wait method.
 *
 * Constructor: sends reply to source,
 *             posts Irecv for block data
 *             The receive buffer is the block's data array, which should exist already
 *
 * do_handle: sends ack for data message to source
 *
 *Note The constructor does not send the "reply" message to the source to let it know that
 *Note the data message can be sent.  This should be done AFTER this object is constructed
 *Note order to ensure that the Irecv has been posted (and thus ensuring that the message
 *Note bypasses MPI buffers
 *
 */
class PutDataAsync: public AsyncBase {

public:
	PutDataAsync(int mpi_source, int put_data_tag, ServerBlock* block, int pc);
	virtual ~PutDataAsync() {}

private:
	ServerBlock* block_;
	int mpi_source_;
	int tag_;
	MPI_Request mpi_request_;

	bool do_test();
	void do_wait();
	void do_handle();

	std::string to_string() const;

};







/** This class manages the pending asynchronous communication operations on a block.  It contains
 * a list of pending operations on a block, and provides a set of factory methods
 * to create and save AsyncOp operations.
 *
 * Currently, get_block_for_reading and get_block_for_writing stop and wait for
 * pending operations to be completed before returning. get_block_for_accumulate,
 * on the other hand does not wait, but adds its async_op to the list
 * and returns.
 *
 * All asynchronous operations are handled in the order they are submitted to this list.
 *
 * Each asynchronous operation, OP,  should have an add_OP method in this class which creates
 * an instance of an appropriate subclass of AsyncBase and adds it to the pending list.
 */
class BlockAsyncManager {
public:
	BlockAsyncManager(){}
	~BlockAsyncManager(){
		check(pending_.empty(), "deleting block with pending async ops");
	}
	void add_put_accumulate_data_request(int mpi_source,
			int put_accumulate_data_tag, ServerBlock* block,
			int pc);

	void add_put_data_request(int mpi_source, int put_data_tag,
			ServerBlock* block, int pc);

	void add_get_reply(int mpi_source, int get_tag, ServerBlock *, int pc);


	/**
	 *
	 * @return true if this block has pending operations
	 */
	bool has_pending() {
		return !pending_.empty();
	}

	/**
	 * Attempts to handle all pending items in the list, in order.  If an item is found that
	 * is not enabled, false is returned. Otherwise, the list will be empty and true is returned.
	 * On a return value of true, the caller should delete this block from its set of blocks with pending
	 * items.
	 *
	 * @return  false if the pending list for the block is not empty.
	 *
	 */
	bool try_handle_all() {
		std::list<AsyncBase*>::iterator it = pending_.begin();
		while(it != pending_.end()){
			bool res = (*it)->try_handle();
			if (res) {
				delete *it;
				it = pending_.erase(it);
			}
			else return false;
		}
		return true;
	}


	/** Handles all pending asyncs on this block, waiting for each one in turn to be enabled
	 * */

	void wait_all() {
		if (!has_pending()) {
			return;
		}
		std::list<AsyncBase*>::iterator it = pending_.begin();
		while (it != pending_.end()) {
			(*it)->wait();
			bool res = (*it)->try_handle();
			check(res, "in BlockAsyncManager::wait--handle ready op failed");
			delete *it;
			it = pending_.erase(it);
		}
	}

private:
	std::list<AsyncBase*> pending_;
	DISALLOW_COPY_AND_ASSIGN (BlockAsyncManager);
};

} /* namespace sip */

#endif /* ASYNC_STATE_H_ */
