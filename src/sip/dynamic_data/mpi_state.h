/*
 * mpi_state.h
 *
 *  Created on: Apr 11, 2014
 *      Author: basbas
 */

#ifndef MPI_STATE_H_
#define MPI_STATE_H_

#include <mpi.h>
namespace sip {



class MPIState{
public:
	MPIState():mpi_request_(MPI_REQUEST_NULL){
	}
	~MPIState(){
		if (!check_and_warn(!pending(), "deleting block with pending request")){
			//the block is pending, and we have raised a warning
			wait();
		}
	}
	bool pending(){
		return mpi_request_ != MPI_REQUEST_NULL;
	}
	void wait(){
		MPI_Wait(&mpi_request_, MPI_STATUS_IGNORE);
	}

	bool test(){
		int flag=0;
		MPI_Test(&mpi_request_, &flag, MPI_STATUS_IGNORE);
		return flag;
	}

	void wait(int expected_count){
		MPI_Status status;
		MPI_Wait(&mpi_request_, &status);
		int received_count;
		MPI_Get_count(&status, MPI_DOUBLE, &received_count);
			check(received_count == expected_count,
					"message's double count different than expected");
	}
	void set_request(MPI_Request mpi_request){
		check_and_warn(!pending(), "setting mpi_request for pending block"); //this should never happen.  Wait should occur in the get_block_for_* method.
		mpi_request_= mpi_request;
	}

private:


//	friend class SialOpsParallel;
	MPI_Request mpi_request_;

	friend class Block;
	DISALLOW_COPY_AND_ASSIGN(MPIState);
};


} /* namespace sip */

#endif /* MPI_STATE_H_ */
