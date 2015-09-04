
#ifndef __TEST_CONTROLLER_PARALLEL__
#define __TEST_CONTROLLER_PARALLEL__

#include "config.h"
#include <string>
#include <iostream>
#include "setup_reader.h"
#include "test_constants.h"
#include "sialx_timer.h"
#include "server_timer.h"
#include "profile_timer.h"


namespace sip{
class SipTables;
class WorkerPersistentArrayManager;
class Interpreter;
class SialPrinterForTests;
class IntTable;
class ServerPersistentArrayManager;
class SIPServer;
class ProfileTimerStore;
class ProfileInterpreter;

}


/** This class controls tests with at least one worker and at least one server.  The configuration is
 * determined by the sip built-in sip_mpi_attr.
 *
 * Currently, failures in the sip (check, input_check, sial_check) whether expected or not, cause the main
 * test program to call mpi abort and crash.
 *
 * To run a test, create the .dat file if desired with name job.dat and pass it to the TestControllerParallel constructor
 *
 * Then, for each SIAL program in the test, call
 *
 * initSipTables();
 * run();
 * perform checks
 *
 * initSiptTables();
 * run();
 * perform checks
 *
 * etc.
 *
 * The sial program names are obtained from the .dat file.  The controller keeps track of the sial program, so calling initSipTables() will
 * The get the next program.
 *
 * The run method does the right thing, depending on whether the process is a worker or server.
 */
class TestControllerParallel {
public:
	TestControllerParallel(std::string job, bool has_dot_dat_file, bool verbose, std::string comment, std::ostream& sial_output,
			bool expect_success=true);
	virtual  ~TestControllerParallel() ;

	virtual void initSipTables(const std::string& sial_dir_name = dir_name);
	virtual void run();

	virtual int int_value(const std::string& name);
	virtual double scalar_value(const std::string& name);
	virtual double* local_block(const std::string& name, const std::vector<int>indices);
	virtual double* static_array(const std::string& name);
	virtual void print_timers(std::ostream& out);

	virtual int num_workers();
	virtual int num_servers();
	virtual std::string expectedOutput();

	virtual bool runWorker();

	//virtual sip::IntTable* int_table();

#ifdef HAVE_MPI
	virtual bool runServer();
#endif

	const std::string job_;
	const std::string comment_;
	bool verbose_;
	setup::SetupReader* setup_reader_;
	sip::SipTables* sip_tables_;
	sip::WorkerPersistentArrayManager* wpam_;
	sip::ServerPersistentArrayManager* spam_;
	sip::SIPServer* server_;
	sip::Interpreter* worker_;
	std::ostream& sial_output_;
	sip::SialPrinterForTests* printer_;
	sip::SialxTimer* sialx_timers_;
	sip::ServerTimer* server_timer_;
	bool this_test_enabled_;
	bool expect_success_;
	int prog_number_;
	std::string prog_name_;
	setup::SetupReader::SialProgList *progs_;

};

#endif //__TEST_CONTROLLER_PARALLEL__

