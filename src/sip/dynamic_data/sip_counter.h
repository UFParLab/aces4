/*
 * sip_counter.h
 *
 *  Created on: Feb 2, 2015
 *      Author: njindal
 */

#ifndef SIP_COUNTER_H_
#define SIP_COUNTER_H_

#include "config.h"
#include <cstddef>
#include <vector>
#include <string>
#include <iostream>

#define MAX(x, y) (x ^ ((x ^ y) & -(x < y)))	// Bit Hacks - http://graphics.stanford.edu/~seander/bithacks.html#IntegerMinOrMax

namespace sip {

class Counter {
public:
	Counter(const std::string& name_, bool register_counter);
	Counter(const std::string& name_);
	void inc() { counter_++ ; }
	void inc(std::size_t delta) { counter_ += delta;  }
	std::size_t get_value() { return counter_; }
	const std::string& name() { return name_; }

	static void print_counters(std::ostream& out);
	static void clear_list(){list_.clear();}

private:
	std::size_t counter_;
	const std::string name_;

	static std::vector<Counter*> list_;

};

class MaxCounter {
public:
	MaxCounter(const std::string& name_, bool register_counter);
	MaxCounter(const std::string& name_);
	void inc() { counter_++ ; max_ = MAX(max_, counter_); }
	void inc(std::size_t delta) { counter_ += delta; max_ = MAX(max_, counter_); }
	void dec() { counter_--; }
	void dec(std::size_t delta) { counter_-= delta; }
	std::size_t get_max() { return max_;}
	std::size_t get_value() { return counter_; }
	const std::string& name() { return name_; }

	static void print_max_counters(std::ostream& out);
	static void clear_list(){list_.clear();}

private:
	std::size_t counter_;
	std::size_t max_;
	const std::string name_;

	static std::vector<MaxCounter*> list_;

};

} /* namespace sip */

#endif /* SIP_COUNTER_H_ */
