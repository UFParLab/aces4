/*
 * sial_ops_sequential.h
 *
 *  Created on: Apr 8, 2014
 *      Author: basbas
 */

#ifndef SIAL_OPS_SEQUENTIAL_H_
#define SIAL_OPS_SEQUENTIAL_H_

#include "sip.h"
#include "sip_tables.h"
#include "block_manager.h"
#include "data_manager.h"
#include "worker_persistent_array_manager.h"

namespace sip {

class SialxTimer;

class SialOpsSequential {
public:
	SialOpsSequential(DataManager &,
			WorkerPersistentArrayManager*,
			SialxTimer*,
			const SipTables&);
	~SialOpsSequential();

	/** implements a global SIAL barrier */
	void sip_barrier(int pc);

	/** SIAL operations on arrays */
	void create_distributed(int array_id, int pc);
	void restore_distributed(int array_id, IdBlockMap<Block>* bid_map, int pc);
	void delete_distributed(int array_id, int pc);
	void get(BlockId&, int pc);
	void put_replace(BlockId&, const Block::BlockPtr, int pc);
	void put_accumulate(BlockId&, const Block::BlockPtr, int pc);
	void put_initialize(const BlockId&, double value, int pc);
	void put_increment(const BlockId&, double value, int pc);
	void put_scale(const BlockId&, double value, int pc);

	void destroy_served(int array_id, int);
	void request(BlockId&, int);
	void prequest(BlockId&, BlockId&, int);
	void prepare(BlockId&, Block::BlockPtr, int);
	void prepare_accumulate(BlockId&, Block::BlockPtr, int);

	void collective_sum(double rhs_value, int dest_array_slot, int pc);
	bool assert_same(int source_array_slot, int pc){return true;}
	void broadcast_static(Block::BlockPtr, int source_worker, int pc){}  //nop for single process version

	void set_persistent(Interpreter*, int array_id, int string_slot, int pc);
	void restore_persistent(Interpreter*, int array_id, int string_slot, int pc);

	void end_program(int pc);



//	/**
//	 * Logs type of statement and line number
//	 * @param type
//	 * @param line
//	 */
//	void log_statement(opcode_t type, int line);

	/** wrapper around these methods in the block_manager.  Checks for data
	 * races due to missing barrier and implements the wait for blocks of
	 * distributed and served arrays.
	 *
	 * @param id
	 * @return
	 */
	Block::BlockPtr get_block_for_reading(const BlockId& id, int pc);

	Block::BlockPtr get_block_for_writing(const BlockId& id, int pc,
			bool is_scope_extent);

	Block::BlockPtr get_block_for_updating(const BlockId& id, int pc);

private:

	const SipTables& sip_tables_;
	DataManager& data_manager_;
	BlockManager& block_manager_;
	WorkerPersistentArrayManager* persistent_array_manager_;

	/**
	 * values for mode_ array
	 */
	enum array_mode {
		NONE, READ, WRITE, UPDATE
	};
	/** vector of mode for each array
	 * Used to detect data races
	 */
	std::vector<array_mode> mode_;


//	/**
//	 * returns true if the mode associated with an array is compatible with
//	 * the given mode.  Updates the array's mode. write-write conflicts are
//	 * not detected since that check must be done on a per-block basis.
//	 *
//	 * @param array_id
//	 * @param mode
//	 * @return
//	 */
//	bool check_and_set_mode(int array_id, array_mode mode);
//
//	/**
//	 * Gets the array id from the block id and invokes the above method
//	 * @param id
//	 * @param mode
//	 * @return
//	 */
//	bool check_and_set_mode(const BlockId& id, array_mode mode);
//
//	/**
//	 * resets the mode of all arrays to NONE.  This should be invoked
//	 * in SIP_barriers.
//	 */
//	void reset_mode();

	DISALLOW_COPY_AND_ASSIGN(SialOpsSequential);

} //SailOpsSequential
;

} /* namespace sip */

#endif /* SIAL_OPS_SEQUENTIAL_H_ */
