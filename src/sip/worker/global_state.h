/*
 * global_state.h
 *
 *  Created on: Jan 28, 2014
 *      Author: njindal
 */

#ifndef GLOBAL_STATE_H_
#define GLOBAL_STATE_H_

#include <string>
#include <iostream>
#include <ctime>
#include <locale>

#include "sip.h"
#ifdef HAVE_MPI
#include "sip_mpi_attr.h"
#endif

namespace sip {

/**
 * Maintains the global state of the SIP.
 */
class GlobalState {
public:

	static bool is_first_program() { return prog_num == 0; };

	static void increment_program() {
		prog_num++;
		std::cerr << "+++++++++++incremented program number++++++++  " << prog_num << std::endl << std::flush;};

	static int get_program_num() { return prog_num; };
	static void set_program_num(int num) { prog_num = num; }

	static void set_program_name(std::string s) { prog_name = s; }

	static std::string get_program_name() { return prog_name; }

	static void reset_program_count() { prog_num = 0; }

	static void set_max_worker_data_memory_usage(std::size_t m) {
		prev_max_worker_data_memory_usage = max_worker_data_memory_usage;
		max_worker_data_memory_usage = m;
	}
	static void set_max_server_data_memory_usage(std::size_t m) {
		prev_max_server_data_memory_usage = max_server_data_memory_usage;
		max_server_data_memory_usage = m;
	}

	static std::size_t get_max_worker_data_memory_usage() {  return max_worker_data_memory_usage; }
	static std::size_t get_max_server_data_memory_usage() {  return max_server_data_memory_usage; }
	static std::string set_job_id(){
			size_t bytes;
			char idstr[30];
			std::time_t t = std::time(NULL);
			bytes = std::strftime(idstr, sizeof(idstr), "_%Y%b%d_%H%M%S", std::localtime(&t));
			CHECK(bytes > 0, "error initializing job_id");
#ifdef HAVE_MPI
			//all procs need to have same value, use rank 0's value
		int err = MPI_Bcast(idstr, bytes+1, MPI_CHAR, 0, MPI_COMM_WORLD);
		CHECK(err == MPI_SUCCESS, "failure broadcasting job_id");
#endif
		job_id = idstr;
		return job_id;
	}


	/**
	 * Set the job id to the given value
	 *
	 * This is used in testing.  Normally, the job id should be calculated from the
	 * time stamp using the parameterless version of this method.
	 *
	 * @param the_job_id
	 * @return
	 */
	static std::string set_job_id(std::string the_job_id){
		job_id = the_job_id;
		return job_id;
	}

	static std::string get_job_id(){return job_id;}

	static void set_restart_id(std::string the_restart_id){
		restart_id = the_restart_id;
	}
	static std::string get_restart_id(){return restart_id;}

	static void reinitialize(){
		prog_num = 0;
		prog_name = "";
		max_worker_data_memory_usage = 2147483648; // Default 2GB
		max_server_data_memory_usage = 2147483648; // Default 2GB
		prev_max_worker_data_memory_usage = 2147483648; // Default 2GB
		prev_max_server_data_memory_usage = 2147483648; // Default 2GB
		job_id = "";
		restart_id = "";
	}

	friend std::ostream& operator <<(std::ostream&, const GlobalState&);


private:
	static int prog_num;
	static std::string prog_name;
	static std::size_t max_worker_data_memory_usage;
	static std::size_t max_server_data_memory_usage;
	static std::size_t prev_max_worker_data_memory_usage;
	static std::size_t prev_max_server_data_memory_usage;
	static std::string job_id;
	static std::string restart_id;
};

} /* namespace sip */

#endif /* GLOBAL_STATE_H_ */
