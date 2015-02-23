/*
 * sialx_timer.h
 *
 *  Created on: Feb 4, 2015
 *      Author: njindal
 */

#ifndef SIALX_TIMER_H_
#define SIALX_TIMER_H_

#include "config.h"
#include "sip.h"
#include "sip_tables.h"

#include <vector>
#include <cstddef>
#include <iostream>

namespace sip {
/*! This really simple timer just keeps track of epochs & time. */
struct SimpleTimer_t{
	SimpleTimer_t() : on_(false), start_time_(0.0), value_(0.0), epochs_(0), last_recorded_(0.0) {}
	void start() { CHECK(!on_,"starting on timer"); start_time_ = GETTIME; on_ = true; last_recorded_ = 0.0;}
	double pause() { double elapsed = GETTIME - start_time_; value_ += elapsed; on_ = false; epochs_++; last_recorded_ = elapsed; return elapsed; }
	double get_last_recorded_and_clear() { double tmp = last_recorded_; last_recorded_ = 0.0; return tmp; }

	double last_recorded_; 	//! The value of the previously collected elapsed time.
	bool on_;				//! Whether the timer is on or off
	double start_time_;		//! When the timer was started
	double value_;			//! The total time collected at this timer
	std::size_t epochs_;	//! Number of times this timer was switched on and off
};

/*! Measure one unit of a sialx program. This could be a PC or a Line*/
class SialxUnitTimer{
public:
	void start_total_time() { total_time_timer_.start(); }
	double pause_total_time() { return total_time_timer_.pause(); }
	void start_block_wait() { block_wait_timer_.start(); }
	double pause_block_wait() { return block_wait_timer_.pause(); }

	void record_total_time(double val) { total_time_timer_.value_ += val; total_time_timer_.epochs_ += 1; }
	void record_block_wait_time(double val) { block_wait_timer_.value_ += val; }

	double get_total_time() const {return total_time_timer_.value_;}
	double get_block_wait_time() const {return block_wait_timer_.value_; }

	double get_last_recorded_total_time_and_clear() { return total_time_timer_.get_last_recorded_and_clear(); }
	double get_last_recorded_block_wait_and_clear() { return block_wait_timer_.get_last_recorded_and_clear(); }

	std::size_t get_num_epochs() const {return total_time_timer_.epochs_;}

	/*! Sums up other SialxUnitTimer instance into this one, Used when merging SIPMaPTimer instances */
	void accumulate_other(const SialxUnitTimer& other){
		total_time_timer_.value_ += total_time_timer_.value_;
		total_time_timer_.epochs_ += total_time_timer_.epochs_;
		block_wait_timer_.value_ += block_wait_timer_.value_;
		block_wait_timer_.epochs_ += block_wait_timer_.epochs_;
	}

private:
	SimpleTimer_t total_time_timer_;	/*! timer to measure total time (for a sialx PC or line) */
	SimpleTimer_t block_wait_timer_;	/*! timer to measure block wait time */
};

/*! Encapsulates a vector of sial unit timers.
 * An instance of this class is used by the interpreter to measure time per PC or Sial line
 */
class SialxTimer {
public:
	SialxTimer(int max_slots);
	SialxUnitTimer& operator[](int slot) {return list_.at(slot); }
	SialxUnitTimer& timer(int slot) {return list_.at(slot); }
	void print_timers(std::ostream& out, const SipTables& sip_tables) const;
protected:
	SialxTimer(const SialxTimer& other);
	int max_slots_;
	std::vector<SialxUnitTimer> list_;
};

} /* namespace sip */

#endif /* SIALX_TIMER_H_ */
