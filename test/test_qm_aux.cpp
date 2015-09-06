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

/* eom-ccsd test, ZMAT is:
test
O -0.00000007     0.06307336     0.00000000
H -0.75198755    -0.50051034    -0.00000000
H  0.75198873    -0.50050946    -0.00000000
O -0.00000007     0.06307336     5.00000000
H -0.75198755    -0.50051034     5.00000000
H  0.75198873    -0.50050946     5.00000000

*ACES2(BASIS=3-21G
scf_conv=10
cc_conv=10
spherical=off
excite=eomee
estate_sym=6
estate_tol=8
symmetry=off
CALC=ccsd)

*SIP
MAXMEM=1500
SIAL_PROGRAM = scf_rhf.siox
SIAL_PROGRAM = tran_rhf_no4v.siox
SIAL_PROGRAM = rccsd_rhf.siox
SIAL_PROGRAM = rlambda_rhf.siox
SIAL_PROGRAM = rcis_rhf.siox
SIAL_PROGRAM = lr_eom_ccsd_rhf.siox
*/

TEST(Sial_QM,eom_water_dimer_test){
	std::string job("eom_water_dimer_test");

	std::stringstream output;

	TestControllerParallel controller(job, true, VERBOSE_TEST, "", output);
//
// SCF
	controller.initSipTables(qm_dir_name);
	controller.run();

	if (attr->global_rank() == 0) {
		double scf_energy = controller.scalar_value("scf_energy");
		ASSERT_NEAR(-151.16750250443988, scf_energy, 1e-10);
	}
//
// TRAN
	controller.initSipTables(qm_dir_name);
	controller.run();
//
// ccsd
	controller.initSipTables(qm_dir_name);
	controller.run();

	if (attr->global_rank() == 0) {
		double ccsd_energy = controller.scalar_value("ccsd_energy");
		ASSERT_NEAR(-151.42397520907252, ccsd_energy, 1e-10);
	}
//
// lambda 
	controller.initSipTables(qm_dir_name);
	controller.run();

	if (attr->global_rank() == 0) {
		double lambda_pseudo = controller.scalar_value("lambda_pseudo");
		ASSERT_NEAR(-0.25562663232767, lambda_pseudo, 1e-10);

                double * dipole = controller.static_array("dipole");
                double expected[] = {-0.00000000004082,0.00000142524718,-1.79437953464681};
                int i = 0;
                for (i; i < 2; i++){
                    ASSERT_NEAR(dipole[i], expected[i], 1e-6);
                }
	}
//
// CIS
	controller.initSipTables(qm_dir_name);
	controller.run();

	if (attr->global_rank() == 0) {
		double * sek0 = controller.static_array("sek0");
		double expected[] = {0.36143174674959,0.36156223386319,0.43379285233412,0.43379853039095,0.45184743894526,0.45257152504802};
		int i = 0;
		for (i; i < 6; i++){
		    ASSERT_NEAR(sek0[i], expected[i], 1e-10);
		}
	}
//
// eom
	controller.initSipTables(qm_dir_name);
	controller.run();

	if (attr->global_rank() == 0) {
		double * sek0 = controller.static_array("sek0");
		double expected[] = {0.327194541288917,0.327326642480730,0.410786177035338,0.410791948567762,0.421263454584640,0.421968829931049};
		int i = 0;
		for (i; i < 6; i++){
		    ASSERT_NEAR(sek0[i], expected[i], 1e-6);
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



