/**
 * blocks.h
 *
 * This file contains several classes used to represent blocks.
 *
 * BlockId:  uniquely identifies a block and is used to look up the block in the
 * block map.  The BlockId contains the array name, index values, and if a subblock,
 * a pointer to the parent BlockId.
 *
 * BlockShape:  wraps an array with the number of elements in the block for
 * each index.  Unused indices are given the value 1, so that Shape objects don't depend
 * on knowing the rank.
 *
 * BlockSelector: contains the array id and the ids of each index.  One can obtain a
 * BlockId from a BlockSelector by looking up the current values of each index.
 *
 * Block: contains the shape, size (for convenience), and a pointer to the actual data
 * of the block. Various primitive operations on blocks are implemented in this class.
 *
 *  Created on: Aug 8, 2013
 *      Author: Beverly Sanders
 */

#ifndef BLOCKS_H_
#define BLOCKS_H_

#include "config.h"
#include <bitset>
//#include <utility>   // for std::rel_ops.  Allows relational ops for BlockId to be derived from == and <
#include "aces_defs.h"
#include "sip.h"
#include "array_constants.h"
#include "sip_interface.h"
#include "block_shape.h"
#include <iostream>

#ifdef HAVE_MPI
#include "mpi_state.h"
#endif //HAVE_MPI

namespace sip {
	class Interpreter;
	class SIPServer;
	class SIPMPIUtils;
	class SialOpsParallel;
}

namespace sip {
class Block;






/** A block of data together with its shape. Size is derived from the shape
 * and is stored for convenience.  The shape of a block cannot change once
 * created, although the data may.  In contrast to the other classes, which are
 * small enough to freely copy, Blocks need to manage their data carefully.  Also,
 * generally, data structures will hold Block pointers, allowing for polymorphism
 * later if we ever want to have some other kind of block later.
 *
 * The BlockManager is solely responsible for creating and destroying blocks
 * and freeing and allocating data memory.
 */
class Block {
public:

	typedef Block* BlockPtr;
	typedef double * dataPtr;
	typedef int permute_t[MAX_RANK];


    /** Constructs a new Block with the given shape and allocates memory for its data
     * @param shape of new Block
     */
	explicit Block(BlockShape);

    /** Constructs a new Block with the given shape and data
     *
     * @param  shape
     * @param  pointer to data
     */
	Block(BlockShape, dataPtr);

	/** Constructs a new Block for a scalar.  size_ and all elements of the shape are set to 1
	 * This allows scalars to be handled uniformly with non-scalars
	 *
	 * @param pointer to scalar
	 */
	Block(dataPtr);

	/**
	 * Deletes data in block if any.  If an MPI request associated with this
	 * block is pending, it waits until it has been satisfied and issues a warning.
	 */
	~Block();

    int size();
    const BlockShape& shape();
    dataPtr get_data();
    dataPtr fill(double value);
    dataPtr scale(double value);
    dataPtr copy_data_(BlockPtr source_block, int offset = 0);
    dataPtr transpose_copy(BlockPtr source, int rank, permute_t&);
    dataPtr accumulate_data(BlockPtr source);
    dataPtr increment_elements(double delta);

    /*!extracts slice from this block and copies to the given destination block, which must exist and have memory for data allocated, Returns dest*/
	dataPtr extract_slice(int rank, offset_array_t& offsets, BlockPtr destination);

	/*!inserts given source block into this one */
    void insert_slice(int rank, offset_array_t& offsets, BlockPtr source);

	friend std::ostream& operator<<(std::ostream&, const Block &);

	/**
	 * Frees up host data and sets appropriate status flags
	 */
	void free_host_data();

	/**
	 * Allocate memory for host
	 */
	void allocate_host_data();

	// GPU specific super instructions.
#ifdef HAVE_CUDA
	/**
	 * Factory to get a block with memory allocated on the GPU.
	 * @param shape of new Block
	 */
	static Block::BlockPtr new_gpu_block(BlockShape shape);

    dataPtr get_gpu_data();
    dataPtr gpu_fill(double value);
    dataPtr gpu_scale(double value);
    dataPtr gpu_copy_data(BlockPtr source);
    dataPtr gpu_transpose_copy(BlockPtr source, int rank, permute_t&);
    dataPtr gpu_accumulate_data(BlockPtr source);

    // To manipulate status flags
    bool is_on_host()			{return status_[Block::onHost];}
    bool is_on_gpu()			{return status_[Block::onGPU];}
    bool is_dirty_on_host()		{return status_[Block::dirtyOnHost];}
    bool is_dirty_on_gpu()		{return status_[Block::dirtyOnGPU];}
    bool is_dirty_on_all()		{return status_[Block::dirtyOnHost] && status_[Block::dirtyOnGPU];}

    void set_on_host()			{status_[Block::onHost] = true;}
    void set_on_gpu()			{status_[Block::onGPU] = true;}
    void set_dirty_on_gpu()		{status_[Block::dirtyOnGPU] = true;}
    void set_dirty_on_host()	{status_[Block::dirtyOnHost] = true;}

    void unset_on_host()		{status_[Block::onHost] = false;}
    void unset_on_gpu()			{status_[Block::onGPU] = false;}
    void unset_dirty_on_gpu()	{status_[Block::dirtyOnGPU] = false;}
    void unset_dirty_on_host()	{status_[Block::dirtyOnHost] = false;}

    /**
     * Frees up gpu data and sets appropriate flags.
     */
    void free_gpu_data();

    /**
     * Allocate memory on gpu for data.
     */
    void allocate_gpu_data();

#endif

#ifdef HAVE_MPI
	MPIState& state() { return state_; }
#endif

private:

	BlockShape shape_;
    int size_;
	dataPtr data_;

	//TODO encapsulate this
	dataPtr gpu_data_;


	// Why bitset is a good idea
	// http://www.drdobbs.com/the-standard-librarian-bitsets-and-bit-v/184401382
	enum BlockStatus {
		onHost			= 0,	// Block is on host
		onGPU			= 1,	// Block is on device (GPU)
		dirtyOnHost 	= 2,	// Block dirty on host
		dirtyOnGPU 	    = 3		// Block dirty on device (GPU)
	};
	std::bitset<4> status_;

	// No one should be using the compare operator.
	// TODO Figure out what to do with the GPU pointer.
	bool operator==(const Block& rhs) const;


	/** encapsulates MPI related state info.
	 * This is defined last so its destructor will be called first.
	 *
	 * TODO get rid of the type defs.
	 */
#ifdef HAVE_MPI
	MPIState state_;
//#else
//	EmptyMPIState state_;
#endif

	friend class DataManager;	// So that data_ of blocks wrapping
								// Scalars can be set to NULL before destroying them.
	DISALLOW_COPY_AND_ASSIGN(Block);
};


} /* namespace sip */

#endif /* BLOCKS_H_ */
