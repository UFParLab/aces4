/*
 * contiguous_local_array_manager.cpp
 *
 *  Created on: Aug 20, 2014
 *      Author: basbas
 */

#include "contiguous_local_array_manager.h"
#include "contiguous_array_manager.h"
#include "sip_tables.h"

namespace sip {


ContiguousLocalArrayManager::ContiguousLocalArrayManager(SipTables& sip_tables):
		sip_tables_(sip_tables), block_map_(sip_tables.num_arrays()){
}
ContiguousLocalArrayManager::~ContiguousLocalArrayManager(){
	for (int i = 0; i < sip_tables_.num_arrays(); ++i)
		block_map_.delete_per_array_map_and_blocks(i);
}

void ContiguousLocalArrayManager::allocate_contiguous_local(const ContiguousLocalBlockId& id){
	check(block_map_.disjoint(id), "attempting to allocate contiguous local that overlaps with existing",
			current_line());
	create_block(id);
}

void ContiguousLocalArrayManager::deallocate_contiguous_local(const ContiguousLocalBlockId& id){
	block_map_.delete_block(id);
}

Block::BlockPtr ContiguousLocalArrayManager::get_block_for_writing(const ContiguousLocalBlockId& id,
		WriteBackList& write_back_list){
	Block::BlockPtr containing_region;
	int rank;
	offset_array_t offsets;
	Block::BlockPtr region = get_block(id, rank, containing_region, offsets);
	if (region != containing_region) write_back_list.push_back(new WriteBack(rank, containing_region, region, offsets));
    if (region == NULL) region = create_block(id);
	return region;
}

Block::BlockPtr ContiguousLocalArrayManager::get_block_for_reading(const ContiguousLocalBlockId& id,
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


Block::BlockPtr ContiguousLocalArrayManager::get_block_for_updating(const ContiguousLocalBlockId& id,
		WriteBackList& write_back_list){
	Block::BlockPtr containing_region;
	int rank;
	offset_array_t offsets;
	Block::BlockPtr region = get_block(id, rank, containing_region, offsets);
	sial_check(region != NULL, "Attempting to update non-existent region of contiguous local array" + id.str(sip_tables_), current_line());
	if (region != containing_region) write_back_list.push_back(new WriteBack(rank, containing_region, region, offsets));
	return region;
}

Block::BlockPtr ContiguousLocalArrayManager::get_block_for_accumulate(const ContiguousLocalBlockId& id,
		WriteBackList& write_back_list){
	Block::BlockPtr containing_region;
	int rank;
	offset_array_t offsets;
	Block::BlockPtr region = get_block(id, rank, containing_region, offsets);
	if (region != NULL && region != containing_region) write_back_list.push_back(new WriteBack(rank, containing_region, region, offsets));
    if (region == NULL){
    	region = create_block(id);
    	region->fill(0.0);
    }
	return region;
}

Block::BlockPtr ContiguousLocalArrayManager::create_block(const ContiguousLocalBlockId& id){
	int array_slot = id.array_id();
	int array_rank = sip_tables_.array_rank(array_slot);
	BlockShape shape = sip_tables_.contiguous_region_shape(array_slot, id.lower_index_values_, id.upper_index_values_);
			try {
				Block::BlockPtr region = new Block(shape);
				block_map_.insert_block(id, region);  //this will fail if there is overlap.
				return region;
			} catch (const std::out_of_range& oor){
				std::cerr << "At line " << __LINE__ << " in file " << __FILE__ << std::endl;
				std::cerr << *this << std::endl;
				fail(" Could not create block, out of memory", current_line());
				return NULL;
			}
}

Block::BlockPtr ContiguousLocalArrayManager::get_block(const ContiguousLocalBlockId& id, int& rank, Block::BlockPtr& contiguous, sip::offset_array_t& offsets){
	ContiguousLocalBlockId enclosing_id;
    Block* region = block_map_.enclosing_block(id,  enclosing_id);
    if (region == NULL) return region;
    if (enclosing_id == id){
    	contiguous = region;
    	rank = sip_tables_.array_rank(id.array_id());
    	std::fill(offsets+0, offsets+MAX_RANK, 0);
    	return region;
    }
    Block* enclosing_region = block_map_.enclosing_block(enclosing_id, enclosing_id);
    const index_selector_t& selector = sip_tables_.selectors(id.array_id_);
    BlockShape enclosing_shape = sip_tables_.contiguous_region_shape(enclosing_id.array_id(),
    		enclosing_id.lower_index_values_, enclosing_id.upper_index_values_);
    //get offsets of block_id into enclosing region
    		for (int i = 0; i < rank; ++i) {
    			offsets[i] = sip_tables_.offset_into_contiguous_region(selector[i],
    					enclosing_id.lower_index_values(i), id.lower_index_values(i));
    		}
    	//set offsets of unused indices to 0
    		std::fill(offsets + rank, offsets + MAX_RANK, 0);

    //get shape of requested block to be extracted
    BlockShape id_shape = sip_tables_.contiguous_region_shape(id.array_id(),
		id.lower_index_values_, id.upper_index_values_);

    //allocate a new block and copy data from contiguous block
    Block* subregion = new Block(id_shape);
    enclosing_region->extract_slice(rank, offsets, subregion);
    return subregion;
}


std::ostream& operator<<(std::ostream& os, const ContiguousLocalArrayManager& obj) {
	os << "block_map_:" << std::endl;
	os << obj.block_map_ << std::endl;

	return os;
}

//Block::BlockPtr ContiguousArrayManager::get_block(const BlockId& block_id, int& rank,
//		Block::BlockPtr& contiguous, sip::offset_array_t& offsets) {
////get contiguous array that contains block block_id, which must exist, and get its selectors and shape
//	int array_id = block_id.array_id();
//	rank = sip_tables_.array_rank(array_id);
//	contiguous = get_array(array_id);
//	sip::check(contiguous != NULL, "contiguous array not allocated");
//	const sip::index_selector_t& selector = sip_tables_.selectors(array_id);
//	BlockShape array_shape = sip_tables_.contiguous_array_shape(array_id); //shape of containing contiguous array
//
////get offsets of block_id in the containing array
//	for (int i = 0; i < rank; ++i) {
//		offsets[i] = sip_tables_.offset_into_contiguous(selector[i],
//				block_id.index_values(i));
//	}
////set offsets of unused indices to 0
//	std::fill(offsets + rank, offsets + MAX_RANK, 0);
//
////get shape of subblock
//	BlockShape block_shape = sip_tables_.shape(block_id);
//
////allocate a new block and copy data from contiguous block
//	Block::BlockPtr block = new Block(block_shape);
//	contiguous->extract_slice(rank, offsets, block);
//	return block;
//}
} /* namespace sip */
