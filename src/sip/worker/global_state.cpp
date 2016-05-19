/*
 * global_state.cpp
 *
 *  Created on: Jan 28, 2014
 *      Author: njindal
 */

#include <global_state.h>

namespace sip {

//int JobControl::global->prog_num = -1;
int JobControl::global->prog_num = 0;
std::string JobControl::global->prog_name = "";
size_t JobControl::global->max_worker_data_memory_usage = 2147483648; // Default 2GB
size_t JobControl::global->max_server_data_memory_usage = 2147483648; // Default 2GB
size_t JobControl::global->prev_max_worker_data_memory_usage = 2147483648; // Default 2GB
size_t JobControl::global->prev_max_server_data_memory_usage = 2147483648; // Default 2GB
std::string JobControl::global->job_id = "";
std::string JobControl::global->restart_id = "";



std::ostream& operator<<(std::ostream& os, const GlobalState & dummy){
	os << "*************************************" << std::endl;
    os << "job_id: " << JobControl::global->job_id << std::endl;
    os << "restart_id: " << JobControl::global->restart_id << std::endl;
    os << "prog_name: " << JobControl::global->prog_name << std::endl;
    os << "prog_num: " << JobControl::global->prog_num << std::endl;
    os << "global_rank: " << SIPMPIAttr::get_instance().global_rank() << std::endl;
    os << "company_rank: "<< SIPMPIAttr::get_instance().company_rank() << std::endl;
    os << "role: " << (SIPMPIAttr::get_instance().is_server() ? "server" : "worker") << std::endl;
    os << "**************************************" << std::endl;
    return os;
}



} /* namespace sip */
