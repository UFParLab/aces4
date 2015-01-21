#include "gtest/gtest.h"
#include <fstream>

#include "siox_reader.h"
#include "io_utils.h"
#include "setup_reader.h"

#include "sip_tables.h"
#include "interpreter.h"
#include "setup_interface.h"
#include "sip_interface.h"
#include "data_manager.h"
#include "test_controller.h"
#include "test_controller_parallel.h"
#include "test_constants.h"
#include "sip_mpi_attr.h"


#include "block.h"

#ifdef HAVE_TAU
#include <TAU.h>
#endif


//bool VERBOSE_TEST = false;
bool VERBOSE_TEST = true;




TEST(Sial_QM,ccsdpt_test){
	std::string job("ccsdpt_test");

	std::stringstream sial_output;
	std::ofstream instrumentation_output;
	if(attr->is_worker() && attr->is_company_master()){ //only open the file at master worker
		std::stringstream ss1;
		ss1 << "inst_"<< job << ".csv";
		std::string filename1 = ss1.str();
		instrumentation_output.open(filename1.c_str(),std::ofstream::out);
	}


	TestControllerParallel controller(job, true, VERBOSE_TEST, "", sial_output);

	controller.initSipTables(qm_dir_name);
	controller.run();
	controller.gather_opcode_histogram(instrumentation_output);
	controller.gather_pc_histogram(instrumentation_output);
	controller.gather_timing(instrumentation_output);


	controller.initSipTables(qm_dir_name);
	controller.run();
	controller.gather_opcode_histogram(instrumentation_output);
	controller.gather_pc_histogram(instrumentation_output);
	controller.gather_timing(instrumentation_output);

	controller.initSipTables(qm_dir_name);
	controller.run();
	controller.gather_opcode_histogram(instrumentation_output);
	controller.gather_pc_histogram(instrumentation_output);
	controller.gather_timing(instrumentation_output);

	controller.initSipTables(qm_dir_name);
	controller.run();
	controller.gather_opcode_histogram(instrumentation_output);
	controller.gather_pc_histogram(instrumentation_output);
	controller.gather_timing(instrumentation_output);

	controller.initSipTables(qm_dir_name);
	controller.run();	std::stringstream ss4;
	controller.gather_opcode_histogram(instrumentation_output);
	controller.gather_pc_histogram(instrumentation_output);
	controller.gather_timing(instrumentation_output);

	if (attr->global_rank() == 0) {
		double eaab = controller.scalar_value("eaab");
		ASSERT_NEAR(-0.0010909776247261387949, eaab, 1e-10);
		double esaab =controller.scalar_value("esaab");
		ASSERT_NEAR(8.5548065773238419758e-05, esaab, 1e-10);
//		controller.print_timers(std::cout);
	}

}
//****************************************************************************************************************

//void bt_sighandler(int signum) {
//	std::cerr << "Interrupt signal (" << signum << ") received." << std::endl;
//	FAIL();
//	abort();
//}

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
	sip::SIPMPIUtils::check_err(MPI_Comm_size(MPI_COMM_WORLD, &num_procs), __LINE__,__FILE__);

	if (num_procs < 2) {
		std::cerr << "Please run this test with at least 2 mpi ranks"
				<< std::endl;
		return -1;
	}
	sip::SIPMPIUtils::set_error_handler();
	sip::SIPMPIAttr &sip_mpi_attr = sip::SIPMPIAttr::get_instance();
	attr = &sip_mpi_attr;
	double tick = MPI_Wtick();
	    printf("A single MPI tick is %0.9f seconds\n", tick);
	    fflush(stdout);
#endif
	barrier();
#ifdef HAVE_TAU
	TAU_PROFILE_SET_NODE(0);
	TAU_STATIC_PHASE_START("SIP Main");
#endif


	printf("Running main() from test_sial.cpp\n");
	testing::InitGoogleTest(&argc, argv);
	barrier();
	int result = RUN_ALL_TESTS();

	std::cout << "Rank  " << attr->global_rank() << " Finished RUN_ALL_TEST() " << std::endl << std::flush;

#ifdef HAVE_TAU
	TAU_STATIC_PHASE_STOP("SIP Main");
#endif
	barrier();
#ifdef HAVE_MPI
	MPI_Finalize();
#endif
	return result;

}



