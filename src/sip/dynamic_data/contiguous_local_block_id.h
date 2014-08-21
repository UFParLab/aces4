/*
 * contiguous_local_block_id.h
 *
 *  Created on: Aug 20, 2014
 *      Author: basbas
 */

#ifndef CONTIGUOUS_LOCAL_BLOCK_ID_H_
#define CONTIGUOUS_LOCAL_BLOCK_ID_H_


#include <utility>   // for std::rel_ops.  Allows relational ops for BlockId to be derived from == and <
#include "sip.h"

using namespace std::rel_ops;

namespace sip {
class SipTables;

class ContiguousLocalBlockId {

public:

	//TODO Is this necessary?
	ContiguousLocalBlockId();

	/** Constructor
	 *
	 * @param array_id
	 * @param lower_index_values
	 * @param upper_index_values
	 */
	ContiguousLocalBlockId(int array_id,
			const index_value_array_t& lower_index_values,
			const index_value_array_t& upper_index_values);

	/** Another constructor for normal blocks, taking rank and vectors (instead of index_value_array_t)
	 *
	 * @param array_id
	 * @param rank
	 * @param lower_index_values
	 * @param upper_index_values
	 */
	ContiguousLocalBlockId(int array_id, int rank,
			const std::vector<int>& lower_index_values_vector,
			const std::vector<int>& upper_index_values_vector);

	/** Copy constructor.  Performs deep copy
	 *
	 * @param ContiguousLocalBlockId to copy
	 */
	ContiguousLocalBlockId(const ContiguousLocalBlockId&); //copy constructor, performs deep copy

	/** Assign constructor.
	 *
	 * @param tmp
	 * @return
	 */
	ContiguousLocalBlockId  operator=(ContiguousLocalBlockId tmp ); //assignment constructor

	/** Destructor
	 */
	~ContiguousLocalBlockId();

	/** Overload ==.  Performs "deep" comparison of all fields.
	 * Overloading == and < is required because we are using ContiguousLocalBlockIds as
	 * keys in the block map, which is currently an STL map instance,
	 * where map is a binary tree.
	 *
	 * @param rhs
	 * @return
	 */
	bool operator==(const ContiguousLocalBlockId& rhs) const;

	/** Overload <.
	 *  a < b if the a.array_id_ < b.array_id_ or a.array_id_ = b.array_id_
	 *       &&  a.upper_index_values_ < b.lower_index_values
	 *  where index values are compared using lexicographic ordering.
	 *
	 *  If contiguous locals overlap, then neither a<b nor b<a.
	 *
	 * @param rhs
	 * @return
	 */
	bool operator<(const ContiguousLocalBlockId& rhs) const;

	/** compares ranges.  Like == except that the array id is not compared.
	 *
	 * @param rhs
	 * @return
	 */
	bool equal_ranges(const ContiguousLocalBlockId& rhs) const;

	bool encloses(const ContiguousLocalBlockId& rhs) const;

	bool disjoint(const ContiguousLocalBlockId& rhs) const;

	bool well_formed();
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
	int lower_index_values (int i) const {return lower_index_values_[i];}
	int upper_index_values (int i) const {return upper_index_values_[i];}

	/**
	 *
	 * @return a string representation of this ContiguousLocalBlockId.
	 * Uses the SipTable to look up the name.
	 */
	std::string str(const SipTables& sip_tables) const;

	friend std::ostream& operator<<(std::ostream&, const ContiguousLocalBlockId &);

private:
	int array_id_; //ID of the array, represented by the slot the array table
	index_value_array_t  lower_index_values_;  //these are visible in sial.  Unused slots have value unused_index_value
	index_value_array_t  upper_index_values_;

	/** checks that the lower < upper for each index*/


	friend class Interpreter;
	friend class BlockManager;
	friend class SIPServer;
	friend class SIPMPIUtils;
	friend class SipTables;
	friend class DataDistribution;
	friend class ContiguousLocalArrayManager;

};

} /* namespace sip */

#endif /* CONTIGUOUS_LOCAL_BLOCK_ID_H_ */
