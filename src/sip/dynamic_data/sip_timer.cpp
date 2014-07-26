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

#ifdef HAVE_PAPI
	#include "papi.h"
	#define PAPI
#endif

#ifdef HAVE_TAU
	#include <TAU.h>
	#define MAX_TAU_IDENT_LEN 64
#endif

namespace sip {

const long long LinuxSIPTimers::_timer_off_value_ = -1;

//*********************************************************************
//								Linux Timers
//*********************************************************************

LinuxSIPTimers::LinuxSIPTimers(int max_slots) : max_slots(max_slots) {
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
	timer_on_[slot] = clock();
}

void LinuxSIPTimers::pause_timer(int slot) {
	assert (slot < max_slots);
	assert (timer_on_[slot] != _timer_off_value_);
	timer_list_[slot] += clock() - timer_on_[slot];
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
	for (int i = 0; i < max_slots; i++)
		if (timer_on_[i] != _timer_off_value_){
			SIP_LOG(std::cerr<<"Timer left on : "<<i<<std::endl);
			return false;
		}
	return true;
}


//*********************************************************************
//								PAPI Timers
//*********************************************************************
#ifdef HAVE_PAPI

PAPISIPTimers::PAPISIPTimers(int max_slots) : LinuxSIPTimers(max_slots) {
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
	assert (slot < max_slots);
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
TAUSIPTimers::TAUSIPTimers(int max_slots) : max_slots(max_slots) {
	tau_timers_ = new void*[max_slots];
	for (int i=0; i<max_slots; i++)
		tau_timers_[i] = NULL;
}

TAUSIPTimers::~TAUSIPTimers(){
	delete [] tau_timers_;

}

void TAUSIPTimers::start_timer(int slot) {
	void *timer = tau_timers_[slot];
	if (timer == NULL){
		char name[MAX_TAU_IDENT_LEN];
		sprintf(name, "%d : Line %d", sip::GlobalState::get_program_num(), slot);
		TAU_PROFILER_CREATE(timer, name, "", TAU_USER);
		tau_timers_[slot] = timer;
	}
	TAU_PROFILER_START(timer);

}

void TAUSIPTimers::pause_timer(int slot) {
	char name[MAX_TAU_IDENT_LEN];
	sprintf(name, "%d : Line %d", sip::GlobalState::get_program_num(), slot);
	void *timer = tau_timers_[slot];
	sip::check(timer != NULL, "Error in Tau Timer management !", current_line());
	TAU_PROFILER_STOP(timer);
}

void TAUSIPTimers::print_timers(PrintTimers<TAUSIPTimers>& p){
	p.execute(*this);
}
#endif // HAVE_TAU


} /* namespace sip */
