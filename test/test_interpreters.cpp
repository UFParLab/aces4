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

#ifdef HAVE_TAU
#include <TAU.h>
#endif


#include "test_constants.h"
#include "test_controller.h"
#include "test_controller_parallel.h"

bool VERBOSE_TEST = true;


TEST(ProfileInterpreter, test1){

	std::string job("contraction_small_test");
	int norb = 1;
	if (attr->global_rank() == 0){
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

	sip::ProfileTimer::Key key = controller.key_for_line(27);	// Contraction line.
	std::pair<long, long> time_count_pair = controller.profile_timer_store_->get_from_store(key);
	ASSERT_EQ(time_count_pair.second, 1);
	ASSERT_GT(time_count_pair.first, 0);

}



//****************************************************************************************************************

/**
 * All workers in rank distribution
 */
class AllWorkerRankDistribution : public sip::RankDistribution{
public:
	virtual bool is_server(int rank, int size){
		return false;
	}
	virtual int local_server_to_communicate(int rank, int size){
		return -1;
	}
	virtual bool is_local_worker_to_communicate(int rank, int size){
		return false;
	}
};

int main(int argc, char **argv) {


#ifdef HAVE_MPI
	MPI_Init(&argc, &argv);
	int num_procs;
	sip::SIPMPIUtils::check_err(MPI_Comm_size(MPI_COMM_WORLD, &num_procs));

	if (num_procs < 2) {
		std::cerr << "Please run this test with at least 2 mpi ranks"
				<< std::endl;
		return -1;
	}
	AllWorkerRankDistribution all_workers_rank_dist;
	sip::SIPMPIAttr::set_rank_distribution(&all_workers_rank_dist);
	sip::SIPMPIUtils::set_error_handler();
#endif

	sip::SIPMPIAttr &sip_mpi_attr = sip::SIPMPIAttr::get_instance();
	attr = &sip_mpi_attr;
	barrier();

	INIT_GLOBAL_TIMERS(&argc, &argv);

	check_expected_datasizes();

	printf("Running main() from %s\n",__FILE__);
	testing::InitGoogleTest(&argc, argv);
	barrier();
	int result = RUN_ALL_TESTS();

	FINALIZE_GLOBAL_TIMERS();

	barrier();

#ifdef HAVE_MPI
	MPI_Finalize();
#endif

	return result;

}


