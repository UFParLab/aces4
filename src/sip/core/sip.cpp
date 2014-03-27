/*! abort.cpp
 * 
 *
 *  Created on: Jul 6, 2013
 *      Author: Beverly Sanders
 */

#include "config.h"
#include "sip.h"
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <unistd.h>
#include "sip_interface.h"
#include "global_state.h"

#ifdef HAVE_TAU
#include <TAU.h>
#endif

#ifdef __GNUC__
#include <execinfo.h>
#endif

#ifdef HAVE_MPI
#include "mpi.h"
#endif


void sip_abort() {

// Get backtrace
#ifdef __GNUC__

//	std::cerr<<"Error at line number :"<<current_line()<<std::endl;

	std::cerr << "\nBacktrace:" << std::endl;

	//std::cerr<<"__GNUC__ defined !" << std::endl;

	void *array[10];
	size_t size;

	// get void*'s for all entries on the stack
	size = backtrace(array, 10);

	// print out all the frames to stderr
	backtrace_symbols_fd(array, size, STDERR_FILENO);
	fflush(stdout);
	fflush(stderr);
#ifdef HAVE_TAU
	TAU_PROFILE_EXIT("Collecting TAU info before exiting...");
#endif
#endif

#ifdef HAVE_MPI
	//throw std::logic_error("logic error");
	MPI_Abort(MPI_COMM_WORLD, -1);
#else
	throw std::logic_error("logic error");
	//exit(EXIT_FAILURE);
#endif


}


namespace sip {



const int SETUP_MAGIC = 23121991;
const int SETUP_VERSION = 1;
//const int NUM_INDEX_TYPES = 7;


const int SIOX_MAGIC = 70707;
const int SIOX_VERSION = 1;
const int SIOX_RELEASE = 2;

const int MAX_OMP_THREADS = 8;


void check(bool condition, std::string message, int line){
	if (condition) return;
	std::cerr << "FATAL ERROR: " << message;
	if (line > 0){
		std::string prog = GlobalState::get_program_name();
		int length = prog.size()-std::string(".siox").size();
		std::cerr << " at "<< prog.substr(0,length) << ":" << line;
	}
	std::cerr << std::endl;
	sip_abort();
	//throw std::logic_error("logic error");
}


bool check_and_warn(bool condition, std::string message, int line){
	if (condition) return true;
	std::cerr << "WARNING:  "  << message;
	if (line > 0){
		std::cerr << " at "<< GlobalState::get_program_name() << ":" << line;
	}
	std::cerr << std::endl;
	return false;
}

void fail(std::string message, int line){
//	std::cerr << "FATAL ERROR: " << message;
//	if (line > 0){
//		std::cerr << " at "<< GlobalState::get_program_name() << ":" << line;
//	}
//	sip_abort();
//	//throw std::logic_error("logic error");
	check(false, message, line);
}



} //namespace sip
