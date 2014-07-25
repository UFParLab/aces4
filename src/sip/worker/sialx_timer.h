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

namespace sip{

class SialxTimer{

public:
	SialxTimer(int max_slots);

	void start_timer(int slot); /*! Starts timer for a given slot */
	void pause_timer(int slot); /*! Pauses timer for a given slot. */
	void print_timers(std::vector<std::string> line_to_str); /*! For each slot, the total time and the average time is printed */

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

	DISALLOW_COPY_AND_ASSIGN(SialxTimer);
};

}


#endif /* SIALX_TIMER_H_ */
