/*
 * race_detection_interpreter.h
 *
 *  Created on: Mar 27, 2015
 *      Author: njindal
 */

#ifndef SIP_WORKER_RACE_DETECTION_INTERPRETER_H_
#define SIP_WORKER_RACE_DETECTION_INTERPRETER_H_

#include "abstract_control_flow_interpreter.h"
#include "distributed_block_consistency.h"

namespace sip {
class SipTables;

typedef std::map<BlockId, DistributedBlockConsistency> BlockIdConsistencyMap;
typedef std::map<int, BlockIdConsistencyMap> ArrayBlockConsistencyMap;
typedef std::map<int, bool> ArrayIdDeletedMap;
struct PardoSectionConsistencyInfo{
	ArrayBlockConsistencyMap blocks_consistency_map_;
	ArrayIdDeletedMap array_id_deleted_map;
};
typedef std::map<int, PardoSectionConsistencyInfo> BarrierBlockConsistencyMap; // pc of barrier -> info

/**
 * Interprets a program and plays back block GET, PUT, PUT+ and Barriers
 * to make sure program has correct block semantics.
 */
class RaceDetectionInterpreter: public AbstractControlFlowInterpreter {
public:
	RaceDetectionInterpreter(int worker_rank, int num_workers, const SipTables& sipTables, BarrierBlockConsistencyMap&);
	virtual ~RaceDetectionInterpreter();



	virtual SialPrinter* get_printer() { return NULL; }
	virtual void post_interpret(int old_pc, int new_pc) {}
	virtual void pre_interpret(int pc) {}
	virtual void do_post_sial_program() {}

	// Scalar methods to skip
//	virtual void handle_scalar_exp_op(int pc) { expression_stack_.pop(); expression_stack_.pop(); expression_stack_.push(0.0); }
//	virtual void handle_scalar_divide_op(int pc) { expression_stack_.pop(); expression_stack_.pop(); expression_stack_.push(0.0);}

	// Methods to override to figure out if blocks are doing the correct thing.
	virtual void handle_sip_barrier_op(int pc);
	virtual void handle_program_end(int &pc);
	virtual void handle_get_op(int pc);
	virtual void handle_put_accumulate_op(int pc);
	virtual void handle_put_replace_op(int pc) ;
	virtual void handle_put_initialize_op(int pc);
	virtual void handle_put_increment_op(int pc);
	virtual void handle_put_scale_op(int pc);
	virtual void handle_delete_op(int pc);

private:
	int last_seen_barrier_pc_;

	BarrierBlockConsistencyMap& barrier_block_consistency_map_;

	void put_sum_into_block_semantics(const BlockId &lhs_id);
	void put_replace_block_semantics(const BlockId &lhs_id);
};

} /* namespace sip */

#endif /* SIP_WORKER_RACE_DETECTION_INTERPRETER_H_ */
