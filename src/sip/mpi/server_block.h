/*
 * server_block.h
 *
 *  Created on: Mar 25, 2014
 *      Author: njindal
 */

#ifndef SERVER_BLOCK_H_
#define SERVER_BLOCK_H_

#include <bitset>
#include <new>
#include "id_block_map.h"
#include "sip_mpi_constants.h"
#include "mpi_state.h"

namespace sip {

class SIPServer;
class DiskBackedArraysIO;
class DiskBackedBlockMap;

/**
 * Maintains a Block in the memory of a server.
 * Each block maintains its status. The status informs the client of a
 * ServerBlock instance of how to treat it.
 *
 * The "status" of a ServerBlock consists of :
 * a) Whether it is in memory (I)
 * b) Whether it is on disk (O)
 * c) Whether it is dirty in memory (D)
 *
 * The operations that can be performed are:
 * 1. Read Block (get_block_for_reading)
 * 2. Write Block (get_block_for_writing)
 * 3. Update Block (get_block_for_updating)
 * 4. Flush Block To Disk (in case of limit_reached() being true).
 *
 * For each operation, representing IOD as 3 bits as initial state,
 * the action and final state is specified for each operation.
 *
 * // In Memory Operations
 *
 * Operation : Read Block
 * 100 -> No Action 			-> 100
 * 010 -> Alloc, Read from disk	-> 110
 * 101 -> No Action 			-> 101
 * 110 -> No Action 			-> 101
 * 111 -> No Action 			-> 110
 *
 * Operation : Write Block
 * 100 -> No Action 			-> 101
 * 010 -> Alloc					-> 111
 * 101 -> No Action 			-> 101
 * 110 -> No Action 			-> 111
 * 111 -> No Action 			-> 111
 *
 * Operation : Update Block
 * 100 -> No Action 			-> 101
 * 010 -> Alloc, Read from disk	-> 111
 * 101 -> No Action				-> 111
 * 110 -> No Action				-> 111
 * 111 -> No Action				-> 111
 *
 * // On Disk Operations (Assumes memory available)
 *
 * Operation : Write Block to Disk
 * 100 -> Write to disk 		-> 110
 * 010 -> No Action				-> 010
 * 101 -> Write to disk			-> 110
 * 110 -> No Action				-> 110
 * 111 -> Write to disk			-> 110
 *
 * Operation : Read Block from Disk
 * 100 -> No Action				-> 100
 * 010 -> Read from disk		-> 110
 * 101 -> No Action				-> 101
 * 110 -> No Action				-> 110
 * 111 -> No Action				-> 111
 *
 */
class ServerBlock {
public:
	typedef double * dataPtr;
	typedef ServerBlock* ServerBlockPtr;

	/**
	 * Constructs a block, allocating size number
	 * of double precision numbers; optionally
	 * initializes all elements to 0 (default true).
	 * @param size
	 * @param init
	 */
	explicit ServerBlock(int size, bool init=true);
	/**
	 * Constructs a block with a given pointer to
	 * double precision numbers and size. data
	 * parameter can be NULL.
	 * @param size
	 * @param data can be NULL
	 */
	explicit ServerBlock(int size, dataPtr data);

	~ServerBlock();

	void set_dirty() { disk_status_[ServerBlock::DIRTY_IN_MEMORY] = true; }
	void set_in_memory() { disk_status_[ServerBlock::IN_MEMORY] = true; }
	void set_on_disk() { disk_status_[ServerBlock::ON_DISK] = true; }

	void unset_dirty() { disk_status_[ServerBlock::DIRTY_IN_MEMORY] = false; }
	void unset_in_memory() { disk_status_[ServerBlock::IN_MEMORY] = false; }
	void unset_on_disk() { disk_status_[ServerBlock::ON_DISK] = false; }

	bool is_dirty() { return disk_status_[ServerBlock::DIRTY_IN_MEMORY]; }
	bool is_in_memory() { return disk_status_[ServerBlock::IN_MEMORY]; }
	bool is_on_disk() { return disk_status_[ServerBlock::ON_DISK]; }

	/**
	 * Updates and checks the consistency of a server block.
	 * @param type
	 * @param worker
	 * @return false if brought into inconsistent state by operation
	 */
	bool update_and_check_consistency(SIPMPIConstants::MessageType_t operation, int worker);

	/**
	 * Resets consistency status of block to (NONE, OPEN).
	 */
	void reset_consistency_status ();

	dataPtr get_data() { return data_; }
	void set_data(dataPtr data) { data_ = data; }

	int size() { return size_; }

    dataPtr accumulate_data(size_t size, dataPtr to_add); /*! for all elements, this->data += to_add->data */
    dataPtr fill_data(size_t size, double value);
    dataPtr increment_data(size_t size, double delta);
    dataPtr scale_data(size_t size, double factor);

    void free_in_memory_data();						/*! Frees FP data allocated in memory, sets status */
    void allocate_in_memory_data(bool init=true); 	/*! Allocs mem for FP data, optionally initializes to 0*/

    MPIState& state() { return state_; }
	MPI_Request* mpi_request() { return &(state_.mpi_request_); }
	bool test(){ return state_.test(); }
	void wait(){ state_.wait();}

	static std::size_t allocated_bytes();	        /*! maximum allocatable mem less used mem (for FP data only) */

	friend std::ostream& operator<< (std::ostream& os, const ServerBlock& block);

private:
    const int size_;/**< Number of elements in block */
	dataPtr data_;	/**< Pointer to block of data */
    MPIState state_;/**< For blocks busy in async MPI communication */

	enum ServerBlockStatus {
		IN_MEMORY		= 0,	// Block is on host
		ON_DISK			= 1,	// Block is on device (GPU)
		DIRTY_IN_MEMORY	= 2,	// Block dirty on host
	};
	std::bitset<3> disk_status_;


	/**	 Structures and methods to check for block consistency during GET, PUT, PUT+ operations
	 * from different workers in a pardo section
	 */

	enum ServerBlockMode {
		NONE = 2,			// Block not being worked on
		READ = 3, 			// GET/REQUEST done on block
		WRITE = 4, 			// PUT/PREPARE done on block
		ACCUMULATE = 5, 	// PUT+/PREPARE+ done on block
		SINGLE_WORKER = 6,	// PUT/PUT+ and GET done by same worker
		INVALID_MODE = 999
	};

	/** Can convert OPEN to ONE_WORKER and ONE_WORKER to MULTIPLE_WORKER */
	enum ServerBlockWorker {
		OPEN = -3,				// Block not worked on
		ONE_WORKER = -2,		// Just one worker worked on block, UNUSED - actual rank is stored
		MULTIPLE_WORKER = -1,	// More than one worker worked on block
		INVALID_WORKER = -999
	};

	/**
	 * The set of consistent states that a block can be in are shown.
	 * Each row denote a starting state. A state is shown as <ServerBlockMode><ServerBlockWorker>
	 * w is an arbitrary rank. w1 is another arbitrary rank not equal to w.
	 * Each column is an action on a block, the state of the block
	 * is changed to that shown in the row.
	 * In the table, for ServerBlockMode
	 * N = NONE, R = READ, W = WRITE, A = ACCUMULATE, S = SINGLE_WORKER
	 * For ServerBlockWorker
	 * O = OPEN, w = worker rank (ONE_WORKER),  w1 = some other worker, M = MULTIPLE_WORKER
	 *
	 * The positions denoted with 'X' are error conditions.
	 *
	 *
	 *    		  GET,w    PUT,w    PUT_ACC,w   GET,w1     PUT,w1  PUT_ACC,w1
	 *		NO      Rw      Ww         Aw         Rw1        Ww1     Aw1
	 *		Rw      Rw      Sw         Sw         RM          X       X
	 *		RM      RM       X          X          RM         X       X
	 *		Ww      Sw      Sw         Sw          X          X       X
	 *		Aw      Sw      Sw         Aw          X          X       AM
	 *		AM       X       X         AM          X          X       AM
	 *		Sw      Sw      Sw         Sw          X          X       X
	 */

	std::pair<ServerBlockMode, int> consistency_status_; /*! State of block */

	const static std::size_t field_members_size_;
	static std::size_t allocated_bytes_;

	friend DiskBackedArraysIO;
	friend DiskBackedBlockMap;
	friend IdBlockMap<ServerBlock>;
};

} /* namespace sip */

#endif /* SERVER_BLOCK_H_ */
