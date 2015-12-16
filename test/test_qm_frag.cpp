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
O  -0.001684     1.516645     0.000000
H  -0.905818     1.830641     0.000000
H  -0.082578     0.556134     0.000000
O  -0.001684    -1.393774     0.000000
H   0.507668    -1.684875     0.758561
H   0.507668    -1.684875    -0.758561

*ACES2(BASIS=3-21G
scf_conv=12
cc_conv=12
estate_tol=1
estate_sym=1
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
		ASSERT_NEAR(0.00392118115294, e1exc_at, 1e-10);
		double e10pol_at = controller.scalar_value("e10pol_at");
		ASSERT_NEAR(-0.01393567927922, e10pol_at, 1e-10);
		double singles = controller.scalar_value("singles");
		ASSERT_NEAR(-0.00075493885372, singles, 1e-10);
		double dimer_doubles = controller.scalar_value("dimer_doubles");
		ASSERT_NEAR(-0.00048177099748, dimer_doubles, 1e-10);
		double fragment_doubles = controller.scalar_value("fragment_doubles");
		ASSERT_NEAR(-0.25889198174987, fragment_doubles, 1e-10);
		double mono_lccd = controller.scalar_value("mono_lccd");
		ASSERT_NEAR(0.258920875522295, mono_lccd, 1e-10);
	}

}

TEST(Sial_QM_FRAG,mcpt2_222_test){
	std::string job("mcpt2_222_test");

	std::stringstream output;

	TestControllerParallel controller(job, true, VERBOSE_TEST, "", output);
//
// MOI SCF
	controller.initSipTables(qm_dir_name);
	controller.run();

	if (attr->global_rank() == 0) {
		double scf_energy = controller.scalar_value("scf_energy");
		ASSERT_NEAR(-99.45988112164575, scf_energy, 1e-10);
	}
//
// frag SCF
	controller.initSipTables(qm_dir_name);
	controller.run();
//
// mcpt
	controller.initSipTables(qm_dir_name);
	controller.run();

	if (attr->global_rank() == 0) {
		double e1exc_at = controller.scalar_value("e1exc_at");
		ASSERT_NEAR(0.07666842263301, e1exc_at, 1e-10);
		double e10pol_at = controller.scalar_value("e10pol_at");
		ASSERT_NEAR(-0.05673548393582, e10pol_at, 1e-10);
		double singles = controller.scalar_value("singles");
		ASSERT_NEAR(-0.00258787489593, singles, 1e-10);
		double dimer_doubles = controller.scalar_value("dimer_doubles");
		ASSERT_NEAR(-0.00271042341177, dimer_doubles, 1e-10);
		double fragment_doubles = controller.scalar_value("fragment_doubles");
		ASSERT_NEAR(-1.00322095294998, fragment_doubles, 1e-10);
		double mono_lccd = controller.scalar_value("mono_lccd");
		ASSERT_NEAR(1.00337537138491, mono_lccd, 1e-10);
	}
//
// CIS
	controller.initSipTables(qm_dir_name);
	controller.run();

	if (attr->global_rank() == 0) {
		double * sek0 = controller.static_array("sek0");
		double expected[] = {0.43245480933972,0.43245480933973};
		int i = 0;
		for (i; i < 2; i++){
		    ASSERT_NEAR(sek0[i], expected[i], 1e-8);
		}
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



