/**
 * Special super instruction
 *
 * swaps the contents of two blocks. The blocks need to have the same number of elements.
 *
 *  Created on: Aug 25, 2013
 *      Author: Beverly Sanders
 */


#include <iostream>
#include "sip_interface.h"

void swap_blocks(int& array_slot_0, int& rank_0, int * index_values_0, int& size_0, int * extents_0, double * data_0,
        int& array_slot_1, int& rank_1, int * index_values_1, int& size_1, int * extents_1, double * data_1, int& ierr){

        ierr = 0;
        if (size_0 != size_1){
        	ierr = 1;
        	return;
        }
        for (int i = 0; i < size_0; ++i){
        	double tmp = data_0[i];
        	data_0[i] = data_1[i];
        	data_1[i] = tmp;
        }
        return;
}
