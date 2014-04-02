#include "gtest/gtest.h"
#include "siox_reader.h"
#include "io_utils.h"
#include "setup_reader.h"

#include "sip_tables.h"
#include "interpreter.h"
#include "setup_interface.h"
#include "sip_interface.h"
#include "data_manager.h"

#include "blocks.h"

#ifdef HAVE_TAU
#include <TAU.h>
#endif

static const std::string dir_name("src/sialx/qm/");

/** Single node ccsd test*/
TEST(Sial_QM,ccsdpt_test){
	std::cout << "****************************************\n";
	sip::DataManager::scope_count=0;
	//create setup_file
	std::string job("ccsdpt_test");
	std::cout << "JOBNAME = " << job << std::endl;

	//read and print setup_file
	setup::BinaryInputFile setup_file(job + ".dat");

	// setup_reader.read(setup_file);
	setup::SetupReader setup_reader(setup_file);
	std::cout << "SETUP READER DATA:\n" << setup_reader<< std::endl;

	//interpret the program
	setup::SetupReader::SialProgList &progs = setup_reader.sial_prog_list_;
	setup::SetupReader::SialProgList::iterator it;
	{
		sip::PersistentArrayManager<sip::Block,sip::Interpreter>* pbm;
		pbm = new sip::PersistentArrayManager<sip::Block, sip::Interpreter>();
		it = progs.begin();
		while (it != progs.end()){
			std::cout << "\n\n\n\nstarting SIAL PROGRAM  "<< *it << std::endl;
			std::string sialfpath(dir_name);
			sialfpath.append(*it);
			setup::BinaryInputFile siox_file(sialfpath);
			sip::SipTables sipTables(setup_reader, siox_file);
			//std::cout << "SIP TABLES" << '\n' << sipTables << std::endl;
			sip::SialxTimer sialxTimer(sipTables.max_timer_slots());
			sip::Interpreter runner(sipTables, sialxTimer, pbm);

			runner.interpret();
			std::cout<<"Persistent Array manager right after program " << sialfpath << " :"<<std::endl<< *pbm;
			pbm->save_marked_arrays(&runner);
			std::cout<<"Persistent Array Manager after saving " << sialfpath << " :"<<std::endl<< *pbm;
			ASSERT_EQ(0, sip::DataManager::scope_count);
			std::cout << "\nSIAL PROGRAM TERMINATED"<< std::endl;

			++it;
			if (it == progs.end()){	// Last Program
				double eaab = runner.scalar_value("eaab");
				ASSERT_NEAR(-0.0010909776247261387949, eaab, 1e-12);
				double esaab = runner.scalar_value("esaab");
				ASSERT_NEAR(8.5548065773238419758e-05, esaab, 1e-12);
			}
		}
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
