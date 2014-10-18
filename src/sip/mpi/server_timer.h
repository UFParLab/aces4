/*
 * server_timer.h
 *
 *  Created on: Jul 28, 2014
 *      Author: njindal
 */

#ifndef SERVER_TIMER_H_
#define SERVER_TIMER_H_

#include <vector>
#include "sip.h"
#include "config.h"
#include "sip_timer.h"

namespace sip {

class ServerTimer {
public:
	/**
	 * The types of timer supported per sialx line
	 */
	enum TimerKind_t {
		TOTALTIME = 0,		/*! Total time */
		BLOCKWAITTIME = 1,	/*! Sum of reads, writes & searching through DS */
		READTIME = 2,		/*! Reading from disk */
		WRITETIME = 3,		/*! Writing to disk */
		NUMBER_TIMER_KINDS_
	};

	ServerTimer(int sialx_lines);
	~ServerTimer();

	void start_timer(int line_number, TimerKind_t kind); /*! Starts timer for a sialx line */
	void pause_timer(int line_number, TimerKind_t kind); /*! Pauses timer for a sialx line. */
	void print_timers(std::vector<std::string> line_to_str); /*! For each slot, the total time and the average time is printed */

private:

	/** Underlying timer either Linux, PAPI or TAU timers */
	SipTimer_t delegate_;
	const int sialx_lines_;
};

} /* namespace sip */

#endif /* SERVER_TIMER_H_ */
