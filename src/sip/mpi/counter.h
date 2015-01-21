/*
 * counter.h
 *
 *  Created on: Jan 19, 2015
 *      Author: basbas
 */

#ifndef COUNTER_H_
#define COUNTER_H_


#include <ostream>
#include <vector>
#include "sip.h"
#ifdef HAVE_MPI
#include "sip_mpi_attr.h"
#endif
namespace sip {

class Counter {
typedef  std::vector<Counter*> CounterList;

/* TODO the lists are global variables.  Will fix that later if these classes are useful */

public:
	Counter(std::string name, bool register_counter);
	~Counter();
	void inc(size_t delta=1){value_ +=delta;}
	size_t get_value(){return value_;}
	void set_value(size_t value){value_ = value;}
	void gather_from_company(std::ostream& out);
	std::string name(){return name_;}
	static void gather_all(std::ostream& out);
	static void clear_list(){list_.clear();}

private:
	size_t value_;
	std::string name_;
	static CounterList list_;
};

class MaxCounter {
public:
	MaxCounter(std::string name, bool register_counter);
	~MaxCounter();
	void inc(size_t delta=1){value_+=delta;  max_= (value_>max_)?value_:max_; }
	void dec(size_t delta= 1){value_-=delta;}
	size_t get_value(){return value_;}
	size_t get_max(){return max_;}
	void set_value(size_t value){value_ = value; max_= (value_>max_)?value_:max_ ;}
	void gather_from_company(std::ostream& out);
	std::string name(){return name_;}
	static void gather_all(std::ostream& out);
	static void clear_list(){list_.clear();}

private:
	size_t value_;
	size_t max_;
	std::string name_;
	static std::vector<MaxCounter*> list_;

};

class SimpleTimer {
public:
	SimpleTimer(std::string name, bool register_timer);
	~SimpleTimer();
	void start();
	double pause();
	size_t get_num_epochs(){return num_epochs_;}
    double get_max_epoch(){return max_epoch_;}
	double get_value(){return value_;}
	void set_value(double value){value_ = value;}
	void gather_from_company(std::ostream& out);
	std::string name(){return name_;}
	static void gather_all(std::ostream& out);
	static void clear_list(){list_.clear();}
private:
	bool on_;
	double start_time_;
	double value_; //total time
	double max_epoch_;
	size_t num_epochs_;
	std::string name_;
	static std::vector<SimpleTimer*> list_;
};



} /* namespace sip */

#endif /* COUNTER_H_ */
