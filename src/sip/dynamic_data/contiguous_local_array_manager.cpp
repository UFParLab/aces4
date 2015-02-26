/*
 * contiguous_local_array_manager.cpp
 *
 *  Created on: Aug 20, 2014
 *      Author: basbas
 */

#include "contiguous_local_array_manager.h"
#include "contiguous_array_manager.h"
#include "sip_tables.h"
#include "block_manager.h"
#include "id_block_map.h"

namespace sip {


ContiguousLocalArrayManager::ContiguousLocalArrayManager(const SipTables& sip_tables, BlockManager& block_manager):
		sip_tables_(sip_tables), block_map_(block_manager.block_map_){
}
ContiguousLocalArrayManager::~ContiguousLocalArrayManager(){
  /* the block_map_ is deleted by the BlockManager */
}

void ContiguousLocalArrayManager::allocate_contiguous_local(const BlockId& id){
	create_block(id); //create_block initializes this to 0.  Check this
}

void ContiguousLocalArrayManager::deallocate_contiguous_local(const BlockId& id){
	block_map_.delete_block(id);
}

Block::BlockPtr ContiguousLocalArrayManager::get_block_for_writing(const BlockId& id,
		WriteBackList& write_back_list){
	Block::BlockPtr containing_region;
	int rank;
	offset_array_t offsets;
	Block::BlockPtr region = get_block(id, rank, containing_region, offsets);
	sial_check(region != NULL, "Contiguous local block " + id.str(sip_tables_) + " must be explicitly allocated", current_line());
	if (region != containing_region){
		WriteBack* wb = new WriteBack(rank, containing_region, region, offsets);
		write_back_list.push_back(wb);
	}
	return region;
}

Block::BlockPtr ContiguousLocalArrayManager::get_block_for_reading(const BlockId& id,
		ReadBlockList& read_block_list){
	Block::BlockPtr containing_region;
	int rank;
	offset_array_t offsets;
	Block::BlockPtr region = get_block(id, rank, containing_region, offsets);
	sial_check(region != NULL, "Attempting to read non-existent region of contiguous local array" + id.str(sip_tables_), current_line());
	if (region != containing_region){
		read_block_list.push_back(region);
	}
	return region;
}


Block::BlockPtr ContiguousLocalArrayManager::get_block_for_updating(const BlockId& id,
		WriteBackList& write_back_list){
	Block::BlockPtr containing_region;
	int rank;
	offset_array_t offsets;
	Block::BlockPtr region = get_block(id, rank, containing_region, offsets);
	sial_check(region != NULL, "Attempting to update non-existent region of contiguous local array" + id.str(sip_tables_), current_line());
	if (region != containing_region) write_back_list.push_back(new WriteBack(rank, containing_region, region, offsets));
	return region;
}

Block::BlockPtr ContiguousLocalArrayManager::get_block_for_accumulate(const BlockId& id,
		WriteBackList& write_back_list){
	Block::BlockPtr containing_region;
	int rank;
	offset_array_t offsets;
	Block::BlockPtr region = get_block(id, rank, containing_region, offsets);
	if (region != NULL && region != containing_region) write_back_list.push_back(new WriteBack(rank, containing_region, region, offsets));
    if (region == NULL){
    	region = create_block(id);
//    	region->fill(0.0);  currently done when data for block is allocated with new in create_block(id)
    }
	return region;
}

Block::BlockPtr ContiguousLocalArrayManager::create_block(const BlockId& id){
	int array_slot = id.array_id();
	int array_rank = sip_tables_.array_rank(array_slot);
	const BlockShape shape(sip_tables_.contiguous_region_shape(array_rank, array_slot, id.index_values_, id.parent_id_ptr_->index_values_));
	try {
		Block::BlockPtr region = new Block(shape);
		region->fill(0.0);
		block_map_.insert_block(id, region);  //this will fail if there is overlap.
		return region;
	} catch (const std::out_of_range& oor){
		std::cerr << "At line " << __LINE__ << " in file " << __FILE__ << std::endl;
		std::cerr << *this << std::endl;
		fail(" create_block in ContiguousLocalArrayManager failed", current_line());
		return NULL;
	}
}



Block::BlockPtr ContiguousLocalArrayManager::get_block(const BlockId& id, int& rank, Block::BlockPtr& enclosing_block, sip::offset_array_t& offsets){
	rank = sip_tables_.array_rank(id.array_id());

	//Look for an exact match first
	Block::BlockPtr block = block_map_.block(id);
	if (block != NULL){
    	enclosing_block = block;
    	std::fill(offsets+0, offsets+MAX_RANK, 0);
    	return block;
    }

	//Not found, look for enclosing block
	BlockId enclosing_id;
	enclosing_block = block_map_.enclosing_contiguous(id, enclosing_id);
	if (enclosing_block){ //found an enclosing contiguous block.  Extract and return the desired block.

		//look up selector for declared indices
			const index_selector_t& selector= sip_tables_.selectors(id.array_id());
         //get offsets of requested block into enclosing region
    		for (int i = 0; i < rank; ++i) {
    			 offsets[i] = sip_tables_.offset_into_contiguous_region(selector[i],
    					enclosing_id.index_values(i), id.index_values(i));
    		}
    		std::fill(offsets + rank, offsets + MAX_RANK, 0);

    //get shape of  block to be extracted
    BlockShape id_shape = sip_tables_.contiguous_region_shape(rank, id.array_id(),
		id.index_values_, id.parent_id_ptr_->index_values_);

    //allocate a new block and copy data from the contiguous block
    block = new Block(id_shape);
    block->fill(0.0);
    enclosing_block->extract_slice(rank, offsets, block);
	}

	//if an enclosing block was not found, block will be NULL.  Otherwise it will be the requested part of the enclosing block
    return block;
}


std::ostream& operator<<(std::ostream& os, const ContiguousLocalArrayManager& obj) {
	os << "block_map_:" << std::endl;
	os << obj.block_map_ << std::endl;

	return os;
}


} /* namespace sip */
