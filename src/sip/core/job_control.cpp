/*
 * job_control.cpp
 *
 *  Created on: Nov 7, 2015
 *      Author: basbas
 */

#include "job_control.h"
#include "sip_mpi_attr.h"
#include "sip.h"
#include <iostream>

namespace sip {


JobControl::JobControl(std::string job_id):
			max_worker_data_memory_usage_(default_max_worker_data_memory_usage),
			max_server_data_memory_usage_(default_max_server_data_memory_usage),
			prog_name_(""),
			job_id_(job_id),
			restart_id_(""),
			prog_num_(0){
}

JobControl::JobControl(std::string job_id, std::string restart_id, int prog_num):
        max_worker_data_memory_usage_(default_max_worker_data_memory_usage),
		max_server_data_memory_usage_(default_max_server_data_memory_usage),
		prog_name_(""),
		job_id_(job_id),
		restart_id_(restart_id),
		prog_num_(prog_num){
}

JobControl::JobControl(std::string job_id,
		std::size_t max_worker_data_memory_usage,
		std::size_t max_server_data_memory_usage):
					max_worker_data_memory_usage_(max_worker_data_memory_usage),
					max_server_data_memory_usage_(max_server_data_memory_usage),
					prog_name_(""),
					job_id_(job_id),
					restart_id_(""),
					prog_num_(0){
}


JobControl::JobControl(std::string job_id, std::string restart_id, int prog_num,
		std::size_t max_worker_data_memory_usage,
		std::size_t max_server_data_memory_usage):
					max_worker_data_memory_usage_(default_max_worker_data_memory_usage),
					max_server_data_memory_usage_(default_max_server_data_memory_usage),
					prog_name_(""),
					job_id_(job_id),
					restart_id_(restart_id),
					prog_num_(prog_num){

}

JobControl::~JobControl(){}

std::string JobControl::make_job_id(){
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
//		job_id_ = idstr;
	return idstr;
}

std::ostream& operator <<(std::ostream& os, const JobControl& obj){
	os << "*************************************" << std::endl;
    os << "job_id: " << obj.job_id_ << std::endl;
    os << "restart_id: " << obj.restart_id_ << std::endl;
    os << "prog_name: " << obj.prog_name_ << std::endl;
    os << "prog_num: " << obj.prog_num_ << std::endl;
    os << "global_rank: " << SIPMPIAttr::get_instance().global_rank() << std::endl;
    os << "company_rank: "<< SIPMPIAttr::get_instance().company_rank() << std::endl;
    os << "role: " << (SIPMPIAttr::get_instance().is_server() ? "server" : "worker") << std::endl;
    os << "**************************************" << std::endl;
    return os;
}

JobControl* JobControl::global = NULL;  //the globally accessible object.

} /* namespace sip */
