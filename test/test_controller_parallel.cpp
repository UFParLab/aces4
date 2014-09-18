/*
 * test_controller_parallel.cpp
 *

 */



#include "test_controller_parallel.h"
#include "config.h"

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

#include "sip_server.h"
#include "sip_mpi_attr.h"
#include "global_state.h"
#include "sip_mpi_utils.h"


#include "test_constants.h"

/** This class controls tests with at least one worker and at least one server.  The configuration is
 * determined by the sip built-in sip_mpi_attr.
 *
 * Currently, failures in the sip (check, input_check, sial_check) whether expected or not, cause the main
 * test program to call mpi abort and crash.
 *
 */

TestControllerParallel::TestControllerParallel(std::string job,
		bool has_dot_dat_file, bool verbose, std::string comment,
		std::ostream& sial_output, bool expect_success) :
		job_(job), verbose_(verbose), comment_(comment), sial_output_(
				sial_output), sip_tables_(NULL), wpam_(NULL), this_test_enabled_(
				true), expect_success_(expect_success), prog_number_(0), spam_(
				NULL), server_(NULL), worker_(NULL), printer_(NULL), server_timer_(NULL), sialx_timers_(NULL) {
	barrier();
	sip::GlobalState::reinitialize();
	if (has_dot_dat_file) {
		setup::BinaryInputFile setup_file(job + ".dat");
		setup_reader_ = new setup::SetupReader(setup_file);
		progs_ = &setup_reader_->sial_prog_list();
	} else {
		setup_reader_ = setup::SetupReader::get_empty_reader();
		progs_ = new std::vector<std::string>();
		progs_->push_back(job + ".siox");
	}
	if (attr->is_worker())
		wpam_ = new sip::WorkerPersistentArrayManager();
	else
		spam_ = new sip::ServerPersistentArrayManager();
	if (verbose) {
		std::cout << "****** Creating controller for test " << job_
				<< " *********!!!\n" << std::flush;
	}
	barrier();
}

TestControllerParallel::~TestControllerParallel() {
	if (verbose_)
		std::cout << "\nRank " << attr->global_rank() << " Controller for  "
				<< job_ << " is being deleted" << std::endl << std::flush;
#ifdef HAVE_MPI
	if (server_)
		delete server_;
#endif
	if (worker_)
		delete worker_;
	if (wpam_)
		delete wpam_;
#ifdef HAVE_MPI
	if (spam_)
		delete spam_;
#endif
	if (printer_)
		delete printer_;
	if (setup_reader_)
		delete setup_reader_;
	if (sip_tables_)
		delete sip_tables_;
	if (sialx_timers_)
		delete sialx_timers_;
#ifdef HAVE_MPI
	if (server_timer_)
		delete server_timer_;
#endif
}


void TestControllerParallel::initSipTables(const std::string& sial_dir_name) {
	barrier();
	prog_name_ = progs_->at(prog_number_++);
	sip::GlobalState::set_program_name(prog_name_);
	sip::GlobalState::increment_program();
	std::string siox_path = sial_dir_name + prog_name_;
	setup::BinaryInputFile siox_file(siox_path);
	//remove objects left from previous sial programs to avoid memory leaks

	if (worker_) delete worker_;
	if (sialx_timers_)
		delete sialx_timers_;
#ifdef HAVE_MPI
	if (server_) delete server_;
	if (server_timer_) delete server_timer_;
#endif
	if (sip_tables_) delete sip_tables_;
	sip_tables_ = new sip::SipTables(*setup_reader_, siox_file);
	if (verbose_) {
		//rank 0 prints and .siox files contents
		if (attr->global_rank() == 0) {
			std::cout << "JOBNAME = " << job_ << ", PROGRAM NAME = " << prog_name_ << std::endl << std::flush;
			std::cout << "SETUP READER DATA:\n" << *setup_reader_
			<< std::endl << std::flush;
			std::cout << "SIP TABLES" << '\n' << *sip_tables_ << std::endl
			<< std::flush;
			std::cout << comment_ << std::endl << std::flush;
		}
	}
	printer_ = new sip::SialPrinterForTests(sial_output_, attr->global_rank(), *sip_tables_);
	barrier();
}

//sip::IntTable* TestControllerParallel::int_table() {
//	return &(sip_tables_->int_table_);
//}

int TestControllerParallel::int_value(const std::string& name) {
	try {
		return worker_->data_manager().int_value(name);
	} catch (const std::exception& e) {
		std::cerr << "FAILURE: " << name
				<< " not found in int map.  This is probably a bug in the test."
				<< std::endl << std::flush;
		ADD_FAILURE();
		return -1;
	}
}

double TestControllerParallel::scalar_value(const std::string& name) {
	try {
		return worker_->data_manager().scalar_value(name);
	} catch (const std::exception& e) {
		std::cerr << "FAILURE: " << name
				<< " not found in scalar map.  This is probably a bug in the test."
				<< std::endl << std::flush;
		ADD_FAILURE();
		return -1;
	}
}

void TestControllerParallel::print_timers(){
	std::cout << "in print_timers() " << std::endl;
	if (sip_tables_ == NULL || sialx_timers_ == NULL){
		std::cout << "sip_table_ " << (sip_tables_==NULL ? "NULL" : "OK")
				<< "sialx_timer " << (sialx_timers_==NULL ? "NULL" : "OK") << std::endl << std::flush;
		return;
	}
	const std::vector<std::string> lno2name = sip_tables_->line_num_to_name();
	sialx_timers_->print_timers(lno2name, sial_output_);
	sial_output_<< std::flush;
}

void TestControllerParallel::run() {
	barrier();
	if (attr->is_worker()) {
		runWorker();
	} else {
		runServer();
	}
	barrier();
}

int TestControllerParallel::num_workers() {
	return attr->num_workers();
}

int TestControllerParallel::num_servers() {
	return attr->num_servers();
}

std::string TestControllerParallel::expectedOutput() {
	std::string expected_file_name(expected_output_dir_name + job_ + ".txt");
	std::ifstream t(expected_file_name.c_str());
	if (t.fail()) {
		std::cerr << "file containing expected output not found" << std::endl
				<< std::flush;
		ADD_FAILURE();
	}
	std::stringstream buffer;
	buffer << t.rdbuf();
	return buffer.str();
}

double* TestControllerParallel::local_block(const std::string& name,
		const std::vector<int> indices) {
	try {
		int array_slot = sip_tables_->array_slot(name);
		int rank = sip_tables_->array_rank(array_slot);
		sip::BlockId id(array_slot, rank, indices);
		sip::Block::BlockPtr block =
				worker_->data_manager_.block_manager_.get_block_for_reading(id);
		return block->get_data();
	} catch (const std::exception& e) {
		std::cerr << "FAILURE: block of array " << name
				<< " not found.  This is probably a bug in the test."
				<< std::endl << std::flush;
		ADD_FAILURE();
		return NULL;
	}
}

#ifdef HAVE_MPI
bool TestControllerParallel::runServer() {
	if (this_test_enabled_) {
		sip::DataDistribution data_distribution(*sip_tables_, *attr);
		sip::ServerTimer server_timer(sip_tables_->max_timer_slots());
		server_ = new sip::SIPServer(*sip_tables_, data_distribution, *attr,
				spam_, server_timer);
		barrier();
		if (verbose_)
			std::cout << "Rank " << attr->global_rank() << " SIAL PROGRAM "
					<< prog_name_ << " STARTING SERVER " << std::endl
					<< std::flush;
		if (expect_success_) { //if success is expected, catch the exception and fail, otherwise, let enclosing test deal with it.
			try {
				server_->run();
			} catch (const std::exception& e) {
				std::cerr << "exception thrown in server: " << e.what();
				ADD_FAILURE();
			}
		} else {
			server_->run();
		}

		if (verbose_) {
			if (std::cout != sial_output_)
				std::cout << sial_output_.rdbuf();
			std::cout << "\nRank " << attr->global_rank() << " SIAL PROGRAM "
					<< job_ << "SERVER TERMINATED " << std::endl << std::flush;
		}
		spam_->save_marked_arrays(server_);
	}
	sial_output_ << std::flush;
	return this_test_enabled_;

}
#endif

bool TestControllerParallel::runWorker() {
	if (this_test_enabled_) {



		int slot = sip_tables_->max_timer_slots();
		sialx_timers_ = new sip::SialxTimer(sip_tables_->max_timer_slots());
		worker_ = new sip::Interpreter(*sip_tables_, sialx_timers_, printer_, wpam_);
		barrier();

		if (verbose_)
			std::cout << "Rank " << attr->global_rank() << " SIAL PROGRAM "
					<< prog_name_ << " STARTING WORKER " << std::endl
					<< std::flush;
		if (expect_success_) { //if success is expected, catch the exception and fail, otherwise, let enclosing test deal with it.
			try {
				worker_->interpret();
			} catch (const std::exception& e) {
				std::cerr << "exception thrown in worker: " << e.what();
				ADD_FAILURE();
			}
		} else {
			worker_->interpret();
		}

		if (verbose_) {
			if (std::cout != sial_output_)
				std::cout << sial_output_.rdbuf();
			std::cout << "\nRank " << attr->global_rank() << " SIAL PROGRAM "
					<< prog_name_ << " TERMINATED WORKER " << std::endl
					<< std::flush;
		}
		worker_->post_sial_program();
		wpam_->save_marked_arrays(worker_);
	}
	sial_output_ << std::flush;
	return this_test_enabled_;
}


