/*
 * global_state.cpp
 *
 *  Created on: Jan 28, 2014
 *      Author: njindal
 */

#include <global_state.h>

namespace sip {

int GlobalState::prog_num = -1;
std::string GlobalState::prog_name = "";
size_t GlobalState::max_worker_data_memory_usage = 2147483648; // Default 2GB
size_t GlobalState::max_server_data_memory_usage = 2147483648; // Default 2GB
size_t GlobalState::prev_max_worker_data_memory_usage = 2147483648; // Default 2GB
size_t GlobalState::prev_max_server_data_memory_usage = 2147483648; // Default 2GB
} /* namespace sip */
