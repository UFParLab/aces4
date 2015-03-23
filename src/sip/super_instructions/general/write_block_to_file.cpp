/*
 * write_block_to_file.cpp
 *
 *  Created on: Dec 11, 2014
 *      Author: njindal
 */

#include <fstream>
#include <sstream>
#include <string>
#include "interpreter.h"
#include "sip_interface.h"
#include "io_utils.h"

void write_block_to_file(int& array_slot, int& rank, int* index_values, int& size, int* extents,  double* data, int& ierr){

	std::string array_name(sip::Interpreter::global_interpreter()->array_name(array_slot));
	std::stringstream block_name_ss;
	block_name_ss<< array_name << "[" << index_values[0];
	for (int i=1; i<rank; ++i)
		block_name_ss << "." << index_values[i];
	block_name_ss << "]";

	std::stringstream file_ss;
	file_ss << block_name_ss.str() << ".blk";

	setup::BinaryOutputFile b2dfile(file_ss.str().c_str());
	std::cout << "Writing contents of block " << block_name_ss.str() << " to file " << file_ss.str() << std::endl;

	b2dfile.write_int(rank);
	b2dfile.write_int_array(rank, extents);
	b2dfile.write_double_array(size, data);
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



