#include "config.h"
#include "siox_reader.h"
#include "io_utils.h"
#include "setup_reader.h"
#include "assert.h"
#include "sialx_timer.h"
#include "sip_tables.h"
#include "interpreter.h"
#include "sialx_interpreter.h"
#include "setup_interface.h"
#include "sip_interface.h"
#include "data_manager.h"
#include "worker_persistent_array_manager.h"
#include "block.h"
#include "global_state.h"

#include <vector>
#include <sstream>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fenv.h>

#include <execinfo.h>
#include <signal.h>

#ifdef HAVE_MPI
#include "mpi.h"
#include "sip_mpi_attr.h"
#include "sip_server.h"
#include "sip_mpi_utils.h"
#include "server_persistent_array_manager.h"
#endif


#ifdef HAVE_TAU
#include <TAU.h>
#endif

/**
 * http://stackoverflow.com/questions/3151779/how-its-better-to-invoke-gdb-from-program-to-print-its-stacktrace/4611112#4611112
 */
void bt_sighandler(int signum) {
	std::cerr << "Interrupt signal (" << signum << ") received." << std::endl;
    sip_abort();
}

int main(int argc, char* argv[]) {
    
    feenableexcept(FE_DIVBYZERO);
    feenableexcept(FE_OVERFLOW);
    feenableexcept(FE_INVALID);

    signal(SIGSEGV, bt_sighandler);
    signal(SIGFPE, bt_sighandler);
    signal(SIGTERM, bt_sighandler);
    signal(SIGINT, bt_sighandler);
    signal(SIGABRT, bt_sighandler);

#ifdef HAVE_MPI
	/* MPI Initialization */
	MPI_Init(&argc, &argv);

	sip::SIPMPIUtils::set_error_handler();
	sip::SIPMPIAttr &sip_mpi_attr = sip::SIPMPIAttr::get_instance(); // singleton instance.
	std::cout<<sip_mpi_attr<<std::endl;
#endif

#ifdef HAVE_TAU
	TAU_INIT(&argc, &argv);
	#ifdef HAVE_MPI
	  TAU_PROFILE_SET_NODE(sip_mpi_attr.global_rank());
	#else
	  TAU_PROFILE_SET_NODE(0);
	#endif
	TAU_STATIC_PHASE_START("SIP Main");
#endif

//TODO  move this to  a test suite.
//	// Check sizes of data types.
//	// In the MPI version, the TAG is used to communicate information
//	// The various bits needed to send information to other nodes
//	// sums up to 32.
//	sip::check(sizeof(int) >= 4, "Size of integer should be 4 bytes or more");
//	sip::check(sizeof(double) >= 8, "Size of double should be 8 bytes or more");
//	sip::check(sizeof(long long) >= 8, "Size of long long should be 8 bytes or more");

	// Default initialization file is data.dat
	char *init_file = "data.dat";
	// Default directory for compiled sialx files is "."
	char *sialx_file_dir = ".";

	std::size_t memory = 2147483648;	// Default memory usage : 2 GB

	// Read about getopt here : http://www.gnu.org/software/libc/manual/html_node/Getopt.html
	// d: name of .dat file.
	// s: directory to look for siox files
	// m: approximate memory to be used. Actual usage will be more than this.
	// h & ? are for help. They require no arguments
	const char *optString = "d:s:m:h?";
	int c;
	while ((c = getopt(argc, argv, optString)) != -1){
		switch (c) {
		case 'd':{
			init_file = optarg;
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
			std::cerr << "Usage : "<< argv[0] <<" -d <init_data_file> -s <sialx_files_directory> -m <max_memory_in_gigabytes>" << std::endl;
			std::cerr << "\tDefaults: data file - \"data.dat\", sialx directory - \".\", Memory : 2GB" << std::endl;
			std::cerr << "\tm is the approximate memory to use. Actual usage will be more." << std::endl;
			std::cerr << "\t-? or -h to display this usage dialogue" << std::endl;
			return 1;
		}
	}

	// Set Approx Max memory usage
	sip::GlobalState::set_max_data_memory_usage(memory);

	//create setup_file
	std::string job(init_file);
	SIP_MASTER_LOG(std::cout << "Initializing data from " << job << std::endl);

	//initialize setup data
	setup::BinaryInputFile setup_file(job);
	setup::SetupReader setup_reader(setup_file);
	setup_reader.aces_validate();

	SIP_MASTER_LOG(std::cout << "SETUP READER DATA:\n" << setup_reader << std::endl);

	setup::SetupReader::SialProgList &progs = setup_reader.sial_prog_list();
	setup::SetupReader::SialProgList::iterator it;

#ifdef HAVE_MPI
	sip::ServerPersistentArrayManager persistent_server;
	sip::WorkerPersistentArrayManager persistent_worker;
#else
	sip::WorkerPersistentArrayManager persistent_worker;
#endif //HAVE_MPI


	for (it = progs.begin(); it != progs.end(); ++it) {
		std::string sialfpath;
		sialfpath.append(sialx_file_dir);
		sialfpath.append("/");
		sialfpath.append(*it);

		sip::GlobalState::set_program_name(*it);
		sip::GlobalState::increment_program();
#ifdef HAVE_TAU
		//TAU_REGISTER_EVENT(tau_event, it->c_str());
		//TAU_EVENT(tau_event, sip::GlobalState::get_program_num());
		//TAU_STATIC_PHASE_START(it->c_str());
		TAU_PHASE_CREATE_DYNAMIC(tau_dtimer, it->c_str(), "", TAU_USER);
		TAU_PHASE_START(tau_dtimer);
#endif

		setup::BinaryInputFile siox_file(sialfpath);
		sip::SipTables sipTables(setup_reader, siox_file);

		SIP_MASTER_LOG(std::cout << "SIP TABLES" << '\n' << sipTables << std::endl);
		SIP_MASTER_LOG(std::cout << "Executing siox file : " << sialfpath << std::endl);


		const std::vector<std::string> lno2name = sipTables.line_num_to_name();
#ifdef HAVE_MPI
		sip::DataDistribution data_distribution(sipTables, sip_mpi_attr);

		// TODO Broadcast from worker master to all servers & workers.
		if (sip_mpi_attr.is_server()){
			sip::ServerTimer server_timer(sipTables.max_timer_slots());
			sip::SIPServer server(sipTables, data_distribution, sip_mpi_attr, &persistent_server, server_timer);
			server.run();
			SIP_LOG(std::cout<<"PBM after program at Server "<< sip_mpi_attr.global_rank()<< " : " << sialfpath << " :"<<std::endl<<persistent_server);
			persistent_server.save_marked_arrays(&server);
			server_timer.print_timers(lno2name);
		} else
#endif

		//interpret current program on worker
		{
			sip::SialxTimer sialxTimer(sipTables.max_timer_slots());
			sip::SialxInterpreter runner(sipTables, &sialxTimer, NULL, &persistent_worker);

			SIP_MASTER(std::cout << "SIAL PROGRAM OUTPUT for "<< sialfpath << std::endl);
			runner.interpret();
			runner.post_sial_program();
			persistent_worker.save_marked_arrays(&runner);
			SIP_MASTER_LOG(std::cout<<"Persistent array manager at master worker after program " << sialfpath << " :"<<std::endl<< persistent_worker);
			SIP_MASTER(std::cout << "\nSIAL PROGRAM " << sialfpath << " TERMINATED" << std::endl);

			sialxTimer.print_timers(lno2name);

		}// end of worker or server

#ifdef HAVE_TAU
		//TAU_STATIC_PHASE_STOP(it->c_str());
  		TAU_PHASE_STOP(tau_dtimer);
#endif

#ifdef HAVE_MPI
      sip::SIPMPIUtils::check_err(MPI_Barrier(MPI_COMM_WORLD));
#endif
	} //end of loop over programs


#ifdef HAVE_TAU
		TAU_STATIC_PHASE_STOP("SIP Main");
#endif


#ifdef HAVE_MPI
	sip::SIPMPIAttr::cleanup(); // Delete singleton instance
	MPI_Finalize();
#endif


	return 0;
}
