/*
 * contiguous_array_manager.cpp
 *
 *  Created on: Oct 5, 2013
 *      Author: basbas
 */

#include <utility>

#include "config.h"
#include "contiguous_array_manager.h"
#include "sip_tables.h"
#include "setup_reader.h"
#include "global_state.h"

#ifdef HAVE_MPI
#include "sip_mpi_attr.h"
#endif

namespace sip {

WriteBack::WriteBack(int rank, Block::BlockPtr contiguous_block,
		Block::BlockPtr block, const offset_array_t& offsets) :
		rank_(rank), contiguous_block_(contiguous_block), block_(block), done_(
				false) {
	std::copy(offsets + 0, offsets + MAX_RANK, offsets_ + 0);
}

WriteBack::~WriteBack() {
	delete block_;
}
void WriteBack::do_write_back() {
	sip::check(!done_, "SIP bug:  called doWriteBack twice");
	contiguous_block_->insert_slice(rank_, offsets_, block_);
	done_ = true;
}

std::ostream& operator<<(std::ostream& os, const WriteBack& obj) {
	os << "WriteBack: rank= " << obj.rank_ << " offset_array_t=[";
	for (int i = 0; i < MAX_RANK; ++i) {
		os << (i == 0 ? "" : ",") << obj.offsets_[i];
	}
	os << "]" << std::endl;
	os << "target contiguous_block_ :" << std::endl;
	os << *obj.contiguous_block_ << std::endl;
	os << "source block " << std::endl;
	os << *obj.block_;
	bool done_;
	os << "contiguous_array_map_:" << std::endl;
	return os;
}



ContiguousArrayManager::ContiguousArrayManager(sip::SipTables& sip_tables,
		setup::SetupReader& setup_reader) :
		sip_tables_(sip_tables), setup_reader_(setup_reader) {
	//create static arrays in sial program.  All static arrays are allocated a startup
	int num_arrays = sip_tables_.num_arrays();
	for (int i = 0; i < num_arrays; ++i) {
		if (sip_tables_.is_contiguous(i) && sip_tables_.array_rank(i) > 0) {
			//check whether array has been predefined
			// FIXME TODO HACK - Check current program number.
			// The assumption is that only the first program reads predefined arrays from the
			// initialization data. The following ones dont.

			if (sip_tables_.is_predefined(i)) {

				SIP_LOG(
						std::cout << "Array " << sip_tables_.array_name(i)<< " is predefined..." << std::endl);

				Block::BlockPtr block = NULL;
				std::string name = sip_tables_.array_name(i);
				setup::SetupReader::NamePredefinedContiguousArrayMapIterator b =
						setup_reader_.name_to_predefined_contiguous_array_map_.find(
								name);
                if (b != setup_reader_.name_to_predefined_contiguous_array_map_.end()){        
					//array is predefined and in setup reader
					block = b->second.second;
					insert_contiguous_array(i, block);
				} else { //is predefined, but not in setreader.  Set to zero, insert into setup_reader map so it "owns" it and deletes it.

				    SIP_MASTER(check_and_warn(false, "No data for predefined static array " + name));
					block = create_contiguous_array(i);
					int block_rank = sip_tables_.array_rank(i);
					std::pair<int, Block::BlockPtr> zeroed_block_pair = std::make_pair(i, block);
					setup_reader_.name_to_predefined_contiguous_array_map_.insert(make_pair(name, zeroed_block_pair));
				}
			} else { //not predefined, just create it
				create_contiguous_array(i);

			}
		} //end if contiguous
	} // end loop
}


/** delete all contiguous arrays except the predefined ones. */
ContiguousArrayManager::~ContiguousArrayManager() {
	ContiguousArrayMap::iterator it;
	for (it = contiguous_array_map_.begin(); it != contiguous_array_map_.end();
			++it) {
		int i = it->first;
		if (it->second != NULL && !sip_tables_.is_predefined(i)) {
			SIP_LOG(
					std::cout<<"Deleting contiguous array : "<<sip_tables_.array_name(i)<<std::endl);
			delete it->second;
			it->second = NULL;
		}
	}
	contiguous_array_map_.clear();
}

Block::BlockPtr ContiguousArrayManager::insert_contiguous_array(int array_id,
		Block::BlockPtr block_ptr) {
	sip::check(block_ptr != NULL && block_ptr->get_data() != NULL,
			"Trying to insert null block_ptr or null block into contiguous array manager\n");
	ContiguousArrayMap::iterator it = contiguous_array_map_.find(array_id);
	if (it != contiguous_array_map_.end()){
		delete it->second;
		it->second = NULL;
		contiguous_array_map_.erase(it);
	}
	contiguous_array_map_[array_id] = block_ptr;

	SIP_LOG(
			std::cout<<"Contiguous Block of array "<<sip_tables_.array_name(array_id)<<std::endl);
	sip::check(
			block_ptr->shape() == sip_tables_.contiguous_array_shape(array_id),
			std::string("array ") + sip_tables_.array_name(array_id)
					+ std::string(
							"shape inconsistent in Sial program and inserted array "));
	return block_ptr;
}

Block::BlockPtr ContiguousArrayManager::create_contiguous_array(int array_id) {
	BlockShape shape = sip_tables_.contiguous_array_shape(array_id);
	SIP_LOG(
			std::cout<< "creating contiguous array " << sip_tables_.array_name(array_id) << " with shape " << shape << " and array id :" << array_id << std::endl);
	Block::BlockPtr block_ptr = new Block(shape);
	const std::pair<ContiguousArrayMap::iterator, bool> &ret =
			contiguous_array_map_.insert(
					std::pair<int, Block::BlockPtr>(array_id, block_ptr));
	sip::check(ret.second,
			std::string(
					"attempting to create contiguous array that already exists"));
	return block_ptr;
}

Block::BlockPtr ContiguousArrayManager::get_block_for_updating(
		const BlockId& block_id, WriteBackList& write_back_list) {
	int rank = 0;
	Block::BlockPtr contiguous = NULL;
	sip::offset_array_t offsets;
	Block::BlockPtr block = get_block(block_id, rank, contiguous, offsets);
	write_back_list.push_back(new WriteBack(rank, contiguous, block, offsets));
	return block;
}

Block::BlockPtr ContiguousArrayManager::get_block_for_reading(
		const BlockId& block_id, ReadBlockList& read_block_list) {
	int rank = 0;
	Block::BlockPtr contiguous = NULL;
	sip::offset_array_t offsets;
	Block::BlockPtr block = get_block(block_id, rank, contiguous, offsets);
	read_block_list.push_back(block);
	return block;
}

std::ostream& operator<<(std::ostream& os, const ContiguousArrayManager& obj) {
	os << "contiguous_array_map_:" << std::endl;
	{
		ContiguousArrayManager::ContiguousArrayMap::const_iterator it;
		for (it = obj.contiguous_array_map_.begin();
				it != obj.contiguous_array_map_.end(); ++it) {
			os << it->first << std::endl;  //print the array id
		}
	}
	return os;
}

Block::BlockPtr ContiguousArrayManager::get_array(int array_id) {
	ContiguousArrayMap::iterator b = contiguous_array_map_.find(array_id);
	if (b != contiguous_array_map_.end()) {
		return b->second;
	}
	return NULL;
}

Block::BlockPtr ContiguousArrayManager::get_and_remove_array(int array_id) {
	ContiguousArrayMap::iterator b = contiguous_array_map_.find(array_id);
	if (b != contiguous_array_map_.end()) {
		Block::BlockPtr tmp = b->second;
		contiguous_array_map_.erase(b);
		return tmp;
	}
	return NULL;
}

Block::BlockPtr ContiguousArrayManager::get_block(const BlockId& block_id, int& rank,
		Block::BlockPtr& contiguous, sip::offset_array_t& offsets) {
//get contiguous array that contains block block_id, which must exist, and get its selectors and shape
	int array_id = block_id.array_id();
	rank = sip_tables_.array_rank(array_id);
	contiguous = get_array(array_id);
	sip::check(contiguous != NULL, "contiguous array not allocated");
	const sip::index_selector_t& selector = sip_tables_.selectors(array_id);
	BlockShape array_shape = sip_tables_.contiguous_array_shape(array_id); //shape of containing contiguous array

//get offsets of block_id in the containing array
	for (int i = 0; i < rank; ++i) {
		offsets[i] = sip_tables_.offset_into_contiguous(selector[i],
				block_id.index_values(i));
	}
//set offsets of unused indices to 0
	std::fill(offsets + rank, offsets + MAX_RANK, 0);

//get shape of subblock
	BlockShape block_shape = sip_tables_.shape(block_id);

//allocate a new block and copy data from contiguous block
	Block::BlockPtr block = new Block(block_shape);
	contiguous->extract_slice(rank, offsets, block);
	return block;
}

} /* namespace sip */
