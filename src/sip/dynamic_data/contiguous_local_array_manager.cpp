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
	create_block(id); //so initialized to 0.  Perhaps we don't need this.
}

void ContiguousLocalArrayManager::deallocate_contiguous_local(const BlockId& id){
	block_map_.delete_block(id);
}

Block::BlockPtr ContiguousLocalArrayManager::get_block_for_writing(const BlockId& id,
		WriteBackList& write_back_list){
	std::cout << "get_block_for_writing id " << id << std::endl;
	Block::BlockPtr containing_region;
	int rank;
	offset_array_t offsets;
	Block::BlockPtr region = get_block(id, rank, containing_region, offsets);
    if (region == NULL){
    	region = create_block(id);
	return region;
    }
	std::cout << "in get block for writing:  printing region " << *region << std::endl << std::flush;  //DEBUG
	std::cout << "in get block for writing:  printing containing_region " << *containing_region << std::endl << std::flush;  //DEBUG
	if (region != containing_region){
		WriteBack* wb = new WriteBack(rank, containing_region, region, offsets);
		std::cout << "in get_block_for_writing "<<   //*wb <<
				std::endl << std::flush;
		write_back_list.push_back(wb);
	}
	return region;
}

Block::BlockPtr ContiguousLocalArrayManager::get_block_for_reading(const BlockId& id,
		ReadBlockList& read_block_list){
	std::cout << "get_block_for_reading id " << id << std::endl;
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
    	region->fill(0.0);
    }
	return region;
}

Block::BlockPtr ContiguousLocalArrayManager::create_block(const BlockId& id){
	int array_slot = id.array_id();
	int array_rank = sip_tables_.array_rank(array_slot);
	const BlockShape shape(sip_tables_.contiguous_region_shape(array_slot, id.index_values_, id.parent_id_ptr_->index_values_));
			try {
				Block::BlockPtr region = new Block(shape);
				block_map_.insert_block(id, region);  //this will fail if there is overlap.
				return region;
			} catch (const std::out_of_range& oor){
				std::cerr << "At line " << __LINE__ << " in file " << __FILE__ << std::endl;
				std::cerr << *this << std::endl;
				fail(" create_block in ContiguousLocalArrayManager failed", current_line());
				return NULL;
			}
}


/** In the contiguous array manager, since the enclosing contiguous arrays are static in the sial program, and include the entire
 * array, they will always exist.  Here we return the block, or NULL if there is no block and o enclosing block.
 * @param id
 * @param rank
 * @param contiguous
 * @param offsets
 * @return
 */
Block::BlockPtr ContiguousLocalArrayManager::get_block(const BlockId& id, int& rank, Block::BlockPtr& contiguous, sip::offset_array_t& offsets){
	Block::BlockPtr block = block_map_.block(id);
	rank = sip_tables_.array_rank(id.array_id());
//DEBUG	std::cout << "in car::get_block, rank, id = " << rank << ", " << id << std::endl << std::flush;
	if (block != NULL){
    	contiguous = block;
    	std::fill(offsets+0, offsets+MAX_RANK, 0);
    	return block;
    }

	BlockId glb_id;
	Block::BlockPtr glb_block = NULL;
//DEBUG	std::cout << "before calling  GLB block" << std::endl << std::flush;
	glb_block = block_map_.GLB(id, glb_id);
	if (glb_block){
//DEBUG		std::cout << "found GLB block" << std::endl << std::flush;
		//a GLB block was found.  Does it enclose the desired block?
		if (glb_id.encloses(id)){
			contiguous = glb_block;
//DEBUG			std::cout << "found enclosing block " << glb_id << std::endl << std::flush;
			//look up declared indices for array
			const index_selector_t& selector= sip_tables_.selectors(id.array_id());

		BlockShape enclosing_shape = sip_tables_.contiguous_region_shape(glb_id.array_id(),
		   glb_id.index_values_, glb_id.parent_id_ptr_->index_values_);
		std::cout << "enclosing_shape from sip_tables"<< enclosing_shape << std::endl;
		std::cout << "enclosing_shape_from_block" << glb_block->shape() << std::endl;
         //get offsets of block_id into enclosing region
    		for (int i = 0; i < rank; ++i) {
    			 offsets[i] = sip_tables_.offset_into_contiguous_region(selector[i],
    					glb_id.index_values(i), id.index_values(i));
    		}
    		std::fill(offsets + rank, offsets + MAX_RANK, 0);
    //get shape of  block to be extracted
    		std::cout << "id.index_values_, id.parent_id_ptr_->index_values_ "<< std::endl;
    		for (int i = 0; i < MAX_RANK; ++i){
    			std::cout << id.index_values_[i] << ",";
    		}
    		std::cout << "][";
      		for (int i = 0; i < MAX_RANK; ++i){
        			std::cout << id.parent_id_ptr_->index_values_[i] << ",";
        		}
      		std::cout << std::endl;

    BlockShape id_shape = sip_tables_.contiguous_region_shape(id.array_id(),
		id.index_values_, id.parent_id_ptr_->index_values_);
	std::cout << "id_shape "<< id_shape << std::endl;
    //allocate a new block and copy data from contiguous block
    block = new Block(id_shape);
    std::cout << "block " << *block << std::endl;
    std::cout << "before extract_slice rank, offsets, from block " << rank <<  ",";
    for (int i = 0; i < MAX_RANK; ++i){
    		std::cout << offsets[i] << ",";
    }
 //   	std::cout 			<< "," << *glb_block << std::endl;
    glb_block->extract_slice(rank, offsets, block);

	}
	}
    return block;
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
