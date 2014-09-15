
#ifndef TEST_CONTROLLER_H_
#define TEST_CONTROLLER_H_

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
}

/** This class controls tests with no servers.  Multiple MPI processes just execute the same program and don't communicate.
 * For programs that need servers, use a TestControllerParallel object.
 */
class TestController {
public:
	TestController(std::string job, bool has_dot_dat_file, bool verbose, std::string comment, std::ostream& sial_output,
			bool expect_success=true);
	~TestController();
	sip::IntTable* int_table();

	void initSipTables(const std::string& sial_dir_name = dir_name);
	void run();
	int int_value(const std::string& name);
	double scalar_value(const std::string& name);
	double* local_block(const std::string& name, const std::vector<int>indices);
	void runWorker();
	int num_workers();
	std::string expectedOutput();



	const std::string job_;
	const std::string comment_;
	bool verbose_;
	setup::SetupReader* setup_reader_;
	sip::SipTables* sip_tables_;
	sip::WorkerPersistentArrayManager* wpam_;
	sip::Interpreter* worker_;
	std::ostream& sial_output_;
	sip::SialPrinterForTests* printer_;
	bool expect_success_;

	int prog_number_;
	std::string prog_name_;
	setup::SetupReader::SialProgList *progs_;

};

#endif //TEST_CONTROLLER_H_
