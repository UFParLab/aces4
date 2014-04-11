/*
 * sip_mpi_utils.cpp
 *
 *  Created on: Jan 17, 2014
 *      Author: njindal
 */

#include <sip_mpi_utils.h>

#include "mpi.h"
#include "sip_mpi_attr.h"

#include <memory>
#include <cstdio>
#include <cstring>

namespace sip {


void SIPMPIUtils::set_error_handler(){
	MPI_Errhandler_set(MPI_COMM_WORLD, MPI_ERRORS_RETURN);
	SIPMPIAttr & mpi_attr = SIPMPIAttr::get_instance();
	MPI_Errhandler_set(mpi_attr.company_communicator(), MPI_ERRORS_RETURN);
}

void SIPMPIUtils::check_err(int err){

	if (err == MPI_SUCCESS){
		return;
	} else {
		int len, eclass;
	    char estring[MPI_MAX_ERROR_STRING];
	    MPI_Error_class(err, &eclass);
	    MPI_Error_string(err, estring, &len);
	    printf("Error %d: %s\n", eclass, estring);
	    fflush(stdout);
        fail("MPI Error !\n");
	}
}

} /* namespace sip */
