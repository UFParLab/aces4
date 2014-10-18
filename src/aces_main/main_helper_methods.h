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



static void mpi_init(int *argc, char ***argv){
#ifdef HAVE_MPI
	/* MPI Initialization */
	MPI_Init(argc, argv);
	sip::SIPMPIUtils::set_error_handler();
#endif //HAVE_MPI
}

static void mpi_finalize(){
#ifdef HAVE_MPI
	MPI_Finalize();
#endif
}

static void barrier(){
#ifdef HAVE_MPI
      sip::SIPMPIUtils::check_err(MPI_Barrier(MPI_COMM_WORLD));
#endif
}

void check_expected_datasizes() {
	// Check sizes of data types.
	// In the MPI version, the TAG is used to communicate information
	// The various bits needed to send information to other nodes
	// sums up to 32.
	sip::check(sizeof(int) >= 4, "Size of integer should be 4 bytes or more");
	sip::check(sizeof(double) >= 8, "Size of double should be 8 bytes or more");
	sip::check(sizeof(long long) >= 8,
			"Size of long long should be 8 bytes or more");
}

#endif /* MAIN_HELPER_METHODS_H_ */
