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

const long long SIPMaPTimer::_timer_off_value_ = -1;


SIPMaPTimer::SIPMaPTimer(const int sialx_lines): max_slots_(sialx_lines){
	timer_list_ = new long long[max_slots_]();		// Paren causes it to be initialized to 0;
	timer_switched_ = new long long[max_slots_]();
}

SIPMaPTimer::~SIPMaPTimer() {}


void SIPMaPTimer::record_time(int line_number, long time){
	timer_list_[line_number] += time;
	timer_switched_[line_number] ++;
}

void SIPMaPTimer::print_timers(const std::vector<std::string>& line_to_str, std::ostream& out) {
	out << "Timers for Program " << GlobalState::get_program_name() << std::endl;
	long long * timers = timer_list_;
	long long * timer_counts = timer_switched_;
	const int LW = 10;	// Line num
	const int CW = 15;	// Time
	const int SW = 20;	// String

	out << "Timers" << std::endl
			<< std::setw(LW) << std::left << "Line"
			<< std::setw(SW) << std::left << "Type"
			<< std::setw(CW) << std::left << "Avg"
			<< std::setw(CW) << std::left << "Tot" << std::endl;

	out.precision(6); // Reset precision to 6 places.
	for (int i = 1; i < max_slots_; i++) {
		if (timer_counts[i] > 0L) {
			double tot_time = to_seconds(timers[i]);	// Microsecond to second
			double avg_time = tot_time / timer_counts[i];

			out << std::setw(LW) << std::left << i
					<< std::setw(SW) << std::left << line_to_str.at(i)
					<< std::setw(CW) << std::left << avg_time
					<< std::setw(CW) << std::left << tot_time << std::endl;
		}
	}
	out << std::endl;
}


} /* namespace sip */
