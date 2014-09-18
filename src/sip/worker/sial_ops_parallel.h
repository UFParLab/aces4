/*
 * sial_ops_parallel.h
 *
 */

#ifndef SIAL_OPS_PARALLEL_H_
#define SIAL_OPS_PARALLEL_H_

#include <mpi.h>
#include "sip.h"
//#include "sip_tables.h"
#include "barrier_support.h"
#include "async_acks.h"
#include "sip_mpi_attr.h"
#include "block_manager.h"
#include "data_distribution.h"
//#include "data_manager.h"
//#include "worker_persistent_array_manager.h"

namespace sip {

class SialxTimer;
class WorkerPersistentArrayManager;
class DataManager;
class SipTables;

class SialOpsParallel {
public:
	//PersistentArrayManager is pointer so it won't be
	//deleted in the destructor--it has a lifespan
	//beyond SIAL programs.
	SialOpsParallel(DataManager &,
			WorkerPersistentArrayManager*,
			SialxTimer*,
			const SipTables&);
	~SialOpsParallel();

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

	void collective_sum(double rhs_value, int dest_array_slot);
	bool assert_same(int source_array_slot);
	void broadcast_static(int source_array_slot, int source_worker);

	void set_persistent(Interpreter*, int array_id, int string_slot);
	void restore_persistent(Interpreter*, int array_id, int string_slot);

	void end_program();

//	void print_to_ostream(std::ostream& out, const std::string& to_print);

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
	 * Get block for reading may block, the current line is passed in for the block wait timer.
	 *
	 * @param id
	 * @return
	 */
	Block::BlockPtr get_block_for_reading(const BlockId& id, int line);

	Block::BlockPtr get_block_for_writing(const BlockId& id,
			bool is_scope_extent = false);

	Block::BlockPtr get_block_for_updating(const BlockId& id);

private:

	const SipTables& sip_tables_;
	SIPMPIAttr & sip_mpi_attr_;
	DataManager& data_manager_;
	BlockManager& block_manager_;
	WorkerPersistentArrayManager* persistent_array_manager_;
	SialxTimer* sialx_timers_;

	AsyncAcks ack_handler_;
	BarrierSupport barrier_support_;
	DataDistribution data_distribution_; // Data distribution scheme

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

	/**
	 * checks that the number of double values received is the same as expected.
	 * If not, it is a fatal error
	 */
	void check_double_count(MPI_Status& status, int expected_count);

	/**
	 * If there is a pending request for block b, wait for it
	 * to be satisfied.  Otherwise, return immediately.
	 *
	 * Requires b != NULL
	 * @param b
	 * @param line the current line number in sial code. Used to reference timer for block wait time.
	 * @return  the input parameter--for convenience
	 */
	Block::BlockPtr wait_and_check(Block::BlockPtr b, int line);

	/**
	 * returns true if the mode associated with an array is compatible with
	 * the given mode.  Updates the array's mode. write-write conflicts are
	 * not detected since that check must be done on a per-block basis.
	 *
	 * @param array_id
	 * @param mode
	 * @return
	 */
	bool check_and_set_mode(int array_id, array_mode mode);

	/**
	 * Gets the array id from the block id and invokes the above method
	 * @param id
	 * @param mode
	 * @return
	 */
	bool check_and_set_mode(const BlockId& id, array_mode mode);

	/**
	 * resets the mode of all arrays to NONE.  This should be invoked
	 * in SIP_barriers.
	 */
	void reset_mode();

	bool nearlyEqual(double a, double  b, double epsilon);

	DISALLOW_COPY_AND_ASSIGN(SialOpsParallel);

} //SailOpsParallel
;

} /* namespace sip */

#endif /* SIAL_OPS_PARALLEL_H_ */
