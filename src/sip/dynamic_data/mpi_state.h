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
	}


	void wait(){
		MPI_Wait(&mpi_request_, MPI_STATUS_IGNORE);
	}

	bool test(){
		int flag=0;
		MPI_Test(&mpi_request_, &flag, MPI_STATUS_IGNORE);
		return flag;
	}

//	void wait(int expected_count){
//		MPI_Status status;
//		MPI_Wait(&mpi_request_, &status);
//		int received_count;
//		MPI_Get_count(&status, MPI_DOUBLE, &received_count);
//			check(received_count == expected_count,
//					"message's double count different than expected");
//	}


private:


	MPI_Request mpi_request_;

	friend class Block;
	DISALLOW_COPY_AND_ASSIGN(MPIState);
};


} /* namespace sip */

#endif /* MPI_STATE_H_ */
