#include "gtest/gtest.h"
#include "siox_reader.h"
#include "io_utils.h"
#include "setup_reader.h"

#include "sip_tables.h"
#include "interpreter.h"
#include "setup_interface.h"
#include "sip_interface.h"
#include "data_manager.h"
#include "global_state.h"

#include "worker_persistent_array_manager.h"
#include "server_persistent_array_manager.h"

#include "block.h"

#ifdef HAVE_TAU
#include <TAU.h>
#endif

#ifdef HAVE_MPI
#include "sip_server.h"
#include "sip_mpi_attr.h"
#include "global_state.h"
#include "sip_mpi_utils.h"
#else
#include "sip_attr.h"
#endif

void list_block_map();
static const std::string dir_name("src/sialx/test/");

#ifdef HAVE_MPI
sip::SIPMPIAttr *attr;
void barrier(){sip::SIPMPIUtils::check_err(MPI_Barrier(MPI_COMM_WORLD));}
#else
sip::SIPAttr *attr = &sip::SIPAttr::get_instance();
void barrier(){}
#endif

TEST(Sial,empty){
	std::cout << "****************************************\n"<< std::flush;
	sip::DataManager::scope_count=0;
	//create setup_file
	std::string job("empty");


	//no setup file, but the siox readers expects a SetupReader, so create an empty one.
	setup::SetupReader &setup_reader = *setup::SetupReader::get_empty_reader();



	std::string prog_name = job + ".siox";
	std::string siox_dir(dir_name);
	setup::BinaryInputFile siox_file(siox_dir + prog_name);
	sip::SipTables sipTables(setup_reader, siox_file);
	if (attr->global_rank() == 0){
		std::cout << "JOBNAME = " << job << std::endl<< std::flush;
		std::cout << "SETUP READER DATA:\n" << setup_reader<< std::endl<< std::flush;
	    std::cout << "SIP TABLES" << '\n' << sipTables << std::endl<< std::flush;
	    std::cout << "COMMENT:  This is an empty program.  It should simply terminate.\n\n" << std::endl<< std::flush;
	}
    barrier();
    sip::WorkerPersistentArrayManager wpam;
	#ifdef HAVE_MPI
		sip::ServerPersistentArrayManager spam;

		sip::DataDistribution data_distribution(sipTables, *attr);
		sip::GlobalState::set_program_name(prog_name);
		sip::GlobalState::increment_program();
		if (attr->is_server()){
			sip::SIPServer server(sipTables, data_distribution, *attr, &spam);
			barrier();
			std::cout << "Rank " << attr->global_rank() << " SERVER " << job << " STARTING"<< std::endl << std::flush;
			server.run();
			std::cout << "\nRank " << attr->global_rank() <<" SERVER " << job << " TERMINATED"<< std::endl << std::flush;
			barrier();
	      //CHECK SERVER STATE HERE
		} else
#endif
//	interpret the program
	{
		sip::SialxTimer sialxTimer(sipTables.max_timer_slots());
		sip::Interpreter runner(sipTables, sialxTimer);
		barrier();
		std::cout << "Rank " << attr->global_rank() << " SIAL PROGRAM " << job << " STARTING"<< std::endl << std::flush;
		runner.interpret();
		std::cout << "\nRank " << attr->global_rank() <<" SIAL PROGRAM " << job << " TERMINATED"<< std::endl << std::flush;
		barrier();
	    //CHECK WORKER STATE HERE
	}

}

TEST(Sial,scalars){
	std::cout << "****************************************\n"<< std::flush;
	sip::DataManager::scope_count=0;
	//create setup_file
	std::string job("scalars");


double x = 3.456;
double y = -0.1;

if (attr->global_rank() == 0){
	init_setup(job.c_str());
	set_scalar("x",x);
	set_scalar("y",y);
	std::string tmp = job + ".siox";
	const char* nm= tmp.c_str();
	add_sial_program(nm);
	finalize_setup();
}

barrier();
//read and print setup_file
setup::BinaryInputFile setup_file(job + ".dat");
setup::SetupReader setup_reader(setup_file);

//get siox name from setup, load and print the sip tables
std::string prog_name = setup_reader.sial_prog_list_.at(0);
std::string siox_dir(dir_name);
setup::BinaryInputFile siox_file(siox_dir + prog_name);
sip::SipTables sipTables(setup_reader, siox_file);



	if (attr->global_rank() == 0){
		std::cout << "JOBNAME = " << job << std::endl<< std::flush;
		std::cout << "SETUP READER DATA:\n" << setup_reader<< std::endl<< std::flush;
	    std::cout << "SIP TABLES" << '\n' << sipTables << std::endl<< std::flush;
	}
    barrier();
    sip::WorkerPersistentArrayManager wpam;
	#ifdef HAVE_MPI
		sip::ServerPersistentArrayManager spam;

		sip::DataDistribution data_distribution(sipTables, *attr);
		sip::GlobalState::set_program_name(prog_name);
		sip::GlobalState::increment_program();
		if (attr->is_server()){
			sip::SIPServer server(sipTables, data_distribution, *attr, &spam);
			barrier();
			std::cout << "Rank " << attr->global_rank() << " SERVER " << job << " STARTING"<< std::endl << std::flush;
			server.run();
			std::cout << "\nRank " << attr->global_rank() <<" SERVER " << job << " TERMINATED"<< std::endl << std::flush;
			barrier();
	      //CHECK SERVER STATE HERE
		} else
#endif
//	interpret the program
	{
		sip::SialxTimer sialxTimer(sipTables.max_timer_slots());
		sip::Interpreter runner(sipTables, sialxTimer);
		barrier();
		std::cout << "Rank " << attr->global_rank() << " SIAL PROGRAM " << job << " STARTING"<< std::endl << std::flush;
		runner.interpret();
		std::cout << "\nRank " << attr->global_rank() <<" SIAL PROGRAM " << job << " TERMINATED"<< std::endl << std::flush;
		barrier();
		ASSERT_DOUBLE_EQ(x, scalar_value("x"));
		ASSERT_DOUBLE_EQ(y, scalar_value("y"));
		ASSERT_DOUBLE_EQ(x, scalar_value("z"));
		ASSERT_DOUBLE_EQ(99.99, scalar_value("zz"));
		ASSERT_EQ(0, sip::DataManager::scope_count);
	}

}
//TEST(SimpleMPI,Simple_Put_Test){
//	std::cout << "****************************************\n";
//	sip::GlobalState::reset_program_count();
//	sip::DataManager::scope_count=0;
//	//create setup_file
//	std::string job("put_test");
//	std::cout << "JOBNAME = " << job << std::endl;
//	int norb = 2;
//
//
//
//	if (attr->global_rank() == 0){
//		init_setup(job.c_str());
//		set_constant("norb",norb);
//		std::string tmp = job + ".siox";
//		const char* nm = tmp.c_str();
//		add_sial_program(nm);
//		int segs[]  = {3, 4};
//		set_aoindex_info(2,segs);
//		finalize_setup();
//	}
//
//	barrier();
//
//	setup::BinaryInputFile setup_file(job + ".dat");
//	setup::SetupReader setup_reader(setup_file);
//	std::cout << "SETUP READER DATA:\n" << setup_reader<< std::endl;
//
//	//get siox name from setup, load and print the sip tables
//	std::string prog_name = setup_reader.sial_prog_list_.at(0);
//	std::string siox_dir(dir_name);
//	setup::BinaryInputFile siox_file(siox_dir + prog_name);
//	sip::SipTables sipTables(setup_reader, siox_file);
//	std::cout << "SIP TABLES" << '\n' << sipTables << std::endl;
//
//
//	//interpret the program
//
//	sip::GlobalState::set_program_name(prog_name);
//	sip::GlobalState::increment_program();
//	sip::WorkerPersistentArrayManager wpam;
//
//#ifdef HAVE_MPI
//	sip::ServerPersistentArrayManager spam;
//
//	sip::DataDistribution data_distribution(sipTables, *attr);
//	sip::GlobalState::set_program_name(prog_name);
//	sip::GlobalState::increment_program();
//	if (attr->is_server()){
//		sip::SIPServer server(sipTables, data_distribution, *attr, &spam);
//		server.run();
//
//	} else {
//#endif
//		sip::SialxTimer sialxTimer(sipTables.max_timer_slots());
//		sip::Interpreter runner(sipTables, sialxTimer, &wpam);
//		runner.interpret();
//		ASSERT_EQ(0, sip::DataManager::scope_count);
//
//		std::cout << "\nSIAL PROGRAM TERMINATED"<< std::endl;
//
//		int x_slot = sip::Interpreter::global_interpreter->array_slot(std::string("x"));
//		int y_slot = sip::Interpreter::global_interpreter->array_slot(std::string("y"));
//		sip::index_selector_t x_indices, y_indices;
//		for (int i = 0; i < MAX_RANK; i++) x_indices[i] = y_indices[i] = sip::unused_index_value;
//		x_indices[0] = y_indices[0] = 1;
//		x_indices[1] = y_indices[0] = 1;
//		sip::BlockId x_bid(x_slot, x_indices);
//		sip::BlockId y_bid(y_slot, y_indices);
//
//		sip::Block::BlockPtr x_bptr = sip::Interpreter::global_interpreter->get_block_for_reading(x_bid);
//		sip::Block::dataPtr x_data = x_bptr->get_data();
//		sip::Block::BlockPtr y_bptr = sip::Interpreter::global_interpreter->get_block_for_reading(y_bid);
//		sip::Block::dataPtr y_data = y_bptr->get_data();
//
//		for (int i=0; i<3*3; i++){
//			ASSERT_EQ(1, x_data[i]);
//			ASSERT_EQ(2, y_data[i]);
//		}
//#ifdef HAVE_MPI
//		}
//#endif
//}


//// Sanity test to check for no compiler errors, crashes, etc.
//TEST(SimpleMPI,persistent_empty_mpi){
//	sip::GlobalState::reset_program_count();
//	sip::SIPMPIAttr &sip_mpi_attr = sip::SIPMPIAttr::get_instance();
//	int my_rank = sip_mpi_attr.global_rank();
//
//	sip::DataManager::scope_count=0;
//	//create setup_file
//	std::string job("persistent_empty_mpi");
//	std::cout << "JOBNAME = " << job << std::endl;
//	double x = 3.456;
//	int norb = 4;
//
//	if (sip_mpi_attr.global_rank() == 0){
//		init_setup(job.c_str());
//		set_scalar("x",x);
//		set_constant("norb",norb);
//		std::string tmp = job + "1.siox";
//		const char* nm= tmp.c_str();
//		add_sial_program(nm);
//		std::string tmp1 = job + "2.siox";
//		const char* nm1= tmp1.c_str();
//		add_sial_program(nm1);
//		int segs[]  = {2,3,4,1};
//		set_aoindex_info(4,segs);
//		finalize_setup();
//	}
//
//	sip::SIPMPIUtils::check_err(MPI_Barrier(MPI_COMM_WORLD));
//
//	//read and print setup_file
//	setup::BinaryInputFile setup_file(job + ".dat");
//	setup::SetupReader setup_reader(setup_file);
//	std::cout << "SETUP READER DATA:\n" << setup_reader<< std::endl;
//
//	sip::WorkerPersistentArrayManager wpam;
//	sip::ServerPersistentArrayManager spam;
//
//	//Execute first program
//	//get siox name from setup, load and print the sip tables
//	std::string prog_name = setup_reader.sial_prog_list_.at(0);
//	std::string siox_dir(dir_name);
//	setup::BinaryInputFile siox_file(siox_dir + prog_name);
//	sip::SipTables sipTables(setup_reader, siox_file);
//	if (!sip_mpi_attr.is_server()) {std::cout << "SIP TABLES for " << prog_name << '\n' << sipTables << std::endl;}
//
//	//create worker and server
//	sip::DataDistribution data_distribution(sipTables, sip_mpi_attr);
//	sip::GlobalState::set_program_name(prog_name);
//	sip::GlobalState::increment_program();
//	if (sip_mpi_attr.global_rank()==0){   std::cout << "\n\n\n\n>>>>>>>>>>>>starting SIAL PROGRAM  "<< job << std::endl;}
//
//	std::cout << "rank " << my_rank << " reached first barrier in test" << std::endl << std::flush;
//	MPI_Barrier(MPI_COMM_WORLD);
//	std::cout << "rank " << my_rank << " passed first barrier in test" << std::endl << std::flush;
//
//	if (sip_mpi_attr.is_server()){
//		sip::SIPServer server(sipTables, data_distribution, sip_mpi_attr, &spam);
//		std::cout << "reached prog 1 server barrier" << std::endl << std::flush;
//		MPI_Barrier(MPI_COMM_WORLD);
//		std::cout<<"passed prog 1 server barrier" << std::endl;
//		server.run();
//		spam.save_marked_arrays(&server);
//	} else {
//		sip::SialxTimer sialxTimer(sipTables.max_timer_slots());
//		sip::Interpreter runner(sipTables, sialxTimer, &wpam);
//		std::cout << "reached prg 1 worker barrier " << std::endl << std::flush;
//		MPI_Barrier(MPI_COMM_WORLD);
//		std::cout << "passed prog 1 worker barrier "  << std::endl << std::flush;
//		runner.interpret();
//		wpam.save_marked_arrays(&runner);
//	}
//
//   std::cout << std::flush;
//   if (sip_mpi_attr.global_rank()==0){
//	   std::cout << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@" << std::endl << std::flush;
//	   std::cout << "SETUP READER DATA FOR SECOND PROGRAM:\n" << setup_reader<< std::endl;
//   }
//
//	std::string prog_name2 = setup_reader.sial_prog_list_.at(1);
//	setup::BinaryInputFile siox_file2(siox_dir + prog_name2);
//	sip::SipTables sipTables2(setup_reader, siox_file2);
//	if (sip_mpi_attr.global_rank()==0){
//		std::cout << "SIP TABLES FOR " << prog_name2 << '\n' << sipTables2 << std::endl;
//	}
//	sip::DataDistribution data_distribution2(sipTables2, sip_mpi_attr);
//	sip::GlobalState::set_program_name(prog_name);
//	sip::GlobalState::increment_program();
//	std::cout << "rank " << my_rank << " reached second barrier in test" << std::endl << std::flush;
//	MPI_Barrier(MPI_COMM_WORLD);
//	std::cout << "rank " << my_rank << " passed second barrier in test" << std::endl << std::flush;
//
//	if (sip_mpi_attr.is_server()){
//		sip::SIPServer server(sipTables2, data_distribution2, sip_mpi_attr, &spam);
//		std::cout << "reached prog 2 server barrier " << std::endl << std::flush;
//		MPI_Barrier(MPI_COMM_WORLD);
//		std::cout<<"passed prog 2 server barrier" << std::endl<< std::flush;
//		server.run();
//		spam.save_marked_arrays(&server);
//	} else {
//		sip::SialxTimer sialxTimers(sipTables2.max_timer_slots());
//		sip::Interpreter runner(sipTables2, sialxTimers, &wpam);
//		std::cout << "reached prog 2 worker barrier " << std::endl << std::flush;
//		MPI_Barrier(MPI_COMM_WORLD);
//		std::cout << "passed prog 2 worker barrier"<< job  << std::endl<< std::flush;
//		runner.interpret();
//		wpam.save_marked_arrays(&runner);
//		std::cout << "\nSIAL PROGRAM 2 TERMINATED"<< std::endl;
//	}
//
//	std::cout << "rank " << my_rank << " reached third barrier in test" << std::endl << std::flush;
//	MPI_Barrier(MPI_COMM_WORLD);
//	std::cout << "rank " << my_rank << " passed third barrier in test" << std::endl << std::flush;
//
//}
//
//TEST(SimpleMPI,persistent_distributed_array_mpi){
//
//	sip::GlobalState::reset_program_count();
//	sip::SIPMPIAttr &sip_mpi_attr = sip::SIPMPIAttr::get_instance();
//	int my_rank = sip_mpi_attr.global_rank();
//
//	std::cout << "****************************************\n";
//	sip::DataManager::scope_count=0;
//	//create setup_file
//	std::string job("persistent_distributed_array_mpi");
//	std::cout << "JOBNAME = " << job << std::endl;
//	double x = 3.456;
//	int norb = 2;
//	int segs[]  = {2,3};
//
//
//	if (sip_mpi_attr.global_rank() == 0){
//		init_setup(job.c_str());
//		set_scalar("x",x);
//		set_constant("norb",norb);
//		std::string tmp = job + "1.siox";
//		const char* nm= tmp.c_str();
//		add_sial_program(nm);
//		std::string tmp1 = job + "2.siox";
//		const char* nm1= tmp1.c_str();
//		add_sial_program(nm1);
//		set_aoindex_info(2,segs);
//		finalize_setup();
//	}
//
//	sip::SIPMPIUtils::check_err(MPI_Barrier(MPI_COMM_WORLD));
//
//	setup::BinaryInputFile setup_file(job + ".dat");
//	setup::SetupReader setup_reader(setup_file);
//
//	std::cout << "SETUP READER DATA:\n" << setup_reader<< std::endl;
//
//
//	//get siox name from setup, load and print the sip tables
//	std::string prog_name = setup_reader.sial_prog_list_.at(0);
//	std::string siox_dir(dir_name);
//	setup::BinaryInputFile siox_file(siox_dir + prog_name);
//	sip::SipTables sipTables(setup_reader, siox_file);
//	if (!sip_mpi_attr.is_server()) {std::cout << "SIP TABLES" << '\n' << sipTables << std::endl;}
//
//	if (sip_mpi_attr.global_rank()==0){   std::cout << "\n\n\n\n>>>>>>>>>>>>starting SIAL PROGRAM  "<< job << std::endl;}
//
//	//create worker and server
//	sip::DataDistribution data_distribution(sipTables, sip_mpi_attr);
//	sip::GlobalState::set_program_name(prog_name);
//	sip::GlobalState::increment_program();
//	sip::WorkerPersistentArrayManager wpam;
//	sip::ServerPersistentArrayManager spam;
//
//	std::cout << "rank " << my_rank << " reached first barrier" << std::endl << std::flush;
//	MPI_Barrier(MPI_COMM_WORLD);
//	std::cout << "rank " << my_rank << " passed first barrier" << std::endl << std::flush;
//
//	if (sip_mpi_attr.is_server()){
//		sip::SIPServer server(sipTables, data_distribution, sip_mpi_attr, &spam);
//		std::cout << "at first barrier in prog 1 at server" << std::endl << std::flush;
//		MPI_Barrier(MPI_COMM_WORLD);
//		std::cout<<"passed first barrier at server, starting server" << std::endl;
//		server.run();
//		spam.save_marked_arrays(&server);
//		std::cout << "Server state after termination" << server << std::endl;
//	} else {
//		sip::SialxTimer sialxTimer(sipTables.max_timer_slots());
//		sip::Interpreter runner(sipTables, sialxTimer,  &wpam);
//		std::cout << "at first barrier in prog 1 at worker" << std::endl << std::flush;
//		MPI_Barrier(MPI_COMM_WORLD);
//		std::cout << "after first barrier; starting worker for "<< job  << std::endl;
//		runner.interpret();
//		wpam.save_marked_arrays(&runner);
//		std::cout << "\n end of prog1 at worker"<< std::endl;
//
//	}
//
//	std::cout << std::flush;
//	if (sip_mpi_attr.global_rank()==0){
//		std::cout << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@" << std::endl << std::flush;
//		std::cout << "SETUP READER DATA FOR SECOND PROGRAM:\n" << setup_reader<< std::endl;
//	}
//
//	std::string prog_name2 = setup_reader.sial_prog_list_.at(1);
//	setup::BinaryInputFile siox_file2(siox_dir + prog_name2);
//	sip::SipTables sipTables2(setup_reader, siox_file2);
//
//	if (sip_mpi_attr.global_rank()==0){
//		std::cout << "SIP TABLES FOR " << prog_name2 << '\n' << sipTables2 << std::endl;
//	}
//
//	sip::DataDistribution data_distribution2(sipTables2, sip_mpi_attr);
//	sip::GlobalState::set_program_name(prog_name);
//	sip::GlobalState::increment_program();
//	std::cout << "rank " << my_rank << " reached second barrier in test" << std::endl << std::flush;
//	MPI_Barrier(MPI_COMM_WORLD);
//	std::cout << "rank " << my_rank << " passed second barrier in test" << std::endl << std::flush;
//
//	if (sip_mpi_attr.is_server()){
//		sip::SIPServer server(sipTables2, data_distribution2, sip_mpi_attr, &spam);
//		std::cout << "barrier in prog 2 at server" << std::endl << std::flush;
//		MPI_Barrier(MPI_COMM_WORLD);
//		std::cout<< "rank " << my_rank << "starting server for prog 2" << std::endl;
//		server.run();
//		std::cout<< "rank " << my_rank  << "Server state after termination of prog2" << server << std::endl;
//	} else {
//		sip::SialxTimer sialxTimer2(sipTables2.max_timer_slots());
//		sip::Interpreter runner(sipTables2, sialxTimer2,  &wpam);
//		std::cout << "rank " << my_rank << "barrier in prog 2 at worker" << std::endl << std::flush;
//		MPI_Barrier(MPI_COMM_WORLD);
//		std::cout << "rank " << my_rank << "starting worker for prog2"<< job  << std::endl;
//		runner.interpret();
//		std::cout << "\nSIAL PROGRAM 2 TERMINATED"<< std::endl;
//
//
//		// Test contents of blocks of distributed array "b"
//
//		// Get the data for local array block "b"
//		int b_slot = runner.array_slot(std::string("lb"));
//
//		// Test b(1,1)
//		sip::index_selector_t b_indices_1;
//		b_indices_1[0] = 1; b_indices_1[1] = 1;
//		for (int i = 2; i < MAX_RANK; i++) b_indices_1[i] = sip::unused_index_value;
//		sip::BlockId b_bid_1(b_slot, b_indices_1);
//		std::cout << b_bid_1 << std::endl;
//		sip::Block::BlockPtr b_bptr_1 = runner.get_block_for_reading(b_bid_1);
//		sip::Block::dataPtr b_data_1 = b_bptr_1->get_data();
//		std::cout << " Comparing block " << b_bid_1 << std::endl;
//		double fill_seq_1_1 = 1.0;
//		for (int i=0; i<segs[0]; i++){
//			for (int j=0; j<segs[0]; j++){
//				ASSERT_DOUBLE_EQ(fill_seq_1_1, b_data_1[i*segs[0] + j]);
//				fill_seq_1_1++;
//			}
//		}
//
//		// Test b(2, 2)
//		sip::index_selector_t b_indices_2;
//		b_indices_2[0] = 2; b_indices_2[1] = 2;
//		for (int i = 2; i < MAX_RANK; i++) b_indices_2[i] = sip::unused_index_value;
//		sip::BlockId b_bid_2(b_slot, b_indices_2);
//		std::cout << b_bid_2 << std::endl;
//		sip::Block::BlockPtr b_bptr_2 = runner.get_block_for_reading(b_bid_2);
//		sip::Block::dataPtr b_data_2 = b_bptr_2->get_data();
//		std::cout << " Comparing block " << b_bid_2 << std::endl;
//		double fill_seq_2_2 = 4.0;
//		for (int i=0; i<segs[1]; i++){
//			for (int j=0; j<segs[1]; j++){
//				ASSERT_DOUBLE_EQ(fill_seq_2_2, b_data_2[i*segs[1] + j]);
//				fill_seq_2_2++;
//			}
//		}
//
//		// Test b(2,1)
//		sip::index_selector_t b_indices_3;
//		b_indices_3[0] = 2; b_indices_3[1] = 1;
//		for (int i = 2; i < MAX_RANK; i++) b_indices_3[i] = sip::unused_index_value;
//		sip::BlockId b_bid_3(b_slot, b_indices_3);
//		std::cout << b_bid_3 << std::endl;
//		sip::Block::BlockPtr b_bptr_3 = runner.get_block_for_reading(b_bid_3);
//		sip::Block::dataPtr b_data_3 = b_bptr_3->get_data();
//		std::cout << " Comparing block " << b_bid_3 << std::endl;
//		double fill_seq_2_1 = 3.0;
//		for (int i=0; i<segs[1]; i++){
//			for (int j=0; j<segs[0]; j++){
//				ASSERT_DOUBLE_EQ(fill_seq_2_1, b_data_3[i*segs[0] + j]);
//				fill_seq_2_1++;
//			}
//		}
//
//
//	}
//
//	std::cout << "rank " << my_rank << " reached third barrier in test" << std::endl << std::flush;
//	MPI_Barrier(MPI_COMM_WORLD);
//	std::cout << "rank " << my_rank << " passed third barrier in test" << std::endl << std::flush;
//
//}
//
//
///************************************************/
//
//TEST(SimpleMPI,get_mpi){
//
//	sip::GlobalState::reset_program_count();
//	sip::SIPMPIAttr &sip_mpi_attr = sip::SIPMPIAttr::get_instance();
//
//	std::cout << "****************************************\n";
//	sip::DataManager::scope_count=0;
//	//create setup_file
//	std::string job("get_mpi");
//	std::cout << "JOBNAME = " << job << std::endl;
//	double x = 3.456;
//	int norb = 4;
//	int segs[]  = {2,3,4,1};
//
//	if (sip_mpi_attr.global_rank() == 0){
//		init_setup(job.c_str());
//		set_scalar("x",x);
//		set_constant("norb",norb);
//		std::string tmp = job + ".siox";
//		const char* nm= tmp.c_str();
//		add_sial_program(nm);
//		set_aoindex_info(4,segs);
//		finalize_setup();
//	}
//
//	sip::SIPMPIUtils::check_err(MPI_Barrier(MPI_COMM_WORLD));
//
//	setup::BinaryInputFile setup_file(job + ".dat");
//	setup::SetupReader setup_reader(setup_file);
//
//	std::cout << "SETUP READER DATA:\n" << setup_reader<< std::endl;
//
//
//	//get siox name from setup, load and print the sip tables
//	std::string prog_name = setup_reader.sial_prog_list_.at(0);
//	std::string siox_dir(dir_name);
//	setup::BinaryInputFile siox_file(siox_dir + prog_name);
//	sip::SipTables sipTables(setup_reader, siox_file);
//
//	if (sip_mpi_attr.global_rank()==0){   std::cout << "\n\n\n\nstarting SIAL PROGRAM  "<< job << std::endl;}
//
//	sip::DataDistribution data_distribution(sipTables, sip_mpi_attr);
//	sip::GlobalState::set_program_name(prog_name);
//	sip::GlobalState::increment_program();
//
//	if (sip_mpi_attr.is_server()){
//		sip::SIPServer server(sipTables, data_distribution, sip_mpi_attr, NULL);
//		MPI_Barrier(MPI_COMM_WORLD);
//		std::cout<<"starting server" << std::endl;
//		server.run();
//		std::cout << "Server state after termination" << server << std::endl;
//	} else {
//		sip::SialxTimer sialxTimer(sipTables.max_timer_slots());
//		sip::Interpreter runner(sipTables, sialxTimer,  NULL);
//		MPI_Barrier(MPI_COMM_WORLD);
//		std::cout << "starting worker for "<< job  << std::endl;
//		runner.interpret();
//		std::cout << "\nSIAL PROGRAM TERMINATED"<< std::endl;
//		list_block_map();
//
//		// Test a(1,1)
//		// Get the data for local array block "b"
//		int a_slot = runner.array_slot(std::string("a"));
//
//		sip::index_selector_t a_indices_1;
//		a_indices_1[0] = 1; a_indices_1[1] = 1;
//		for (int i = 2; i < MAX_RANK; i++) a_indices_1[i] = sip::unused_index_value;
//		sip::BlockId a_bid_1(a_slot, a_indices_1);
//		std::cout << a_bid_1 << std::endl;
//		sip::Block::BlockPtr a_bptr_1 = runner.get_block_for_reading(a_bid_1);
//		sip::Block::dataPtr a_data_1 = a_bptr_1->get_data();
//		std::cout << " Comparing block " << a_bid_1 << std::endl;
//		for (int i=0; i<segs[0]; i++){
//			for (int j=0; j<segs[0]; j++){
//				ASSERT_DOUBLE_EQ(42*3, a_data_1[i*segs[0] + j]);
//			}
//		}
//	}
//}
//
///************************************************/
//
//// TODO ===========================================
//// TODO AUTOMATE
//// TODO ===========================================
//// Nothing should crash. Find something to test.
//TEST(SimpleMPI,unmatched_get){
//
//	sip::GlobalState::reset_program_count();
//	sip::SIPMPIAttr &sip_mpi_attr = sip::SIPMPIAttr::get_instance();
//
//	std::cout << "****************************************\n";
//	sip::DataManager::scope_count=0;
//	//create setup_file
//	std::string job("unmatched_get");
//	std::cout << "JOBNAME = " << job << std::endl;
//	double x = 3.456;
//	int norb = 4;
//
//
//	if (sip_mpi_attr.global_rank() == 0){
//		init_setup(job.c_str());
//		set_scalar("x",x);
//		set_constant("norb",norb);
//		std::string tmp = job + ".siox";
//		const char* nm= tmp.c_str();
//		add_sial_program(nm);
//		int segs[]  = {2,3,4,1};
//		set_aoindex_info(4,segs);
//		finalize_setup();
//	}
//
//	sip::SIPMPIUtils::check_err(MPI_Barrier(MPI_COMM_WORLD));
//
//	setup::BinaryInputFile setup_file(job + ".dat");
//	setup::SetupReader setup_reader(setup_file);
//
//	std::cout << "SETUP READER DATA:\n" << setup_reader<< std::endl;
//
//
//	//get siox name from setup, load and print the sip tables
//	std::string prog_name = setup_reader.sial_prog_list_.at(0);
//	std::string siox_dir(dir_name);
//	setup::BinaryInputFile siox_file(siox_dir + prog_name);
//	sip::SipTables sipTables(setup_reader, siox_file);
//
//	if (sip_mpi_attr.global_rank()==0){   std::cout << "\n\n\n\nstarting SIAL PROGRAM  "<< job << std::endl;}
//
//	sip::DataDistribution data_distribution(sipTables, sip_mpi_attr);
//	sip::GlobalState::set_program_name(prog_name);
//	sip::GlobalState::increment_program();
//
//	if (sip_mpi_attr.is_server()){
//		sip::SIPServer server(sipTables, data_distribution, sip_mpi_attr, NULL);
//		MPI_Barrier(MPI_COMM_WORLD);
//		std::cout<<"starting server" << std::endl;
//		server.run();
//		std::cout << "Server state after termination" << server << std::endl;
//	} else {
//		sip::SialxTimer sialxTimer(sipTables.max_timer_slots());
//		sip::Interpreter runner(sipTables, sialxTimer,  NULL);
//		MPI_Barrier(MPI_COMM_WORLD);
//		std::cout << "starting worker for "<< job  << std::endl;
//		runner.interpret();
//		std::cout << "\nSIAL PROGRAM TERMINATED"<< std::endl;
//		list_block_map();
//	}
//	std::cout << "this test should have printed a warning about unmatched get";
//}
//
//// TODO ===========================================
//// TODO AUTOMATE
//// TODO ===========================================
//// Nothing should crash. Find something to test.
//TEST(SimpleMPI,delete_mpi){
//
//	sip::GlobalState::reset_program_count();
//	sip::SIPMPIAttr &sip_mpi_attr = sip::SIPMPIAttr::get_instance();
//
//	std::cout << "****************************************\n";
//	sip::DataManager::scope_count=0;
//	//create setup_file
//	std::string job("delete_mpi");
//	std::cout << "JOBNAME = " << job << std::endl;
//	double x = 3.456;
//	int norb = 4;
//
//	if (sip_mpi_attr.global_rank() == 0){
//		init_setup(job.c_str());
//		set_scalar("x",x);
//		set_constant("norb",norb);
//		std::string tmp = job + ".siox";
//		const char* nm= tmp.c_str();
//		add_sial_program(nm);
//		int segs[]  = {2,3,4,1};
//		set_aoindex_info(4,segs);
//		finalize_setup();
//	}
//
//	sip::SIPMPIUtils::check_err(MPI_Barrier(MPI_COMM_WORLD));
//
//	setup::BinaryInputFile setup_file(job + ".dat");
//	setup::SetupReader setup_reader(setup_file);
//
//	std::cout << "SETUP READER DATA:\n" << setup_reader<< std::endl;
//
//	//get siox name from setup, load and print the sip tables
//	std::string prog_name = setup_reader.sial_prog_list_.at(0);
//	std::string siox_dir(dir_name);
//	setup::BinaryInputFile siox_file(siox_dir + prog_name);
//	sip::SipTables sipTables(setup_reader, siox_file);
//
//	if (sip_mpi_attr.global_rank()==0){   std::cout << "\n\n\n\nstarting SIAL PROGRAM  "<< job << std::endl;}
//
//	sip::DataDistribution data_distribution(sipTables, sip_mpi_attr);
//	sip::GlobalState::set_program_name(prog_name);
//	sip::GlobalState::increment_program();
//
//	if (sip_mpi_attr.is_server()){
//		sip::SIPServer server(sipTables, data_distribution, sip_mpi_attr, NULL);
//		MPI_Barrier(MPI_COMM_WORLD);
//		std::cout<<"starting server" << std::endl;
//		server.run();
//		std::cout << "Server state after termination" << server << std::endl;
//
//	} else {
//		sip::SialxTimer sialxTimer(sipTables.max_timer_slots());
//		sip::Interpreter runner(sipTables, sialxTimer, NULL);
//		MPI_Barrier(MPI_COMM_WORLD);
//		std::cout << "starting worker for "<< job  << std::endl;
//		runner.interpret();
//		std::cout << "\nSIAL PROGRAM TERMINATED"<< std::endl;
//	}
//}
//
//
//// TODO ===========================================
//// TODO AUTOMATE
//// TODO ===========================================
//// Nothing should crash. Find something to test.
//// Tested in get_mpi
//TEST(SimpleMPI,put_accumulate_mpi){
//
//	sip::GlobalState::reset_program_count();
//	sip::SIPMPIAttr &sip_mpi_attr = sip::SIPMPIAttr::get_instance();
//
//	std::cout << "****************************************\n";
//	sip::DataManager::scope_count=0;
//	//create setup_file
//	std::string job("put_accumulate_mpi");
//	std::cout << "JOBNAME = " << job << std::endl;
//	double x = 3.456;
//	int norb = 4;
//
//
//	if (sip_mpi_attr.global_rank() == 0){
//		init_setup(job.c_str());
//		set_scalar("x",x);
//		set_constant("norb",norb);
//		std::string tmp = job + ".siox";
//		const char* nm= tmp.c_str();
//		add_sial_program(nm);
//		int segs[]  = {2,3,4,1};
//		set_aoindex_info(4,segs);
//		finalize_setup();
//	}
//
//	sip::SIPMPIUtils::check_err(MPI_Barrier(MPI_COMM_WORLD));
//
//	setup::BinaryInputFile setup_file(job + ".dat");
//	// setup_reader.read(setup_file);
//	setup::SetupReader setup_reader(setup_file);
//
//	std::cout << "SETUP READER DATA:\n" << setup_reader<< std::endl;
//
//
//	//get siox name from setup, load and print the sip tables
//	std::string prog_name = setup_reader.sial_prog_list_.at(0);
//	std::string siox_dir(dir_name);
//	setup::BinaryInputFile siox_file(siox_dir + prog_name);
//	sip::SipTables sipTables(setup_reader, siox_file);
//
//	if (sip_mpi_attr.global_rank()==0){   std::cout << "\n\n\n\nstarting SIAL PROGRAM  "<< job << std::endl;}
//
//	sip::DataDistribution data_distribution(sipTables, sip_mpi_attr);
//	sip::GlobalState::set_program_name(prog_name);
//	sip::GlobalState::increment_program();
//
//	if (sip_mpi_attr.is_server()){
//		sip::SIPServer server(sipTables, data_distribution, sip_mpi_attr, NULL);
//		MPI_Barrier(MPI_COMM_WORLD);
//		std::cout<<"starting server" << std::endl;
//		server.run();
//		std::cout << "Server state after termination" << server << std::endl;
//	} else {
//		sip::SialxTimer sialxTimer(sipTables.max_timer_slots());
//		sip::Interpreter runner(sipTables, sialxTimer,  NULL);
//		MPI_Barrier(MPI_COMM_WORLD);
//		std::cout << "starting worker for "<< job  << std::endl;
//		runner.interpret();
//		std::cout << "\nSIAL PROGRAM TERMINATED"<< std::endl;
//	}
//}
//
//
//// TODO ===========================================
//// TODO AUTOMATE
//// TODO ===========================================
//// Nothing should crash. Find something to test.
//// Tested in get_mpi
//TEST(SimpleMPI,put_test_mpi){
//
//	sip::GlobalState::reset_program_count();
//	sip::SIPMPIAttr &sip_mpi_attr = sip::SIPMPIAttr::get_instance();
//
//	std::cout << "****************************************\n";
//	sip::DataManager::scope_count=0;
//	//create setup_file
//	std::string job("put_test_mpi");
//	std::cout << "JOBNAME = " << job << std::endl;
//	double x = 3.456;
//	int norb = 4;
//
//
//	if (sip_mpi_attr.global_rank() == 0){
//		init_setup(job.c_str());
//		set_scalar("x",x);
//		set_constant("norb",norb);
//		std::string tmp = job + ".siox";
//		const char* nm= tmp.c_str();
//		add_sial_program(nm);
//		int segs[]  = {2,3,4,1};
//		set_aoindex_info(4,segs);
//		finalize_setup();
//	}
//
//	sip::SIPMPIUtils::check_err(MPI_Barrier(MPI_COMM_WORLD));
//
//	setup::BinaryInputFile setup_file(job + ".dat");
//	setup::SetupReader setup_reader(setup_file);
//
//	std::cout << "SETUP READER DATA:\n" << setup_reader<< std::endl;
//
//	//get siox name from setup, load and print the sip tables
//	std::string prog_name = setup_reader.sial_prog_list_.at(0);
//	std::string siox_dir(dir_name);
//	setup::BinaryInputFile siox_file(siox_dir + prog_name);
//	sip::SipTables sipTables(setup_reader, siox_file);
//
//	//create worker and server
//	if (sip_mpi_attr.global_rank()==0){   std::cout << "\n\n\n\nstarting SIAL PROGRAM  "<< job << std::endl;}
//
//
//	sip::DataDistribution data_distribution(sipTables, sip_mpi_attr);
//	sip::GlobalState::set_program_name(prog_name);
//	sip::GlobalState::increment_program();
//	if (sip_mpi_attr.is_server()){
//		sip::SIPServer server(sipTables, data_distribution, sip_mpi_attr, NULL);
//		MPI_Barrier(MPI_COMM_WORLD);
//		std::cout<<"starting server" << std::endl;
//		server.run();
//		std::cout << "Server state after termination" << server << std::endl;
//	} else {
//		sip::SialxTimer sialxTimer(sipTables.max_timer_slots());
//		sip::Interpreter runner(sipTables, sialxTimer,  NULL);
//		MPI_Barrier(MPI_COMM_WORLD);
//		std::cout << "starting worker for "<< job  << std::endl;
//		runner.interpret();
//		std::cout << "\nSIAL PROGRAM TERMINATED"<< std::endl;
//	}
//}
//
//// TODO ===========================================
//// TODO AUTOMATE
//// TODO ===========================================
//// Nothing should crash. Find something to test.
//// Prints to stdout.
//TEST(SimpleMPI,all_rank_print){
//
//	sip::GlobalState::reset_program_count();
//	sip::SIPMPIAttr &sip_mpi_attr = sip::SIPMPIAttr::get_instance();
//
//	std::cout << "****************************************\n";
//	sip::DataManager::scope_count=0;
//	//create setup_file
//	std::string job("all_rank_print_test");
//	std::cout << "JOBNAME = " << job << std::endl;
//	double x = 3.456;
//	int norb = 2;
//
//
//	if (sip_mpi_attr.global_rank() == 0){
//		init_setup(job.c_str());
//		set_scalar("x",x);
//		set_constant("norb",norb);
//		std::string tmp = job + ".siox";
//		const char* nm= tmp.c_str();
//		add_sial_program(nm);
//		int segs[]  = {2,3};
//		set_aoindex_info(2,segs);
//		finalize_setup();
//	}
//
//	sip::SIPMPIUtils::check_err(MPI_Barrier(MPI_COMM_WORLD));
//
//	setup::BinaryInputFile setup_file(job + ".dat");
//	setup::SetupReader setup_reader(setup_file);
//
//	std::cout << "SETUP READER DATA:\n" << setup_reader<< std::endl;
//
//	//get siox name from setup, load and print the sip tables
//	std::string prog_name = setup_reader.sial_prog_list_.at(0);
//	std::string siox_dir(dir_name);
//	setup::BinaryInputFile siox_file(siox_dir + prog_name);
//	sip::SipTables sipTables(setup_reader, siox_file);
//
//	//create worker and server
//	if (sip_mpi_attr.global_rank()==0){   std::cout << "\n\n\n\nstarting SIAL PROGRAM  "<< job << std::endl;}
//
//
//	sip::DataDistribution data_distribution(sipTables, sip_mpi_attr);
//	sip::GlobalState::set_program_name(prog_name);
//	sip::GlobalState::increment_program();
//	if (sip_mpi_attr.is_server()){
//		sip::SIPServer server(sipTables, data_distribution, sip_mpi_attr, NULL);
//		MPI_Barrier(MPI_COMM_WORLD);
//		std::cout<<"starting server" << std::endl;
//		server.run();
//		std::cout << "Server state after termination" << server << std::endl;
//	} else {
//		sip::SialxTimer sialxTimer(sipTables.max_timer_slots());
//		sip::Interpreter runner(sipTables, sialxTimer,  NULL);
//		MPI_Barrier(MPI_COMM_WORLD);
//		std::cout << "starting worker for "<< job  << std::endl;
//		runner.interpret();
//		std::cout << "\nSIAL PROGRAM TERMINATED"<< std::endl;
//	}
//}
//
//
//// TODO ===========================================
//// TODO AUTOMATE
//// TODO ===========================================
//// Nothing should crash. Find something to test.
//TEST(SimpleMPI,Message_Number_Wraparound){
//
//	sip::GlobalState::reset_program_count();
//	sip::SIPMPIAttr &sip_mpi_attr = sip::SIPMPIAttr::get_instance();
//
//	std::cout << "****************************************\n";
//	sip::DataManager::scope_count=0;
//	//create setup_file
//	std::string job("message_number_wraparound_test");
//	std::cout << "JOBNAME = " << job << std::endl;
//
//	if (sip_mpi_attr.global_rank() == 0){
//		init_setup(job.c_str());
//		set_constant("norb",1);
//		std::string tmp = job + ".siox";
//		const char* nm= tmp.c_str();
//		add_sial_program(nm);
//		int segs[]  = {1};
//		set_aoindex_info(1,segs);
//		finalize_setup();
//	}
//
//	sip::SIPMPIUtils::check_err(MPI_Barrier(MPI_COMM_WORLD));
//
//	setup::BinaryInputFile setup_file(job + ".dat");
//	setup::SetupReader setup_reader(setup_file);
//
//	std::cout << "SETUP READER DATA:\n" << setup_reader<< std::endl;
//
//	//get siox name from setup, load and print the sip tables
//	std::string prog_name = setup_reader.sial_prog_list_.at(0);
//	std::string siox_dir(dir_name);
//	setup::BinaryInputFile siox_file(siox_dir + prog_name);
//	sip::SipTables sipTables(setup_reader, siox_file);
//
//	//create worker and server
//	if (sip_mpi_attr.global_rank()==0){   std::cout << "\n\n\n\nstarting SIAL PROGRAM  "<< job << std::endl;}
//
//
//	sip::DataDistribution data_distribution(sipTables, sip_mpi_attr);
//	sip::GlobalState::set_program_name(prog_name);
//	sip::GlobalState::increment_program();
//	if (sip_mpi_attr.is_server()){
//		sip::SIPServer server(sipTables, data_distribution, sip_mpi_attr, NULL);
//		MPI_Barrier(MPI_COMM_WORLD);
//		std::cout<<"starting server" << std::endl;
//		server.run();
//		std::cout << "Server state after termination" << server << std::endl;
//	} else {
//		sip::SialxTimer sialxTimer(sipTables.max_timer_slots());
//		sip::Interpreter runner(sipTables, sialxTimer,  NULL);
//		MPI_Barrier(MPI_COMM_WORLD);
//		std::cout << "starting worker for "<< job  << std::endl;
//		runner.interpret();
//		std::cout << "\nSIAL PROGRAM TERMINATED"<< std::endl;
//	}
//}
//
//
//TEST(SimpleMPI,Pardo_Loop_Test){
//
//	sip::GlobalState::reset_program_count();
//	sip::SIPMPIAttr &sip_mpi_attr = sip::SIPMPIAttr::get_instance();
//
//	std::cout << "****************************************\n";
//	sip::DataManager::scope_count=0;
//	//create setup_file
//	std::string job("pardo_loop");
//	std::cout << "JOBNAME = " << job << std::endl;
//
//	if (sip_mpi_attr.global_rank() == 0){
//		init_setup(job.c_str());
//		set_constant("norb",1);
//		std::string tmp = job + ".siox";
//		const char* nm= tmp.c_str();
//		add_sial_program(nm);
//		int segs[]  = {1};
//		set_aoindex_info(1,segs);
//		finalize_setup();
//	}
//
//	sip::SIPMPIUtils::check_err(MPI_Barrier(MPI_COMM_WORLD));
//
//	setup::BinaryInputFile setup_file(job + ".dat");
//	setup::SetupReader setup_reader(setup_file);
//
//	std::cout << "SETUP READER DATA:\n" << setup_reader<< std::endl;
//
//	//get siox name from setup, load and print the sip tables
//	std::string prog_name = setup_reader.sial_prog_list_.at(0);
//	std::string siox_dir(dir_name);
//	setup::BinaryInputFile siox_file(siox_dir + prog_name);
//	sip::SipTables sipTables(setup_reader, siox_file);
//
//	//create worker and server
//	if (sip_mpi_attr.global_rank()==0){   std::cout << "\n\n\n\nstarting SIAL PROGRAM  "<< job << std::endl;}
//
//
//	sip::DataDistribution data_distribution(sipTables, sip_mpi_attr);
//	sip::GlobalState::set_program_name(prog_name);
//	sip::GlobalState::increment_program();
//	if (sip_mpi_attr.is_server()){
//		sip::SIPServer server(sipTables, data_distribution, sip_mpi_attr, NULL);
//		MPI_Barrier(MPI_COMM_WORLD);
//		std::cout<<"starting server" << std::endl;
//		server.run();
//		std::cout << "Server state after termination" << server << std::endl;
//	} else {
//		sip::SialxTimer sialxTimer(sipTables.max_timer_slots());
//		sip::Interpreter runner(sipTables, sialxTimer,  NULL);
//		MPI_Barrier(MPI_COMM_WORLD);
//		std::cout << "starting worker for "<< job  << std::endl;
//		runner.interpret();
//		std::cout << "\nSIAL PROGRAM TERMINATED"<< std::endl;
//		ASSERT_DOUBLE_EQ(80, runner.data_manager_.scalar_value("total"));
//	}
//}
//

int main(int argc, char **argv) {

#ifdef HAVE_MPI
	MPI_Init(&argc, &argv);
	int num_procs;
	sip::SIPMPIUtils::check_err(MPI_Comm_size(MPI_COMM_WORLD, &num_procs));

		if (num_procs < 2){
			std::cerr<<"Please run this test with at least 2 mpi ranks"<<std::endl;
			return -1;
		}
	sip::SIPMPIUtils::set_error_handler();
	sip::SIPMPIAttr &sip_mpi_attr = sip::SIPMPIAttr::get_instance();
	attr = &sip_mpi_attr;
#endif

#ifdef HAVE_TAU
	TAU_PROFILE_SET_NODE(0);
	TAU_STATIC_PHASE_START("SIP Main");
#endif

//	sip::check(sizeof(int) >= 4, "Size of integer should be 4 bytes or more");
//	sip::check(sizeof(double) >= 8, "Size of double should be 8 bytes or more");
//	sip::check(sizeof(long long) >= 8, "Size of long long should be 8 bytes or more");
//
//	int num_procs;
//	sip::SIPMPIUtils::check_err(MPI_Comm_size(MPI_COMM_WORLD, &num_procs));
//
//	if (num_procs < 2){
//		std::cerr<<"Please run this test with at least 2 mpi ranks"<<std::endl;
//		return -1;
//	}
//
//	sip::SIPMPIUtils::set_error_handler();
//	sip::SIPMPIAttr &sip_mpi_attr = sip::SIPMPIAttr::get_instance();
//

	printf("Running main() from master_test_main.cpp\n");
	testing::InitGoogleTest(&argc, argv);
	int result = RUN_ALL_TESTS();

#ifdef HAVE_TAU
	TAU_STATIC_PHASE_STOP("SIP Main");
#endif
#ifdef HAVE_MPI
	 MPI_Finalize();
#endif
	return result;

}
