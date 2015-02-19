/*
 * profile_interpreter_main.cpp
 *
 *  Created on: Oct 8, 2014
 *      Author: njindal
 */


#include "main_helper_methods.h"
#include "profile_timer.h"
#include "profile_timer_store.h"
#include "profile_interpreter.h"

int main(int argc, char* argv[]) {
	check_expected_datasizes();
    setup_signal_and_exception_handlers();
    mpi_init(&argc, &argv);

	Aces4Parameters parameters = parse_command_line_parameters(argc, argv);
	set_rank_distribution(parameters);
	sip::SIPMPIAttr &sip_mpi_attr = sip::SIPMPIAttr::get_instance(); // singleton instance.
	std::cout<<sip_mpi_attr<<std::endl;

	//INIT_GLOBAL_TIMERS(&argc, &argv);

	// Set Approx Max memory usage
	sip::GlobalState::set_max_data_memory_usage(parameters.memory);


	setup::SetupReader* setup_reader = read_init_file(parameters);

	SIP_MASTER_LOG(std::cout << "SETUP READER DATA:\n" << setup_reader << std::endl);

	setup::SetupReader::SialProgList &progs = setup_reader->sial_prog_list();
	setup::SetupReader::SialProgList::const_iterator it;

#ifdef HAVE_MPI
	sip::ServerPersistentArrayManager persistent_server;
	sip::WorkerPersistentArrayManager persistent_worker;
#else
	sip::WorkerPersistentArrayManager persistent_worker;
#endif //HAVE_MPI


	// Temporary in memory database for faster store
	sip::ProfileTimerStore profile_timer_store(":memory:");

	for (it = progs.begin(); it != progs.end(); ++it) {
		std::cout << it->c_str() << std::endl;
		std::string sialfpath;
		sialfpath.append(parameters.sialx_file_dir);
		sialfpath.append("/");
		sialfpath.append(*it);

		sip::GlobalState::set_program_name(*it);
		sip::GlobalState::increment_program();

		//START_TAU_SIALX_PROGRAM_DYNAMIC_PHASE(it->c_str());

		setup::BinaryInputFile siox_file(sialfpath);
		sip::SipTables sipTables(*setup_reader, siox_file);

		SIP_MASTER_LOG(std::cout << "SIP TABLES" << '\n' << sipTables << std::endl);
		SIP_MASTER_LOG(std::cout << "Executing siox file : " << sialfpath << std::endl);


		//const std::vector<std::string> lno2name = sipTables.line_num_to_name();
#ifdef HAVE_MPI
		sip::DataDistribution data_distribution(sipTables, sip_mpi_attr);

		// TODO Broadcast from worker master to all servers & workers.
		if (sip_mpi_attr.is_server()){
			sip::ServerTimer server_timer(sipTables.max_timer_slots());

			sip::Timer init_server_timer("Initialize Server");
			init_server_timer.start();
			sip::SIPServer server(sipTables, data_distribution, sip_mpi_attr, &persistent_server, server_timer);
			init_server_timer.pause();

			sip::Timer server_run_timer("Server Run");
			server_run_timer.start();
			server.run();
			server_run_timer.pause();

			SIP_LOG(std::cout<<"PBM after program at Server "<< sip_mpi_attr.global_rank()<< " : " << sialfpath << " :"<<std::endl<<persistent_server);

			sip::Timer save_persistent_timer("Save Persistent Arrays");
			save_persistent_timer.start();
			persistent_server.save_marked_arrays(&server);
			save_persistent_timer.pause();

			char server_timer_file_name[64];
			std::sprintf(server_timer_file_name, "server.profile.%d", sip_mpi_attr.company_rank());
			std::ofstream server_file(server_timer_file_name, std::ofstream::app);
			//server_timer.print_timers(lno2name, server_file);
			server_timer.print_timers(server_file, sipTables);
			sip::Timer::print_timers(server_file);
			sip::Counter::print_counters(server_file);
			sip::MaxCounter::print_max_counters(server_file);
		} else
#endif

		//interpret current program on worker
		{

			sip::SialxTimer sialxTimer(sipTables.max_timer_slots());
			sip::ProfileTimer profile_timer(sialxTimer);

			sip::Timer initialize_worker_timer("Initialize Worker");
			initialize_worker_timer.start();
			sip::ProfileInterpreter runner(sipTables, profile_timer, sialxTimer, NULL, &persistent_worker);
			initialize_worker_timer.pause();

			SIP_MASTER(std::cout << "SIAL PROGRAM OUTPUT for "<< sialfpath << " Started at " << sip_timestamp() << std::endl);

			sip::Timer interpret_timer("Interpret Program");
			interpret_timer.start();
			runner.interpret();
			interpret_timer.pause();

			sip::Timer post_sial_timer("Post SIAL Program");
			post_sial_timer.start();
			runner.post_sial_program();
			post_sial_timer.pause();


			sip::Timer save_persistent_timer("Save Persistent Arrays");
			save_persistent_timer.start();
			persistent_worker.save_marked_arrays(&runner);
			save_persistent_timer.pause();

			SIP_MASTER_LOG(std::cout<<"Persistent array manager at master worker after program " << sialfpath << " :"<<std::endl<< persistent_worker);
			SIP_MASTER(std::cout << "\nSIAL PROGRAM " << sialfpath << " TERMINATED at " << sip_timestamp() << std::endl);

			sip::Timer save_to_store_timer("Save To Store");
			save_to_store_timer.start();
			profile_timer.save_to_store(profile_timer_store); // Saves to database store.
			save_to_store_timer.pause();

			char sialx_timer_file_name[64];
			std::sprintf(sialx_timer_file_name, "worker.profile.%d", sip_mpi_attr.company_rank());
			std::ofstream worker_file(sialx_timer_file_name, std::ofstream::app);

			sialxTimer.print_timers(worker_file, sipTables);
			profile_timer.print_timers(worker_file);

			sip::Timer::print_timers(worker_file);
			sip::Counter::print_counters(worker_file);
			sip::MaxCounter::print_max_counters(worker_file);


		}// end of worker or server
		// Reset counters for next program
		sip::Counter::clear_list();
		sip::MaxCounter::clear_list();
		sip::Timer::clear_list();
		///STOP_TAU_SIALX_PROGRAM_DYNAMIC_PHASE();

  		barrier();
	} //end of loop over programs

	// Sets the database name
	// For a job from ar1.dat, the profile database will be
	// profile.ar1.dat.0 for rank 0
	// profile.ar1.dat.1 for rank 1 and so on.
	std::stringstream db_name;
	db_name << "profile.db." << sip_mpi_attr.global_rank();
	sip::ProfileTimerStore disk_profile_timer_store(db_name.str());
	profile_timer_store.backup_to_other(disk_profile_timer_store);

	delete setup_reader;
	//FINALIZE_GLOBAL_TIMERS();
	sip::SIPMPIAttr::cleanup(); // Delete singleton instance
	mpi_finalize();

	return 0;
}

