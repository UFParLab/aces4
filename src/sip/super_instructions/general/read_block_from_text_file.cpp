#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "interpreter.h"
#include "sip_interface.h"
#include "io_utils.h"

/**
 * Reads a block from a ASCII text file into the given block.
 * @param array_slot
 * @param rank
 * @param index_values
 * @param size
 * @param extents
 * @param data
 * @param ierr
 */
void read_block_from_text_file(int& array_slot, int& rank, int* index_values, int& size, int* extents,  double* data, int& ierr){

	std::string array_name(sip::Interpreter::global_interpreter->array_name(array_slot));
	std::stringstream block_name_ss;
	block_name_ss<< array_name << "[" << index_values[0];
	for (int i=1; i<rank; ++i)
		block_name_ss << "." << index_values[i];
	block_name_ss << "]";

	std::stringstream file_ss;
	file_ss << block_name_ss.str() << ".txt";

	std::ifstream input_file(file_ss.str().c_str());
	std::cout << "Reading contents of block " << block_name_ss.str() << " from file " << file_ss.str() << std::endl;

	int *indices = new int[rank];

	long read_size = 0;
	while (!input_file.eof()) {
		for (int i=0; i<rank; ++i){
			input_file >> indices[i];
		}
		double val;
		input_file >> val;
		read_size++;

		int loc = indices[rank-1] - 1;
		for (int i=rank-2; i>=0; --i){
			loc *= extents[i];
			loc += indices[i] - 1;
		}

		sip::check (loc != -1, "Invalid index for block " + block_name_ss.str());
		data[loc] = val;

	}

	sip::check(read_size == size, "Incorrect number of elements in file " + file_ss.str());

	delete [] indices;

}
