/*
 * contiguous_local_id_block_map.h
 *
 *  Created on: Aug 20, 2014
 *      Author: basbas
 */

#ifndef CONTIGUOUS_LOCAL_ID_BLOCK_MAP_H_
#define CONTIGUOUS_LOCAL_ID_BLOCK_MAP_H_


#include <map>
#include <vector>
#include "contiguous_local_block_id.h"
#include "block.h"


namespace sip {

class ContiguousLocalIdBlockMap {
public:


	typedef std::map<ContiguousLocalBlockId, Block*> ContiguousLocalPerArrayMap;
	typedef std::vector<ContiguousLocalPerArrayMap*> ContiguousLocalBlockMapVector;
	typedef std::vector<std::map<ContiguousLocalBlockId, Block* >* >::size_type size_type;


	explicit ContiguousLocalIdBlockMap(int array_table_size);
	~ContiguousLocalIdBlockMap();

	/** Obtains the requested contiguous_local region or a contiguous local region that contains the requested one.
	 *
	 * @param [in] block_id  ContiguousLocalBlockId of the the requested contiguous_local region
	 * @param [out] contained_block_id Enclosing region of requested region.  (unchanged if requested region doesn't exist in map)
	 * @return pointer to containing region, or NULL if no region containing it is in the map.
	 *
	 *  block_id == contained_block_id iff exactly the requested region was found in the map.
	 */
	Block* enclosing_block(const ContiguousLocalBlockId& block_id, ContiguousLocalBlockId& enclosing_block_id);


	/**
	 * Indicates if the indicated block is disjoint from all other blocks currently in the map.
	 */
	bool disjoint(const ContiguousLocalBlockId& block_id);

//	/** Obtains requested block, creating if it doesn't exist
//	 * It is a fatal error to ask for a block that will overlap an existing one.
//	 *
//	 * @param [in] block_id
//	 * @param [out] size
//	 * @returns pointer to existing or new block
//	 */
//	Block* get_or_create_block(const ContiguousLocalBlockId& block_id, size_type size, bool initialize = true);

	/** inserts given block in the IdBlockMap.
	 *
	 * It is a fatal error to insert a block that already exists, or a block that overlaps an existing one.
	 *
	 * @param id BlockId of block to insert
	 * @param block_ptr pointer to Block to insert
	 */
	void insert_block(const ContiguousLocalBlockId& block_id, Block* block_ptr);

	/** Obtains requested block and remove it from the map.
	 * Requires the requested block is in the map
	 *
	 * @param id block id
	 * @return pointer to requested block
	 */
	Block* get_and_remove_block(const ContiguousLocalBlockId& block_id);
	/** removes given contiguous region from the map and deletes it.
	 * Require that the region exist and that the ranges match exactly.
	 * It is a fatal error to try to delete a region that doesn't exist
	 *
	 * @param id ContiguousLocalBlockId of block to delete
	 */
	void delete_block(const ContiguousLocalBlockId& id);
	/**
	 * per_array_map returns a pointer to the map containing blocks of the indicated array.
	 * If the map doesn't exist, one is created and added to the list of maps.
	 * This routine never returns NULL
	 *
	 * @param array_id
	 * @return pointer to PerArrayMap holding the blocks of the indicated array.
	 */
	ContiguousLocalPerArrayMap* per_array_map(int array_id);
	/**
	 *deletes the map containing blocks of the indicated array
	 *along with the blocks in the map
	 *
	 * @param array_id
	 * @return bytes (of block data_) deleted
	 */

	/**
	 * Deletes all entries in a ContiguousLocalPerArrayMap
	 * @param map_ptr
	 * @return bytes (of block data_) deleted
	 */
	void delete_blocks_from_per_array_map(ContiguousLocalPerArrayMap* map_ptr);

	size_type size() const;
	/**
	 *deletes the map containing blocks of the indicated array
	 *along with the blocks in the map
	 *
	 * @param array_id
	 * @return bytes (of block data_) deleted
	 */
	void delete_per_array_map_and_blocks(int array_id);

	ContiguousLocalPerArrayMap* operator[](unsigned i);
    const ContiguousLocalPerArrayMap* operator[](unsigned i) const;

    friend class Interpreter;
	friend std::ostream& operator<<(std::ostream&, const ContiguousLocalIdBlockMap&);

private:
	ContiguousLocalBlockMapVector block_map_;
	DISALLOW_COPY_AND_ASSIGN(ContiguousLocalIdBlockMap);

};

} /* namespace sip */

#endif /* CONTIGUOUS_LOCAL_ID_BLOCK_MAP_H_ */
