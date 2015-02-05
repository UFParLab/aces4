/*
 * sip_timer.h
 *
 *	Timers constructed using non-virtual inheritance, templating & delegation
 *
 *  Created on: Jul 23, 2014
 *      Author: jindal
 */

#ifndef SIP_TIMER_H_
#define SIP_TIMER_H_

#include "config.h"
#include "sip.h"
#include "sip_mpi_utils.h"
#include "sip_mpi_attr.h"
#include "print_timers.h"

#include <ctime>

#ifdef HAVE_MPI
#include <mpi.h>
#endif // HAVE_MPI

#ifdef HAVE_TAU
	#define INIT_GLOBAL_TIMERS(argc, argv) 	\
		TAU_INIT(argc, argv);\
		TAU_PROFILE_SET_NODE(sip::SIPMPIAttr::get_instance().global_rank());\
		TAU_STATIC_PHASE_START("SIP Main");

	#define FINALIZE_GLOBAL_TIMERS()\
		TAU_STATIC_PHASE_STOP("SIP Main");

#define START_TAU_SIALX_PROGRAM_DYNAMIC_PHASE(p) \
		TAU_PHASE_CREATE_DYNAMIC(tau_dtimer, p, "", TAU_USER);\
		TAU_PHASE_START(tau_dtimer);

#define STOP_TAU_SIALX_PROGRAM_DYNAMIC_PHASE()\
  		TAU_PHASE_STOP(tau_dtimer);

#else
	#define	INIT_GLOBAL_TIMERS(argc, argv) ;
	#define FINALIZE_GLOBAL_TIMERS();
	#define START_TAU_SIALX_PROGRAM_DYNAMIC_PHASE(p);
	#define STOP_TAU_SIALX_PROGRAM_DYNAMIC_PHASE() ;
#endif

namespace sip {

class LinuxSIPTimers {
public:
	LinuxSIPTimers(int max_slots);	/*! Constructs Timer with a given max num of slots */
	~LinuxSIPTimers();
	void start_timer(int slot);	/*! Starts timer with given slot */
	void pause_timer(int slot);	/*! Pauses timer with given slot */
	long long* get_timers();		/*! Returns time recorded at each slot */
	long long* get_timer_count();	/*! Returns number of times each timer was switched on & off */
	bool check_timers_off();	/*! Returns true if all timers have been turned off */
	static double to_seconds(long long default_unit) { return (double)default_unit / CLOCKS_PER_SEC; }
	void print_timers(PrintTimers<LinuxSIPTimers>&); /*! Print out the timers */

	int max_slots() { return max_slots_; }

protected :
	const int max_slots_;							/*!	Maximum number of timer slots */
	const static long long _timer_off_value_;	/*! Sentinel Value */

	long long *timer_list_;		/*! Contains total 'time unit' for every line 		*/
	long long *timer_switched_;	/*! Number of times a timer was switched on & off 	*/
	long long *timer_on_; 		/*! Whether a timer is turned on or off 			*/

	DISALLOW_COPY_AND_ASSIGN(LinuxSIPTimers);
};

//*********************************************************************

#ifdef HAVE_PAPI
class PAPISIPTimers : public LinuxSIPTimers{
public:
	PAPISIPTimers(int max_slots_);
	~PAPISIPTimers();
	void start_timer(int slot);
	void pause_timer(int slot);
	bool check_timers_off();	/*! Returns true if all timers have been turned off */
	static double to_seconds(long long default_unit) { return (double)default_unit / 1000000l; }

	void print_timers(PrintTimers<PAPISIPTimers>& p); /*! Print out the timers */

protected:
	DISALLOW_COPY_AND_ASSIGN(PAPISIPTimers);
};
#endif // HAVE_PAPI


//*********************************************************************

#ifdef HAVE_TAU
class TAUSIPTimers {
public:
	TAUSIPTimers(int max_slots_);
	~TAUSIPTimers();
	void start_timer(int slot);
	void pause_timer(int slot);

	void ** get_tau_timers();

	long long* get_timers();		/*! Returns time recorded at each slot */
	long long* get_timer_count();	/*! Returns number of times each timer was switched on & off */

	void print_timers(PrintTimers<TAUSIPTimers>& p); /*! Fixes the string associated with each timer. */
	static double to_seconds(long long default_unit) { return (double)default_unit / CLOCKS_PER_SEC; }

	bool check_timers_off() { ; /* Do Nothing */ }
	int max_slots() { return max_slots_; }
protected:
	void **tau_timers_;

	const int max_slots_;		/*!	Maximum number of timer slots */
	long long *timer_list_;		/*! Contains total 'time unit' for every line 		*/
	long long *timer_switched_;	/*! Number of times a timer was switched on & off 	*/

	long previous_max_slots_;
	static long already_allocated_slots_;

	DISALLOW_COPY_AND_ASSIGN(TAUSIPTimers);
};
#endif // HAVE_TAU

#ifdef HAVE_TAU
	typedef TAUSIPTimers SipTimer_t;
#elif defined HAVE_PAPI
	typedef PAPISIPTimers SipTimer_t;
#else
	typedef LinuxSIPTimers SipTimer_t;
#endif


} /* namespace sip */



// Utility Functions to collect & merge LinuxSIPTimers & PAPISIPTimers from various workers & servers

#ifdef HAVE_MPI
/**
 * MPI_Reduce Reduction function for Timers
 * @param r_in
 * @param r_inout
 * @param len
 * @param type
 */
void sip_timer_reduce_op_function(void* r_in, void* r_inout, int *len, MPI_Datatype *type);

namespace sip{
/**
 * Reduces timers from all workers/servers to COMPANY_MASTER_RANK for that company.
 * Puts the aggregated timers and timer counts into passed in vectors
 * @param timer [in]
 * @param timers_vector [out]
 * @param timer_counts_vector [out]
 */
template<typename TIMER>
void mpi_reduce_sip_timers(TIMER& timer, std::vector<long long>& timers_vector, std::vector<long long>& timer_counts_vector, MPI_Comm company) {
	sip::SIPMPIAttr &attr = sip::SIPMPIAttr::get_instance();
	//CHECK(attr.is_worker(), "Trying to reduce timer on a non-worker rank !");
	long long * timers = timer.get_timers();
	long long * timer_counts = timer.get_timer_count();

	// Data to send to reduce
	long long * sendbuf = new long long[2*timer.max_slots() + 1]();
	sendbuf[0] = timer.max_slots();
	std::copy(timer_counts + 0, timer_counts + timer.max_slots(), sendbuf+1);
	std::copy(timers + 0, timers + timer.max_slots(), sendbuf+1+ timer.max_slots());

	long long * recvbuf = new long long[2*timer.max_slots() + 1]();

	int master = attr.COMPANY_MASTER_RANK;

	// The data will be structured as
	// Length of arrays 1 & 2
	// Array1 -> timer_switched_ array
	// Array2 -> timer_list_ array
	MPI_Datatype sip_timer_reduce_dt; // MPI Type for timer data to be reduced.
	MPI_Op sip_timer_reduce_op;	// MPI OP to reduce timer data.
	SIPMPIUtils::check_err(MPI_Type_contiguous(timer.max_slots()*2+1, MPI_LONG_LONG, &sip_timer_reduce_dt));
	SIPMPIUtils::check_err(MPI_Type_commit(&sip_timer_reduce_dt));
	SIPMPIUtils::check_err(MPI_Op_create((MPI_User_function *)sip_timer_reduce_op_function, 1, &sip_timer_reduce_op));

	SIPMPIUtils::check_err(MPI_Reduce(sendbuf, recvbuf, 1, sip_timer_reduce_dt, sip_timer_reduce_op, master, company));
	if (attr.is_company_master()){
		std::copy(recvbuf+1, recvbuf+1+timer.max_slots(), timer_counts_vector.begin());
		std::copy(recvbuf+1+timer.max_slots(), recvbuf+1+2*timer.max_slots(), timers_vector.begin());
	}

	// Cleanup
	delete [] sendbuf;
	delete [] recvbuf;

	SIPMPIUtils::check_err(MPI_Type_free(&sip_timer_reduce_dt));
	SIPMPIUtils::check_err(MPI_Op_free(&sip_timer_reduce_op));
}
} // Namespace sip

#endif // HAVE_MPI

#endif /* SIP_TIMER_H_ */
