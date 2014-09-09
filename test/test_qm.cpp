#include "gtest/gtest.h"
#include "siox_reader.h"
#include "io_utils.h"
#include "setup_reader.h"

#include "sip_tables.h"
#include "interpreter.h"
#include "setup_interface.h"
#include "sip_interface.h"
#include "data_manager.h"
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
		ASSERT_NEAR(-0.0010909776247261387949, eaab, 1e-10);
		double esaab =controller.scalar_value("esaab");
		ASSERT_NEAR(8.5548065773238419758e-05, esaab, 1e-10);
	}
}



int main(int argc, char **argv) {

#ifdef HAVE_TAU
	TAU_PROFILE_SET_NODE(0);
	TAU_STATIC_PHASE_START("SIP Main");
#endif

	sip::check(sizeof(int) >= 4, "Size of integer should be 4 bytes or more");
	sip::check(sizeof(double) >= 8, "Size of double should be 8 bytes or more");
	sip::check(sizeof(long long) >= 8, "Size of long long should be 8 bytes or more");

	printf("Running main() from test_qm.cpp\n");
	testing::InitGoogleTest(&argc, argv);
	int result = RUN_ALL_TESTS();

#ifdef HAVE_TAU
	TAU_STATIC_PHASE_STOP("SIP Main");
#endif

	return result;

}
