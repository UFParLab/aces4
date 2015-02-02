/*
 * sip_static_named_timers.h
 *
 *  Created on: Feb 1, 2015
 *      Author: njindal
 */

#ifndef SIP_STATIC_NAMED_TIMERS_H_
#define SIP_STATIC_NAMED_TIMERS_H_

#include <iostream>

#include "config.h"
#include "sip.h"
#include "sip_timer.h"


namespace sip {

class SIPStaticNamedTimers {
public:

/** The types of static timers supported
  * First timer must begin with a 0
  */
#define SIP_STATIC_TIMERS\
	STATIC_TIMER(	INIT_SIP_TABLES, 				0, 	"Initialize SIP Tables")\
	STATIC_TIMER(	SAVE_PERSISTENT_ARRAYS, 		1, 	"Save Persistent Arrays")\
	STATIC_TIMER(	INIT_SERVER, 					2, 	"Initialize Server")\
	STATIC_TIMER(	INTERPRET_PROGRAM, 				3, 	"Interpret Program")\
	STATIC_TIMER(	SERVER_RUN, 					4, 	"Server Run")\




	enum Timers_t {
	#define STATIC_TIMER(e, n, s) e = n,
		SIP_STATIC_TIMERS
	#undef STATIC_TIMER
		NUMBER_TIMERS_
	};

	const char* intToTimerName(int i) const;

	SIPStaticNamedTimers();
	void start_timer(Timers_t timer) { delegate_.start_timer(static_cast<int>(timer)); }
	void pause_timer(Timers_t timer) { delegate_.pause_timer(static_cast<int>(timer)); }

	void print_timers(std::ostream& out);

private:

	SipTimer_t delegate_;
	DISALLOW_COPY_AND_ASSIGN(SIPStaticNamedTimers);

};

} /* namespace sip */

#endif /* SIP_STATIC_NAMED_TIMERS_H_ */
