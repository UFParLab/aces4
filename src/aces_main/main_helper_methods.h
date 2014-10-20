/*
 * main_helper_methods.h
 *
 *  Created on: Oct 18, 2014
 *      Author: njindal
 */

#ifndef MAIN_HELPER_METHODS_H_
#define MAIN_HELPER_METHODS_H_

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

#include <vector>
#include <sstream>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fenv.h>

#include <execinfo.h>
#include <signal.h>

#ifdef HAVE_MPI
#include "mpi.h"
#include "sip_server.h"
#include "sip_mpi_utils.h"
#include "server_persistent_array_manager.h"
#endif

#ifdef HAVE_TAU
#include <TAU.h>
#endif

/**
 * http://stackoverflow.com/questions/3151779/how-its-better-to-invoke-gdb-from-program-to-print-its-stacktrace/4611112#4611112
 */
void bt_sighandler(int signum) {
	std::cerr << "Interrupt signal (" << signum << ") received." << std::endl;
    sip_abort();
}

/**
 * Prints usage of the aces4 program(s)
 * @param program_name
 */
static void print_usage(const std::string& program_name) {
	std::cerr << "Usage : " << program_name << " -d <init_data_file> -j <init_json_file> -s <sialx_files_directory> -m <max_memory_in_gigabytes>" << std::endl;
	std::cerr << "\t -d : binary initialization data file " << std::endl;
	std::cerr << "\t -j : json initialization file, use EITHER -d or -j " << std::endl;
	std::cerr << "\t -s : directory of compiled sialx files  " << std::endl;
	std::cerr << "\t -m : memory to be used by workers (and servers) specified in GB " << std::endl;
	std::cerr << "\tDefaults: data file - \"data.dat\", sialx directory - \".\", Memory : 2GB" << std::endl;
	std::cerr << "\tm is the approximate memory to use. Actual usage will be more." << std::endl;
	std::cerr << "\t-? or -h to display this usage dialogue" << std::endl;
}

/**
 * Enables floating point exceptions & registers
 * signal handlers so as to print a stack trace
 * when the program exits due to failure of some kind
 */
static void setup_signal_and_exception_handlers() {
	feenableexcept(FE_DIVBYZERO);
	feenableexcept(FE_OVERFLOW);
	feenableexcept(FE_INVALID);
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
	sip::SIPMPIUtils::set_error_handler();
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
	std::size_t memory;
	std::string job;
	Aces4Parameters(){
		init_binary_file = "data.dat";	// Default initialization file is data.dat
		sialx_file_dir = ".";			// Default directory for compiled sialx files is "."
		init_json_file = NULL;			// Json file, no default file
		memory = 2147483648;			// Default memory usage : 2 GB
		init_json_specified = false;
		init_binary_specified = false;
		job = "";
	}
};

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
	// m: approximate memory to be used. Actual usage will be more than this.
	// h & ? are for help. They require no arguments
	const char* optString = "d:j:s:m:h?";
	int c;
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
		case 'm': {
			std::string memory_string(optarg);
			std::stringstream ss(memory_string);
			double memory_in_gb;
			ss >> memory_in_gb;
			parameters.memory = memory_in_gb * 1024 * 1024 * 1024;
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
		std::cerr
				<< "Cannot specify both init binary data file and json init file !"
				<< std::endl;
		std::string program_name = argv[0];
		print_usage(program_name);
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

#endif /* MAIN_HELPER_METHODS_H_ */
