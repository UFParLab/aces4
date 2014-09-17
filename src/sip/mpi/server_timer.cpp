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
		delegate_(1 + NUMBER_TIMER_KINDS_*(sialx_lines)){}

ServerTimer::~ServerTimer() {}



//*********************************************************************
//						Multi node print
//*********************************************************************

/**
 * MPI_Reduce Reduction function for Timers
 * @param r_in
 * @param r_inout
 * @param len
 * @param type
 */
void server_timer_reduce_op_function(void* r_in, void* r_inout, int *len, MPI_Datatype *type){
	long long * in = (long long*)r_in;
	long long * inout = (long long*)r_inout;
	for (int l=0; l<*len; l++){
		long long num_timers = in[0];
		sip::check(inout[0] == in[0], "Data corruption when trying to reduce timers !");
		// Sum up the number of times each timer is switched on & off
		// Sum up the the total time spent at each line.
		in++; inout++;	// 0th position has the length
		for (int i=0; i<num_timers*2; i++){
			*inout += *in;
			in++; inout++;
		}
	}
}

/**
 * PrintTimers instance for servers.
 * Prints out total & average times of each interesting sialx line to stdout.
 * The average and total is over all servers.
 */
template<typename TIMER>
class ServerPrint : public PrintTimers<TIMER> {
public:
	ServerPrint(const std::vector<std::string> &line_to_str, int sialx_lines) :
		line_to_str_(line_to_str), sialx_lines_(sialx_lines) {}
	virtual ~ServerPrint(){}
	virtual void execute(TIMER& timer){

		mpi_reduce_timers(timer); // Reduce timers to server master.

		// Print from the server master.
		if (SIPMPIAttr::get_instance().is_company_master()){

			std::cout << "Timers for Program " << GlobalState::get_program_name() << std::endl;

			long long * timers = timer.get_timers();
			long long * timer_counts = timer.get_timer_count();
			const int LW = 10;	// Line num
			const int CW = 15;	// Time
			const int SW = 20;	// String

			assert(timer.check_timers_off());
			std::cout<<"Timers"<<std::endl
				<<std::setw(LW)<<std::left<<"Line"
				<<std::setw(SW)<<std::left<<"Type"
				<<std::setw(CW)<<std::left<<"Avg"
				<<std::setw(CW)<<std::left<<"AvgBlkWait"
				<<std::setw(CW)<<std::left<<"AvgDiskRead"
				<<std::setw(CW)<<std::left<<"AvgDiskWrite"
				<<std::setw(CW)<<std::left<<"Tot"
				<<std::endl;

			for (int i=1; i<sialx_lines_; i++){

				int tot_time_offset = i + static_cast<int>(ServerTimer::TOTALTIME) * sialx_lines_;
				if (timer_counts[tot_time_offset] > 0L){
					double tot_time = timer.to_seconds(timers[tot_time_offset]);	// Microsecond to second
					double avg_time = tot_time / timer_counts[tot_time_offset];

					double tot_blk_wait = 0;
					double avg_blk_wait = 0;
					double tot_disk_read = 0;
					double avg_disk_read = 0;
					double tot_disk_write = 0;
					double avg_disk_write = 0;

					int blk_wait_offset = i + static_cast<int>(ServerTimer::BLOCKWAITTIME) * sialx_lines_;
					if (timer_counts[blk_wait_offset] > 0L){
						tot_blk_wait = timer.to_seconds(timers[blk_wait_offset]);
						avg_blk_wait = tot_blk_wait / timer_counts[blk_wait_offset];
					}

					int read_timer_offset = i + static_cast<int>(ServerTimer::READTIME) * sialx_lines_;
					if (timer_counts[read_timer_offset] > 0L){
						tot_disk_read = timer.to_seconds(timers[read_timer_offset]);
						avg_disk_read = tot_disk_read / timer_counts[read_timer_offset];
					}

					int write_timer_offset = i + static_cast<int>(ServerTimer::WRITETIME) * sialx_lines_;
					if (timer_counts[write_timer_offset] > 0L){
						tot_disk_write = timer.to_seconds(timers[write_timer_offset]);
						avg_disk_write = tot_disk_write / timer_counts[write_timer_offset];
					}

					std::cout<<std::setw(LW)<<std::left << i
							<< std::setw(SW)<< std::left << line_to_str_.at(i)
							<< std::setw(CW)<< std::left << avg_time
							<< std::setw(CW)<< std::left << avg_blk_wait
							<< std::setw(CW)<< std::left << avg_disk_read
							<< std::setw(CW)<< std::left << avg_disk_write
							<< std::setw(CW)<< std::left << tot_time
							<< std::endl;
				}
			}
			std::cout<<std::endl;
		}
	}
private:
	const std::vector<std::string>& line_to_str_;
	const int sialx_lines_;

	/**
	 * Reduce all server timers to server master.
	 * @param timer
	 */
	void mpi_reduce_timers(TIMER& timer){
		sip::SIPMPIAttr &attr = sip::SIPMPIAttr::get_instance();
		sip::check(attr.is_server(), "Trying to reduce timer on a non-server rank !");
		long long * timers = timer.get_timers();
		long long * timer_counts = timer.get_timer_count();

		// Data to send to reduce
		long long * sendbuf = new long long[2*timer.max_slots + 1];
		sendbuf[0] = timer.max_slots;
		// The data will be structured as
		// Length of arrays 1 & 2
		// Array1 -> timer_switched_ array
		// Array2 -> timer_list_ array
		std::copy(timer_counts + 0, timer_counts + timer.max_slots, sendbuf+1);
		std::copy(timers + 0, timers + timer.max_slots, sendbuf+1+ timer.max_slots);

		long long * recvbuf = new long long[2*timer.max_slots + 1]();

		int server_master = attr.COMPANY_MASTER_RANK;
		MPI_Comm server_company = attr.company_communicator();

		MPI_Datatype server_timer_reduce_dt; // MPI Type for timer data to be reduced.
		MPI_Op server_timer_reduce_op;	// MPI OP to reduce timer data.
		SIPMPIUtils::check_err(MPI_Type_contiguous(timer.max_slots*2+1, MPI_LONG_LONG, &server_timer_reduce_dt));
		SIPMPIUtils::check_err(MPI_Type_commit(&server_timer_reduce_dt));
		SIPMPIUtils::check_err(MPI_Op_create((MPI_User_function *)server_timer_reduce_op_function, 1, &server_timer_reduce_op));

		SIPMPIUtils::check_err(MPI_Reduce(sendbuf, recvbuf, 1, server_timer_reduce_dt, server_timer_reduce_op, server_master, server_company));

		if (attr.is_company_master()){
			std::copy(recvbuf+1, recvbuf+1+timer.max_slots, timer_counts);
			std::copy(recvbuf+1+timer.max_slots, recvbuf+1+2*timer.max_slots, timers);
		}

		// Cleanup
		delete [] sendbuf;
		delete [] recvbuf;

		SIPMPIUtils::check_err(MPI_Type_free(&server_timer_reduce_dt));
		SIPMPIUtils::check_err(MPI_Op_free(&server_timer_reduce_op));
	}
};

//*********************************************************************
//				Print for TAUTimers (Fixes timer names)
//*********************************************************************
#ifdef HAVE_TAU
template<typename TIMER>
class TAUTimersPrint : public PrintTimers<TIMER> {
public:
	TAUTimersPrint(const std::vector<std::string> &line_to_str, int sialx_lines)
		:line_to_str_(line_to_str), sialx_lines_(sialx_lines) {}
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
};
#endif // HAVE_TAU




void ServerTimer::start_timer(int line_number, TimerKind_t kind){
	check(kind < NUMBER_TIMER_KINDS_, "Invalid timer type", line_number);
	check(line_number <= sialx_lines_, "Invalid line number", line_number);
	delegate_.start_timer(line_number + ((int)kind) * sialx_lines_);
}

void ServerTimer::pause_timer(int line_number, TimerKind_t kind){
	check(kind < NUMBER_TIMER_KINDS_, "Invalid timer type", line_number);
	check(line_number <= sialx_lines_, "Invalid line number", line_number);
	delegate_.pause_timer(line_number + ((int)kind) * sialx_lines_);
}


void ServerTimer::print_timers(std::vector<std::string> line_to_str) {
#ifdef HAVE_TAU
	typedef TAUTimersPrint<TimerType_t> PrintTimersType_t;
#elif defined HAVE_MPI
	typedef ServerPrint<TimerType_t> PrintTimersType_t;
#else
	typedef SingleNodePrint<TimerType_t> PrintTimersType_t;
#endif
	PrintTimersType_t p(line_to_str, sialx_lines_);
	delegate_.print_timers(p);
}

} /* namespace sip */
