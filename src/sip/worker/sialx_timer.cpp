/*
 * sialx_timer.cpp
 *
 *  Created on: Feb 4, 2015
 *      Author: njindal
 */

#include <sialx_timer.h>

#include <iomanip>
#include "global_state.h"

namespace sip {

SialxTimer::SialxTimer(int max_slots) :
		max_slots_(max_slots), list_(max_slots, SialxUnitTimer()) {
}

SialxTimer::SialxTimer(const SialxTimer& other):
		max_slots_(other.max_slots_), list_(other.list_){
}


void SialxTimer::print_timers(std::ostream& out_, const SipTables& sip_tables) const {
	out_ << "SialxTimers for Program " << GlobalState::get_program_name() << std::endl;
	const int LW = 8;			// Line Number & PC Width
	const int SW = 25;			// String
	const int CW = 12;			// Time

	out_<<std::setw(LW)<<std::left<<"PC"
		<<std::setw(LW)<<std::left<<"Line"
		<<std::setw(SW)<<std::left<<"Name"
		<<std::setw(CW)<<std::left<<"Time"
		<<std::setw(CW)<<std::left<<"BlkWtTime"
		<<std::setw(CW)<<std::left<<"CommTime"
		<<std::setw(CW)<<std::left<<"Epochs"
		<<std::endl;

	double sum_of_total_time = 0.0;
	double sum_of_blkwt_time = 0.0;
	double sum_of_comm_time = 0.0;
	double sum_of_epochs = 0.0;

	std::vector<SialxUnitTimer>::const_iterator it = list_.begin();
	for (int i=0; it != list_.end(); ++it, ++i){
		const SialxUnitTimer& timer = *it;
		opcode_t opcode = sip_tables.op_table().opcode(i);
		int line_number = sip_tables.op_table().line_number(i);
		std::string name = opcodeToName(opcode);
		if (opcode == execute_op){
			int func_slot = sip_tables.op_table().arg0(i);
			name = sip_tables.special_instruction_manager().name(func_slot);
		}
		double total_time = timer.get_total_time();
		sum_of_total_time += total_time;
		double block_wait_time = timer.get_block_wait_time();
		sum_of_blkwt_time += block_wait_time;
		double communication_time = timer.get_communication_time();
		sum_of_comm_time += communication_time;
		std::size_t epochs = timer.get_num_epochs();
		sum_of_epochs += epochs;
		out_<< std::setw(LW)<< std::left << i		// PC
			<< std::setw(LW)<< std::left << line_number
			<< std::setw(SW)<< std::left << name
			<< std::setw(CW)<< std::left << total_time
			<< std::setw(CW)<< std::left << block_wait_time
			<< std::setw(CW)<< std::left << communication_time
			<< std::setw(CW)<< std::left << epochs
			<< std::endl;
	}
	out_<< std::setw(LW)<< std::left << -1		// PC
		<< std::setw(LW)<< std::left << -1
		<< std::setw(SW)<< std::left << "Totals"
		<< std::setw(CW)<< std::left << sum_of_total_time
		<< std::setw(CW)<< std::left << sum_of_blkwt_time
		<< std::setw(CW)<< std::left << sum_of_comm_time
		<< std::setw(CW)<< std::left << sum_of_epochs
		<< std::endl;

	out_ << std::endl;
}


} /* namespace sip */
