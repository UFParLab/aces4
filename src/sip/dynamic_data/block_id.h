/*
 * block_id.h
 *
 */

#ifndef BLOCK_ID_H_
#define BLOCK_ID_H_


#include <utility>   // for std::rel_ops.  Allows relational ops for BlockId to be derived from == and <
#include "sip.h"

using namespace std::rel_ops;

namespace sip {

/** A BlockId concretely identifies a block.
 *
 * A BlockId consists of
 * 	 the array_id
 * 	 an array of values for the indices, which select segments
 * 	 a pointer to a parent block, if this ID is for a subblock.  If not a subblock this field is NULL.
 *
 * 	 Because BlockIds are used as keys in various maps, operators < and = have been provided.
 *
 * 	 The parent_id_ptr_ is a pointer to allow (future) arbitrary nesting of subindices.
 * 	 To simplify memory management, this instance will make its own copy of parent_id_ptr_ and delete it in its
 * 	 destructor.  The copy constructor has been provided.
 *
 */
class BlockId {
public:

	static const int MPI_COUNT = MAX_RANK + 1;
	typedef int mpi_block_id_t[MPI_COUNT];


	BlockId();

	/** Constructor for normal blocks.  parent_id_is set to NULL.
	 *
	 * @param array_id
	 * @param index_values
	 */
	BlockId(int array_id, const index_value_array_t& index_values);

	/** Constructor for normal blocks with null parent_id.
	 * Values are in an MAX_RANK + 1 length array were element 0
	 * is the array_id and the rest are the index_values.
	 *
	 * Typically, this constructor is used when the BlockId has been
	 * sent in an MPI message.
	 *
	 * @param buffer
	 */

	BlockId(const mpi_block_id_t buffer);

	/**
	 * Constructor that returns a new BlockId that is the same
	 * as the given one except that the array_id has been replaced.
	 * This is used when restoring arrays.
	 *
	 * Requires parent_id_ptr_ of given BlockId to be null;
	 *
	 * @param array_id
	 * @param old_block
	 */

	BlockId(int array_id, const BlockId& old_block);

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


	/**
	 * Converts this BlockID into an array that can be sent in an MPI
	 * message.  In the current implementation, this is just a
	 * cast. The parent_id_ptr_ field is ignored because distributed/served
	 * arrays are not subarrays.
	 *
	 * @return
	 */
	int* to_mpi_array(){
		return reinterpret_cast<int *>(this);
	}

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


	/**
	 *
	 * @return the array component of this block id
	 */
	int array_id() const {return array_id_;}

	/**
	 *
	 * @param i  the index
	 * @return  thevalue of index i
	 */
	int index_values (int i) const {return index_values_[i];}

	/**
	 *
	 * @return a string representation of this BlockId
	 */
	std::string str();

	friend std::ostream& operator<<(std::ostream&, const BlockId &);

private:
	int array_id_; //ID of the array, represented by the slot the array table
	index_value_array_t  index_values_;  //these are visible in sial.  Unused slots have value unused_index_value
	BlockId * parent_id_ptr_;

	friend class Interpreter;
	friend class BlockManager;
	friend class SIPServer;
	friend class SIPMPIUtils;

};

} /* namespace sip */

#endif /* BLOCK_ID_H_ */
