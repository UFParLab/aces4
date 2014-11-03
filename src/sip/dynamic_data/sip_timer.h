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
	double to_seconds(long long default_unit) { return (double)default_unit / CLOCKS_PER_SEC; }

	void print_timers(PrintTimers<LinuxSIPTimers>&); /*! Print out the timers */

	int max_slots() { return max_slots_; }
protected :
	const int max_slots_;							/*!	Maximum number of timer slots */
	const static long long _timer_off_value_;	/*! Sentinel Value */

	long long *timer_list_;		/*! Contains total 'time unit' for every line 		*/
	long long *timer_on_; 		/*! Whether a timer is turned on or off 			*/
	long long *timer_switched_;	/*! Number of times a timer was switched on & off 	*/

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
	double to_seconds(long long default_unit) { return (double)default_unit / 1000000l; }

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

	void print_timers(PrintTimers<TAUSIPTimers>& p); /*! Fixes the string associated with each timer. */

	void ** get_tau_timers();

protected:
	const int max_slots_;	/*!	Maximum number of timer slots */
	void **tau_timers_;
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

#endif /* SIP_TIMER_H_ */
