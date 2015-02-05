/*
 * server_timer.cpp
 *
 *  Created on: Feb 4, 2015
 *      Author: njindal
 */

#include <server_timer.h>

#include <iomanip>

#include "global_state.h"

namespace sip {

ServerTimer::ServerTimer(int max_slots) :
		max_slots_(max_slots), list_(max_slots, ServerUnitTimer()) {
}

void ServerTimer::print_timers(std::ostream& out_, const SipTables& sip_tables){
	out_ << "Timers for Program " << GlobalState::get_program_name() << std::endl;
	const int LW = 8;			// Line Number & PC Width
	const int SW = 25;			// String
	const int CW = 12;			// Time

	out_<<std::setw(LW)<<std::left<<"PC"
		<<std::setw(LW)<<std::left<<"Line"
		<<std::setw(SW)<<std::left<<"Name"
		<<std::setw(CW)<<std::left<<"Time"
		<<std::setw(CW)<<std::left<<"BlkWtTime"
		<<std::setw(CW)<<std::left<<"DiskRdTime"
		<<std::setw(CW)<<std::left<<"DiskWrtTime"
		<<std::setw(CW)<<std::left<<"Epochs"
		<<std::endl;

	std::vector<ServerUnitTimer>::const_iterator it = list_.begin();
	for (int i=0; it != list_.end(); ++it, ++i){
		const ServerUnitTimer& timer = *it;
		opcode_t opcode = sip_tables.op_table().opcode(i);
		int line_number = sip_tables.op_table().line_number(i);
		const std::string& name = opcodeToName(opcode);
		double total_time = timer.get_total_time();
		double block_wait_time = timer.get_block_wait_time();
		double disk_read_time = timer.get_disk_read_time();
		double disk_write_time = timer.get_disk_write_time();
		std::size_t epochs = timer.get_num_epochs();
		out_<< std::setw(LW)<< std::left << i		// PC
			<< std::setw(LW)<< std::left << line_number
			<< std::setw(SW)<< std::left << name
			<< std::setw(CW)<< std::left << total_time
			<< std::setw(CW)<< std::left << block_wait_time
			<< std::setw(CW)<< std::left << disk_read_time
			<< std::setw(CW)<< std::left << disk_write_time
			<< std::setw(CW)<< std::left << epochs
			<< std::endl;
	}

	out_ << std::endl;
}

} /* namespace sip */
