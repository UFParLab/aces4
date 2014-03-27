/*
 * contiguous_array_manager.h
 *
 * Classes in this file manage a map containing contiguous arrays.  Currently,
 * these should be predefined before the SIAL program starts, i.e. should be declared
 * in the SIAL program as static.
 *
 * Contiguous arrays are used in two ways:  as one large block containing the entire array, and
 * as subblocks.  Methods are provided for both situations.
 *
 * Only entire contiguous arrays are stored in the map.
 *
 * Subblocks are supported by copying the elements belonging to a subblock into a
 * contiguous subblock.  This can then be handled by super instructions and the
 * interpreter in the same way that other blocks are handled.  If the block is modified,
 * it must be copied back into the enclosing contiguous block.  A WriteBack object is
 * provided to the interpreter, which is responsible for determining when the write-back should occur.
 * Note that this approach may not have the correct semantics if the same block is
 * handled multiple times in one super instruction, and aliases are created:
 * for example execute si a[i,j] a[i,j].  Thus the compiler does not allow the same
 * superinstruction to appear in multiple arguments unless all are reads.
 *
 *
 *  Created on: Oct 5, 2013
 *      Author: Beverly Sanders
 */

#ifndef CONTIGUOUS_ARRAY_MANAGER_H_
#define CONTIGUOUS_ARRAY_MANAGER_H_

#include <map>
#include <vector>
#include <stack>
#include "blocks.h"

namespace sip {
class SipTables;
}

namespace setup {
class SetupReader;
}

namespace sip {

/** Objects of this class record the information necessary for the interpreter to write modified subblocks back into
 * the enclosing contiguous block.
 */
class WriteBack {
public:
	WriteBack(int rank, Block::BlockPtr contiguous_block, Block::BlockPtr block, const offset_array_t& offsets);
	~WriteBack();
	void do_write_back();
	Block::BlockPtr get_block() { return block_; }
	friend std::ostream& operator<<(std::ostream&, const WriteBack&);
private:
	int rank_;
	Block::BlockPtr contiguous_block_;
	Block::BlockPtr block_;
	offset_array_t offsets_;
	bool done_;
	DISALLOW_COPY_AND_ASSIGN(WriteBack);
};

typedef  std::vector<WriteBack*> WriteBackList;

/** Manages contiguous (static) arrays
 *
 */
class ContiguousArrayManager {
public:
	typedef std::map<int, Block::BlockPtr> ContiguousArrayMap;  //maps array_ids to arrays
	typedef std::map<int, Block::BlockPtr>::iterator ContiguousArrayMapIterator;

	/** Creates a new ContiguousArrayManager and populates it with static
	 * arrays declared in the SIAL program represented by the given sipTables
	 *
	 * @param sipTables
	 */
	ContiguousArrayManager(sip::SipTables&, setup::SetupReader&);
	~ContiguousArrayManager();


	/**
	 * Inserts given block into the continguous_array_map with the given array_id.
	 * If a block with that name already exists, it is replaced with warning.
	 * Requires block and block->data to both be non-null.
	 *
	 * @param array_id
	 * @param block
	 * @return
	 */
	Block::BlockPtr insert_contiguous_array(int array_id, Block::BlockPtr block);

	/**
	 * Allocates memory for a new contiguous array with the given id, and records it in the contiguous_array_map.
	 * The shape of the array, is obtained from the sipTables.
	 * The contents are initialized to zero.
	 *
	 * Requires that this array does not already exist in the map.
	 *
	 * @param array_id slot of array to create
	 * @return BlockPtr to the Block containing the entire array
	 */
	Block::BlockPtr create_contiguous_array(int array_id);

	/**
	 * Creates a contiguous array with the given id and data, and records it in the contiguous_array_map.
	 * The shape of the array is obtained from the sipTables.
	 * This does not allocate new memory for the block data and copy, rather it takes ownership of the
	 * data array via the pointer and wraps it in a Block object.
	 *
	 * Usually, the data will come from the setup file.
	 *
	 * Requires that the array does not already exist in the map.
	 *
	 * @param array_id
	 * @param data  Block::dataPtr pointer to data contained in this array.
	 * @return pointer to new Block containing the entire array
	 */
	Block::BlockPtr create_contiguous_array(int array_id, Block::dataPtr data);

//	/**
//	 * Removes the given contiguous array from the map and deletes its data array
//	 * @param array_id
//	 */
//	void remove_contiguous_array(int array_id);


	/** Gets the indicated subblock of a contiguous array.  This is accomplished by allocating memory for the
	 * subblock and copying the elements, resulting in a contiguously allocated subblock that can
	 * be treated uniformly with blocks of other SIAL arrays.
	 *
	 * Since this subblock is intended to be modified, it must be written back to the enclosing contiguous array.
	 * This is accomplished by creating a WriteBack object for the block and appending it to the given WriteBackList.
	 * It is the reponsibility of the interpreter to invoke the do_write_back method for each object in the list
	 * at an appropriate time.
	 *
	 * ????? is responsible for deleting the subblock allocated in this method
	 *
	 * @param [in] block_id  the ID of the desired block
	 * @param [inout] the list of blocks to write back when an instruction is finished. A write_back object for the returned
	 * 				subblock is added to this list
	 * @return BlockPtr referring to contiguous copy of desired subblock of this array.
	 */
	Block::BlockPtr get_block_for_updating(const BlockId&, WriteBackList&);

	/** Gets the indicated subblock of a contiguous array.  This is accomplished by allocating memory for the
	 * subblock and copying the elements, resulting in a contiguously allocated subblock that can
	 * be treated uniformly with blocks of other SIAL arrays.
	 *
	 * Since this subblock is not intended to be modified, it does not need to be written back.
	 *
	 * ????? is responsible for deleting the subblock allocated in this method
	 *
	 * @param [in] block_id  the ID of the desired block
	 * @return BlockPtr referring to contiguous copy of desired subblock of this array.
	 */
	Block::BlockPtr get_block_for_reading(const BlockId&);

	/** Returns a pointer to a Block containing the entire contiguous array, or NULL if the array does not exist.
	 *
	 * @param array_id  the array_table slot of the desired array
	 * @return  BlockPtr to a  Block containing the entire contiguous array or NULL if the array does not exist.
	 */
	Block::BlockPtr get_array(int array_id);


	/**
	 * Returns a pointer to a Block containing the contiguous array, or NULL if the array does not exist.
	 * Also removes this array from the contiguous array map.  This routine is used to move the
	 * contiguous array into another data structure, such as persistent array manager.
	 *
	 * @param array_id
	 * @return
	 */
	Block::BlockPtr get_and_remove_array(int array_id);


	friend std::ostream& operator<<(std::ostream&, const ContiguousArrayManager&);


/**TODO is this needed?  correct?? */
	friend void print_static_array(int& array_slot, int& rank, int* index_values, int& size, int* extents,  double* data, int& ierr);  //this doesn't seem to work

private:

	/** Performs the actually work to get a contiguous copy of a subblock.
	 * Several values that are obtained in the
	 * course of the method are returned for possible use by callers.
	 *
	 * @param [in] Id of subblock to create
	 * @param [out] rank
	 * @param [out] contiguous BlockPtr to contiguous array that contains the indicated subblock.
	 * @param [out] block BlockPtr to contiguous copy of subblock
	 * @param offsets [out] array containing offsets in each of first element of subblock in containing array.
	 */
	void get_block(const BlockId&, int& rank, Block::BlockPtr& contiguous, Block::BlockPtr& block, sip::offset_array_t& offsets);

	/** map from array slot number to block containing contiguous array */
	ContiguousArrayMap contiguous_array_map_;
	sip::SipTables & sip_tables_;
	setup::SetupReader & setup_reader_;


	DISALLOW_COPY_AND_ASSIGN(ContiguousArrayManager);
};

} /* namespace sip */
#endif /* CONTIGUOUS_ARRAY_MANAGER_H_ */
