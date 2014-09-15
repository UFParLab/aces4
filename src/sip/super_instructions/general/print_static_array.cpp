/*
 * print_block.cpp
 *
 * Prints the first MAX_TO_PRINT elements of a block in rows of the size of the first dimension
 *
 * Values are printed in the order they are stored in memory.
 *
 *  Created on: Aug 25, 2013
 *      Author: Beverly Sanders
 */


#include <iostream>
#include "sip_interface.h"
#include "interpreter.h"
#include "contiguous_array_manager.h"

void print_static_array(int& array_slot, int& rank, int* index_values, int& size, int* extents,  double* data, int& ierr){
//	array::Block::BlockPtr  contiguous =
//			sip::Interpreter::global_interpreter->data_manager_->contiguous_array_manager_.get_array(array_slot);
//	double * contig_data = contiguous->get_data();
//	int row_size = contiguous->shape().segment_sizes_[0];
//	int array_size = contiguous->size();
//	std::cout << "printing static array " << sip::array_name_value(array_slot) << " size=" << array_size << std::endl;
//	int i;
//    for (i = 0; i < array_size; ++i){
//    	if (i%row_size == 0) std::cout << std::endl;
//    	std::cout.width(6);
//    	std::cout << *(contig_data+i);
//    }
//
//    std::cout << std::endl;
	if (size == 1) {
	std::cout << "static array " << sip::array_name_value(array_slot) << " size=" << size ;
	} else {
	std::cout << "static array " << sip::array_name_value(array_slot) << " size=" << size << std::endl;
	}
	int row_size = extents[0];
	int i;
	for (i = 0; i < size; ++i){
		//if (i%row_size == 0) std::cout << std::endl;
                std::cout.precision(8);
                std::cout.setf(std::ios_base::fixed);
                std::cout << *(data+i);
                std::cout << " ";
        }
        std::cout << std::endl;
}
