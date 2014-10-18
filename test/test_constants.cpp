/*
 * test_constants.cpp
 *
 *  Created on: Aug 15, 2014
 *      Author: njindal
 */

#include "test_constants.h"
#include "sip_mpi_attr.h"
#include "sip_mpi_utils.h"

const std::string dir_name("src/sialx/test/");
const std::string qm_dir_name("src/sialx/qm/");
const std::string expected_output_dir_name("../test/expected_output/");

sip::SIPMPIAttr *attr = NULL;

#ifdef HAVE_MPI
    void barrier() {
    	sip::SIPMPIUtils::check_err (MPI_Barrier(MPI_COMM_WORLD));
    }
#else
	void barrier(){}
#endif


void check_expected_datasizes() {
	// Check sizes of data types.
	// In the MPI version, the TAG is used to communicate information
	// The various bits needed to send information to other nodes
	// sums up to 32.
	sip::check(sizeof(int) >= 4, "Size of integer should be 4 bytes or more");
	sip::check(sizeof(double) >= 8, "Size of double should be 8 bytes or more");
	sip::check(sizeof(long long) >= 8,
			"Size of long long should be 8 bytes or more");
}
