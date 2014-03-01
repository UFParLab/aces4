/**
 * Special super instruction
 *
 * Prints the first MAX_TO_PRINT elements of a block in rows of size OUTPUT_ROW_SIZE
 *
 * Values are printed in the order they are stored in memory.
 *
 *  Created on: Aug 25, 2013
 *      Author: Beverly Sanders
 */


#include <iostream>
#include "sip_interface.h"

void print_block(int& array_slot, int& rank, int* index_values, int& size, int* extents,  double* data, int& ierr){
	int MAX_TO_PRINT = 1024;
	int OUTPUT_ROW_SIZE = extents[0];

	std::cout << "printing " << (size < MAX_TO_PRINT?size:MAX_TO_PRINT) << " elements of block " << sip::array_name_value(array_slot)<< '[';
			for (int i = 0; i < rank; ++i){
				std::cout << (i == 0?"":",") << index_values[i];
				std::cout << "(" << extents[i] << ")";
			}
	std::cout << "], size="<< size ;
	int i;
    for (i = 0; i < size && i < MAX_TO_PRINT; ++i){
    	if (i%OUTPUT_ROW_SIZE == 0) std::cout << std::endl;
    //	std::cout << std::endl;
    	std::cout.width(6);
    	std::cout << *(data+i);
    }
    if (i == MAX_TO_PRINT){
    	std::cout << "....";
    }
    std::cout << std::endl;
}
