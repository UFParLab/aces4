#include "sialx_timer.h"
#include "assert.h"
#include "sip_interface.h"
#include "global_state.h"
#include "print_timers.h"
#include <iostream>
#include <iomanip>
#include <cstdio>
#include <cstdlib>


#ifdef HAVE_MPI
#include <mpi.h>
#include "sip_mpi_attr.h"
#include "sip_mpi_utils.h"
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
	SingleNodePrint(const std::vector<std::string> &line_to_str) : line_to_str_(line_to_str) {}
	virtual ~SingleNodePrint(){}
	virtual void execute(TIMER& timer){
		long long * timers = timer.get_timers();
		long long * timer_counts = timer.get_timer_count();
		const int LW = 10;	// Line num
		const int CW = 15;	// String
		const int SW = 20;	// Time

		assert(timer.check_timers_off());
		std::cout<<"Timers"<<std::endl
			<<std::setw(LW)<<std::left<<"Line"
			<<std::setw(SW)<<std::left<<"Type"
			<<std::setw(CW)<<std::left<<"Total"
			<<std::setw(CW)<<std::left<<"Average"
			<<std::endl;
		for (int i=0; i<timer.max_slots; i++){
			if (timer_counts[i] > 0L){
				double tot_time = timer.to_seconds(timers[i]);	// Microsecond to second
				double avg_time = tot_time / timer_counts[i];
				std::cout<<std::setw(LW)<<std::left << i
						<< std::setw(SW)<< std::left << line_to_str_.at(i)
				        << std::setw(CW)<< std::left << tot_time
				        << std::setw(CW)<< std::left << avg_time
				        << std::endl;
			}
		}
		std::cout<<std::endl;
	}
private:
	const std::vector<std::string>& line_to_str_;
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
	MultiNodePrint(const std::vector<std::string> &line_to_str) : line_to_str_(line_to_str) {}
	virtual ~MultiNodePrint(){}
	virtual void execute(TIMER& timer){

		mpi_reduce_timers(timer);

		// Print from the worker master.

		if (SIPMPIAttr::get_instance().is_company_master()){

			long long * timers = timer.get_timers();
			long long * timer_counts = timer.get_timer_count();
			const int LW = 10;	// Line num
			const int CW = 15;	// String
			const int SW = 20;	// Time

			assert(timer.check_timers_off());
			std::cout<<"Timers"<<std::endl
				<<std::setw(LW)<<std::left<<"Line"
				<<std::setw(SW)<<std::left<<"Type"
				<<std::setw(CW)<<std::left<<"Total"
				<<std::setw(CW)<<std::left<<"Average"
				<<std::endl;
			for (int i=0; i<timer.max_slots; i++){
				if (timer_counts[i] > 0L){
					double tot_time = timer.to_seconds(timers[i]);	// Microsecond to second
					double avg_time = tot_time / timer_counts[i];
					std::cout<<std::setw(LW)<<std::left << i
							<< std::setw(SW)<< std::left << line_to_str_.at(i)
							<< std::setw(CW)<< std::left << tot_time
							<< std::setw(CW)<< std::left << avg_time
							<< std::endl;
				}
			}
			std::cout<<std::endl;
		}
	}
private:
	const std::vector<std::string>& line_to_str_;
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
// 						Dummy Print for TAUTimers
//*********************************************************************
template<typename TIMER>
class TAUTimersPrint : public PrintTimers<TIMER> {
public:
	TAUTimersPrint(const std::vector<std::string> &line_to_str) {/* Do Nothing */}
	virtual ~TAUTimersPrint() {/* Do Nothing */}
	virtual void execute(TIMER& timer) {}
};

//*********************************************************************
// 						Methods for SialxTimers
//*********************************************************************

SialxTimer::SialxTimer(int max_slots) : delegate_(max_slots+1){}

void SialxTimer::start_timer(int slot){
	delegate_.start_timer(slot);
}

void SialxTimer::pause_timer(int slot){
	delegate_.pause_timer(slot);
}




void SialxTimer::print_timers(std::vector<std::string> line_to_str) {
#ifdef HAVE_TAU
	typedef TAUTimersPrint<TimerType_t> PrintTimersType_t;
#elif defined HAVE_MPI
	typedef MultiNodePrint<TimerType_t> PrintTimersType_t;
#else
	typedef SingleNodePrint<TimerType_t> PrintTimersType_t;
#endif
	PrintTimersType_t p(line_to_str);
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
