/*
 * sip_counter.cpp
 *
 *  Created on: Feb 2, 2015
 *      Author: njindal
 */

#include <sip_counter.h>
#include <iomanip>
#include "global_state.h"

namespace sip {

// Static Variables
std::vector<Counter*>  Counter::list_;
std::vector<MaxCounter*>  MaxCounter::list_;

Counter::Counter(const std::string& name) : name_(name), counter_(0) {
	list_.push_back(this);
}
Counter::Counter(const std::string& name, bool register_counter) : name_(name), counter_(0) {
	if (register_counter)
		list_.push_back(this);
}

MaxCounter::MaxCounter(const std::string& name,  bool register_counter) :name_(name), counter_(0), max_(0) {
	if (register_counter)
		list_.push_back(this);
}

MaxCounter::MaxCounter(const std::string& name) :name_(name), counter_(0), max_(0) {
	list_.push_back(this);
}

void Counter::print_counters(std::ostream& out_){
	out_ << "Counters for Program " << GlobalState::get_program_name() << std::endl;
	const int CW = 12;			// Count
	const int SW = 64;			// String

	out_<<std::setw(SW)<<std::left<<"Name"
		<<std::setw(CW)<<std::left<<"Count"
		<<std::endl;

	std::vector<Counter*>::const_iterator it = list_.begin();
	for (; it != list_.end(); ++it){
		Counter* counter = *it;
		const std::string& name = counter->name();
		std::size_t count = counter->get_value();
		out_<< std::setw(SW)<< std::left << name
			<< std::setw(CW)<< std::left << count
			<< std::endl;
	}

	out_ << std::endl;
}

void MaxCounter::print_max_counters(std::ostream& out_){
	out_ << "MaxCounters for Program " << GlobalState::get_program_name() << std::endl;
	const int CW = 12;			// Count
	const int SW = 64;			// String

	out_<<std::setw(SW)<<std::left<<"Name"
		<<std::setw(CW)<<std::left<<"MaxCount"
		<<std::setw(CW)<<std::left<<"Count"
		<<std::endl;

	std::vector<MaxCounter*>::const_iterator it = list_.begin();
	for (; it != list_.end(); ++it){
		MaxCounter* max_counter = *it;
		const std::string& name = max_counter->name();
		std::size_t max_count = max_counter->get_max();
		std::size_t count = max_counter->get_value();
		out_<< std::setw(SW)<< std::left << name
			<< std::setw(CW)<< std::left << max_count << " "
			<< std::setw(CW)<< std::left << count << " "
			<< std::endl;
	}

	out_ << std::endl;
}


} /* namespace sip */
