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

namespace sip {

class SIPMaPTimer {
public:
	SIPMaPTimer(const int sialx_lines_);
	~SIPMaPTimer();

	void record_time(int line_number, long time);

	void print_timers(const std::vector<std::string>& line_to_str, std::ostream& out=std::cout);

private:

	/**
	 * Utility function to convert time in
	 * microseconds to seconds
	 * @param time_in_milli
	 * @return
	 */
	double to_seconds(long long time_in_micro) { return time_in_micro / 1000000.0; }

	const int max_slots_;

	const static long long _timer_off_value_;	/*! Sentinel Value */

	long long *timer_list_;		/*! Contains total 'time unit' for every line 		*/
	long long *timer_switched_;	/*! Number of times a timer was switched on & off 	*/
};

} /* namespace sip */

#endif /* SIPMAP_TIMER_H_ */
