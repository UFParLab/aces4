/*
 * print_worker_checkpoint.cpp
 *
 *  Created on: NOv 21, 2015
 *      Author: basbas
 */


#include "io_utils.h"
#include "setup_interface.h"
#ifdef HAVE_MPI
#include <mpi.h>
#endif
#include "block.h"



void print_usage(const std::string& program_name) {
	std::cerr
			<< "Prints the contents of a given checkpoint file)"
			<< std::endl;
	std::cerr << "Usage : " << program_name << " -c <checkpoint filename> "
			<< std::endl;
	std::cerr << "\t-? or -h to display this usage dialogue" << std::endl;
}

int main(int argc, char* argv[]) {

#ifdef HAVE_MPI
	/* MPI Initialization */
	MPI_Init(&argc, &argv);
#endif
	std::string filename = "";

	// Read about getopt here : http://www.gnu.org/software/libc/manual/html_node/Getopt.html
	const char *optString = "c:h?";
	int c;
	while ((c = getopt(argc, argv, optString)) != -1){
		switch (c) {
		case 'c':
			filename = std::string(optarg);

			break;
		case 'h':case '?':
		default:
			std::string program_name = argv[0];
			print_usage(program_name);
			return 1;
		}
	}


	setup::InputStream * file;
	setup::BinaryInputFile *bfile = new setup::BinaryInputFile(filename);  //checkpoint file opened here
	file = bfile;
	//restore scalars
//			std::cerr<< "WorkerPersistentArrayManager opened the file " << filename << std::endl << std::flush;
	int num_scalars = file->read_int();
//			std::cerr << "\nnum_scalars=" << num_scalars << std::endl << std::flush;
	std::cout << filename << ":" << std::endl;
	for (int i=0; i < num_scalars; ++i){
		std::string name = file->read_string();
		double value = file->read_double();
		std::cout << name << " = " << value << std::endl;
//				std::cerr << "initializing persistent scalar "<< name << "=" << value << " from restart" << std::endl << std::flush;
	}
	//restore contiguous arrays
	int num_arrays = file->read_int();
//			std::cerr << "\n num_arrays=" << num_arrays << std::endl << std::flush;
	for (int i = 0; i < num_arrays; i++) {
		// Name of array
		std::string name = file->read_string();
		// Rank of array
		int rank = file->read_int();
		// Dimensions
		int * dims = file->read_int_array(&rank);
		// Number of elements
		int num_data_elems = 1;
		for (int i=0; i<rank; i++){
			num_data_elems *= dims[i];
		}

		double * data = file->read_double_array(&num_data_elems);
		sip::segment_size_array_t dim_sizes;
		std::copy(dims+0, dims+rank,dim_sizes);
		if (rank < MAX_RANK) std::fill(dim_sizes+rank, dim_sizes+MAX_RANK, 1);
		sip::BlockShape shape(dim_sizes, rank);
		delete [] dims;

		sip::Block::BlockPtr block = new sip::Block(shape,data);

		std::cout <<  name  << ":" << std::endl << *block << std::endl << std::endl;
	}
	std::cout << std::flush;


#ifdef HAVE_MPI
	MPI_Finalize();
#endif

	return 0;
}



