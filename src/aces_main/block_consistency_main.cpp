/*
 * block_consistency_main.cpp
 *
 *  Created on: Mar 28, 2015
 *      Author: njindal
 */

#include "block_consistency_helper_methods.h"


int main(int argc, char* argv[]) {
	check_expected_datasizes();
    setup_signal_and_exception_handlers();
    mpi_init(&argc, &argv);

    // Since this executable need not be run with servers
    // or with 2 ranks, all ranks are made to be workers.
    sip::AllWorkerRankDistribution all_workers_rank_dist;
    sip::SIPMPIAttr::set_rank_distribution(&all_workers_rank_dist);
	sip::SIPMPIAttr &sip_mpi_attr = sip::SIPMPIAttr::get_instance(); // singleton instance.

	//INIT_GLOBAL_TIMERS(&argc, &argv);

	Aces4Parameters parameters = parse_command_line_parameters(argc, argv);

	const int num_workers = parameters.num_workers;

	setup::SetupReader* setup_reader = read_init_file(parameters);
	SIP_MASTER_LOG(std::cout << "SETUP READER DATA:\n" << setup_reader << std::endl);

	setup::SetupReader::SialProgList &progs = setup_reader->sial_prog_list();
	setup::SetupReader::SialProgList::const_iterator it;

	for (it = progs.begin(); it != progs.end(); ++it) {
		std::cout << it->c_str() << std::endl;
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

		sip::BlockConsistencyInterpreter runner(num_workers, sipTables);
		runner.interpret();
		runner.post_sial_program();

		sip::Counter::clear_list();
		sip::MaxCounter::clear_list();
		sip::Timer::clear_list();

  		barrier();
	} //end of loop over programs

	delete setup_reader;
	sip::SIPMPIAttr::cleanup(); // Delete singleton instance
	mpi_finalize();

	return 0;
}






