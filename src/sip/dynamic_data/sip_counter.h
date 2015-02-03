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

#define MAX(x, y) (x ^ ((x ^ y) & -(x < y)))	// Bit Hacks - http://graphics.stanford.edu/~seander/bithacks.html#IntegerMinOrMax

namespace sip {

class Counter {
public:
	Counter(const std::string& name_);
	void inc(int slot) { counter_++ ; }
	void inc(int slot, std::size_t delta) { counter_ += delta;  }

private:
	std::size_t counter_;
	const std::string name_;
};

class MaxCounter {
public:
	MaxCounter(const std::string& name_);
	void inc(int slot) { counter_++ ; max_ = MAX(max_, counter_); }
	void inc(int slot, std::size_t delta) { counter_ += delta; max_ = MAX(max_, counter_); }
	void dec(int slot) { counter_--; }
	void dec(int slot, std::size_t delta) { counter_-= delta; }
	std::size_t get_max() { return max_;}

private:
	std::size_t counter_;
	std::size_t max_;
	const std::string name_;
};

class CounterFactory {
public:
	~CounterFactory();
	Counter* getNewCounter(const std::string& name);
	MaxCounter* getNewMaxCounter(const std::string& name);

private:
	std::vector<Counter*> counters_;
	std::vector<MaxCounter*> max_counters_;
};


} /* namespace sip */

#endif /* SIP_COUNTER_H_ */
