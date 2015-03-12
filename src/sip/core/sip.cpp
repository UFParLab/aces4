/*! abort.cpp
 * 
 *
 *  Created on: Jul 6, 2013
 *      Author: Beverly Sanders
 */

#include "config.h"
#include "sip.h"
#include <iostream>
#include <sstream>
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
#include "sip_mpi_attr.h"
#endif


void sip_abort() {

// Get backtrace
#ifdef __GNUC__

	std::cerr << "\nBacktrace:" << std::endl;
	void *array[10];
	size_t size;

	// get void*'s for all entries on the stack
	size = backtrace(array, 10);

	// print out all the frames to stderr
	backtrace_symbols_fd(array, size, STDERR_FILENO);
#ifdef HAVE_TAU
	TAU_PROFILE_EXIT("Collecting TAU info before exiting...");
#endif
#endif

#ifdef HAVE_MPI
	//throw std::logic_error("logic error");
	MPI_Abort(MPI_COMM_WORLD, -1);
#else
	//throw std::logic_error("logic error");
	exit(EXIT_FAILURE);
#endif // __GNUC__
	fflush(stdout);
	fflush(stderr);

}

void sip_abort(std::string m) {

    std::cerr << m << std::endl;
// Get backtrace
#ifdef __GNUC__
	std::cerr << "\nBacktrace:" << std::endl;
	void *array[10];
	size_t size;

	// get void*'s for all entries on the stack
	size = backtrace(array, 10);

	// print out all the frames to stderr
	backtrace_symbols_fd(array, size, STDERR_FILENO);
#ifdef HAVE_TAU
	TAU_PROFILE_EXIT("Collecting TAU info before exiting...");
#endif
#endif // __GNUC__

#ifdef HAVE_MPI
	if (sip::SIPMPIAttr::get_instance().is_worker())
		std::cerr << "worker rank " << sip::SIPMPIAttr::get_instance().global_rank() << ": " << m << std::endl<< std::flush;
	else
		std::cerr << "server rank " << sip::SIPMPIAttr::get_instance().global_rank() << ": " << m << std::endl<< std::flush;

	fflush(stdout);
	fflush(stderr);
	usleep(1000000); //give output time to appear before aborting
	MPI_Abort(MPI_COMM_WORLD, -1);
#else
	throw std::logic_error(m);
	//exit(EXIT_FAILURE);
#endif
	fflush(stdout);
	fflush(stderr);


}
namespace sip {

bool _sip_debug_print = true;
bool _all_rank_print = false;

//bool should_all_ranks_print() { return _all_rank_print; }


const int SETUP_MAGIC = 23121991;
const int SETUP_VERSION = 1;
//const int NUM_INDEX_TYPES = 7;


const int SIOX_MAGIC = 70707;
const int SIOX_VERSION = 2; //EXPECTS NEW OPCODES
const int SIOX_RELEASE = 0;

const int MAX_OMP_THREADS = 8;


void check(bool condition, std::string message, int line){
	if (condition) return;
//	std::cerr << "FATAL ERROR: " << message;
//	if (line > 0){
//		std::string prog = GlobalState::get_program_name();
//		int length = prog.size()-std::string(".siox").size();
//		std::cerr << " at "<< prog.substr(0,length) << ":" << line;
//	}
//	std::cerr << std::endl << std::flush;
	std::stringstream s;
	s << "FATAL ERROR: " << message;
	if (line > 0){
		std::string prog = GlobalState::get_program_name();
		int length = prog.size()-std::string(".siox").size();
		s << " at "<< prog.substr(0,length) << ":" << line;
	}
	s << std::endl << std::flush;
	sip_abort(s.str());
	//throw std::logic_error("logic error");
}


bool check_and_warn(bool condition, std::string message, int line){
	if (condition) return true;
	std::cerr << "WARNING:  "  << message;
	if (line > 0){
		std::cerr << " at "<< GlobalState::get_program_name() << ":" << line;
	}
	std::cerr << std::endl << std::flush;
	return false;
}

void fail(std::string message, int line){
	check(false, message, line);
}

void input_check(bool condition, std::string m, int line){
	check(condition, "INVALID SETUP OR SIAL PROGRAM:  " + m, line);
}

bool input_warn(bool condition, std::string m, int line){
	return check_and_warn(condition, "INVALID SETUP OR SIAL PROGRAM:  " + m, line);
}

void sial_check(bool condition, std::string m, int line){
    check(condition, "LIKELY ERRONEOUS SIAL PROGRAM. " + m, line);
}

bool sial_warn(bool condition, std::string m, int line){
    return check_and_warn(condition, "POSSIBLY ERRONEOUS SIAL PROGRAM:  " + m, line);
}

} //namespace sip
