/*
 * server_timer.h
 *
 *  Created on: Feb 4, 2015
 *      Author: njindal
 */

#ifndef SERVER_TIMER_H_
#define SERVER_TIMER_H_

#include "config.h"
#include "sip.h"
#include "sip_tables.h"
#include "sialx_timer.h"

#include <vector>
#include <cstddef>
#include <iostream>

namespace sip {
/*! Measured various activities that the server does to service a request from a worker per unit
 * of a sialx program. This could be a PC or a Line
 */
class ServerUnitTimer {
public:
	void start_total_time() { total_time_timer_.start(); }
	double pause_total_time() { return total_time_timer_.pause(); }
	void start_block_wait() { block_wait_timer_.start(); }
	double pause_block_wait() { return block_wait_timer_.pause(); }
	void start_disk_read() { read_disk_timer_.start(); }
	double pause_disk_read() { return read_disk_timer_.pause(); }
	void start_disk_write() { write_disk_timer_.start(); }
	double pause_disk_write() { return write_disk_timer_.pause(); }

	double get_total_time() const {return total_time_timer_.value_;}
	double get_block_wait_time() const {return block_wait_timer_.value_; }
	double get_disk_read_time() const {return read_disk_timer_.value_;}
	double get_disk_write_time() const {return write_disk_timer_.value_; }

	std::size_t get_num_epochs() const {return total_time_timer_.epochs_;}

private:
	SimpleTimer_t total_time_timer_;	/*! timer to measure total time (for a sialx PC or line) */
	SimpleTimer_t block_wait_timer_;	/*! timer to measure block wait time */
	SimpleTimer_t read_disk_timer_;		/*! timer to measure disk read time */
	SimpleTimer_t write_disk_timer_;	/*! timer to measure disk write time */
};

class ServerTimer {
public:
	ServerTimer(int max_slots);
	ServerUnitTimer& operator[](int slot) {return list_.at(slot); }
	ServerUnitTimer& timer(int slot) {return list_.at(slot); }
	void print_timers(std::ostream& out, const SipTables& sip_tables);
private:
	const int max_slots_;
	std::vector<ServerUnitTimer> list_;
};

} /* namespace sip */

#endif /* SERVER_TIMER_H_ */
