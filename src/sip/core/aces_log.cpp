/*
 * aces_log.cpp
 *
 *  Created on: Nov 2, 2015
 *      Author: basbas
 */

#include "aces_log.h"
#include <iostream>
#include <fstream>
#include <istream>
#include <string>

namespace sip {

AcesLog::AcesLog(std::string jobid, bool is_restart) :
				filename_(jobid + ".log"), log_file_(NULL) {
	if (jobid == "") return;
	SIPMPIAttr& mpi_attr = SIPMPIAttr::get_instance();
	if (is_restart) { //all processes open file in read-only mode
			log_file_ = new std::fstream(filename_.c_str(), std::fstream::in);
			CHECK(log_file_->good(), "failure opening aces_log file " + filename_ +" for restart job id " + jobid);
	} else { //this is a new file, server master opens the file in write mode and intializes with value 0.
		if (mpi_attr.is_company_master() && mpi_attr.is_server()) {
			log_file_ = new std::fstream(filename_.c_str(), std::fstream::out | std::fstream::trunc);
			CHECK(log_file_->good(), "failure opening aces_log file " + filename_ + " for job id " + jobid);
		}
	}
}

AcesLog::~AcesLog() {
	if (is_open()) {
		log_file_->close();
		delete log_file_;
	}
}

void AcesLog::write_prog_num(int prog_num) {
	if (!is_open()) return;
	log_file_->seekp(0);
	*log_file_ << prog_num << std::endl << std::flush;
}

int AcesLog::read_prog_num() {
	int val;
	log_file_->seekg(0) >> val;
	return val;
}

} /* namespace sip */
