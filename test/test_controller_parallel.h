
#ifndef __TEST_CONTROLLER_PARALLEL__
#define __TEST_CONTROLLER_PARALLEL__

#include "config.h"
#include <string>
#include <iostream>
#include "setup_reader.h"



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
 */
class TestControllerParallel {
public:
	TestControllerParallel(std::string job, bool has_dot_dat_file, bool verbose, std::string comment, std::ostream& sial_output,
			bool expect_success=true);
	~TestControllerParallel() ;

	void initSipTables();
	sip::IntTable* int_table();
	int int_value(const std::string& name);
	double scalar_value(const std::string& name);
	void run();
	int num_workers();
	int num_servers();
	std::string expectedOutput();
	double* local_block(const std::string& name, const std::vector<int>indices);

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

