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
#include "counter.h"
#include "sip_mpi_utils.h"
//#include "data_manager.h"
//#include "worker_persistent_array_manager.h"

namespace sip {

class SialxTimer;
class WorkerPersistentArrayManager;
class DataManager;
class SipTables;
class Interpreter;


class SialOpsParallel {
public:



	//PersistentArrayManager is pointer so it won't be
	//deleted in the destructor--it has a lifespan
	//beyond SIAL programs.
	SialOpsParallel(DataManager &,
			WorkerPersistentArrayManager*,
//			SialxTimer*,
			const SipTables&);
	~SialOpsParallel();

	/** implements a global SIAL barrier */
	void sip_barrier(int pc);

	/** SIAL operations on arrays */
	void create_distributed(int array_id, int pc);
	void restore_distributed(int array_id, IdBlockMap<Block>* bid_map, int pc);
	void delete_distributed(int array_id, int pc);
	void get(BlockId&, int pc);
	void put_replace(BlockId&, const Block::BlockPtr, int pc);
	void put_accumulate(BlockId&, const Block::BlockPtr, int pc);
	void put_initialize(BlockId&, double value, int pc);
	void put_increment(BlockId&, double value, int pc);
	void put_scale(BlockId&, double value, int pc);

	void destroy_served(int array_id, int pc);
	void request(BlockId&, int pc);
	void prequest(BlockId&, BlockId&, int pc);
	void prepare(BlockId&, Block::BlockPtr, int pc);
	void prepare_accumulate(BlockId&, Block::BlockPtr, int pc);

	void collective_sum(double rhs_value, int dest_array_slot);
	bool assert_same(int source_array_slot);
	void broadcast_static(Block::BlockPtr block, int source_worker);

	void set_persistent(Interpreter*, int array_id, int string_slot, int pc);
	void restore_persistent(Interpreter*, int array_id, int string_slot, int pc);

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
	 * Get block for reading may block, the current pc is passed in for the block wait timer.
	 *
	 * @param id
	 * @return
	 */
	Block::BlockPtr get_block_for_reading(const BlockId& id, int pc);

	Block::BlockPtr get_block_for_writing(const BlockId& id,
			bool is_scope_extent, int pc);

	Block::BlockPtr get_block_for_updating(const BlockId& id, int pc);


	void reduce() { wait_time_.reduce(); }

	void print_op_table_stats(std::ostream& os,
						const SipTables& sip_tables) const {
		wait_time_.print_op_table_stats_impl(os, sip_tables);
	}


	/** mpi related types and variable */
	//TODOD is this the right place for this?
    const static int id_line_section_size;


    //TODO change line to pc
	struct Put_scalar_op_message_t{
	    double value_;
	    int line_;
	    int section_;
		BlockId id_;
	};

	MPI_Datatype mpi_put_scalar_op_type_;
    MPI_Datatype block_id_type_;
	void initialize_mpi_type();

	friend class Interpreter;
	friend class WorkerStatistics;
private:

	const SipTables& sip_tables_;
	SIPMPIAttr & sip_mpi_attr_;
	DataManager& data_manager_;
	BlockManager& block_manager_;
	WorkerPersistentArrayManager* persistent_array_manager_;

	AsyncAcks ack_handler_;
	BarrierSupport barrier_support_;
	DataDistribution data_distribution_; // Data distribution scheme
	MPIScalarOpType mpi_type_;

	// Instrumentation
	MPITimerList wait_time_; //"block wait time"
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
	 * @param pc  current index in optable. Used to index the wait_time_ timer list.
	 * @return  the input parameter--for convenience
	 */
	Block::BlockPtr wait_and_check(Block::BlockPtr b, int pc);

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

	friend void ::list_blocks_with_number();

	DISALLOW_COPY_AND_ASSIGN(SialOpsParallel);

} //SailOpsParallel
;

} /* namespace sip */

#endif /* SIAL_OPS_PARALLEL_H_ */
