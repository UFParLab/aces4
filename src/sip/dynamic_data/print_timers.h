/*
 * print_timers.h
 *
 *  Created on: Jul 24, 2014
 *      Author: jindal
 */

#ifndef PRINT_TIMERS_H_
#define PRINT_TIMERS_H_

namespace sip {
/**
 * Abstract templatized class for command pattern.
 * Decouples collecting of timing data
 * and dumping it out to screen / disk.
 */
template <typename TIMER>
class PrintTimers {
public:
	PrintTimers(){};
	virtual ~PrintTimers(){};
	virtual void execute(TIMER&)=0;
};

} /* namespace sip */

#endif /* PRINT_TIMERS_H_ */
