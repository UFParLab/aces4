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

#ifdef HAVE_MPI
#include <mpi.h>
#endif // HAVE_MPI

namespace sip {


class LinuxSIPTimers {
public:
	LinuxSIPTimers(int max_slots);	/*! Constructs Timer with a given max num of slots */
	~LinuxSIPTimers();
	void start_timer(int slot);	/*! Starts timer with given slot */
	void pause_timer(int slot);	/*! Pauses timer with given slot */
	long long* get_timers();		/*! Returns time recorded at each slot */
	long long* get_timer_count();	/*! Returns number of times each timer was switched on & off */

	void print_timers(PrintTimers<LinuxSIPTimers>&); /*! Print out the timers */

protected :
	const static long long _timer_off_value_ = -1;	/*! Sentinel Value */
	const int max_slots_;							/*!	Maximum number of timer slots */

	long long *timer_list_;		/*! Contains total 'time unit' for every line 		*/
	long long *timer_on_; 		/*! Whether a timer is turned on or off 			*/
	long long *timer_switched_;	/*! Number of times a timer was switched on & off 	*/

	bool check_timers_off();	/*! Returns true if all timers have been turned off */
	DISALLOW_COPY_AND_ASSIGN(LinuxSIPTimers);
};

//*********************************************************************

#ifdef HAVE_PAPI
class PAPISIPTimers : public LinuxSIPTimers{
public:
	PAPISIPTimers(int max_slots);
	~PAPISIPTimers();
	void start_timer(int slot);
	void pause_timer(int slot);

	void print_timers(PrintTimers<PAPISIPTimers>& p) /*! Print out the timers */

protected:
	DISALLOW_COPY_AND_ASSIGN(PAPISIPTimers);
};
#endif // HAVE_PAPI


//*********************************************************************

#ifdef HAVE_TAU
class TAUSIPTimers {
public:
	TAUSIPTimers(int max_slots);
	~TAUSIPTimers();
	void start_timer(int slot);
	void pause_timer(int slot);

protected:
	const int max_slots_;	/*!	Maximum number of timer slots */
	void **tau_timers_;
	DISALLOW_COPY_AND_ASSIGN(TAUSIPTimers);
};
#endif // HAVE_TAU



//*********************************************************************

//#ifdef HAVE_MPI
//
//template <typename DELEGATE>
//class MPISIPTimers {
//public:
//	MPISIPTimers(int max_slots) : delegate_(max_slots) {}
//	~MPISIPTimers(){}
//	void start_timer(int slot) { delegate_.start_timer(slot); }
//	void pause_timer(int slot) { delegate_.pause_timer(slot); }
//protected:
//	DELEGATE delegate_;
//	void mpi_reduce_timers();			/*! Reduce the timers to the worker master 	*/
//};
//
//#endif // HAVE_MPI

} /* namespace sip */

#endif /* SIP_TIMER_H_ */
