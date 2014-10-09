/*
 * sipmap_interpreter.cpp
 *
 *  Created on: Oct 9, 2014
 *      Author: njindal
 */

#include "sipmap_interpreter.h"
#include "profile_timer_store.h"
#include "sipmap_timer.h"

namespace sip {

SIPMaPInterpreter::SIPMaPInterpreter(const SipTables& sipTables, const ProfileTimerStore& profile_timer_store, SIPMaPTimer& sipmap_timer):
	SialxInterpreter(sipTables, NULL, NULL, NULL), profile_timer_store_(profile_timer_store), sipmap_timer_(sipmap_timer){
}

SIPMaPInterpreter::~SIPMaPInterpreter() {}

} /* namespace sip */
