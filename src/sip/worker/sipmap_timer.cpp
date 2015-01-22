/*
 * sipmap_timer.cpp
 *
 *  Created on: Oct 9, 2014
 *      Author: njindal
 */

#include "sipmap_timer.h"
#include "sialx_print_timer.h"
#include <iomanip>
#include "global_state.h"

namespace sip {

const long long SIPMaPTimer::_timer_off_value_ = -1;


SIPMaPTimer::SIPMaPTimer(const int sialx_lines):
		sialx_lines_(sialx_lines),
		max_slots_(SialxTimer::NUMBER_TIMER_KINDS_*(1 + sialx_lines)),
		timer_list_(max_slots_, 0L), timer_switched_(max_slots_, 0L) {
//	timer_list_ = new long long[max_slots_]();		// C++ feature Paren causes it to be initialized to 0;
//	timer_switched_ = new long long[max_slots_]();
}

SIPMaPTimer::SIPMaPTimer(const SIPMaPTimer& other):
		sialx_lines_(other.sialx_lines_),
		max_slots_(other.max_slots_),
		timer_list_(max_slots_, 0L), timer_switched_(max_slots_, 0L) {
//	timer_list_ = new long long[max_slots_]();		// C++ feature Paren causes it to be initialized to 0;
//	timer_switched_ = new long long[max_slots_]();
	std::copy(other.timer_list_.begin(), other.timer_list_.end(), timer_list_.begin());
	std::copy(other.timer_switched_.begin(), other.timer_switched_.end(), timer_switched_.begin());
}


SIPMaPTimer::~SIPMaPTimer() {
//	delete [] timer_list_;
//	delete [] timer_switched_;
}


void SIPMaPTimer::record_time(int line_number, long time, SialxTimer::TimerKind_t kind){
	timer_list_.at(line_number + ((int)kind) * sialx_lines_) += time;
	timer_switched_.at(line_number + ((int)kind) * sialx_lines_) ++;
}

void SIPMaPTimer::merge_other_timer(const SIPMaPTimer& other){
	CHECK(sialx_lines_ == other.sialx_lines_, "Different sialx_lines_ when trying to merge other SIPMaPTimer");
	CHECK(max_slots_ == other.max_slots_, "Different max_slots when trying to merge other SIPMaPTimer");
	for (int i=0; i<max_slots_; ++i)
		timer_list_.at(i) += other.timer_list_.at(i);
	for (int i=0; i<max_slots_; ++i)
		timer_switched_.at(i) += other.timer_switched_.at(i);
}

const SIPMaPTimer& SIPMaPTimer::operator=(const SIPMaPTimer& other){
	this->max_slots_ = other.max_slots_;
	this->sialx_lines_ = other.sialx_lines_;
	timer_list_.clear();
	timer_switched_.clear();
//	delete [] timer_list_;
//	delete [] timer_switched_;
//	timer_list_ = new long long[max_slots_]();		// C++ feature Paren causes it to be initialized to 0;
//	timer_switched_ = new long long[max_slots_]();
//	std::copy(other.timer_list_ + 0, other.timer_list_ + max_slots_, timer_list_);
//	std::copy(other.timer_switched_ + 0, other.timer_switched_ + max_slots_, timer_switched_);
	std::copy(other.timer_list_.begin(), other.timer_list_.end(), timer_list_.begin());
	std::copy(other.timer_switched_.begin(), other.timer_switched_.end(), timer_switched_.begin());
	return *this;
}



void SIPMaPTimer::print_timers(const std::vector<std::string>& line_to_str, std::ostream& out_) {
	SingleNodePrint<SIPMaPTimer> printer(line_to_str, sialx_lines_, out_);
	printer.execute(*this);
}


} /* namespace sip */
