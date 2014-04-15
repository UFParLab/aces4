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

#include "block.h"

#ifdef HAVE_TAU
#include <TAU.h>
#endif

static const std::string dir_name("src/sialx/test/");

extern "C"{
int test_transpose_op(double*);
int test_transpose4d_op(double*);
int test_contraction_small2(double*);
}

TEST(Sial,empty){
	std::cout << "****************************************\n";
	sip::DataManager::scope_count=0;
	//create setup_file
	std::string job("empty");
	std::cout << "JOBNAME = " << job << std::endl;

	//initialize setup data

	//no setup file, but the siox readers expects a SetupReader, so create an empty one.
	setup::SetupReader &setup_reader = *setup::SetupReader::get_empty_reader();
	std::cout << "SETUP READER DATA:\n" << setup_reader<< std::endl;


	std::string prog_name = job + ".siox";
	std::string siox_dir(dir_name);
	setup::BinaryInputFile siox_file(siox_dir + prog_name);
//	// sip::SioxReader siox_reader(&sipTables, &siox_file, &setup_reader);
////	siox_reader.read();
	sip::SipTables sipTables(setup_reader, siox_file);
	std::cout << "SIP TABLES" << '\n' << sipTables << std::endl;

	//interpret the program
	{
	sip::SialxTimer sialxTimer(sipTables.max_timer_slots());

	sip::Interpreter runner(sipTables, sialxTimer);
	std::cout << "SIAL PROGRAM OUTPUT" << std::endl;
	runner.interpret();
	std::cout << "\nSIAL PROGRAM TERMINATED"<< std::endl;
	}
}

TEST(Sial,scalars){
	std::cout << "****************************************\n";
	sip::DataManager::scope_count=0;
	//create setup_file
	std::string job("scalars");
	std::cout << "JOBNAME = " << job << std::endl;
	double x = 3.456;
	double y = -0.1;

	{	init_setup(job.c_str());
		set_scalar("x",x);
		set_scalar("y",y);
		std::string tmp = job + ".siox";
		const char* nm= tmp.c_str();
		add_sial_program(nm);
		finalize_setup();
	}

	//read and print setup_file
	// setup::SetupReader setup_reader;
	setup::BinaryInputFile setup_file(job + ".dat");
	setup::SetupReader setup_reader(setup_file);
	// setup_reader.read(setup_file);
	std::cout << "SETUP READER DATA:\n" << setup_reader<< std::endl;

	//get siox name from setup, load and print the sip tables
	std::string prog_name = setup_reader.sial_prog_list_.at(0);
	std::string siox_dir(dir_name);
	setup::BinaryInputFile siox_file(siox_dir + prog_name);
//	// sip::SioxReader siox_reader(&sipTables, &siox_file, &setup_reader);
////	siox_reader.read();
	sip::SipTables sipTables(setup_reader, siox_file);
	std::cout << "SIP TABLES" << '\n' << sipTables << std::endl;

	//interpret the program
	{
	sip::SialxTimer sialxTimer(sipTables.max_timer_slots());
	sip::Interpreter runner(sipTables, sialxTimer);
	std::cout << "SIAL PROGRAM OUTPUT" << std::endl;
	runner.interpret();
	std::cout << "\nSIAL PROGRAM TERMINATED"<< std::endl;
	ASSERT_DOUBLE_EQ(x, scalar_value("x"));
	ASSERT_DOUBLE_EQ(y, scalar_value("y"));
	ASSERT_DOUBLE_EQ(x, scalar_value("z"));
	ASSERT_DOUBLE_EQ(99.99, scalar_value("zz"));
	ASSERT_EQ(0, sip::DataManager::scope_count);
	}
}


TEST(Sial,persistent_scalars){
	std::cout << "****************************************\n";
	sip::DataManager::scope_count=0;
	//create setup_file
	std::string job("persistent_scalars");
	std::cout << "JOBNAME = " << job << std::endl;
	double x = 3.456;
	double y = -0.1;

	{	init_setup(job.c_str());
		set_scalar("x",x);
		set_scalar("y",y);
		std::string tmp1 = job + "_1.siox";
		const char* nm1= tmp1.c_str();
		add_sial_program(nm1);
		std::string tmp2 = job + "_2.siox";
		const char* nm2= tmp2.c_str();
		add_sial_program(nm2);
		finalize_setup();
	}

	sip::PersistentArrayManager<sip::Block, sip::Interpreter>* pam;
	pam = new sip::PersistentArrayManager<sip::Block, sip::Interpreter>();

	//read and print setup_file
	// setup::SetupReader setup_reader;
	setup::BinaryInputFile setup_file(job + ".dat");
	setup::SetupReader setup_reader(setup_file);
	// setup_reader.read(setup_file);
	std::cout << "SETUP READER DATA:\n" << setup_reader<< std::endl;

	//get siox name from setup, load and print the sip tables
	std::string prog_name = setup_reader.sial_prog_list_.at(0);
	std::string siox_dir(dir_name);
	setup::BinaryInputFile siox_file(siox_dir + prog_name);
//	// sip::SioxReader siox_reader(&sipTables, &siox_file, &setup_reader);
////	siox_reader.read();
	sip::SipTables sipTables(setup_reader, siox_file);
	std::cout << "SIP TABLES" << '\n' << sipTables << std::endl;

	//interpret the program
	{
	sip::SialxTimer sialxTimer(sipTables.max_timer_slots());
	sip::Interpreter runner(sipTables, sialxTimer, pam);
	std::cout << "SIAL PROGRAM OUTPUT" << std::endl;
	runner.interpret();
	std::cout << "\nSIAL PROGRAM TERMINATED"<< std::endl;
//	ASSERT_DOUBLE_EQ(x, scalar_value("x"));
	ASSERT_DOUBLE_EQ(y, scalar_value("y"));
	ASSERT_DOUBLE_EQ(x, scalar_value("z"));
	ASSERT_DOUBLE_EQ(99.99, scalar_value("zz"));
	ASSERT_EQ(0, sip::DataManager::scope_count);
	pam->save_marked_arrays(&runner);
	std::cout << "pam:" << std::endl << *pam << std::endl << "%%%%%%%%%%%%"<< std::endl;
	}

	//Now do the second program
	//get siox name from setup, load and print the sip tables
	std::string prog_name1 = setup_reader.sial_prog_list_.at(1);
	setup::BinaryInputFile siox_file1(siox_dir + prog_name1);
	sip::SipTables sipTables1(setup_reader, siox_file1);
	std::cout << "SIP TABLES" << '\n' << sipTables << std::endl;

	//interpret the program
	{
	sip::SialxTimer sialxTimer1(sipTables1.max_timer_slots());
	sip::Interpreter runner2(sipTables1, sialxTimer1, pam);
	std::cout << "SIAL PROGRAM OUTPUT" << std::endl;
	runner2.interpret();
	std::cout << "\nSIAL PROGRAM TERMINATED"<< std::endl;
	ASSERT_DOUBLE_EQ(x+1, scalar_value("x"));
	ASSERT_DOUBLE_EQ(y, scalar_value("y"));
	ASSERT_DOUBLE_EQ(6, scalar_value("e"));
	ASSERT_EQ(0, sip::DataManager::scope_count);
	}
	delete pam;
}



TEST(Sial,helloworld){
	std::cout << "****************************************\n";
	sip::DataManager::scope_count=0;
	//create setup_file
	std::string job("helloworld");
	std::cout << "JOBNAME = " << job << std::endl;

	//no setup file, but the siox readers expects a SetupReader, so create an empty one.
	setup::SetupReader &setup_reader = *setup::SetupReader::get_empty_reader();

	std::cout << "SETUP READER DATA:\n" << setup_reader<< std::endl;


	std::string siox_dir(dir_name);
	setup::BinaryInputFile siox_file(siox_dir + job + ".siox");
//	// sip::SioxReader siox_reader(&sipTables, &siox_file, &setup_reader);
//	siox_reader.read();
	sip::SipTables sipTables(setup_reader, siox_file);
	std::cout << "SIP TABLES" << '\n' << sipTables << std::endl;

	//interpret the program
	{
	sip::SialxTimer sialxTimer(sipTables.max_timer_slots());

	sip::Interpreter runner(sipTables, sialxTimer);
	std::cout << "SIAL PROGRAM OUTPUT" << std::endl;
	runner.interpret();
	std::cout << "\nSIAL PROGRAM TERMINATED"<< std::endl;
	ASSERT_EQ(0, sip::DataManager::scope_count);

	}
}


TEST(Sial,no_arg_user_sub) {
	std::cout << "****************************************\n";
	sip::DataManager::scope_count=0;
	//no setup file, but the siox readers expects a SetupReader, so create an empty one.
	setup::SetupReader &setup_reader = *setup::SetupReader::get_empty_reader();

	std::string name("no_arg_user_sub.siox");
	std::string siox_dir(dir_name);
	setup::BinaryInputFile siox_file(siox_dir + name);

	sip::SipTables sipTables(setup_reader, siox_file);
//	// sip::SioxReader siox_reader(&sipTables, &siox_file, &setup_reader);
//	siox_reader.read();

	std::cout << "SIP TABLES" << std::endl;
	std::cout << sipTables;

	sip::SialxTimer sialxTimer(sipTables.max_timer_slots());
	sip::PersistentArrayManager<sip::Block,sip::Interpreter>* pbm;
	sip::Interpreter runner(sipTables, sialxTimer);
	std::cout << "\nINSTANTIATED INTERPRETER" << std::endl;
	runner.interpret();
	ASSERT_EQ(0, sip::DataManager::scope_count);
	std::cout << "\nCOMPLETED PROGRAM"<< std::endl;
	std::cout << "\nafter Interpreter destructor"<< std::endl;
}



TEST(Sial,index_decs) {
	std::cout << "****************************************\n";
	sip::DataManager::scope_count=0;

	//create setup_file
	std::string job("index_decs");

	{
		init_setup(job.c_str());
		//now add data
		set_constant("norb",15);
		int segs[] = {5,6,7,8};
		set_aoindex_info(4,segs);

		//add the first program for this job ad finalize--no need to change this if only one sial program
		std::string tmp = job + ".siox";
		const char* nm= tmp.c_str();
		add_sial_program(nm);
		finalize_setup();
	}

	//read and print setup_file;
	setup::BinaryInputFile setup_file(job + ".dat");
	setup::SetupReader setup_reader(setup_file);
	// setup_reader.read(setup_file);
	setup_reader.dump_data();

	//get siox name from setup, load and print the sip tables
	std::string prog_name = setup_reader.sial_prog_list_.at(0);
	std::string siox_dir(dir_name);
	setup::BinaryInputFile siox_file(siox_dir + prog_name);
	sip::SipTables sipTables(setup_reader, siox_file);
//	// sip::SioxReader siox_reader(&sipTables, &siox_file, &setup_reader);
//	siox_reader.read();
	std::cout << "SIP TABLES" << '\n' << sipTables;

	//interpret the program
	sip::SialxTimer sialxTimer(sipTables.max_timer_slots());
	sip::PersistentArrayManager<sip::Block,sip::Interpreter>* pbm;
	sip::Interpreter runner(sipTables, sialxTimer);
	std::cout << "\nSIAL PROGRAM OUTPUT" << std::endl;
	runner.interpret();
	ASSERT_EQ(0, sip::DataManager::scope_count);
	std::cout << "\nCOMPLETED PROGRAM"<< std::endl;
}


TEST(Sial,where_clause){
	std::cout << "****************************************\n";
	sip::DataManager::scope_count=0;
	//create setup_file
	std::string job("where_clause");
	std::cout << "JOBNAME = " << job << std::endl;

	//initialize setup data

	//no setup file, but the siox readers expects a SetupReader, so create an empty one.
	// setup::SetupReader setup_reader;
	setup::SetupReader &setup_reader = *setup::SetupReader::get_empty_reader();

	std::cout << "SETUP READER DATA:\n" << setup_reader<< std::endl;


	std::string prog_name = job + ".siox";
	std::string siox_dir(dir_name);
	setup::BinaryInputFile siox_file(siox_dir + prog_name);
//	// sip::SioxReader siox_reader(&sipTables, &siox_file, &setup_reader);
//	siox_reader.read();
	sip::SipTables sipTables(setup_reader, siox_file);
	std::cout << "SIP TABLES" << '\n' << sipTables << std::endl;

	//interpret the program
	{
	sip::SialxTimer sialxTimer(sipTables.max_timer_slots());
	sip::PersistentArrayManager<sip::Block,sip::Interpreter>* pbm;
	sip::Interpreter runner(sipTables, sialxTimer);
	std::cout << "SIAL PROGRAM OUTPUT" << std::endl;
	runner.interpret();
	ASSERT_EQ(0, sip::DataManager::scope_count);
	std::cout << "\nSIAL PROGRAM TERMINATED"<< std::endl;
	}
}

TEST(Sial,ifelse){
	std::cout << "****************************************\n";
	sip::DataManager::scope_count=0;
	//create setup_file
	std::string job("ifelse");
	std::cout << "JOBNAME = " << job << std::endl;

	//initialize setup data

	//no setup file, but the siox readers expects a SetupReader, so create an empty one.
	setup::SetupReader &setup_reader = *setup::SetupReader::get_empty_reader();

	std::cout << "SETUP READER DATA:\n" << setup_reader<< std::endl;


	std::string prog_name = job + ".siox";
	std::string siox_dir(dir_name);
	setup::BinaryInputFile siox_file(siox_dir + prog_name);
	// sip::SioxReader siox_reader(&sipTables, &siox_file, &setup_reader);
//	siox_reader.read();
	sip::SipTables sipTables(setup_reader, siox_file);
	std::cout << "SIP TABLES" << '\n' << sipTables << std::endl;

	//interpret the program
	{
	sip::SialxTimer sialxTimer(sipTables.max_timer_slots());
	sip::PersistentArrayManager<sip::Block,sip::Interpreter>* pbm;
	sip::Interpreter runner(sipTables, sialxTimer);
	std::cout << "SIAL PROGRAM OUTPUT" << std::endl;
	runner.interpret();
	ASSERT_EQ(0, sip::DataManager::scope_count);
	std::cout << "\nSIAL PROGRAM TERMINATED"<< std::endl;
	}
}

TEST(Sial,loop_over_simple_indices){
	std::cout << "****************************************\n";
	sip::DataManager::scope_count=0;
	//create setup_file
	std::string job("loop_over_simple_indices");
	std::cout << "JOBNAME = " << job << std::endl;

	//initialize setup data

	//no setup file, but the siox readers expects a SetupReader, so create an empty one.
	setup::SetupReader &setup_reader = *setup::SetupReader::get_empty_reader();

	std::cout << "SETUP READER DATA:\n" << setup_reader<< std::endl;


	std::string prog_name = job + ".siox";
	std::string siox_dir(dir_name);
	setup::BinaryInputFile siox_file(siox_dir + prog_name);
	// sip::SioxReader siox_reader(&sipTables, &siox_file, &setup_reader);
	//	siox_reader.read();
	sip::SipTables sipTables(setup_reader, siox_file);
	std::cout << "SIP TABLES" << '\n' << sipTables << std::endl;

	//interpret the program
	{
	sip::SialxTimer sialxTimer(sipTables.max_timer_slots());
	sip::PersistentArrayManager<sip::Block,sip::Interpreter>* pbm;
	sip::Interpreter runner(sipTables, sialxTimer);
	std::cout << "SIAL PROGRAM OUTPUT" << std::endl;
	runner.interpret();
	ASSERT_EQ(0, sip::DataManager::scope_count);
	std::cout << "\nSIAL PROGRAM TERMINATED"<< std::endl;
	}
	}
	TEST(Sial,pardo_loop){
	std::cout << "****************************************\n";
	sip::DataManager::scope_count=0;
	//create setup_file
	std::string job("pardo_loop");
	std::cout << "JOBNAME = " << job << std::endl;

	//initialize setup data

	//no setup file, but the siox readers expects a SetupReader, so create an empty one.
	setup::SetupReader &setup_reader = *setup::SetupReader::get_empty_reader();

	std::cout << "SETUP READER DATA:\n" << setup_reader<< std::endl;


	std::string prog_name = job + ".siox";
	std::string siox_dir(dir_name);
	setup::BinaryInputFile siox_file(siox_dir + prog_name);
	// sip::SioxReader siox_reader(&sipTables, &siox_file, &setup_reader);
	//	siox_reader.read();
	sip::SipTables sipTables(setup_reader, siox_file);
	std::cout << "SIP TABLES" << '\n' << sipTables << std::endl;

	//interpret the program
	{
	sip::SialxTimer sialxTimer(sipTables.max_timer_slots());
	sip::PersistentArrayManager<sip::Block,sip::Interpreter>* pbm;
	sip::Interpreter runner(sipTables, sialxTimer);
	std::cout << "SIAL PROGRAM OUTPUT" << std::endl;
	runner.interpret();
	ASSERT_EQ(0, sip::DataManager::scope_count);
	ASSERT_DOUBLE_EQ(80, sip::Interpreter::global_interpreter->data_manager_.scalar_value("counter"));
	std::cout << "\nSIAL PROGRAM TERMINATED"<< std::endl;
	}
}
TEST(Sial,tmp_arrays){
	std::cout << "****************************************\n";
	sip::DataManager::scope_count=0;
	//create setup_file
	std::string job("tmp_arrays");
	std::cout << "JOBNAME = " << job << std::endl;
	double x = 3.456;
	double y = -0.1;
	int norb = 3;

	{	init_setup(job.c_str());
		set_scalar("x",x);
		set_scalar("y",y);
		set_constant("norb",norb);
		std::string tmp = job + ".siox";
		const char* nm= tmp.c_str();
		add_sial_program(nm);
		int segs[]  = {2,3,4};
		set_aoindex_info(3,segs);
		finalize_setup();
	}

	//read and print setup_file
		setup::BinaryInputFile setup_file(job + ".dat");
	setup::SetupReader setup_reader(setup_file);
	std::cout << "SETUP READER DATA:\n" << setup_reader<< std::endl;


	//get siox name from setup, load and print the sip tables
	std::string prog_name = setup_reader.sial_prog_list_.at(0);
	std::string siox_dir(dir_name);
	setup::BinaryInputFile siox_file(siox_dir + prog_name);
	// sip::SioxReader siox_reader(&sipTables, &siox_file, &setup_reader);
	//	siox_reader.read();
	sip::SipTables sipTables(setup_reader, siox_file);
	std::cout << "SIP TABLES" << '\n' << sipTables << std::endl;

	//interpret the program
	{
	sip::SialxTimer sialxTimer(sipTables.max_timer_slots());
	sip::PersistentArrayManager<sip::Block,sip::Interpreter>* pbm;
	sip::Interpreter runner(sipTables, sialxTimer);
	std::cout << "SIAL PROGRAM OUTPUT" << std::endl;
	runner.interpret();
	ASSERT_EQ(0, sip::DataManager::scope_count);
	std::cout << "\nSIAL PROGRAM TERMINATED"<< std::endl;
	;
	}
}

TEST(Sial,tmp_arrays_2){
	std::cout << "****************************************\n";
	sip::DataManager::scope_count=0;
	//create setup_file
	std::string job("tmp_arrays_2");
	std::cout << "JOBNAME = " << job << std::endl;
	double x = 3.456;
	double y = -0.1;
	int norb = 3;

	{	init_setup(job.c_str());
		set_scalar("x",x);
		set_scalar("y",y);
		set_constant("norb",norb);
		std::string tmp = job + ".siox";
		const char* nm= tmp.c_str();
		add_sial_program(nm);
		int segs[]  = {2,3,4};
		set_aoindex_info(3,segs);
		finalize_setup();
	}

	//read and print setup_file

	setup::BinaryInputFile setup_file(job + ".dat");
	setup::SetupReader setup_reader(setup_file);

	std::cout << "SETUP READER DATA:\n" << setup_reader<< std::endl;


	//get siox name from setup, load and print the sip tables
	std::string prog_name = setup_reader.sial_prog_list_.at(0);
	std::string siox_dir(dir_name);
	setup::BinaryInputFile siox_file(siox_dir + prog_name);
	// sip::SioxReader siox_reader(&sipTables, &siox_file, &setup_reader);
	//	siox_reader.read();
	sip::SipTables sipTables(setup_reader, siox_file);
	std::cout << "SIP TABLES" << '\n' << sipTables << std::endl;

	//interpret the program
	{
	sip::SialxTimer sialxTimer(sipTables.max_timer_slots());
	sip::PersistentArrayManager<sip::Block,sip::Interpreter>* pbm;
	sip::Interpreter runner(sipTables, sialxTimer);
	std::cout << "SIAL PROGRAM OUTPUT" << std::endl;
	runner.interpret();
	ASSERT_EQ(0, sip::DataManager::scope_count);
	std::cout << "\nSIAL PROGRAM TERMINATED"<< std::endl;
	;
	}
}


TEST(Sial,exit_statement_test){

	std::cout << "****************************************\n";
	sip::DataManager::scope_count=0;
	//create setup_file
	std::string job("exit_statement_test");
	std::cout << "JOBNAME = " << job << std::endl;
	double x = 3.456;
	double y = -0.1;
	int norb = 3;

	{	init_setup(job.c_str());
		set_scalar("x",x);
		set_scalar("y",y);
		set_constant("norb",norb);
		std::string tmp = job + ".siox";
		const char* nm= tmp.c_str();
		add_sial_program(nm);
		int segs[]  = {2,3,4,2,3,4,2,3,4,2,3,4,2,3,4};
		set_aoindex_info(15,segs);
		finalize_setup();
	}

	//read and print setup_file
		setup::BinaryInputFile setup_file(job + ".dat");
	setup::SetupReader setup_reader(setup_file);
	std::cout << "SETUP READER DATA:\n" << setup_reader<< std::endl;


	//get siox name from setup, load and print the sip tables
	std::string prog_name = setup_reader.sial_prog_list_.at(0);
	std::string siox_dir(dir_name);
	setup::BinaryInputFile siox_file(siox_dir + prog_name);
	// sip::SioxReader siox_reader(&sipTables, &siox_file, &setup_reader);
	//	siox_reader.read();
	sip::SipTables sipTables(setup_reader, siox_file);
	std::cout << "SIP TABLES" << '\n' << sipTables << std::endl;

	//interpret the program
	{
	sip::SialxTimer sialxTimer(sipTables.max_timer_slots());
	sip::PersistentArrayManager<sip::Block,sip::Interpreter>* pbm;
	sip::Interpreter runner(sipTables, sialxTimer);
	std::cout << "SIAL PROGRAM OUTPUT" << std::endl;
	runner.interpret();
	ASSERT_EQ(0, sip::DataManager::scope_count);
	std::cout << "\nSIAL PROGRAM TERMINATED"<< std::endl;

	;
	}
}

TEST(Sial,transpose_tmp){
	std::cout << "****************************************\n";
	sip::DataManager::scope_count=0;
	//create setup_file
	std::string job("transpose_tmp");
	std::cout << "JOBNAME = " << job << std::endl;
	double x = 3.456;
	double y = -0.1;
	int norb = 3;

	{	init_setup(job.c_str());
		set_scalar("x",x);
		set_scalar("y",y);
		set_constant("norb",norb);
		std::string tmp = job + ".siox";
		const char* nm= tmp.c_str();
		add_sial_program(nm);
		int segs[]  = {8,12,10};
		set_aoindex_info(3,segs);
		finalize_setup();
	}

	//read and print setup_file
		setup::BinaryInputFile setup_file(job + ".dat");
	setup::SetupReader setup_reader(setup_file);
	std::cout << "SETUP READER DATA:\n" << setup_reader<< std::endl;


	//get siox name from setup, load and print the sip tables
	std::string prog_name = setup_reader.sial_prog_list_.at(0);
	std::string siox_dir(dir_name);
	setup::BinaryInputFile siox_file(siox_dir + prog_name);
	// sip::SioxReader siox_reader(&sipTables, &siox_file, &setup_reader);
	//	siox_reader.read();
	sip::SipTables sipTables(setup_reader, siox_file);
	std::cout << "SIP TABLES" << '\n' << sipTables << std::endl;

	//interpret the program
	{
	sip::SialxTimer sialxTimer(sipTables.max_timer_slots());
	sip::PersistentArrayManager<sip::Block,sip::Interpreter>* pbm;
	sip::Interpreter runner(sipTables, sialxTimer);
	std::cout << "SIAL PROGRAM OUTPUT" << std::endl;
	runner.interpret();
	ASSERT_EQ(0, sip::DataManager::scope_count);
	std::cout << "\nSIAL PROGRAM TERMINATED"<< std::endl;

	// Get the data for local array block "b"
	int b_slot = sip::Interpreter::global_interpreter->array_slot(std::string("b"));
	sip::index_selector_t b_indices;
	b_indices[0] = 1; b_indices[1] = 1;
	for (int i = 2; i < MAX_RANK; i++) b_indices[i] = sip::unused_index_value;
	sip::BlockId b_bid(b_slot, b_indices);
	std::cout << b_bid << std::endl;
	sip::Block::BlockPtr b_bptr = sip::Interpreter::global_interpreter->get_block_for_reading(b_bid);
	sip::Block::dataPtr b_data = b_bptr->get_data();

	int passed = test_transpose_op(b_data);
	ASSERT_TRUE(passed);

	}

}

TEST(Sial,fill_sequential){
	std::cout << "****************************************\n";
	sip::DataManager::scope_count=0;
	//create setup_file
	std::string job("fill_sequential_test");
	std::cout << "JOBNAME = " << job << std::endl;
	double x = 3.456;
	int norb = 2;

	{	init_setup(job.c_str());
		set_scalar("x",x);
		set_constant("norb",norb);
		std::string tmp = job + ".siox";
		const char* nm= tmp.c_str();
		add_sial_program(nm);
		int segs[]  = {4,5};
		set_aoindex_info(2,segs);
		finalize_setup();
	}

	//read and print setup_file
		setup::BinaryInputFile setup_file(job + ".dat");
	setup::SetupReader setup_reader(setup_file);
	std::cout << "SETUP READER DATA:\n" << setup_reader<< std::endl;


	//get siox name from setup, load and print the sip tables
	std::string prog_name = setup_reader.sial_prog_list_.at(0);
	std::string siox_dir(dir_name);
	setup::BinaryInputFile siox_file(siox_dir + prog_name);
	// sip::SioxReader siox_reader(&sipTables, &siox_file, &setup_reader);
	//	siox_reader.read();
	sip::SipTables sipTables(setup_reader, siox_file);
	std::cout << "SIP TABLES" << '\n' << sipTables << std::endl;

	//interpret the program
	{
	sip::SialxTimer sialxTimer(sipTables.max_timer_slots());
	sip::PersistentArrayManager<sip::Block,sip::Interpreter>* pbm;
	sip::Interpreter runner(sipTables, sialxTimer);
	std::cout << "SIAL PROGRAM OUTPUT" << std::endl;
	runner.interpret();
	ASSERT_EQ(0, sip::DataManager::scope_count);
	std::cout << "\nSIAL PROGRAM TERMINATED"<< std::endl;
	//	runner.print_block()
	;
	}
}


TEST(Sial,contraction_small_test){
	std::cout << "****************************************\n";
	sip::DataManager::scope_count=0;
	//create setup_file
	std::string job("contraction_small_test");
	std::cout << "JOBNAME = " << job << std::endl;
	int aoindex_array[] = {15,15,15,15,15,15,15,15,14,14,14,14,12,12};

	{	init_setup(job.c_str());
		set_aoindex_info(14, aoindex_array);
		std::string tmp = job + ".siox";
		const char* nm= tmp.c_str();
		add_sial_program(nm);
		finalize_setup();
	}

	//read and print setup_file
		setup::BinaryInputFile setup_file(job + ".dat");
	setup::SetupReader setup_reader(setup_file);
	std::cout << "SETUP READER DATA:\n" << setup_reader<< std::endl;


	//get siox name from setup, load and print the sip tables
	std::string prog_name = setup_reader.sial_prog_list_.at(0);
	std::string siox_dir(dir_name);
	setup::BinaryInputFile siox_file(siox_dir + prog_name);
	// sip::SioxReader siox_reader(&sipTables, &siox_file, &setup_reader);
	//	siox_reader.read();
	sip::SipTables sipTables(setup_reader, siox_file);
	std::cout << "SIP TABLES" << '\n' << sipTables << std::endl;

	//interpret the program
	{
	sip::SialxTimer sialxTimer(sipTables.max_timer_slots());
	sip::PersistentArrayManager<sip::Block,sip::Interpreter>* pbm;
	sip::Interpreter runner(sipTables, sialxTimer);
	std::cout << "SIAL PROGRAM OUTPUT" << std::endl;
	runner.interpret();
	ASSERT_EQ(0, sip::DataManager::scope_count);

	// Get the data for local array block "c"
	int c_slot = sip::Interpreter::global_interpreter->array_slot(
			std::string("c"));
	sip::index_selector_t c_indices;
	c_indices[0] = 1;
	c_indices[1] = 1;
	for (int i = 2; i < MAX_RANK; i++)
		c_indices[i] = sip::unused_index_value;
	sip::BlockId c_bid(c_slot, c_indices);
	std::cout << c_bid << std::endl;
	sip::Block::BlockPtr c_bptr =
			sip::Interpreter::global_interpreter->get_block_for_reading(
					c_bid);
	sip::Block::dataPtr c_data = c_bptr->get_data();

	// Compare it with the reference
	const int I = 15;
	const int J = 15;
	const int K = 15;
	const int L = 15;

	double a[I][J][K][L];
	double b[J][K];
	double c[I][L];

	int cntr = 0.0;
	for (int i=0; i<I; i++)
		for (int j=0; j<J; j++)
			for (int k=0; k<K; k++)
				for (int l=0; l<L; l++)
					// behavior of super instruction fill_block_cyclic a(i,j,k,l) 1.0
					a[i][j][k][l] = (cntr++ % 20) + 1;

	for (int i=0; i<I; i++)
		for (int l=0; l<L; l++)
			c[i][l] = 0;

	cntr = 0.0;
	for (int j = 0; j < J; j++)
		for (int k = 0; k < K; k++)
			// behavior of super instruction fill_block_cyclic b(j, k) 1.0
			b[j][k] = (cntr++ % 20) + 1;

	for (int i=0; i<I; i++)
		for (int j=0; j<J; j++)
			for (int k=0; k<K; k++)
				for (int l=0; l<L; l++)
					// c[i][l] needs to be in column major order
					c[l][i] += a[i][j][k][l] * b[j][k];

	for (int i = 0; i < I; i++) {
		for (int l = 0; l < L; l++) {
			ASSERT_DOUBLE_EQ(c[l][i], c_data[i * L + l]);
		}
		std::cout << std::endl;
	}

	std::cout << "\nSIAL PROGRAM TERMINATED"<< std::endl;
	}
}



TEST(Sial,contraction_small_test2){
	std::cout << "****************************************\n";
	sip::DataManager::scope_count=0;
	//create setup_file
	std::string job("contraction_small_test2");
	std::cout << "JOBNAME = " << job << std::endl;

	{	init_setup(job.c_str());

		//int aosegs[]  = {2};
		int aosegs[]  = {9};
		set_aoindex_info(1,aosegs);
		int virtoccsegs[] ={5, 4};
		//int virtoccsegs[] = {2, 2};
		set_moaindex_info(2, virtoccsegs);
		set_constant("baocc", 1);
		set_constant("eaocc", 1);
		set_constant("bavirt", 2);
		set_constant("eavirt", 2);

		std::string tmp = job + ".siox";
		const char* nm= tmp.c_str();
		add_sial_program(nm);
		finalize_setup();
	}

	//read and print setup_file
		setup::BinaryInputFile setup_file(job + ".dat");
	setup::SetupReader setup_reader(setup_file);
	std::cout << "SETUP READER DATA:\n" << setup_reader<< std::endl;


	//get siox name from setup, load and print the sip tables
	std::string prog_name = setup_reader.sial_prog_list_.at(0);
	std::string siox_dir(dir_name);
	setup::BinaryInputFile siox_file(siox_dir + prog_name);
	// sip::SioxReader siox_reader(&sipTables, &siox_file, &setup_reader);
	//	siox_reader.read();
	sip::SipTables sipTables(setup_reader, siox_file);
	std::cout << "SIP TABLES" << '\n' << sipTables << std::endl;

	//interpret the program
	{
	sip::SialxTimer sialxTimer(sipTables.max_timer_slots());
	sip::PersistentArrayManager<sip::Block,sip::Interpreter>* pbm;
	sip::Interpreter runner(sipTables, sialxTimer);
	std::cout << "SIAL PROGRAM OUTPUT" << std::endl;
	runner.interpret();
	ASSERT_EQ(0, sip::DataManager::scope_count);

	// Get the data for local array block "c"
	int c_slot = sip::Interpreter::global_interpreter->array_slot(
			std::string("c"));
	sip::index_selector_t c_indices;
	c_indices[0] = 1;
	c_indices[1] = 1;
	c_indices[2] = 2;
	c_indices[3] = 1;
	for (int i = 4; i < MAX_RANK; i++)
		c_indices[i] = sip::unused_index_value;
	sip::BlockId c_bid(c_slot, c_indices);
	std::cout << c_bid << std::endl;
	sip::Block::BlockPtr c_bptr =
			sip::Interpreter::global_interpreter->get_block_for_reading(
					c_bid);
	sip::Block::dataPtr c_data = c_bptr->get_data();

	// Compare it with the reference
	//    const int MU = 9;
	//    const int LA = 9;
	//    const int I = 5;
	//    const int I1 = 5;
	//    const int A1 = 4;

	/*
	const int MU = 2;
	const int LA = 2;
	const int I = 1;
	const int I1 = 1;
	const int A1 = 1;

	double a[MU][I1][I][LA];
	double b[LA][A1];
	double c[MU][I1][A1][I];

	int line = 0;
	int cntr = 0.0;

	std::cout<<"Ref a"<<std::endl;
	for (int mu=0; mu<MU; mu++)
		for (int i1=0; i1<I1; i1++)
			for(int i=0; i<I; i++)
				for (int la=0; la<LA; la++){
					a[mu][i1][i][la] = (cntr++ % 20) + 1;
					//a[mu][i1][la][i] = 2.0;
					std::cout<<a[mu][i1][i][la]<<"\t";
					if (++line % 9 == 0)
						std::cout<<std::endl;
				}
	std::cout<<std::endl;

	for (int mu=0; mu<MU; mu++)
		for (int i1=0; i1<I1; i1++)
			for (int a1=0; a1<A1; a1++)
				for(int i=0; i<I; i++)
					c[mu][i1][a1][i] = 0;

	line = 0;
	cntr = 0.0;
	std::cout<<"Ref b"<<std::endl;
	for (int la=0; la<LA; la++)
		for (int a1=0; a1<A1; a1++){
			b[la][a1] = (cntr++ % 20) + 1;
			//b[la][a1] = 2.0;
			std::cout<<b[la][a1]<<"\t";
			if (++line % 9 == 0)
				std::cout<<std::endl;
		}
	std::cout<<std::endl;

	int flag = 0;
	 for (int mu=0; mu<MU; mu++){
		for (int i1=0; i1<I1; i1++){
			for(int i=0; i<I; i++){
				for (int a1=0; a1<A1; a1++){
					for (int la=0; la<LA; la++){
						if (!flag){
							std::cout<<a[mu][i1][i][la]<<"*"<<b[la][a1]<<std::endl;
						}
						c[mu][i1][a1][i] += a[mu][i1][i][la] * b[la][a1] ;
					}
					flag = 1;
				}
			}
		}
	}


	 line = 0;
	 std::cout<<"Ref c"<<std::endl;
	 for (int mu=0; mu<MU; mu++)
		for (int i1=0; i1<I1; i1++)
			for (int a1=0; a1<A1; a1++)
				for(int i=0; i<I; i++){
					std::cout<<c[mu][i1][a1][i]<<"\t";
					if (++line % 9 == 0)
						std::cout<<std::endl;
				}
	 std::cout<<std::endl;


	 for (int mu=0; mu<MU; mu++)
		for (int i1=0; i1<I1; i1++)
			for (int a1=0; a1<A1; a1++)
				for(int i=0; i<I; i++)
					//ASSERT_DOUBLE_EQ(c[mu][i1][a1][i], c_data[mu + i1*MU + a1*I1*MU + i*A1*I1*MU]);
					ASSERT_DOUBLE_EQ(c[mu][i1][a1][i], c_data[mu*I1*A1*I + i1*A1*I + a1*I + i]);
	*/
	int matched = test_contraction_small2(c_data);
	ASSERT_TRUE(matched);

	std::cout << "\nSIAL PROGRAM TERMINATED"<< std::endl;
	}
}


TEST(Sial,sum_op){
	std::cout << "****************************************\n";
	sip::DataManager::scope_count=0;
	//create setup_file
	std::string job("sum_op_test");
	std::cout << "JOBNAME = " << job << std::endl;
	double x = 3.456;
	int norb = 2;

	{	init_setup(job.c_str());
		set_scalar("x",x);
		set_constant("norb",norb);
		std::string tmp = job + ".siox";
		const char* nm= tmp.c_str();
		add_sial_program(nm);
		int segs[]  = {20,5};
		set_aoindex_info(2,segs);
		finalize_setup();
	}

	//read and print setup_file
		setup::BinaryInputFile setup_file(job + ".dat");
	setup::SetupReader setup_reader(setup_file);
	std::cout << "SETUP READER DATA:\n" << setup_reader<< std::endl;


	//get siox name from setup, load and print the sip tables
	std::string prog_name = setup_reader.sial_prog_list_.at(0);
	std::string siox_dir(dir_name);
	setup::BinaryInputFile siox_file(siox_dir + prog_name);
	//	// sip::SioxReader siox_reader(&sipTables, &siox_file, &setup_reader);
	//	siox_reader.read();
	sip::SipTables sipTables(setup_reader, siox_file);
	std::cout << "SIP TABLES" << '\n' << sipTables << std::endl;

	//interpret the program
	{
	sip::SialxTimer sialxTimer(sipTables.max_timer_slots());
	sip::PersistentArrayManager<sip::Block,sip::Interpreter>* pbm;
	sip::Interpreter runner(sipTables, sialxTimer);
	std::cout << "SIAL PROGRAM OUTPUT" << std::endl;
	runner.interpret();
	ASSERT_EQ(0, sip::DataManager::scope_count);
	std::cout << "\nSIAL PROGRAM TERMINATED"<< std::endl;
	//	runner.print_block();

	// Reference Calculation
	const int I = 20;
	const int J = 20;

	double a[I][J];
	double c[I][J];
	double d[I][J];
	double e[I][J];

	int cntr = 100.0;
	for (int i=0; i<I; i++)
		for (int j=0; j<J; j++)
		// behavior of super instruction fill_block_sequential a(i,j) 100.0
			a[i][j] = cntr++;
	cntr = 50.0;
	for (int i=0; i<I; i++)
		for (int j=0; j<J; j++)
		// behavior of super instruction fill_block_sequential c(i,j) 50.0
			c[i][j] = cntr++;

	for (int i=0; i<I; i++)
		for (int j=0; j<J; j++)
			d[i][j] = a[i][j] + c[i][j];

	for (int i=0; i<I; i++)
		for (int j=0; j<J; j++)
			e[i][j] = d[i][j] - c[i][j];

	std::cout<<"Reference d"<<std::endl;
	for (int i=0; i<I; i++){
			for (int j=0; j<J; j++){
				std::cout<<d[i][j]<<" ";
			}
		std::cout<<std::endl;
	}

	std::cout<<"Reference e"<<std::endl;
		for (int i=0; i<I; i++){
				for (int j=0; j<J; j++){
					std::cout<<e[i][j]<<" ";
				}
			std::cout<<std::endl;
		}

	// Get the data for local array block "c"
	int d_slot = sip::Interpreter::global_interpreter->array_slot(std::string("d"));
	sip::index_selector_t d_indices;
	d_indices[0] = 1; d_indices[1] = 1;
	for (int i = 2; i < MAX_RANK; i++) d_indices[i] = sip::unused_index_value;
	sip::BlockId d_bid(d_slot, d_indices);
	std::cout << d_bid << std::endl;
	sip::Block::BlockPtr d_bptr = sip::Interpreter::global_interpreter->get_block_for_reading(d_bid);
	sip::Block::dataPtr d_data = d_bptr->get_data();

	int e_slot = sip::Interpreter::global_interpreter->array_slot(std::string("e"));
	sip::index_selector_t e_indices;
	e_indices[0] = 1; e_indices[1] = 1;
	for (int i = 2; i < MAX_RANK; i++) e_indices[i] = sip::unused_index_value;
	sip::BlockId e_bid(e_slot, e_indices);
	std::cout << e_bid << std::endl;
	sip::Block::BlockPtr e_bptr = sip::Interpreter::global_interpreter->get_block_for_reading(e_bid);
	sip::Block::dataPtr e_data = e_bptr->get_data();

	for (int i=0; i<I; i++)
		for (int j=0; j<J; j++)
			ASSERT_DOUBLE_EQ(d_data[i*J+j], d[i][j]);	// d_data is in column major

	for (int i=0; i<I; i++)
		for (int j=0; j<J; j++)
			ASSERT_DOUBLE_EQ(e_data[i*J+j], e[i][j]);	// e_data is in column major

	}
}

TEST(Sial,print_block_test){

	std::cout << "****************************************\n";
	sip::DataManager::scope_count=0;
	//create setup_file
	std::string job("print_block_test");
	std::cout << "JOBNAME = " << job << std::endl;
	double x = 3.456;
	int norb = 2;

	{	init_setup(job.c_str());
		set_scalar("x",x);
		set_constant("norb",norb);
		std::string tmp = job + ".siox";
		const char* nm= tmp.c_str();
		add_sial_program(nm);
		int segs[]  = {2,3};
		set_aoindex_info(2,segs);
		finalize_setup();
	}

	//read and print setup_file
		setup::BinaryInputFile setup_file(job + ".dat");
	setup::SetupReader setup_reader(setup_file);
	std::cout << "SETUP READER DATA:\n" << setup_reader<< std::endl;


	//get siox name from setup, load and print the sip tables
	std::string prog_name = setup_reader.sial_prog_list_.at(0);
	std::string siox_dir(dir_name);
	setup::BinaryInputFile siox_file(siox_dir + prog_name);
	// sip::SioxReader siox_reader(&sipTables, &siox_file, &setup_reader);
	//	siox_reader.read();
	sip::SipTables sipTables(setup_reader, siox_file);
	std::cout << "SIP TABLES" << '\n' << sipTables << std::endl;

	//interpret the program
	{
	sip::SialxTimer sialxTimer(sipTables.max_timer_slots());
	sip::PersistentArrayManager<sip::Block,sip::Interpreter>* pbm;
	sip::Interpreter runner(sipTables, sialxTimer);
	std::cout << "SIAL PROGRAM OUTPUT" << std::endl;
	runner.interpret();
	ASSERT_EQ(0, sip::DataManager::scope_count);
	std::cout << "\nSIAL PROGRAM TERMINATED"<< std::endl;
	//	runner.print_block()
	;
	}
}

TEST(Sial,subindex_test){
	std::cout << "****************************************\n";
	sip::DataManager::scope_count=0;
	//create setup_file
	std::string job("subindex_test");
	std::cout << "JOBNAME = " << job << std::endl;
	double x = 3.456;
	int norb = 2;

	{	init_setup(job.c_str());
		set_scalar("x",x);
		set_constant("norb",norb);
		std::string tmp = job + ".siox";
		const char* nm= tmp.c_str();
		add_sial_program(nm);
		int segs[]  = {8,16};
		set_aoindex_info(2,segs);
		finalize_setup();
	}

	//read and print setup_file
		setup::BinaryInputFile setup_file(job + ".dat");
	setup::SetupReader setup_reader(setup_file);
	std::cout << "SETUP READER DATA:\n" << setup_reader<< std::endl;


	//get siox name from setup, load and print the sip tables
	std::string prog_name = setup_reader.sial_prog_list_.at(0);
	std::string siox_dir(dir_name);
	setup::BinaryInputFile siox_file(siox_dir + prog_name);
	// sip::SioxReader siox_reader(&sipTables, &siox_file, &setup_reader);
	//	siox_reader.read();
	sip::SipTables sipTables(setup_reader, siox_file);
	std::cout << "SIP TABLES" << '\n' << sipTables << std::endl;

	//interpret the program
	{
	sip::SialxTimer sialxTimer(sipTables.max_timer_slots());
	sip::PersistentArrayManager<sip::Block,sip::Interpreter>* pbm;
	sip::Interpreter runner(sipTables, sialxTimer);
	std::cout << "SIAL PROGRAM OUTPUT" << std::endl;
	runner.interpret();
	ASSERT_EQ(0, sip::DataManager::scope_count);
	std::cout << "\nSIAL PROGRAM TERMINATED"<< std::endl;
	//	runner.print_block()
	;
	}
}



TEST(Sial,insert_slice_test){
	std::cout << "****************************************\n";
	sip::DataManager::scope_count=0;
	//create setup_file
	std::string job("insert_slice_test");
	std::cout << "JOBNAME = " << job << std::endl;
	double x = 3.456;
	int norb = 2;

	{	init_setup(job.c_str());
		set_scalar("x",x);
		set_constant("norb",norb);
		std::string tmp = job + ".siox";
		const char* nm= tmp.c_str();
		add_sial_program(nm);
		int segs[]  = {8,16};
		set_aoindex_info(2,segs);
		finalize_setup();
	}

	//read and print setup_file
	//// setup::SetupReader setup_reader;

	setup::BinaryInputFile setup_file(job + ".dat");
	// setup_reader.read(setup_file);
	setup::SetupReader setup_reader(setup_file);

	std::cout << "SETUP READER DATA:\n" << setup_reader<< std::endl;


	//get siox name from setup, load and print the sip tables
	std::string prog_name = setup_reader.sial_prog_list_.at(0);
	std::string siox_dir(dir_name);
	setup::BinaryInputFile siox_file(siox_dir + prog_name);
	// sip::SioxReader siox_reader(&sipTables, &siox_file, &setup_reader);
//	siox_reader.read();
	sip::SipTables sipTables(setup_reader, siox_file);
	std::cout << "SIP TABLES" << '\n' << sipTables << std::endl;

	//interpret the program
	{
	sip::SialxTimer sialxTimer(sipTables.max_timer_slots());
	sip::PersistentArrayManager<sip::Block,sip::Interpreter>* pbm;
	sip::Interpreter runner(sipTables, sialxTimer);
	std::cout << "SIAL PROGRAM OUTPUT" << std::endl;
	runner.interpret();
	ASSERT_EQ(0, sip::DataManager::scope_count);
	std::cout << "\nSIAL PROGRAM TERMINATED"<< std::endl;
//	runner.print_block()
;
	}
}

TEST(Sial,static_array_test){
	std::cout << "****************************************\n";
	sip::DataManager::scope_count=0;
	//create setup_file
	std::string job("static_array_test");
	std::cout << "JOBNAME = " << job << std::endl;
	double x = 3.456;
	int norb = 2;

	{	init_setup(job.c_str());
		set_scalar("x",x);
		set_constant("norb",norb);
		std::string tmp = job + ".siox";
		const char* nm= tmp.c_str();
		add_sial_program(nm);
		int segs[]  = {8,16};
		set_aoindex_info(2,segs);
		finalize_setup();
	}

	//read and print setup_file
	//// setup::SetupReader setup_reader;

	setup::BinaryInputFile setup_file(job + ".dat");
	// setup_reader.read(setup_file);
	setup::SetupReader setup_reader(setup_file);

	std::cout << "SETUP READER DATA:\n" << setup_reader<< std::endl;


	//get siox name from setup, load and print the sip tables
	std::string prog_name = setup_reader.sial_prog_list_.at(0);
	std::string siox_dir(dir_name);
	setup::BinaryInputFile siox_file(siox_dir + prog_name);
	// sip::SioxReader siox_reader(&sipTables, &siox_file, &setup_reader);
//	siox_reader.read();
	sip::SipTables sipTables(setup_reader, siox_file);
	std::cout << "SIP TABLES" << '\n' << sipTables << std::endl;


	//interpret the program
	{
	sip::SialxTimer sialxTimer(sipTables.max_timer_slots());

	sip::Interpreter runner(sipTables, sialxTimer);
	std::cout << "SIAL PROGRAM OUTPUT" << std::endl;
	runner.interpret();
	ASSERT_EQ(0, sip::DataManager::scope_count);
	std::cout << "\nSIAL PROGRAM TERMINATED"<< std::endl;
//	runner.print_block()
;
	}
}




TEST(Sial,local_arrays){
	std::cout << "****************************************\n";
	sip::DataManager::scope_count=0;
	//create setup_file
	std::string job("local_arrays");
	std::cout << "JOBNAME = " << job << std::endl;
	double x = 3.456;
	int norb = 2;

	{	init_setup(job.c_str());
		set_scalar("x",x);
		set_constant("norb",norb);
		std::string tmp = job + ".siox";
		const char* nm= tmp.c_str();
		add_sial_program(nm);
		int segs[]  = {2,3};
		set_aoindex_info(2,segs);
		finalize_setup();
	}

	//read and print setup_file
	//// setup::SetupReader setup_reader;

	setup::BinaryInputFile setup_file(job + ".dat");
	// setup_reader.read(setup_file);
	setup::SetupReader setup_reader(setup_file);

	std::cout << "SETUP READER DATA:\n" << setup_reader<< std::endl;


	//get siox name from setup, load and print the sip tables
	std::string prog_name = setup_reader.sial_prog_list_.at(0);
	std::string siox_dir(dir_name);
	setup::BinaryInputFile siox_file(siox_dir + prog_name);
	// sip::SioxReader siox_reader(&sipTables, &siox_file, &setup_reader);
//	siox_reader.read();
	sip::SipTables sipTables(setup_reader, siox_file);
	std::cout << "SIP TABLES" << '\n' << sipTables << std::endl;

	//interpret the program
	{
	sip::SialxTimer sialxTimer(sipTables.max_timer_slots());
	sip::PersistentArrayManager<sip::Block,sip::Interpreter>* pbm;
	sip::Interpreter runner(sipTables, sialxTimer);
	std::cout << "SIAL PROGRAM OUTPUT" << std::endl;
	runner.interpret();
	ASSERT_EQ(0, sip::DataManager::scope_count);
	std::cout << "\nSIAL PROGRAM TERMINATED"<< std::endl;
//	runner.print_block()
;
	}
}

TEST(Sial,local_arrays_wild){
	std::cout << "****************************************\n";
	sip::DataManager::scope_count=0;
	//create setup_file
	std::string job("local_arrays_wild");
	std::cout << "JOBNAME = " << job << std::endl;
	double x = 3.456;
	int norb = 2;

	{	init_setup(job.c_str());
		set_scalar("x",x);
		set_constant("norb",norb);
		std::string tmp = job + ".siox";
		const char* nm= tmp.c_str();
		add_sial_program(nm);
		int segs[]  = {2,3};
		set_aoindex_info(2,segs);
		finalize_setup();
	}

	//read and print setup_file
	//// setup::SetupReader setup_reader;

	setup::BinaryInputFile setup_file(job + ".dat");
	// setup_reader.read(setup_file);
	setup::SetupReader setup_reader(setup_file);

	std::cout << "SETUP READER DATA:\n" << setup_reader<< std::endl;


	//get siox name from setup, load and print the sip tables
	std::string prog_name = setup_reader.sial_prog_list_.at(0);
	std::string siox_dir(dir_name);
	setup::BinaryInputFile siox_file(siox_dir + prog_name);
	// sip::SioxReader siox_reader(&sipTables, &siox_file, &setup_reader);
//	siox_reader.read();
	sip::SipTables sipTables(setup_reader, siox_file);
	std::cout << "SIP TABLES" << '\n' << sipTables << std::endl;

	//interpret the program
	{
	sip::SialxTimer sialxTimer(sipTables.max_timer_slots());
	sip::PersistentArrayManager<sip::Block,sip::Interpreter>* pbm;
	sip::Interpreter runner(sipTables, sialxTimer);
	std::cout << "SIAL PROGRAM OUTPUT" << std::endl;
	runner.interpret();
	ASSERT_EQ(0, sip::DataManager::scope_count);
	std::cout << "\nSIAL PROGRAM TERMINATED"<< std::endl;
//	runner.print_block()
;
	}
}

TEST(Sial,put_test){
	std::cout << "****************************************\n";
	sip::DataManager::scope_count=0;
	//create setup_file
	std::string job("put_test");
	std::cout << "JOBNAME = " << job << std::endl;
	double x = 3.456;
	int norb = 2;

	{	init_setup(job.c_str());
		set_scalar("x",x);
		set_constant("norb",norb);
		std::string tmp = job + ".siox";
		const char* nm= tmp.c_str();
		add_sial_program(nm);
		int segs[]  = {2,3};
		set_aoindex_info(2,segs);
		finalize_setup();
	}

	//read and print setup_file
	//// setup::SetupReader setup_reader;

	setup::BinaryInputFile setup_file(job + ".dat");
	// setup_reader.read(setup_file);
	setup::SetupReader setup_reader(setup_file);

	std::cout << "SETUP READER DATA:\n" << setup_reader<< std::endl;


	//get siox name from setup, load and print the sip tables
	std::string prog_name = setup_reader.sial_prog_list_.at(0);
	std::string siox_dir(dir_name);
	setup::BinaryInputFile siox_file(siox_dir + prog_name);
	// sip::SioxReader siox_reader(&sipTables, &siox_file, &setup_reader);
//	siox_reader.read();
	sip::SipTables sipTables(setup_reader, siox_file);
	std::cout << "SIP TABLES" << '\n' << sipTables << std::endl;

	//interpret the program
	{
	sip::SialxTimer sialxTimer(sipTables.max_timer_slots());
	sip::PersistentArrayManager<sip::Block,sip::Interpreter>* pbm;
	sip::Interpreter runner(sipTables, sialxTimer);
	std::cout << "SIAL PROGRAM OUTPUT" << std::endl;
	runner.interpret();
	ASSERT_EQ(0, sip::DataManager::scope_count);
	std::cout << "\nSIAL PROGRAM TERMINATED"<< std::endl;
//	runner.print_block()
;
	}
}


TEST(Sial,gpu_contraction_small){
	std::cout << "****************************************\n";
	sip::DataManager::scope_count=0;
	//create setup_file
	std::string job("gpu_contraction_small_test");
	std::cout << "JOBNAME = " << job << std::endl;
	int aoindex_array[] = {15,15,15,15,15,15,15,15,14,14,14,14,12,12};

	{	init_setup(job.c_str());
		set_aoindex_info(14, aoindex_array);
		std::string tmp = job + ".siox";
		const char* nm= tmp.c_str();
		add_sial_program(nm);
		finalize_setup();
	}

	//read and print setup_file
	//// setup::SetupReader setup_reader;

	setup::BinaryInputFile setup_file(job + ".dat");
	// setup_reader.read(setup_file);
	setup::SetupReader setup_reader(setup_file);

	std::cout << "SETUP READER DATA:\n" << setup_reader<< std::endl;


	//get siox name from setup, load and print the sip tables
	std::string prog_name = setup_reader.sial_prog_list_.at(0);
	std::string siox_dir(dir_name);
	setup::BinaryInputFile siox_file(siox_dir + prog_name);
	// sip::SioxReader siox_reader(&sipTables, &siox_file, &setup_reader);
//	siox_reader.read();
	sip::SipTables sipTables(setup_reader, siox_file);
	std::cout << "SIP TABLES" << '\n' << sipTables << std::endl;

	//interpret the program
	{
	sip::SialxTimer sialxTimer(sipTables.max_timer_slots());
	sip::PersistentArrayManager<sip::Block,sip::Interpreter>* pbm;
	sip::Interpreter runner(sipTables, sialxTimer);
	std::cout << "SIAL PROGRAM OUTPUT" << std::endl;
	runner.interpret();
	ASSERT_EQ(0, sip::DataManager::scope_count);

	// Get the data for local array block "c"
	int c_slot = sip::Interpreter::global_interpreter->array_slot(
			std::string("c"));
	sip::index_selector_t c_indices;
	c_indices[0] = 1;
	c_indices[1] = 1;
	for (int i = 2; i < MAX_RANK; i++)
		c_indices[i] = sip::unused_index_value;
	sip::BlockId c_bid(c_slot, c_indices);
	std::cout << c_bid << std::endl;
	sip::Block::BlockPtr c_bptr =
			sip::Interpreter::global_interpreter->get_block_for_reading(
					c_bid);
	sip::Block::dataPtr c_data = c_bptr->get_data();

	// Compare it with the reference
	const int I = 15;
	const int J = 15;
	const int K = 15;
	const int L = 15;

	double a[I][J][K][L];
	double b[J][K];
	double c[I][L];

	int cntr = 0.0;
	for (int i=0; i<I; i++)
		for (int j=0; j<J; j++)
			for (int k=0; k<K; k++)
				for (int l=0; l<L; l++)
					// behavior of super instruction fill_block_cyclic a(i,j,k,l) 1.0
					a[i][j][k][l] = (cntr++ % 20) + 1;

	for (int i=0; i<I; i++)
		for (int l=0; l<L; l++)
			c[i][l] = 0;

	cntr = 0.0;
	for (int j = 0; j < J; j++)
		for (int k = 0; k < K; k++)
			// behavior of super instruction fill_block_cyclic b(j, k) 1.0
			b[j][k] = (cntr++ % 20) + 1;

	for (int i=0; i<I; i++)
		for (int j=0; j<J; j++)
			for (int k=0; k<K; k++)
				for (int l=0; l<L; l++)
					// c[i][l] needs to be in column major order
					c[l][i] += a[i][j][k][l] * b[j][k];

	for (int i = 0; i < I; i++) {
		for (int l = 0; l < L; l++) {
			ASSERT_DOUBLE_EQ(c[l][i], c_data[i * L + l]);
		}
		std::cout << std::endl;
	}

	std::cout << "\nSIAL PROGRAM TERMINATED"<< std::endl;
	}
}

TEST(Sial,gpu_sum_op){
	std::cout << "****************************************\n";
	sip::DataManager::scope_count=0;
	//create setup_file
	std::string job("gpu_sum_op_test");
	std::cout << "JOBNAME = " << job << std::endl;
	double x = 3.456;
	int norb = 2;

	{	init_setup(job.c_str());
		set_scalar("x",x);
		set_constant("norb",norb);
		std::string tmp = job + ".siox";
		const char* nm= tmp.c_str();
		add_sial_program(nm);
		int segs[]  = {20,5};
		set_aoindex_info(2,segs);
		finalize_setup();
	}

	//read and print setup_file
	//// setup::SetupReader setup_reader;

	setup::BinaryInputFile setup_file(job + ".dat");
	// setup_reader.read(setup_file);
	setup::SetupReader setup_reader(setup_file);

	std::cout << "SETUP READER DATA:\n" << setup_reader<< std::endl;


	//get siox name from setup, load and print the sip tables
	std::string prog_name = setup_reader.sial_prog_list_.at(0);
	std::string siox_dir(dir_name);
	setup::BinaryInputFile siox_file(siox_dir + prog_name);
	// sip::SioxReader siox_reader(&sipTables, &siox_file, &setup_reader);
//	siox_reader.read();
	sip::SipTables sipTables(setup_reader, siox_file);
	std::cout << "SIP TABLES" << '\n' << sipTables << std::endl;

	//interpret the program
	{
	sip::SialxTimer sialxTimer(sipTables.max_timer_slots());
	sip::PersistentArrayManager<sip::Block,sip::Interpreter>* pbm;
	sip::Interpreter runner(sipTables, sialxTimer);
	std::cout << "SIAL PROGRAM OUTPUT" << std::endl;
	runner.interpret();
	ASSERT_EQ(0, sip::DataManager::scope_count);
	std::cout << "\nSIAL PROGRAM TERMINATED"<< std::endl;
//	runner.print_block();

	// Reference Calculation
	const int I = 20;
	const int J = 20;

	double a[I][J];
	double c[I][J];
	double d[I][J];
	double e[I][J];

	int cntr = 100.0;
	for (int i=0; i<I; i++)
		for (int j=0; j<J; j++)
		// behavior of super instruction fill_block_sequential a(i,j) 100.0
			a[i][j] = cntr++;
	cntr = 50.0;
	for (int i=0; i<I; i++)
		for (int j=0; j<J; j++)
		// behavior of super instruction fill_block_sequential c(i,j) 50.0
			c[i][j] = cntr++;

	for (int i=0; i<I; i++)
		for (int j=0; j<J; j++)
			d[i][j] = a[i][j] + c[i][j];

	for (int i=0; i<I; i++)
		for (int j=0; j<J; j++)
			e[i][j] = d[i][j] - c[i][j];

	std::cout<<"Reference d"<<std::endl;
	for (int i=0; i<I; i++){
			for (int j=0; j<J; j++){
				std::cout<<d[i][j]<<" ";
			}
		std::cout<<std::endl;
	}

	std::cout<<"Reference e"<<std::endl;
		for (int i=0; i<I; i++){
				for (int j=0; j<J; j++){
					std::cout<<e[i][j]<<" ";
				}
			std::cout<<std::endl;
		}

	// Get the data for local array block "c"
	int d_slot = sip::Interpreter::global_interpreter->array_slot(std::string("d"));
	sip::index_selector_t d_indices;
	d_indices[0] = 1; d_indices[1] = 1;
	for (int i = 2; i < MAX_RANK; i++) d_indices[i] = sip::unused_index_value;
	sip::BlockId d_bid(d_slot, d_indices);
	std::cout << d_bid << std::endl;
	sip::Block::BlockPtr d_bptr = sip::Interpreter::global_interpreter->get_block_for_reading(d_bid);
	sip::Block::dataPtr d_data = d_bptr->get_data();

	int e_slot = sip::Interpreter::global_interpreter->array_slot(std::string("e"));
	sip::index_selector_t e_indices;
	e_indices[0] = 1; e_indices[1] = 1;
	for (int i = 2; i < MAX_RANK; i++) e_indices[i] = sip::unused_index_value;
	sip::BlockId e_bid(e_slot, e_indices);
	std::cout << e_bid << std::endl;
	sip::Block::BlockPtr e_bptr = sip::Interpreter::global_interpreter->get_block_for_reading(e_bid);
	sip::Block::dataPtr e_data = e_bptr->get_data();

	for (int i=0; i<I; i++)
		for (int j=0; j<J; j++)
			ASSERT_DOUBLE_EQ(d_data[i*J+j], d[i][j]);	// d_data is in column major

	for (int i=0; i<I; i++)
		for (int j=0; j<J; j++)
			ASSERT_DOUBLE_EQ(e_data[i*J+j], e[i][j]);	// e_data is in column major

	}
}


TEST(Sial,gpu_ops){
	std::cout << "****************************************\n";
	sip::DataManager::scope_count=0;
	//create setup_file
	std::string job("gpu_ops");
	std::cout << "JOBNAME = " << job << std::endl;
	double x = 3.456;
	int norb = 2;

	{	init_setup(job.c_str());
//			set_scalar("x",x);
//			set_constant("norb",norb);
		std::string tmp = job + ".siox";
		const char* nm= tmp.c_str();
		add_sial_program(nm);
		int segs[]  = {4,4,4,4,4,4,4,4	};
		set_aoindex_info(8,segs);
		finalize_setup();
	}

	//read and print setup_file
	//// setup::SetupReader setup_reader;

	setup::BinaryInputFile setup_file(job + ".dat");
	// setup_reader.read(setup_file);
	setup::SetupReader setup_reader(setup_file);

	std::cout << "SETUP READER DATA:\n" << setup_reader<< std::endl;


	//get siox name from setup, load and print the sip tables
	std::string prog_name = setup_reader.sial_prog_list_.at(0);
	std::string siox_dir(dir_name);
	setup::BinaryInputFile siox_file(siox_dir + prog_name);
	// sip::SioxReader siox_reader(&sipTables, &siox_file, &setup_reader);
//	siox_reader.read();
	sip::SipTables sipTables(setup_reader, siox_file);
	std::cout << "SIP TABLES" << '\n' << sipTables << std::endl;

	//interpret the program
	{
	sip::SialxTimer sialxTimer(sipTables.max_timer_slots());
	sip::PersistentArrayManager<sip::Block,sip::Interpreter>* pbm;
	sip::Interpreter runner(sipTables, sialxTimer);

	std::cout << "SIAL PROGRAM OUTPUT" << std::endl;
	runner.interpret();
	ASSERT_DOUBLE_EQ(0, sip::DataManager::scope_count);
	std::cout << "\nSIAL PROGRAM TERMINATED"<< std::endl;
//	runner.print_block()
;
	}
}


TEST(Sial,contract_to_scalar){
	std::cout << "****************************************\n";
	sip::DataManager::scope_count=0;
	//create setup_file
	std::string job("contract_to_scalar");
	std::cout << "JOBNAME = " << job << std::endl;
	//double x = 3.456;
	int norb = 2;

	{	init_setup(job.c_str());
		//set_scalar("x",x);
		set_constant("norb",norb);
		std::string tmp = job + ".siox";
		const char* nm= tmp.c_str();
		add_sial_program(nm);
		int segs[]  = {8,8};
		set_aoindex_info(2,segs);
		finalize_setup();
	}

	//read and print setup_file
	//// setup::SetupReader setup_reader;

	setup::BinaryInputFile setup_file(job + ".dat");
	// setup_reader.read(setup_file);
	setup::SetupReader setup_reader(setup_file);

	std::cout << "SETUP READER DATA:\n" << setup_reader<< std::endl;


	//get siox name from setup, load and print the sip tables
	std::string prog_name = setup_reader.sial_prog_list_.at(0);
	std::string siox_dir(dir_name);
	setup::BinaryInputFile siox_file(siox_dir + prog_name);
	// sip::SioxReader siox_reader(&sipTables, &siox_file, &setup_reader);
//	siox_reader.read();
	sip::SipTables sipTables(setup_reader, siox_file);
	std::cout << "SIP TABLES" << '\n' << sipTables << std::endl;

	//interpret the program
	{
	sip::SialxTimer sialxTimer(sipTables.max_timer_slots());
	sip::PersistentArrayManager<sip::Block,sip::Interpreter>* pbm;
	sip::Interpreter runner(sipTables, sialxTimer);
	std::cout << "SIAL PROGRAM OUTPUT" << std::endl;
	runner.interpret();
	ASSERT_EQ(0, sip::DataManager::scope_count);
	std::cout << "\nSIAL PROGRAM TERMINATED"<< std::endl;
	double actual_x = runner.data_manager_.scalar_value("x");
//	runner.print_block();

	// Compare it with the reference
	const int I = 8;
	const int J = 8;

	double a[I][J];
	double b[I][J];
	double ref_x;

	int cntr = 0.0;
	for (int i=0; i<I; i++)
		for (int j=0; j<J; j++)
		 // behavior of super instruction fill_block_cyclic a(i,j) 1.0
			a[i][j] = (cntr++ % 20) + 1;

	cntr = 4.0;
	for (int i=0; i<I; i++)
		for (int j=0; j<J; j++)
			// behavior of super instruction fill_block_cyclic b(i, j) 5.0
			b[i][j] = (cntr++ % 20) + 1;
	ref_x = 0;
	for (int i=0; i<I; i++)
		for (int j=0; j<J; j++)
			ref_x += a[i][j] * b[i][j];

	ASSERT_DOUBLE_EQ(ref_x, actual_x);
	std::cout << std::endl;
	}
}

TEST(Sial,gpu_contract_to_scalar){
	std::cout << "****************************************\n";
	sip::DataManager::scope_count=0;
	//create setup_file
	std::string job("gpu_contract_to_scalar");
	std::cout << "JOBNAME = " << job << std::endl;
	//double x = 3.456;
	int norb = 2;

	{	init_setup(job.c_str());
		//set_scalar("x",x);
		set_constant("norb",norb);
		std::string tmp = job + ".siox";
		const char* nm= tmp.c_str();
		add_sial_program(nm);
		int segs[]  = {8,8};
		set_aoindex_info(2,segs);
		finalize_setup();
	}

	//read and print setup_file
	//// setup::SetupReader setup_reader;

	setup::BinaryInputFile setup_file(job + ".dat");
	// setup_reader.read(setup_file);
	setup::SetupReader setup_reader(setup_file);

	std::cout << "SETUP READER DATA:\n" << setup_reader<< std::endl;


	//get siox name from setup, load and print the sip tables
	std::string prog_name = setup_reader.sial_prog_list_.at(0);
	std::string siox_dir(dir_name);
	setup::BinaryInputFile siox_file(siox_dir + prog_name);
	// sip::SioxReader siox_reader(&sipTables, &siox_file, &setup_reader);
//	siox_reader.read();
	sip::SipTables sipTables(setup_reader, siox_file);
	std::cout << "SIP TABLES" << '\n' << sipTables << std::endl;

	//interpret the program
	{
	sip::SialxTimer sialxTimer(sipTables.max_timer_slots());
	sip::PersistentArrayManager<sip::Block,sip::Interpreter>* pbm;
	sip::Interpreter runner(sipTables, sialxTimer);
	std::cout << "SIAL PROGRAM OUTPUT" << std::endl;
	runner.interpret();
	ASSERT_EQ(0, sip::DataManager::scope_count);
	std::cout << "\nSIAL PROGRAM TERMINATED"<< std::endl;
	double actual_c = runner.data_manager_.scalar_value("x");
//	runner.print_block();

	// Compare it with the reference
	const int I = 8;
	const int J = 8;

	double a[I][J];
	double b[I][J];
	double ref_c;

	int cntr = 0.0;
	for (int i=0; i<I; i++)
		for (int j=0; j<J; j++)
		 // behavior of super instruction fill_block_cyclic a(i,j) 1.0
			a[i][j] = (cntr++ % 20) + 1;

	cntr = 4.0;
	for (int i=0; i<I; i++)
		for (int j=0; j<J; j++)
			// behavior of super instruction fill_block_cyclic b(i, j) 5.0
			b[i][j] = (cntr++ % 20) + 1;
	ref_c = 0;
	for (int i=0; i<I; i++)
		for (int j=0; j<J; j++)
			ref_c += a[i][j] * b[i][j];

	ASSERT_DOUBLE_EQ(ref_c, actual_c);
	std::cout << std::endl;
	}
}

TEST(Sial,gpu_transpose_tmp){
	std::cout << "****************************************\n";
	sip::DataManager::scope_count=0;
	//create setup_file
	std::string job("gpu_transpose_tmp");
	std::cout << "JOBNAME = " << job << std::endl;
	double x = 3.456;
	double y = -0.1;
	int norb = 3;

	{	init_setup(job.c_str());
		set_scalar("x",x);
		set_scalar("y",y);
		set_constant("norb",norb);
		std::string tmp = job + ".siox";
		const char* nm= tmp.c_str();
		add_sial_program(nm);
		int segs[]  = {8,12,10};
		set_aoindex_info(3,segs);
		finalize_setup();
	}

	//read and print setup_file
	//// setup::SetupReader setup_reader;

	setup::BinaryInputFile setup_file(job + ".dat");
	// setup_reader.read(setup_file);
	setup::SetupReader setup_reader(setup_file);

	std::cout << "SETUP READER DATA:\n" << setup_reader<< std::endl;


	//get siox name from setup, load and print the sip tables
	std::string prog_name = setup_reader.sial_prog_list_.at(0);
	std::string siox_dir(dir_name);
	setup::BinaryInputFile siox_file(siox_dir + prog_name);
	// sip::SioxReader siox_reader(&sipTables, &siox_file, &setup_reader);
//	siox_reader.read();
	sip::SipTables sipTables(setup_reader, siox_file);
	std::cout << "SIP TABLES" << '\n' << sipTables << std::endl;

	//interpret the program
	{
	sip::SialxTimer sialxTimer(sipTables.max_timer_slots());
	sip::PersistentArrayManager<sip::Block,sip::Interpreter>* pbm;
	sip::Interpreter runner(sipTables, sialxTimer);
	std::cout << "SIAL PROGRAM OUTPUT" << std::endl;
	runner.interpret();
	ASSERT_EQ(0, sip::DataManager::scope_count);
	std::cout << "\nSIAL PROGRAM TERMINATED"<< std::endl;

	// Get the data for local array block "b"
	int b_slot = sip::Interpreter::global_interpreter->array_slot(std::string("b"));
	sip::index_selector_t b_indices;
	b_indices[0] = 1; b_indices[1] = 1;
	for (int i = 2; i < MAX_RANK; i++) b_indices[i] = sip::unused_index_value;
	sip::BlockId b_bid(b_slot, b_indices);
	std::cout << b_bid << std::endl;
	sip::Block::BlockPtr b_bptr = sip::Interpreter::global_interpreter->get_block_for_reading(b_bid);
	sip::Block::dataPtr b_data = b_bptr->get_data();


//		const int I = 8;
//		const int J = 8;
//		const int K = 8;
//		double b_ref[J][K][I];
//		double a[I][J][K];
//
//		int cntr = 53;
//		for (int i=0; i<I; i++){
//			for (int j=0; j<J; j++){
//				for (int k=0; k<K; k++){
//					a[i][j][k] = cntr++;
//				}
//			}
//		}
//
//		for (int i = 0; i < I; i++) {
//			for (int j = 0; j < J; j++) {
//				for (int k = 0; k < K; k++) {
//					b_ref[j][k][i] = a[i][j][k];
//				}
//			}
//		}
//
//        // Printing out reference
//        std::cout<<"Reference b"<<std::endl;
//		for (int i = 0; i < I; i++) {
//			for (int j = 0; j < J; j++) {
//				for (int k = 0; k < K; k++) {
//					std::cout<<b_ref[i][j][k]<<" ";
//				}
//			}
//            std::cout<<std::endl;
//		}
//
//        // Printing out calculated
//        std::cout<<"Caclulated b"<<std::endl;
//		for (int i = 0; i < I; i++) {
//			for (int j = 0; j < J; j++) {
//				for (int k = 0; k < K; k++) {
//					std::cout<< b_data[i + j*I + k*J*I]<<" ";
//				}
//			}
//            std::cout<<std::endl;
//		}
//
//		for (int i = 0; i < I; i++) {
//			for (int j = 0; j < J; j++) {
//				for (int k = 0; k < K; k++) {
//				    ASSERT_EQ(b_ref[i][j][k], b_data[i + j*I + k*J*I]);
//				}
//			}
//		}

	int passed = test_transpose_op(b_data);
	ASSERT_TRUE(passed);

	}

}

TEST(Sial,simple_indices_assignments){
	std::cout << "****************************************\n";
	sip::DataManager::scope_count=0;
	//create setup_file
	std::string job("simple_indices_assignments");
	std::cout << "JOBNAME = " << job << std::endl;
	double x = 3.456;
	int norb = 2;

	{	init_setup(job.c_str());
		set_scalar("x",x);
		set_constant("norb",norb);
		std::string tmp = job + ".siox";
		const char* nm= tmp.c_str();
		add_sial_program(nm);
		int segs[]  = {8,8};
		set_aoindex_info(2,segs);
		finalize_setup();
	}

	//read and print setup_file
	//// setup::SetupReader setup_reader;

	setup::BinaryInputFile setup_file(job + ".dat");
	// setup_reader.read(setup_file);
	setup::SetupReader setup_reader(setup_file);

	std::cout << "SETUP READER DATA:\n" << setup_reader<< std::endl;


	//get siox name from setup, load and print the sip tables
	std::string prog_name = setup_reader.sial_prog_list_.at(0);
	std::string siox_dir(dir_name);
	setup::BinaryInputFile siox_file(siox_dir + prog_name);
	// sip::SioxReader siox_reader(&sipTables, &siox_file, &setup_reader);
//	siox_reader.read();
	sip::SipTables sipTables(setup_reader, siox_file);
	std::cout << "SIP TABLES" << '\n' << sipTables << std::endl;

	//interpret the program
	{
	sip::SialxTimer sialxTimer(sipTables.max_timer_slots());
	sip::PersistentArrayManager<sip::Block,sip::Interpreter>* pbm;
	sip::Interpreter runner(sipTables, sialxTimer);
	std::cout << "SIAL PROGRAM OUTPUT" << std::endl;
	runner.interpret();
	ASSERT_EQ(0, sip::DataManager::scope_count);
	std::cout << "\nSIAL PROGRAM TERMINATED"<< std::endl;
	}

}



TEST(Sial,self_multiply_op){
	std::cout << "****************************************\n";
	sip::DataManager::scope_count=0;
	//create setup_file
	std::string job("self_multiply_test");
	std::cout << "JOBNAME = " << job << std::endl;
	double x = 3.456;
	int norb = 2;

	{	init_setup(job.c_str());
		set_scalar("x",x);
		set_constant("norb",norb);
		std::string tmp = job + ".siox";
		const char* nm= tmp.c_str();
		add_sial_program(nm);
		int segs[]  = {20,5};
		set_aoindex_info(2,segs);
		finalize_setup();
	}

	//read and print setup_file
	//// setup::SetupReader setup_reader;

	setup::BinaryInputFile setup_file(job + ".dat");
	// setup_reader.read(setup_file);
	setup::SetupReader setup_reader(setup_file);

	std::cout << "SETUP READER DATA:\n" << setup_reader<< std::endl;


	//get siox name from setup, load and print the sip tables
	std::string prog_name = setup_reader.sial_prog_list_.at(0);
	std::string siox_dir(dir_name);
	setup::BinaryInputFile siox_file(siox_dir + prog_name);
	// sip::SioxReader siox_reader(&sipTables, &siox_file, &setup_reader);
//	siox_reader.read();
	sip::SipTables sipTables(setup_reader, siox_file);
	std::cout << "SIP TABLES" << '\n' << sipTables << std::endl;

	//interpret the program
	{
	sip::SialxTimer sialxTimer(sipTables.max_timer_slots());
	sip::PersistentArrayManager<sip::Block,sip::Interpreter>* pbm;
	sip::Interpreter runner(sipTables, sialxTimer);
	std::cout << "SIAL PROGRAM OUTPUT" << std::endl;
	runner.interpret();
	ASSERT_EQ(0, sip::DataManager::scope_count);
	std::cout << "\nSIAL PROGRAM TERMINATED"<< std::endl;
//	runner.print_block();

	// Reference Calculation
	const int I = 20;
	const int J = 20;

	double a[I][J];

	int cntr = 100.0;
	for (int i=0; i<I; i++)
		for (int j=0; j<J; j++)
		// behavior of super instruction fill_block_sequential a(i,j) 100.0
			a[i][j] = cntr++ * 3.0;

	// Get the data for local array block "c"
	int a_slot = sip::Interpreter::global_interpreter->array_slot(std::string("a"));
	sip::index_selector_t a_indices;
	a_indices[0] = 1; a_indices[1] = 1;
	for (int i = 2; i < MAX_RANK; i++) a_indices[i] = sip::unused_index_value;
	sip::BlockId a_bid(a_slot, a_indices);
	std::cout << a_bid << std::endl;
	sip::Block::BlockPtr a_bptr = sip::Interpreter::global_interpreter->get_block_for_reading(a_bid);
	sip::Block::dataPtr a_data = a_bptr->get_data();

	for (int i=0; i<I; i++)
		for (int j=0; j<J; j++)
			ASSERT_DOUBLE_EQ(a_data[i*J+j], a[i][j]);
	}
}

TEST(Sial,gpu_self_multiply_op){
	std::cout << "****************************************\n";
	sip::DataManager::scope_count=0;
	//create setup_file
	std::string job("gpu_self_multiply_test");
	std::cout << "JOBNAME = " << job << std::endl;
	double x = 3.456;
	int norb = 2;

	{	init_setup(job.c_str());
		set_scalar("x",x);
		set_constant("norb",norb);
		std::string tmp = job + ".siox";
		const char* nm= tmp.c_str();
		add_sial_program(nm);
		int segs[]  = {20,5};
		set_aoindex_info(2,segs);
		finalize_setup();
	}

	//read and print setup_file
	//// setup::SetupReader setup_reader;

	setup::BinaryInputFile setup_file(job + ".dat");
	// setup_reader.read(setup_file);
	setup::SetupReader setup_reader(setup_file);

	std::cout << "SETUP READER DATA:\n" << setup_reader<< std::endl;


	//get siox name from setup, load and print the sip tables
	std::string prog_name = setup_reader.sial_prog_list_.at(0);
	std::string siox_dir(dir_name);
	setup::BinaryInputFile siox_file(siox_dir + prog_name);
	// sip::SioxReader siox_reader(&sipTables, &siox_file, &setup_reader);
//	siox_reader.read();
	sip::SipTables sipTables(setup_reader, siox_file);
	std::cout << "SIP TABLES" << '\n' << sipTables << std::endl;

	//interpret the program
	{
	sip::SialxTimer sialxTimer(sipTables.max_timer_slots());
	sip::PersistentArrayManager<sip::Block,sip::Interpreter>* pbm;
	sip::Interpreter runner(sipTables, sialxTimer);
	std::cout << "SIAL PROGRAM OUTPUT" << std::endl;
	runner.interpret();
	ASSERT_EQ(0, sip::DataManager::scope_count);
	std::cout << "\nSIAL PROGRAM TERMINATED"<< std::endl;
//	runner.print_block();

	// Reference Calculation
	const int I = 20;
	const int J = 20;

	double a[I][J];

	int cntr = 100.0;
	for (int i=0; i<I; i++)
		for (int j=0; j<J; j++)
		// behavior of super instruction fill_block_sequential a(i,j) 100.0
			a[i][j] = cntr++ * 3.0;

	// Get the data for local array block "c"
	int a_slot = sip::Interpreter::global_interpreter->array_slot(std::string("a"));
	sip::index_selector_t a_indices;
	a_indices[0] = 1; a_indices[1] = 1;
	for (int i = 2; i < MAX_RANK; i++) a_indices[i] = sip::unused_index_value;
	sip::BlockId a_bid(a_slot, a_indices);
	std::cout << a_bid << std::endl;
	sip::Block::BlockPtr a_bptr = sip::Interpreter::global_interpreter->get_block_for_reading(a_bid);
	sip::Block::dataPtr a_data = a_bptr->get_data();

	for (int i=0; i<I; i++)
		for (int j=0; j<J; j++)
			ASSERT_DOUBLE_EQ(a_data[i*J+j], a[i][j]);
	}
}



TEST(Sial,get_int_array_test){
	std::cout << "****************************************\n";
	sip::DataManager::scope_count=0;
	//create setup_file
	std::string job("get_int_array_test");
	std::cout << "JOBNAME = " << job << std::endl;
	double x = 3.456;
	int norb = 2;

	{	init_setup(job.c_str());
		set_scalar("x",x);
		set_constant("norb",norb);
		std::string tmp = job + ".siox";
		const char* nm= tmp.c_str();
		add_sial_program(nm);
		int segs[]  = {8,8};
		int int_array_data[] = {1,2,3,4,5,6,7,8};
		int length[] = {8};
		set_aoindex_info(2,segs);
		set_predefined_integer_array("int_array_data", 1,length, int_array_data);
		finalize_setup();
	}

	//read and print setup_file
	//// setup::SetupReader setup_reader;

	setup::BinaryInputFile setup_file(job + ".dat");
	// setup_reader.read(setup_file);
	setup::SetupReader setup_reader(setup_file);

	std::cout << "SETUP READER DATA:\n" << setup_reader<< std::endl;


	//get siox name from setup, load and print the sip tables
	std::string prog_name = setup_reader.sial_prog_list_.at(0);
	std::string siox_dir(dir_name);
	setup::BinaryInputFile siox_file(siox_dir + prog_name);
	// sip::SioxReader siox_reader(&sipTables, &siox_file, &setup_reader);
//	siox_reader.read();
	sip::SipTables sipTables(setup_reader, siox_file);
	std::cout << "SIP TABLES" << '\n' << sipTables << std::endl;

	//interpret the program
	{
	sip::SialxTimer sialxTimer(sipTables.max_timer_slots());
	sip::PersistentArrayManager<sip::Block,sip::Interpreter>* pbm;
	sip::Interpreter runner(sipTables, sialxTimer);
	std::cout << "SIAL PROGRAM OUTPUT" << std::endl;
	runner.interpret();
	ASSERT_EQ(0, sip::DataManager::scope_count);
	std::cout << "\nSIAL PROGRAM TERMINATED"<< std::endl;
//	runner.print_block()
;

	// Get the data for local array block "c"
	int c_slot = sip::Interpreter::global_interpreter->array_slot(
		std::string("c"));
	sip::index_selector_t c_indices;
	c_indices[0] = 1;
	c_indices[1] = 1;
	for (int i = 2; i < MAX_RANK; i++)
		c_indices[i] = sip::unused_index_value;
	sip::BlockId c_bid(c_slot, c_indices);
	std::cout << c_bid << std::endl;
	sip::Block::BlockPtr c_bptr =
		sip::Interpreter::global_interpreter->get_block_for_reading(
				c_bid);
	sip::Block::dataPtr c_data = c_bptr->get_data();

	for (int i=1; i<=8; i++){
		ASSERT_DOUBLE_EQ(i, c_data[i-1]);
	}

	}
}

TEST(Sial,get_scalar_array_test){
	std::cout << "****************************************\n";
	sip::DataManager::scope_count=0;
	//create setup_file
	std::string job("get_scalar_array_test");
	std::cout << "JOBNAME = " << job << std::endl;
	double x = 3.456;
	int norb = 2;

	{	init_setup(job.c_str());
		set_scalar("x",x);
		set_constant("norb",norb);
		std::string tmp = job + ".siox";
		const char* nm= tmp.c_str();
		add_sial_program(nm);
		int segs[]  = {8,8};
		double scalar_array_data[] = {2.0,3.0,4.0,5.0,6.0,7.0,8.0,9.0};
		int length[] = {8};
		set_aoindex_info(2,segs);
		set_predefined_scalar_array("scalar_array_data", 1,length, scalar_array_data);
		finalize_setup();
	}

	//read and print setup_file
	//// setup::SetupReader setup_reader;

	setup::BinaryInputFile setup_file(job + ".dat");
	// setup_reader.read(setup_file);
	setup::SetupReader setup_reader(setup_file);

	std::cout << "SETUP READER DATA:\n" << setup_reader<< std::endl;


	//get siox name from setup, load and print the sip tables
	std::string prog_name = setup_reader.sial_prog_list_.at(0);
	std::string siox_dir(dir_name);
	setup::BinaryInputFile siox_file(siox_dir + prog_name);
	// sip::SioxReader siox_reader(&sipTables, &siox_file, &setup_reader);
//	siox_reader.read();
	sip::SipTables sipTables(setup_reader, siox_file);
	std::cout << "SIP TABLES" << '\n' << sipTables << std::endl;

	//interpret the program
	{
	sip::SialxTimer sialxTimer(sipTables.max_timer_slots());
	sip::PersistentArrayManager<sip::Block,sip::Interpreter>* pbm;
	sip::Interpreter runner(sipTables, sialxTimer);
	std::cout << "SIAL PROGRAM OUTPUT" << std::endl;
	runner.interpret();
	ASSERT_EQ(0, sip::DataManager::scope_count);
	std::cout << "\nSIAL PROGRAM TERMINATED"<< std::endl;
//	runner.print_block()
;
	// Get the data for local array block "c"
	int c_slot = sip::Interpreter::global_interpreter->array_slot(
		std::string("c"));
	sip::index_selector_t c_indices;
	c_indices[0] = 1;
	c_indices[1] = 1;
	for (int i = 2; i < MAX_RANK; i++)
		c_indices[i] = sip::unused_index_value;
	sip::BlockId c_bid(c_slot, c_indices);
	std::cout << c_bid << std::endl;
	sip::Block::BlockPtr c_bptr =
		sip::Interpreter::global_interpreter->get_block_for_reading(
				c_bid);
	sip::Block::dataPtr c_data = c_bptr->get_data();

	for (int i=2; i<=9; i++){
		ASSERT_DOUBLE_EQ(i, c_data[i-2]);
	}
	}
}


TEST(Sial,get_scratch_array_test){
	std::cout << "****************************************\n";
	sip::DataManager::scope_count=0;
	//create setup_file
	std::string job("get_scratch_array_test");
	std::cout << "JOBNAME = " << job << std::endl;
	double x = 3.456;
	int norb = 2;

	{
		init_setup(job.c_str());
		set_constant("norb",norb);
		std::string tmp = job + ".siox";
		const char* nm= tmp.c_str();
		add_sial_program(nm);
		int segs[]  = {32,8};
		int length[] = {8};
		set_aoindex_info(2,segs);
		finalize_setup();
	}

	//read and print setup_file
	//// setup::SetupReader setup_reader;

	setup::BinaryInputFile setup_file(job + ".dat");
	// setup_reader.read(setup_file);
	setup::SetupReader setup_reader(setup_file);

	std::cout << "SETUP READER DATA:\n" << setup_reader<< std::endl;


	//get siox name from setup, load and print the sip tables
	std::string prog_name = setup_reader.sial_prog_list_.at(0);
	std::string siox_dir(dir_name);
	setup::BinaryInputFile siox_file(siox_dir + prog_name);
	// sip::SioxReader siox_reader(&sipTables, &siox_file, &setup_reader);
//	siox_reader.read();
	sip::SipTables sipTables(setup_reader, siox_file);
	std::cout << "SIP TABLES" << '\n' << sipTables << std::endl;

	//interpret the program
	{
	sip::SialxTimer sialxTimer(sipTables.max_timer_slots());
	sip::PersistentArrayManager<sip::Block,sip::Interpreter>* pbm;
	sip::Interpreter runner(sipTables, sialxTimer);
	std::cout << "SIAL PROGRAM OUTPUT" << std::endl;
	runner.interpret();
	ASSERT_EQ(0, sip::DataManager::scope_count);
	std::cout << "\nSIAL PROGRAM TERMINATED"<< std::endl;
//	runner.print_block()
;
	// Get the data for local array block "c"
	int c_slot = sip::Interpreter::global_interpreter->array_slot(
		std::string("c"));
	sip::index_selector_t c_indices;
	c_indices[0] = 1;
	c_indices[1] = 1;
	for (int i = 2; i < MAX_RANK; i++)
		c_indices[i] = sip::unused_index_value;
	sip::BlockId c_bid(c_slot, c_indices);
	std::cout << c_bid << std::endl;
	sip::Block::BlockPtr c_bptr =
		sip::Interpreter::global_interpreter->get_block_for_reading(
				c_bid);
	sip::Block::dataPtr c_data = c_bptr->get_data();

	for (int i=1; i<=32; i++){
		ASSERT_DOUBLE_EQ(i, c_data[i-1]);
	}
	}

}


TEST(Sial,gpu_contraction_predefined){
	std::cout << "****************************************\n";
	sip::DataManager::scope_count=0;
	//create setup_file
	std::string job("gpu_contraction_predefined_test");
	std::cout << "JOBNAME = " << job << std::endl;
	int aoindex_array[] = {15,15,15,15,15,15,15,15,14,14,14,14,12,12};

	const int A = 15;
	const int B = 15;
	double ca[A][B];

	const char * arrName = "ca";
	int dims[2] = {15, 15};
	for (int i=0; i<A; i++)
		for (int j=0; j<B; j++)
			ca[i][j] = 1;

	{	init_setup(job.c_str());
		set_aoindex_info(14, aoindex_array);
		set_predefined_scalar_array(arrName, 2, dims, &ca[0][0]);
		std::string tmp = job + ".siox";
		const char* nm= tmp.c_str();
		add_sial_program(nm);
		finalize_setup();
	}

	//read and print setup_file
	//// setup::SetupReader setup_reader;

	setup::BinaryInputFile setup_file(job + ".dat");
	// setup_reader.read(setup_file);
	setup::SetupReader setup_reader(setup_file);

	std::cout << "SETUP READER DATA:\n" << setup_reader<< std::endl;


	//get siox name from setup, load and print the sip tables
	std::string prog_name = setup_reader.sial_prog_list_.at(0);
	std::string siox_dir(dir_name);
	setup::BinaryInputFile siox_file(siox_dir + prog_name);
	// sip::SioxReader siox_reader(&sipTables, &siox_file, &setup_reader);
//	siox_reader.read();
	sip::SipTables sipTables(setup_reader, siox_file);
	std::cout << "SIP TABLES" << '\n' << sipTables << std::endl;

	//interpret the program
	{
	sip::SialxTimer sialxTimer(sipTables.max_timer_slots());
	sip::PersistentArrayManager<sip::Block,sip::Interpreter>* pbm;
	sip::Interpreter runner(sipTables, sialxTimer);
	std::cout << "SIAL PROGRAM OUTPUT" << std::endl;
	runner.interpret();
	ASSERT_EQ(0, sip::DataManager::scope_count);

	// Get value of scalar "v1"
	double actual_v1 = runner.data_manager_.scalar_value("v1");

	// Compare it with the reference
	int cntr = 0;
	double v2[A][B];
	for (int i=0; i<A; i++)
		for (int j=0; j<B; j++)
			v2[i][j] = cntr++ % 20 + 1;

	double ref_v1 = 0;
	for (int i=0; i<A; i++)
		for (int j=0; j<B; j++)
			ref_v1 += v2[i][j] * ca[i][j];

	ASSERT_DOUBLE_EQ(ref_v1, actual_v1);

	std::cout << "\nSIAL PROGRAM TERMINATED"<< std::endl;
	}
}

TEST(Sial,transpose4d_tmp){
	std::cout << "****************************************\n";
	sip::DataManager::scope_count=0;
	//create setup_file
	std::string job("transpose4d_tmp");
	std::cout << "JOBNAME = " << job << std::endl;
	double x = 3.456;
	double y = -0.1;
	int norb = 3;

	{	init_setup(job.c_str());
		set_scalar("x",x);
		set_scalar("y",y);
		set_constant("norb",norb);
		std::string tmp = job + ".siox";
		const char* nm= tmp.c_str();
		add_sial_program(nm);
		//int aosegs[]  = {8};
		//set_aoindex_info(1,aosegs);
		int virtoccsegs[] = {1, 5, 4};
		set_moaindex_info(3, virtoccsegs);
		set_constant("baocc", 1);
		set_constant("eaocc", 1);
		set_constant("bavirt", 2);
		set_constant("eavirt", 3);
		finalize_setup();
	}

	//read and print setup_file
	//// setup::SetupReader setup_reader;

	setup::BinaryInputFile setup_file(job + ".dat");
	// setup_reader.read(setup_file);
	setup::SetupReader setup_reader(setup_file);

	std::cout << "SETUP READER DATA:\n" << setup_reader<< std::endl;


	//get siox name from setup, load and print the sip tables
	std::string prog_name = setup_reader.sial_prog_list_.at(0);
	std::string siox_dir(dir_name);
	setup::BinaryInputFile siox_file(siox_dir + prog_name);
	// sip::SioxReader siox_reader(&sipTables, &siox_file, &setup_reader);
//	siox_reader.read();
	sip::SipTables sipTables(setup_reader, siox_file);
	std::cout << "SIP TABLES" << '\n' << sipTables << std::endl;

	//interpret the program
	{
	sip::SialxTimer sialxTimer(sipTables.max_timer_slots());
	sip::PersistentArrayManager<sip::Block,sip::Interpreter>* pbm;
	sip::Interpreter runner(sipTables, sialxTimer);
	std::cout << "SIAL PROGRAM OUTPUT" << std::endl;
	runner.interpret();
	ASSERT_EQ(0, sip::DataManager::scope_count);
	std::cout << "\nSIAL PROGRAM TERMINATED"<< std::endl;

	std::cout << "PRINTING BLOCK MAP" << std::endl;
	std::cout << sip::Interpreter::global_interpreter->data_manager_.block_manager_ << std::endl;

	// Get the data for local array block "b"
	int b_slot = sip::Interpreter::global_interpreter->array_slot(std::string("b"));
	sip::index_selector_t b_indices;
	b_indices[0] = 1; b_indices[1] = 1;
	for (int i = 2; i < MAX_RANK; i++) b_indices[i] = sip::unused_index_value;
	sip::BlockId b_bid(b_slot, b_indices);
	std::cout << "b_bid = " << b_bid << std::endl;
//		array::Block::BlockPtr b_bptr = sip::Interpreter::global_interpreter->get_block_for_reading(b_bid);
//		array::Block::dataPtr b_data = b_bptr->get_data();
//
//
//		int passed = test_transpose4d_op(b_data);
//		ASSERT_TRUE(passed);

	}

}

TEST(Sial,transpose4d_square_tmp){
	std::cout << "****************************************\n";
	sip::DataManager::scope_count=0;
	//create setup_file
	std::string job("transpose4d_square_tmp");
	std::cout << "JOBNAME = " << job << std::endl;
	double x = 3.456;
	double y = -0.1;
	int norb = 3;

	{	init_setup(job.c_str());
		set_scalar("x",x);
		set_scalar("y",y);
		set_constant("norb",norb);
		std::string tmp = job + ".siox";
		const char* nm= tmp.c_str();
		add_sial_program(nm);
		int segs[]  = {8,8,8};
		set_aoindex_info(3,segs);
		finalize_setup();
	}

	//read and print setup_file
	// setup::SetupReader setup_reader;

	setup::BinaryInputFile setup_file(job + ".dat");
	// setup_reader.read(setup_file);
	setup::SetupReader setup_reader(setup_file);

	std::cout << "SETUP READER DATA:\n" << setup_reader<< std::endl;


	//get siox name from setup, load and print the sip tables
	std::string prog_name = setup_reader.sial_prog_list_.at(0);
	std::string siox_dir(dir_name);
	setup::BinaryInputFile siox_file(siox_dir + prog_name);
	// sip::SioxReader siox_reader(&sipTables, &siox_file, &setup_reader);
//	siox_reader.read();
	sip::SipTables sipTables(setup_reader, siox_file);
	std::cout << "SIP TABLES" << '\n' << sipTables << std::endl;

	//interpret the program
	{
	sip::SialxTimer sialxTimer(sipTables.max_timer_slots());
	sip::PersistentArrayManager<sip::Block,sip::Interpreter>* pbm;
	sip::Interpreter runner(sipTables, sialxTimer);
	std::cout << "SIAL PROGRAM OUTPUT" << std::endl;
	runner.interpret();
	ASSERT_EQ(0, sip::DataManager::scope_count);
	std::cout << "\nSIAL PROGRAM TERMINATED"<< std::endl;

	// Get the data for local array block "b"
	int b_slot = sip::Interpreter::global_interpreter->array_slot(std::string("b"));
	sip::index_selector_t b_indices;
	b_indices[0] = 1; b_indices[1] = 1;
	for (int i = 2; i < MAX_RANK; i++) b_indices[i] = sip::unused_index_value;
	sip::BlockId b_bid(b_slot, b_indices);
	std::cout << b_bid << std::endl;
	sip::Block::BlockPtr b_bptr = sip::Interpreter::global_interpreter->get_block_for_reading(b_bid);
	sip::Block::dataPtr b_data = b_bptr->get_data();


	const int I = 8;
	const int J = 8;
	const int K = 8;
	const int L = 8;
	double a[I][J][K][L];
	double b[K][J][I][L];
	double esum1 = 0.0;
	double esum2 = 0.0;
	double esum3 = 0.0;

	// Fill A
	int counter = 0.0;
	for (int i=0; i<I; i++)
		for (int j=0; j<J; j++)
			for (int k=0; k<K; k++)
				for (int l=0; l<L; l++)
					a[i][j][k][l] = (counter++ %20) + 1;

	// Fill B
	for (int i=0; i<I; i++)
		for (int j=0; j<J; j++)
			for (int k=0; k<K; k++)
				for (int l=0; l<L; l++)
					b[k][j][i][l] = a[i][j][k][l];

	// Calculate esum1 & esum2;
	for (int i=0; i<I; i++)
		for (int j=0; j<J; j++)
			for (int k=0; k<K; k++)
				for (int l=0; l<L; l++){
					esum1 += a[i][j][k][l] * a[i][j][k][l];
					esum2 += b[k][j][i][l] * b[k][j][i][l];
					esum3 += a[i][j][k][l] * b[k][j][i][l];
				}

	double actual_esum1 = runner.data_manager_.scalar_value("esum1");
	double actual_esum2 = runner.data_manager_.scalar_value("esum2");
	double actual_esum3 = runner.data_manager_.scalar_value("esum3");

	ASSERT_DOUBLE_EQ(esum1, actual_esum1);
	ASSERT_DOUBLE_EQ(esum2, actual_esum2);
	ASSERT_DOUBLE_EQ(esum3, actual_esum3);

	}
}

TEST(Sial,reproduce_transpose_problem){
	std::cout << "****************************************\n";
	sip::DataManager::scope_count=0;
	//create setup_file
	std::string job("reproduce_transpose_problem");
	std::cout << "JOBNAME = " << job << std::endl;
	double x = 3.456;
	double y = -0.1;
	int norb = 3;

	{	init_setup(job.c_str());
		set_scalar("x",x);
		set_scalar("y",y);
		set_constant("norb",norb);
		std::string tmp = job + ".siox";
		const char* nm= tmp.c_str();
		add_sial_program(nm);
		//int aosegs[]  = {8};
		//set_aoindex_info(1,aosegs);
		int virtoccsegs[] = {1, 5, 4};
		set_moaindex_info(3, virtoccsegs);
		set_constant("baocc", 1);
		set_constant("eaocc", 1);
		set_constant("bavirt", 2);
		set_constant("eavirt", 3);
		finalize_setup();
	}

	//read and print setup_file
	//// setup::SetupReader setup_reader;

	setup::BinaryInputFile setup_file(job + ".dat");
	// setup_reader.read(setup_file);
	setup::SetupReader setup_reader(setup_file);

	std::cout << "SETUP READER DATA:\n" << setup_reader<< std::endl;


	//get siox name from setup, load and print the sip tables
	std::string prog_name = setup_reader.sial_prog_list_.at(0);
	std::string siox_dir(dir_name);
	setup::BinaryInputFile siox_file(siox_dir + prog_name);
	// sip::SioxReader siox_reader(&sipTables, &siox_file, &setup_reader);
//	siox_reader.read();
	sip::SipTables sipTables(setup_reader, siox_file);
	std::cout << "SIP TABLES" << '\n' << sipTables << std::endl;

	//interpret the program
	{
	sip::SialxTimer sialxTimer(sipTables.max_timer_slots());
	sip::PersistentArrayManager<sip::Block,sip::Interpreter>* pbm;
	sip::Interpreter runner(sipTables, sialxTimer);
	std::cout << "SIAL PROGRAM OUTPUT" << std::endl;
	runner.interpret();
	ASSERT_EQ(0, sip::DataManager::scope_count);
	std::cout << "\nSIAL PROGRAM TERMINATED"<< std::endl;

	std::cout << "PRINTING BLOCK MAP" << std::endl;
	std::cout << sip::Interpreter::global_interpreter->data_manager_.block_manager_ << std::endl;




	}

}

TEST(Sial,assign_to_static_array_test){
	std::cout << "****************************************\n";
	sip::DataManager::scope_count=0;
	//create setup_file
	std::string job("assign_to_static_array_test");
	std::cout << "JOBNAME = " << job << std::endl;
	double x = 3.456;
	int norb = 4;

	{	init_setup(job.c_str());
		set_scalar("x",x);
		set_constant("norb",norb);
		std::string tmp = job + ".siox";
		const char* nm= tmp.c_str();
		add_sial_program(nm);
		int segs[]  = {8,16,5,6};
		set_aoindex_info(4,segs);
		finalize_setup();
	}

	//read and print setup_file
	//// setup::SetupReader setup_reader;

	setup::BinaryInputFile setup_file(job + ".dat");
	// setup_reader.read(setup_file);
	setup::SetupReader setup_reader(setup_file);
	std::cout << "SETUP READER DATA:\n" << setup_reader<< std::endl;


	//get siox name from setup, load and print the sip tables
	std::string prog_name = setup_reader.sial_prog_list_.at(0);
	std::string siox_dir(dir_name);
	setup::BinaryInputFile siox_file(siox_dir + prog_name);
	// sip::SioxReader siox_reader(&sipTables, &siox_file, &setup_reader);
//	siox_reader.read();
	sip::SipTables sipTables(setup_reader, siox_file);
	std::cout << "SIP TABLES" << '\n' << sipTables << std::endl;


	//interpret the program
	{
	sip::SialxTimer sialxTimer(sipTables.max_timer_slots());
	sip::PersistentArrayManager<sip::Block,sip::Interpreter>* pbm;
	sip::Interpreter runner(sipTables, sialxTimer);
	std::cout << "SIAL PROGRAM OUTPUT" << std::endl;
	runner.interpret();
	ASSERT_EQ(0, sip::DataManager::scope_count);
	std::cout << "\nSIAL PROGRAM TERMINATED"<< std::endl;
//	runner.print_block()
;
	}
}

TEST(Sial,set_persistent_test){
	std::cout << "****************************************\n";
	sip::DataManager::scope_count=0;
	//create setup_file
	std::string job("set_persistent_test");
	std::string job2("restore_persistent_test");
	std::cout << "JOBNAME = " << job << std::endl;
	double x = 3.456;
	int norb = 2;

	{
		init_setup(job.c_str());
		set_scalar("x",x);
		double ca_array[6] = {7, 8, 9, 10, 11, 12};
		int dims[] = {2, 2};
		set_predefined_scalar_array("ca", 2, dims, ca_array);
		int segs[]  = {2,3};
		set_aoindex_info(2,segs);
		set_constant("norb",norb);
		std::string tmp = job + ".siox";
		std::string tmp2 = job2 + ".siox";
		const char* nm= tmp.c_str();
		const char* nm2 = tmp2.c_str();
		add_sial_program(nm);
		add_sial_program(nm2);
		finalize_setup();
	}

	//read and print setup_file
	setup::BinaryInputFile setup_file(job + ".dat");
	setup::SetupReader setup_reader(setup_file);

	std::cout << "SETUP READER DATA:\n" << setup_reader<< std::endl;

	sip::PersistentArrayManager<sip::Block,sip::Interpreter>* pbm;
	pbm = new sip::PersistentArrayManager<sip::Block,sip::Interpreter>();

	{

			//get siox name from setup, load and print the sip tables
			std::string prog_name = setup_reader.sial_prog_list_.at(0);
			sip::GlobalState::set_program_name(prog_name);
			std::string siox_dir(dir_name);
			setup::BinaryInputFile siox_file(siox_dir + prog_name);
		//	// sip::SioxReader siox_reader(&sipTables, &siox_file, &setup_reader);
		//	siox_reader.read();
			sip::SipTables sipTables(setup_reader, siox_file);
			std::cout << "SIP TABLES" << '\n' << sipTables << std::endl;

		//interpret the program
		{
			sip::SialxTimer sialxTimer(sipTables.max_timer_slots());
			sip::Interpreter runner(sipTables, sialxTimer, pbm);
			std::cout << "SIAL PROGRAM OUTPUT" << std::endl;
			runner.interpret();
			std::cout<<"PBM:"<<std::endl << *pbm << std::endl;
			pbm->save_marked_arrays(&runner);
			std::cout <<"PBM after saving:"<<std::endl << *pbm << std::endl;
			ASSERT_EQ(0, sip::DataManager::scope_count);
			std::cout << "\nSIAL PROGRAM TERMINATED"<< std::endl;
		}
	}
	std::cout<<"PBM1:"<<std::endl << *pbm << std::endl;

	{

			//get siox name from setup, load and print the sip tables
			std::string prog_name2 = setup_reader.sial_prog_list_.at(1);
			std::string siox_dir(dir_name);
			setup::BinaryInputFile siox_file(siox_dir + prog_name2);
			sip::GlobalState::set_program_name(prog_name2);
		//	// sip::SioxReader siox_reader(&sipTables, &siox_file, &setup_reader);
		//	siox_reader.read();
			sip::SipTables sipTables(setup_reader, siox_file);
			std::cout << "SIP TABLES" << '\n' << sipTables << std::endl;

		//interpret the program
		{
			sip::SialxTimer sialxTimer(sipTables.max_timer_slots());
			sip::Interpreter runner(sipTables, sialxTimer, pbm);
			std::cout << "SIAL PROGRAM OUTPUT" << std::endl;
			runner.interpret();
			ASSERT_EQ(0, sip::DataManager::scope_count);
			std::cout << "\nSIAL PROGRAM TERMINATED"<< std::endl;
		}
	}
	std::cout<<"PBM3:"<<std::endl;
	std::cout<<*pbm<<std::endl;

}

TEST(Sial,persistent_static_array_test){
	std::cout << "****************************************\n";
	sip::DataManager::scope_count=0;
	//create setup_file
	std::string job("persistent_static_array_test");
	std::cout << "JOBNAME = " << job << std::endl;
	double x = 3.456;
	int norb = 2;

	{	init_setup(job.c_str());
		set_scalar("x",x);
		set_constant("norb",norb);
		std::string tmp = job + "1.siox";
		const char* nm= tmp.c_str();
		add_sial_program(nm);
		std::string tmp1 = job + "2.siox";
		const char* nm1= tmp1.c_str();
		add_sial_program(nm1);
		int segs[]  = {8,16};
		set_aoindex_info(2,segs);
		finalize_setup();
	}

	//read and print setup_file
	//// setup::SetupReader setup_reader;

	setup::BinaryInputFile setup_file(job + ".dat");
	// setup_reader.read(setup_file);
	setup::SetupReader setup_reader(setup_file);

	std::cout << "SETUP READER DATA:\n" << setup_reader<< std::endl;
	sip::PersistentArrayManager<sip::Block,sip::Interpreter>* pbm;
	pbm = new sip::PersistentArrayManager<sip::Block,sip::Interpreter>();
	//get siox name from setup, load and print the sip tables
	std::string prog_name = setup_reader.sial_prog_list_.at(0);
	std::string siox_dir(dir_name);
	setup::BinaryInputFile siox_file(siox_dir + prog_name);
	// sip::SioxReader siox_reader(&sipTables, &siox_file, &setup_reader);
//	siox_reader.read();
	sip::SipTables sipTables(setup_reader, siox_file);
	std::cout << "SIP TABLES" << '\n' << sipTables << std::endl;


	//interpret the program
	{
	sip::SialxTimer sialxTimer(sipTables.max_timer_slots());

	sip::Interpreter runner(sipTables, sialxTimer, pbm);
	std::cout << "SIAL PROGRAM OUTPUT" << std::endl;
	runner.interpret();
	ASSERT_EQ(0, sip::DataManager::scope_count);
	pbm->save_marked_arrays(&runner);
	std::cout << "\nSIAL PROGRAM TERMINATED"<< std::endl;
//	runner.print_block()
;
	}
	//get siox name from setup, load and print the sip tables
	std::string prog_name2 = setup_reader.sial_prog_list_.at(1);
	setup::BinaryInputFile siox_file2(siox_dir + prog_name2);
	// sip::SioxReader siox_reader(&sipTables, &siox_file, &setup_reader);
//	siox_reader.read();
	sip::SipTables sipTables2(setup_reader, siox_file2);
	std::cout << "SIP TABLES FOR PROGRAM 2" << '\n' << sipTables2 << std::endl;


	//interpret the program
	{
	sip::SialxTimer sialxTimer(sipTables2.max_timer_slots());

	sip::Interpreter runner(sipTables2, sialxTimer, pbm);
	std::cout << "SIAL PROGRAM OUTPUT" << std::endl;
	runner.interpret();
	ASSERT_EQ(0, sip::DataManager::scope_count);

	std::cout << "\nSIAL PROGRAM TERMINATED"<< std::endl;
//	runner.print_block()
	}
	delete pbm;
}


TEST(Sial,persistent_distributed_array_test){
	std::cout << "****************************************\n";
	sip::DataManager::scope_count=0;
	//create setup_file
	std::string job("persistent_distributed_array_test");
	std::cout << "JOBNAME = " << job << std::endl;
	double x = 3.456;
	int norb = 2;

	{	init_setup(job.c_str());
		set_scalar("x",x);
		set_constant("norb",norb);
		std::string tmp = job + "1.siox";
		const char* nm= tmp.c_str();
		add_sial_program(nm);
		std::string tmp1 = job + "2.siox";
		const char* nm1= tmp1.c_str();
		add_sial_program(nm1);
		int segs[]  = {8,16};
		set_aoindex_info(2,segs);
		finalize_setup();
	}

	//read and print setup_file
	//// setup::SetupReader setup_reader;

	setup::BinaryInputFile setup_file(job + ".dat");
	// setup_reader.read(setup_file);
	setup::SetupReader setup_reader(setup_file);

	std::cout << "SETUP READER DATA:\n" << setup_reader<< std::endl;
	sip::PersistentArrayManager<sip::Block,sip::Interpreter>* pbm;
	pbm = new sip::PersistentArrayManager<sip::Block,sip::Interpreter>();
	//get siox name from setup, load and print the sip tables
	std::string prog_name = setup_reader.sial_prog_list_.at(0);
	std::string siox_dir(dir_name);
	setup::BinaryInputFile siox_file(siox_dir + prog_name);
	// sip::SioxReader siox_reader(&sipTables, &siox_file, &setup_reader);
//	siox_reader.read();
	sip::SipTables sipTables(setup_reader, siox_file);
	std::cout << "SIP TABLES" << '\n' << sipTables << std::endl;


	//interpret the program
	{
	sip::SialxTimer sialxTimer(sipTables.max_timer_slots());

	sip::Interpreter runner(sipTables, sialxTimer, pbm);
	std::cout << "SIAL PROGRAM OUTPUT" << std::endl;
	runner.interpret();
	ASSERT_EQ(0, sip::DataManager::scope_count);
	pbm->save_marked_arrays(&runner);
	std::cout << "\nSIAL PROGRAM TERMINATED"<< std::endl;
//	runner.print_block()
;
	}
	//get siox name from setup, load and print the sip tables
	std::string prog_name2 = setup_reader.sial_prog_list_.at(1);
	setup::BinaryInputFile siox_file2(siox_dir + prog_name2);
	// sip::SioxReader siox_reader(&sipTables, &siox_file, &setup_reader);
//	siox_reader.read();
	sip::SipTables sipTables2(setup_reader, siox_file2);
	std::cout << "SIP TABLES FOR PROGRAM 2" << '\n' << sipTables2 << std::endl;


	//interpret the program
	{

	sip::SialxTimer sialxTimer2(sipTables2.max_timer_slots());

	sip::Interpreter runner2(sipTables2, sialxTimer2, pbm);
	std::cout << "SIAL PROGRAM OUTPUT" << std::endl;
	runner2.interpret();
	ASSERT_EQ(0, sip::DataManager::scope_count);

	std::cout << "\nSIAL PROGRAM TERMINATED"<< std::endl;
//	runner.print_block()
	}
	//delete pbm;
}
int main(int argc, char **argv) {

#ifdef HAVE_TAU
	TAU_PROFILE_SET_NODE(0);
	TAU_STATIC_PHASE_START("SIP Main");
#endif

	sip::check(sizeof(int) >= 4, "Size of integer should be 4 bytes or more");
	sip::check(sizeof(double) >= 8, "Size of double should be 8 bytes or more");
	sip::check(sizeof(long long) >= 8, "Size of long long should be 8 bytes or more");

	printf("Running main() from test_simple.cpp\n");
	testing::InitGoogleTest(&argc, argv);
	int result =  RUN_ALL_TESTS();


#ifdef HAVE_TAU
	TAU_STATIC_PHASE_STOP("SIP Main");
#endif

	return result;

}

