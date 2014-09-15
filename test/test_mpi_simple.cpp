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

#include "worker_persistent_array_manager.h"
#include "server_persistent_array_manager.h"

#include "block.h"

#ifdef HAVE_TAU
#include <TAU.h>
#endif

//#ifdef HAVE_MPI
//#include "sip_server.h"
//#include "sip_mpi_attr.h"
//#include "global_state.h"
//#include "sip_mpi_utils.h"
//#else
//#include "sip_attr.h"
//#endif

#include "test_controller.h"

extern "C"{
int test_transpose_op(double*);
int test_transpose4d_op(double*);
int test_contraction_small2(double*);
}

//bool VERBOSE_TEST = false;
bool VERBOSE_TEST = true;







TEST(BasicSial,DISABLED_exit_statement_test){
	std::string job("exit_statement_test");
	sip::DataManager::scope_count=0;
	double x = 3.456;
	double y = -0.1;
	int norb = 3;
	{
		init_setup(job.c_str());
		set_scalar("x",x);
		set_scalar("y",y);
		set_constant("norb",norb);
		std::string tmp = job + ".siox";
		const char* nm= tmp.c_str();
		add_sial_program(nm);
		int segs[]  = {2,3,4,2,3,4,2,3,4,2,3,4,2,3,4};
		set_aoindex_info(15,segs);
		finalize_setup();
	}
	std::stringstream output;
	TestControllerParallel controller(job, true, VERBOSE_TEST, "", output);
	controller.initSipTables();
	controller.run();
	if(attr->is_worker()){
	EXPECT_DOUBLE_EQ(12, controller.int_value("counter_j"));
	EXPECT_DOUBLE_EQ(4, controller.int_value("counter_i"));
	EXPECT_EQ(0, sip::DataManager::scope_count);
	EXPECT_TRUE(controller.worker_->all_stacks_empty());
	EXPECT_EQ(0, controller.worker_->num_blocks_in_blockmap());
	}
}




///* This test does contraction on GPU is available and implemented */
//TEST(BasicSial,DISABLED_contraction_small_test2){
//	std::string job("contraction_small_test2");
//	std::cout << "JOBNAME = " << job << std::endl;
//	{
//		init_setup(job.c_str());
//		int aosegs[]  = {9};
//		set_aoindex_info(1,aosegs);
//		int virtoccsegs[] ={5, 4};
//		set_moaindex_info(2, virtoccsegs);
//		set_constant("baocc", 1);
//		set_constant("eaocc", 1);
//		set_constant("bavirt", 2);
//		set_constant("eavirt", 2);
//		std::string tmp = job + ".siox";
//		const char* nm= tmp.c_str();
//		add_sial_program(nm);
//		finalize_setup();
//	}
//	std::stringstream output;
//	TestController controller(job, true, VERBOSE_TEST, "", output);
//	controller.initSipTables();
//	controller.runWorker();
//	//reference calculation
//		// Get the data for local array block "c"
//		int c_slot = controller.worker_->array_slot(std::string("c"));
//		sip::index_selector_t c_indices;
//		c_indices[0] = 1;
//		c_indices[1] = 1;
//		c_indices[2] = 2;
//		c_indices[3] = 1;
//		for (int i = 4; i < MAX_RANK; i++) c_indices[i] = sip::unused_index_value;
//		sip::BlockId c_bid(c_slot, c_indices);
//		std::cout << c_bid << std::endl;
//		sip::Block::BlockPtr c_bptr = controller.worker_->get_block_for_reading(c_bid);
//		sip::Block::dataPtr c_data = c_bptr->get_data();
//		int matched = test_contraction_small2(c_data);
//		EXPECT_TRUE(matched);
//		EXPECT_TRUE(controller.worker_->all_stacks_empty());
//
//}


TEST(Sial,put_test) {
	barrier();
	std::string job("put_test");
	int norb = 3;
	int segs[] = {2,3,2};
	if (attr->global_rank() == 0) {
		init_setup(job.c_str());
		set_constant("norb", norb);
		set_constant("norb_squared", norb*norb);
		std::string tmp = job + ".siox";
		const char* nm = tmp.c_str();
		add_sial_program(nm);
		set_aoindex_info(3, segs);
		finalize_setup();
	}
	barrier();
	std::stringstream output;
		TestControllerParallel controller(job, true, VERBOSE_TEST, " ", output);
		controller.initSipTables();
		if (attr->is_worker()) {
			std::cout << "creating and starting a worker" << std::endl << std::flush;
			controller.runWorker();
			EXPECT_TRUE(controller.worker_->all_stacks_empty());
			std::vector<int> index_vec;
			for (int i = 0; i < norb; ++i){
				for (int j = 0; j < norb; ++j){
					int k = (i*norb + j)+1;
					index_vec.push_back(k);
					double * local_block = controller.local_block("result",index_vec);
					double value = local_block[0];
					double expected = k*k*segs[i]*segs[j];
					std::cout << "k,value= " << k << " " << value << std::endl;
					ASSERT_DOUBLE_EQ(expected, value);
					index_vec.clear();
				}
			}
		} else {
			std::cout << "creating and starting a server" << std::endl << std::flush;
			controller.runServer();
		}


}

TEST(SimpleMPI,persistent_distributed_array_mpi){

	sip::GlobalState::reset_program_count();
	sip::SIPMPIAttr &sip_mpi_attr = sip::SIPMPIAttr::get_instance();
	int my_rank = sip_mpi_attr.global_rank();

	std::cout << "****************************************\n";
	sip::DataManager::scope_count=0;
	//create setup_file
	std::string job("persistent_distributed_array_mpi");
	std::cout << "JOBNAME = " << job << std::endl;
	double x = 3.456;
	int norb = 2;
	int segs[]  = {2,3};


	if (attr.global_rank() == 0){
		init_setup(job.c_str());
		set_scalar("x",x);
		set_constant("norb",norb);
		std::string tmp = job + "1.siox";
		const char* nm= tmp.c_str();
		add_sial_program(nm);
		std::string tmp1 = job + "2.siox";
		const char* nm1= tmp1.c_str();
		add_sial_program(nm1);
		set_aoindex_info(2,segs);
		finalize_setup();
	}

	sip::SIPMPIUtils::check_err(MPI_Barrier(MPI_COMM_WORLD));

	setup::BinaryInputFile setup_file(job + ".dat");
	setup::SetupReader setup_reader(setup_file);

	std::cout << "SETUP READER DATA:\n" << setup_reader<< std::endl;


	//get siox name from setup, load and print the sip tables
	std::string prog_name = setup_reader.sial_prog_list_.at(0);
	std::string siox_dir(dir_name);
	setup::BinaryInputFile siox_file(siox_dir + prog_name);
	sip::SipTables sipTables(setup_reader, siox_file);
	if (!sip_mpi_attr.is_server()) {std::cout << "SIP TABLES" << '\n' << sipTables << std::endl;}

	if (sip_mpi_attr.global_rank()==0){   std::cout << "\n\n\n\n>>>>>>>>>>>>starting SIAL PROGRAM  "<< job << std::endl;}

	//create worker and server
	sip::DataDistribution data_distribution(sipTables, sip_mpi_attr);
	sip::GlobalState::set_program_name(prog_name);
	sip::GlobalState::increment_program();
	sip::WorkerPersistentArrayManager wpam;
	sip::ServerPersistentArrayManager spam;

	std::cout << "rank " << my_rank << " reached first barrier" << std::endl << std::flush;
	MPI_Barrier(MPI_COMM_WORLD);
	std::cout << "rank " << my_rank << " passed first barrier" << std::endl << std::flush;

	if (sip_mpi_attr.is_server()){
		sip::SIPServer server(sipTables, data_distribution, sip_mpi_attr, &spam);
		std::cout << "at first barrier in prog 1 at server" << std::endl << std::flush;
		MPI_Barrier(MPI_COMM_WORLD);
		std::cout<<"passed first barrier at server, starting server" << std::endl;
		server.run();
		spam.save_marked_arrays(&server);
		std::cout << "Server state after termination" << server << std::endl;
	} else {
		sip::SialxTimer sialxTimer(sipTables.max_timer_slots());
		sip::Interpreter runner(sipTables, sialxTimer,  &wpam);
		std::cout << "at first barrier in prog 1 at worker" << std::endl << std::flush;
		MPI_Barrier(MPI_COMM_WORLD);
		std::cout << "after first barrier; starting worker for "<< job  << std::endl;
		runner.interpret();
		wpam.save_marked_arrays(&runner);
		std::cout << "\n end of prog1 at worker"<< std::endl;

	}

	std::cout << std::flush;
	if (sip_mpi_attr.global_rank()==0){
		std::cout << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@" << std::endl << std::flush;
		std::cout << "SETUP READER DATA FOR SECOND PROGRAM:\n" << setup_reader<< std::endl;
	}

	std::string prog_name2 = setup_reader.sial_prog_list_.at(1);
	setup::BinaryInputFile siox_file2(siox_dir + prog_name2);
	sip::SipTables sipTables2(setup_reader, siox_file2);

	if (sip_mpi_attr.global_rank()==0){
		std::cout << "SIP TABLES FOR " << prog_name2 << '\n' << sipTables2 << std::endl;
	}

	sip::DataDistribution data_distribution2(sipTables2, sip_mpi_attr);
	sip::GlobalState::set_program_name(prog_name);
	sip::GlobalState::increment_program();
	std::cout << "rank " << my_rank << " reached second barrier in test" << std::endl << std::flush;
	MPI_Barrier(MPI_COMM_WORLD);
	std::cout << "rank " << my_rank << " passed second barrier in test" << std::endl << std::flush;

	if (sip_mpi_attr.is_server()){
		sip::SIPServer server(sipTables2, data_distribution2, sip_mpi_attr, &spam);
		std::cout << "barrier in prog 2 at server" << std::endl << std::flush;
		MPI_Barrier(MPI_COMM_WORLD);
		std::cout<< "rank " << my_rank << "starting server for prog 2" << std::endl;
		server.run();
		std::cout<< "rank " << my_rank  << "Server state after termination of prog2" << server << std::endl;
	} else {
		sip::SialxTimer sialxTimer2(sipTables2.max_timer_slots());
		sip::Interpreter runner(sipTables2, sialxTimer2,  &wpam);
		std::cout << "rank " << my_rank << "barrier in prog 2 at worker" << std::endl << std::flush;
		MPI_Barrier(MPI_COMM_WORLD);
		std::cout << "rank " << my_rank << "starting worker for prog2"<< job  << std::endl;
		runner.interpret();
		std::cout << "\nSIAL PROGRAM 2 TERMINATED"<< std::endl;


		// Test contents of blocks of distributed array "b"

		// Get the data for local array block "b"
		int b_slot = runner.array_slot(std::string("lb"));

		// Test b(1,1)
		sip::index_selector_t b_indices_1;
		b_indices_1[0] = 1; b_indices_1[1] = 1;
		for (int i = 2; i < MAX_RANK; i++) b_indices_1[i] = sip::unused_index_value;
		sip::BlockId b_bid_1(b_slot, b_indices_1);
		std::cout << b_bid_1 << std::endl;
		sip::Block::BlockPtr b_bptr_1 = runner.get_block_for_reading(b_bid_1);
		sip::Block::dataPtr b_data_1 = b_bptr_1->get_data();
		std::cout << " Comparing block " << b_bid_1 << std::endl;
		double fill_seq_1_1 = 1.0;
		for (int i=0; i<segs[0]; i++){
			for (int j=0; j<segs[0]; j++){
				ASSERT_DOUBLE_EQ(fill_seq_1_1, b_data_1[i*segs[0] + j]);
				fill_seq_1_1++;
			}
		}

		// Test b(2, 2)
		sip::index_selector_t b_indices_2;
		b_indices_2[0] = 2; b_indices_2[1] = 2;
		for (int i = 2; i < MAX_RANK; i++) b_indices_2[i] = sip::unused_index_value;
		sip::BlockId b_bid_2(b_slot, b_indices_2);
		std::cout << b_bid_2 << std::endl;
		sip::Block::BlockPtr b_bptr_2 = runner.get_block_for_reading(b_bid_2);
		sip::Block::dataPtr b_data_2 = b_bptr_2->get_data();
		std::cout << " Comparing block " << b_bid_2 << std::endl;
		double fill_seq_2_2 = 4.0;
		for (int i=0; i<segs[1]; i++){
			for (int j=0; j<segs[1]; j++){
				ASSERT_DOUBLE_EQ(fill_seq_2_2, b_data_2[i*segs[1] + j]);
				fill_seq_2_2++;
			}
		}

		// Test b(2,1)
		sip::index_selector_t b_indices_3;
		b_indices_3[0] = 2; b_indices_3[1] = 1;
		for (int i = 2; i < MAX_RANK; i++) b_indices_3[i] = sip::unused_index_value;
		sip::BlockId b_bid_3(b_slot, b_indices_3);
		std::cout << b_bid_3 << std::endl;
		sip::Block::BlockPtr b_bptr_3 = runner.get_block_for_reading(b_bid_3);
		sip::Block::dataPtr b_data_3 = b_bptr_3->get_data();
		std::cout << " Comparing block " << b_bid_3 << std::endl;
		double fill_seq_2_1 = 3.0;
		for (int i=0; i<segs[1]; i++){
			for (int j=0; j<segs[0]; j++){
				ASSERT_DOUBLE_EQ(fill_seq_2_1, b_data_3[i*segs[0] + j]);
				fill_seq_2_1++;
			}
		}


	}

	std::cout << "rank " << my_rank << " reached third barrier in test" << std::endl << std::flush;
	MPI_Barrier(MPI_COMM_WORLD);
	std::cout << "rank " << my_rank << " passed third barrier in test" << std::endl << std::flush;

}




//****************************************************************************************************************

void bt_sighandler(int signum) {
	std::cerr << "Interrupt signal (" << signum << ") received." << std::endl;
    FAIL();
    abort();
}


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
	sip::SIPMPIUtils::set_error_handler();
	sip::SIPMPIAttr &sip_mpi_attr = sip::SIPMPIAttr::get_instance();
	attr = &sip_mpi_attr;
#endif

#ifdef HAVE_TAU
	TAU_PROFILE_SET_NODE(0);
	TAU_STATIC_PHASE_START("SIP Main");
#endif

//	sip::check(sizeof(int) >= 4, "Size of integer should be 4 bytes or more");
//	sip::check(sizeof(double) >= 8, "Size of double should be 8 bytes or more");
//	sip::check(sizeof(long long) >= 8, "Size of long long should be 8 bytes or more");
//
//	int num_procs;
//	sip::SIPMPIUtils::check_err(MPI_Comm_size(MPI_COMM_WORLD, &num_procs));
//
//	if (num_procs < 2){
//		std::cerr<<"Please run this test with at least 2 mpi ranks"<<std::endl;
//		return -1;
//	}
//
//	sip::SIPMPIUtils::set_error_handler();
//	sip::SIPMPIAttr &sip_mpi_attr = sip::SIPMPIAttr::get_instance();
//

	printf("Running main() from test_simple.cpp\n");
	testing::InitGoogleTest(&argc, argv);
	int result = RUN_ALL_TESTS();

#ifdef HAVE_TAU
	TAU_STATIC_PHASE_STOP("SIP Main");
#endif
#ifdef HAVE_MPI
	MPI_Finalize();
#endif
	return result;

}
