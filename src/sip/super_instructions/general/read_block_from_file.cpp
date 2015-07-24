/*
 * read_block_from_file.cpp
 *
 *  Created on: Jul 23, 2015
 *      Author: njindal
 */



#include <fstream>
#include <sstream>
#include <string>
#include "interpreter.h"
#include "sip_interface.h"
#include "io_utils.h"

void read_block_from_file(int& array_slot, int& rank, int* index_values, int& size, int* extents,  double* data, int& ierr){

	std::string array_name(sip::Interpreter::global_interpreter->array_name(array_slot));
	std::stringstream block_name_ss;
	block_name_ss<< array_name << "[" << index_values[0];
	for (int i=1; i<rank; ++i)
		block_name_ss << "." << index_values[i];
	block_name_ss << "]";

	std::stringstream file_ss;
	file_ss << block_name_ss.str() << ".blk";

	setup::BinaryInputFile d2m(file_ss.str().c_str());
	std::cout << "Reading contents of block " << file_ss.str() << " to memory " << block_name_ss.str() << std::endl;

	int * read_extents;
	double *read_data;

	int dummy = d2m.read_int();
	read_extents = d2m.read_int_array(&rank);
	read_data = d2m.read_double_array(&size);

	std::copy(read_extents+0, read_extents+rank, extents);
	std::copy(read_data+0, read_data+size, data);

	delete [] read_extents;
	delete [] read_data;


	/**
	b2dfile << rank;
	for (int i=0; i<rank; ++i){
		b2dfile << extents[i];
	}
    for (int i=0; i<size; ++i){
       	b2dfile << data[i];
    }
    b2dfile.close();
    **/
}



