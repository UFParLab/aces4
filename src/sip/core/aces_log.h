/*
 * aces_log.h
 *
 *  Created on: Nov 2, 2015
 *      Author: basbas
 */

#ifndef ACES_LOG_H_
#define ACES_LOG_H_


#include <iostream>
#include <fstream>
#include "sip_mpi_attr.h"

namespace sip {

/**
 * Encapsulates log file for restart from the beginning of a sial program.
 *
 * The name is <jobid>.log
 *
 * During construction, a 0 is written to the file, which should contain the program
 * number of the next sial program.  In other words, the program can be restarted with
 * any program number at most the one in the file.
 *
 * In parallel executions, the server master handles the log.
 */
class AcesLog {
public:

/**
 * Opens a log file for the job.
 *
 * @param jobid
 * @param is_restart
 */
	explicit AcesLog(std::string jobid, bool is_restart);

	//closes the log file
	~AcesLog();

	/** Updates the sial program number
	 *
	 * PRECONDITION:  is_open()
	 *
	 * @param prog_num
	 */
	void write_prog_num(int prog_num);

	/**
	 * Reads the value in the log.  This is the value of the next sial program.
	 *
	 * The job can be restarted by setting the sial program number to any non-negative
	 * value at most this one.
	 *
	 * PRECONDITION: is_open();
	 */
	int read_prog_num();

	bool is_open(){
		return log_file_ != NULL && log_file_->is_open();
	}

private:
	std::fstream* log_file_;
	std::string filename_;



};

} /* namespace sip */

#endif /* ACES_LOG_H_ */
