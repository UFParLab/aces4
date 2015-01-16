/*
 * global_state.h
 *
 *  Created on: Jan 28, 2014
 *      Author: njindal
 */

#ifndef GLOBAL_STATE_H_
#define GLOBAL_STATE_H_

#include <string>

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

	static void set_max_data_memory_usage(std::size_t m) { max_data_memory_usage = m; }

	static std::size_t get_max_data_memory_usage() { return max_data_memory_usage; }

	static int get_default_worker_server_ratio() { return default_worker_server_ratio; }

	static void reinitialize(){
		prog_num = -1;
		prog_name = "";
		max_data_memory_usage = 2147483648; // Default 2GB
	}

private:
	static int prog_num;
	static std::string prog_name;
	static std::size_t max_data_memory_usage;
	static int default_worker_server_ratio;
};

} /* namespace sip */

#endif /* GLOBAL_STATE_H_ */
