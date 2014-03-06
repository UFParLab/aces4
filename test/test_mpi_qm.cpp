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

#include "blocks.h"

#ifdef HAVE_TAU
#include <TAU.h>
#endif

static const std::string dir_name("src/sialx/qm/");

TEST(Sial_QM,ccsdpt_test){

	sip::SIPMPIAttr &sip_mpi_attr = sip::SIPMPIAttr::get_instance();

	//create setup_file
	std::string job("ccsdpt_test");
	std::cout << "JOBNAME = " << job << std::endl;

	//read and print setup_file
	setup::BinaryInputFile setup_file(job + ".dat");

	// setup_reader.read(setup_file);
	setup::SetupReader setup_reader(setup_file);

	//interpret the program
	setup::SetupReader::SialProgList &progs = setup_reader.sial_prog_list_;
	setup::SetupReader::SialProgList::iterator it;
	{
		sip::PersistentArrayManager pbm;
		it = progs.begin();
		while (it != progs.end()){
			std::string sialfpath;
			sialfpath.append(dir_name);
			sialfpath.append("/");
			sialfpath.append(*it);
			setup::BinaryInputFile siox_file(sialfpath);
			sip::SipTables sipTables(setup_reader, siox_file);
			//std::cout << "SIP TABLES" << '\n' << sipTables << std::endl;

			sip::DataDistribution data_distribution(sipTables, sip_mpi_attr);
			if (sip_mpi_attr.is_server()){
				sip::SIPServer server(sipTables, data_distribution, sip_mpi_attr, pbm, pbm);
				server.run();
				SIP_LOG(std::cout<<"PBM after program at Server "<< sip_mpi_attr.global_rank()<< " : " << sialfpath << " :"<<std::endl<<pbm);
				++it;

			} else {
				sip::SialxTimer sialxTimer(sipTables.max_timer_slots());
				sip::Interpreter runner(sipTables, sialxTimer, pbm, pbm, sip_mpi_attr, data_distribution);
				std::cout << "SIAL PROGRAM OUTPUT for "<<*it  << std::endl;
				runner.interpret();
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
			pbm.clear_marked_arrays();
		}
	}
}

int main(int argc, char **argv) {

	MPI_Init(&argc, &argv);

#ifdef HAVE_TAU
	TAU_PROFILE_SET_NODE(0);
	TAU_STATIC_PHASE_START("SIP Main");
#endif

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
