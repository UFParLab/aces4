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

private:
	static int prog_num;
	static std::string prog_name;
	static std::size_t max_data_memory_usage;
};

} /* namespace sip */

#endif /* GLOBAL_STATE_H_ */
