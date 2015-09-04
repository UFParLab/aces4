
#ifndef TEST_CONTROLLER_H_
#define TEST_CONTROLLER_H_

#include <race_detection_interpreter.h>
#include <string>
#include <iostream>
#include <map>
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
	virtual ~TestController();
	virtual void initSipTables(const std::string& sial_dir_name = dir_name);
	virtual void runWorker();

	void run();

/** These methods can be called to retrieve values computed during the sial program. */
	sip::IntTable* int_table();
	int int_value(const std::string& name);
	double scalar_value(const std::string& name);
	double* local_block(const std::string& name, const std::vector<int>indices);
	double* static_array(const std::string& name);
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




class RaceDetectionTestController : public TestController{
public:
	RaceDetectionTestController (int num_workers, std::string job, bool has_dot_dat_file, bool verbose, std::string comment, std::ostream& sial_output,
			bool expect_success=true);
	virtual ~RaceDetectionTestController() {}
	virtual void runWorker();
	virtual void initSipTables(const std::string& sial_dir_name = block_consistency_dir_name){
		TestController::initSipTables(sial_dir_name);
	}
private:
	const int num_workers_;
	sip::BarrierBlockConsistencyMap barrier_block_consistency_map_;

};

#endif //TEST_CONTROLLER_H_
