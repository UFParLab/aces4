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
#include "async_ops.h"
#include "distributed_block_consistency.h"

namespace sip {

class SIPServer;
class DiskBackedArraysIO;
class DiskBackedBlockMap;
class PendingAsyncManager;



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
class ServerBlock;

class DiskBackingState{
public:
	void set_dirty() { disk_status_[DiskBackingState::DIRTY_IN_MEMORY] = true; }
	void set_in_memory() { disk_status_[DiskBackingState::IN_MEMORY] = true; }
	void set_on_disk() { disk_status_[DiskBackingState::ON_DISK] = true; }

	void unset_dirty() { disk_status_[DiskBackingState::DIRTY_IN_MEMORY] = false; }
	void unset_in_memory() { disk_status_[DiskBackingState::IN_MEMORY] = false; }
	void unset_on_disk() { disk_status_[DiskBackingState::ON_DISK] = false; }

	bool is_dirty() { return disk_status_[DiskBackingState::DIRTY_IN_MEMORY]; }
	bool is_in_memory() { return disk_status_[DiskBackingState::IN_MEMORY]; }
	bool is_on_disk() { return disk_status_[DiskBackingState::ON_DISK]; }
private:
	DiskBackingState(){
		disk_status_[IN_MEMORY] = false;
		disk_status_[ON_DISK] = false;
		disk_status_[DIRTY_IN_MEMORY] = false;
	}
	enum ServerBlockStatus {
		IN_MEMORY		= 0,	// Block is on host
		ON_DISK			= 1,	// Block is on device (GPU)
		DIRTY_IN_MEMORY	= 2,	// Block dirty on host
	};
	std::bitset<3> disk_status_;
	friend ServerBlock;
	DISALLOW_COPY_AND_ASSIGN(DiskBackingState);
};

class ServerBlock {
public:
	typedef double * dataPtr;
	typedef ServerBlock* ServerBlockPtr;

	/**
	 * Constructs a block, allocating size number
	 * of double precision numbers; optionally
	 * initializes all elements to 0
	 * @param size
	 * @param init
	 */
	explicit ServerBlock(int size, bool init);
	/**
	 * Constructs a block with a given pointer to
	 * double precision numbers and size. data
	 * parameter can be NULL.
	 * @param size
	 * @param data can be NULL
	 */
	explicit ServerBlock(int size, dataPtr data);

	~ServerBlock();



	/**
	 * Updates and checks the consistency of a server block.
	 * @param type
	 * @param worker
	 * @return false if brought into inconsistent state by operation
	 */
	bool update_and_check_consistency(SIPMPIConstants::MessageType_t operation, int worker, int section){
			return race_state_.update_and_check_consistency(operation, worker, section);
}

    dataPtr get_data() { return data_; }
	void set_data(dataPtr data) { data_ = data; }

	int size() { return size_; }

    dataPtr accumulate_data(size_t size, dataPtr to_add); /*! for all elements, this->data += to_add->data */
    dataPtr fill_data(size_t size, double value);
    dataPtr increment_data(size_t size, double delta);
    dataPtr scale_data(size_t size, double factor);

    void free_in_memory_data();						/*! Frees FP data allocated in memory, sets status */
    void allocate_in_memory_data(bool init); 	/*! Allocs mem for FP data, optionally initializes to 0*/


	void wait(){ async_state_.wait_all();}

	static std::size_t allocated_bytes();	        /*! maximum allocatable mem less used mem (for FP data only) */

	friend std::ostream& operator<< (std::ostream& os, const ServerBlock& block);

private:
    const int size_;/**< Number of elements in block */
	dataPtr data_;	/**< Pointer to block of data */
	ServerBlockAsyncManager async_state_; /** handles async communication operations */
    DiskBackingState disk_state_;
    DistributedBlockConsistency race_state_;


//	const static std::size_t field_members_size_;
	static std::size_t allocated_bytes_;

	friend DiskBackedArraysIO;
	friend DiskBackedBlockMap;
	friend IdBlockMap<ServerBlock>;
	friend PendingAsyncManager;

	DISALLOW_COPY_AND_ASSIGN(ServerBlock);
};

} /* namespace sip */

#endif /* SERVER_BLOCK_H_ */
