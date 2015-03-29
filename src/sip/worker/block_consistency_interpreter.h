/*
 * block_consistency_interpreter.h
 *
 *  Created on: Mar 27, 2015
 *      Author: njindal
 */

#ifndef SIP_WORKER_BLOCK_CONSISTENCY_INTERPRETER_H_
#define SIP_WORKER_BLOCK_CONSISTENCY_INTERPRETER_H_

#include "sialx_interpreter.h"
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
class BlockConsistencyInterpreter: public SialxInterpreter {
public:
	BlockConsistencyInterpreter(int worker_rank, int num_workers, const SipTables& sipTables, BarrierBlockConsistencyMap&);
	virtual ~BlockConsistencyInterpreter();


	virtual void handle_broadcast_static_op(int pc) { fail("handle_broadcast_static not modeled!", line_number()); }
	virtual void handle_allocate_op(int pc) { /* TODO Time to zero out blocks */ }
	virtual void handle_deallocate_op(int pc) {}
	virtual void handle_allocate_contiguous_op(int pc) { fail ("handle_allocate_contiguous_op not modeled!", line_number()); }
	virtual void handle_deallocate_contiguous_op(int pc) { fail ("handle_deallocate_contiguous_op not modeled!", line_number()); }

	virtual void handle_collective_sum_op(int pc) {  expression_stack_.pop(); /* TODO Time for MPI_BCast */ }
	virtual void handle_assert_same_op(int pc) { /* TODO Time for MPI_BCast */ }
	virtual void handle_print_string_op(int pc) { control_stack_.pop();}
	virtual void handle_print_scalar_op(int pc) {  expression_stack_.pop();}
	virtual void handle_print_int_op(int pc) { control_stack_.pop();}
	virtual void handle_print_index_op(int pc) { control_stack_.pop();}
	virtual void handle_print_block_op(int pc) { block_selector_stack_.pop(); }
	virtual void handle_println_op(int pc) {}
	virtual void handle_gpu_on_op(int pc) {}
	virtual void handle_gpu_off_op(int pc) {}
	virtual void handle_set_persistent_op(int pc) {}
	virtual void handle_restore_persistent_op(int pc) {}
	virtual void handle_create_op(int pc) {}

	virtual void handle_block_copy_op(int pc) 		{ block_selector_stack_.pop(); }
	virtual void handle_block_permute_op(int pc) 	{ block_selector_stack_.pop(); block_selector_stack_.pop();}
	virtual void handle_block_fill_op(int pc) 		{ expression_stack_.pop(); }
	virtual void handle_block_scale_op(int pc) 		{ expression_stack_.pop(); }
	virtual void handle_block_scale_assign_op(int pc){ block_selector_stack_.pop(); expression_stack_.pop();}
	virtual void handle_block_accumulate_scalar_op(int pc){ expression_stack_.pop(); }
	virtual void handle_block_add_op(int pc)		{ block_selector_stack_.pop(); block_selector_stack_.pop();}
	virtual void handle_block_subtract_op(int pc)	{ block_selector_stack_.pop(); block_selector_stack_.pop();}
	virtual void handle_block_contract_op(int pc)	{ block_selector_stack_.pop(); block_selector_stack_.pop();}
	virtual void handle_block_contract_to_scalar_op(int pc) { block_selector_stack_.pop(); block_selector_stack_.pop(); expression_stack_.push(0.0);}
	virtual void handle_block_load_scalar_op(int pc){ block_selector_stack_.pop(); expression_stack_.push(0.0);}
	virtual void handle_execute_op(int pc);

	// Methods to override to figure out if blocks are doing the correct thing.

	virtual void handle_pardo_op(int &pc);	//! Overridden to simulate 1 worker at a time
	virtual void handle_sip_barrier_op(int pc);
	virtual void handle_program_end(int &pc);
	virtual void handle_get_op(int pc);
	virtual void handle_put_accumulate_op(int pc);
	virtual void handle_put_replace_op(int pc) ;
	virtual void handle_delete_op(int pc);

private:
	const int worker_rank_;
	const int num_workers_;
	const SipTables& sip_tables_;

	int last_seen_barrier_pc_;

	BarrierBlockConsistencyMap& barrier_block_consistency_map_;

};

} /* namespace sip */

#endif /* SIP_WORKER_BLOCK_CONSISTENCY_INTERPRETER_H_ */
