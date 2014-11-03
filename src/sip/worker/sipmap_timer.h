/*
 * sipmap_timer.h
 *
 *  Created on: Oct 9, 2014
 *      Author: njindal
 */

#ifndef SIPMAP_TIMER_H_
#define SIPMAP_TIMER_H_

#include <vector>
#include <iostream>
#include "sialx_timer.h"
#include "sip.h"

namespace sip {

class SIPMaPTimer {
public:

//#define SIPMAP_TIMERKINDS\
//	TIMERKINDS(TOTALTIME, 0, "Total Time")				/*! Total Time for a sialx line*/ \
//	TIMERKINDS(BLOCKWAITTIME, 1, "Block Wait Time")		/*! Block wait time for a sialx line*/
//
//	enum TimerKind_t {
//	#define TIMERKINDS(e, n, s) e = n,
//		SIPMAP_TIMERKINDS
//	#undef TIMERKINDS
//		NUMBER_TIMER_KINDS_
//	};

	SIPMaPTimer(const int sialx_lines_);
	SIPMaPTimer(const SIPMaPTimer&);
	~SIPMaPTimer();
	void record_time(int line_number, long time, SialxTimer::TimerKind_t kind);
	void merge_other_timer(const SIPMaPTimer& other);			//! Merges other timer into this one
	void print_timers(const std::vector<std::string>& line_to_str, std::ostream& out=std::cout);

	long long* get_timers() { return &(timer_list_[0]); };			/*! Returns time recorded at each slot */
	long long* get_timer_count() { return &(timer_switched_[0]); };	/*! Returns number of times each timer was switched on & off */
	bool check_timers_off() { return true; /* No OP */ }		/*! Returns true if all timers have been turned off */

	const SIPMaPTimer& operator=(const SIPMaPTimer&);

	/**
	 * Utility function to convert time in
	 * microseconds to seconds
	 * @param time_in_milli
	 * @return
	 */
	double to_seconds(long long time_in_micro) { return time_in_micro / 1000000.0; }

	int max_slots() { return max_slots_; }

private:

	int max_slots_;
	int sialx_lines_;

	const static long long _timer_off_value_;	/*! Sentinel Value */

//	long long *timer_list_;		/*! Contains total 'time unit' for every line 		*/
//	long long *timer_switched_;	/*! Number of times a timer was switched on & off 	*/

	std::vector<long long> timer_list_;
	std::vector<long long> timer_switched_;


};

} /* namespace sip */

#endif /* SIPMAP_TIMER_H_ */
