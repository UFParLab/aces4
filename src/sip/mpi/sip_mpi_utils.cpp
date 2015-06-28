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
//	std::cout << "  in SIPMPIUtils::set_error_handler" << std::endl << std::flush;
	MPI_Errhandler_set(MPI_COMM_WORLD, MPI_ERRORS_RETURN);
	SIPMPIAttr & mpi_attr = SIPMPIAttr::get_instance();
//	std::cout << "Rank " << mpi_attr.global_rank()  << "  in SIPMPIUtils::set_error_handler:  mpi_attr.company_communicator()!=NULL " << (mpi_attr.company_communicator() != NULL) << std::endl << std::flush;
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
	    fprintf(stderr, "Error %d:%s\n", eclass, estring);
	    fflush(stdout);
        fail("MPI Error !\n");
	}
}
void SIPMPIUtils::check_err(int err, int line, char * file){

	if (err == MPI_SUCCESS){
		return;
	} else {
		int len, eclass;
	    char estring[MPI_MAX_ERROR_STRING];
	    MPI_Error_class(err, &eclass);
	    MPI_Error_string(err, estring, &len);
	    fprintf(stderr, "Error %d: %s at line %d in file %s\n", eclass, estring, line, file);
	    fflush(stdout);
        fail("MPI Error !\n");
	}
}


} /* namespace sip */
