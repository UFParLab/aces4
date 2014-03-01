/*
 * sialx_timer.h
 *
 *  Created on: Nov 2, 2013
 *      Author: jindal
 */

#ifndef SIALX_TIMER_H_
#define SIALX_TIMER_H_

#include <vector>
#include "sip.h"
#include "config.h"

#ifdef HAVE_PAPI
#include "papi.h"
#define PAPI
#else
#include <ctime>
#endif

#ifdef HAVE_TAU
#include <TAU.h>
#endif

#ifdef HAVE_MPI
#include "mpi.h"
#include "sip_mpi_attr.h"
#include "sip_mpi_utils.h"
/**
 * Reduction function for Timers
 */
void sialx_timer_reduce_op_function(void* in, void* inout, int *len, MPI_Datatype *type);

#endif

namespace sip{

class SialxTimer{

public:
	SialxTimer(int max_slots);
	~SialxTimer();
	/**
	 * Starts timer for a given slot
	 * @param slot
	 */
	void start_timer(int slot);
	/**
	 * Pauses timer for a given slot.
	 * @param slot
	 */
	void pause_timer(int slot);
	/**
	 * For each slot, the total time and the average time is printed
	 * @param line_to_str
	 */
	void print_timers(std::vector<std::string> line_to_str);


#ifdef HAVE_MPI
	/**
	 * Reduce the timers to the worker master
	 * @param
	 * @param
	 * @return
	 */
	void mpi_reduce_timers();
#endif

	/**
	 * To print a SialxTimer object
	 * @param
	 * @param
	 * @return
	 */
	friend std::ostream& operator<<(std::ostream&, const SialxTimer &);


private:
	const static long long _timer_off_value_ = -1;	// Sentinel Value
	int max_slots_;				//	Maximum number of slots
	long long *timer_list_;		// Contains total 'time unit' for every line
	long long *timer_on_; 		// Whether a timer is turned on or off
	long long *timer_switched_;	// Number of times a timer was switched on & off

#ifdef HAVE_MPI
    MPI_Datatype sialx_timer_reduce_dt; // MPI Type for timer data to be reduced.
    MPI_Op sialx_timer_reduce_op;		// MPI OP to reduce timer data.
#endif

#ifdef HAVE_TAU
    void **tau_timers_;
#endif

	/**
	 * returns true if all timers have been turned off
	 */
	bool check_timers_off();

	/**
	 * Disallow the default copy constructor and assign operator
	 */
	DISALLOW_COPY_AND_ASSIGN(SialxTimer);


};

}


#endif /* SIALX_TIMER_H_ */
