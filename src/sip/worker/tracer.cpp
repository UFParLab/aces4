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
		opcode_histogram_(SIPMPIAttr::get_instance().company_communicator(), last_op-goto_op-1),
		pc_histogram_(SIPMPIAttr::get_instance().company_communicator(), sip_tables.op_table_.size()),
//		total_time_(sip_tables.op_table_.size(),0.0),
//		last_time_ (0.0),
		run_loop_timer_(SIPMPIAttr::get_instance().company_communicator()),
		opcode_timer_(SIPMPIAttr::get_instance().company_communicator(), sip_tables.op_table_.size()),
		last_pc_(0),
		last_opcode_(sip_tables.op_table_.opcode(0)){
}

std::ostream& operator<<(std::ostream& os, const Tracer& obj){
// 		os << "PC Histogram" << std::endl << obj.pc_histogram_ << std::endl;
// 		os << "Opcode Histogram" << std::endl << obj.opcode_histogram_ << std::endl;
// 		os << "Timer output" << std::endl;
// 		os << "run_loop_timer_" << obj.run_loop_timer_ << std::endl;
// 		os << "opcode_timer_"<< obj.opcode_timer_ << std::endl;
 		os << "Worker opcode_timer_" << std::endl;
 		obj.opcode_timer_.print_op_table_stats(os, obj.sip_tables_);
 		return os;
}

//std::string Tracer::histogram_to_string() {
//	std::stringstream ss;
//	ss << "Opcode frequency" << std::endl;
//	int first_op_int = opcodeToInt(goto_op);
//	int opcode_int = first_op_int;
//	int invalid_op_int = opcodeToInt(invalid_op);
//	std::vector<std::size_t>::iterator it;
//	for (it = opcode_histogram_.begin() + first_op_int;
//			it < opcode_histogram_.end(); ++it) {
//		std::size_t val = *it;
//		if (first_op_int <= opcode_int && opcode_int < invalid_op_int
//				&& val != 0) {
//			std::string opname = opcodeToName(intToOpcode(opcode_int));
//			ss << opname << "," << opcode_int << "," << val << std::endl;
//		}
//		opcode_int++;
//
//	}
//	ss << std::endl << "Optable entry frequency" << std::endl;
//	int index = 0;
//	for (it = pc_histogram_.begin(); it != pc_histogram_.end(); ++it) {
//		int val = *it;
//		ss << index++ << "," << val << std::endl;
//	}
//	return ss.str();
//}

//void Tracer::gather_pc_histogram_to_csv(std::ostream& ss) {
//
//
//	const SIPMPIAttr& mpi_attr = SIPMPIAttr::get_instance();
//	int my_worker_rank = mpi_attr.company_rank();
//	int num_workers = mpi_attr.company_size();
//
//	size_t pcsize = pc_histogram_.size();
//	std::vector<size_t> rbuf(num_workers * pcsize,0);
//
//	if (num_workers > 1) {
//		MPI_Gather(pc_histogram_.data(), pcsize, MPI_LONG_LONG, rbuf.data(),
//				pcsize, MPI_LONG_LONG, mpi_attr.worker_master(),
//				mpi_attr.company_communicator());
//	} else {
//		rbuf = pc_histogram_;
//	}
//	if (mpi_attr.is_company_master()) {
//		for (int i = 0; i < pcsize; i++) {
//			ss << sip_tables_.op_table_.line_number(i) << "," << i << ", " << opcodeToName((sip_tables_.op_table_).opcode(i));
//			for (int j = 0; j < num_workers * pcsize; j += pcsize) {
//				ss << "," << rbuf[i + j];
//			}
//			ss << std::endl;
//		}
//	}
//
//

//}

//void Tracer::gather_opcode_histogram_to_csv(std::ostream&  ss) {
//
//
//	const SIPMPIAttr& mpi_attr = SIPMPIAttr::get_instance();
//	int my_worker_rank = mpi_attr.company_rank();
//	int num_workers = mpi_attr.company_size();
//
//	int first_op_int = static_cast<int>(goto_op);
//	int opcode_int = first_op_int;
//	int invalid_op_int = static_cast<int>(invalid_op);
//
//	size_t num_ops = last_op-goto_op-1;
//
//	check(num_ops ==  opcode_histogram_.size(), "unexpected opcode histogram size");
//	std::vector<size_t> rbuf(num_workers * num_ops,0);
//
//	if (num_workers > 1) {
//		MPI_Gather(opcode_histogram_.data(), num_ops, MPI_LONG_LONG,
//				rbuf.data(), num_ops, MPI_LONG_LONG, mpi_attr.worker_master(),
//				mpi_attr.company_communicator());
//	} else {
//		rbuf = opcode_histogram_;
//	}
//
//
//
//	if (mpi_attr.is_company_master()) {
//		for (int i = first_op_int; i < invalid_op_int ; i++) {
//			ss << i << ", " << opcodeToName(static_cast<opcode_t>(i));
//				int pos = i-first_op_int;
//				for (int j = 0; j < num_workers * num_ops; j += num_ops) {
//						ss << "," << rbuf[pos + j];
//					}
//					ss << std::endl;
//				}
//
//}
//
//
//
//
//}


//void Tracer::gather_timing_to_csv(std::ostream& ss) {
//
//
//	const SIPMPIAttr& mpi_attr = SIPMPIAttr::get_instance();
//	int my_worker_rank = mpi_attr.company_rank();
//	int num_workers = mpi_attr.company_size();
//
//	size_t time_size = total_time_.size();
//	std::vector<double> rbuf(num_workers * time_size);
//
//	if (num_workers > 1) {
//
//		MPI_Gather(total_time_.data(), time_size, MPI_DOUBLE, rbuf.data(),
//				time_size, MPI_DOUBLE, mpi_attr.worker_master(),
//				mpi_attr.company_communicator());
//
//	} else {
//		rbuf = total_time_;
//	}
//	if (mpi_attr.is_company_master()) {
//		for (int i = 0; i < time_size; i++) {
////			ss << i << ", " << opcodeToName((sip_tables_.op_table_).opcode(i));
//			ss << sip_tables_.op_table_.line_number(i) << "," << i << ", " << opcodeToName((sip_tables_.op_table_).opcode(i));
//			for (int j = 0; j < num_workers * time_size; j += time_size) {
//				ss << "," << rbuf[i + j];
//			}
//			ss << std::endl;
//		}
//	}



//}
#endif //HAVE_MPI

//	}

} /* namespace sip */

