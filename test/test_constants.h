/*
 * test_constants.h
 *
 *  Created on: Aug 1, 2014
 *      Author: basbas
 */

#ifndef TEST_CONSTANTS_H_
#define TEST_CONSTANTS_H_

#include "sip_mpi_attr.h"

extern const std::string dir_name;
extern const std::string qm_dir_name;
extern const std::string expected_output_dir_name;

extern sip::SIPMPIAttr *attr;

void barrier();



#endif /* TEST_CONSTANTS_H_ */
