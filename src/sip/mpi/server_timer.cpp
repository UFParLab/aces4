/*
 * server_timer.cpp
 *
 *  Created on: Jul 28, 2014
 *      Author: njindal
 */

#include <server_timer.h>
#include <iostream>
#include <iomanip>
#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <mpi.h>

#include "assert.h"
#include "sip_interface.h"
#include "print_timers.h"

#include "sip_mpi_attr.h"
#include "sip_mpi_utils.h"

#include "global_state.h"

#ifdef HAVE_TAU
#include <TAU.h>
#endif

namespace sip {

ServerTimer::ServerTimer(int sialx_lines):
		sialx_lines_(sialx_lines),
		delegate_(NUMBER_TIMER_KINDS_*(1 + sialx_lines)){
	/* Need NUMBER_TIMER_KINDS_ times the maximum number of slots,
	 * one for each kind defined in the TimerKind_t enum.
	 * One extra added since line numbers begin at 1.
	 * The 0th slot is for the entire program
	 */
}

ServerTimer::~ServerTimer() {}



//*********************************************************************
//						Multi node print
//*********************************************************************

/**
 * PrintTimers instance for servers.
 * Prints out total & average times of each interesting sialx line to stdout.
 * The average and total is over all servers.
 */
template<typename TIMER>
class ServerPrint : public PrintTimers<TIMER> {
public:
	ServerPrint(const std::vector<std::string> &line_to_str, int sialx_lines, std::ostream& out) :
		line_to_str_(line_to_str), sialx_lines_(sialx_lines), out_(out) {}
	virtual ~ServerPrint(){}
	virtual void execute(TIMER& timer){
		sip::SIPMPIAttr &attr = sip::SIPMPIAttr::get_instance();
		int num_servers = attr.num_servers();

		std::vector<long long> timers_vector(timer.max_slots(), 0L);
		std::vector<long long> timer_counts_vector(timer.max_slots(), 0L);
		sip::mpi_reduce_sip_timers(timer, timers_vector, timer_counts_vector, attr.company_communicator()); // Reduce timers to server master.

		// Print from the server master.
		if (SIPMPIAttr::get_instance().is_company_master()){

			out_ << "Timers for Program " << GlobalState::get_program_name() << std::endl;

			long long * timers = &timers_vector[0];
			long long * timer_counts = &timer_counts_vector[0];
			const int LW = 8;	// Line num
			const int CW = 12;	// Time
			const int SW = 20;	// String
			const int PRECISION = 6; 	// Precision

			out_.precision(PRECISION); // Reset precision to 6 places.

			assert(timer.check_timers_off());
			out_<<"Timers"<<std::endl
				<<std::setw(LW)<<std::left<<"Line"
				<<std::setw(SW)<<std::left<<"Type"
				<<std::setw(CW)<<std::left<<"Avg"			// Average time spent per server
				<<std::setw(CW)<<std::left<<"AvgBlkWt"		// Average block wait per server
				<<std::setw(CW)<<std::left<<"AvgDskRd"		// Average disk read per server
				<<std::setw(CW)<<std::left<<"AvgDskWrte"	// Average disk write per server
				<<std::setw(CW)<<std::left<<"AvgCount"		// Average count per server
				//<<std::setw(CW)<<std::left<<"Tot"
				<<std::endl;

			double total_wall_time = timer.to_seconds(timers[0] / (double)timer_counts[0]);
			double sum_of_avg_busy_time = 0.0;

			for (int i=1; i<sialx_lines_; i++){

				int tot_time_offset = i + static_cast<int>(ServerTimer::TOTALTIME) * sialx_lines_;
				if (timer_counts[tot_time_offset] > 0L){
					long long timer_count = timer_counts[tot_time_offset];
					double tot_time = timer.to_seconds(timers[tot_time_offset]);	// Microsecond to second
					double avg_time = tot_time / num_servers;
					double avg_count =  timer_count /(double)num_servers;
					sum_of_avg_busy_time += avg_time;

					double tot_blk_wait = 0;
					double avg_blk_wait = 0;
					double tot_disk_read = 0;
					double avg_disk_read = 0;
					double tot_disk_write = 0;
					double avg_disk_write = 0;

					int blk_wait_offset = i + static_cast<int>(ServerTimer::BLOCKWAITTIME) * sialx_lines_;
					if (timer_counts[blk_wait_offset] > 0L){
						tot_blk_wait = timer.to_seconds(timers[blk_wait_offset]);
						avg_blk_wait = tot_blk_wait / num_servers;
					}

					int read_timer_offset = i + static_cast<int>(ServerTimer::READTIME) * sialx_lines_;
					if (timer_counts[read_timer_offset] > 0L){
						tot_disk_read = timer.to_seconds(timers[read_timer_offset]);
						avg_disk_read = tot_disk_read / num_servers;
					}

					int write_timer_offset = i + static_cast<int>(ServerTimer::WRITETIME) * sialx_lines_;
					if (timer_counts[write_timer_offset] > 0L){
						tot_disk_write = timer.to_seconds(timers[write_timer_offset]);
						avg_disk_write = tot_disk_write / num_servers;
					}

					out_<<std::setw(LW)<<std::left << i
							<< std::setw(SW)<< std::left << line_to_str_.at(i)
							<< std::setw(CW)<< std::left << avg_time
							<< std::setw(CW)<< std::left << avg_blk_wait
							<< std::setw(CW)<< std::left << avg_disk_read
							<< std::setw(CW)<< std::left << avg_disk_write
							<< std::setw(CW)<< std::left << avg_count
							//<< std::setw(CW)<< std::left << tot_time
							<< std::endl;
				}
			}
			out_ << "Total Walltime : " << total_wall_time << " Seconds "<< std::endl;
			out_ << "Average Busy Time : " << sum_of_avg_busy_time << " Seconds " << std::endl;
			out_ << "Average Idle Time : " << (total_wall_time - sum_of_avg_busy_time) << " Seconds " << std::endl;
			out_<<std::endl;
		}
	}
private:
	const std::vector<std::string>& line_to_str_;
	const int sialx_lines_;
	std::ostream& out_;
};

//*********************************************************************
//				Print for TAUTimers (Fixes timer names)
//*********************************************************************
#ifdef HAVE_TAU
template<typename TIMER>
class TAUTimersPrint : public PrintTimers<TIMER> {
public:
	TAUTimersPrint(const std::vector<std::string> &line_to_str, int sialx_lines, std::ostream& out)
		:line_to_str_(line_to_str), sialx_lines_(sialx_lines), out_(out) {}
	virtual ~TAUTimersPrint() {/* Do Nothing */}
	virtual void execute(TIMER& timer) {
		void ** tau_timers = timer.get_tau_timers();

		std::vector<std::string>::const_iterator it = line_to_str_.begin();
		for (int line_num = 0; it!= line_to_str_.end(); ++it, ++line_num){
			const std::string &line_str = *it;
			if (line_str != ""){

				int total_time_timer_offset = line_num + sialx_lines_ * static_cast<int>(ServerTimer::TOTALTIME);
				if (tau_timers[total_time_timer_offset] != NULL){
					// Set total time string
					std::stringstream tot_sstr;
					tot_sstr << sip::GlobalState::get_program_num() << ": " << line_num << ": "  << " Total " << line_str;
					const char *tau_string = tot_sstr.str().c_str();
					TAU_PROFILE_TIMER_SET_NAME(tau_timers[total_time_timer_offset], tau_string);
				}

				int block_wait_timer_offset = line_num + sialx_lines_ * static_cast<int>(ServerTimer::BLOCKWAITTIME);
				if (tau_timers[block_wait_timer_offset] != NULL){
					// Set block wait time string
					std::stringstream blkw_sstr;
					blkw_sstr << sip::GlobalState::get_program_num() << ":" << line_num <<":" << " Blkwait " << line_str ;
					const char *tau_string = blkw_sstr.str().c_str();
					TAU_PROFILE_TIMER_SET_NAME(tau_timers[block_wait_timer_offset], tau_string);
				}

				int read_timer_offset = line_num + sialx_lines_ * static_cast<int>(ServerTimer::READTIME);
				if (tau_timers[read_timer_offset] != NULL){
					// Set disk read time string
					std::stringstream readd_sstr;
					readd_sstr << sip::GlobalState::get_program_num() << ":" << line_num <<":" << " ReadDisk " << line_str ;
					const char *tau_string = readd_sstr.str().c_str();
					TAU_PROFILE_TIMER_SET_NAME(tau_timers[read_timer_offset], tau_string);
				}

				int write_timer_offset = line_num + sialx_lines_ * static_cast<int>(ServerTimer::WRITETIME);
				if (tau_timers[write_timer_offset] != NULL){
					// Set disk write time string
					std::stringstream writed_sstr;
					writed_sstr << sip::GlobalState::get_program_num() << ":" << line_num <<":" << " WriteDisk " << line_str ;
					const char *tau_string = writed_sstr.str().c_str();
					TAU_PROFILE_TIMER_SET_NAME(tau_timers[write_timer_offset], tau_string);
				}

			}
		}
	}

private:
	const std::vector<std::string>& line_to_str_;
	const int sialx_lines_;
	std::ostream& out_;
};
#endif // HAVE_TAU




void ServerTimer::start_timer(int line_number, TimerKind_t kind){
	CHECK_WITH_LINE(kind < NUMBER_TIMER_KINDS_, "Invalid timer type", line_number);
	CHECK_WITH_LINE(line_number <= sialx_lines_, "Invalid line number", line_number);
	delegate_.start_timer(line_number + ((int)kind) * sialx_lines_);
}

void ServerTimer::pause_timer(int line_number, TimerKind_t kind){
	CHECK_WITH_LINE(kind < NUMBER_TIMER_KINDS_, "Invalid timer type", line_number);
	CHECK_WITH_LINE(line_number <= sialx_lines_, "Invalid line number", line_number);
	delegate_.pause_timer(line_number + ((int)kind) * sialx_lines_);
}


void ServerTimer::print_timers(std::vector<std::string> line_to_str, std::ostream& out) {
#ifdef HAVE_TAU
	typedef TAUTimersPrint<SipTimer_t> PrintTimersType_t;
#elif defined HAVE_MPI
	typedef ServerPrint<SipTimer_t> PrintTimersType_t;
#endif
	PrintTimersType_t p(line_to_str, sialx_lines_, out);
	delegate_.print_timers(p);
}

void ServerTimer::start_program_timer(){
#ifndef HAVE_TAU
	delegate_.start_timer(0);
#endif
}

void ServerTimer::stop_program_timer(){
#ifndef HAVE_TAU
	delegate_.pause_timer(0);
#endif
}


} /* namespace sip */
