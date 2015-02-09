/*
 * sipmap_timer.cpp
 *
 *  Created on: Oct 9, 2014
 *      Author: njindal
 */

#include "sipmap_timer.h"
#include <iomanip>
#include "global_state.h"

namespace sip {

SIPMaPTimer::SIPMaPTimer(int max_slots): SialxTimer(max_slots_) {}

SIPMaPTimer::SIPMaPTimer(const SIPMaPTimer& other): SialxTimer(other){}


void SIPMaPTimer::merge_other_timer(const SIPMaPTimer& other){
	CHECK(max_slots_ == other.max_slots_, "Different max_slots when trying to merge other SIPMaPTimer");

	std::vector<SialxUnitTimer>::const_iterator other_it = other.list_.begin();
	std::vector<SialxUnitTimer>::iterator this_it = this->list_.begin();
	for (; other_it != other.list_.end(); ++this_it, ++other_it){
		this_it->accumulate_other(*other_it);
	}
}

const SIPMaPTimer& SIPMaPTimer::operator=(const SIPMaPTimer& other){
	this->max_slots_ = other.max_slots_;
	list_.clear();
	std::copy(other.list_.begin(), other.list_.end(), list_.begin());
	return *this;
}


} /* namespace sip */
