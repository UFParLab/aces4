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


bool VERBOSE_TEST = false;
//bool VERBOSE_TEST = true;

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

/*
HF 333 lattice 
F                  2.23799966    2.28306000    0.00000000
H                  1.43957928    2.74014000    0.00000000
F                  2.23799966   -0.42194107    0.00000000
H                  1.43957928    0.03514195    0.00000000
F                  0.07705753    0.81516406    0.00000000
H                 -0.71799817    0.35225324    0.00000000
F                  0.07705753   -1.88984000    0.00000000
H                 -0.71799817   -2.35275000    0.00000000
F                  4.39894000    0.81516406    0.00000000
H                  3.60389000    0.35225324    0.00000000
F                  4.39894000   -1.88984000    0.00000000
H                  3.60389000   -2.35275000    0.00000000
F                  0.07705753    3.52016000    0.00000000
H                 -0.71799817    3.05725000    0.00000000
F                  4.39894000    3.52016000    0.00000000
H                  3.60389000    3.05725000    0.00000000
F                  2.23799966   -3.12694000    0.00000000
H                  1.43957928   -2.66986000    0.00000000
F                  2.23799966    2.28306000    3.42000000
H                  1.43957928    2.74014000    3.42000000
F                  2.23799966   -0.42194107    3.42000000
H                  1.43957928    0.03514195    3.42000000
F                  0.07705753    0.81516406    3.42000000
H                 -0.71799817    0.35225324    3.42000000
F                  0.07705753   -1.88984000    3.42000000
H                 -0.71799817   -2.35275000    3.42000000
F                  4.39894000    0.81516406    3.42000000
H                  3.60389000    0.35225324    3.42000000
F                  4.39894000   -1.88984000    3.42000000
H                  3.60389000   -2.35275000    3.42000000
F                  0.07705753    3.52016000    3.42000000
H                 -0.71799817    3.05725000    3.42000000
F                  4.39894000    3.52016000    3.42000000
H                  3.60389000    3.05725000    3.42000000
F                  2.23799966   -3.12694000    3.42000000
H                  1.43957928   -2.66986000    3.42000000
F                  2.23799966    2.28306000   -3.42000000
H                  1.43957928    2.74014000   -3.42000000
F                  2.23799966   -0.42194107   -3.42000000
H                  1.43957928    0.03514195   -3.42000000
F                  0.07705753    0.81516406   -3.42000000
H                 -0.71799817    0.35225324   -3.42000000
F                  0.07705753   -1.88984000   -3.42000000
H                 -0.71799817   -2.35275000   -3.42000000
F                  4.39894000    0.81516406   -3.42000000
H                  3.60389000    0.35225324   -3.42000000
F                  4.39894000   -1.88984000   -3.42000000
H                  3.60389000   -2.35275000   -3.42000000
F                  0.07705753    3.52016000   -3.42000000
H                 -0.71799817    3.05725000   -3.42000000
F                  4.39894000    3.52016000   -3.42000000
H                  3.60389000    3.05725000   -3.42000000
F                  2.23799966   -3.12694000   -3.42000000
H                  1.43957928   -2.66986000   -3.42000000

*ACES2(BASIS=3-21G
CALC=SCF 
CC_CONV=7 
SCF_CONV=10
spherical=off
REF=RHF   
COORDINATES=CARTESIAN
SYMMETRY=NONE)

*SIP
MAXMEM=120000
TIMERS=NO
SIAL_PROGRAM = frag_rhf.siox  

*FRAGLIST
27  14.0 14.0
2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2
1 2
3 4
5 6
7 8
9 10
11 12
13 14
15 16
17 18
19 20
21 22
23 24
25 26
27 28
29 30
31 32
33 34
35 36
37 38
39 40
41 42
43 44
45 46
47 48
49 50
51 52
53 54

*/

TEST(Sial_QM_FRAG,frag_scf_333_test){
	std::string job("frag_scf_333_test");

	std::stringstream output;

	TestControllerParallel controller(job, true, VERBOSE_TEST, "", output);
//
// frag_rhf
	controller.initSipTables(qm_dir_name);
	controller.run();

	if (attr->global_rank() == 0) {
		double total_scf_energy = controller.scalar_value("total_scf_energy");
		ASSERT_NEAR(-2685.41678798317707, total_scf_energy, 1e-10);
	}

}

TEST(Sial_QM_FRAG,DISABLED_mcpt2_222_test){
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
SIAL_PROGRAM = frag_rhf.siox
SIAL_PROGRAM = frag_pol_rhf.siox
SIAL_PROGRAM = fef_ccpt2.siox

*FRAGLIST
2 10.0 10.0
3 3
1 2 3
4 5 6
*/
TEST(Sial_QM_FRAG,fef_ccpt2_water_test){
	std::string job("fef_ccpt2_water_test");

	std::stringstream output;

	TestControllerParallel controller(job, true, VERBOSE_TEST, "", output);
//
// frag SCF
	controller.initSipTables(qm_dir_name);
	controller.run();
	if (attr->global_rank() == 0) {
		double total_scf_energy = controller.scalar_value("total_scf_energy");
		ASSERT_NEAR(-151.17093637179818,total_scf_energy, 1e-10);
	}
//
// frag pol SCF
	controller.initSipTables(qm_dir_name);
	controller.run();
	if (attr->global_rank() == 0) {
		double total_scf_energy = controller.scalar_value("total_scf_energy");
		ASSERT_NEAR(-151.18557576266943,total_scf_energy, 1e-10);
	}
//
// fef-ccpt
	controller.initSipTables(qm_dir_name);
	controller.run();

	if (attr->global_rank() == 0) {
		double e1exc_at = controller.scalar_value("e1exc_at");
		ASSERT_NEAR(0.00357466687985, e1exc_at, 1e-10);
		double dimer_doubles = controller.scalar_value("dimer_doubles");
		ASSERT_NEAR(-0.00045777487375, dimer_doubles, 1e-10);
		double fragment_doubles = controller.scalar_value("fragment_doubles");
		ASSERT_NEAR(-0.25782252185281, fragment_doubles, 1e-10);
		double mono_lccd = controller.scalar_value("mono_lccd");
		ASSERT_NEAR(-0.25784884147465, mono_lccd, 1e-10);
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



