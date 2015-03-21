/*
 * sip_timer.cpp
 *
 *  Created on: Feb 4, 2015
 *      Author: njindal
 */

#include <sip_timer.h>

#include <iomanip>
#include <algorithm>

#include "global_state.h"

namespace sip {

std::vector<Timer*> Timer::list_;
bool Timer::register_by_default_ = true;


Timer::Timer(const std::string name) :
		name_(name), on_(false), epochs_(0),
		start_time_(0.0), value_(0.0) {
	if (register_by_default_)
		list_.push_back(this);
}

Timer::Timer(const std::string name, bool register_timer) :
		name_(name), on_(false), epochs_(0),
		start_time_(0.0), value_(0.0) {
	if (register_timer)
#pragma omp critical
		list_.push_back(this);
}

void Timer::print_timers(std::ostream& out_){
	out_ << "Timers for Program " << GlobalState::get_program_name() << std::endl;
	const int SW = 32;			// String
	const int CW = 12;			// Time

	out_<<std::setw(SW)<<std::left<<"Name"
		<<std::setw(CW)<<std::left<<"Time"
		<<std::setw(CW)<<std::left<<"Epochs"
		<<std::endl;

	std::vector<Timer*>::const_iterator it = list_.begin();
	for (; it != list_.end(); ++it){
		Timer* timer = *it;
		std::string name = timer->name();
		std::replace(name.begin(), name.end(), ' ', '_');
		double time = timer->get_value();
		std::size_t epochs = timer->get_num_epochs();
		out_<< std::setw(SW)<< std::left << name
			<< std::setw(CW)<< std::left << time
			<< std::setw(CW)<< std::left << epochs
			<< std::endl;
	}

	out_ << std::endl;
}



} /* namespace sip */
