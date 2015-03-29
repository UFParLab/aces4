/*
 * block_consistency_helper_methods.h
 *
 *  Created on: Mar 28, 2015
 *      Author: njindal
 */

#ifndef ACES_MAIN_BLOCK_CONSISTENCY_HELPER_METHODS_H_
#define ACES_MAIN_BLOCK_CONSISTENCY_HELPER_METHODS_H_

#include "config.h"
#include "siox_reader.h"
#include "io_utils.h"
#include "setup_reader.h"
#include "assert.h"
#include "sialx_timer.h"
#include "sip_tables.h"
#include "interpreter.h"
#include "sialx_interpreter.h"
#include "setup_interface.h"
#include "sip_interface.h"
#include "data_manager.h"
#include "worker_persistent_array_manager.h"
#include "block.h"
#include "global_state.h"
#include "sip_mpi_attr.h"
#include "profile_timer.h"
#include "profile_timer_store.h"
#include "profile_interpreter.h"
#include "sipmap_interpreter.h"
#include "sipmap_timer.h"
#include "remote_array_model.h"
#include "rank_distribution.h"
#include "block_consistency_interpreter.h"

#include <vector>
#include <sstream>
#include <iostream>
#include <fstream>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fenv.h>

#include <execinfo.h>
#include <signal.h>

#ifdef HAVE_MPI
#include "mpi.h"
#include "sip_mpi_utils.h"
#endif


/**
 * http://stackoverflow.com/questions/3151779/how-its-better-to-invoke-gdb-from-program-to-print-its-stacktrace/4611112#4611112
 */
void bt_sighandler(int signum) {
	std::cerr << "Interrupt signal (" << signum << ") received." << std::endl;
    sip_abort();
}


/**
 * Enables floating point exceptions & registers
 * signal handlers so as to print a stack trace
 * when the program exits due to failure of some kind
 */
static void setup_signal_and_exception_handlers() {
#ifdef __GLIBC__
#ifdef __GNU_LIBRARY__
#ifdef _GNU_SOURCE
    feenableexcept(FE_DIVBYZERO);
    feenableexcept(FE_OVERFLOW);
    feenableexcept(FE_INVALID);
#endif // _GNU_SOURCE
#endif // __GNU_LIBRARY__
#endif // __GLIBC__
	signal(SIGSEGV, bt_sighandler);
	signal(SIGFPE, bt_sighandler);
	signal(SIGTERM, bt_sighandler);
	signal(SIGINT, bt_sighandler);
	signal(SIGABRT, bt_sighandler);
}

/**
 * IFDEF wrapped initialization of MPI.
 * The mpi error handler is also set
 * to return the error code instead of having
 * it fail implicitly
 * @param argc
 * @param argv
 */
static void mpi_init(int *argc, char ***argv){
#ifdef HAVE_MPI
	/* MPI Initialization */
	MPI_Init(argc, argv);
#endif //HAVE_MPI
}

/**
 * IFDEF wrapped finalization of MPI
 */
static void mpi_finalize(){
#ifdef HAVE_MPI
	MPI_Finalize();
#endif
}

/**
 * IFDEF wrapped mpi barrier
 */
static void barrier(){
#ifdef HAVE_MPI
      sip::SIPMPIUtils::check_err(MPI_Barrier(MPI_COMM_WORLD));
#endif
}

/**
 * Check sizes of data types.
 * In the MPI version, the TAG is used to communicate information
 * The various bits needed to send information to other nodes
 * sums up to 32.
 */
void check_expected_datasizes() {

	sip::check(sizeof(int) >= 4, "Size of integer should be 4 bytes or more");
	sip::check(sizeof(double) >= 8, "Size of double should be 8 bytes or more");
	sip::check(sizeof(long long) >= 8,
			"Size of long long should be 8 bytes or more");
}

/**
 * Encapsulates all the parameters the user passes into the
 * aces4 executable(s)
 */
struct Aces4Parameters{
	char * init_binary_file;
	char * init_json_file;
	char * sialx_file_dir;
	bool init_json_specified ;
	bool init_binary_specified;
	std::string job;
	int num_workers;
	double percentage_of_workers;
	Aces4Parameters(){
		init_binary_file = "data.dat";	// Default initialization file is data.dat
		sialx_file_dir = ".";			// Default directory for compiled sialx files is "."
		init_json_file = NULL;			// Json file, no default file
		init_json_specified = false;
		init_binary_specified = false;
		job = "";
		num_workers = -1;
		percentage_of_workers = 100.0;
	}
};


/**
 * Prints usage of the aces4 program(s)
 * @param program_name
 */
static void print_usage(const std::string& program_name) {
	std::cerr << "Usage : " << program_name << " [options] " << std::endl;
	std::cerr << "\t -d : binary initialization Data file " << std::endl;
	std::cerr << "\t -j : Json initialization file, use EITHER -d or -j " << std::endl;
	std::cerr << "\t -s : directory of compiled Sialx files  " << std::endl;
	std::cerr << "\t -w : number of Workers to simulate  " << std::endl;
	std::cerr << "\t -x : approXimate percentage of workers to simulate  (default 100%)" << std::endl;
	std::cerr << "\tDefaults: " << std::endl;
	std::cerr << "\t\tdata file - \"data.dat\""<< std::endl;
	std::cerr << "\t\tsialx directory - \".\"" << std::endl;
	std::cerr << "\t-? or -h to display this usage dialogue" << std::endl;
}

template<typename T>
static T read_from_optarg() {
	std::string str(optarg);
	std::stringstream ss(str);
	T d;
	ss >> d;
	return d;
}

/**
 * Parses the command line arguments
 * and returns all the parameters in
 * an instance of Aces4Parameters.
 * @param argc
 * @param argv
 * @return
 */
Aces4Parameters parse_command_line_parameters(int argc, char* argv[]) {
	Aces4Parameters parameters;
	// Read about getopt here : http://www.gnu.org/software/libc/manual/html_node/Getopt.html
	// d: name of .dat file.
	// j: name of json file (must specify either of d or j, not both.
	// s: directory to look for siox files
	// w : number of workers
	// x : approXimate number of workers (in %) to simulate
	// h & ? are for help. They require no arguments
	const char* optString = "a:d:j:s:p:w:r:x:h?";
	int c;

	bool workers_specified = false;

	while ((c = getopt(argc, argv, optString)) != -1) {
		switch (c) {
		case 'd': {
			parameters.init_binary_file = optarg;
			parameters.init_binary_specified = true;
		}
			break;
		case 'j': {
			parameters.init_json_file = optarg;
			parameters.init_json_specified = true;
		}
			break;
		case 's': {
			parameters.sialx_file_dir = optarg;
		}
			break;
		case 'w' : {
			parameters.num_workers = read_from_optarg<int>();
			workers_specified = true;
		}
			break;
		case 'x' : {
			parameters.percentage_of_workers = read_from_optarg<double>();
		}
			break;
		case 'h':
		case '?':
		default:
			std::string program_name = argv[0];
			print_usage(program_name);
			exit(1);
		}
	}
	if (parameters.init_binary_specified && parameters.init_json_specified) {
		std::cerr << "Cannot specify both init binary data file and json init file !" << std::endl;
		//std::string program_name = argv[0];
		print_usage( argv[0]);
		exit(1);
	}

	if (!workers_specified) {
		std::cerr << "Must specify the number of workers" << std::endl;
		print_usage( argv[0]);
		exit(1);
	}

	if (parameters.percentage_of_workers <= 0 || parameters.percentage_of_workers > 100.0){
		std::cerr << "Must specify a percentage of the workers to approximate (0<X<=100)" << std::endl;
		print_usage(argv[0]);
		exit(1);
	}

	if (parameters.init_json_specified)
		parameters.job = parameters.init_json_file;
	else
		parameters.job = parameters.init_binary_file;

	return parameters;
}

/**
 * Reads the initialization file (json or binary)
 * and returns a new SetupReader instance allocated on the heap.
 * It is the callers responsibility to cleanup this instance.
 * @param parameters
 * @return
 */
setup::SetupReader* read_init_file(const Aces4Parameters& parameters) {
	setup::SetupReader *setup_reader = NULL;
	if (parameters.init_json_specified) {
		std::ifstream json_ifstream(parameters.init_json_file,
				std::ifstream::in);
		setup_reader = new setup::SetupReader(json_ifstream);
	} else {
		//create setup_file
		SIP_MASTER_LOG(
				std::cout << "Initializing data from " << parameters.job << std::endl);
		setup::BinaryInputFile setup_file(parameters.job); //initialize setup data
		setup_reader = new setup::SetupReader(setup_file);
	}
	setup_reader->aces_validate();
	return setup_reader;
}





#endif /* ACES_MAIN_BLOCK_CONSISTENCY_HELPER_METHODS_H_ */
