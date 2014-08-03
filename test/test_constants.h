/*
 * test_constants.h
 *
 *  Created on: Aug 1, 2014
 *      Author: basbas
 */

#ifndef TEST_CONSTANTS_H_
#define TEST_CONSTANTS_H_

#include "sip_mpi_attr.h"




const std::string dir_name("src/sialx/test/");
const std::string qm_dir_name("src/sialx/qm/");
const std::string expected_output_dir_name("../test/expected_output/");

static sip::SIPMPIAttr *attr;

#ifdef HAVE_MPI
    void barrier() {
//    	std::cout<< "calling barrier from rank "<< attr->global_rank() << std::endl << std::flush;
    			sip::SIPMPIUtils::check_err (MPI_Barrier(MPI_COMM_WORLD));
//    	MPI_Barrier(MPI_COMM_WORLD);
//        std::cout<< "passing barrier from rank "<< attr->global_rank() << std::endl << std::flush;
    }
#else
	void barrier(){}
#endif



#endif /* TEST_CONSTANTS_H_ */
