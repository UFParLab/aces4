/*
 * test_interpreters.cpp
 *
 *  Created on: Nov 10, 2014
 *      Author: njindal
 */


#include "gtest/gtest.h"
#include <fenv.h>
#include <execinfo.h>
#include <signal.h>
#include <cstdlib>
#include <cassert>
#include <utility>
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
#include "sip_timer.h"
#include "profile_interpreter.h"
#include "profile_timer.h"
#include "profile_timer_store.h"
#include "worker_persistent_array_manager.h"
#include "block.h"
#include "rank_distribution.h"
#include "block_consistency_interpreter.h"

#ifdef HAVE_TAU
#include <TAU.h>
#endif

#ifdef HAVE_MPI
#include "sip_mpi_utils.h"
#endif

#include "test_constants.h"
#include "test_controller.h"
#include "test_controller_parallel.h"

bool VERBOSE_TEST = true;


TEST(ProfileInterpreter, test1){
	if (attr->global_rank() == 0){
		std::string job("contraction_small_test");
		int norb = 1;
		{
			init_setup(job.c_str());
			set_constant("norb",norb);
			std::string tmp = job + ".siox";
			const char* nm= tmp.c_str();
			add_sial_program(nm);
			int segs[]  = {40};
			set_aoindex_info(1,segs);
			finalize_setup();
		}
		std::stringstream output;
		ProfileInterpreterTestControllerParallel controller(job, true, VERBOSE_TEST, "", output);
		controller.initSipTables();
		controller.run();

		sip::ProfileTimer::Key key = controller.key_for_pc(16);	// PC for Contraction line.
		std::pair<double, long> time_count_pair = controller.profile_timer_store_->get_from_store(key);
		ASSERT_EQ(time_count_pair.second, 1);
		ASSERT_GT(time_count_pair.first, 0.0);
	}

}

TEST(BlockConsistencyInterpreter, test1){

	if (attr->global_rank() == 0){
		for (int i=2; i<6; ++i){
			std::string job("test1");
			int norb = 2;
			int num_workers = i;
			{
				init_setup(job.c_str());
				set_constant("norb",norb);
				std::string tmp = job + ".siox";
				const char* nm= tmp.c_str();
				add_sial_program(nm);
				int segs[]  = {40, 40};
				set_aoindex_info(2,segs);
				finalize_setup();
			}
			std::stringstream output;
			BlockConsistencyTestController controller(num_workers, job, true, VERBOSE_TEST, "", output, false);
			controller.initSipTables();
			ASSERT_THROW(controller.runWorker(), std::logic_error);
		}
	}
}

TEST(BlockConsistencyInterpreter, test2){

	if (attr->global_rank() == 0){
		for (int i=2; i<6; ++i){
			std::string job("test2");
			int norb = 2;
			int num_workers = i;
			{
				init_setup(job.c_str());
				set_constant("norb",norb);
				std::string tmp = job + ".siox";
				const char* nm= tmp.c_str();
				add_sial_program(nm);
				int segs[]  = {40, 40};
				set_aoindex_info(2,segs);
				finalize_setup();
			}
			std::stringstream output;
			BlockConsistencyTestController controller(num_workers, job, true, VERBOSE_TEST, "", output, false);
			controller.initSipTables();
			ASSERT_THROW(controller.runWorker(), std::logic_error);
		}
	}
}

TEST(BlockConsistencyInterpreter, test3){

	if (attr->global_rank() == 0){
		for (int i=2; i<6; ++i){
			std::string job("test3");
			int norb = 2;
			int num_workers = i;
			{
				init_setup(job.c_str());
				set_constant("norb",norb);
				std::string tmp = job + ".siox";
				const char* nm= tmp.c_str();
				add_sial_program(nm);
				int segs[]  = {40, 40};
				set_aoindex_info(2,segs);
				finalize_setup();
			}
			std::stringstream output;
			BlockConsistencyTestController controller(num_workers, job, true, VERBOSE_TEST, "", output, false);
			controller.initSipTables();
			ASSERT_THROW(controller.runWorker(), std::logic_error);
		}
	}
}

TEST(BlockConsistencyInterpreter, test4){

	if (attr->global_rank() == 0){
		for (int i=2; i<6; ++i){
			std::string job("test4");
			int norb = 2;
			int num_workers = i;
			{
				init_setup(job.c_str());
				set_constant("norb",norb);
				std::string tmp = job + ".siox";
				const char* nm= tmp.c_str();
				add_sial_program(nm);
				int segs[]  = {40, 40};
				set_aoindex_info(2,segs);
				finalize_setup();
			}
			std::stringstream output;
			BlockConsistencyTestController controller(num_workers, job, true, VERBOSE_TEST, "", output, false);
			controller.initSipTables();
			controller.runWorker();
		}
	}
}


//****************************************************************************************************************


int main(int argc, char **argv) {


#ifdef HAVE_MPI
	MPI_Init(&argc, &argv);
	int num_procs;
	sip::SIPMPIUtils::check_err(MPI_Comm_size(MPI_COMM_WORLD, &num_procs));

//	if (num_procs < 2) {
//		std::cerr << "Please run this test with at least 2 mpi ranks"
//				<< std::endl;
//		return -1;
//	}
	sip::AllWorkerRankDistribution all_workers_rank_dist;
	sip::SIPMPIAttr::set_rank_distribution(&all_workers_rank_dist);
	sip::SIPMPIUtils::set_error_handler();
#endif

	sip::SIPMPIAttr &sip_mpi_attr = sip::SIPMPIAttr::get_instance();
	attr = &sip_mpi_attr;
	check_expected_datasizes();

	printf("Running main() from %s\n",__FILE__);
	testing::InitGoogleTest(&argc, argv);
	int result = RUN_ALL_TESTS();

#ifdef HAVE_MPI
	MPI_Finalize();
#endif

	return result;

}


