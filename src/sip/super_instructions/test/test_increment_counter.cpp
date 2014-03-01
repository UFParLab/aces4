/*
 * test_increment_counter.cpp
 *
 * Collects a counter which a test can then check the value of.
 *
 *  Created on: Oct 25, 2013
 *      Author: njindal
 */

#include <iostream>
#include "../sip_interface.h"

void test_increment_counter(int& array_slot, int& rank, int* index_values, int& size, int* extents,  double* data, int& ierr){
	static int counter = 0;
	data[0] = ++counter;
	ierr = 0;
}
