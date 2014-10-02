/*
 * get_my_rank.cpp
 *
 *  Created on: Oct 19, 2013
 *      Author: basbas
 */


/**
 * get_my_rank
 *
 * special get_my_rank w
 *
 * Gets MPI rank.  If not using MPI, returns 0.
 *
 *  Created on: Oc 19, 2013
 *      Author: Beverly Sanders
 */


#include <iostream>
#include "config.h"
#include "sip_interface.h"

#ifdef HAVE_MPI
#include "sip_mpi_attr.h"
#endif

void get_my_rank(int& array_slot, int& rank, int* index_values, int& size, int* extents,  double* data, int& ierr){
#ifdef HAVE_MPI
	data[0] = sip::SIPMPIAttr::get_instance().company_rank();
	ierr = 0;
#else
	data[0] = 0;
	ierr = 0;
#endif
}


