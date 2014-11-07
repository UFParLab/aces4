/*
 * sipmap_main.cpp
 *
 *  Created on: Oct 9, 2014
 *      Author: njindal
 */

#include "sipmap_helper_methods.h"


int main(int argc, char* argv[]) {
	check_expected_datasizes();
    setup_signal_and_exception_handlers();
    mpi_init(&argc, &argv);
	sip::SIPMPIAttr &sip_mpi_attr = sip::SIPMPIAttr::get_instance(); // singleton instance.
	std::cout<<sip_mpi_attr<<std::endl;

	INIT_GLOBAL_TIMERS(&argc, &argv);

	Aces4Parameters parameters = parse_command_line_parameters(argc, argv);

	const int num_workers = parameters.num_workers;
	const int num_servers = parameters.num_servers;

	// Set Approx Max memory usage - Does not apply to SIPMAP
	sip::GlobalState::set_max_data_memory_usage(parameters.memory);

	setup::SetupReader* setup_reader = read_init_file(parameters);
	SIP_MASTER_LOG(std::cout << "SETUP READER DATA:\n" << setup_reader << std::endl);

	setup::SetupReader::SialProgList &progs = setup_reader->sial_prog_list();
	setup::SetupReader::SialProgList::const_iterator it;

	// Sets the database name from the parameters.
	sip::ProfileTimerStore profile_timer_store(parameters.profiledb.c_str());

	for (it = progs.begin(); it != progs.end(); ++it) {
		std::cout << it->c_str() << std::endl;
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
		//sip::DataDistribution data_distribution(sipTables, sip_mpi_attr);



		//interpret current program on worker
		{
			const sip::RemoteArrayModel::Parameters remote_array_model_parameters(10.0, 10.0); // TODO FIXME
			const sip::RemoteArrayModel remote_array_model(sipTables, remote_array_model_parameters);

			std::vector<sip::SIPMaPInterpreter::PardoSectionsInfoVector_t> pardo_sections_info_vector;
			std::vector<sip::SIPMaPTimer> sipmap_timer_vector;
			for (int worker_rank=0; worker_rank<num_workers; ++worker_rank){

				sip::SIPMaPTimer sipmap_timer(sipTables.max_timer_slots());
				sip::SIPMaPInterpreter runner(worker_rank, num_workers, sipTables, remote_array_model, profile_timer_store, sipmap_timer);
				SIP_MASTER(std::cout << "SIAL PROGRAM OUTPUT for "<< sialfpath << std::endl);
				runner.interpret();
				runner.post_sial_program();
				SIP_MASTER(std::cout << "\nSIAL PROGRAM " << sialfpath << " TERMINATED" << std::endl);
				sip::SIPMaPInterpreter::PardoSectionsInfoVector_t& pardo_sections_info =
						runner.get_pardo_section_times();

				pardo_sections_info_vector.push_back(pardo_sections_info);
				sipmap_timer_vector.push_back(sipmap_timer);
			}

			sip::SIPMaPTimer merged_timer = sip::SIPMaPInterpreter::merge_sipmap_timers(pardo_sections_info_vector, sipmap_timer_vector);
			merged_timer.print_timers(lno2name);

			sipmap_timer_vector.clear();
			pardo_sections_info_vector.clear();

		}// end of worker


		STOP_TAU_SIALX_PROGRAM_DYNAMIC_PHASE();

  		barrier();
	} //end of loop over programs

	delete setup_reader;
	FINALIZE_GLOBAL_TIMERS();
	sip::SIPMPIAttr::cleanup(); // Delete singleton instance
	mpi_finalize();

	return 0;
}



