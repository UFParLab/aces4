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

class SialOpsSequential {
public:
	SialOpsSequential(DataManager &,
			WorkerPersistentArrayManager*,
			SipTables&);
	~SialOpsSequential();

	/** implements a global SIAL barrier */
	void sip_barrier();

	/** SIAL operations on arrays */
	void create_distributed(int array_id);
	void restore_distributed(int array_id, IdBlockMap<Block>* bid_map);
	void delete_distributed(int array_id);
	void get(BlockId&);
	void put_replace(BlockId&, const Block::BlockPtr);
	void put_accumulate(BlockId&, const Block::BlockPtr);

	void destroy_served(int array_id);
	void request(BlockId&);
	void prequest(BlockId&, BlockId&);
	void prepare(BlockId&, Block::BlockPtr);
	void prepare_accumulate(BlockId&, Block::BlockPtr);

	void collective_sum(int source_array_slot, int dest_array_slot);

	void set_persistent(Interpreter*, int array_id, int string_slot);
	void restore_persistent(Interpreter*, int array_id, int string_slot);

	void end_program();

	/**
	 * Logs type of statement and line number
	 * @param type
	 * @param line
	 */
	void log_statement(opcode_t type, int line);

	/** wrapper around these methods in the block_manager.  Checks for data
	 * races due to missing barrier and implements the wait for blocks of
	 * distributed and served arrays.
	 *
	 * @param id
	 * @return
	 */
	Block::BlockPtr get_block_for_reading(const BlockId& id);

	Block::BlockPtr get_block_for_writing(const BlockId& id,
			bool is_scope_extent = false);

	Block::BlockPtr get_block_for_updating(const BlockId& id);

private:

	SipTables& sip_tables_;
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
