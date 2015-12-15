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


//bool VERBOSE_TEST = false;
bool VERBOSE_TEST = true;

TEST(Sial_QM_FRAG,DISABLED_mcpt2_test){
	std::string job("mcpt2_test");

	std::stringstream output;

	TestControllerParallel controller(job, true, VERBOSE_TEST, "", output);
//
// mcpt
	controller.initSipTables(qm_dir_name);
	controller.run();

	if (attr->global_rank() == 0) {
		double e1x_at = controller.scalar_value("e1x_at");
		ASSERT_NEAR(0.01026999465441, e1x_at, 1e-10);
		double e10pol_at = controller.scalar_value("e10pol_at");
		ASSERT_NEAR(-0.01868181159399, e10pol_at, 1e-10);
		double singles = controller.scalar_value("singles");
		ASSERT_NEAR(-0.00106726385203, singles, 1e-10);
		double dimer_doubles = controller.scalar_value("dimer_doubles");
		ASSERT_NEAR(-0.00055251119550, dimer_doubles, 1e-10);
		double fragment_doubles = controller.scalar_value("fragment_doubles");
		ASSERT_NEAR(-0.25081012125402, fragment_doubles, 1e-10);
		double mono_lccd = controller.scalar_value("mono_lccd");
		ASSERT_NEAR(0.25084388836779, mono_lccd, 1e-10);
	}

}


/*
 water dimer
 O -0.00000007     0.06307336     0.00000000
 H -0.75198755    -0.50051034     0.00000000
 H  0.75198873    -0.50050946     0.00000000
 O -0.00000007     0.06307336   5.00000000
 H -0.75198755    -0.50051034   5.00000000
 H  0.75198873    -0.50050946   5.00000000

 *ACES2(BASIS=3-21G
 scf_conv=12
 cc_conv=12
 estate_tol=8
 estate_sym=4
 spherical=off
 excite=eomee
 symmetry=off
 NOREORI=ON
 CALC=ccsd)

 *SIP
 MAXMEM=120000
 SIAL_PROGRAM = mcpt2_corr_lowmem.siox

 *FRAGLIST
 2 10.0 10.0
 3 3
 1 2 3
 4 5 6
*/
TEST(Sial_QM_FRAG,mcpt2_water_test){
	std::string job("mcpt2_water_test");

	std::stringstream output;

	TestControllerParallel controller(job, true, VERBOSE_TEST, "", output);
//
// mcpt
	controller.initSipTables(qm_dir_name);
	controller.run();

	if (attr->global_rank() == 0) {
		double e1exc_at = controller.scalar_value("e1exc_at");
		ASSERT_NEAR(-0.00002319025225, e1exc_at, 1e-10);
		double e10pol_at = controller.scalar_value("e10pol_at");
		ASSERT_NEAR(0.00590428495666, e10pol_at, 1e-10);
		double singles = controller.scalar_value("singles");
		ASSERT_NEAR(-0.00012759140498, singles, 1e-10);
		double dimer_doubles = controller.scalar_value("dimer_doubles");
		ASSERT_NEAR(-0.00013637010071, dimer_doubles, 1e-10);
		double fragment_doubles = controller.scalar_value("fragment_doubles");
		ASSERT_NEAR(-0.25554107195002, fragment_doubles, 1e-10);
		double mono_lccd = controller.scalar_value("mono_lccd");
		ASSERT_NEAR(0.255547494689395, mono_lccd, 1e-10);
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
#endif
	barrier();


	printf("Running main() from test_qm_frag.cpp\n");
	testing::InitGoogleTest(&argc, argv);
	barrier();
	int result = RUN_ALL_TESTS();

	std::cout << "Rank  " << attr->global_rank() << " Finished RUN_ALL_TEST() " << std::endl << std::flush;

	barrier();
#ifdef HAVE_MPI
	MPI_Finalize();
#endif
	return result;

}



