/*
 * contiguous_local_array_manager.h
 *
 *  Created on: Aug 20, 2014
 *      Author: basbas
 */

#ifndef CONTIGUOUS_LOCAL_ARRAY_MANAGER_H_
#define CONTIGUOUS_LOCAL_ARRAY_MANAGER_H_

#include "contiguous_local_block_id.h"
#include "contiguous_local_id_block_map.h"
#include "contiguous_array_manager.h"
namespace sip {


class ContiguousLocalArrayManager {
public:
	ContiguousLocalArrayManager(SipTables& sip_tables);
	~ContiguousLocalArrayManager();

	void allocate_contiguous_local(const ContiguousLocalBlockId& id);
	void deallocate_contiguous_local(const ContiguousLocalBlockId& id);

	Block::BlockPtr get_block_for_writing(const ContiguousLocalBlockId& id, WriteBackList& write_back_list);
	Block::BlockPtr get_block_for_reading(const ContiguousLocalBlockId& id, ReadBlockList& read_block_list);
	Block::BlockPtr get_block_for_updating(const ContiguousLocalBlockId& id, WriteBackList& write_back_list);
	Block::BlockPtr get_block_for_accumulate(const ContiguousLocalBlockId& id, WriteBackList& write_back_list);

//	/** Returns a pointer to the first region for this array in the map or NULL if the array does not exist.
//	 *
//	 * @param array_id  the array_table slot of the desired array
//	 * @return  BlockPtr to a  Block containing the first region (according to the ordering, usually there will be only on) in the map.
//	 */
//	Block::BlockPtr get_array(int array_id);



	friend std::ostream& operator<<(std::ostream&, const ContiguousLocalArrayManager&);

	friend class DataManager;
	friend class Interpreter;

private:
	SipTables& sip_tables_;
	ContiguousLocalIdBlockMap block_map_;

	/** Performs the actually work to get a contiguous copy of a subblock.
	 * Several values that are obtained in the
	 * course of the method are returned for possible use by callers.
	 *
	 * @param [in] Id of subblock to create
	 * @param [out] rank
	 * @param [out] contiguous BlockPtr to contiguous array that contains the indicated subblock.
	 * @param offsets [out] array containing offsets in each of first element of subblock in containing array.
	 * @return BlockPtr to contiguous copy of subblock
	 */
	Block::BlockPtr get_block(const ContiguousLocalBlockId&, int& rank, Block::BlockPtr& block, sip::offset_array_t& offsets);
	Block::BlockPtr  create_block(const ContiguousLocalBlockId& block_id);
	DISALLOW_COPY_AND_ASSIGN(ContiguousLocalArrayManager);

};

} /* namespace sip */

#endif /* CONTIGUOUS_LOCAL_ARRAY_MANAGER_H_ */
