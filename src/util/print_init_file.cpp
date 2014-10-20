/*
 * print_init_file.cpp
 *
 *  Created on: Oct 16, 2014
 *      Author: njindal
 */

#include "sip_mpi_attr.h"
#include "config.h"
#include "siox_reader.h"
#include "io_utils.h"
#include "setup_reader.h"
#include "assert.h"
#include "sialx_timer.h"
#include "sip_tables.h"
#include "setup_interface.h"

#include <vector>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fenv.h>
#include <execinfo.h>
#include <signal.h>

void print_usage(const std::string& program_name) {
	std::cerr
			<< "Prints the contents of a given .dat file (or initialization file)"
			<< std::endl;
	std::cerr << "Usage : " << program_name << " -d <init_data_file> -j"
			<< std::endl;
	std::cerr << "\t -d : binary initialization data file " << std::endl;
	std::cerr << "\t -j : json initialization file, use EITHER -d or -j "
			<< std::endl;
	std::cerr << "\t -J : prints out json output" << std::endl;
	std::cerr << "\t-? or -h to display this usage dialogue" << std::endl;
	std::cerr << "\tDefault data file is \"data.dat\"." << std::endl;
}

int main(int argc, char* argv[]) {

#ifdef HAVE_MPI
	/* MPI Initialization */
	MPI_Init(&argc, &argv);
#endif


	// Default initialization file is data.dat
	char *init_file = "data.dat";
	// Json file, no default file
	char *json_file = NULL;

	bool print_json = false;

	bool json_specified = false;
	bool init_file_specified = false;

	// Read about getopt here : http://www.gnu.org/software/libc/manual/html_node/Getopt.html
	const char *optString = "d:j:Jh?";
	int c;
	while ((c = getopt(argc, argv, optString)) != -1){
		switch (c) {
		case 'd':
			init_file = optarg;
			init_file_specified = true;
			break;
		case 'j':
			json_file = optarg;
			json_specified = true;
			break;
		case 'J':
			print_json = true;
			break;
		case 'h':case '?':
		default:
			std::string program_name = argv[0];
			print_usage(program_name);
			return 1;
		}
	}

	if (init_file_specified && json_specified){
		std::cerr << "Cannot specify both init binary data file and json init file !" << std::endl;
		std::string program_name = argv[0];
		print_usage(program_name);
		return 1;
	}


	setup::SetupReader *setup_reader = NULL;

	if (json_specified){
		std::ifstream json_ifstream(json_file, std::ifstream::in);
		setup_reader = new setup::SetupReader(json_ifstream);
	} else {
		//create setup_file
		std::string job(init_file);
		SIP_MASTER_LOG(std::cout << "Initializing data from " << job << std::endl);
		setup::BinaryInputFile setup_file(job); //initialize setup data
		setup_reader = new setup::SetupReader(setup_file);
	}


	if (print_json)
		std::cout << setup_reader->get_json_string();
	else
		std::cout << "SetupReader::" <<std::endl << *setup_reader << std::endl;

	delete setup_reader;

#ifdef HAVE_MPI
	MPI_Finalize();
#endif

	return 0;
}



