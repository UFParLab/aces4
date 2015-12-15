/*
 * tracer.cpp
 *
 *  Created on: Jul 24, 2014
 *      Author: basbas
 */

#include "tracer.h"
#include <sstream>
#include <iostream>


namespace sip {

#ifdef HAVE_MPI
Tracer::Tracer(const SipTables& sip_tables) :
		sip_tables_(sip_tables),
//		opcode_histogram_(SIPMPIAttr::get_instance().company_communicator(), last_op-goto_op-1),
//		pc_histogram_(SIPMPIAttr::get_instance().company_communicator(), sip_tables.op_table_.size()),
////		total_time_(sip_tables.op_table_.size(),0.0),
//		last_time_ (0.0),
		run_loop_timer_(SIPMPIAttr::get_instance().company_communicator()),
		opcode_timer_(SIPMPIAttr::get_instance().company_communicator(), sip_tables.op_table_.size()+1),
		last_pc_(0),
		last_opcode_(sip_tables.op_table_.opcode(0)){
}


/* uncomment to print more */
std::ostream& operator<<(std::ostream& os, const Tracer& obj){
//		os << "PC Histogram" << std::endl << obj.pc_histogram_ << std::endl;
//		os << "Opcode Histogram" << std::endl << obj.opcode_histogram_ << std::endl;
// 		os << "Timer output" << std::endl;
 		os << "Worker run_loop_timer_" << std::endl << obj.run_loop_timer_ << std::endl;
// 		os << "opcode_timer_"<< obj.opcode_timer_ << std::endl;
 		os << "Worker opcode_timer_" << std::endl;
// 		os << obj.opcode_timer_ << std::endl << std::endl;
 		obj.opcode_timer_.print_op_table_stats(os, obj.sip_tables_, false);
 		return os;
}

#else


Tracer::Tracer(const SipTables& sip_tables) :
		sip_tables_(sip_tables),
//		opcode_histogram_(last_op-goto_op-1),
//		pc_histogram_(sip_tables.op_table_.size()),
//		total_time_(sip_tables.op_table_.size(),0.0),
//		last_time_ (0.0),
		run_loop_timer_(),
		opcode_timer_(sip_tables.op_table_.size()+1),
		last_pc_(0),
		last_opcode_(sip_tables.op_table_.opcode(0)){
}


/* uncomment to print more */
std::ostream& operator<<(std::ostream& os, const Tracer& obj){
//		os << "PC Histogram" << std::endl << obj.pc_histogram_ << std::endl;
//		os << "Opcode Histogram" << std::endl << obj.opcode_histogram_ << std::endl;
// 		os << "Timer output" << std::endl;
 		os << "Worker run_loop_timer_" << std::endl << obj.run_loop_timer_ << std::endl;
// 		os << "opcode_timer_"<< obj.opcode_timer_ << std::endl;
 		os << "Worker opcode_timer_" << std::endl;
// 		os << obj.opcode_timer_ << std::endl << std::endl;
 		obj.opcode_timer_.print_op_table_stats(os, obj.sip_tables_);
 		return os;
}




#endif //HAVE_MPI



} /* namespace sip */

