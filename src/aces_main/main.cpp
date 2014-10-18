#include "main_helper_methods.h"


int main(int argc, char* argv[]) {
	check_expected_datasizes();
    setup_signal_and_exception_handlers();
    mpi_init(&argc, &argv);
	sip::SIPMPIAttr &sip_mpi_attr = sip::SIPMPIAttr::get_instance(); // singleton instance.
	std::cout<<sip_mpi_attr<<std::endl;

	INIT_GLOBAL_TIMERS(&argc, &argv);

	char *init_file = "data.dat"; // Default initialization file is data.dat
	char *sialx_file_dir = ".";   // Default directory for compiled sialx files is "."
	char *json_file = NULL;		  // Json file, no default file
	bool json_specified = false;
	bool init_file_specified = false;

	std::size_t memory = 2147483648;	// Default memory usage : 2 GB

	// Read about getopt here : http://www.gnu.org/software/libc/manual/html_node/Getopt.html
	// d: name of .dat file.
	// j: name of json file (must specify either of d or j, not both.
	// s: directory to look for siox files
	// m: approximate memory to be used. Actual usage will be more than this.
	// h & ? are for help. They require no arguments
	const char *optString = "d:j:s:m:h?";
	int c;
	while ((c = getopt(argc, argv, optString)) != -1){
		switch (c) {
		case 'd':{
			init_file = optarg;
			init_file_specified = true;
		}
			break;
		case 'j' :{
			json_file = optarg;
			json_specified = true;
		}
			break;
		case 's':{
			sialx_file_dir = optarg;
		}
			break;
		case 'm':{
			std::string memory_string(optarg);
			std::stringstream ss(memory_string);
			double memory_in_gb;
			ss >> memory_in_gb;
			memory = memory_in_gb * 1024 * 1024 * 1024;
		}
			break;
		case 'h':case '?':
		default:
			std::string program_name = argv[0];
			print_usage(program_name);
			return 1;
		}
	}

	if (init_file_specified && json_specified){
		std::cerr << "Cannot specify both init binary data file and json init file !" << std::endl;
		std::string program_name = argv[0];
		print_usage(program_name);
		return 1;
	}

	// Set Approx Max memory usage
	sip::GlobalState::set_max_data_memory_usage(memory);
	setup::SetupReader *setup_reader = NULL;

	if (json_specified){
		std::ifstream json_ifstream(json_file, std::ifstream::in);
		setup_reader = new setup::SetupReader(json_ifstream);
	} else {
		//create setup_file
		std::string job(init_file);
		SIP_MASTER_LOG(std::cout << "Initializing data from " << job << std::endl);
		setup::BinaryInputFile setup_file(job); //initialize setup data
		setup_reader = new setup::SetupReader(setup_file);
	}
	setup_reader->aces_validate();

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
		std::cout << it->c_str() << std::endl;
		std::string sialfpath;
		sialfpath.append(sialx_file_dir);
		sialfpath.append("/");
		sialfpath.append(*it);

		sip::GlobalState::set_program_name(*it);
		sip::GlobalState::increment_program();

		START_SIALX_PROGRAM_DYNAMIC_PHASE(it->c_str());

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
			server_timer.start_program_timer();
			sip::SIPServer server(sipTables, data_distribution, sip_mpi_attr, &persistent_server, server_timer);
			server.run();
			SIP_LOG(std::cout<<"PBM after program at Server "<< sip_mpi_attr.global_rank()<< " : " << sialfpath << " :"<<std::endl<<persistent_server);
			persistent_server.save_marked_arrays(&server);
			server_timer.stop_program_timer();
			server_timer.print_timers(lno2name);
		} else
#endif

		//interpret current program on worker
		{
			sip::SialxTimer sialxTimer(sipTables.max_timer_slots());
			sialxTimer.start_program_timer();
			sip::SialxInterpreter runner(sipTables, &sialxTimer, NULL, &persistent_worker);
			SIP_MASTER(std::cout << "SIAL PROGRAM OUTPUT for "<< sialfpath << std::endl);
			runner.interpret();
			runner.post_sial_program();
			persistent_worker.save_marked_arrays(&runner);
			SIP_MASTER_LOG(std::cout<<"Persistent array manager at master worker after program " << sialfpath << " :"<<std::endl<< persistent_worker);
			SIP_MASTER(std::cout << "\nSIAL PROGRAM " << sialfpath << " TERMINATED" << std::endl);
			sialxTimer.start_program_timer();
			sialxTimer.print_timers(lno2name);

		}// end of worker or server

		STOP_SIALX_PROGRAM_DYNAMIC_PHASE();

  		barrier();
	} //end of loop over programs

	delete setup_reader;
	FINALIZE_GLOBAL_TIMERS();
	sip::SIPMPIAttr::cleanup(); // Delete singleton instance
	mpi_finalize();

	return 0;
}
