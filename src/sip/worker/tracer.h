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

namespace sip {

class Tracer {
public:
	Tracer(SialxInterpreter* worker, const SipTables& sip_table, std::ostream& out):
		worker_(worker),
		sip_tables_(sip_table),
		out_(out),
		show_int_table_(true),
		show_control_stack_(true),
		opcode_histogram_(last_op),
		pc_histogram_(sip_tables_.op_table_.entries_.size())
    {}
	~Tracer(){}

	void setShowIntTable(bool show){show_int_table_ = show;}

	void trace(int pc, opcode_t opcode){
//		out_ << "%%%%%%%%%%%%%%%%%%%\n" << sip_tables_.op_table_.entries_[pc];
//		out_ << std::endl;
//		if(show_int_table_){
//		out_ << "________IntTable\n" << sip_tables_.int_table_ ;
//		}
//		if(show_control_stack_){
//			int size = worker_->control_stack_.size();
//			if (size > 0)
//				out_ << "control stack top: " <<  worker_->control_stack_.top() << std::endl;
//		}
		opcode_histogram_[opcode]++;
		pc_histogram_[pc]++;
	}


	std::string histogram_to_string(){
		std::stringstream ss;
		ss << "Opcode frequency\n";
		opcode_t pc = goto_op;
		std::vector<int>::iterator it;
		for (it = opcode_histogram_.begin(); it != opcode_histogram_.end(); ++it){
			int val = *it;
			if (val != 0){
				std::string opname = opcodeToName(pc);
				ss << opname << ": " << val << std::endl;
			}
		}
		ss<< "Line (actually optable entry) frequency\n";
				for (it = pc_histogram_.begin(); it != pc_histogram_.end(); ++it){
					int val = *it;
					if (val != 0){
						ss << pc << ": " << val << std::endl;
					}
				}
		return ss.str();
	}

private:
	bool show_int_table_;
	bool show_control_stack_;
	bool show_selector_stack_;
	bool show_expression_stack_;
	const SipTables& sip_tables_;
	SialxInterpreter* worker_;
	std::ostream& out_;
	std::vector<int> opcode_histogram_;  //this records the number of times each opcode has been executed
	                                     //can be used to evaluate test coverage of the sial interpreter.
	std::vector<int> pc_histogram_;      //this records the number of times each line (or optable entry) in the sial program has been executed.
	                                     //can be used to find dead code in a sial program.

	DISALLOW_COPY_AND_ASSIGN(Tracer);
};

} /* namespace sip */

#endif /* TRACER_H_ */
