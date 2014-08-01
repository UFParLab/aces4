#include "gtest/gtest.h"
#include <fenv.h>
#include <execinfo.h>
#include <signal.h>
#include <cstdlib>
#include "siox_reader.h"
#include "io_utils.h"
#include "setup_reader.h"

#include "sip_tables.h"
#include "interpreter.h"
#include "setup_interface.h"
#include "sip_interface.h"
#include "data_manager.h"
#include "global_state.h"
#include "sial_printer.h"

#include "worker_persistent_array_manager.h"
#include "server_persistent_array_manager.h"

#include "block.h"

#ifdef HAVE_TAU
#include <TAU.h>
#endif

#ifdef HAVE_MPI
#include "sip_server.h"
#include "sip_mpi_attr.h"
#include "global_state.h"
#include "sip_mpi_utils.h"



//static const std::string dir_name("src/sialx/test/");
//static const std::string expected_output_dir_name("../test/expected_output/");
//
//
//    sip::SIPMPIAttr *attr;
//    void barrier() {sip::SIPMPIUtils::check_err (MPI_Barrier(MPI_COMM_WORLD));}
extern sip::SIPMPIAttr *attr;
 void barrier();
 extern const std::string dir_name;
 extern const std::string expected_output_dir_name;
/** This class controls tests with at least one worker and one server*/

class TestControllerParallel {
public:
	TestControllerParallel(std::string job, bool has_dot_dat_file, bool verbose, std::string comment, std::ostream& sial_output,
			bool expect_success=true):
	job_(job),
	verbose_(verbose),
	comment_(comment),
	sial_output_(sial_output),
	sip_tables_(NULL),
	wpam_(NULL),
	this_test_enabled_(true),
	expect_success_(expect_success),
#ifdef HAVE_MPI
	spam_(NULL),
	server_(NULL),
#endif
	worker_(NULL) {
		if (has_dot_dat_file){
			barrier();
			setup::BinaryInputFile setup_file(job + ".dat");
			setup_reader_ = new setup::SetupReader(setup_file);
			setup::SetupReader::SialProgList &progs = setup_reader_->sial_prog_list_;
			std::string prog_name = progs[0];
			sip::GlobalState::set_program_name(prog_name);
			siox_path = dir_name + prog_name;
		}
		else{
			setup_reader_= setup::SetupReader::get_empty_reader();
			std::string prog_name = job_ + ".siox";
			sip::GlobalState::set_program_name(prog_name);
			siox_path = dir_name + prog_name;
		}
		if (verbose) {
			std::cout << "**************** STARTING TEST " << job_
			<< " ***********************!!!\n"
			<< std::flush;
		}
	}

	TestControllerParallel(std::string job, bool has_dot_dat_file, bool verbose, std::string comment, std::ostream& sial_output,
			int min_workers, int max_workers, int min_servers, int max_servers, bool expect_success=true):
	job_(job),
	verbose_(verbose),
	comment_(comment),
	sial_output_(sial_output),
	sip_tables_(NULL),
	wpam_(NULL),
	expect_success_(expect_success),
	printer_(NULL),
	spam_(NULL),
	server_(NULL),
	worker_(NULL) {

		int num_procs = attr->global_size();
		int num_workers = attr->num_workers();
		int num_servers = attr->num_servers();
		this_test_enabled_ = (min_servers <= num_servers) && (num_servers <= max_servers) &&(min_workers <= num_workers) && (num_workers <= max_workers);
		if (has_dot_dat_file){
			barrier();
			setup::BinaryInputFile setup_file(job + ".dat");
			setup_reader_ = new setup::SetupReader(setup_file);
		}
		else{
			setup_reader_= setup::SetupReader::get_empty_reader();
		}
		if (verbose) {
			std::cout << "**************** STARTING TEST " << job_
			<< " ***********************!!!\n"
			<< std::flush;
		}
	}


	~TestControllerParallel() {
		if (setup_reader_)
			delete setup_reader_;
		if (sip_tables_)
			delete sip_tables_;
		if (wpam_)
			delete wpam_;
#ifdef HAVE_MPI
		if (spam_)
			delete spam_;
		if (server_)
			delete server_;
#endif
		if (worker_)
			delete worker_;
		if (verbose_)
			std::cout << "\nRank " << attr->global_rank() << " TEST " << job_
					<< " TERMINATED" << std::endl << std::flush;
		if (printer_) delete printer_;
	}
	const std::string job_;
	const std::string comment_;
	bool verbose_;
	setup::SetupReader* setup_reader_;
	sip::SipTables* sip_tables_;
	sip::WorkerPersistentArrayManager* wpam_;
#ifdef HAVE_MPI
	sip::ServerPersistentArrayManager* spam_;
	sip::SIPServer* server_;
#endif
	sip::Interpreter* worker_;
	std::ostream& sial_output_;
	sip::SialPrinterForTests* printer_;
	bool this_test_enabled_;
	bool expect_success_;
	std::string siox_path;

	sip::IntTable* int_table(){return &(sip_tables_->int_table_);}

	int int_value(const std::string& name){
		try{
			return worker_->data_manager_.int_value(name);
		}
		catch(const std::exception& e){
			std::cerr << "FAILURE: " << name << " not found in int map.  This is probably a bug in the test." << std::endl << std::flush;
			ADD_FAILURE();
			return -1;
		}
	}

	double scalar_value(const std::string& name){
		try{
			return worker_->data_manager_.scalar_value(name);
		}
		catch(const std::exception& e){
			std::cerr << "FAILURE: " << name << " not found in scalar map.  This is probably a bug in the test." << std::endl << std::flush;
			ADD_FAILURE();
			return -1;
		}
	}

	void initSipTables() {
		setup::BinaryInputFile siox_file(siox_path);
		sip_tables_ = new sip::SipTables(*setup_reader_, siox_file);
		if (verbose_) {
			//rank 0 prints and .siox files contents
			if (attr->global_rank() == 0) {
				std::cout << "JOBNAME = " << job_ << std::endl << std::flush;
				std::cout << "SETUP READER DATA:\n" << *setup_reader_
						<< std::endl << std::flush;
				std::cout << "SIP TABLES" << '\n' << *sip_tables_ << std::endl
						<< std::flush;
				std::cout << comment_ << std::endl << std::flush;
			}
		}
		printer_ = new sip::SialPrinterForTests(sial_output_, attr->global_rank(), *sip_tables_);
	}

	bool runWorker() {
		if (this_test_enabled_){
		worker_ = new sip::Interpreter(*sip_tables_, printer_);
		barrier();
		if (verbose_)
			std::cout << "Rank " << attr->global_rank() << " SIAL PROGRAM "
					<< job_ << " STARTING" << std::endl << std::flush;
		if (expect_success_){ //if success is expected, catch the exception and fail, otherwise, let enclosing test deal with it.
		try{
		worker_->interpret();
		worker_->post_sial_program();
		}
		catch (const std::exception& e){
			std::cerr << "exception thrown in worker: " << e.what();
		    ADD_FAILURE();
		}
		}
			else{
				worker_->interpret();
			}


		if (verbose_){
			if (std::cout != sial_output_) std::cout << sial_output_.rdbuf();
			std::cout << "\nRank " << attr->global_rank() << " SIAL PROGRAM "
					<< job_ << " TERMINATED" << std::endl << std::flush;
		}
		barrier();
		}
		return this_test_enabled_;
	}
	bool runServer() {
		if (this_test_enabled_){
			sip::DataDistribution data_distribution(*sip_tables_, *attr);
			server_ = new sip::SIPServer(*sip_tables_, data_distribution, *attr, spam_);
			barrier();
			if (verbose_)
				std::cout << "Rank " << attr->global_rank() << " SIAL PROGRAM "
						<< job_ << " STARTING SERVER" << std::endl << std::flush;
			if (expect_success_){ //if success is expected, catch the exception and fail, otherwise, let enclosing test deal with it.
			try{
			server_->run();
			}
			catch (const std::exception& e){
				std::cerr << "exception thrown in server: " << e.what();
			    ADD_FAILURE();
			}
			}
				else{
					server_->run();
				}


			if (verbose_){
				if (std::cout != sial_output_) std::cout << sial_output_.rdbuf();
				std::cout << "\nRank " << attr->global_rank() << " SIAL PROGRAM "
						<< job_ << "SERVER TERMINATED" << std::endl << std::flush;
			}
			barrier();
			}
			return this_test_enabled_;

	}
	int num_workers(){return attr->num_workers();}
	int num_servers(){return attr->num_servers();}

	std::string expectedOutput(){
		std::string expected_file_name(expected_output_dir_name + job_ + ".txt");
		std::ifstream t(expected_file_name.c_str());
		if (t.fail()){
			std::cerr << "file containing expected output not found" << std::endl << std::flush;
			ADD_FAILURE();
		}
		std::stringstream buffer;
		buffer << t.rdbuf();
		return buffer.str();
	}

};

#endif
