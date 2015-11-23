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

/*
test
O -0.00000007     0.06307336     0.00000000
H -0.75198755    -0.50051034    -0.00000000
H  0.75198873    -0.50050946    -0.00000000

*ACES2(BASIS=3-21G
scf_conv=12
spherical=off
CALC=ccsd)

*SIP
MAXMEM=1500
SIAL_PROGRAM = scf_rhf.siox
*/
TEST(Sial_BasicQM,scf_rhf_aguess_test){
	std::string job("scf_rhf_aguess_test");

	std::stringstream output;

	TestControllerParallel controller(job, true, VERBOSE_TEST, "", output);
//
// SCF
	controller.initSipTables(qm_dir_name);
	controller.run();

	if (attr->global_rank() == 0) {
		double scf_energy = controller.scalar_value("scf_energy");
		ASSERT_NEAR(-75.5843267427, scf_energy, 1e-10);
	}
}

/*
test
O -0.00000007     0.06307336     0.00000000
H -0.75198755    -0.50051034    -0.00000000
H  0.75198873    -0.50050946    -0.00000000

*ACES2(BASIS=3-21G
scf_conv=12
spherical=off
CALC=ccsd)

*SIP
MAXMEM=1500
SIAL_PROGRAM = scf_uhf.siox
*/
TEST(Sial_BasicQM,scf_uhfrhf_aguess_test){
	std::string job("scf_uhfrhf_aguess_test");

	std::stringstream output;

	TestControllerParallel controller(job, true, VERBOSE_TEST, "", output);
//
// SCF
	controller.initSipTables(qm_dir_name);
	controller.run();

	if (attr->global_rank() == 0) {
		double scf_energy = controller.scalar_value("scf_energy");
		ASSERT_NEAR(-75.5843267427, scf_energy, 1e-10);
	}
}

/*
eth test
C                  0.00000000   -0.67759997   -0.00000000
H                  0.92414474   -1.21655197    0.00000000
H                 -0.92414474   -1.21655197    0.00000000
C                 -0.00000000    0.67759997   -0.00000000
H                 -0.92414474    1.21655197    0.00000000
H                  0.92414474    1.21655197   -0.00000000

*ACES2(calc=scf
ref=uhf
basis=3-21G
scf_conv=12
mult=3
spherical=off)

*SIP
MAXMEM=1500
SIAL_PROGRAM = scf_uhf.siox
*/
TEST(Sial_BasicQM,scf_uhf_triplet_aguess_test){
	std::string job("scf_uhf_triplet_aguess_test");

	std::stringstream output;

	TestControllerParallel controller(job, true, VERBOSE_TEST, "", output);
//
// SCF
	controller.initSipTables(qm_dir_name);
	controller.run();

	if (attr->global_rank() == 0) {
		double scf_energy = controller.scalar_value("scf_energy");
		ASSERT_NEAR(-77.48031103718395, scf_energy, 1e-10);
	}
}

/* linear dependence test
test
C            .000000     .000000     .762209
C            .000000     .000000    -.762209
H            .000000    1.018957    1.157229
H           -.882443    -.509479    1.157229
H            .882443    -.509479    1.157229
H            .000000   -1.018957   -1.157229
H           -.882443     .509479   -1.157229
H            .882443     .509479   -1.157229

*ACES2(BASIS=3-21++G
SCF_MAXCYC=1
lindep_tol=3
spherical=off
CALC=scf)

*SIP
MAXMEM=3000
SIAL_PROGRAM = scf_rhf_coreh.siox

*/
TEST(Sial_BasicQM,lindep_test){
	std::string job("lindep_test");

	std::stringstream output;

	TestControllerParallel controller(job, true, VERBOSE_TEST, "", output);
//
// SCF
	controller.initSipTables(qm_dir_name);
	controller.run();

	if (attr->global_rank() == 0) {
		double scf_energy = controller.scalar_value("scf_energy");
		ASSERT_NEAR(-64.24332859583458, scf_energy, 1e-10);
	}
}

/* PLACE THIS TEST LAST
ecp test
CL
CL 1 R
CL 1 R 2 A

R=1.5
A=108.0

*ACES2(ECP=ON,SYM=OFF,CALC=CCSD,MEM=1gb,CHARGE=-1)

CL:SBKJC
CL:SBKJC
CL:SBKJC

CL:SBKJC
CL:SBKJC
CL:SBKJC

*SIP
MAXMEM=1500
SIAL_PROGRAM=test_ecpint.siox

*/
TEST(Sial_BasicQM,test_1el_ecp_ints){
	std::string job("test_1el_ecp_ints");

	std::stringstream output;

	TestControllerParallel controller(job, true, VERBOSE_TEST, "", output);
//
// ecp test code
	controller.initSipTables(dir_name);
	controller.run();

	if (attr->global_rank() == 0) {
		double ecp_checksum = controller.scalar_value("ecp_checksum");
		ASSERT_NEAR(21.33193395347041, ecp_checksum, 1e-10);
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


	printf("Running main() from test_basic_qm.cpp\n");
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



