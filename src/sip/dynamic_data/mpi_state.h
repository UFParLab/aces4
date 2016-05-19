/*
 * mpi_state.h
 *
 *  Created on: Apr 11, 2014
 *      Author: basbas
 */

#ifndef MPI_STATE_H_
#define MPI_STATE_H_
#include "config.h"
#include "sip.h"
#include <iostream>
#include <sstream>
#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <unistd.h>
#include "sip_interface.h"
#include "job_control.h"

#include <mpi.h>
#include <execinfo.h>

namespace sip {

    extern bool called_finalize;

class SialOpsParallel;
class SIPServer;

class MPIState{
public:
	MPIState():mpi_request_(MPI_REQUEST_NULL){}

	~MPIState(){}

    void wait(){
        if (mpi_request_ != MPI_REQUEST_NULL)
    		MPI_Wait(&mpi_request_, MPI_STATUS_IGNORE);
	}

	bool test(){
		int flag=0;
		MPI_Test(&mpi_request_, &flag, MPI_STATUS_IGNORE);
		return flag;
	}

//	// currently, it should never happen that a block has more than one outstanding request
//	// This may change if workers send put_data messages asynchronously in another thread.
//	// For the time being, do nothing and return false if request is not null.
//    MPI_Request* set_request(){
//    	if(mpi_request_!= MPI_REQUEST_NULL) {check_and_warn(false, "replacing pending MPI request");}
//    	return &mpi_request_;
//    }
private:


	friend class SialOpsParallel;
    friend class SIPServer;
	MPI_Request mpi_request_;

	friend class Block;
	friend class ServerBlock;
	DISALLOW_COPY_AND_ASSIGN(MPIState);
};


} /* namespace sip */

#endif /* MPI_STATE_H_ */
