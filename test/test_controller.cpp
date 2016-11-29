
#include "test_controller.h"

#include "gtest/gtest.h"
#include <fenv.h>
#include <execinfo.h>
#include <signal.h>
#include <cstdlib>
#include "siox_reader.h"
#include "io_utils.h"
#include "setup_reader.h"

#include "aces_defs.h"
#include "sip_tables.h"
#include "interpreter.h"
#include "setup_interface.h"
#include "sip_interface.h"
#include "data_manager.h"
#include "job_control.h"
#include "sial_printer.h"

#include "worker_persistent_array_manager.h"

#include "block.h"

#ifdef HAVE_TAU
#include <TAU.h>
#endif

#ifdef HAVE_MPI
//#include "sip_server.h"
//#include "sip_mpi_attr.h"
//#include "job_control.h"
//#include "sip_mpi_utils.h"
//#else
//#include "sip_mpi_attr.h"
#endif

#include "test_constants.h"
#include "job_control.h"

TestController::TestController(std::string job, bool has_dot_dat_file,
		bool verbose, std::string comment, std::ostream& sial_output,
		bool expect_success) :
		job_(job), verbose_(verbose), comment_(comment), sial_output_(
				sial_output), sip_tables_(NULL), wpam_(NULL), expect_success_(
				expect_success), worker_(NULL), printer_(NULL),
				prog_number_(0){
// 	barrier();
//	if (has_dot_dat_file) {
//		barrier();
//		setup::BinaryInputFile setup_file(job + ".dat");
//		setup_reader_ = new setup::SetupReader(setup_file);
//		setup::SetupReader::SialProgList &progs = setup_reader_->sial_prog_list();
//		std::string prog_name = progs[0];
//		siox_path = dir_name + prog_name;
//	} else {
//		setup_reader_ = setup::SetupReader::get_empty_reader();
//		std::string prog_name = job_ + ".siox";
//		siox_path = dir_name + prog_name;
//		std::cout << "siox_path: " << siox_path << std::endl << std::flush;
//	}

	sip::JobControl::set_global_job_control(new sip::JobControl(sip::JobControl::make_job_id()));
	sip::MemoryTracker::set_global_memory_tracker(new sip::MemoryTracker());

	std::cout << "job_id" << sip::JobControl::global->get_job_id() << std::endl << std::flush;
	if (has_dot_dat_file) {
		setup::BinaryInputFile setup_file(job + ".dat");
		setup_reader_ = new setup::SetupReader(setup_file);
		progs_ = &setup_reader_->sial_prog_list();
	} else {
		setup_reader_ = setup::SetupReader::get_empty_reader();
		progs_ = new std::vector<std::string>();
		progs_->push_back(job + ".siox");
	}
	wpam_ = new sip::WorkerPersistentArrayManager();
	if (verbose) {
		std::cout << "**************** STARTING TEST " << job_
				<< " ***********************!!!\n" << std::flush;
	}
//	barrier();
}


TestController::~TestController() {
	if (setup_reader_)
		delete setup_reader_;
	if (wpam_)
		delete wpam_;
	if (worker_)
		delete worker_;
	if (verbose_)
		std::cout << "\nRank " << attr->global_rank() << " TEST " << job_
				<< " TERMINATED" << std::endl << std::flush;
	if (printer_)
		delete printer_;
	if (sip_tables_)
		delete sip_tables_;
}

sip::IntTable* TestController::int_table() {
	return &(sip_tables_->int_table_);
}

void TestController::initSipTables(const std::string& sial_dir_name) {
//	barrier();
//	setup::BinaryInputFile siox_file(siox_path);
//	sip_tables_ = new sip::SipTables(*setup_reader_, siox_file);
//	if (verbose_) {
//		//rank 0 prints and .siox files contents
//		if (attr->global_rank() == 0) {
//			std::cout << "JOBNAME = " << job_ << std::endl << std::flush;
//			std::cout << "SETUP READER DATA:\n" << *setup_reader_ << std::endl
//					<< std::flush;
//			std::cout << "SIP TABLES" << '\n' << *sip_tables_ << std::endl
//					<< std::flush;
//			std::cout << comment_ << std::endl << std::flush;
//		}
//	}
//	printer_ = new sip::SialPrinterForTests(sial_output_, attr->global_rank(),
//			*sip_tables_);
//	barrier();
//	prog_name_ = progs_->at(prog_number_++);
	prog_number_ = sip::JobControl::global->get_program_num();
	prog_name_ = progs_->at(prog_number_);
	sip::JobControl::global->set_program_name(prog_name_);
//	sip::JobControl::global->increment_program();
	std::string siox_path = sial_dir_name + prog_name_;
	setup::BinaryInputFile siox_file(siox_path);
	if (!sip_tables_) delete sip_tables_;
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
//	barrier();
}

void TestController::run() {
	runWorker();
}


int TestController::int_value(const std::string& name) {
	try {
		return worker_->data_manager_.int_value(name);
	} catch (const std::exception& e) {
		std::cerr << "FAILURE: " << name
				<< " not found in int map.  This is probably a bug in the test."
				<< std::endl << std::flush;
		ADD_FAILURE();
		return -1;
	}
}

double TestController::scalar_value(const std::string& name) {
	try {
		return worker_->data_manager_.scalar_value(name);
	} catch (const std::exception& e) {
		std::cerr << "FAILURE: " << name
				<< " not found in scalar map.  This is probably a bug in the test."
				<< std::endl << std::flush;
		ADD_FAILURE();
		return -1;
	}
}

double* TestController::static_array(const std::string& name){
	int array_slot = sip_tables_->array_slot(name);
	sip::Block::BlockPtr block =
			worker_->get_static(array_slot);
	return block->get_data();
}

double* TestController::local_block(const std::string& name,
		const std::vector<int> indices) {
	std::cout << "looking up block " << name << " " << indices[0] << std::endl;
	try {
		int array_slot = sip_tables_->array_slot(name);
		int rank = sip_tables_->array_rank(array_slot);
//			int *indices_to_pass = new int[MAX_RANK];
//			std::copy(indices, indices + rank, indices_to_pass);
//			std::fill(indices_to_pass + rank, indices_to_pass + MAX_RANK, sip::unused_index_value);
//			for (int i = 0 ; i < MAX_RANK; ++i){
//				std::cout << indices_to_pass[i];
//			}
//			sip::BlockId id(array_slot, indices_to_pass);
		sip::BlockId id(array_slot, rank, indices);
		std::cout << "this is the block id " << id.str(*sip_tables_)
				<< std::endl;
		sip::Block::BlockPtr block =
				worker_->data_manager_.block_manager_.get_block_for_reading(id);
		return block->get_data();
//		    delete [] indices_to_pass;
	} catch (const std::exception& e) {
		std::cerr << "FAILURE: block of array " << name
				<< " not found.  This is probably a bug in the test."
				<< std::endl << std::flush;
		ADD_FAILURE();
		return NULL;
	}
}


void TestController::runWorker() {
	// Clear previous worker_ to avoid leak
	if (worker_ != NULL)
		delete worker_;
//	sip::SialxTimer sialx_timers(sip_tables_->op_table_size());
	worker_ = new sip::Interpreter(*sip_tables_,  printer_, wpam_);
//	barrier();
	if (verbose_)
		std::cout << "Rank " << attr->global_rank() << " SIAL PROGRAM " << job_
				<< " STARTING" << std::endl << std::flush;
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
		if (&std::cout != &sial_output_)
			std::cout << sial_output_.rdbuf();
		std::cout << "\nRank " << attr->global_rank() << " SIAL PROGRAM "
				<< job_ << " TERMINATED" << std::endl << std::flush;
	}
//	barrier();
	wpam_->save_marked_arrays(worker_);
	std::stringstream tt;
	tt << sip::JobControl::global->get_job_id() << "." << sip::JobControl::global->get_program_num() << "." << "worker_checkpoint";
	std::string worker_checkpoint_filename = tt.str();
	wpam_->checkpoint_persistent(worker_checkpoint_filename);

}

int TestController::num_workers() {
	return attr->num_workers();
}

std::string TestController::expectedOutput() {
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




