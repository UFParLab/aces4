/*
 * sip_counter.cpp
 *
 *  Created on: Feb 2, 2015
 *      Author: njindal
 */

#include <sip_counter.h>

namespace sip {

Counter::Counter(const std::string& name) :name_(name), counter_(0) {}

MaxCounter::MaxCounter(const std::string& name) :name_(name), counter_(0), max_(-1) {}


CounterFactory::~CounterFactory(){
	// Delete all allocated Counters
	for (std::vector<Counter*>::iterator it = counters_.begin(); it!= counters_.end(); ++it){
		Counter * counter = *it;
		delete counter;
	}
	// Delete all allocated MaxCounters
	for (std::vector<MaxCounter*>::iterator it = max_counters_.begin(); it!= max_counters_.end(); ++it){
		MaxCounter * max_counter = *it;
		delete max_counter;
	}
}

MaxCounter* CounterFactory::getNewMaxCounter(const std::string& name){
	MaxCounter* max_counter = new MaxCounter(name);
	max_counters_.push_back(max_counter);
	return max_counter;
}


Counter* CounterFactory::getNewCounter(const std::string& name){
	Counter* counter = new Counter(name);
	counters_.push_back(counter);
	return counter;
}


} /* namespace sip */
