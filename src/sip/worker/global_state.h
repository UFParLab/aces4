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
namespace sip {

/**
 * Maintains the global state of the SIP.
 */
class GlobalState {
public:

	static bool is_first_program() { return prog_num == 0; };

	static void increment_program() { prog_num++; };

	static int get_program_num() { return prog_num; };

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

	static void reinitialize(){
		prog_num = -1;
		prog_name = "";
		max_worker_data_memory_usage = prev_max_worker_data_memory_usage; // Default 2GB
		max_server_data_memory_usage = prev_max_worker_data_memory_usage;
	}

private:
	static int prog_num;
	static std::string prog_name;
	static std::size_t max_worker_data_memory_usage;
	static std::size_t max_server_data_memory_usage;
	static std::size_t prev_max_worker_data_memory_usage;
	static std::size_t prev_max_server_data_memory_usage;
};

} /* namespace sip */

#endif /* GLOBAL_STATE_H_ */
