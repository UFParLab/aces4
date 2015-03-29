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


void check(bool condition, const std::string& message, int line){
	if (condition) return;
	fail(message, line);
}
void check(bool condition, const char* message, int line){
	if (condition) return;
	fail(message, line);
}



bool check_and_warn(bool condition, const std::string& message, int line){
	if (condition) return true;
	warn(message, line);
	return false;
}
bool check_and_warn(bool condition, const char* message, int line){
	if (condition) return true;
	warn(message, line);
	return false;
}

void warn(const char* message, int line) {
	std::cerr << "WARNING:  " << message;
	if (line > 0) {
		std::cerr << " at " << GlobalState::get_program_name() << ":" << line;
	}
	std::cerr << std::endl << std::flush;
}
void warn(const std::string& message, int line) {
	std::cerr << "WARNING:  " << message;
	if (line > 0) {
		std::cerr << " at " << GlobalState::get_program_name() << ":" << line;
	}
	std::cerr << std::endl << std::flush;
}



void fail(const std::string& message, int line){
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
void fail(const char* message, int line){
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

void fail_with_exception(const std::string& message, int line){
	std::stringstream s;
	s << "FATAL ERROR: " << message;
	if (line > 0){
		std::string prog = GlobalState::get_program_name();
		int length = prog.size()-std::string(".siox").size();
		s << " at "<< prog.substr(0,length) << ":" << line;
	}
	s << std::endl << std::flush;
	throw std::logic_error(s.str().c_str());
}


void input_check(bool condition, const std::string& m, int line){
	if (!condition)
		fail("INVALID SETUP OR SIAL PROGRAM:  " + m, line);
}
void input_check(bool condition, const char* m, int line){
	if (!condition)
		fail("INVALID SETUP OR SIAL PROGRAM:  " + std::string(m), line);
}
void input_fail(const char* m, int line){
	fail("INVALID SETUP OR SIAL PROGRAM:  " + std::string(m), line);
}

bool input_warn(bool condition, const std::string& m, int line){
	if (!condition)
		warn("INVALID SETUP OR SIAL PROGRAM:  " + m, line);
	return condition;
}
bool input_warn(bool condition, const char* m, int line){
	if (!condition)
		warn("INVALID SETUP OR SIAL PROGRAM:  " + std::string(m), line);
	return condition;
}

void sial_check(bool condition, const std::string& m, int line){
	if (!condition)
    	fail("LIKELY ERRONEOUS SIAL PROGRAM. " + m, line);
}
void sial_check(bool condition, const char* m, int line){
	if (!condition)
		fail("LIKELY ERRONEOUS SIAL PROGRAM. " + std::string(m), line);
}
void sial_fail(const std::string& m, int line){
	fail("LIKELY ERRONEOUS SIAL PROGRAM. " + m, line);
}

bool sial_warn(bool condition, const std::string& m, int line){
	if (!condition)
		warn("POSSIBLY ERRONEOUS SIAL PROGRAM:  " + m, line);
	return condition;
}
bool sial_warn(bool condition, const char* m, int line){
	if (!condition)
		warn("POSSIBLY ERRONEOUS SIAL PROGRAM:  " + std::string(m), line);
	return condition;
}

} //namespace sip
