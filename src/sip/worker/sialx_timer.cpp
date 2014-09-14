#include "sialx_timer.h"
#include "assert.h"
#include "sip_interface.h"
#include "global_state.h"
#include "print_timers.h"
#include <iostream>
#include <iomanip>
#include <cstdio>
#include <cstdlib>
#include <sstream>


#ifdef HAVE_MPI
#include <mpi.h>
#include "sip_mpi_attr.h"
#include "sip_mpi_utils.h"
#endif

#ifdef HAVE_TAU
#include <TAU.h>
#endif

namespace sip{

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
	SingleNodePrint(const std::vector<std::string> &line_to_str, int sialx_lines)
		: line_to_str_(line_to_str), sialx_lines_(sialx_lines) {}
	virtual ~SingleNodePrint(){}
	virtual void execute(TIMER& timer){

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
			<<std::setw(CW)<<std::left<<"Tot"
			<<std::setw(CW)<<std::left<<"TotBlkWait"
			<<std::endl;

		int total_time_timer_offset = static_cast<int>(SialxTimer::TOTALTIME);
		int block_wait_timer_offset = static_cast<int>(SialxTimer::BLOCKWAITTIME);

		std::cout.precision(6); // Reset precision to 6 places.
		for (int i=1; i<timer.max_slots - sialx_lines_; i++){
			if (timer_counts[i + total_time_timer_offset * sialx_lines_] > 0L){
				double tot_time = timer.to_seconds(timers[i + total_time_timer_offset * sialx_lines_]);	// Microsecond to second
				double avg_time = tot_time / timer_counts[i + total_time_timer_offset * sialx_lines_];
				double tot_blk_wait = 0;
				double avg_blk_wait = 0;

				if (timer_counts[i + block_wait_timer_offset * sialx_lines_] > 0L){
					tot_blk_wait = timer.to_seconds(timers[i + block_wait_timer_offset * sialx_lines_]);
					avg_blk_wait = tot_blk_wait / timer_counts[i + block_wait_timer_offset * sialx_lines_];
				}

				std::cout<<std::setw(LW)<<std::left << i
						<< std::setw(SW)<< std::left << line_to_str_.at(i)
						<< std::setw(CW)<< std::left << avg_time
						<< std::setw(CW)<< std::left << avg_blk_wait
						<< std::setw(CW)<< std::left << tot_time
						<< std::setw(CW)<< std::left << tot_blk_wait
						<< std::endl;
			}
		}
		std::cout<<std::endl;
	}
private:
	const std::vector<std::string>& line_to_str_;
	const int sialx_lines_;
};

//*********************************************************************
//						Multi node print
//*********************************************************************

#ifdef HAVE_MPI

/**
 * MPI_Reduce Reduction function for Timers
 * @param r_in
 * @param r_inout
 * @param len
 * @param type
 */
void sialx_timer_reduce_op_function(void* r_in, void* r_inout, int *len, MPI_Datatype *type){
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
 * PrintTimers instance for multi node version of aces4.
 * Prints out total & average times of each interesting sialx line to stdout.
 * The average and total is over all workers.
 */
template<typename TIMER>
class MultiNodePrint : public PrintTimers<TIMER> {
public:
	MultiNodePrint(const std::vector<std::string> &line_to_str, int sialx_lines) :
		line_to_str_(line_to_str), sialx_lines_(sialx_lines) {}
	virtual ~MultiNodePrint(){}
	virtual void execute(TIMER& timer){

		mpi_reduce_timers(timer);

		// Print from the worker master.

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
				<<std::setw(CW)<<std::left<<"Tot"
				<<std::setw(CW)<<std::left<<"TotBlkWait"
				<<std::endl;

			int total_time_timer_offset = static_cast<int>(SialxTimer::TOTALTIME);
			int block_wait_timer_offset = static_cast<int>(SialxTimer::BLOCKWAITTIME);

			std::cout.precision(6); // Reset precision to 6 places.
			for (int i=1; i<timer.max_slots - sialx_lines_; i++){

				if (timer_counts[i + total_time_timer_offset * sialx_lines_] > 0L){
					double tot_time = timer.to_seconds(timers[i + total_time_timer_offset * sialx_lines_]);	// Microsecond to second
					double avg_time = tot_time / timer_counts[i + total_time_timer_offset * sialx_lines_];
					double tot_blk_wait = 0;
					double avg_blk_wait = 0;

					if (timer_counts[i + block_wait_timer_offset * sialx_lines_] > 0L){
						tot_blk_wait = timer.to_seconds(timers[i + block_wait_timer_offset * sialx_lines_]);
						avg_blk_wait = tot_blk_wait / timer_counts[i + block_wait_timer_offset * sialx_lines_];
					}
					std::cout<<std::setw(LW)<<std::left << i
							<< std::setw(SW)<< std::left << line_to_str_.at(i)
							<< std::setw(CW)<< std::left << avg_time
							<< std::setw(CW)<< std::left << avg_blk_wait
							<< std::setw(CW)<< std::left << tot_time
							<< std::setw(CW)<< std::left << tot_blk_wait
							<< std::endl;
				}
			}
			std::cout<<std::endl;
		}
	}
private:
	const std::vector<std::string>& line_to_str_;
	const int sialx_lines_;
	void mpi_reduce_timers(TIMER& timer){
		sip::SIPMPIAttr &attr = sip::SIPMPIAttr::get_instance();
		sip::check(attr.is_worker(), "Trying to reduce timer on a non-worker rank !");
		long long * timers = timer.get_timers();
		long long * timer_counts = timer.get_timer_count();

		// Data to send to reduce
		long long * sendbuf = new long long[2*timer.max_slots + 1];
		sendbuf[0] = timer.max_slots;
		std::copy(timer_counts + 0, timer_counts + timer.max_slots, sendbuf+1);
		std::copy(timers + 0, timers + timer.max_slots, sendbuf+1+ timer.max_slots);

		long long * recvbuf = new long long[2*timer.max_slots + 1]();

		int worker_master = attr.worker_master();
		MPI_Comm worker_company = attr.company_communicator();

		// The data will be structured as
		// Length of arrays 1 & 2
		// Array1 -> timer_switched_ array
		// Array2 -> timer_list_ array
		MPI_Datatype sialx_timer_reduce_dt; // MPI Type for timer data to be reduced.
		MPI_Op sialx_timer_reduce_op;	// MPI OP to reduce timer data.
		SIPMPIUtils::check_err(MPI_Type_contiguous(timer.max_slots*2+1, MPI_LONG_LONG, &sialx_timer_reduce_dt));
		SIPMPIUtils::check_err(MPI_Type_commit(&sialx_timer_reduce_dt));
		SIPMPIUtils::check_err(MPI_Op_create((MPI_User_function *)sialx_timer_reduce_op_function, 1, &sialx_timer_reduce_op));

		SIPMPIUtils::check_err(MPI_Reduce(sendbuf, recvbuf, 1, sialx_timer_reduce_dt, sialx_timer_reduce_op, worker_master, worker_company));

		if (attr.is_company_master()){
			std::copy(recvbuf+1, recvbuf+1+timer.max_slots, timer_counts);
			std::copy(recvbuf+1+timer.max_slots, recvbuf+1+2*timer.max_slots, timers);
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
	TAUTimersPrint(const std::vector<std::string> &line_to_str, int sialx_lines)
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

//*********************************************************************
// 						Methods for SialxTimers
//*********************************************************************

SialxTimer::SialxTimer(int sialx_lines) :
		delegate_(1 + NUMBER_TIMER_KINDS_*(sialx_lines)),
		sialx_lines_(sialx_lines){
	/* Need NUMBER_TIMER_KINDS_ times the maximum number of slots,
	 * one for each kind defined in the TimerKind_t enum.
	 * One extra added since line numbers begin at 1.
	 */
}

void SialxTimer::start_timer(int line_number, TimerKind_t kind){
	check(kind < NUMBER_TIMER_KINDS_, "Invalid timer type", line_number);
	check(line_number <= sialx_lines_, "Invalid line number", line_number);
	delegate_.start_timer(line_number + ((int)kind) * sialx_lines_);
}

void SialxTimer::pause_timer(int line_number, TimerKind_t kind){
	check(kind < NUMBER_TIMER_KINDS_, "Invalid timer type", line_number);
	check(line_number <= sialx_lines_, "Invalid line number", line_number);
	delegate_.pause_timer(line_number + ((int)kind) * sialx_lines_);
}


void SialxTimer::print_timers(std::vector<std::string> line_to_str) {
#ifdef HAVE_TAU
	typedef TAUTimersPrint<TimerType_t> PrintTimersType_t;
#elif defined HAVE_MPI
	typedef MultiNodePrint<TimerType_t> PrintTimersType_t;
#else
	typedef SingleNodePrint<TimerType_t> PrintTimersType_t;
#endif
	PrintTimersType_t p(line_to_str, sialx_lines_);
	delegate_.print_timers(p);
}


//#ifndef HAVE_TAU
//std::ostream& operator<<(std::ostream& os, const SialxTimer& obj) {
//	os << "SialxTimer [\n";
//	os << "max slots : " << obj.max_slots_ << std::endl;
//	os << "timer_switched { " ;
//	for (int i=0; i<obj.max_slots_; i++)
//		os << obj.timer_switched_[i] << " ";
//	os << "}" << std::endl;
//	os << "timer_on { " ;
//	for (int i=0; i<obj.max_slots_; i++)
//		os << obj.timer_on_[i] << " ";
//	os << "}" << std::endl;
//	os << "timer_list {" ;
//	for (int i=0; i<obj.max_slots_; i++)
//		os << obj.timer_list_[i] << " ";
//	os << " }" << std::endl;
//	os << "]" << std::endl;
//	return os;
//}
//#endif

}
