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
#include "counter.h"

#ifdef HAVE_MPI
#include "sip_mpi_attr.h"
#endif

namespace sip {
class Interpreter;

#ifdef HAVE_MPI

class Tracer {
public:
	explicit Tracer(const SipTables&);
	~Tracer() {	}

	//call this before starting optable loop
	void init_trace() {
		last_time_ = opcode_timer_.get_time();
		run_loop_timer_.start();
	}
	void stop_trace() {
		run_loop_timer_.pause();
	}

	//put at bottom of loop (to handle initialization properly)
	void trace_op(int pc, opcode_t opcode) {
		opcode_histogram_.inc(last_opcode_ - goto_op);
		pc_histogram_.inc(last_pc_);
		double time = opcode_timer_.get_time();

		opcode_timer_.inc(last_pc_, time - last_time_);
		last_time_ = time;
		last_pc_ = pc;
		last_opcode_ = opcode;
	}

	void gather() {
//		pc_histogram_.gather();
//		opcode_histogram_.gather();
		run_loop_timer_.gather();
		run_loop_timer_.reduce();
		opcode_timer_.gather();
		opcode_timer_.reduce();
	}

//	void gather_pc_histogram_to_csv(std::ostream& os){
//		pc_histogram_.gather();
//		os << pc_histogram_ << std::endl;
//	}
//	void gather_opcode_histogram_to_csv(std::ostream& os){
//		opcode_histogram_.gather();
//		os << opcode_histogram_ << std::endl;
//	}
//	void gather_timing_to_csv(std::ostream&);
//	 	 friend class Interpreter;

	friend std::ostream& operator<<(std::ostream& os, const Tracer& obj);

private:

	MPICounterList opcode_histogram_; //this records the number of times each opcode has been executed
									  //can be used to evaluate test coverage of the sial interpreter.
	MPICounterList pc_histogram_; //this records the number of times each line (or optable entry) in the sial program has been executed.
								  //can be used to find dead code in a sial program.
	MPITimer run_loop_timer_;
	MPITimerList opcode_timer_;
	size_t last_pc_;
	opcode_t last_opcode_;
	double last_time_;

	const SipTables& sip_tables_;

	DISALLOW_COPY_AND_ASSIGN(Tracer);
};

#else
class Tracer {
public:
	explicit Tracer(const SipTables&);
	~Tracer() {}

	//call this before starting optable loop
	void init_trace() {
		last_time_ = opcode_timer_.get_time();
		run_loop_timer_.start();
	}
	void stop_trace() {
		run_loop_timer_.pause();
	}

	//put at bottom of loop (to handle initialization properly)
	void trace_op(int pc, opcode_t opcode) {
		opcode_histogram_.inc(last_opcode_ - goto_op);
		pc_histogram_.inc(last_pc_);

		double time = opcode_timer_.get_time();
		opcode_timer_.inc(last_pc_, time - last_time_);
		last_time_ = time;
		last_pc_ = pc;
		last_opcode_ = opcode;
	}

	void gather() {
//		pc_histogram_.gather();
//		opcode_histogram_.gather();
		run_loop_timer_.gather();
		run_loop_timer_.reduce();
		opcode_timer_.gather();
		opcode_timer_.reduce();
	}


	friend std::ostream& operator<<(std::ostream& os, const Tracer& obj);

private:

	SingleNodeCounterList opcode_histogram_; //this records the number of times each opcode has been executed
									  //can be used to evaluate test coverage of the sial interpreter.
	SingleNodeCounterList pc_histogram_; //this records the number of times each line (or optable entry) in the sial program has been executed.
								  //can be used to find dead code in a sial program.
	LinuxTimer run_loop_timer_;
	LinuxTimerList opcode_timer_;
	size_t last_pc_;
	opcode_t last_opcode_;
	double last_time_;

	const SipTables& sip_tables_;

	DISALLOW_COPY_AND_ASSIGN(Tracer);
};

#endif //HAVE_MPI

} /* namespace sip */

#endif /* TRACER_H_ */
