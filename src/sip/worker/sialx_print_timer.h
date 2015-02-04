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
		long long * timers = timer.get_timers();
		long long * timer_counts = timer.get_timer_count();
		out_ << "Timers for Program " << GlobalState::get_program_name() << std::endl;

		const int LW = 10;	// Line num
		const int CW = 15;	// Time
		const int SW = 30;	// String

		assert(timer.check_timers_off());
		out_<<std::setw(LW)<<std::left<<"Line"
			<<std::setw(SW)<<std::left<<"Type"
			<<std::setw(CW)<<std::left<<"Tot"
			//<<std::setw(CW)<<std::left<<"TotBlkWait"
			<<std::setw(CW)<<std::left<<"AvgPerCall"
			//<<std::setw(CW)<<std::left<<"AvgBlkWait"
			<<std::setw(CW)<<std::left<<"Count"
			<<std::endl;

		int total_time_timer_offset = static_cast<int>(SialxTimer::TOTALTIME);
		int block_wait_timer_offset = static_cast<int>(SialxTimer::BLOCKWAITTIME);

		out_.precision(6); // Reset precision to 6 places.
		for (int i=1; i<=sialx_lines_; i++){
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
						<< std::setw(CW)<< std::left << tot_time
						//<< std::setw(CW)<< std::left << tot_blk_wait
						<< std::setw(CW)<< std::left << avg_time
						//<< std::setw(CW)<< std::left << avg_blk_wait
						<< std::setw(CW)<< std::left << count
						<< std::endl;
			}
		}
		out_ << "Total Walltime : " << timer.to_seconds(timers[0] / (double)timer_counts[0]) << " Seconds "<< std::endl;
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
		long long * timers = timer.get_timers();
		long long * timer_counts = timer.get_timer_count();
		printTable(timer, timers, timer_counts);
	}

protected:
	const std::vector<std::string>& line_to_str_;
	const int sialx_lines_;
	std::ostream& out_;

	void printTable(TIMER& timer, long long * timers, long long * timer_counts) {

		out_ << "Timers for Program " << GlobalState::get_program_name() << std::endl;
		const int LW = 8;			// Line num
		const int CW = 12;			// Time
		const int SW = 25;			// String
		const int PRECISION = 6; 	// Precision

		out_.precision(PRECISION); // Reset precision to 6 places.

		assert(timer.check_timers_off());
		out_<<std::setw(LW)<<std::left<<"Line"			// Line Number
			<<std::setw(SW)<<std::left<<"Type"			// Type of instruction
			<<std::setw(CW)<<std::left<<"Tot"			// Time spent
			<<std::setw(CW)<<std::left<<"TotBlkWt"		// Block wait
			<<std::setw(CW)<<std::left<<"Count"			// Counts
			<<std::setw(CW)<<std::left<<"PerCall"		// Average time per call
			<<std::setw(CW)<<std::left<<"BlkWtPC"		// Average block wait per call
			<<std::endl;

		int total_time_timer_offset = static_cast<int>(SialxTimer::TOTALTIME);
		int block_wait_timer_offset = static_cast<int>(SialxTimer::BLOCKWAITTIME);

		double sum_of_block_wait_time = 0.0;
		double total_wall_time = timer.to_seconds(timers[0] / (double)timer_counts[0]);

		for (int i=1; i<=sialx_lines_; i++){

			if (timer_counts[i + total_time_timer_offset * sialx_lines_] > 0L){
				double tot_time = timer.to_seconds(timers[i + total_time_timer_offset * sialx_lines_]);	// Microsecond to second
				long count = timer_counts[i + total_time_timer_offset * sialx_lines_];
				double per_call = tot_time / count;
				double tot_blk_wait = 0;
				double blk_wait_per_call = 0;

				if (timer_counts[i + block_wait_timer_offset * sialx_lines_] > 0L){
					tot_blk_wait = timer.to_seconds(timers[i + block_wait_timer_offset * sialx_lines_]);
					blk_wait_per_call = tot_blk_wait / timer_counts[i + block_wait_timer_offset * sialx_lines_];
					sum_of_block_wait_time += tot_blk_wait;
				}
				out_	<< std::setw(LW)<< std::left << i
						<< std::setw(SW)<< std::left << line_to_str_.at(i)
						<< std::setw(CW)<< std::left << tot_time
						<< std::setw(CW)<< std::left << tot_blk_wait
						<< std::setw(CW)<< std::left << count
						<< std::setw(CW)<< std::left << per_call
						<< std::setw(CW)<< std::left << blk_wait_per_call
						<< std::endl;
			}
		}
		out_ << "Total Walltime : " << total_wall_time << " Seconds "<< std::endl;
		out_ << "Total Block Wait time : " << sum_of_block_wait_time << " Seconds " << std::endl;
		out_ << std::endl;
	}
};


//*********************************************************************
//						Aggregated Multi node print
//*********************************************************************

template<typename TIMER>
class MultiNodeAggregatePrint : public PrintTimers<TIMER> {
public:
	MultiNodeAggregatePrint(const std::vector<std::string> &line_to_str, int sialx_lines, std::ostream& out) :
		line_to_str_(line_to_str), sialx_lines_(sialx_lines), out_(out) {}
	virtual ~MultiNodeAggregatePrint(){}
	virtual void execute(TIMER& timer){
		sip::SIPMPIAttr &attr = sip::SIPMPIAttr::get_instance();
		std::vector<long long> timers_vector(timer.max_slots(), 0L);
		std::vector<long long> timer_counts_vector(timer.max_slots(), 0L);

		sip::mpi_reduce_sip_timers(timer, timers_vector, timer_counts_vector, attr.company_communicator());

		/**
		 * The printing originally modified the times & the counts array
		 * on the master worker and printed them. This has been changed.
		 * Here's why:
		 *
		 * At the end of a program, each workers SialxTimer maintains
		 * timers for only its worker.
		 * For printing, the timers are aggregated from all workers into
		 * temporary vectors and printed.
		 * The ProfileInterpreter queries the SialxTimer, that is holds a
		 * reference to, for Total & Block wait times at each line and prints
		 * them to screen and saves them to a backing store (SQLite Database).
		 * If the SialxTimer arrays for the master were to be modified when
		 * the timers are printed, the profile timer might give back different
		 * information depending on whether the SialxTimers were printed before
		 * or after the ProfileTimers.
		 */


		long long * timers = &timers_vector[0];
		long long * timer_counts = &timer_counts_vector[0];
		if (SIPMPIAttr::get_instance().is_company_master()){
			printAggregateTable(timer, timers, timer_counts);
		}
	}

protected:

	void printAggregateTable(TIMER& timer, long long * timers, long long * timer_counts) {
		sip::SIPMPIAttr &attr = sip::SIPMPIAttr::get_instance();
		int num_workers = attr.num_workers();

		out_ << "Timers for Program " << GlobalState::get_program_name() << std::endl;
		const int LW = 8;			// Line num
		const int CW = 12;			// Time
		const int SW = 25;			// String
		const int PRECISION = 6; 	// Precision

		out_.precision(PRECISION); // Reset precision to 6 places.

		assert(timer.check_timers_off());
		out_<<std::setw(LW)<<std::left<<"Line"			// Line Number
			<<std::setw(SW)<<std::left<<"Type"			// Type of instruction
			<<std::setw(CW)<<std::left<<"Avg"			// Average time spent per worker
			<<std::setw(CW)<<std::left<<"AvgBlkWt"		// Average block wait per worker
			<<std::setw(CW)<<std::left<<"AvgCount"		// Average counts per workers
			<<std::setw(CW)<<std::left<<"PerCall"		// Average time per call per worker
			<<std::setw(CW)<<std::left<<"BlkWtPC"		// Average block wait per call per worker
			//<<std::setw(CW)<<std::left<<"Tot"			// Sum of time spent over all workers
			//<<std::setw(CW)<<std::left<<"TotBlkWt"	// Sum of block wait time over all workers
			<<std::setw(CW)<<std::left<<"Count"			// Sum of counts over all workers
			<<std::endl;

		int total_time_timer_offset = static_cast<int>(SialxTimer::TOTALTIME);
		int block_wait_timer_offset = static_cast<int>(SialxTimer::BLOCKWAITTIME);

		double sum_of_average_block_wait_time = 0.0;
		double total_wall_time = timer.to_seconds(timers[0] / (double)timer_counts[0]);

		for (int i=1; i<=sialx_lines_; i++){

			if (timer_counts[i + total_time_timer_offset * sialx_lines_] > 0L){
				double tot_time = timer.to_seconds(timers[i + total_time_timer_offset * sialx_lines_]);	// Microsecond to second
				long count = timer_counts[i + total_time_timer_offset * sialx_lines_];
				double avg_count = count / num_workers;
				double per_call = tot_time / count;
				double avg = tot_time / num_workers;
				double tot_blk_wait = 0;
				double avg_blk_wait = 0;
				double blk_wait_per_call = 0;

				if (timer_counts[i + block_wait_timer_offset * sialx_lines_] > 0L){
					tot_blk_wait = timer.to_seconds(timers[i + block_wait_timer_offset * sialx_lines_]);
					blk_wait_per_call = tot_blk_wait / timer_counts[i + block_wait_timer_offset * sialx_lines_];
					avg_blk_wait = tot_blk_wait / num_workers;
					sum_of_average_block_wait_time += avg_blk_wait;
				}
				out_	<< std::setw(LW)<< std::left << i
						<< std::setw(SW)<< std::left << line_to_str_.at(i)
						<< std::setw(CW)<< std::left << avg
						<< std::setw(CW)<< std::left << avg_blk_wait
						<< std::setw(CW)<< std::left << avg_count
						<< std::setw(CW)<< std::left << per_call
						<< std::setw(CW)<< std::left << blk_wait_per_call
						//<< std::setw(CW)<< std::left << tot_time
						//<< std::setw(CW)<< std::left << tot_blk_wait
						<< std::setw(CW)<< std::left << count
						<< std::endl;
			}
		}
		out_ << "Total Walltime : " << total_wall_time << " Seconds "<< std::endl;
		out_ << "Total Block Wait time : " << sum_of_average_block_wait_time << " Seconds " << std::endl;
		out_ << std::endl;
	}

	const std::vector<std::string>& line_to_str_;
	const int sialx_lines_;
	std::ostream& out_;
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
