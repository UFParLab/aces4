/*
 * test_constants.cpp
 *
 *  Created on: Aug 15, 2014
 *      Author: njindal
 */

#include "test_constants.h"
#include "sip_mpi_attr.h"
#ifdef HAVE_MPI
#include "sip_mpi_utils.h"
#endif

//const std::string dir_name("src/sialx/test/");
//const std::string qm_dir_name("src/sialx/qm/");
const std::string dir_name("src/sialx/");
const std::string qm_dir_name("src/sialx/");
const std::string expected_output_dir_name("./");

sip::SIPMPIAttr *attr = NULL;

#ifdef HAVE_MPI
    void barrier() {
    	sip::SIPMPIUtils::check_err (MPI_Barrier(MPI_COMM_WORLD));
    }
#else
	void barrier(){}
#endif
