/*
 * block_shape.h
 *
 *  Created on: Apr 6, 2014
 *      Author: basbas
 */

#ifndef BLOCK_SHAPE_H_
#define BLOCK_SHAPE_H_

#include <utility>
#include "sip.h"


using namespace std::rel_ops;
namespace sip {

/** Describes the shape of the block in terms of the size of each index
 * Currently, BlockShape does not include the rank, so it expects unused
 * segment sizes to have value 1 */
//TODO Consider adding rank to the shape

class BlockShape {
public:
	BlockShape();
//	explicit BlockShape(const segment_size_array_t&);
	BlockShape(const segment_size_array_t&, int rank);
	~BlockShape();

	bool operator==(const BlockShape& rhs) const;
	bool operator<(const BlockShape& rhs) const;
	int num_elems() const;

	/**
	 * this counts the number of "trailing" ones and subtracts that from
	 * the MAX dim to guess the rank.
	 * @return
	 */
	int get_inferred_rank() const;

	friend std::ostream& operator<<(std::ostream&, const BlockShape &);
	friend class Block;
//	friend class Interpreter;
//	friend class SialPrinter;


//todo  this should be private
	segment_size_array_t segment_sizes_;

private:
//	DISALLOW_COPY_AND_ASSIGN(BlockShape);
};

} /* namespace sip */

#endif /* BLOCK_SHAPE_H_ */
