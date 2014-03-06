/*
 * contiguous_array_manager.cpp
 *
 *  Created on: Oct 5, 2013
 *      Author: basbas
 */

#include "config.h"
#include "contiguous_array_manager.h"
#include "sip_tables.h"
#include "setup_reader.h"
#include "global_state.h"

namespace sip {

WriteBack::WriteBack(int rank, Block::BlockPtr contiguous_block,
		Block::BlockPtr block, const offset_array_t& offsets) :
		rank_(rank), contiguous_block_(contiguous_block), block_(block), done_(false) {
	std::copy(offsets+0, offsets+MAX_RANK, offsets_+0);
}


WriteBack::~WriteBack() {
}
void WriteBack::do_write_back() {
	sip::check(!done_,"SIP bug:  called doWriteBack twice");
	contiguous_block_->insert_slice(rank_, offsets_, block_);
	done_=true;
}

std::ostream& operator<<(std::ostream& os, const WriteBack& obj) {
	os << "WriteBack: rank= " << obj.rank_ << " offset_array_t=[";
    for (int i = 0; i < MAX_RANK; ++i){ os << (i == 0 ? "" : ",")  << obj.offsets_[i]; }
	os << "]" << std::endl;
	os << "target contiguous_block_ :" << std::endl;
	os << * obj.contiguous_block_ << std::endl;
	os << "source block " << std::endl;
	os << * obj.block_;
	bool done_;
	os << "contiguous_array_map_:" << std::endl;
	return os;
}

ContiguousArrayManager::ContiguousArrayManager(sip::SipTables& sipTables, setup::SetupReader& setup_reader, PersistentArrayManager& pbm_read, PersistentArrayManager &pbm_write) :
		pbm_read_(pbm_read), pbm_write_(pbm_write),
		sipTables_(sipTables),
		setup_reader_(setup_reader){
	//create static arrays in sial program
	//TODO implement predefined array lookup
	int num_arrays = sipTables_.num_arrays();
	for (int i = 0; i < num_arrays; ++i){
		if (sipTables_.is_contiguous(i) && sipTables_.array_rank(i)>0){
//			std::cout << "creating contiguous array " << sipTables_.array_name(i) << std::endl;
//			sip::check_and_warn(sipTables_.is_predefined(i),
//					std::string("predefined static arrays not implemented, allocating static array ") +
//					sipTables_.array_name(i) +  " and setting to 0");

			//check whether array has been predefined
			// FIXME TODO HACK - Check current program number.
			// The assumption is that only the first program reads predefined arrays from the
			// initialization data. The following ones dont.

			if (sip::GlobalState::is_first_program()){

				if (sipTables_.is_predefined(i)) {

					SIP_LOG(std::cout << "Array " << sipTables_.array_name(i)<< " is predefined..." << std::endl);

					Block::BlockPtr block = NULL;
					std::string name = sipTables_.array_name(i);
					setup::SetupReader::NamePredefinedContiguousArrayMapIterator b =
							setup_reader_.name_to_predefined_contiguous_array_map_.find(name);
					if (b != setup_reader_.name_to_predefined_contiguous_array_map_.end()) {
						block = b->second;
					}
					if (block != NULL)
						// Copy the data in the block when reading from setup reader instead of
						// copying the pointer.
						create_contiguous_array(i, block->clone());
					else
						create_contiguous_array(i);
				} else {
					create_contiguous_array(i);
				}
			} else {
				create_contiguous_array(i);

			}
		}
	}
}

ContiguousArrayManager::~ContiguousArrayManager() {
	ContiguousArrayMap::iterator it;
	for (it=contiguous_array_map_.begin(); it!= contiguous_array_map_.end(); ++it){
		int i = it->first;
		if (!pbm_write_.is_array_persistent(i) && it->second != NULL){
			SIP_LOG(std::cout<<"Deleting contiguous array : "<<sipTables_.array_name(i)<<std::endl);
			delete it->second;
			it->second = NULL;
		}
	}
	contiguous_array_map_.clear();
}

Block::BlockPtr ContiguousArrayManager::create_contiguous_array(int array_id, Block::BlockPtr block_ptr){
	sip::check(block_ptr != NULL, "Trying to insert null data into contiguous array manager\n");
	//if (block == NULL) {return create_contiguous_array(array_id);}
	//Block::BlockPtr blk = create_contiguous_array(array_id, block->get_data());
	sip::check(block_ptr->get_data() != NULL, "Trying to put NULL data into contiguous array !");
	ContiguousArrayMap::iterator it = contiguous_array_map_.find(array_id);
	if (it != contiguous_array_map_.end()){
		if (block_ptr->get_data() != it->second->get_data()){
			SIP_LOG(sip::check_and_warn(false, std::string("attempting to create contiguous array that already exists"), current_line()));
			sip::check(it->second != NULL, "Trying to delete NULL block !", current_line());
			delete it->second;
			contiguous_array_map_.erase(array_id);
		}
	}

	const std::pair<ContiguousArrayMap::iterator, bool> &ret = contiguous_array_map_.insert(std::pair<int, Block::BlockPtr>(array_id, block_ptr));


	//	std::cout << "getting contiguous array " << sipTables_.array_name(array_id) << std::endl;
//	std::cout << blk->shape_ << std::endl;
//	std::cout << "********" <<std::endl;
//	std::cout << block->shape_ << std::endl;

	SIP_LOG(std::cout<<"Contiguous Block of array "<<sipTables_.array_name(array_id)<<std::endl);
	//std::cout<<*(ret.first->second)<<std::endl;

	sip::check(block_ptr->shape() == sipTables_.contiguous_array_shape(array_id),
			std::string("array ") + sipTables_.array_name(array_id)
					+ std::string("shape inconsistent in Sial program and dat file "));
	return block_ptr;
}

Block::BlockPtr ContiguousArrayManager::create_contiguous_array(int array_id) {
	BlockShape shape = sipTables_.contiguous_array_shape(array_id);
	SIP_LOG(std::cout<< "creating contiguous array " << sipTables_.array_name(array_id)
			<< " with shape " << shape << " and array id :" << array_id << std::endl);
	Block::BlockPtr block_ptr = new Block(shape);
	const std::pair<ContiguousArrayMap::iterator, bool> &ret =
			contiguous_array_map_.insert(std::pair<int, Block::BlockPtr>(array_id, block_ptr));
	sip::check(ret.second, std::string("attempting to create contiguous array that already exists"));
	return block_ptr;
}


//Block::BlockPtr ContiguousArrayManager::create_contiguous_array(int array_id, Block::dataPtr data) {
//	BlockShape shape = sipTables_.contiguous_array_shape(array_id);
//	Block::BlockPtr block_ptr = new Block(shape, data);
////	std::pair<ContiguousArrayMap::iterator, bool> ret;
//    ContiguousArrayMap::iterator it = contiguous_array_map_.find(array_id);
//	sip::check_and_warn(it != contiguous_array_map_.end(),	std::string("attempting to create contiguous array that already exists"));
//    if (it != contiguous_array_map_.end()){
//    	//if (block_ptr->get_data() != it->second->get_data())
//    		delete it->second;
//    }
//    sip::check(block_ptr->get_data() != NULL, "Trying to put NULL data into contiguous array !");
//    contiguous_array_map_.insert(std::pair<int, Block::BlockPtr>(array_id, block_ptr));
//	return block_ptr;
//}

//TODO not tested
//void ContiguousArrayManager::remove_contiguous_array(int array_id) {
//	ContiguousArrayMap::iterator block_to_remove = contiguous_array_map_.find(array_id);
//	contiguous_array_map_.erase(block_to_remove);
//        delete block_to_remove->second;
//}

Block::BlockPtr ContiguousArrayManager::get_block_for_updating(const BlockId& block_id, WriteBackList& write_back_list) {
	 int rank = 0;
	 Block::BlockPtr contiguous = NULL;
	 Block::BlockPtr block = NULL;
	 sip::offset_array_t offsets;
	 get_block(block_id, rank, contiguous, block, offsets);
	 write_back_list.push_back(new WriteBack(rank, contiguous, block, offsets));
	 return block;
}


Block::BlockPtr ContiguousArrayManager::get_block_for_reading(const BlockId& block_id) {
	 int rank = 0;
	 Block::BlockPtr contiguous = NULL;
	 Block::BlockPtr block = NULL;
	 sip::offset_array_t offsets;
	 get_block(block_id, rank, contiguous, block, offsets);
	 return block;
}

std::ostream& operator<<(std::ostream& os, const ContiguousArrayManager& obj) {
	os << "contiguous_array_map_:" << std::endl;
	{
		ContiguousArrayManager::ContiguousArrayMap::const_iterator it;
		for (it = obj.contiguous_array_map_.begin(); it != obj.contiguous_array_map_.end(); ++it) {
			os << it->first << std::endl;  //print the array id
		}
	}
	return os;
}

Block::BlockPtr ContiguousArrayManager::get_array(int id) {
	ContiguousArrayMap::iterator b = contiguous_array_map_.find(id);
	if (b != contiguous_array_map_.end()) {
		return b->second;
	}
	return NULL;
}


void ContiguousArrayManager::get_block(const BlockId& block_id, int& rank, Block::BlockPtr& contiguous, Block::BlockPtr& block, sip::offset_array_t& offsets){
	//get contiguous array that contains block block_id, which must exist, and get its selectors and shape
	int array_id = block_id.array_id();
	rank = sipTables_.array_rank(array_id);
	contiguous = get_array(array_id);
	sip::check(contiguous != NULL, "contiguous array not allocated");
	const sip::index_selector_t& selector = sipTables_.selectors(array_id);
	BlockShape array_shape = sipTables_.contiguous_array_shape(array_id); //shape of containing contiguous array

	//get offsets of block_id in the containing array
	for (int i = 0; i < rank; ++i) {
		offsets[i] = sipTables_.offset_into_contiguous(selector[i],
				block_id.index_values(i));
	}
	//set offsets of unused indices to 0
	std::fill(offsets + rank, offsets + MAX_RANK, 0);

	//get shape of subblock
	BlockShape block_shape = sipTables_.shape(block_id);

//	//DEBUG print info about block
//    std::cout << "ContiguousArrayManager::get_block " << block_id << "  offsets: [";
//    		for (int i = 0; i < MAX_RANK; ++i){
//    			std::cout << (i ==0 ? "" : ",") << offsets[i];
//    		}
//    std::cout << "] shape: [";
//	for (int i = 0; i < MAX_RANK; ++i){
//		std::cout << (i ==0 ? "" : ",") << block_shape.segment_sizes_[i];
//	}
//    std::cout << ']' << std::endl;

	//allocate a new block and copy data from contiguous block
	block = new Block(block_shape);
	contiguous->extract_slice(rank, offsets, block);
    return;
}


void ContiguousArrayManager::save_persistent_contig_arrays(){
	ContiguousArrayMap::iterator it;
	for (it=contiguous_array_map_.begin(); it!= contiguous_array_map_.end(); ++it){
		int i = it->first;
		if (pbm_write_.is_array_persistent(i)){
			pbm_write_.save_contiguous_array(i, it->second);
			it->second = NULL;
		}

	}
}



} /* namespace sip */
