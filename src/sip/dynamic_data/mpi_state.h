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
	void wait(int expected_count){
		MPI_Status status;
		MPI_Wait(&mpi_request_, &status);
		int received_count;
		MPI_Get_count(&status, MPI_DOUBLE, &received_count);
			check(received_count == expected_count,
					"message's double count different than expected");
	}

private:


	friend class SialOpsParallel;
	MPI_Request mpi_request_;
	DISALLOW_COPY_AND_ASSIGN(MPIState);
};


} /* namespace sip */

#endif /* MPI_STATE_H_ */
