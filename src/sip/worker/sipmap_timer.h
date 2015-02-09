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
#include "sip.h"
#include "sialx_timer.h"
#include "sip_tables.h"

namespace sip {

class SIPMaPTimer : public SialxTimer {
public:

	SIPMaPTimer(const int sialx_lines_);
	SIPMaPTimer(const SIPMaPTimer&);

	const SIPMaPTimer& operator=(const SIPMaPTimer&);

	/*! Merges other timer into this one */
	void merge_other_timer(const SIPMaPTimer& other);

	/* Inherits these methods & fields from class SialxTmer 				*/
	/* SialxUnitTimer& operator[](int slot) {return list_.at(slot); } 		*/
	/* SialxUnitTimer& timer(int slot) {return list_.at(slot); }			*/
	/* void print_timers(std::ostream& out, const SipTables& sip_tables);	*/

	/* const int max_slots_;												*/
	/* std::vector<SialxUnitTimer> list_;									*/

};

} /* namespace sip */

#endif /* SIPMAP_TIMER_H_ */
