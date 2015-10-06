#include "config.h"
#include "siox_reader.h"
#include "io_utils.h"
#include "setup_reader.h"
#include "assert.h"
#include "sip_tables.h"
#include "interpreter.h"
#include "setup_interface.h"
#include "sip_interface.h"
#include "data_manager.h"
#include "worker_persistent_array_manager.h"
#include "block.h"
#include "global_state.h"
#include "tracer.h"
#include "counter.h"

#include <vector>
#include <sstream>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fenv.h>
#include <ctime>
#include <fstream>

#include <execinfo.h>
#include <signal.h>

#ifdef HAVE_MPI
#include "mpi.h"
#include "sip_mpi_attr.h"
#include "sip_server.h"
#include "sip_mpi_utils.h"
#include "server_persistent_array_manager.h"
#endif


/**
 * http://stackoverflow.com/questions/3151779/how-its-better-to-invoke-gdb-from-program-to-print-its-stacktrace/4611112#4611112
 */
void bt_sighandler(int signum) {
	std::cerr << "Interrupt signal (" << signum << ") received." << std::endl;
    sip_abort();
}

/**
 * Check sizes of data types.
 * In the MPI version, the TAG is used to communicate information
 * The various bits needed to send information to other nodes
 * sums up to 32.
 */
void check_expected_datasizes() {

    sip::check(sizeof(int) >= 4, "Size of integer should be 4 bytes or more");
    sip::check(sizeof(double) >= 8, "Size of double should be 8 bytes or more");
    sip::check(sizeof(long long) >= 8,
            "Size of long long should be 8 bytes or more");
}

/**
 * Helper method to read args
 */
template<typename T>
static T read_from_optarg() {
    std::string str(optarg);
    std::stringstream ss(str);
    T d;
    ss >> d;
    return d;
}

/**
 * Returns the system timestamp as a string
 * @return
 */
std::string sip_timestamp(){
    time_t rawtime;
    struct tm * timeinfo;
    time (&rawtime);
    timeinfo = localtime (&rawtime);
    std::stringstream ss;
    std::string timestamp_str(asctime(timeinfo));
    return timestamp_str;
}

/**
 * Enables floating point exceptions & registers
 * signal handlers so as to print a stack trace
 * when the program exits due to failure of some kind
 */
static void setup_signal_and_exception_handlers() {
#ifdef __GLIBC__
#ifdef __GNU_LIBRARY__
#ifdef _GNU_SOURCE
    feenableexcept(FE_DIVBYZERO);
    feenableexcept(FE_OVERFLOW);
    feenableexcept(FE_INVALID);
#endif // _GNU_SOURCE
#endif // __GNU_LIBRARY__
#endif // __GLIBC__
    signal(SIGSEGV, bt_sighandler);
    signal(SIGFPE, bt_sighandler);
    signal(SIGTERM, bt_sighandler);
    signal(SIGINT, bt_sighandler);
    signal(SIGABRT, bt_sighandler);
}


/**
 * Encapsulates all the parameters the user passes into the
 * aces4 executable(s)
 */
struct Aces4Parameters{
    char * init_binary_file;
    char * init_json_file;
    char * sialx_file_dir;
    bool init_json_specified ;
    bool init_binary_specified;
    bool memory_specified;
    bool worker_memory_specified;
    bool server_memory_specified;
    std::size_t memory;
    std::size_t worker_memory;
    std::size_t server_memory;
    std::string job;
    int num_workers;
    int num_servers;
    Aces4Parameters(){
        init_binary_file = "data.dat";  // Default initialization file is data.dat
        sialx_file_dir = ".";           // Default directory for compiled sialx files is "."
        init_json_file = NULL;          // Json file, no default file
        memory = 2147483648;            // Default memory usage : 2 GB
        worker_memory = 2147483648;
        server_memory = 2147483648;
        init_json_specified = false;
        init_binary_specified = false;
        worker_memory_specified = false;
        server_memory_specified = false;
        memory_specified = false;
        job = "";
        num_workers = -1;
        num_servers = -1;
    }
};

/**
 * Prints usage of the aces4 program(s)
 * @param program_name
 */
static void print_usage(const std::string& program_name) {

    std::cerr << "Usage : " << program_name << " -d <init_data_file> -j <init_json_file> -s <sialx_files_directory> -m <max_memory_in_gigabytes>" << std::endl;
    std::cerr << "\t -d : binary initialization data file " << std::endl;
    std::cerr << "\t -j : json initialization file, use EITHER -d or -j " << std::endl;
    std::cerr << "\t -s : directory of compiled sialx files  " << std::endl;
	std::cerr << "\t -m : approx. memory to use for workers and servers. Actual usage will be more." << std::endl;
	std::cerr << "\t -w : approx. memory for workers. Actual usage will be more." << std::endl;
	std::cerr << "\t -v : approx. memory for servers. Actual usage will be more." << std::endl;
    std::cerr << "\t -q : number of workers  " << std::endl;
    std::cerr << "\t -r : number of servers  " << std::endl;
    std::cerr << "\tDefaults: data file - \"data.dat\", sialx directory - \".\", Memory : 2GB" << std::endl;
    std::cerr << "\tm is the approximate memory to use. Actual usage will be more." << std::endl;
    std::cerr << "\tworkers & servers are distributed in a 2:1 ratio for an MPI build" << std::endl;
    std::cerr << "\t-? or -h to display this usage dialogue" << std::endl;
}


/**
 * Parses the command line arguments
 * and returns all the parameters in
 * an instance of Aces4Parameters.
 * @param argc
 * @param argv
 * @return
 */
Aces4Parameters parse_command_line_parameters(int argc, char* argv[]) {
    Aces4Parameters parameters;
    // Read about getopt here : http://www.gnu.org/software/libc/manual/html_node/Getopt.html
    // d: name of .dat file.
    // j: name of json file (must specify either of d or j, not both.
    // s: directory to look for siox files
    // m: approximate memory to be used. Actual usage will be more than this.
    // w: approximate memory for workers to be used.
    // v: approximate memory for servers to be used.
    // q: number of workers
    // r: number of servers
    // h & ? are for help. They require no arguments
    const char* optString = "d:j:s:m:w:v:q:r:h?";
    int c;
    while ((c = getopt(argc, argv, optString)) != -1) {
        switch (c) {
        case 'd': {
            parameters.init_binary_file = optarg;
            parameters.init_binary_specified = true;
        }
            break;
        case 'j': {
            parameters.init_json_file = optarg;
            parameters.init_json_specified = true;
        }
            break;
        case 's': {
            parameters.sialx_file_dir = optarg;
        }
            break;
        case 'm': {
            double memory_in_gb = read_from_optarg<double>();
            parameters.memory = memory_in_gb * 1024L * 1024L * 1024L;
            parameters.memory_specified = true;
        }
            break;
        case 'w': {
            double memory_in_gb = read_from_optarg<double>();
            parameters.worker_memory = memory_in_gb * 1024L * 1024L * 1024L;
            parameters.worker_memory_specified = true;
        }
        	break;
        case 'v': {
            double memory_in_gb = read_from_optarg<double>();
            parameters.server_memory = memory_in_gb * 1024L * 1024L * 1024L;
            parameters.server_memory_specified = true;
        }
        	break;
        case 'q' : {
            parameters.num_workers = read_from_optarg<int>();
        }
            break;
        case 'r' : {
            parameters.num_servers = read_from_optarg<int>();
        }
            break;
        case 'h':
        case '?':
        default:
            std::string program_name = argv[0];
            print_usage(program_name);
            exit(1);
        }
    }

    // Error if both .dat & .json file specified
    if (parameters.init_binary_specified && parameters.init_json_specified) {
        std::cerr << "Cannot specify both init binary data file and json init file !" << std::endl;
        std::string program_name = argv[0];
        print_usage(program_name);
        exit(1);
    }

    // If memory is specified, set both worker & server memory to that value
    // If individual server or worker memory is specified, use that value
    if (parameters.memory_specified){
    	if (!parameters.worker_memory_specified)
    		parameters.worker_memory = parameters.memory;
    	if (!parameters.server_memory_specified)
    		parameters.server_memory = parameters.memory;
    }

    if (parameters.init_json_specified)
        parameters.job = parameters.init_json_file;
    else
        parameters.job = parameters.init_binary_file;

    // If number of workers & servers not specified, use the default 2:1 ratio for workers:servers
    const int default_worker_server_ratio = 2; // 2:1
    int num_processes = sip::SIPMPIAttr::comm_world_size();
    if (parameters.num_workers == -1 || parameters.num_servers == -1){
        // Option not specified. Use default
        if (num_processes < default_worker_server_ratio + 1){
            parameters.num_servers = 1;
        } else {
            parameters.num_servers = num_processes / (default_worker_server_ratio + 1);
        }
        parameters.num_workers = num_processes - parameters.num_servers;
    } else if (parameters.num_workers + parameters.num_servers != num_processes){
        std::stringstream err_ss;
        err_ss << "Number of workers (" << parameters.num_workers << ")"
                << "and number of servers (" << parameters.num_servers << ")"
                << "must add up to the number of MPI processes (" << num_processes << ")";
        sip::fail (err_ss.str());
    }


    return parameters;
}

/**
 * Reads the initialization file (json or binary)
 * and returns a new SetupReader instance allocated on the heap.
 * It is the callers responsibility to cleanup this instance.
 * @param parameters
 * @return
 */
setup::SetupReader* read_init_file(const Aces4Parameters& parameters) {
    setup::SetupReader *setup_reader = NULL;
    if (parameters.init_json_specified) {
        std::ifstream json_ifstream(parameters.init_json_file,
                std::ifstream::in);
        setup_reader = new setup::SetupReader(json_ifstream);
    } else {
        //create setup_file
        SIP_MASTER_LOG(
                std::cout << "Initializing data from " << parameters.job << std::endl);
        setup::BinaryInputFile setup_file(parameters.job); //initialize setup data
        setup_reader = new setup::SetupReader(setup_file);
    }
    setup_reader->aces_validate();
    return setup_reader;
}

/**
 * Sets the rank distribution from a given set of Aces4Parameters.
 */
void set_rank_distribution(const Aces4Parameters& parameters) {
#ifdef HAVE_MPI
    sip::ConfigurableRankDistribution *rank_distribution =
            new sip::ConfigurableRankDistribution(parameters.num_workers, parameters.num_servers);
    sip::SIPMPIAttr::set_rank_distribution(rank_distribution);
#endif
}


int main(int argc, char* argv[]) {

    check_expected_datasizes();
    setup_signal_and_exception_handlers();

#ifdef HAVE_MPI
	/* MPI Initialization */
	MPI_Init(&argc, &argv);
#endif // HAVE_MPI

    Aces4Parameters parameters = parse_command_line_parameters(argc, argv);

#ifdef HAVE_MPI    
    set_rank_distribution(parameters);
	sip::SIPMPIUtils::set_error_handler();
#endif

	sip::SIPMPIAttr &sip_mpi_attr = sip::SIPMPIAttr::get_instance(); // singleton instance.
	std::cout<<sip_mpi_attr<<std::endl;

	// Set Approx Max memory usage
	sip::GlobalState::set_max_worker_data_memory_usage(parameters.worker_memory);
	sip::GlobalState::set_max_server_data_memory_usage(parameters.server_memory);//small enough for disk back use in eom test

	//create setup_file
    setup::SetupReader* setup_reader = read_init_file(parameters);
	SIP_MASTER_LOG(std::cout << "SETUP READER DATA:\n" << setup_reader << std::endl);
	setup::SetupReader::SialProgList &progs = setup_reader->sial_prog_list();
	setup::SetupReader::SialProgList::iterator it;

//#ifdef HAVE_MPI
// opening files for timer printout
	std::string job(parameters.job);
	job.resize(job.size()-4); //remove the ".dat" from the job string
	std::ofstream server_timer_output;
	std::ofstream worker_timer_output;
//	if (sip_mpi_attr.is_company_master()) {
#ifdef HAVE_MPI
			server_timer_output.open((std::string("server_data_for_").append(job).append(".csv")).c_str());
#endif
			worker_timer_output.open((std::string("worker_data_for_").append(job).append(".csv")).c_str());
//	}
//#endif

#ifdef HAVE_MPI
	sip::ServerPersistentArrayManager persistent_server;
	sip::WorkerPersistentArrayManager persistent_worker;

	std::ostream& server_stat_os = server_timer_output;
	std::ostream& worker_stat_os = worker_timer_output;
#else
	sip::WorkerPersistentArrayManager persistent_worker;
	std::ostream& worker_stat_os = std::cout;
#endif //HAVE_MPI


	for (it = progs.begin(); it != progs.end(); ++it) {
//		sip::SimpleTimer setup;
//		setup.start();
		std::string sialfpath;
		sialfpath.append(parameters.sialx_file_dir);
		sialfpath.append("/");
		sialfpath.append(*it);

		sip::GlobalState::set_program_name(*it);
		sip::GlobalState::increment_program();

		setup::BinaryInputFile siox_file(sialfpath);
		sip::SipTables sipTables(*setup_reader, siox_file);

		SIP_MASTER_LOG(std::cout << "SIP TABLES" << '\n' << sipTables << std::endl);
		SIP_MASTER_LOG(std::cout << "Executing siox file : " << sialfpath << std::endl);


//		const std::vector<std::string> lno2name = sipTables.line_num_to_name();
#ifdef HAVE_MPI
		sip::DataDistribution data_distribution(sipTables, sip_mpi_attr);

		// TODO Broadcast from worker master to all servers & workers.
	if (sip_mpi_attr.is_server()){
		//			sip::ServerTimer server_timer(sipTables.op_table_size());
		sip::SIPServer server(sipTables, data_distribution, sip_mpi_attr, &persistent_server);
		server.run();
		SIP_LOG(std::cout<<"PBM after program at Server "<< sip_mpi_attr.global_rank()<< " : " << sialfpath << " :"<<std::endl<<persistent_server;);

			sip::MPITimer save_persistent_timer(sip_mpi_attr.company_communicator());
			sip::MPITimerList save_persistent_timers(sip_mpi_attr.company_communicator(), sipTables.num_arrays());

			save_persistent_timer.start();
			persistent_server.save_marked_arrays(&server, &save_persistent_timers);
			save_persistent_timer.pause();


		//print worker stats before barrier
		MPI_Barrier(MPI_COMM_WORLD);

		server.gather_and_print_statistics(server_stat_os);
		//print persistent array stats
		save_persistent_timer.gather();
		if(sip_mpi_attr.is_company_master()){
			server_stat_os << std::endl << "Save persistent array times" << std::endl;
			server_stat_os << save_persistent_timer << std::endl << std::flush;
		}
	} else
#endif

	//interpret current program on worker
	{

		//sip::SialxTimer sialxTimer(sipTables.max_timer_slots());
		//sip::SialxTimer sialxTimer(sipTables.op_table_size());

		sip::Interpreter runner(sipTables,  &persistent_worker);

		SIP_MASTER(std::cout << "SIAL PROGRAM OUTPUT for "<< sialfpath << " Started at " << sip_timestamp() << std::endl);

		runner.interpret();
		runner.post_sial_program();
		persistent_worker.save_marked_arrays(&runner);
		SIP_MASTER_LOG(std::cout<<"Persistent array manager at master worker after program " << sialfpath << " :"<<std::endl<< persistent_worker;)

		SIP_MASTER(std::cout << "\nSIAL PROGRAM " << sialfpath << " TERMINATED at " << sip_timestamp() << std::endl);
		runner.gather_and_print_statistics(worker_stat_os);
#ifdef HAVE_MPI
		MPI_Barrier(MPI_COMM_WORLD);
#endif
		//print server stats

		}// end of worker or server


#ifdef HAVE_MPI
		sip::SIPMPIUtils::check_err(MPI_Barrier(MPI_COMM_WORLD));
#endif
	} //end of loop over programs

#ifdef HAVE_MPI
	sip::SIPMPIAttr::cleanup(); // Delete singleton instance
	MPI_Finalize();
#endif
	return 0;
}
