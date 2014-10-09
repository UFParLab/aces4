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

	SialxTimer(int sialx_lines);

	void start_timer(int line_number, TimerKind_t kind); /*! Starts timer for a sialx line */
	void pause_timer(int line_number, TimerKind_t kind); /*! Pauses timer for a sialx line. */
	void print_timers(const std::vector<std::string>& line_to_str, std::ostream& out = std::cout); /*! For each slot, the total time and the average time is printed */


private:

/** Underlying timer either Linux, PAPI or TAU timers */
#ifdef HAVE_TAU
	typedef TAUSIPTimers TimerType_t;
#elif defined HAVE_PAPI
	typedef PAPISIPTimers TimerType_t;
#else
	typedef LinuxSIPTimers TimerType_t;
#endif

	TimerType_t delegate_;
	const int sialx_lines_;

	DISALLOW_COPY_AND_ASSIGN(SialxTimer);
};

}


#endif /* SIALX_TIMER_H_ */
