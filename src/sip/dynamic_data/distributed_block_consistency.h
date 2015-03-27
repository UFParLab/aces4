/*
 * distributed_block_consistency.h
 *
 *  Created on: Mar 27, 2015
 *      Author: njindal
 */

#ifndef SIP_DYNAMIC_DATA_DISTRIBUTED_BLOCK_CONSISTENCY_H_
#define SIP_DYNAMIC_DATA_DISTRIBUTED_BLOCK_CONSISTENCY_H_

#include "sip.h"
#include "sip_mpi_constants.h"


namespace sip {

class DistributedBlockConsistency {
public:
	DistributedBlockConsistency();
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


private:

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
	 * w is an arbitrary rank. w1 is another arbitrary rank not equal to w1.
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

};

} /* namespace sip */

#endif /* SIP_DYNAMIC_DATA_DISTRIBUTED_BLOCK_CONSISTENCY_H_ */
