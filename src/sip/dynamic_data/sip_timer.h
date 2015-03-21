/*
 * sip_timer.h
 *
 *  Created on: Feb 4, 2015
 *      Author: njindal
 */

#ifndef SIP_TIMER_H_
#define SIP_TIMER_H_

#include "config.h"
#include "sip.h"

#include <string>
#include <cstddef>
#include <iostream>


namespace sip {

/** Simple timer to measure named SIP events */
class Timer {
public:
	Timer(const std::string name, bool register_timer);
	Timer(const std::string name);

	void start() {
		CHECK(!on_,"starting on timer" + name_);
		start_time_ = GETTIME;
		on_=true;
	}
	double pause() {
		double curr = GETTIME;
		double elapsed = curr - start_time_;
		value_ = value_ + elapsed;
		epochs_++;
		on_= false;
		return elapsed;
	}
	double get_value(){return value_;}
	std::size_t get_num_epochs(){return epochs_;}
	void set_value(double value){value_ = value;}
	std::string name(){return name_;}

	static void print_timers(std::ostream& out);
	static void clear_list(){list_.clear();}

	static void set_register_by_default(bool v) { register_by_default_ = v;}

private:
	bool on_;					/*! if timer is turned on */
	double start_time_;			/*! time when timer was started */
	double value_;		 		/*!	total time in seconds */
	const std::string name_;	/*! name of counter */
	std::size_t epochs_;		/*! number of times, timer was switched on & off */

	static std::vector<Timer*> list_;
	static bool register_by_default_;
};

} /* namespace sip */

#endif /* SIP_TIMER_H_ */
