
#ifndef __TEST_CONTROLLER_PARALLEL__
#define __TEST_CONTROLLER_PARALLEL__

#include "config.h"
#include <string>
#include <iostream>
#include "setup_reader.h"
#include "test_constants.h"



namespace sip{
class SipTables;
class WorkerPersistentArrayManager;
class Interpreter;
class SialPrinterForTests;
class IntTable;
class ServerPersistentArrayManager;
class SIPServer;

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

	/** adds a restart id
	 *
	 * use JobControl::global->get_job_id to get the job id of the preceding job, then
	 * use that as the value to pass here to test restart.
	 *
	 * @param job
	 * @param has_dot_dat_file
	 * @param verbose
	 * @param comment
	 * @param sial_output
	 * @param restart_id
	 * @param expect_success
	 */
	TestControllerParallel(std::string job,
			bool has_dot_dat_file, bool verbose, std::string comment,
			std::ostream& sial_output, std::string restart_id, int restart_prognum, bool expect_success=true);
	~TestControllerParallel() ;

	void initSipTables(const std::string& sial_dir_name = dir_name);
	void run();

	int int_value(const std::string& name);
	double scalar_value(const std::string& name);
	double* local_block(const std::string& name, const std::vector<int>indices);
	double* static_array(const std::string& name);
	void print_timers(std::ostream& out);

	int num_workers();
	int num_servers();
	std::string expectedOutput();

	sip::IntTable* int_table();
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
	bool this_test_enabled_;
	bool expect_success_;
	int prog_number_;
	std::string prog_name_;
	setup::SetupReader::SialProgList *progs_;

	bool runServer();
	bool runWorker();

};

#endif //__TEST_CONTROLLER_PARALLEL__

