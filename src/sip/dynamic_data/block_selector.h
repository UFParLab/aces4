/*
 * block_selector.h
 *
 *  Created on: Apr 6, 2014
 *      Author: basbas
 */

#ifndef BLOCK_SELECTOR_H_
#define BLOCK_SELECTOR_H_

#include "sip.h"
namespace sip {


/** The array slot along with the index slots.  Together with the values of the arrays, determines a block.
 * There are two special constants.  The value that indicates an unused index, and the value for wildcards.
 * The latter must match the value assumed by the compiler which is 90909.  These are defined in
 * core/array_constants.h.
 *
 * To ensure proper initialization
 * of unused dimensions, the constructor requires the rank.
 *
 * If the rank of a non-scalar is zero, this should be a contiguous (static) array, and the whole array is selected.
 * Otherwise the rank should be the same as the rank in the array table.
 *
 * Note that the equality operator ignores the array_id.
 */
class BlockSelector {
public:
	BlockSelector(int rank, int array_id, const index_selector_t&);
	int rank_;
	int array_id_;
	index_selector_t index_ids_;
	bool operator==(const BlockSelector& rhs) const;
	friend std::ostream& operator<<(std::ostream&, const BlockSelector &);
};



} /* namespace sip */

#endif /* BLOCK_SELECTOR_H_ */
