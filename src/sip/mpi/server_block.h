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
 * For each operation, represeting IOD as 3 bits as initial state,
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

	~ServerBlock();

	void set_dirty() { status_[ServerBlock::dirtyInMemory] = true; }
	void set_in_memory() { status_[ServerBlock::inMemory] = true; }
	void set_on_disk() { status_[ServerBlock::onDisk] = true; }

	void unset_dirty() { status_[ServerBlock::dirtyInMemory] = false; }
	void unset_in_memory() { status_[ServerBlock::inMemory] = false; }
	void unset_on_disk() { status_[ServerBlock::onDisk] = false; }

	bool is_dirty() { return status_[ServerBlock::dirtyInMemory]; }
	bool is_in_memory() { return status_[ServerBlock::inMemory]; }
	bool is_on_disk() { return status_[ServerBlock::onDisk]; }

	dataPtr get_data() { return data_; }
	void set_data(dataPtr data) { data_ = data; }

	int size() { return size_; }

    dataPtr accumulate_data(size_t size, dataPtr to_add); /*! for all elements, this->data += to_add */


    void free_in_memory_data();						/*! Frees FP data allocated in memory, sets status */
    void allocate_in_memory_data(bool init=true); 	/*! Allocs mem for FP data, optionally initializes to 0*/


	static bool limit_reached();			/*! Whether mem limit reached. Should start writing to disk if true */
	static std::size_t remaining_memory();	/*! maximum allocatable mem less used mem (for FP data only) */

	friend std::ostream& operator<< (std::ostream& os, const ServerBlock& block);

private:
    /** Constructs a block, allocating size number 
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

    const int size_;/**< Number of elements in block */
	dataPtr data_;	/**< Pointer to block of data */

	enum ServerBlockStatus {
		inMemory		= 0,	// Block is on host
		onDisk			= 1,	// Block is on device (GPU)
		dirtyInMemory 	= 2,	// Block dirty on host
	};
	std::bitset<3> status_;

	const static std::size_t field_members_size_;
	const static std::size_t max_allocated_bytes_;
	static std::size_t allocated_bytes_;

	friend SIPServer;
	friend DiskBackedArraysIO;
	friend DiskBackedBlockMap;
	friend IdBlockMap<ServerBlock>;
};

} /* namespace sip */

#endif /* SERVER_BLOCK_H_ */
