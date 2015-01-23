#include "sialx_timer.h"
#include "assert.h"
#include "sip_interface.h"
#include "global_state.h"
#include "print_timers.h"
#include "sialx_print_timer.h"
#include "sip_mpi_attr.h"
#include <iostream>
#include <iomanip>
#include <cstdio>
#include <cstdlib>
#include <sstream>

namespace sip{


//*********************************************************************
// 						Methods for SialxTimers
//*********************************************************************

SialxTimer::SialxTimer(int sialx_lines) :
		delegate_(NUMBER_TIMER_KINDS_*(1 + sialx_lines)),
		sialx_lines_(sialx_lines){
	/* Need NUMBER_TIMER_KINDS_ times the maximum number of slots,
	 * one for each kind defined in the TimerKind_t enum.
	 * One extra added since line numbers begin at 1.
	 * The 0th slot is for the entire program
	 */
}

void SialxTimer::start_timer(int line_number, TimerKind_t kind){
	delegate_.start_timer(line_number + static_cast<int>(kind) * sialx_lines_);
}

void SialxTimer::pause_timer(int line_number, TimerKind_t kind){
	delegate_.pause_timer(line_number + static_cast<int>(kind) * sialx_lines_);
}


long long SialxTimer::get_timer_value(int line_number, TimerKind_t kind){
	const long long * timer_values = delegate_.get_timers();
	return timer_values[line_number + static_cast<int>(kind) * sialx_lines_];
}

long long SialxTimer::get_timer_count(int line_number, TimerKind_t kind){
	const long long * timer_counts = delegate_.get_timer_count();
	return timer_counts[line_number + static_cast<int>(kind) * sialx_lines_];
}


void SialxTimer::print_timers(const std::vector<std::string>& line_to_str, std::ostream& out) {
#ifdef HAVE_TAU
	typedef TAUTimersPrint<SipTimer_t> PrintTimersType_t;
#elif defined HAVE_MPI
	typedef MultiNodePrint<SipTimer_t> PrintTimersType_t;
#else
	typedef SingleNodePrint<SipTimer_t> PrintTimersType_t;
#endif
	PrintTimersType_t p(line_to_str, sialx_lines_, out);
	delegate_.print_timers(p);
}


void SialxTimer::start_program_timer(){
#ifndef HAVE_TAU
	delegate_.start_timer(0);
#endif
}
void SialxTimer::stop_program_timer(){
#ifndef HAVE_TAU
	delegate_.pause_timer(0);
#endif
}


} /* Namespace sip */
