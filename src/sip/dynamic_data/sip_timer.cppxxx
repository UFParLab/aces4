/*
 * sip_timer.cpp
 *
 *  Created on: Jul 23, 2014
 *      Author: jindal
 */

#include "config.h"
#include "sip.h"
#include <sip_timer.h>
#include <iostream>
#include <cassert>
#include <ctime>
#include <algorithm>

#include "global_state.h"
#include "sip_interface.h"
#include "sip_mpi_attr.h"

#ifdef HAVE_PAPI
	#include "papi.h"
	#define PAPI
#endif

#ifdef HAVE_TAU
	#include <TAU.h>
	#define MAX_TAU_IDENT_LEN 64
#endif

#ifdef HAVE_MPI
	#include "mpi.h"
#endif

namespace sip {


const long long LinuxSIPTimers::_timer_off_value_ = -1;

//*********************************************************************
//								Linux Timers
//*********************************************************************

LinuxSIPTimers::LinuxSIPTimers(int max_slots) : max_slots_(max_slots) {
	timer_list_ = new long long[max_slots];
	timer_on_ = new long long[max_slots];
	timer_switched_ = new long long[max_slots];
	assert (clock() != -1); // Make sure that clock() works

	// Initialize the lists
	std::fill(timer_list_+0, timer_list_+max_slots, 0);
	std::fill(timer_on_+0, timer_on_+max_slots, _timer_off_value_);
	std::fill(timer_switched_+0, timer_switched_+max_slots, 0L);
}

LinuxSIPTimers::~LinuxSIPTimers(){
	delete [] timer_list_;
	delete [] timer_on_;
	delete [] timer_switched_;
}

void LinuxSIPTimers::start_timer(int slot) {
#ifdef HAVE_MPI
	// MPI_Wtime returns time in seconds. We convert this to the CLOCKS_PER_SEC unit
	timer_on_[slot] = (long)(MPI_Wtime() * CLOCKS_PER_SEC);
#else // HAVE_MPI
	// clock() measure CPU time, not walltime. This is ok for the single node version.
	// Use clock(), since that is the most widely available.
	// Consider using Boost or Chrono
	// http://stackoverflow.com/questions/17432502/how-can-i-measure-cpu-time-and-wall-clock-time-on-both-linux-windows
	timer_on_[slot] = clock();
#endif
}

void LinuxSIPTimers::pause_timer(int slot) {
	assert (slot < max_slots_);
	assert (timer_on_[slot] != _timer_off_value_);
#ifdef HAVE_MPI
	timer_list_[slot] += (long)(MPI_Wtime() * CLOCKS_PER_SEC) - timer_on_[slot];
#else
	timer_list_[slot] += clock() - timer_on_[slot];
#endif
	timer_on_[slot] = _timer_off_value_;
	timer_switched_[slot]++;
}

long long* LinuxSIPTimers::get_timers() {
	return timer_list_;
}

long long* LinuxSIPTimers::get_timer_count(){
	return timer_switched_;
}

void LinuxSIPTimers::print_timers (PrintTimers<LinuxSIPTimers>& p){
	p.execute(*this);
}

bool LinuxSIPTimers::check_timers_off() {
	for (int i = 0; i < max_slots_; i++){
		if (timer_on_[i] != _timer_off_value_){
			std::cerr<<"Timer left on : "<<i<<std::endl;
			return false;
		}
	}
	return true;
}


//*********************************************************************
//								PAPI Timers
//*********************************************************************
#ifdef HAVE_PAPI

PAPISIPTimers::PAPISIPTimers(int max_slots_) : LinuxSIPTimers(max_slots_) {
	int EventSet = PAPI_NULL;
	if (PAPI_library_init(PAPI_VER_CURRENT) != PAPI_VER_CURRENT)
		fail("PAPI_library_init failed !");
	if (PAPI_create_eventset(&EventSet) != PAPI_OK)
		fail("PAPI_create_eventset failed !");
}

PAPISIPTimers::~PAPISIPTimers(){
	//Automatically calls LinuxSIPTimers::~LinuxSIPTimers();
}

void PAPISIPTimers::start_timer(int slot) {
	timer_on_[slot] = PAPI_get_real_usec();
}

void PAPISIPTimers::pause_timer(int slot) {
	assert (slot < max_slots_);
	assert (timer_on_[slot] != _timer_off_value_);
	timer_list_[slot] += PAPI_get_real_usec() - timer_on_[slot];
	timer_on_[slot] = _timer_off_value_;
	timer_switched_[slot]++;
}

bool PAPISIPTimers::check_timers_off() {
	return LinuxSIPTimers::check_timers_off();
}

void PAPISIPTimers::print_timers(PrintTimers<PAPISIPTimers>& p){
	p.execute(*this);
}

#endif

//*********************************************************************
//								TAU Timers
//*********************************************************************
#ifdef HAVE_TAU

long TAUSIPTimers::already_allocated_slots_ = 0;

TAUSIPTimers::TAUSIPTimers(int max_slots_) : max_slots_(max_slots_),
		timer_list_(NULL), timer_switched_(NULL){
	tau_timers_ = new void*[max_slots_];
	for (int i=0; i<max_slots_; i++)
		tau_timers_[i] = NULL;

	already_allocated_slots_ += max_slots_;
	previous_max_slots_ += already_allocated_slots_;
}

TAUSIPTimers::~TAUSIPTimers(){
	delete [] tau_timers_;

	if (timer_list_)
		delete [] timer_list_;
	if(timer_switched_)
		delete [] timer_switched_;

}

void TAUSIPTimers::start_timer(int slot) {
	void *timer = tau_timers_[slot];
	if (timer == NULL){
		char name[MAX_TAU_IDENT_LEN];
		sprintf(name, "%ld", previous_max_slots_+slot);
		TAU_PROFILER_CREATE(timer, name, "", TAU_USER);
		tau_timers_[slot] = timer;
	}
	TAU_PROFILER_START(timer);

}

void TAUSIPTimers::pause_timer(int slot) {
	char name[MAX_TAU_IDENT_LEN];
	sprintf(name, "%ld", previous_max_slots_+slot);
	void *timer = tau_timers_[slot];
	sip::check(timer != NULL, "Error in Tau Timer management !", current_line());
	TAU_PROFILER_STOP(timer);
}

void TAUSIPTimers::print_timers(PrintTimers<TAUSIPTimers>& p){
	p.execute(*this);
}

void ** TAUSIPTimers::get_tau_timers(){
	return tau_timers_;
}


long long *TAUSIPTimers::get_timers(){
	if (!timer_list_)
		timer_list_ = new long long[max_slots_]();		// Parenthesis zero-es out all values

	for (int i=0; i<max_slots_; ++i){
		if (tau_timers_[i] != NULL){
			double incl[TAU_MAX_COUNTERS];
			TAU_PROFILER_GET_INCLUSIVE_VALUES(tau_timers_[i], &incl);
			const char **counters;
			int numcounters;
			TAU_PROFILER_GET_COUNTER_INFO(&counters, &numcounters);
			check (numcounters == 1, "Unexpected behavior in TAU");
			timer_list_[i] = incl[0];
		}
	}
	return timer_list_;

}

long long *TAUSIPTimers::get_timer_count() {
	if (!timer_switched_)
		timer_switched_ = new long long[max_slots_]();
	for (int i=0; i<max_slots_; ++i){
		if (tau_timers_[i] != NULL){
			long calls;
			TAU_PROFILER_GET_CALLS(tau_timers_[i], &calls);
			timer_switched_[i] = calls;
		}
	}
	return timer_switched_;
}

#endif // HAVE_TAU


} /* namespace sip */


// Utility Functions to collect & merge LinuxSIPTimers & PAPISIPTimers

#ifdef HAVE_MPI
void sip_timer_reduce_op_function(void* r_in, void* r_inout, int *len, MPI_Datatype *type){
	long long * in = (long long*)r_in;
	long long * inout = (long long*)r_inout;
	for (int l=0; l<*len; l++){
		long long num_timers = in[0];
		CHECK(inout[0] == in[0], "Data corruption when trying to reduce timers !");
		// Sum up the number of times each timer is switched on & off
		// Sum up the the total time spent at each line.
		in++; inout++;	// 0th position has the length
		for (int i=0; i<num_timers*2; i++){
			*inout += *in;
			in++; inout++;
		}
	}
}

#endif // HAVE_MPI

