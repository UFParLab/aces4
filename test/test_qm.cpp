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

// old ccsd(t) test.... difficult to deal with because the ZMAT source is gone
TEST(Sial_QM,DISABLED_ccsdpt_test){
	std::string job("ccsdpt_test");

	std::stringstream output;

	TestControllerParallel controller(job, true, VERBOSE_TEST, "", output);

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
		ASSERT_NEAR(-0.0010909774775509193, eaab, 1e-10);
		double esaab =controller.scalar_value("esaab");
		ASSERT_NEAR(8.5547845910409156e-05, esaab, 1e-10);
//		controller.print_timers(std::cout);
	}

}

/* new CCSD(T) test.  ZMAT source is:
test
H 0.0 0.0 0.0
F 0.0 0.0 0.917

*ACES2(BASIS=3-21G
scf_conv=12
cc_conv=12
spherical=off
CALC=ccsd)

*SIP
MAXMEM=1500
SIAL_PROGRAM = scf_rhf_coreh.siox
SIAL_PROGRAM = tran_rhf_no4v.siox
SIAL_PROGRAM = rccsd_rhf.siox
SIAL_PROGRAM = rccsdpt_aaa.siox
SIAL_PROGRAM = rccsdpt_aab.siox

*/
TEST(Sial_QM,second_ccsdpt_test){
	std::string job("second_ccsdpt_test");

	std::stringstream output;

	TestControllerParallel controller(job, true, VERBOSE_TEST, "", output);
//
// SCF
	controller.initSipTables(qm_dir_name);
	controller.run();

	if (attr->global_rank() == 0) {
		double scf_energy = controller.scalar_value("scf_energy");
		ASSERT_NEAR(-99.45975176375698, scf_energy, 1e-10);
	}
//
// tran
	controller.initSipTables(qm_dir_name);
	controller.run();
//
// ccsd
	controller.initSipTables(qm_dir_name);
	controller.run();

	if (attr->global_rank() == 0) {
		double ccsd_correlation = controller.scalar_value("ccsd_correlation");
		ASSERT_NEAR(-0.12588695910754, ccsd_correlation, 1e-10);
		double ccsd_energy = controller.scalar_value("ccsd_energy");
		ASSERT_NEAR(-99.58563872286452, ccsd_energy, 1e-10);
	}
//
// ccsdpt aaa
	controller.initSipTables(qm_dir_name);
	controller.run();

	if (attr->global_rank() == 0) {
		double eaaa = controller.scalar_value("eaaa");
		ASSERT_NEAR(-0.00001091437340, eaaa, 1e-10);
		double esaab =controller.scalar_value("esaaa");
		ASSERT_NEAR(0.00000240120432, esaab, 1e-10);
	}
//
// ccsdpt aab
	controller.initSipTables(qm_dir_name);
	controller.run();

	if (attr->global_rank() == 0) {
		double eaab = controller.scalar_value("eaab");
		ASSERT_NEAR(-0.00058787722879, eaab, 1e-10);
		double esaab =controller.scalar_value("esaab");
		ASSERT_NEAR(0.00003533079603, esaab, 1e-10);
		double ccsdpt_energy = controller.scalar_value("ccsdpt_energy");
		ASSERT_NEAR(-99.58619978246637, ccsdpt_energy, 1e-10);
	}

}

/* CIS(D) test, ZMAT is:
test
H 0.0 0.0 0.0
F 0.0 0.0 0.917

*ACES2(BASIS=3-21G
scf_conv=10
cc_conv=10
spherical=off
excite=eomee
estate_sym=2
estate_tol=10
CALC=ccsd)

*SIP
MAXMEM=1500
SIAL_PROGRAM = scf_rhf_coreh.siox
SIAL_PROGRAM = tran_rhf_no3v.siox
SIAL_PROGRAM = rcis_rhf.siox
SIAL_PROGRAM = rcis_d_rhf.siox

*/
TEST(Sial_QM,cis_test){
	std::string job("cis_test");

	std::stringstream output;

	TestControllerParallel controller(job, true, VERBOSE_TEST, "", output);
//
// SCF
	controller.initSipTables(qm_dir_name);
	controller.run();

	if (attr->global_rank() == 0) {
		double scf_energy = controller.scalar_value("scf_energy");
		ASSERT_NEAR(-99.45975176375698, scf_energy, 1e-10);
	}
//
// TRAN
	controller.initSipTables(qm_dir_name);
	controller.run();
//
// CIS
	controller.initSipTables(qm_dir_name);
	controller.run();

	if (attr->global_rank() == 0) {
		double * sek0 = controller.static_array("sek0");
		double expected[] = {0.43427913493064, 0.43427913493252};
		int i = 0;
		for (i; i < 2; i++){
		    ASSERT_NEAR(sek0[i], expected[i], 1e-10);
		}
	}
//
// CIS(D)
	controller.initSipTables(qm_dir_name);
	controller.run();

	if (attr->global_rank() == 0) {
		double * ekd = controller.static_array("ekd");
		double expected[] = {-0.03032115569246, -0.03032115569276};
		int i = 0;
		for (i; i < 2; i++){
		    ASSERT_NEAR(ekd[i], expected[i], 1e-10);
		}
	}

}

/* eom-ccsd test, ZMAT is:
test
O -0.00000007     0.06307336     0.00000000
H -0.75198755    -0.50051034    -0.00000000
H  0.75198873    -0.50050946    -0.00000000

*ACES2(BASIS=3-21G
scf_conv=12
cc_conv=12
spherical=off
excite=eomee
estate_sym=2
estate_tol=9
symmetry=off
CALC=ccsd)

*SIP
MAXMEM=1500
SIAL_PROGRAM = scf_rhf_coreh.siox
SIAL_PROGRAM = tran_rhf_no4v.siox
SIAL_PROGRAM = rccsd_rhf.siox
SIAL_PROGRAM = rlambda_rhf.siox
SIAL_PROGRAM = rcis_rhf.siox
SIAL_PROGRAM = lr_eom_ccsd_rhf.siox

*/
TEST(Sial_QM,DISABLED_eom_test){
	std::string job("eom_test");

	std::stringstream output;

	TestControllerParallel controller(job, true, VERBOSE_TEST, "", output);
//
// SCF
	controller.initSipTables(qm_dir_name);
	controller.run();

	if (attr->global_rank() == 0) {
		double scf_energy = controller.scalar_value("scf_energy");
		ASSERT_NEAR(-75.58432674274034, scf_energy, 1e-10);
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
		ASSERT_NEAR(-75.71251002928709, ccsd_energy, 1e-10);
	}
//
// lambda 
	controller.initSipTables(qm_dir_name);
	controller.run();
//
// CIS
	controller.initSipTables(qm_dir_name);
	controller.run();

	if (attr->global_rank() == 0) {
		double * sek0 = controller.static_array("sek0");
		double expected[] = {0.36275490375537, 0.43493738840536};
		int i = 0;
		for (i; i < 2; i++){
		    ASSERT_NEAR(sek0[i], expected[i], 1e-10);
		}
	}
//
// eom
	controller.initSipTables(qm_dir_name);
	controller.run();

	if (attr->global_rank() == 0) {
		double * sek0 = controller.static_array("sek0");
		double expected[] = {0.32850656893104, 0.41193399028059};
		int i = 0;
		for (i; i < 2; i++){
		    ASSERT_NEAR(sek0[i], expected[i], 1e-8);
		}
	}

}

/*
lambda response dipole test
H 0.0 0.0 0.0
F 0.0 0.0 0.917

*ACES2(BASIS=3-21G
scf_conv=12
cc_conv=12
spherical=off
scf_exporder=10
CALC=ccsd)

*SIP
MAXMEM=1500
SIAL_PROGRAM = scf_rhf_coreh.siox
SIAL_PROGRAM = tran_rhf_no4v.siox
SIAL_PROGRAM = rccsd_rhf.siox
SIAL_PROGRAM = rlambda_rhf.siox
*/
TEST(Sial_QM,rlambda_test){
	std::string job("rlambda_test");

	std::stringstream output;

	TestControllerParallel controller(job, true, VERBOSE_TEST, "", output);
//
// SCF
	controller.initSipTables(qm_dir_name);
	controller.run();
	if (attr->global_rank() == 0) {
                double * dipole = controller.static_array("dipole");
                double expected[] = {0.00000000000000, 0.00000000000000, 0.84792717246707};
                int i = 0;
                for (i; i < 2; i++){
                    ASSERT_NEAR(dipole[i], expected[i], 1e-10);
                }
	}
//
// tran
	controller.initSipTables(qm_dir_name);
	controller.run();
//
// ccsd
	controller.initSipTables(qm_dir_name);
	controller.run();
//
// rlambda
	controller.initSipTables(qm_dir_name);
	controller.run();

	if (attr->global_rank() == 0) {
		double lambda_pseudo = controller.scalar_value("lambda_pseudo");
		ASSERT_NEAR(-0.12592115116563, lambda_pseudo, 1e-10);

                double * dipole = controller.static_array("dipole");
                double expected[] = {0.00000000000000, 0.00000000000000, 0.80028992302928};
                int i = 0;
                for (i; i < 2; i++){
                    ASSERT_NEAR(dipole[i], expected[i], 1e-6);
                }
	}

}

/*
linear CCD test
O -0.00000007     0.06307336     0.00000000
H -0.75198755    -0.50051034    -0.00000000
H  0.75198873    -0.50050946    -0.00000000

*ACES2(BASIS=3-21G
scf_conv=12
cc_conv=12
spherical=off
CALC=ccsd)

*SIP
MAXMEM=1500
SIAL_PROGRAM = scf_rhf_coreh.siox
SIAL_PROGRAM = drop_core_in_sial.siox
SIAL_PROGRAM = tran_rhf_no4v.siox
SIAL_PROGRAM = rlccd_rhf.siox
*/
TEST(Sial_QM,DISABLED_lccd_dropcoreinsial_test){
	std::string job("lccd_test");

	std::stringstream output;

	TestControllerParallel controller(job, true, VERBOSE_TEST, "", output);
//
// SCF
	controller.initSipTables(qm_dir_name);
	controller.run();
	if (attr->global_rank() == 0) {
                double * dipole = controller.static_array("dipole");
                double expected[] = {0.00000081175618, -0.94582210690162, -0.00000000000000};
                int i = 0;
                for (i; i < 2; i++){
                    ASSERT_NEAR(dipole[i], expected[i], 1e-10);
                }
		double scf_energy = controller.scalar_value("scf_energy");
		ASSERT_NEAR(-75.58432674274046, scf_energy, 1e-10);
	}
//
// drop core
	controller.initSipTables(qm_dir_name);
	controller.run();
//
// tran
	controller.initSipTables(qm_dir_name);
	controller.run();
//
// lccd
	controller.initSipTables(qm_dir_name);
	controller.run();
	if (attr->global_rank() == 0) {
		double lccd_correlation = controller.scalar_value("lccd_correlation");
		ASSERT_NEAR(-0.12610179886435, lccd_correlation, 1e-10);
		double lccd_energy = controller.scalar_value("lccd_energy");
		ASSERT_NEAR(-75.71042854160481, lccd_energy, 1e-10);
                double * dipole = controller.static_array("dipole");
                double expected[] = {0.000000, -0.93525122104258496, -0.00000};
                int i = 0;
                for (i; i < 2; i++){
                    ASSERT_NEAR(dipole[i], expected[i], 1e-6);
                }
	}
}

/*
linear CCD test
O -0.00000007     0.06307336     0.00000000
H -0.75198755    -0.50051034    -0.00000000
H  0.75198873    -0.50050946    -0.00000000

*ACES2(BASIS=3-21G
scf_conv=12
cc_conv=12
spherical=off
drop_mo=1-1
CALC=ccsd)

*SIP
MAXMEM=1500
SIAL_PROGRAM = scf_rhf_coreh.siox
SIAL_PROGRAM = tran_rhf_no4v.siox
SIAL_PROGRAM = rlccd_rhf.siox
*/
TEST(Sial_QM,lccd_frozencore_test){
	std::string job("lccd_frozencore_test");

	std::stringstream output;

	TestControllerParallel controller(job, true, VERBOSE_TEST, "", output);
//
// SCF
	controller.initSipTables(qm_dir_name);
	controller.run();
	if (attr->global_rank() == 0) {
                double * dipole = controller.static_array("dipole");
                double expected[] = {0.00000081175618, -0.94582210690162, -0.00000000000000};
                int i = 0;
                for (i; i < 2; i++){
                    ASSERT_NEAR(dipole[i], expected[i], 1e-10);
                }
		double scf_energy = controller.scalar_value("scf_energy");
		ASSERT_NEAR(-75.58432674274046, scf_energy, 1e-10);
	}
//
// tran
	controller.initSipTables(qm_dir_name);
	controller.run();
//
// lccd
	controller.initSipTables(qm_dir_name);
	controller.run();
	if (attr->global_rank() == 0) {
		double lccd_correlation = controller.scalar_value("lccd_correlation");
		ASSERT_NEAR(-0.12610179886435, lccd_correlation, 1e-10);
		double lccd_energy = controller.scalar_value("lccd_energy");
		ASSERT_NEAR(-75.71042854160481, lccd_energy, 1e-10);
                double * dipole = controller.static_array("dipole");
                double expected[] = {0.000000, -0.93525122104258496, -0.00000};
                int i = 0;
                for (i; i < 2; i++){
                    ASSERT_NEAR(dipole[i], expected[i], 1e-6);
                }
	}
}


/*
linear CCSD test
O -0.00000007     0.06307336     0.00000000
H -0.75198755    -0.50051034    -0.00000000
H  0.75198873    -0.50050946    -0.00000000

*ACES2(BASIS=3-21G
scf_conv=12
cc_conv=12
spherical=off
CALC=ccsd)

*SIP
MAXMEM=1500
SIAL_PROGRAM = scf_rhf_coreh.siox
SIAL_PROGRAM = tran_rhf_no4v.siox
SIAL_PROGRAM = rlccsd_rhf.siox
*/
TEST(Sial_QM,DISABLED_lccsd_test){
	std::string job("lccsd_test");

	std::stringstream output;

	TestControllerParallel controller(job, true, VERBOSE_TEST, "", output);
//
// SCF
	controller.initSipTables(qm_dir_name);
	controller.run();
	if (attr->global_rank() == 0) {
                double * dipole = controller.static_array("dipole");
                double expected[] = {0.00000081175618, -0.94582210690162, -0.00000000000000};
                int i = 0;
                for (i; i < 2; i++){
                    ASSERT_NEAR(dipole[i], expected[i], 1e-10);
                }
		double scf_energy = controller.scalar_value("scf_energy");
		ASSERT_NEAR(-75.58432674274046, scf_energy, 1e-10);
	}
//
// tran
	controller.initSipTables(qm_dir_name);
	controller.run();
//
// lccd
	controller.initSipTables(qm_dir_name);
	controller.run();
	if (attr->global_rank() == 0) {
		double lccsd_correlation = controller.scalar_value("lccsd_correlation");
		ASSERT_NEAR(-0.12865706498547, lccsd_correlation, 1e-10);
		double lccsd_energy = controller.scalar_value("lccsd_energy");
		ASSERT_NEAR(-75.71298380772593, lccsd_energy, 1e-10);
	}
}

/* eom-lccsd test, ZMAT is:
test
O -0.00000007     0.06307336     0.00000000
H -0.75198755    -0.50051034    -0.00000000
H  0.75198873    -0.50050946    -0.00000000

*ACES2(BASIS=3-21G
scf_conv=12
cc_conv=12
spherical=off
excite=eomee
estate_sym=2
estate_tol=9
symmetry=off
CALC=ccsd)

*SIP
MAXMEM=1500
SIAL_PROGRAM = scf_rhf_coreh.siox
SIAL_PROGRAM = tran_rhf_no4v.siox
SIAL_PROGRAM = rcis_rhf.siox
SIAL_PROGRAM = rlccsd_rhf.siox
SIAL_PROGRAM = lr_eom_linccsd_rhf.siox

*/
TEST(Sial_QM,DISABLED_eom_lccsd_test){
	std::string job("eom_lccsd_test");

	std::stringstream output;

	TestControllerParallel controller(job, true, VERBOSE_TEST, "", output);
//
// SCF
	controller.initSipTables(qm_dir_name);
	controller.run();

	if (attr->global_rank() == 0) {
		double scf_energy = controller.scalar_value("scf_energy");
		ASSERT_NEAR(-75.58432674274034, scf_energy, 1e-10);
	}
//
// TRAN
	controller.initSipTables(qm_dir_name);
	controller.run();
//
// CIS
	controller.initSipTables(qm_dir_name);
	controller.run();

	if (attr->global_rank() == 0) {
		double * sek0 = controller.static_array("sek0");
		double expected[] = {0.36275490375537, 0.43493738840536};
		int i = 0;
		for (i; i < 2; i++){
		    ASSERT_NEAR(sek0[i], expected[i], 1e-10);
		}
	}
//
// lccsd
	controller.initSipTables(qm_dir_name);
	controller.run();

	if (attr->global_rank() == 0) {
		double lccsd_energy = controller.scalar_value("lccsd_energy");
		ASSERT_NEAR(-75.71298380772593, lccsd_energy, 1e-10);
	}
//
// eom
	controller.initSipTables(qm_dir_name);
	controller.run();

	if (attr->global_rank() == 0) {
		double * sek0 = controller.static_array("sek0");
		double expected[] = {0.32874112347521, 0.41219254536467};
		int i = 0;
		for (i; i < 2; i++){
		    ASSERT_NEAR(sek0[i], expected[i], 1e-8);
		}
	}
}

/* eom-lccd test, ZMAT is:
test
O -0.00000007     0.06307336     0.00000000
H -0.75198755    -0.50051034    -0.00000000
H  0.75198873    -0.50050946    -0.00000000

*ACES2(BASIS=3-21G
scf_conv=12
cc_conv=12
spherical=off
excite=eomee
estate_sym=2
estate_tol=9
symmetry=off
CALC=ccsd)

*SIP
MAXMEM=1500
SIAL_PROGRAM = scf_rhf_coreh.siox
SIAL_PROGRAM = tran_rhf_no4v.siox
SIAL_PROGRAM = rcis_rhf.siox
SIAL_PROGRAM = rlccd_rhf.siox
SIAL_PROGRAM = lr_eom_linccsd_rhf.siox

*/
TEST(Sial_QM,DISABLED_eom_lccd_test){
	std::string job("eom_lccd_test");

	std::stringstream output;

	TestControllerParallel controller(job, true, VERBOSE_TEST, "", output);
//
// SCF
	controller.initSipTables(qm_dir_name);
	controller.run();

	if (attr->global_rank() == 0) {
		double scf_energy = controller.scalar_value("scf_energy");
		ASSERT_NEAR(-75.58432674274034, scf_energy, 1e-10);
	}
//
// TRAN
	controller.initSipTables(qm_dir_name);
	controller.run();
//
// CIS
	controller.initSipTables(qm_dir_name);
	controller.run();

	if (attr->global_rank() == 0) {
		double * sek0 = controller.static_array("sek0");
		double expected[] = {0.36275490375537, 0.43493738840536};
		int i = 0;
		for (i; i < 2; i++){
		    ASSERT_NEAR(sek0[i], expected[i], 1e-10);
		}
	}
//
// lccd
	controller.initSipTables(qm_dir_name);
	controller.run();

	if (attr->global_rank() == 0) {
		double lccd_energy = controller.scalar_value("lccd_energy");
		ASSERT_NEAR(-75.71210049055006, lccd_energy, 1e-10);
	}
//
// eom
	controller.initSipTables(qm_dir_name);
	controller.run();

	if (attr->global_rank() == 0) {
		double * sek0 = controller.static_array("sek0");
		double expected[] = {0.32875641545765, 0.41224004798367};
		int i = 0;
		for (i; i < 2; i++){
		    ASSERT_NEAR(sek0[i], expected[i], 1e-8);
		}
	}
}

/* eom-mbpt test, ZMAT is:
test
O -0.00000007     0.06307336     0.00000000
H -0.75198755    -0.50051034    -0.00000000
H  0.75198873    -0.50050946    -0.00000000

*ACES2(BASIS=3-21G
scf_conv=12
cc_conv=12
spherical=off
excite=eomee
estate_sym=2
estate_tol=9
symmetry=off
CALC=ccsd)

*SIP
MAXMEM=1500
SIAL_PROGRAM = scf_rhf_coreh.siox
SIAL_PROGRAM = tran_rhf_no4v.siox
SIAL_PROGRAM = rcis_rhf.siox
SIAL_PROGRAM = mp2_rhf_disc.siox
SIAL_PROGRAM = lr_eom_linccsd_rhf.siox

*/
TEST(Sial_QM,DISABLED_eom_mp2_test){
	std::string job("eom_mp2_test");

	std::stringstream output;

	TestControllerParallel controller(job, true, VERBOSE_TEST, "", output);
//
// SCF
	controller.initSipTables(qm_dir_name);
	controller.run();

	if (attr->global_rank() == 0) {
		double scf_energy = controller.scalar_value("scf_energy");
		ASSERT_NEAR(-75.58432674274034, scf_energy, 1e-10);
	}
//
// TRAN
	controller.initSipTables(qm_dir_name);
	controller.run();
//
// CIS
	controller.initSipTables(qm_dir_name);
	controller.run();

	if (attr->global_rank() == 0) {
		double * sek0 = controller.static_array("sek0");
		double expected[] = {0.36275490375537, 0.43493738840536};
		int i = 0;
		for (i; i < 2; i++){
		    ASSERT_NEAR(sek0[i], expected[i], 1e-10);
		}
	}
//
// mp2
	controller.initSipTables(qm_dir_name);
	controller.run();

	if (attr->global_rank() == 0) {
		double mp2_energy = controller.scalar_value("mp2_energy");
		ASSERT_NEAR(-75.70540831822183, mp2_energy, 1e-10);
	}
//
// eom
	controller.initSipTables(qm_dir_name);
	controller.run();

	if (attr->global_rank() == 0) {
		double * sek0 = controller.static_array("sek0");
		double expected[] = {0.32702247859224, 0.41071327775950};
		int i = 0;
		for (i; i < 2; i++){
		    ASSERT_NEAR(sek0[i], expected[i], 1e-8);
		}
	}
}

/* lambda CCSD(T) test
test
H 0.0 0.0 0.0
F 0.0 0.0 0.917

*ACES2(BASIS=3-21G
scf_conv=12
cc_conv=12
drop_mo=1-1
spherical=off
CALC=ccsd)

*SIP
MAXMEM=1500
SIAL_PROGRAM = scf_rhf_coreh.siox
SIAL_PROGRAM = tran_rhf_no4v.siox
SIAL_PROGRAM = rccsd_rhf.siox
SIAL_PROGRAM = rlambda_rhf.siox
SIAL_PROGRAM = rlamccsdpt_aaa.siox
SIAL_PROGRAM = lamrccsdpt_aab.siox

*/
TEST(Sial_QM,lamccsdpt_test){
	std::string job("lamccsdpt_test");

	std::stringstream output;

	TestControllerParallel controller(job, true, VERBOSE_TEST, "", output);
//
// SCF
	controller.initSipTables(qm_dir_name);
	controller.run();

	if (attr->global_rank() == 0) {
		double scf_energy = controller.scalar_value("scf_energy");
		ASSERT_NEAR(-99.45975176375698, scf_energy, 1e-10);
	}
//
// tran
	controller.initSipTables(qm_dir_name);
	controller.run();
//
// ccsd
	controller.initSipTables(qm_dir_name);
	controller.run();

	if (attr->global_rank() == 0) {
		double ccsd_energy = controller.scalar_value("ccsd_energy");
		ASSERT_NEAR(-99.583972376431, ccsd_energy, 1e-10);
	}
//
// lambda
	controller.initSipTables(qm_dir_name);
	controller.run();
//
// lambda ccsdpt aaa
	controller.initSipTables(qm_dir_name);
	controller.run();

	if (attr->global_rank() == 0) {
		double eaaa = controller.scalar_value("eaaa");
		ASSERT_NEAR(-0.00001109673867, eaaa, 1e-10);
		double esaab =controller.scalar_value("esaaa");
		ASSERT_NEAR(0.00000190144932, esaab, 1e-10);
	}
//
// ccsdpt aab
	controller.initSipTables(qm_dir_name);
	controller.run();

	if (attr->global_rank() == 0) {
		double eaab = controller.scalar_value("eaab");
		ASSERT_NEAR(-0.00057100058054, eaab, 1e-10);
		double esaab =controller.scalar_value("esaab");
		ASSERT_NEAR(0.00002785417018, esaab, 1e-10);
		double ccsdpt_energy = controller.scalar_value("ccsdpt_energy");
		ASSERT_NEAR(-99.584524718131, ccsdpt_energy, 1e-10);
	}

}
/*
eom-ccsd right test
O -0.00000007     0.06307336     0.00000000
H -0.75198755    -0.50051034    -0.00000000
H  0.75198873    -0.50050946    -0.00000000

*ACES2(BASIS=3-21G
scf_conv=10
cc_conv=10
spherical=off
excite=eomee
estate_sym=4
estate_tol=6
symmetry=off
CALC=ccsd)

*SIP
MAXMEM=1500
SIAL_PROGRAM = scf_rhf.siox
SIAL_PROGRAM = tran_rhf_no4v.siox
SIAL_PROGRAM = rccsd_rhf.siox
SIAL_PROGRAM = rlambda_rhf.siox
SIAL_PROGRAM = rcis_rhf.siox
SIAL_PROGRAM = eom_ccsd_rhf_right.siox

 */
TEST(Sial_QM,eom_ccsd_water_right_test){
	std::string job("eom_ccsd_water_right_test");

	std::stringstream output;

	TestControllerParallel controller(job, true, VERBOSE_TEST, "", output);
//
// SCF
	controller.initSipTables(qm_dir_name);
	controller.run();

	if (attr->global_rank() == 0) {
		double scf_energy = controller.scalar_value("scf_energy");
		ASSERT_NEAR(-75.58432674274033, scf_energy, 1e-10);
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
		ASSERT_NEAR(-75.71251002936883, ccsd_energy, 1e-10);
	}
//
// lambda 
	controller.initSipTables(qm_dir_name);
	controller.run();
//
// CIS
	controller.initSipTables(qm_dir_name);
	controller.run();
//
// eom
	controller.initSipTables(qm_dir_name);
	controller.run();

	if (attr->global_rank() == 0) {
		double * sek0 = controller.static_array("sek0");
		double Eexpected[] = {0.32850657002707, 0.41193399006592, 0.42288344162832, 0.51159731180444};
		int i = 0;
		for (i; i < 4; i++){
		    ASSERT_NEAR(sek0[i], Eexpected[i], 1e-8);
		}
		double *  Rdipmom= controller.static_array("rdipmom");
		double Rexpected[] = {0.17558771, 0.0, 0.56188382, 0.56905669};
		i = 0;
		for (i; i < 4; i++){
		    ASSERT_NEAR(Rdipmom[i], Rexpected[i], 1e-4);
		}
	}
}
/*
eom-ccsd left right test
O -0.00000007     0.06307336     0.00000000
H -0.75198755    -0.50051034    -0.00000000
H  0.75198873    -0.50050946    -0.00000000

*ACES2(BASIS=3-21G
scf_conv=10
cc_conv=10
spherical=off
excite=eomee
estate_sym=4
estate_tol=6
symmetry=off
CALC=ccsd)

*SIP
MAXMEM=1500
SIAL_PROGRAM = scf_rhf.siox
SIAL_PROGRAM = tran_rhf_no4v.siox
SIAL_PROGRAM = rccsd_rhf.siox
SIAL_PROGRAM = rlambda_rhf.siox
SIAL_PROGRAM = rcis_rhf.siox
SIAL_PROGRAM = eom_ccsd_rhf_right.siox
SIAL_PROGRAM = eom_ccsd_rhf_left.siox

 */
TEST(Sial_QM,eom_ccsd_water_test){
	std::string job("eom_ccsd_water_test");

	std::stringstream output;

	TestControllerParallel controller(job, true, VERBOSE_TEST, "", output);
//
// SCF
	controller.initSipTables(qm_dir_name);
	controller.run();

	if (attr->global_rank() == 0) {
		double scf_energy = controller.scalar_value("scf_energy");
		ASSERT_NEAR(-75.58432674274033, scf_energy, 1e-10);
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
		ASSERT_NEAR(-75.71251002936883, ccsd_energy, 1e-10);
	}
//
// lambda 
	controller.initSipTables(qm_dir_name);
	controller.run();
//
// CIS
	controller.initSipTables(qm_dir_name);
	controller.run();
//
// eom right
	controller.initSipTables(qm_dir_name);
	controller.run();

	if (attr->global_rank() == 0) {
		double * sek0 = controller.static_array("sek0");
		double Eexpected[] = {0.32850657002707, 0.41193399006592, 0.42288344162832, 0.51159731180444};
		int i = 0;
		for (i; i < 4; i++){
		    ASSERT_NEAR(sek0[i], Eexpected[i], 1e-8);
		}
	}
//
// eom left
	controller.initSipTables(qm_dir_name);
	controller.run();

	if (attr->global_rank() == 0) {
		double * sek0 = controller.static_array("sek0");
		double Eexpected[] = {0.32850657002707, 0.41193399006592, 0.42288344162832, 0.51159731180444};
		int i = 0;
		for (i; i < 4; i++){
		    ASSERT_NEAR(sek0[i], Eexpected[i], 1e-8);
		}
		double *  oscnorm= controller.static_array("oscnorm");
		double Oexpected[] = {0.00680956, 0.0, 0.09037060, 0.11312310};
		i = 0;
		for (i; i < 4; i++){
		    ASSERT_NEAR(oscnorm[i], Oexpected[i], 1e-4);
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
	TestControllerParallel::sleep_between_tests = 5;  //delay between tests to deal with duplicate jobids due to low resolution of the timestamp used to construct them
#endif
	barrier();


	printf("Running main() from test_sial.cpp\n");
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



