/*
 * sip_static_named_timers.cpp
 *
 *  Created on: Feb 1, 2015
 *      Author: njindal
 */

#include <sip_static_named_timers.h>

#include <string>
#include <iomanip>
#include <cassert>
#include <sstream>

#include "print_timers.h"
#include "global_state.h"

#ifdef HAVE_TAU
#include <TAU.h>
#endif

namespace sip {

SIPStaticNamedTimers::SIPStaticNamedTimers() : delegate_(NUMBER_TIMERS_) {}

const char* SIPStaticNamedTimers::intToTimerName(int i) const{
	CHECK(i < NUMBER_TIMERS_, "Trying to get invalid SIPStaticNamedTimer!");
	switch(i){
	#define STATIC_TIMER(e, n, s) case n: return s;
		SIP_STATIC_TIMERS
	#undef STATIC_TIMER
	}
	sip::fail("Interal error ! opcode not recognized !");
}

//*********************************************************************
//						Single node print
//*********************************************************************
/**
 * Single node version of PrintTimers instance for aces4
 * Prints out total & average times of each interesting sialx line to stdout
 */
template<typename TIMER>
class SingleNodeStaticTimersPrint : public PrintTimers<TIMER> {
public:
	SingleNodeStaticTimersPrint(const SIPStaticNamedTimers& static_named_timers, std::ostream& out)
		: static_named_timers_(static_named_timers), out_(out) {}
	virtual ~SingleNodeStaticTimersPrint(){}
	virtual void execute(TIMER& timer){
		long long * timers = timer.get_timers();
		long long * timer_counts = timer.get_timer_count();

		const int CW = 15;	// Time
		const int SW = 30;	// String
		const int PRECISION = 6; 	// Precision
		out_.precision(PRECISION); // Reset precision to 6 places.

		assert(timer.check_timers_off());
		out_ <<"Timers"<<std::endl
			<<std::setw(SW)<<std::left<<"TimerName"
			<<std::setw(CW)<<std::left<<"AvgPerCall"
			<<std::setw(CW)<<std::left<<"TotTime"
			<<std::setw(CW)<<std::left<<"Count"
			<<std::endl;

		for (int i=0; i<timer.max_slots(); i++){
			if (timer_counts[i] > 0L){
				double tot_time = timer.to_seconds(timers[i]);	// Microsecond to second
				long count = timer_counts[i];
				double avg_time = tot_time / count;

				const char * name = static_named_timers_.intToTimerName(i);
				out_<<std::setw(SW)<<std::left << name
					<< std::setw(CW)<< std::left << avg_time
					<< std::setw(CW)<< std::left << tot_time
					<< std::setw(CW)<< std::left << count
					<< std::endl;
			}
		}
		out_ << std::endl;
	}
private:
	const SIPStaticNamedTimers& static_named_timers_;
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
class MultiNodeStaticTimersPrint : public PrintTimers<TIMER> {
public:
	MultiNodeStaticTimersPrint(const SIPStaticNamedTimers& static_named_timers, std::ostream& out)
		: static_named_timers_(static_named_timers), out_(out) {}
	virtual ~MultiNodeStaticTimersPrint(){}
	virtual void execute(TIMER& timer){
		sip::SIPMPIAttr &attr = sip::SIPMPIAttr::get_instance();
		std::vector<long long> timers_vector(timer.max_slots(), 0L);
		std::vector<long long> timer_counts_vector(timer.max_slots(), 0L);

		sip::mpi_reduce_sip_timers(timer, timers_vector, timer_counts_vector, attr.company_communicator());


		if (SIPMPIAttr::get_instance().is_company_master()){
			sip::SIPMPIAttr &attr = sip::SIPMPIAttr::get_instance();
			int num_workers = attr.num_workers();

			out_ << "Timers for Program " << GlobalState::get_program_name() << std::endl;
			long long * timers = &timers_vector[0];
			long long * timer_counts = &timer_counts_vector[0];
			const int CW = 15;	// Time
			const int SW = 30;	// String
			const int PRECISION = 6; 	// Precision

			out_.precision(PRECISION); // Reset precision to 6 places.

			assert(timer.check_timers_off());
			out_<<"Timers"<<std::endl
				<<std::setw(SW)<<std::left<<"TimerName"
				<<std::setw(CW)<<std::left<<"Avg"			// Average time spent per worker
				<<std::setw(CW)<<std::left<<"AvgCount"		// Average counts per workers
				<<std::setw(CW)<<std::left<<"PerCall"		// Average time per call per worker
				<<std::setw(CW)<<std::left<<"Tot"			// Sum of time spent over all workers
				<<std::setw(CW)<<std::left<<"Count"			// Sum of counts over all workers
				<<std::endl;

			for (int i=0; i<timer.max_slots(); i++){

				if (timer_counts[i] > 0L){
					double tot_time = timer.to_seconds(timers[i]);	// Microsecond to second
					long count = timer_counts[i];
					double avg_count = count / num_workers;
					double per_call = tot_time / count;
					double avg = tot_time / num_workers;

					const char * name = static_named_timers_.intToTimerName(i);
					out_	<< std::setw(SW)<< std::left << name
							<< std::setw(CW)<< std::left << avg
							<< std::setw(CW)<< std::left << avg_count
							<< std::setw(CW)<< std::left << per_call
							<< std::setw(CW)<< std::left << tot_time
							<< std::setw(CW)<< std::left << count
							<< std::endl;
				}
			}
			out_ << std::endl;
		}
	}
private:
	const SIPStaticNamedTimers& static_named_timers_;
	std::ostream& out_;
};
#endif // HAVE_MPI






//*********************************************************************
//				Print for TAUTimers (Fixes timer names)
//*********************************************************************
#ifdef HAVE_TAU
template<typename TIMER>
class TAUStaticTimersPrint : public PrintTimers<TIMER> {
public:
	TAUStaticTimersPrint(const SIPStaticNamedTimers& static_named_timers, std::ostream& ignore)
			: static_named_timers_(static_named_timers) {}
	virtual ~TAUStaticTimersPrint() {/* Do Nothing */}
	virtual void execute(TIMER& timer) {
		void ** tau_timers = timer.get_tau_timers();

		for (int i=0; i<timer.max_slots(); ++i){
			if (tau_timers[i] != NULL){
				std::stringstream tot_sstr;
				const char * name = static_named_timers_.intToTimerName(i);
				tot_sstr << sip::GlobalState::get_program_num() << " : " << name;
				const char *tau_string = tot_sstr.str().c_str();
				TAU_PROFILE_TIMER_SET_NAME(tau_timers[i], tau_string);
			}
		}
	}

private:
	const SIPStaticNamedTimers& static_named_timers_;

};
#endif // HAVE_TAU


void SIPStaticNamedTimers::print_timers(std::ostream& out){
#ifdef HAVE_TAU
	typedef TAUStaticTimersPrint<SipTimer_t> PrintTimersType_t;
#elif defined HAVE_MPI
	typedef MultiNodeStaticTimersPrint<SipTimer_t> PrintTimersType_t;
#else
	typedef SingleNodeStaticTimersPrint<SipTimer_t> PrintTimersType_t;
#endif
	PrintTimersType_t p(*this, out);
	delegate_.print_timers(p);
}

} /* namespace sip */
