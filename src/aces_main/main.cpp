#include "main_helper_methods.h"


int main(int argc, char* argv[]) {
	check_expected_datasizes();
    setup_signal_and_exception_handlers();
    mpi_init(&argc, &argv);

	Aces4Parameters parameters = parse_command_line_parameters(argc, argv);
	set_rank_distribution(parameters);
	sip::SIPMPIAttr &sip_mpi_attr = sip::SIPMPIAttr::get_instance(); // singleton instance.
	std::cout<<sip_mpi_attr<<std::endl;
	INIT_GLOBAL_TIMERS(&argc, &argv);

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


	for (it = progs.begin(); it != progs.end(); ++it) {
		std::string sialfpath;
		sialfpath.append(parameters.sialx_file_dir);
		sialfpath.append("/");
		sialfpath.append(*it);

		sip::GlobalState::set_program_name(*it);
		sip::GlobalState::increment_program();

		START_TAU_SIALX_PROGRAM_DYNAMIC_PHASE(it->c_str());

		setup::BinaryInputFile siox_file(sialfpath);
		sip::SipTables sipTables(*setup_reader, siox_file);

		SIP_MASTER_LOG(std::cout << "SIP TABLES" << '\n' << sipTables << std::endl);
		SIP_MASTER_LOG(std::cout << "Executing siox file : " << sialfpath << std::endl);


		const std::vector<std::string> lno2name = sipTables.line_num_to_name();
#ifdef HAVE_MPI
		sip::DataDistribution data_distribution(sipTables, sip_mpi_attr);

		// TODO Broadcast from worker master to all servers & workers.
		if (sip_mpi_attr.is_server()){
			sip::ServerTimer server_timer(sipTables.max_timer_slots());
			sip::SIPStaticNamedTimers staticNamedTimers;
			sip::CounterFactory counter_factory;

			server_timer.start_program_timer();

			staticNamedTimers.start_timer(sip::SIPStaticNamedTimers::INIT_SERVER);
			sip::SIPServer server(sipTables, data_distribution, sip_mpi_attr, &persistent_server, server_timer, counter_factory);
			staticNamedTimers.pause_timer(sip::SIPStaticNamedTimers::INIT_SERVER);

			staticNamedTimers.start_timer(sip::SIPStaticNamedTimers::SERVER_RUN);
			server.run();
			staticNamedTimers.pause_timer(sip::SIPStaticNamedTimers::SERVER_RUN);

			SIP_LOG(std::cout<<"PBM after program at Server "<< sip_mpi_attr.global_rank()<< " : " << sialfpath << " :"<<std::endl<<persistent_server);

			staticNamedTimers.start_timer(sip::SIPStaticNamedTimers::SAVE_PERSISTENT_ARRAYS);
			persistent_server.save_marked_arrays(&server);
			staticNamedTimers.pause_timer(sip::SIPStaticNamedTimers::SAVE_PERSISTENT_ARRAYS);
			server_timer.stop_program_timer();

			char server_timer_file_name[64];
			std::sprintf(server_timer_file_name, "server.timer.%d", sip_mpi_attr.company_rank());
			std::ofstream server_file(server_timer_file_name, std::ofstream::app);
			server_timer.print_timers(lno2name, server_file);
			staticNamedTimers.print_timers(server_file);

			std::ofstream aggregate_file("server.timer.aggregate", std::ofstream::app);
			server_timer.print_timers(lno2name, aggregate_file);
			staticNamedTimers.print_aggregate_timers(aggregate_file);


		} else
#endif

		//interpret current program on worker
		{
			sip::SialxTimer sialxTimer(sipTables.max_timer_slots());
			sip::SIPStaticNamedTimers staticNamedTimers;
			sip::CounterFactory counter_factory;

			sialxTimer.start_program_timer();
			sip::SialxInterpreter runner(sipTables, &sialxTimer, NULL, &persistent_worker);
			SIP_MASTER(std::cout << "SIAL PROGRAM OUTPUT for "<< sialfpath << " Started at " << sip_timestamp() << std::endl);

			staticNamedTimers.start_timer(sip::SIPStaticNamedTimers::INTERPRET_PROGRAM);
			runner.interpret();
			runner.post_sial_program();
			staticNamedTimers.pause_timer(sip::SIPStaticNamedTimers::INTERPRET_PROGRAM);

			staticNamedTimers.start_timer(sip::SIPStaticNamedTimers::SAVE_PERSISTENT_ARRAYS);
			persistent_worker.save_marked_arrays(&runner);
			staticNamedTimers.pause_timer(sip::SIPStaticNamedTimers::SAVE_PERSISTENT_ARRAYS);

			SIP_MASTER_LOG(std::cout<<"Persistent array manager at master worker after program " << sialfpath << " :"<<std::endl<< persistent_worker);
			SIP_MASTER(std::cout << "\nSIAL PROGRAM " << sialfpath << " TERMINATED at " << sip_timestamp() << std::endl);
			sialxTimer.stop_program_timer();

			char sialx_timer_file_name[64];
			std::sprintf(sialx_timer_file_name, "worker.timer.%d", sip_mpi_attr.company_rank());
			std::ofstream worker_file(sialx_timer_file_name, std::ofstream::app);
			sialxTimer.print_timers(lno2name, worker_file);
			staticNamedTimers.print_timers(worker_file);

			std::ofstream aggregate_file("worker.timer.aggregate", std::ofstream::app);
			sialxTimer.print_aggregate_timers(lno2name, aggregate_file);
			staticNamedTimers.print_aggregate_timers(aggregate_file);

		}// end of worker or server

		STOP_TAU_SIALX_PROGRAM_DYNAMIC_PHASE();

  		barrier();
	} //end of loop over programs

	delete setup_reader;
	FINALIZE_GLOBAL_TIMERS();
	sip::SIPMPIAttr::cleanup(); // Delete singleton instance
	mpi_finalize();

	return 0;
}
