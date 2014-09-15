#include "gtest/gtest.h"
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

#include "block.h"

#ifdef HAVE_TAU
#include <TAU.h>
#endif


//bool VERBOSE_TEST = false;
bool VERBOSE_TEST = true;


TEST(Sial_QM,ccsdpt_test){
	std::string job("ccsdpt_test");

	std::stringstream output;
#ifdef HAVE_MPI
	TestControllerParallel controller(job, true, VERBOSE_TEST, "", output);
#else
	TestController controller (job, true, VERBOSE_TEST, "", output);
#endif

	controller.initSipTables(qm_dir_name);
	controller.run();

	controller.initSipTables(qm_dir_name);
	controller.run();

	controller.initSipTables(qm_dir_name);
	controller.run();

	controller.initSipTables(qm_dir_name);
	controller.run();

	controller.initSipTables(qm_dir_name);
	controller.run();

	if (attr->global_rank() == 0) {
		double eaab = controller.scalar_value("eaab");
		ASSERT_NEAR(-0.0010909776247261387949, eaab, 1e-10);
		double esaab =controller.scalar_value("esaab");
		ASSERT_NEAR(8.5548065773238419758e-05, esaab, 1e-10);
	}
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
