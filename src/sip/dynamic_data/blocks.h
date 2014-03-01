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
#include <utility>   // for std::rel_ops.  Allows relational ops for BlockId to be derived from == and <
#include "aces_defs.h"
#include "sip.h"
#include "array_constants.h"
#include "sip_interface.h"
#include <iostream>


using namespace std::rel_ops;

namespace sip {
	class Interpreter;
	class SIPServer;
	class SIPMPIUtils;
}

namespace sip {

/** A BlockId concretely identifies a block.
 *
 * A BlockId consists of
 * 	 the array_id
 * 	 an array of values for the indices, which select segments
 * 	 a pointer to a parent block, if this ID is for a subblock.  If not a subblock this field is NULL.
 *
 * 	 The parent_id_ptr_ is a pointer to allow (future) arbitrary nesting of subindices.
 * 	 To simplify memory management, this instance will make its own copy of parent_id_ptr_ and delete it in its
 * 	 destructor.  The copy constructor has been provided.
 *
 * 	 TODO change BlockId * to a smart pointer.
 */
class BlockId {
public:
	BlockId();

	/** Constructor for normal blocks.  parent_id_is set to NULL.
	 *
	 * @param array_id
	 * @param index_values
	 */
	BlockId(int array_id, const index_value_array_t& index_values);

	/** Constructor for subblocks
	 *
	 * @param array_id
	 * @param index_values
	 * @param ID of parent block
	 */
	BlockId(int array_id, const index_value_array_t& index_values, const BlockId&);

	/** Another constructor for normal blocks, taking rank and vector (instead of index_value_array_t)
	 *
	 * @param array_id
	 * @param rank
	 * @param index_values
	 */
	BlockId(int array_id, int rank, const std::vector<int>& index_values);

	/** A constructor for a block ID for a scalar or entire contiguous array
	 *
	 * @param array_id
	 */

	BlockId(int array_id);

	/** Copy constructor.  Performs deep copy
	 *
	 * @param BlockId to copy
	 */
	BlockId(const BlockId&); //copy constructor, performs deep copy

	/** Assign constructor.
	 *
	 * @param tmp
	 * @return
	 */
	BlockId  operator=(BlockId tmp ); //assignment constructor

	/** Destructor
	 * Recursively deletes parent block ids.
	 */
	~BlockId();

	/** Overload ==.  Performs "deep" comparison of all fields.
	 * Overloading == and < is required because we are using BlockIds as
	 * keys in the block map, which is currently an STL map instance,
	 * where map is a binary tree.
	 *
	 * @param rhs
	 * @return
	 */
	bool operator==(const BlockId& rhs) const;

	/** Overload <.
	 *  a < b if the a.array_id_ < b.array_id_ or a.array_id_ = b.array_id_  &&  a.index_values_ < b.index_values
	 *  where index values are compared using lexicographic ordering.
	 * @param rhs
	 * @return
	 */
	bool operator<(const BlockId& rhs) const;

	int array_id() const {return array_id_;}
	int index_values (int i) const {return index_values_[i];}

	friend std::ostream& operator<<(std::ostream&, const BlockId &);

	/**
	 * Serialize to send over the network
	 * @param [in]
	 * @param [out]
	 * @return
	 */
	static int* serialize(const BlockId&, int&);
	/**
	 * Deserialize and construct a BlockId object.
	 * @param
	 * @return
	 */
	static BlockId deserialize(const int*);


	/**
	 * Returns the serialized size
	 * @return
	 */
	static int serialized_size();


private:
	int array_id_; //ID of the array, represented by the slot the array table
	index_value_array_t  index_values_;  //these are visible in sial.  Unused slots have value unused_index_value
	BlockId * parent_id_ptr_;

	friend class sip::Interpreter;
	friend class BlockManager;
	friend class sip::SIPServer;
	friend class sip::SIPMPIUtils;

};


/** Describes the shape of the block in terms of the size of each index */
//TODO Consider adding rank to the shape

class BlockShape {
public:
	BlockShape();
	explicit BlockShape(const segment_size_array_t&);
	~BlockShape();
	segment_size_array_t segment_sizes_;
	bool operator==(const BlockShape& rhs) const;
	bool operator<(const BlockShape& rhs) const;
	int num_elems() const;
	friend std::ostream& operator<<(std::ostream&, const BlockShape &);
	friend class Block;
};

/** The array slot along with the index slots.  Together with the values of the arrays, determines a block.
 * There are two special constants.  The value that indicates an unused index, and tvalue for wildcards.
 * The latter must match the value assumed by the compiler which is 90909.  To ensure proper initialization
 * of unused dimensions, the constructor requires the rank.
 *
 * If the rank of a non-scalar is zero, this should be a contiguous (static) array, and the whole array is selected.
 * Otherwise the rank should be the same as the rank in the array table.
 *
 * Note that the equality operator ignores the array_id.
 */
class BlockSelector {
public:
	BlockSelector(int array_id, int rank, const index_selector_t&);
	int array_id_;
	int rank_;
	index_selector_t index_ids_;
	bool operator==(const BlockSelector& rhs) const;
	friend std::ostream& operator<<(std::ostream&, const BlockSelector &);
};



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
	 * Returns a copy of the block
	 * @param
	 * @return
	 */
	BlockPtr clone();

	~Block();

    int size();
    BlockShape shape();
    dataPtr get_data();
    dataPtr fill(double value);
    dataPtr scale(double value);
//    dataPtr copy_data(BlockPtr source);
    dataPtr copy_data_(BlockPtr source_block, int offset = 0);
    dataPtr transpose_copy(BlockPtr source, int rank, permute_t&);
    dataPtr accumulate_data(BlockPtr source);

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

//    /**
//     * serializes a BlockPtr, returns the size through a parameter and the serialized data.
//     * @param
//     * @param
//     * @return
//     */
//    static char* serialize(BlockPtr, int &);
//
//    /**
//     * Deserializes bytes to a BlockPtr
//     * @param
//     * @param size
//     * @return
//     */
//    static BlockPtr deserialize(char*, int);

private:

    Block();

	BlockShape shape_;
    int size_;
	dataPtr data_;
	dataPtr gpu_data_;

	// Why bitset is a good idea
	// http://www.drdobbs.com/the-standard-librarian-bitsets-and-bit-v/184401382
	enum BlockStatus {
		onHost			= 0,	// Block is on host
		onGPU			= 1,	// Block is on device (GPU)
		dirtyOnHost 	= 2,	// Block dirty on host
		dirtyOnGPU 	= 3		// Block dirty on device (GPU)
	};
	std::bitset<4> status_;


	friend class BlockManager;
	friend class sip::Interpreter;
	friend class ContiguousArrayManager;
	friend class sip::SIPMPIUtils;

	// No one should be using the compare operator.
	// TODO Figure out what to do with the GPU pointer.
	bool operator==(const Block& rhs) const;
	DISALLOW_COPY_AND_ASSIGN(Block);

};


///**
// * Blocks on GPU. Much like array::Block
// */
//class GPUBlock {
//public:
//
//	typedef GPUBlock* GPUBlockPtr;
//	typedef double * dataPtr;
//	typedef int permute_t[MAX_RANK];
//
//	/** Constructs a new Block with the given shape and data
//	 *
//	 * @param  shape
//	 * @param  pointer to data
//	 */
//	GPUBlock(BlockShape, dataPtr);
//
//    int size();
//    dataPtr get_data();
//    BlockShape shape();
//
//	~GPUBlock();
//
//private:
//	BlockShape shape_;
//	int size_;
//	dataPtr data_;
//
//	friend class sip::Interpreter;
//	friend class GPUBlockManager;
//
//	// Explicitly disallow comparing blocks.
//	bool operator==(const GPUBlock& rhs) const;
//
//	DISALLOW_COPY_AND_ASSIGN(GPUBlock);
//
//
//};

} /* namespace sip */

#endif /* BLOCKS_H_ */
