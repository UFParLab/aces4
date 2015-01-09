/**
 * Special super instruction
 *
 * get the first element of the block in the first parameter and returns the value in the scalar parameter.
 * Requires that the size of the second parameter be 1.
 *
 * SIAL signature:  special get_first_block_element rw
 *
 *  Created on: Dec. 3, 2014
 *      Author: Beverly Sanders
 */


#include <iostream>
#include "sip_interface.h"

void get_first_block_element(int& array_slot_0, int& rank_0, int * index_values_0, int& size_0, int * extents_0, double * data_0,
        int& array_slot_1, int& rank_1, int * index_values_1, int& size_1, int * extents_1, double * data_1, int& ierr){

        ierr = 0;
//        if (size_1 != 1){
//        	ierr = 1;
//        	std::cerr << "The size of the"
//        	return;
//        }
        data_1[0] = data_0[0];
        return;
}
