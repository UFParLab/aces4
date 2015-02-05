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
#include "sip_timer.h"
#include "sip_interface.h"
#include <iostream>

namespace sip{

class SialxTimer{

public:
	/**
	 * The types of timer supported per sialx line
	 */
#define SIALX_TIMERKINDS\
	TIMERKINDS(TOTALTIME, 0, "Total Time")			/*! Total Time for a sialx line*/ \
	TIMERKINDS(BLOCKWAITTIME, 1, "Block Wait Time")	/*! Block wait time for a sialx line*/

	enum TimerKind_t {
	#define TIMERKINDS(e, n, s) e = n,
		SIALX_TIMERKINDS
	#undef TIMERKINDS
		NUMBER_TIMER_KINDS_
	};

	/**
	 * Constructs a timer for a sialx program which has sialx_lines number of lines.
	 * The number of sialx_lines defines upper-bound on the number of timers needed.
	 * @param sialx_lines
	 */
	SialxTimer(int sialx_lines);

	/**
	 * Starts the timer of a given type and a given sialx line number
	 * @param line_number
	 * @param kind
	 */
	void start_timer(int line_number, TimerKind_t kind); /*! Starts timer for a sialx line */

	/**
	 * Pauses the timer for a given type and a given sialx line number
	 * @param line_number
	 * @param kind
	 */
	void pause_timer(int line_number, TimerKind_t kind); /*! Pauses timer for a sialx line. */

	/**
	 * Returns the total time collected from the
	 * underlying delegate for a given sialx line and a given type.
	 * @param line_number
	 * @param kind
	 * @return the time in the unit measured by the underlying SipTimer instance
	 */
	long long get_timer_value(int line_number, TimerKind_t kind);

	/**
	 * Returns the total number of times the timer was switched on and off
	 * for a given sialx line and a given type.
	 * @param line_number
	 * @param kind
	 * @return the time in the unit measured by the underlying SipTimer instance
	 *
	 */
	long long get_timer_count(int line_number, TimerKind_t kind);

	/**
	 * Converts the unit measured by the underlying SipTimer to seconds.
	 * @param default_unit
	 * @return
	 */
	double to_seconds(long long default_unit) { return SipTimer_t::to_seconds(default_unit); }

	/**
	 * Master worker prints aggregated timers to the given std::ostream (std::cout by default)
	 * @param line_to_str
	 * @param out
	 */
	void print_aggregate_timers(const std::vector<std::string>& line_to_str, std::ostream& out = std::cout); /*! For each slot, the total time and the average time is printed */

	/**
	 * Timer print per worker
	 * @param line_to_str
	 * @param out
	 */
	void print_timers(const std::vector<std::string>& line_to_str, std::ostream& out = std::cout); /*! For each slot, the total time and the average time is printed */

	/**
	 * Starts the program timer.
	 * Expected to be called when the program starts.
	 * Should be called only once during the lifetime of this object
	 */
	void start_program_timer();

	/**
	 * Stops the program timer.
	 * Expected to be called when the program ends.
	 * Should be called only once during the lifetime of this object.
	 */
	void stop_program_timer();

private:

	/** Underlying timer either Linux, PAPI or TAU timers */
	SipTimer_t delegate_;
	const int sialx_lines_;

	DISALLOW_COPY_AND_ASSIGN(SialxTimer);
};

}


#endif /* SIALX_TIMER_H_ */
