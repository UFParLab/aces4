/*
 * tracer.h
 *
 *  Created on: Jul 24, 2014
 *      Author: basbas
 */

#ifndef TRACER_H_
#define TRACER_H_

#include "sip.h"
#include "opcode.h"
#include "sip_tables.h"

#ifdef HAVE_MPI
#include "sip_mpi_attr.h"
#endif

namespace sip {

class Tracer {
public:
	Tracer(const SipTables&);
	~Tracer(){}

	//call this before starting optable loop
void init_trace(){
	last_time_ = MPI_Wtime();

}
//put at bottom of loop (to handle initialization properly)
void trace_op(int pc, opcode_t opcode){
	    opcode_histogram_[last_opcode_-goto_op]= opcode_histogram_[last_opcode_-goto_op]+1;
		pc_histogram_[last_pc_]= pc_histogram_[last_pc_]+1;
		double time = MPI_Wtime();
		total_time_[last_pc_] += (time-last_time_);
		last_time_ = time;
		last_pc_ = pc;
		last_opcode_ = opcode;
	}


	void gather_pc_histogram_to_csv(std::ostream&);
	void gather_opcode_histogram_to_csv(std::ostream&);
	void gather_timing_to_csv(std::ostream&);

private:

	std::vector<std::size_t> opcode_histogram_;  //this records the number of times each opcode has been executed
	                                     //can be used to evaluate test coverage of the sial interpreter.
	std::vector<std::size_t> pc_histogram_;      //this records the number of times each line (or optable entry) in the sial program has been executed.
	                                     //can be used to find dead code in a sial program.
	std::vector<double> total_time_;
	double last_time_;
	int last_pc_;
	opcode_t last_opcode_;

	const SipTables& sip_tables_;

	DISALLOW_COPY_AND_ASSIGN(Tracer);
};

} /* namespace sip */

#endif /* TRACER_H_ */
