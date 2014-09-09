/*
 * print_siptables.cpp
 *
 *  Created on: Jul 8, 2014
 *      Author: njindal
 */

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



int main(int argc, char* argv[]) {

#ifdef HAVE_MPI
	/* MPI Initialization */
	MPI_Init(&argc, &argv);
#endif


	// Default initialization file is data.dat
	char *init_file = "data.dat";
	// Default directory for compiled sialx files is "."
	char *sialx_file_dir = ".";

	// Read about getopt here : http://www.gnu.org/software/libc/manual/html_node/Getopt.html
	// d: means an argument is required for d. Specifies the .dat file.
	// s: means an argument is required for s.
	// h & ? are for help. They require no arguments
	const char *optString = "d:s:h?";
	int c;
	while ((c = getopt(argc, argv, optString)) != -1){
		switch (c) {
		case 'd':
			init_file = optarg;
			break;
		case 's':
			sialx_file_dir = optarg;
			break;
		case 'h':case '?':
		default:
			std::cerr<< "Constructs SIP Tables from a .dat file and .siox files and prints them to stdout" <<std::endl;
			std::cerr<<"Usage : "<<argv[0]<<" -d <init_data_file> -s <sialx_files_directory>"<<std::endl;
			std::cerr<<"\tDefault data file is \"data.dat\". Default sialx directory is \".\""<<std::endl;
			std::cerr<<"\t-? or -h to display this usage dialogue"<<std::endl;
			return 1;
		}
	}

	//create setup_file
	std::string job(init_file);

	//initialize setup data
	setup::BinaryInputFile setup_file(job);
	setup::SetupReader setup_reader(setup_file);

	setup::SetupReader::SialProgList &progs = setup_reader.sial_prog_list();
	setup::SetupReader::SialProgList::iterator it;

	std::cout << "SetupReader::" <<std::endl << setup_reader << std::endl;

	for (it = progs.begin(); it != progs.end(); ++it) {
		std::string sialfpath;
		sialfpath.append(sialx_file_dir);
		sialfpath.append("/");
		sialfpath.append(*it);

		setup::BinaryInputFile siox_file(sialfpath);
		sip::SipTables sipTables(setup_reader, siox_file);

		// Print the sipTables instance
		std::cout << "Sial File : " << sialfpath << std::endl;
		std::cout << sipTables << std::endl;
	}

#ifdef HAVE_MPI
	MPI_Finalize();
#endif

	return 0;
}
