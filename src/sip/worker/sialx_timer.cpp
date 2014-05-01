#include "sialx_timer.h"
#include "assert.h"
#include "sip_interface.h"
#include "global_state.h"
#include <iostream>
#include <iomanip>
#include <cstdio>
#include <cstdlib>

#ifdef HAVE_TAU
#include <TAU.h>
#define MAX_TAU_IDENT_LEN 50
#endif

#ifdef HAVE_MPI

void sialx_timer_reduce_op_function(void* r_in, void* r_inout, int *len, MPI_Datatype *type){
	long long * in = (long long*)r_in;
	long long * inout = (long long*)r_inout;
	for (int l=0; l<*len; l++){
		long long num_timers = in[0];
		sip::check(inout[0] == in[0], "Data corruption when trying to reduce timers !");
		// Sum up the number of times each timer is switched on & off
		// Sum up the the total time spent at each line.
		in++; inout++;	// 0th position has the length
		for (int i=0; i<num_timers*2; i++){
			*inout += *in;
			in++; inout++;
		}
	}
}

#endif

namespace sip{

SialxTimer::SialxTimer(int max_slots){

	max_slots_ = max_slots + 1;	// To account for line number beginning with 1

#ifdef HAVE_TAU
	//tau_timers_ = (void**)malloc(sizeof(void *) * max_slots_);
	tau_timers_ = new void*[max_slots_];
	for (int i=0; i<max_slots_; i++)
		tau_timers_[i] = NULL;
#else
	timer_list_ = new long long[max_slots_];
	timer_on_ = new long long[max_slots_];
	timer_switched_ = new long long[max_slots_];

#ifdef PAPI
	int EventSet = PAPI_NULL;
	if (PAPI_library_init(PAPI_VER_CURRENT) != PAPI_VER_CURRENT)
	  exit(1);
	if (PAPI_create_eventset(&EventSet) != PAPI_OK)
	  exit(1);
#else
	assert (clock() != -1); // Make sure that clock() works
#endif

	// Initialize the lists
	for (int i=0; i<max_slots_; i++)
		timer_list_[i] = 0;	// No ticks yet

	for (int i=0; i<max_slots_; i++)
		timer_on_[i] = _timer_off_value_;

	for (long i=0; i<max_slots_; i++)
		timer_switched_[i] = 0L;

#ifdef HAVE_MPI
	// The data will be structured as
	// Length of arrays 1 & 2
	// Array1 -> timer_switched_ array
	// Array2 -> timer_list_ array
	sip::SIPMPIUtils::check_err(MPI_Type_contiguous(max_slots_*2+1, MPI_LONG_LONG, &sialx_timer_reduce_dt));
	sip::SIPMPIUtils::check_err(MPI_Type_commit(&sialx_timer_reduce_dt));
	sip::SIPMPIUtils::check_err(MPI_Op_create((MPI_User_function *)sialx_timer_reduce_op_function, 1, &sialx_timer_reduce_op));
#endif

#endif	// TAU

}

SialxTimer::~SialxTimer(){
#ifdef HAVE_TAU
	//free(tau_timers_);
	delete [] tau_timers_;
#else
	delete [] timer_list_;
	delete [] timer_on_;
	delete [] timer_switched_;
#ifdef HAVE_MPI
	sip::SIPMPIUtils::check_err(MPI_Type_free(&sialx_timer_reduce_dt));
	sip::SIPMPIUtils::check_err(MPI_Op_free(&sialx_timer_reduce_op));
#endif
#endif // TAU
}

void SialxTimer::start_timer(int slot){
#ifdef HAVE_TAU
	char name[MAX_TAU_IDENT_LEN];
	sprintf(name, "%d : Line %d", sip::GlobalState::get_program_num(), slot);
//	TAU_START(name);
	void *timer = tau_timers_[slot];
	if (timer == NULL){
		TAU_PROFILER_CREATE(timer, name, "", TAU_USER);
		tau_timers_[slot] = timer;
	}
	TAU_PROFILER_START(timer);

#else
	assert (slot < max_slots_);
	assert (timer_on_[slot] == _timer_off_value_);
#ifdef PAPI
	timer_on_[slot] = PAPI_get_real_usec();
#else
	timer_on_[slot] = clock();
#endif
#endif	// TAU
}

void SialxTimer::pause_timer(int slot){
#ifdef HAVE_TAU
	char name[MAX_TAU_IDENT_LEN];
	sprintf(name, "%d : Line %d", sip::GlobalState::get_program_num(), slot);
	//TAU_STOP(name);
	void *timer = tau_timers_[slot];
	sip::check(timer != NULL, "Error in Tau Timer management !", current_line());
	TAU_PROFILER_STOP(timer);
#else
	assert (slot < max_slots_);
	assert (timer_on_[slot] != _timer_off_value_);
#ifdef PAPI
	timer_list_[slot] += PAPI_get_real_usec() - timer_on_[slot];
#else
	timer_list_[slot] += clock() - timer_on_[slot];
#endif
	timer_on_[slot] = _timer_off_value_;
	timer_switched_[slot]++;
#endif //TAU
}

bool SialxTimer::check_timers_off(){
#ifndef HAVE_TAU
	for (int i=0; i<max_slots_; i++)
		if (timer_on_[i] != _timer_off_value_)
			return false;
	return true;
#endif
}


void SialxTimer::print_timers(std::vector<std::string> line_to_str) {
#ifndef HAVE_TAU
	const int LW = 10;	// Line num
	const int CW = 15;	// String
	const int SW = 20;	// Time

	assert(check_timers_off());

	std::cout<<"Timers"<<std::endl;

	std::cout<<std::setw(LW)<<std::left<<"Line"
			<<std::setw(SW)<<std::left<<"Type"
			<<std::setw(CW)<<std::left<<"Total"
			<<std::setw(CW)<<std::left<<"Average"
			<<std::endl;
	for (int i=0; i<max_slots_; i++){
		if (timer_switched_[i] > 0L){
#ifdef PAPI
			double tot_time = (double)timer_list_[i] / 1000000;	// Microsecond to second
#else
			double tot_time = (double)timer_list_[i] / CLOCKS_PER_SEC;
#endif
			double avg_time = tot_time / timer_switched_[i];
//			std::cout<< i <<" \t "<< line_to_str[i] << " \t "<< tot_time <<" \t "<< avg_time << std::endl;
			std::cout<<std::setw(LW)<<std::left << i
					<< std::setw(SW)<< std::left << line_to_str.at(i)
			        << std::setw(CW)<< std::left << tot_time
			        << std::setw(CW)<< std::left << avg_time
			        << std::endl;
		}
	}
	std::cout<<std::endl;
#endif // TAU
}


#ifdef HAVE_MPI
void SialxTimer::mpi_reduce_timers(){
#ifndef HAVE_TAU
	sip::SIPMPIAttr &attr = sip::SIPMPIAttr::get_instance();
	sip::check(attr.is_worker(), "Trying to reduce timer on a non-worker rank !");

	// Data to send to reduce
	long long * sendbuf = new long long[2*max_slots_ + 1];
	sendbuf[0] = max_slots_;
	std::copy(timer_switched_ + 0, timer_switched_ + max_slots_, sendbuf+1);
	std::copy(timer_list_ + 0, timer_list_ + max_slots_, sendbuf+1+max_slots_);

	long long * recvbuf = new long long[2*max_slots_ + 1]();

	int worker_master = attr.worker_master();
	MPI_Comm worker_company = attr.company_communicator();
	sip::SIPMPIUtils::check_err(MPI_Reduce(sendbuf, recvbuf, 1, sialx_timer_reduce_dt, sialx_timer_reduce_op, worker_master, worker_company));

	if (attr.is_company_master()){
		std::copy(recvbuf+1, recvbuf+1+max_slots_, timer_switched_);
		std::copy(recvbuf+1+max_slots_, recvbuf+1+2*max_slots_, timer_list_);
	}

	// Cleanup
	delete [] sendbuf;
	delete [] recvbuf;
#endif	// TAU
}
#endif	// MPI

#ifndef HAVE_TAU
std::ostream& operator<<(std::ostream& os, const SialxTimer& obj) {
	os << "SialxTimer [\n";
	os << "max slots : " << obj.max_slots_ << std::endl;
	os << "timer_switched { " ;
	for (int i=0; i<obj.max_slots_; i++)
		os << obj.timer_switched_[i] << " ";
	os << "}" << std::endl;
	os << "timer_on { " ;
	for (int i=0; i<obj.max_slots_; i++)
		os << obj.timer_on_[i] << " ";
	os << "}" << std::endl;
	os << "timer_list {" ;
	for (int i=0; i<obj.max_slots_; i++)
		os << obj.timer_list_[i] << " ";
	os << " }" << std::endl;
	os << "]" << std::endl;
	return os;
}
#endif

}
