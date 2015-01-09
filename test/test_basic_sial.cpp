/**
 *
 * This file contains test of basic sial language features that do not require
 * interaction with a server.
 *
 */
#include "gtest/gtest.h"
#include <fenv.h>
#include <execinfo.h>
#include <signal.h>
#include <cstdlib>
#include <cassert>
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
#include "rank_distribution.h"
#include "worker_persistent_array_manager.h"

#include "block.h"

#ifdef HAVE_TAU
#include <TAU.h>
#endif

#ifdef HAVE_MPI
#include "test_controller_parallel.h"
#include "server_persistent_array_manager.h"
#endif

#include "test_constants.h"
#include "test_controller.h"

extern "C" {
int test_transpose_op(double*);
int test_transpose4d_op(double*);
int test_contraction_small2(double*);
}

//static const std::string dir_name("src/sialx/test/");
//static const std::string qm_dir_name("src/sialx/qm/");
//static const std::string expected_output_dir_name("../test/expected_output/");


//bool VERBOSE_TEST = false;
bool VERBOSE_TEST = true;

// TODO FIXME
// commented out since the empty program has a distributed array
// and on a multinode aces4, distributed arrays also require a server
//TEST(BasicSial,DISABLED_empty) {
//	TestController controller("empty", false, VERBOSE_TEST,
//			"this is a an empty program using the test controller", std::cout);
//	controller.initSipTables();
//	controller.runWorker();
//	EXPECT_TRUE(controller.worker_->all_stacks_empty());
//}

TEST(SipUnit,contiguous_local_block_id){
	int lower1[] = {2,3,2,3,2,3};
	int upper1[] = {3,4,3,4,3,4};
	sip::BlockId id1(1, lower1, upper1);
	std::cout << "id1=" <<id1 << std::endl << std::flush;
	int lower2[] = {2,3,2,3,2,3};
	int upper2[] = {3,4,3,4,3,4};
	sip::BlockId id2(1, lower2, upper2);
	std::cout << "id2=" <<id2 << std::endl << std::flush;
	EXPECT_TRUE(id1==id2);
	EXPECT_FALSE(id1 < id2);
	EXPECT_FALSE(id2 < id1);
	int lower3[] = {3,3,2,3,2,3};
	int upper3[] = {3,3,2,3,2,3};
	sip::BlockId id3(1, lower3, upper3);
	std::cout << "id3=" <<id3 << std::endl << std::flush;
	EXPECT_FALSE(id1 == id3);
	EXPECT_TRUE(id1 < id3);
	EXPECT_FALSE(id3 < id1);
	int lower4[] = {3,5,3,4,3,4};
	int upper4[] = {3,5,3,4,3,4};
	sip::BlockId id4(1, lower4, upper4);
	std::cout << "id4=" << id4 << std::endl << std::flush;
	EXPECT_FALSE(id1 == id4);
	EXPECT_TRUE(id1 < id4);
	EXPECT_FALSE(id4 < id1);
	//these tests have involve arrays with different ids
	sip::BlockId id5(3, lower1, upper1);
	EXPECT_TRUE(id1<id5);
	EXPECT_TRUE(id4<id5);
	EXPECT_FALSE(id1==id5);

    int lower6[]={4,4,-1,-1,-1,-1};
    int upper6[]={8,8,-1,-1,-1,-1};
	sip::BlockId id6(1, lower6, upper6);
    int lower7[]={6,8,-1,-1,-1,-1};
    int upper7[]={8,8,-1,-1,-1,-1};
	sip::BlockId id7(1, lower7, upper7);
    EXPECT_TRUE(id6.encloses(id7));
    EXPECT_TRUE(id6.overlaps(id7));
    EXPECT_TRUE(id7.overlaps(id6));
}


TEST(BasicSial,contiguous_local){
	std::string job("contiguous_local");
	std::stringstream output;
	if (attr->global_rank() == 0) {
		init_setup(job.c_str());
		//now add data
		set_constant("norb", 4);
		int segs[] = { 5, 6, 7, 8 };
		set_aoindex_info(4, segs);
		//add the first program for this job and finalize
		std::string tmp = job + ".siox";
		const char* nm = tmp.c_str();
		add_sial_program(nm);
		finalize_setup();
	}
	barrier();
	TestController controller(job, true, true, "", output);
	controller.initSipTables();
	controller.runWorker();
}


TEST(BasicSial,contiguous_local_2){
	std::string job("contiguous_local_2");
	std::stringstream output;
	int norb = 12;
	if (attr->global_rank() == 0) {
		init_setup(job.c_str());
		//now add data
		set_constant("norb", norb);
		int segs[] = { 3,4,5,3,4,5,3,4,5,3,4,5 }; //the must be norb elements
		set_aoindex_info(norb, segs);
		//add the first program for this job and finalize
		std::string tmp = job + ".siox";
		const char* nm = tmp.c_str();
		add_sial_program(nm);
		finalize_setup();
	}
	barrier();
	TestController controller(job, true, true, "", output);
	controller.initSipTables();
	controller.runWorker();
	int i,j;
	double* res = controller.static_array("res");
//	for (i = 0; i < norb; i++ ){
//		for(j = 0; i < norb; j++)
//		ASSERT_EQ(i, res[i]
//	}
	ASSERT_TRUE(controller.worker_->all_stacks_empty());
	ASSERT_EQ(0,controller.worker_->num_blocks_in_blockmap());
}

//testing framework cannot gracefully handle expected errors from MPI processes
#ifndef HAVE_MPI
TEST(BasicSial,contiguous_local_allocate_twice_fail){
	std::string job("contiguous_local_allocate_twice_fail");
	std::stringstream output;
	if (attr->global_rank() == 0) {
		init_setup(job.c_str());
		//now add data
		set_constant("norb", 4);
		int segs[] = { 5, 6, 7, 8 };
		set_aoindex_info(4, segs);
		//add the first program for this job and finalize
		std::string tmp = job + ".siox";
		const char* nm = tmp.c_str();
		add_sial_program(nm);
		finalize_setup();
	}
	barrier();
	TestController controller(job, true, true, "", output, false);
	controller.initSipTables();
	ASSERT_THROW(controller.runWorker(), std::logic_error);
}
#endif

//testing framework cannot gracefully handle expected errors from MPI processes
#ifndef HAVE_MPI
TEST(SipUnit,BlockIdInvalidRange){
	std::cout << "\n\n\nTHIS TEST IS EXPECTED TO HAVE A FATAL ERROR!!!\n\n\n" << std::endl << std::flush;
	int lower1[] = {2,3,2,3,2,3};
	int upper1[] = {2,2,3,4,3,4};
	ASSERT_THROW(sip::BlockId id1(1, lower1, upper1),std::logic_error);
}
#endif

TEST(BasicSial,helloworld) {
	TestController controller("helloworld", false, VERBOSE_TEST,
			"this test should print \"hello world\"", std::cout);
	controller.initSipTables();
	controller.runWorker();
	EXPECT_TRUE(controller.worker_->all_stacks_empty());
}

TEST(BasicSial,scalars) {
	std::string job("scalars");
	std::string comment("this test checks scalar initialization");
	double x = 3.456;
	double y = -0.1;
	std::stringstream output;
	//create .dat file
	if (attr->global_rank() == 0) {
		init_setup(job.c_str());
		set_scalar("x", x);
		set_scalar("y", y);
		std::string tmp = job + ".siox";
		const char* nm = tmp.c_str();
		add_sial_program(nm);
		finalize_setup();
	}
	barrier();
	TestController controller(job, true, VERBOSE_TEST, comment, output);
	controller.initSipTables();
	controller.runWorker();

	if (attr->global_rank() == 0)
		EXPECT_EQ(controller.expectedOutput(), output.str());
	EXPECT_TRUE(controller.worker_->all_stacks_empty());
}

TEST(BasicSial,no_arg_user_sub) {
	std::string job("no_arg_user_sub");
	TestController controller(job, false, VERBOSE_TEST, "", std::cout);
	controller.initSipTables();
	controller.runWorker();
	EXPECT_TRUE(controller.worker_->all_stacks_empty());
}

TEST(BasicSial,index_decs) {
	std::string job("index_decs");

	//set up index_decs.dat file
	if (attr->global_rank() == 0) {
		init_setup(job.c_str());
		//now add data
		set_constant("norb", 15);
		int segs[] = { 5, 6, 7, 8 };
		set_aoindex_info(4, segs);
		//add the first program for this job and finalize
		std::string tmp = job + ".siox";
		const char* nm = tmp.c_str();
		add_sial_program(nm);
		finalize_setup();
	}
	barrier();
	TestController controller(job, true, VERBOSE_TEST,
			"sial program is only declarations, does not execute anything",
			std::cout);

	controller.initSipTables();
	//check some properties of the sip tables.
	int i_index_slot = controller.sip_tables_->index_id("i");
	int j_index_slot = controller.sip_tables_->index_id("j");
	int aio_index_slot = controller.sip_tables_->index_id("aoi");
	EXPECT_EQ(1, controller.sip_tables_->lower_seg(i_index_slot));
	EXPECT_EQ(4, controller.sip_tables_->num_segments(i_index_slot));

	EXPECT_EQ(4, controller.sip_tables_->lower_seg(j_index_slot));
	EXPECT_EQ(2, controller.sip_tables_->num_segments(j_index_slot));

	EXPECT_EQ(1, controller.sip_tables_->lower_seg(aio_index_slot));
	EXPECT_EQ(15, controller.sip_tables_->num_segments(aio_index_slot));

	//interpret the program
	controller.runWorker();
}

TEST(BasicSial,where_clause) {
	std::string job("where_clause");
	std::stringstream out;
	TestController controller(job, false, VERBOSE_TEST, "", out);
	controller.initSipTables();
	controller.runWorker();
	EXPECT_TRUE(controller.worker_->all_stacks_empty());
	int counter = controller.int_value("counter");
	EXPECT_DOUBLE_EQ(10, counter);
}

TEST(BasicSial,ifelse) {
	std::string job("ifelse");
	std::stringstream output;
	TestController controller(job, false, VERBOSE_TEST, "", output);
	controller.initSipTables();
	controller.runWorker();
	EXPECT_TRUE(controller.worker_->all_stacks_empty());
	int eq_counter = controller.int_value("eq_counter");
	int neq_counter = controller.int_value("neq_counter");
	EXPECT_EQ(4, eq_counter);
	EXPECT_EQ(20, neq_counter);
}

TEST(BasicSial,loop_over_simple_indices) {
	std::string job("loop_over_simple_indices");
	std::stringstream expected, output;
	TestController controller(job, false, VERBOSE_TEST, "", output);
	controller.initSipTables();
	controller.runWorker();
	if (attr->global_rank() == 0) {
		EXPECT_EQ(controller.expectedOutput(), output.str());
	}
	EXPECT_TRUE(controller.worker_->all_stacks_empty());
}

/** This function takes lower and upper ranges of indices
 * and runs test programs with all dimensions of these indices.
 * These programs test that the proper number of iterations has
 * been executed.  Jobs are pardo_loop_1d, ..., pardo_loop_6d.
 *
 * A variety of tests can be generated by changing the input params.
 */
//void basic_pardo_test(int max_dims, int lower[], int upper[],
//		bool expect_success = true) {
//	assert(max_dims <= 6);
//	for (int num_dims = 1; num_dims <= 6; ++num_dims) {
//		std::stringstream job_ss;
//		job_ss << "pardo_loop_" << num_dims << "d";
//		std::string job = job_ss.str();
//
//		//total number of iters for this sial program
//		int num_iters = 1;
//		for (int j = 0; j < num_dims; ++j) {
//			num_iters *= ((upper[j] - lower[j]) + 1);
//		}
//
//		//create .dat file
//		if (attr->global_rank() == 0) {
//			init_setup(job.c_str());
//			//add values for upper and lower bounds
//			for (int i = 0; i < num_dims; ++i) {
//				std::stringstream lower_ss, upper_ss;
//				lower_ss << "lower" << i;
//				upper_ss << "upper" << i;
//				set_constant(lower_ss.str().c_str(), lower[i]);
//				set_constant(upper_ss.str().c_str(), upper[i]);
//			}
//			std::string tmp = job + ".siox";
//			const char* nm = tmp.c_str();
//			add_sial_program(nm);
//			finalize_setup();
//		}
//
//		TestController controller(job, true, VERBOSE_TEST,
//				"This is a test of " + job, std::cout, expect_success);
//		controller.initSipTables();
//		controller.runWorker();
//		double total = controller.worker_->scalar_value("total");
//		if (VERBOSE_TEST) {
//			std::cout << "num_iters=" << num_iters << ", total=" << total
//					<< std::endl;
//		}
//		EXPECT_EQ(num_iters, int(total));
//	}
//}



/* This doesn't work.  I haven't checked why.  But it is in the test_sial, so don't really need it here. Consider deleting in cleanup */
//void basic_pardo_test(int max_dims, int lower[], int upper[],
//		bool expect_success = true) {
//	assert(max_dims <= 6);
//	for (int num_dims = 1; num_dims <= 6; ++num_dims) {
//		std::stringstream job_ss;
//		job_ss << "pardo_loop_" << num_dims << "d";
//		std::string job = job_ss.str();
//
//		//total number of iters for this sial program
//		int num_iters = 1;
//		for (int j = 0; j < num_dims; ++j) {
//			num_iters *= ((upper[j] - lower[j]) + 1);
//		}
//
//		//create .dat file
//		if (attr->global_rank() == 0) {
//			init_setup(job.c_str());
//			//add values for upper and lower bounds
//			for (int i = 0; i < num_dims; ++i) {
//				std::stringstream lower_ss, upper_ss;
//				lower_ss << "lower" << i;
//				upper_ss << "upper" << i;
//				set_constant(lower_ss.str().c_str(), lower[i]);
//				set_constant(upper_ss.str().c_str(), upper[i]);
//			}
//			std::string tmp = job + ".siox";
//			const char* nm = tmp.c_str();
//			add_sial_program(nm);
//			finalize_setup();
//		}
//
//#ifdef HAVE_MPI
//
//		TestControllerParallel controller(job, true, VERBOSE_TEST,
//				"This is a test of " + job, std::cout, expect_success);
//#else
//		TestController controller(job, true, VERBOSE_TEST,
//						"This is a test of " + job, std::cout, expect_success);
//#endif
//		controller.initSipTables();
//		controller.runWorker();
//		if (attr->global_rank() == 0) {
//			double total = controller.worker_->scalar_value("total");
//			if (VERBOSE_TEST) {
//				std::cout << "num_iters=" << num_iters << ", total=" << total
//						<< std::endl;
//			}
//			int num_workers = attr->num_workers();
//			if (num_workers < num_iters && num_iters % num_workers == 0){
//				EXPECT_EQ(num_iters / num_workers, int(total));
//			} else if (num_workers < num_iters && num_iters % num_workers != 0){// When work can be somewhat evenly distributed.
//				int per_worker_iters = num_iters / num_workers;
//				int remainder_worker_iters = num_iters % num_workers;
//				ASSERT_TRUE(int(total) == per_worker_iters || int(total) == remainder_worker_iters);
//			} else if (num_iters >= num_workers){
//				ASSERT_TRUE(1 == int(total) || 0 == int(total));
//			}
//		}
//	}
//}
//
//TEST(Sial,pardo_loop) {
//	int MAX_DIMS = 6;
//	int lower[] = { 3, 2, 4, 1, 99, -1 };
//	int upper[] = { 7, 6, 5, 1, 101, 2 };
//	basic_pardo_test(6, lower, upper);
//}
//
//TEST(Sial,pardo_loop_corner_case) {
//	int MAX_DIMS = 6;
//	int lower[] = { 1, 1, 1, 1, 1, 1 };
//	int upper[] = { 1, 1, 1, 1, 1, 1 };
//	basic_pardo_test(6, lower, upper);
//}
//
///*This case should fail with a message "FATAL ERROR: Pardo loop index i5 has empty range at :26"
// * IN addition to the assert throw, the controller constructor, which is in basic_pardo_test needs
// * to be passed false it final parameter, This param has default true, so is omitted in  most tests.
// */
//TEST(Sial,pardo_loop_illegal_range) {
//	int MAX_DIMS = 6;
//	int lower[] = { 1, 1, 1, 1, 1, 2 };
//	int upper[] = { 1, 1, 1, 1, 1, 1 };
//	ASSERT_THROW(basic_pardo_test(6, lower, upper, false), std::logic_error);
//
//}

TEST(BasicSial,scalar_ops) {
	std::string job("scalar_ops");
	std::stringstream out;
	TestController controller(job, false, VERBOSE_TEST, "", out);
	controller.initSipTables();
	controller.runWorker();
	EXPECT_TRUE(controller.worker_->all_stacks_empty());
	EXPECT_DOUBLE_EQ(42.0, controller.scalar_value("l"));
	EXPECT_DOUBLE_EQ(-42.0, controller.scalar_value("nl"));
	EXPECT_DOUBLE_EQ(0.0, controller.scalar_value("s0"));
	EXPECT_DOUBLE_EQ(0.0, controller.scalar_value("si0"));
	EXPECT_DOUBLE_EQ(21.0, controller.scalar_value("sd"));
	EXPECT_DOUBLE_EQ(16, controller.scalar_value("sr0"));
	EXPECT_DOUBLE_EQ(4, controller.scalar_value("sr1"));
	EXPECT_DOUBLE_EQ(16, controller.scalar_value("e0"));
	EXPECT_EQ(4, controller.int_value("ci0"));
	EXPECT_EQ(16, controller.int_value("ci1"));
	EXPECT_EQ(-28, controller.int_value("ci2"));
	EXPECT_DOUBLE_EQ(1, controller.scalar_value("re1"));
	EXPECT_DOUBLE_EQ(-1, controller.scalar_value("re2"));
	EXPECT_DOUBLE_EQ(2, controller.scalar_value("rgt2"));
	EXPECT_DOUBLE_EQ(15, controller.scalar_value("rgt3"));
	EXPECT_DOUBLE_EQ(15, controller.scalar_value("rgt4"));
	EXPECT_DOUBLE_EQ(10, controller.scalar_value("rgt5"));
	EXPECT_DOUBLE_EQ(10, controller.scalar_value("rgt6"));
	EXPECT_DOUBLE_EQ(10, controller.scalar_value("rgt7"));
	EXPECT_DOUBLE_EQ(10, controller.scalar_value("rgt8"));
	EXPECT_DOUBLE_EQ(10, controller.scalar_value("rgt9"));

}

TEST(BasicSial,int_ops) {
	std::string job("int_ops");
	std::stringstream out;
	TestController controller(job, false, VERBOSE_TEST, "", out);
	controller.initSipTables();
	controller.runWorker();
	EXPECT_TRUE(controller.worker_->all_stacks_empty());
	EXPECT_EQ(42.0, controller.int_value("l"));
	EXPECT_EQ(-42, controller.int_value("nl"));
	EXPECT_EQ(0.0, controller.int_value("s0"));
	EXPECT_EQ(0.0, controller.int_value("si0"));
	EXPECT_EQ(21.0, controller.int_value("sd"));
	EXPECT_EQ(10, controller.int_value("sr0"));
	EXPECT_EQ(-2, controller.int_value("sr1"));
	EXPECT_EQ(77, controller.int_value("e0"));
	EXPECT_EQ(4, controller.int_value("ci0"));
	EXPECT_EQ(12, controller.int_value("ci1"));
	EXPECT_EQ(3, controller.int_value("ci2"));
	EXPECT_EQ(1, controller.int_value("re1"));
	EXPECT_EQ(-1, controller.int_value("re2"));
	EXPECT_EQ(2, controller.int_value("rgt2"));
	EXPECT_EQ(15, controller.int_value("rgt3"));
	EXPECT_EQ(15, controller.int_value("rgt4"));
	EXPECT_EQ(10, controller.int_value("rgt5"));
	EXPECT_EQ(10, controller.int_value("rgt6"));
	EXPECT_EQ(10, controller.int_value("rgt7"));
	EXPECT_EQ(15, controller.int_value("rgt8"));
	EXPECT_EQ(10, controller.int_value("rgt9"));

}

TEST(BasicSial,int_self_ops){
	std::string job("int_self_ops");
	std::stringstream out;
	TestController controller(job, false, true, "", out);
	controller.initSipTables();
	controller.runWorker();
	EXPECT_TRUE(controller.worker_->all_stacks_empty());
	EXPECT_EQ(76,controller.int_value("x"));
	EXPECT_EQ(44,controller.int_value("y"));
	EXPECT_EQ(-28,controller.int_value("z"));
	EXPECT_EQ(15200,controller.int_value("w"));
}

TEST(BasicSial,tmp_arrays) {
	std::string job("tmp_arrays");
	std::stringstream output;
	double x = 3.456;
	double y = -0.1;
	int norb = 3;
	if (attr->global_rank() == 0) {
		init_setup(job.c_str());
		set_scalar("x", x);
		set_scalar("y", y);
		set_constant("norb", norb);
		std::string tmp = job + ".siox";
		const char* nm = tmp.c_str();
		add_sial_program(nm);
		int segs[] = { 2, 3, 4 };
		set_aoindex_info(3, segs);
		finalize_setup();
	}
	barrier();
	TestController controller(job, true, VERBOSE_TEST, "", output);
	controller.initSipTables();
	controller.runWorker();
	if (attr->global_rank() == 0) {
		EXPECT_EQ(controller.expectedOutput(), output.str());
	}
	EXPECT_TRUE(controller.worker_->all_stacks_empty());
	EXPECT_EQ(0, controller.worker_->num_blocks_in_blockmap());
}

TEST(BasicSial,block_scale_assign) {
	std::string job("block_scale_assign");
	std::stringstream output;
	double x = 3.456;
	double y = -0.1;
	int norb = 3;
	if (attr->global_rank() == 0) {
		init_setup(job.c_str());
		set_scalar("x", x);
		set_scalar("y", y);
		set_constant("norb", norb);
		std::string tmp = job + ".siox";
		const char* nm = tmp.c_str();
		add_sial_program(nm);
		int segs[] = { 2, 3, 4 };
		set_aoindex_info(3, segs);
		finalize_setup();
	}
	barrier();
	TestController controller(job, true, VERBOSE_TEST, "", output);
	controller.initSipTables();
	controller.runWorker();

	EXPECT_TRUE(controller.worker_->all_stacks_empty());
	EXPECT_EQ(0, controller.worker_->num_blocks_in_blockmap());
}
TEST(BasicSial,scalar_valued_blocks) {
	std::string job("scalar_valued_blocks");
	std::stringstream output;
	TestController controller(job, false, true, "", output);
	controller.initSipTables();
	controller.runWorker();
	if (attr->global_rank() == 0) {
		EXPECT_EQ(controller.expectedOutput(), output.str());
	}
	EXPECT_TRUE(controller.worker_->all_stacks_empty());
	EXPECT_EQ(0, controller.worker_->num_blocks_in_blockmap());
}

TEST(BasicSial,tmp_arrays_2) {
	std::string job("tmp_arrays_2");
	std::stringstream output;
	double x = 3.456;
	double y = -0.1;
	int norb = 3;
	if (attr->global_rank() == 0) {
		init_setup(job.c_str());
		set_scalar("x", x);
		set_scalar("y", y);
		set_constant("norb", norb);
		std::string tmp = job + ".siox";
		const char* nm = tmp.c_str();
		add_sial_program(nm);
		int segs[] = { 2, 3, 4 };
		set_aoindex_info(3, segs);
		finalize_setup();
	}
	barrier();
	TestController controller(job, true, VERBOSE_TEST, "", output);
	controller.initSipTables();
	controller.runWorker();
	if (attr->global_rank() == 0) {
		EXPECT_EQ(controller.expectedOutput(), output.str());
	}
	EXPECT_TRUE(controller.worker_->all_stacks_empty());
	EXPECT_EQ(0, controller.worker_->num_blocks_in_blockmap());
}

TEST(BasicSial,exit_statement_test) {
	std::string job("exit_statement_test");
	sip::DataManager::scope_count = 0;
	double x = 3.456;
	double y = -0.1;
	int norb = 3;
	if (attr->global_rank() == 0) {
		init_setup(job.c_str());
		set_scalar("x", x);
		set_scalar("y", y);
		set_constant("norb", norb);
		std::string tmp = job + ".siox";
		const char* nm = tmp.c_str();
		add_sial_program(nm);
		int segs[] = { 2, 3, 4, 2, 3, 4, 2, 3, 4, 2, 3, 4, 2, 3, 4 };
		set_aoindex_info(15, segs);
		finalize_setup();
	}
	barrier();
	std::stringstream output;
	TestController controller(job, true, VERBOSE_TEST, "", output);
	controller.initSipTables();
	controller.runWorker();
	EXPECT_DOUBLE_EQ(12, controller.int_value("counter_j"));
	EXPECT_DOUBLE_EQ(4, controller.int_value("counter_i"));
	EXPECT_EQ(0, sip::DataManager::scope_count);
	EXPECT_TRUE(controller.worker_->all_stacks_empty());
	EXPECT_EQ(0, controller.worker_->num_blocks_in_blockmap());
}

TEST(BasicSial,transpose_tmp) {
	std::string job("transpose_tmp");
	sip::DataManager::scope_count = 0;

	double x = 3.456;
	double y = -0.1;
	int norb = 3;
	if (attr->global_rank() == 0) {
		init_setup(job.c_str());
		set_scalar("x", x);
		set_scalar("y", y);
		set_constant("norb", norb);
		std::string tmp = job + ".siox";
		const char* nm = tmp.c_str();
		add_sial_program(nm);
		int segs[] = { 8, 12, 10 };
		set_aoindex_info(3, segs);
		finalize_setup();
	}
	barrier();
	std::stringstream output;
	TestController controller(job, true, VERBOSE_TEST, "", output);
	controller.initSipTables();
	controller.runWorker();
	// Get the data for local array block "b"
	int b_slot = controller.worker_->array_slot(std::string("b"));
	sip::index_selector_t b_indices;
	b_indices[0] = 1;
	b_indices[1] = 1;
	for (int i = 2; i < MAX_RANK; i++)
		b_indices[i] = sip::unused_index_value;
	sip::BlockId b_bid(b_slot, b_indices);
	sip::Block::BlockPtr b_bptr = controller.worker_->get_block_for_reading(
			b_bid);
	sip::Block::dataPtr b_data = b_bptr->get_data();
	int passed = test_transpose_op(b_data);
	EXPECT_TRUE(passed);
	EXPECT_EQ(0, sip::DataManager::scope_count);
	EXPECT_TRUE(controller.worker_->all_stacks_empty());
//	EXPECT_GT(0, controller.worker_->num_blocks_in_blockmap());
}

TEST(BasicSial,contraction_small_test) {
	std::string job("contraction_small_test");

	int aoindex_array[] = { 15, 15, 15, 15, 15, 15, 15, 15, 14, 14, 14, 14, 12,
			12 };
	if (attr->global_rank() == 0) {
		init_setup(job.c_str());
		set_aoindex_info(14, aoindex_array);
		std::string tmp = job + ".siox";
		const char* nm = tmp.c_str();
		add_sial_program(nm);
		finalize_setup();
	}
	barrier();
	std::stringstream output;
	TestController controller(job, true, VERBOSE_TEST, "", output);
	controller.initSipTables();
	controller.runWorker();

	// Get the data for local array block "c"
	int c_slot = controller.worker_->array_slot(std::string("c"));
	sip::index_selector_t c_indices;
	c_indices[0] = 1;
	c_indices[1] = 1;
	for (int i = 2; i < MAX_RANK; i++)
		c_indices[i] = sip::unused_index_value;
	sip::BlockId c_bid(c_slot, c_indices);
	std::cout << c_bid << std::endl;
	sip::Block::BlockPtr c_bptr = controller.worker_->get_block_for_reading(
			c_bid);
	sip::Block::dataPtr c_data = c_bptr->get_data();

	// Compare it with the reference
	const int I = 15;
	const int J = 15;
	const int K = 15;
	const int L = 15;

	double a[I][J][K][L];
	double b[J][K];
	double c[I][L];

	int cntr = 0.0;
	for (int i = 0; i < I; i++)
		for (int j = 0; j < J; j++)
			for (int k = 0; k < K; k++)
				for (int l = 0; l < L; l++)
					// behavior of super instruction fill_block_cyclic a(i,j,k,l) 1.0
					a[i][j][k][l] = (cntr++ % 20) + 1;

	for (int i = 0; i < I; i++)
		for (int l = 0; l < L; l++)
			c[i][l] = 0;

	cntr = 0.0;
	for (int j = 0; j < J; j++)
		for (int k = 0; k < K; k++)
			// behavior of super instruction fill_block_cyclic b(j, k) 1.0
			b[j][k] = (cntr++ % 20) + 1;

	for (int i = 0; i < I; i++)
		for (int j = 0; j < J; j++)
			for (int k = 0; k < K; k++)
				for (int l = 0; l < L; l++)
					// c[i][l] needs to be in column major order
					c[l][i] += a[i][j][k][l] * b[j][k];

	for (int i = 0; i < I; i++) {
		for (int l = 0; l < L; l++) {
			EXPECT_DOUBLE_EQ(c[l][i], c_data[i * L + l]);
		}
		std::cout << std::endl;
	}
	EXPECT_TRUE(controller.worker_->all_stacks_empty());

}

/* This test does contraction on GPU is available and implemented */
TEST(BasicSial,contraction_small_test2) {
	std::string job("contraction_small_test2");
	std::cout << "JOBNAME = " << job << std::endl;
	if (attr->global_rank() == 0) {
		init_setup(job.c_str());
		int aosegs[] = { 9 };
		set_aoindex_info(1, aosegs);
		int virtoccsegs[] = { 5, 4 };
		set_moaindex_info(2, virtoccsegs);
		set_constant("baocc", 1);
		set_constant("eaocc", 1);
		set_constant("bavirt", 2);
		set_constant("eavirt", 2);
		std::string tmp = job + ".siox";
		const char* nm = tmp.c_str();
		add_sial_program(nm);
		finalize_setup();
	}
	barrier();
	std::stringstream output;
	TestController controller(job, true, VERBOSE_TEST, "", output);
	controller.initSipTables();
	controller.runWorker();
	//reference calculation
	// Get the data for local array block "c"
	int c_slot = controller.worker_->array_slot(std::string("c"));
	sip::index_selector_t c_indices;
	c_indices[0] = 1;
	c_indices[1] = 1;
	c_indices[2] = 2;
	c_indices[3] = 1;
	for (int i = 4; i < MAX_RANK; i++)
		c_indices[i] = sip::unused_index_value;
	sip::BlockId c_bid(c_slot, c_indices);
	std::cout << c_bid << std::endl;
	sip::Block::BlockPtr c_bptr = controller.worker_->get_block_for_reading(
			c_bid);
	sip::Block::dataPtr c_data = c_bptr->get_data();
	int matched = test_contraction_small2(c_data);
	EXPECT_TRUE(matched);
	EXPECT_TRUE(controller.worker_->all_stacks_empty());

}

TEST(BasicSial,sum_op) { //block addition
	std::string job("sum_op_test");
	double x = 3.456;
	int norb = 2;
	if (attr->global_rank() == 0) {
		init_setup(job.c_str());
		set_scalar("x", x);
		set_constant("norb", norb);
		std::string tmp = job + ".siox";
		const char* nm = tmp.c_str();
		add_sial_program(nm);
		int segs[] = { 20, 5 };
		set_aoindex_info(2, segs);
		finalize_setup();
	}
	barrier();
	std::stringstream output;
	TestController controller(job, true, VERBOSE_TEST, "", output);
	controller.initSipTables();
	controller.runWorker();

	// Reference Calculation
	const int I = 20;
	const int J = 20;

	double a[I][J];
	double c[I][J];
	double d[I][J];
	double e[I][J];

	int cntr = 100.0;
	for (int i = 0; i < I; i++)
		for (int j = 0; j < J; j++)
			// behavior of super instruction fill_block_sequential a(i,j) 100.0
			a[i][j] = cntr++;
	cntr = 50.0;
	for (int i = 0; i < I; i++)
		for (int j = 0; j < J; j++)
			// behavior of super instruction fill_block_sequential c(i,j) 50.0
			c[i][j] = cntr++;

	for (int i = 0; i < I; i++)
		for (int j = 0; j < J; j++)
			d[i][j] = a[i][j] + c[i][j];

	for (int i = 0; i < I; i++)
		for (int j = 0; j < J; j++)
			e[i][j] = d[i][j] - c[i][j];

//		std::cout<<"Reference d"<<std::endl;
//		for (int i=0; i<I; i++){
//				for (int j=0; j<J; j++){
//					std::cout<<d[i][j]<<" ";
//				}
//			std::cout<<std::endl;
//		}
//
//		std::cout<<"Reference e"<<std::endl;
//			for (int i=0; i<I; i++){
//					for (int j=0; j<J; j++){
//						std::cout<<e[i][j]<<" ";
//					}
//				std::cout<<std::endl;
//			}

	// Get the data for local array block "c"
	int d_slot = controller.worker_->array_slot(std::string("d"));
	sip::index_selector_t d_indices;
	d_indices[0] = 1;
	d_indices[1] = 1;
	for (int i = 2; i < MAX_RANK; i++)
		d_indices[i] = sip::unused_index_value;
	sip::BlockId d_bid(d_slot, d_indices);
	std::cout << d_bid << std::endl;
	sip::Block::BlockPtr d_bptr = controller.worker_->get_block_for_reading(
			d_bid);
	sip::Block::dataPtr d_data = d_bptr->get_data();

	int e_slot = controller.worker_->array_slot(std::string("e"));
	sip::index_selector_t e_indices;
	e_indices[0] = 1;
	e_indices[1] = 1;
	for (int i = 2; i < MAX_RANK; i++)
		e_indices[i] = sip::unused_index_value;
	sip::BlockId e_bid(e_slot, e_indices);
	std::cout << e_bid << std::endl;
	sip::Block::BlockPtr e_bptr = controller.worker_->get_block_for_reading(
			e_bid);
	sip::Block::dataPtr e_data = e_bptr->get_data();

	// Compare against reference calculation
	for (int i = 0; i < I; i++)
		for (int j = 0; j < J; j++)
			EXPECT_DOUBLE_EQ(d_data[i * J + j], d[i][j]); // d_data is in column major

	for (int i = 0; i < I; i++)
		for (int j = 0; j < J; j++)
			EXPECT_DOUBLE_EQ(e_data[i * J + j], e[i][j]); // e_data is in column major

}

TEST(BasicSial,static_array_test) { //tests extracting blocks from contiguous array
	std::string job("static_array_test");
	double x = 3.456;
	int norb = 2;
	int segs[] = { 3, 4 };
	if (attr->global_rank() == 0) {
		init_setup(job.c_str());
		set_scalar("x", x);
		set_constant("norb", norb);
		std::string tmp = job + ".siox";
		const char* nm = tmp.c_str();
		add_sial_program(nm);
		set_aoindex_info(2, segs);
		finalize_setup();
	}
	barrier();
	std::stringstream output;
	TestController controller(job, true, VERBOSE_TEST, "", output);
	controller.initSipTables();
	controller.runWorker();
	if (attr->global_rank() == 0)
		EXPECT_EQ(controller.expectedOutput(), output.str());
	EXPECT_TRUE(controller.worker_->all_stacks_empty());
}

TEST(BasicSial,local_arrays) {
	std::string job("local_arrays");
	double x = 3.456;
	int norb = 2;
	if (attr->global_rank() == 0) {
		init_setup(job.c_str());
		set_scalar("x", x);
		set_constant("norb", norb);
		std::string tmp = job + ".siox";
		const char* nm = tmp.c_str();
		add_sial_program(nm);
		int segs[] = { 2, 3 };
		set_aoindex_info(2, segs);
		finalize_setup();
	}
	barrier();
	std::stringstream output;
	TestController controller(job, true, VERBOSE_TEST, "", output);
	controller.initSipTables();
	controller.runWorker();
	if (attr->global_rank() == 0)
		EXPECT_EQ(controller.expectedOutput(), output.str());
	EXPECT_TRUE(controller.worker_->all_stacks_empty());
}

TEST(BasicSial,local_arrays_wild) {

	std::string job("local_arrays_wild");
	double x = 3.456;
	int norb = 2;

	if (attr->global_rank() == 0) {
		init_setup(job.c_str());
		set_scalar("x", x);
		set_constant("norb", norb);
		std::string tmp = job + ".siox";
		const char* nm = tmp.c_str();
		add_sial_program(nm);
		int segs[] = { 2, 3 };
		set_aoindex_info(2, segs);
		finalize_setup();
	}
	barrier();
	std::stringstream output;
	TestController controller(job, true, VERBOSE_TEST, "", output);
	controller.initSipTables();
	controller.runWorker();
	if (attr->global_rank() == 0)
		EXPECT_EQ(controller.expectedOutput(), output.str());
	EXPECT_TRUE(controller.worker_->all_stacks_empty());

}

#ifndef HAVE_MPI
//testing framework cannot gracefully handle errors from MPI processes
TEST(Sial,local_arrays_wild_fail) {

	std::string job("local_arrays_wild_fail");
	double x = 3.456;
	int norb = 2;

	if (attr->global_rank() == 0) {
		init_setup(job.c_str());
		set_scalar("x", x);
		set_constant("norb", norb);
		std::string tmp = job + ".siox";
		const char* nm = tmp.c_str();
		add_sial_program(nm);
		int segs[] = {2, 3};
		set_aoindex_info(2, segs);
		finalize_setup();
	}

	barrier();

	std::stringstream output;
	TestController controller(job, true, VERBOSE_TEST, "", output, false);
	controller.initSipTables();
	if (attr->global_rank() == 0)
	EXPECT_THROW(controller.runWorker();, std::logic_error);
}
#endif

TEST(BasicSial,contract_to_scalar) {
	std::string job("contract_to_scalar");
	int norb = 2;
	if (attr->global_rank() == 0) {
		init_setup(job.c_str());
		set_constant("norb", norb);
		std::string tmp = job + ".siox";
		const char* nm = tmp.c_str();
		add_sial_program(nm);
		int segs[] = { 8, 8 };
		set_aoindex_info(2, segs);
		finalize_setup();
	}
	barrier();
	std::stringstream output;
	TestController controller(job, true, VERBOSE_TEST, "", output);
	controller.initSipTables();
	controller.runWorker();
	double actual_x = controller.worker_->data_manager().scalar_value("x");

// Compare it with the reference
	const int I = 8;
	const int J = 8;

	double a[I][J];
	double b[I][J];
	double ref_x;

	int cntr = 0.0;
	for (int i = 0; i < I; i++)
		for (int j = 0; j < J; j++)
			// behavior of super instruction fill_block_cyclic a(i,j) 1.0
			a[i][j] = (cntr++ % 20) + 1;

	cntr = 4.0;
	for (int i = 0; i < I; i++)
		for (int j = 0; j < J; j++)
			// behavior of super instruction fill_block_cyclic b(i, j) 5.0
			b[i][j] = (cntr++ % 20) + 1;
	ref_x = 0;
	for (int i = 0; i < I; i++)
		for (int j = 0; j < J; j++)
			ref_x += a[i][j] * b[i][j];

	EXPECT_DOUBLE_EQ(ref_x, actual_x);
	std::cout << std::endl;

}

TEST(BasicSial,simple_indices_assignments) {
	std::string job("simple_indices_assignments");
	double x = 3.456;
	int norb = 2;
	if (attr->global_rank() == 0) {
		init_setup(job.c_str());
		set_scalar("x", x);
		set_constant("norb", norb);
		std::string tmp = job + ".siox";
		const char* nm = tmp.c_str();
		add_sial_program(nm);
		int segs[] = { 8, 8 };
		set_aoindex_info(2, segs);
		finalize_setup();
	}
	barrier();
	std::stringstream output;
	TestController controller(job, true, VERBOSE_TEST, "", output);
	controller.initSipTables();
	controller.runWorker();
	EXPECT_DOUBLE_EQ(50, controller.scalar_value("x"));
	EXPECT_DOUBLE_EQ(50, controller.scalar_value("y"));

}

TEST(BasicSial,self_multiply_test) {
	std::string job("self_multiply_test");
	double x = 3.456;
	int norb = 2;
	if (attr->global_rank() == 0) {
		init_setup(job.c_str());
		set_scalar("x", x);
		set_constant("norb", norb);
		std::string tmp = job + ".siox";
		const char* nm = tmp.c_str();
		add_sial_program(nm);
		int segs[] = { 20, 5 };
		set_aoindex_info(2, segs);
		finalize_setup();
	}
	barrier();
	std::stringstream output;
	TestController controller(job, true, VERBOSE_TEST, "", output);
	controller.initSipTables();
	controller.runWorker();
// Reference Calculation
	const int I = 20;
	const int J = 20;
	double a[I][J];
	int cntr = 100.0;
	for (int i = 0; i < I; i++)
		for (int j = 0; j < J; j++)
			// behavior of super instruction fill_block_sequential a(i,j) 100.0
			a[i][j] = cntr++ * 3.0;
// Get the data for local array block "c"
	int a_slot = controller.worker_->array_slot(std::string("a"));
	sip::index_selector_t a_indices;
	a_indices[0] = 1;
	a_indices[1] = 1;
	for (int i = 2; i < MAX_RANK; i++)
		a_indices[i] = sip::unused_index_value;
	sip::BlockId a_bid(a_slot, a_indices);
	std::cout << a_bid << std::endl;
	sip::Block::BlockPtr a_bptr = controller.worker_->get_block_for_reading(
			a_bid);
	sip::Block::dataPtr a_data = a_bptr->get_data();
	for (int i = 0; i < I; i++)
		for (int j = 0; j < J; j++)
			EXPECT_DOUBLE_EQ(a_data[i * J + j], a[i][j]);

}

TEST(SipInterface,get_int_array_test) {
	std::string job("get_int_array_test");
	double x = 3.456;
	int norb = 2;
	if (attr->global_rank() == 0) {
		init_setup(job.c_str());
		set_scalar("x", x);
		set_constant("norb", norb);
		std::string tmp = job + ".siox";
		const char* nm = tmp.c_str();
		add_sial_program(nm);
		int segs[] = { 8, 8 };
		int int_array_data[] = { 1, 2, 3, 4, 5, 6, 7, 8 };
		int length[] = { 8 };
		set_aoindex_info(2, segs);
		set_predefined_integer_array("int_array_data", 1, length,
				int_array_data);
		finalize_setup();
	}
	barrier();
	std::stringstream output;
	TestController controller(job, true, VERBOSE_TEST, "", output);
	controller.initSipTables();
	controller.runWorker();
// Get the data for local array block "c"
	int c_slot = controller.worker_->array_slot(std::string("c"));
	sip::index_selector_t c_indices;
	c_indices[0] = 1;
	c_indices[1] = 1;
	for (int i = 2; i < MAX_RANK; i++)
		c_indices[i] = sip::unused_index_value;
	sip::BlockId c_bid(c_slot, c_indices);
	std::cout << c_bid << std::endl;
	sip::Block::BlockPtr c_bptr = controller.worker_->get_block_for_reading(
			c_bid);
	sip::Block::dataPtr c_data = c_bptr->get_data();

// Compare
	for (int i = 1; i <= 8; i++) {
		EXPECT_DOUBLE_EQ(i, c_data[i - 1]);
	}

}

TEST(SipInterface,get_scalar_array_test) {
	std::string job("get_scalar_array_test");
	std::cout << "JOBNAME = " << job << std::endl;
	double x = 3.456;
	int norb = 2;
	if (attr->global_rank() == 0) {
		init_setup(job.c_str());
		set_scalar("x", x);
		set_constant("norb", norb);
		std::string tmp = job + ".siox";
		const char* nm = tmp.c_str();
		add_sial_program(nm);
		int segs[] = { 8, 8 };
		double scalar_array_data[] = { 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0 };
		int length[] = { 8 };
		set_aoindex_info(2, segs);
		set_predefined_scalar_array("scalar_array_data", 1, length,
				scalar_array_data);
		finalize_setup();
	}
	barrier();
	std::stringstream output;
	TestController controller(job, true, VERBOSE_TEST, "", output);
	controller.initSipTables();
	controller.runWorker();
// Get the data for local array block "c"
	int c_slot = controller.worker_->array_slot(std::string("c"));
	sip::index_selector_t c_indices;
	c_indices[0] = 1;
	c_indices[1] = 1;
	for (int i = 2; i < MAX_RANK; i++)
		c_indices[i] = sip::unused_index_value;
	sip::BlockId c_bid(c_slot, c_indices);
	std::cout << c_bid << std::endl;
	sip::Block::BlockPtr c_bptr = controller.worker_->get_block_for_reading(
			c_bid);
	sip::Block::dataPtr c_data = c_bptr->get_data();

	for (int i = 2; i <= 9; i++) {
		EXPECT_DOUBLE_EQ(i, c_data[i - 2]);
	}

}

TEST(SipInterface,get_scratch_array_test) {
	std::string job("get_scratch_array_test");
	double x = 3.456;
	int norb = 2;
	if (attr->global_rank() == 0) {
		init_setup(job.c_str());
		set_constant("norb", norb);
		std::string tmp = job + ".siox";
		const char* nm = tmp.c_str();
		add_sial_program(nm);
		int segs[] = { 32, 8 };
		int length[] = { 8 };
		set_aoindex_info(2, segs);
		finalize_setup();
	}
	barrier();
	std::stringstream output;
	TestController controller(job, true, VERBOSE_TEST, "", output);
	controller.initSipTables();
	controller.runWorker();
// Get the data for local array block "c"
	int c_slot = controller.worker_->array_slot(std::string("c"));
	sip::index_selector_t c_indices;
	c_indices[0] = 1;
	c_indices[1] = 1;
	for (int i = 2; i < MAX_RANK; i++)
		c_indices[i] = sip::unused_index_value;
	sip::BlockId c_bid(c_slot, c_indices);
	std::cout << c_bid << std::endl;
	sip::Block::BlockPtr c_bptr = controller.worker_->get_block_for_reading(
			c_bid);
	sip::Block::dataPtr c_data = c_bptr->get_data();

	for (int i = 1; i <= 32; i++) {
		EXPECT_DOUBLE_EQ(i, c_data[i - 1]);
	}

}

TEST(BasicSial,transpose4d_tmp) {
	std::string job("transpose4d_tmp");
	double x = 3.456;
	double y = -0.1;
	int norb = 3;
	if (attr->global_rank() == 0) {
		init_setup(job.c_str());
		set_scalar("x", x);
		set_scalar("y", y);
		set_constant("norb", norb);
		std::string tmp = job + ".siox";
		const char* nm = tmp.c_str();
		add_sial_program(nm);
		int virtoccsegs[] = { 1, 5, 4 };
		set_moaindex_info(3, virtoccsegs);
		set_constant("baocc", 1);
		set_constant("eaocc", 1);
		set_constant("bavirt", 2);
		set_constant("eavirt", 2);
		finalize_setup();
	}
	barrier();
	std::stringstream output;
	TestController controller(job, true, VERBOSE_TEST, "", output);
	controller.initSipTables();
	controller.runWorker();
// Get the data for local array block "b"
	int b_slot = controller.worker_->array_slot(std::string("b"));
	sip::index_selector_t b_indices;
	b_indices[0] = 2;
	b_indices[1] = 1;
	b_indices[2] = 2;
	b_indices[3] = 1;
	for (int i = 4; i < MAX_RANK; i++)
		b_indices[i] = sip::unused_index_value;
	sip::BlockId b_bid(b_slot, b_indices);
	sip::Block::BlockPtr b_bptr = controller.worker_->get_block_for_reading(
			b_bid);
	sip::Block::dataPtr b_data = b_bptr->get_data();
	int passed = test_transpose4d_op(b_data);
	EXPECT_TRUE(passed);

}

TEST(BasicSial,transpose4d_square_tmp) {
	std::string job("transpose4d_square_tmp");
	double x = 3.456;
	double y = -0.1;
	int norb = 3;
	if (attr->global_rank() == 0) {
		init_setup(job.c_str());
		set_scalar("x", x);
		set_scalar("y", y);
		set_constant("norb", norb);
		std::string tmp = job + ".siox";
		const char* nm = tmp.c_str();
		add_sial_program(nm);
		int segs[] = { 8, 8, 8 };
		set_aoindex_info(3, segs);
		finalize_setup();
	}
	barrier();
	std::stringstream output;
	TestController controller(job, true, VERBOSE_TEST, "", output);
	controller.initSipTables();
	controller.runWorker();
// Get the data for local array block "b"
	int b_slot = controller.worker_->array_slot(std::string("b"));
	sip::index_selector_t b_indices;
	b_indices[0] = 1;
	b_indices[1] = 1;
	for (int i = 2; i < MAX_RANK; i++)
		b_indices[i] = sip::unused_index_value;
	sip::BlockId b_bid(b_slot, b_indices);
	sip::Block::BlockPtr b_bptr = controller.worker_->get_block_for_reading(
			b_bid);
	sip::Block::dataPtr b_data = b_bptr->get_data();
	const int I = 8;
	const int J = 8;
	const int K = 8;
	const int L = 8;
	double a[I][J][K][L];
	double b[K][J][I][L];
	double esum1 = 0.0;
	double esum2 = 0.0;
	double esum3 = 0.0;

// Fill A
	int counter = 0.0;
	for (int i = 0; i < I; i++)
		for (int j = 0; j < J; j++)
			for (int k = 0; k < K; k++)
				for (int l = 0; l < L; l++)
					a[i][j][k][l] = (counter++ % 20) + 1;

// Fill B
	for (int i = 0; i < I; i++)
		for (int j = 0; j < J; j++)
			for (int k = 0; k < K; k++)
				for (int l = 0; l < L; l++)
					b[k][j][i][l] = a[i][j][k][l];

// Calculate esum1 & esum2;
	for (int i = 0; i < I; i++)
		for (int j = 0; j < J; j++)
			for (int k = 0; k < K; k++)
				for (int l = 0; l < L; l++) {
					esum1 += a[i][j][k][l] * a[i][j][k][l];
					esum2 += b[k][j][i][l] * b[k][j][i][l];
					esum3 += a[i][j][k][l] * b[k][j][i][l];
				}

	double actual_esum1 = controller.scalar_value("esum1");
	double actual_esum2 = controller.scalar_value("esum2");
	double actual_esum3 = controller.scalar_value("esum3");

	EXPECT_DOUBLE_EQ(esum1, actual_esum1);
	EXPECT_DOUBLE_EQ(esum2, actual_esum2);
	EXPECT_DOUBLE_EQ(esum3, actual_esum3);

}
//TODO  Add result check
TEST(BasicSial,assign_to_static_array_test) {
	std::string job("assign_to_static_array_test");
	int norb = 3;
	{
		init_setup(job.c_str());
		set_constant("norb", norb);
		std::string tmp = job + ".siox";
		const char* nm = tmp.c_str();
		add_sial_program(nm);
		int segs[] = { 2, 2, 1 };
		set_aoindex_info(3, segs);
		finalize_setup();
	}
	barrier();
	std::stringstream output;
	TestController controller(job, true, true, "", output);
	controller.initSipTables();
	controller.runWorker();
}

TEST(BasicSial,return_sval_test){
	std::string job("return_sval_test");
	std::stringstream output;
	TestController controller(job, false, true, "", output);
	controller.initSipTables();
	controller.runWorker();
}

TEST(BasicSial,index_scalar_cast){
	std::string job("index_scalar_cast");
	int norb = 4;
	{
	init_setup(job.c_str());
	set_constant("norb", norb);
	std::string tmp = job + ".siox";
	const char* nm = tmp.c_str();
	add_sial_program(nm);
	int segs[] = { 2, 2, 1 , 3};
	set_aoindex_info(4, segs);
	finalize_setup();
	}
	barrier();
	std::stringstream output;
	TestController controller(job, true, VERBOSE_TEST, "", output);
	controller.initSipTables();
	controller.runWorker();
	int count = controller.int_value("count");
	int count2 = controller.int_value("count2");
	EXPECT_EQ(norb, count);
	EXPECT_EQ(1, count2);
}
//****************************************************************************************************************

void bt_sighandler(int signum) {
	std::cerr << "Interrupt signal (" << signum << ") received." << std::endl;
	FAIL();
	abort();
}

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
		return -1;
	}
};

int main(int argc, char **argv) {

//    feenableexcept(FE_DIVBYZERO);
//    feenableexcept(FE_OVERFLOW);
//    feenableexcept(FE_INVALID);
//
//    signal(SIGSEGV, bt_sighandler);
//    signal(SIGFPE, bt_sighandler);
//    signal(SIGTERM, bt_sighandler);
//    signal(SIGINT, bt_sighandler);
//    signal(SIGABRT, bt_sighandler);

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
#ifdef HAVE_TAU
	TAU_PROFILE_SET_NODE(0);
	TAU_STATIC_PHASE_START("SIP Main");
#endif

//	sip::check(sizeof(int) >= 4, "Size of integer should be 4 bytes or more");
//	sip::check(sizeof(double) >= 8, "Size of double should be 8 bytes or more");
//	sip::check(sizeof(long long) >= 8, "Size of long long should be 8 bytes or more");
//
	printf("Running main() from %s\n",__FILE__);
	testing::InitGoogleTest(&argc, argv);
	barrier();
	int result = RUN_ALL_TESTS();

#ifdef HAVE_TAU
	TAU_STATIC_PHASE_STOP("SIP Main");
#endif
	barrier();
#ifdef HAVE_MPI
	MPI_Finalize();
#endif
	return result;

}
