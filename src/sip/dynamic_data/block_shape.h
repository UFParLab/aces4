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

} /* namespace sip */

#endif /* BLOCK_SHAPE_H_ */
