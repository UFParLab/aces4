/*
 * sialx_print_timer.cpp
 *
 *  Created on: Oct 31, 2014
 *      Author: njindal
 */

#include "sialx_print_timer.h"


#ifdef HAVE_MPI

/**
 * MPI_Reduce Reduction function for Timers
 * @param r_in
 * @param r_inout
 * @param len
 * @param type
 */
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

#endif // HAVE_MPI
