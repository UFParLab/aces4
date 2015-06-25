/*
 * race_detection_interpreter.cpp
 *
 *  Created on: Mar 27, 2015
 *      Author: njindal
 */

#include <race_detection_interpreter.h>
#include "loop_manager.h"

namespace sip {

RaceDetectionInterpreter::RaceDetectionInterpreter(int worker_rank,
		int num_workers, const SipTables& sipTables,  BarrierBlockConsistencyMap& barrier_block_consistency_map) :
		AbstractControlFlowInterpreter(worker_rank, num_workers, sipTables),
				barrier_block_consistency_map_(barrier_block_consistency_map),
				last_seen_barrier_pc_(0){
}

RaceDetectionInterpreter::~RaceDetectionInterpreter() {}

void RaceDetectionInterpreter::handle_sip_barrier_op(int pc) { last_seen_barrier_pc_++; }

void RaceDetectionInterpreter::handle_program_end(int& pc){}

void RaceDetectionInterpreter::handle_get_op(int pc) {
	PardoSectionConsistencyInfo& pardo_sections_info = barrier_block_consistency_map_[last_seen_barrier_pc_];
	ArrayIdDeletedMap& array_id_deleted_map = pardo_sections_info.array_id_deleted_map;
	ArrayBlockConsistencyMap& blocks_consistency_map_ =  pardo_sections_info.blocks_consistency_map_;
	BlockId id = get_block_id_from_selector_stack();
	int array_id = id.array_id();
	if (array_id_deleted_map[array_id]){
		std::stringstream err_ss;
		err_ss << "From worker " << worker_rank_
				<< " ,trying to GET block "<< id
				<< " of array " << sip_tables_.array_name(array_id)
				<< " which has been deleted at line " << line_number();
		fail_with_exception(err_ss.str(), line_number());
	}
	BlockIdConsistencyMap& array_consistency_map = blocks_consistency_map_[array_id];
	DistributedBlockConsistency& distributed_block_consistency_map = array_consistency_map[id];
	bool consistent = distributed_block_consistency_map.update_and_check_consistency(SIPMPIConstants::GET, worker_rank_);
	if (!consistent){
		std::stringstream err_ss;
		err_ss << "Incorrect GET Block Semantics for " << id << " from worker " << worker_rank_ << " at line " << line_number();
		fail_with_exception(err_ss.str(), line_number());
	}
}

void RaceDetectionInterpreter::put_sum_into_block_semantics(const BlockId &lhs_id) {
	PardoSectionConsistencyInfo& pardo_sections_info = barrier_block_consistency_map_[last_seen_barrier_pc_];
	ArrayIdDeletedMap& array_id_deleted_map = pardo_sections_info.array_id_deleted_map;
	ArrayBlockConsistencyMap& blocks_consistency_map_ = pardo_sections_info.blocks_consistency_map_;
	int array_id = lhs_id.array_id();
	if (array_id_deleted_map[array_id]) {
		std::stringstream err_ss;
		err_ss << "From worker " << worker_rank_
				<< " ,trying to PUT_ACCUMULATE block " << lhs_id << " of array "
				<< sip_tables_.array_name(array_id)
				<< " which has been deleted at line " << line_number();
		fail_with_exception(err_ss.str(), line_number());
	}
	BlockIdConsistencyMap& array_consistency_map = blocks_consistency_map_[array_id];
	DistributedBlockConsistency& distributed_block_consistency_map = array_consistency_map[lhs_id];
	bool consistent = distributed_block_consistency_map.update_and_check_consistency(SIPMPIConstants::PUT_ACCUMULATE, worker_rank_);
	if (!consistent) {
		std::stringstream err_ss;
		err_ss << "Incorrect PUT_ACCUMULATE Block Semantics for " << lhs_id
				<< " from worker " << worker_rank_ << " at line "
				<< line_number();
		fail_with_exception(err_ss.str(), line_number());
	}
}

void RaceDetectionInterpreter::handle_put_accumulate_op(int pc) {
	BlockId rhs_id = get_block_id_from_selector_stack();
	BlockId lhs_id = get_block_id_from_selector_stack();
	put_sum_into_block_semantics(lhs_id);
}

void RaceDetectionInterpreter::handle_put_increment_op(int pc){
	BlockId lhs_id = get_block_id_from_selector_stack();
	put_sum_into_block_semantics(lhs_id);
}
void RaceDetectionInterpreter::handle_put_scale_op(int pc){
	BlockId lhs_id = get_block_id_from_selector_stack();
	put_sum_into_block_semantics(lhs_id);
}

void RaceDetectionInterpreter::put_replace_block_semantics(const BlockId &lhs_id) {
	PardoSectionConsistencyInfo& pardo_sections_info = barrier_block_consistency_map_[last_seen_barrier_pc_];
	ArrayIdDeletedMap& array_id_deleted_map = pardo_sections_info.array_id_deleted_map;
	ArrayBlockConsistencyMap& blocks_consistency_map_ = pardo_sections_info.blocks_consistency_map_;
	int array_id = lhs_id.array_id();
	if (array_id_deleted_map[array_id]) {
		std::stringstream err_ss;
		err_ss << "From worker " << worker_rank_ << " ,trying to PUT block "
				<< lhs_id << " of array " << sip_tables_.array_name(array_id)
				<< " which has been deleted at line " << line_number();
		fail_with_exception(err_ss.str(), line_number());
	}
	BlockIdConsistencyMap& array_consistency_map = blocks_consistency_map_[array_id];
	DistributedBlockConsistency& distributed_block_consistency_map = array_consistency_map[lhs_id];
	bool consistent = distributed_block_consistency_map.update_and_check_consistency(SIPMPIConstants::PUT, worker_rank_);
	if (!consistent) {
		std::stringstream err_ss;
		err_ss << "Incorrect PUT Block Semantics for " << lhs_id
				<< " from worker " << worker_rank_ << " at line "
				<< line_number();
		fail_with_exception(err_ss.str(), line_number());
	}
}

void RaceDetectionInterpreter::handle_put_replace_op(int pc) {
	BlockId rhs_id = get_block_id_from_selector_stack();
	BlockId lhs_id = get_block_id_from_selector_stack();
	put_replace_block_semantics(lhs_id);
}

void RaceDetectionInterpreter::handle_put_initialize_op(int pc) {
	BlockId lhs_id = get_block_id_from_selector_stack();
	put_replace_block_semantics(lhs_id);
}




void RaceDetectionInterpreter::handle_delete_op(int pc){
	int array_id = arg0(pc);
	PardoSectionConsistencyInfo& pardo_sections_info = barrier_block_consistency_map_[last_seen_barrier_pc_];
	ArrayIdDeletedMap& array_id_deleted_map = pardo_sections_info.array_id_deleted_map;
	array_id_deleted_map[array_id] = true;
}



} /* namespace sip */
