#include "gtest/gtest.h"
#include "siox_reader.h"
#include "io_utils.h"
#include "setup_reader.h"

#include "sip_tables.h"
#include "interpreter.h"
#include "setup_interface.h"
#include "sip_interface.h"
#include "data_manager.h"

#include "sip_server.h"
#include "sip_mpi_attr.h"
#include "global_state.h"
#include "block.h"

#ifdef HAVE_TAU
#include <TAU.h>
#endif

static const std::string dir_name("src/sialx/qm/");


TEST(Sial_QM,ccsdpt_test){

	sip::SIPMPIAttr &sip_mpi_attr = sip::SIPMPIAttr::get_instance();
	bool am_master = sip_mpi_attr.global_rank() == 0;
	std::cout << "+++++++++++++++++++++++++++++++++++"<< std::endl << sip_mpi_attr << std::endl  <<
			"my_server=" <<  sip_mpi_attr.my_server() << "+++++++++++++++++++++++++++++++++++"<< std::endl << std::flush;

	MPI_Barrier(MPI_COMM_WORLD);
	//create setup_file
	std::string job("ccsdpt_test");
	std::cout << "JOBNAME = " << job << std::endl;

	//read and print setup_file
	setup::BinaryInputFile setup_file(job + ".dat");

	// setup_reader.read(setup_file);
	setup::SetupReader setup_reader(setup_file);
	if (am_master) std::cout << "SETUP READER DATA:\n" << setup_reader<< std::endl;

	//interpret the program
	setup::SetupReader::SialProgList &progs = setup_reader.sial_prog_list_;
	setup::SetupReader::SialProgList::iterator it;
	{
		sip::PersistentArrayManager<sip::ServerBlock, sip::SIPServer>* persistent_server;
		sip::PersistentArrayManager<sip::Block,sip::Interpreter>* persistent_worker;
		if (sip_mpi_attr.is_server()) persistent_server = new sip::PersistentArrayManager<sip::ServerBlock, sip::SIPServer>();
		else persistent_worker = new sip::PersistentArrayManager<sip::Block,sip::Interpreter>();

		it = progs.begin();
		while (it != progs.end()){
			if (am_master){   std::cout << "\n\n\n\nstarting SIAL PROGRAM  "<< *it << std::endl;}
				std::string sialfpath(dir_name);
				sialfpath.append(*it);
			setup::BinaryInputFile siox_file(sialfpath);
			sip::SipTables sipTables(setup_reader, siox_file);
			sip::GlobalState::set_program_name(*it);
			sip::GlobalState::increment_program();
//			if (am_master) std::cout << "SIP TABLES" << '\n' << sipTables << std::endl;

			sip::DataDistribution data_distribution(sipTables, sip_mpi_attr);
			if (sip_mpi_attr.is_server()){
				sip::SIPServer server(sipTables, data_distribution, sip_mpi_attr, persistent_server);
				std::cout<<"starting server" << std::endl;
				server.run();
				persistent_server->save_marked_arrays(&server);
				SIP_LOG(std::cout<<"PBM after program at Server "<< sip_mpi_attr.global_rank()<< " : " << sialfpath << " :"<<std::endl<<persistent_server);
				++it;

			} else {
				sip::SialxTimer sialxTimer(sipTables.max_timer_slots());
				sip::Interpreter runner(sipTables, sialxTimer,  persistent_worker);
				std::cout << "SIAL PROGRAM OUTPUT for "<<*it  << std::endl;
				runner.interpret();
				persistent_worker->save_marked_arrays(&runner);
				ASSERT_EQ(0, sip::DataManager::scope_count);
				std::cout << "\nSIAL PROGRAM TERMINATED"<< std::endl;
				//std::cout<<"PBM after program " << sialfpath << " :"<<std::endl<<pbm;
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
}

int main(int argc, char **argv) {

	MPI_Init(&argc, &argv);

#ifdef HAVE_TAU
	TAU_PROFILE_SET_NODE(0);
	TAU_STATIC_PHASE_START("SIP Main");
#endif

	sip::check(sizeof(int) >= 4, "Size of integer should be 4 bytes or more");
	sip::check(sizeof(double) >= 8, "Size of double should be 8 bytes or more");
	sip::check(sizeof(long long) >= 8, "Size of long long should be 8 bytes or more");

	int num_procs;
	sip::SIPMPIUtils::check_err(MPI_Comm_size(MPI_COMM_WORLD, &num_procs));

	if (num_procs < 2){
		std::cerr<<"Please run this test with at least 2 mpi ranks"<<std::endl;
		return -1;
	}

	sip::SIPMPIUtils::set_error_handler();
	sip::SIPMPIAttr &sip_mpi_attr = sip::SIPMPIAttr::get_instance();


	printf("Running main() from master_test_main.cpp\n");
	testing::InitGoogleTest(&argc, argv);
	int result = RUN_ALL_TESTS();

#ifdef HAVE_TAU
	TAU_STATIC_PHASE_STOP("SIP Main");
#endif

	 MPI_Finalize();

	return result;

}
