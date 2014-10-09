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

	void print_timers(const std::vector<std::string>& line_to_str, std::ostream& out=std::cout);

private:
	const int sialx_lines_;
};

} /* namespace sip */

#endif /* SIPMAP_TIMER_H_ */
