/*
 * sialx_print_timers.h
 *
 *  Created on: Oct 31, 2014
 *      Author: njindal
 */

#ifndef SIALX_PRINT_TIMER_H_
#define SIALX_PRINT_TIMER_H_

#include "config.h"
#include "sip_interface.h"
#include "global_state.h"
#include "print_timers.h"
#include "sialx_timer.h"
#include "sip_mpi_attr.h"
#include <cassert>
#include <iostream>
#include <iomanip>
#include <cstdio>
#include <cstdlib>
#include <sstream>

#ifdef HAVE_MPI
#include <mpi.h>
#include "sip_mpi_utils.h"
#endif

#ifdef HAVE_TAU
#include <TAU.h>
#endif


#ifdef HAVE_MPI
/**
 * MPI_Reduce Reduction function for Timers
 * @param r_in
 * @param r_inout
 * @param len
 * @param type
 */
void sialx_timer_reduce_op_function(void* r_in, void* r_inout, int *len, MPI_Datatype *type);
#endif // HAVE_MPI


namespace sip {


//*********************************************************************
//						Single node print
//*********************************************************************
/**
 * Single node version of PrintTimers instance for aces4
 * Prints out total & average times of each interesting sialx line to stdout
 */
template<typename TIMER>
class SingleNodePrint : public PrintTimers<TIMER> {
public:
	SingleNodePrint(const std::vector<std::string> &line_to_str, int sialx_lines, std::ostream& out)
		: line_to_str_(line_to_str), sialx_lines_(sialx_lines), out_(out) {}
	virtual ~SingleNodePrint(){}
	virtual void execute(TIMER& timer){

		out_ << "Timers for Program " << GlobalState::get_program_name() << std::endl;
		long long * timers = timer.get_timers();
		long long * timer_counts = timer.get_timer_count();
		const int LW = 10;	// Line num
		const int CW = 15;	// Time
		const int SW = 30;	// String

		assert(timer.check_timers_off());
		out_ <<"Timers"<<std::endl
			<<std::setw(LW)<<std::left<<"Line"
			<<std::setw(SW)<<std::left<<"Type"
			<<std::setw(CW)<<std::left<<"Avg"
			<<std::setw(CW)<<std::left<<"AvgBlkWait"
			<<std::setw(CW)<<std::left<<"Tot"
			<<std::setw(CW)<<std::left<<"TotBlkWait"
			<<std::setw(CW)<<std::left<<"Count"
			<<std::endl;

		int total_time_timer_offset = static_cast<int>(SialxTimer::TOTALTIME);
		int block_wait_timer_offset = static_cast<int>(SialxTimer::BLOCKWAITTIME);

		out_.precision(6); // Reset precision to 6 places.
		for (int i=1; i<timer.max_slots() - sialx_lines_; i++){
			if (timer_counts[i + total_time_timer_offset * sialx_lines_] > 0L){
				double tot_time = timer.to_seconds(timers[i + total_time_timer_offset * sialx_lines_]);	// Microsecond to second
				long count = timer_counts[i + total_time_timer_offset * sialx_lines_];
				double avg_time = tot_time / count;
				double tot_blk_wait = 0;
				double avg_blk_wait = 0;

				if (timer_counts[i + block_wait_timer_offset * sialx_lines_] > 0L){
					tot_blk_wait = timer.to_seconds(timers[i + block_wait_timer_offset * sialx_lines_]);
					avg_blk_wait = tot_blk_wait / timer_counts[i + block_wait_timer_offset * sialx_lines_];
				}

				out_<<std::setw(LW)<<std::left << i
						<< std::setw(SW)<< std::left << line_to_str_.at(i)
						<< std::setw(CW)<< std::left << avg_time
						<< std::setw(CW)<< std::left << avg_blk_wait
						<< std::setw(CW)<< std::left << tot_time
						<< std::setw(CW)<< std::left << tot_blk_wait
						<< std::setw(CW)<< std::left << count
						<< std::endl;
			}
		}
		out_ << std::endl;
	}
private:
	const std::vector<std::string>& line_to_str_;
	const int sialx_lines_;
	std::ostream& out_;
};






//*********************************************************************
//						Multi node print
//*********************************************************************

#ifdef HAVE_MPI

/**
 * PrintTimers instance for multi node version of aces4.
 * Prints out total & average times of each interesting sialx line to stdout.
 * The average and total is over all workers.
 */
template<typename TIMER>
class MultiNodePrint : public PrintTimers<TIMER> {
public:
	MultiNodePrint(const std::vector<std::string> &line_to_str, int sialx_lines, std::ostream& out) :
		line_to_str_(line_to_str), sialx_lines_(sialx_lines), out_(out) {}
	virtual ~MultiNodePrint(){}
	virtual void execute(TIMER& timer){

		mpi_reduce_timers(timer);

		if (SIPMPIAttr::get_instance().is_company_master()){
			out_ << "Timers for Program " << GlobalState::get_program_name() << std::endl;
			long long * timers = timer.get_timers();
			long long * timer_counts = timer.get_timer_count();
			const int LW = 10;	// Line num
			const int CW = 15;	// Time
			const int SW = 30;	// String

			assert(timer.check_timers_off());
			out_<<"Timers"<<std::endl
				<<std::setw(LW)<<std::left<<"Line"
				<<std::setw(SW)<<std::left<<"Type"
				<<std::setw(CW)<<std::left<<"Avg"
				<<std::setw(CW)<<std::left<<"AvgBlkWait"
				<<std::setw(CW)<<std::left<<"Tot"
				<<std::setw(CW)<<std::left<<"TotBlkWait"
				<<std::setw(CW)<<std::left<<"Count"
				<<std::endl;

			int total_time_timer_offset = static_cast<int>(SialxTimer::TOTALTIME);
			int block_wait_timer_offset = static_cast<int>(SialxTimer::BLOCKWAITTIME);

			out_.precision(6); // Reset precision to 6 places.
			for (int i=1; i<timer.max_slots() - sialx_lines_; i++){

				if (timer_counts[i + total_time_timer_offset * sialx_lines_] > 0L){
					double tot_time = timer.to_seconds(timers[i + total_time_timer_offset * sialx_lines_]);	// Microsecond to second
					long count = timer_counts[i + total_time_timer_offset * sialx_lines_];
					double avg_time = tot_time / count;
					double tot_blk_wait = 0;
					double avg_blk_wait = 0;

					if (timer_counts[i + block_wait_timer_offset * sialx_lines_] > 0L){
						tot_blk_wait = timer.to_seconds(timers[i + block_wait_timer_offset * sialx_lines_]);
						avg_blk_wait = tot_blk_wait / timer_counts[i + block_wait_timer_offset * sialx_lines_];
					}
					out_<<std::setw(LW)<<std::left << i
							<< std::setw(SW)<< std::left << line_to_str_.at(i)
							<< std::setw(CW)<< std::left << avg_time
							<< std::setw(CW)<< std::left << avg_blk_wait
							<< std::setw(CW)<< std::left << tot_time
							<< std::setw(CW)<< std::left << tot_blk_wait
							<< std::setw(CW)<< std::left << count
							<< std::endl;
				}
			}
			out_<<std::endl;
		}
	}
private:
	const std::vector<std::string>& line_to_str_;
	const int sialx_lines_;
	std::ostream& out_;
	void mpi_reduce_timers(TIMER& timer){
		sip::SIPMPIAttr &attr = sip::SIPMPIAttr::get_instance();
		sip::check(attr.is_worker(), "Trying to reduce timer on a non-worker rank !");
		long long * timers = timer.get_timers();
		long long * timer_counts = timer.get_timer_count();

		// Data to send to reduce
		long long * sendbuf = new long long[2*timer.max_slots() + 1];
		sendbuf[0] = timer.max_slots();
		std::copy(timer_counts + 0, timer_counts + timer.max_slots(), sendbuf+1);
		std::copy(timers + 0, timers + timer.max_slots(), sendbuf+1+ timer.max_slots());

		long long * recvbuf = new long long[2*timer.max_slots() + 1]();

		int worker_master = attr.worker_master();
		MPI_Comm worker_company = attr.company_communicator();

		// The data will be structured as
		// Length of arrays 1 & 2
		// Array1 -> timer_switched_ array
		// Array2 -> timer_list_ array
		MPI_Datatype sialx_timer_reduce_dt; // MPI Type for timer data to be reduced.
		MPI_Op sialx_timer_reduce_op;	// MPI OP to reduce timer data.
		SIPMPIUtils::check_err(MPI_Type_contiguous(timer.max_slots()*2+1, MPI_LONG_LONG, &sialx_timer_reduce_dt));
		SIPMPIUtils::check_err(MPI_Type_commit(&sialx_timer_reduce_dt));
		SIPMPIUtils::check_err(MPI_Op_create((MPI_User_function *)sialx_timer_reduce_op_function, 1, &sialx_timer_reduce_op));

		SIPMPIUtils::check_err(MPI_Reduce(sendbuf, recvbuf, 1, sialx_timer_reduce_dt, sialx_timer_reduce_op, worker_master, worker_company));

		if (attr.is_company_master()){
			std::copy(recvbuf+1, recvbuf+1+timer.max_slots(), timer_counts);
			std::copy(recvbuf+1+timer.max_slots(), recvbuf+1+2*timer.max_slots(), timers);
		}

		// Cleanup
		delete [] sendbuf;
		delete [] recvbuf;

		SIPMPIUtils::check_err(MPI_Type_free(&sialx_timer_reduce_dt));
		SIPMPIUtils::check_err(MPI_Op_free(&sialx_timer_reduce_op));
	}
};
#endif // HAVE_MPI






//*********************************************************************
//				Print for TAUTimers (Fixes timer names)
//*********************************************************************
#ifdef HAVE_TAU
template<typename TIMER>
class TAUTimersPrint : public PrintTimers<TIMER> {
public:
	TAUTimersPrint(const std::vector<std::string> &line_to_str, int sialx_lines, std::ostream& ignore)
		:line_to_str_(line_to_str), sialx_lines_(sialx_lines) {}
	virtual ~TAUTimersPrint() {/* Do Nothing */}
	virtual void execute(TIMER& timer) {
		void ** tau_timers = timer.get_tau_timers();

		std::vector<std::string>::const_iterator it = line_to_str_.begin();
		for (int line_num = 0; it!= line_to_str_.end(); ++it, ++line_num){
			const std::string &line_str = *it;
			if (line_str != ""){

				int total_time_timer_offset = line_num + sialx_lines_ * static_cast<int>(SialxTimer::TOTALTIME);
				if (tau_timers[total_time_timer_offset] != NULL){
					// Set total time string
					std::stringstream tot_sstr;
					tot_sstr << sip::GlobalState::get_program_num() << ": " << line_num << ": "  << " Total " << line_str;
					const char *tau_string = tot_sstr.str().c_str();
					TAU_PROFILE_TIMER_SET_NAME(tau_timers[total_time_timer_offset], tau_string);
				}

				int block_wait_timer_offset = line_num + sialx_lines_ * static_cast<int>(SialxTimer::BLOCKWAITTIME);
				if (tau_timers[block_wait_timer_offset] != NULL){
					// Set block wait time string
					std::stringstream blkw_sstr;
					blkw_sstr << sip::GlobalState::get_program_num() << ":" << line_num <<":" << " Blkwait " << line_str ;
					const char *tau_string = blkw_sstr.str().c_str();
					TAU_PROFILE_TIMER_SET_NAME(tau_timers[block_wait_timer_offset], tau_string);
				}

			}
		}
	}

private:
	const std::vector<std::string>& line_to_str_;
	const int sialx_lines_;

};
#endif // HAVE_TAU


} /* namespace sip */

#endif /* SIALX_PRINT_TIMER_H_ */
