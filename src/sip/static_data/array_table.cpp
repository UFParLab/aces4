/*! ArrayDescriptor.cpp
 * 
 *
 *  Created on: Jun 5, 2013
 *      Author: Beverly Sanders
 */

#include "array_table.h"
#include "sip_tables.h"
#include "interpreter.h"

namespace sip {

//const int ArrayTableEntry::unused_index_slot = -1;//chosen to throw out of bound exception if accessed.

ArrayTableEntry::ArrayTableEntry():
		name_(""), rank_(-1), array_type_(temp_array_t)
		,scalar_selector_(-1)
		,max_block_size_(0)//will be reinitialized in init_calculated_values method
		,min_block_size_(0)//will be reinitialized in init_calculated_valuesmethod
		,num_blocks_(0)//will be reinitialized in init_calculated_values method
		{
	for (int i = 0; i < MAX_RANK; ++i) {
			this->index_selectors_[i] = unused_index_slot;
	}
}

ArrayTableEntry::ArrayTableEntry(std::string name, int rank, ArrayType_t array_type,
		int index_selectors[MAX_RANK], int scalar_selector) :name_(name),
		rank_(rank), array_type_(array_type), scalar_selector_(scalar_selector) {
	for (int i = 0; i < MAX_RANK; ++i) {
		this->index_selectors_[i] = index_selectors[i];
	}
}

void ArrayTableEntry::init(setup::InputStream &file){
	name_ = file.read_string();
	int n = file.read_int();
	rank_ = n;
	for (int i = 0; i != n; ++i){
		(index_selectors_[i]) = file.read_int();
	}
	for (int i = n; i < MAX_RANK; ++i){
		(index_selectors_[i]) = unused_index_slot;
	}
	array_type_ = intToArrayType_t(file.read_int());
	scalar_selector_ = file.read_int();
}

//void ArrayTableEntry::init_calculated_values(const IndexTable& index_table){
//	size_t num_blocks = 1;
//	 size_t max_seg_size =0;
//	 size_t min_seg_size=0;
//	 size_t num_segs =0;
//	 size_t max_block_size = 1;
//	 size_t min_block_size = 1;
//	for (int i = 0 ; i < rank_; ++i){
//		 int index_slot = index_selectors_[i];
//	     index_table.min_max_num_segs(i, min_seg_size, max_seg_size, num_segs);
//	     max_block_size *= max_seg_size;
//	     min_block_size *= min_seg_size;
//	     num_blocks *= num_segs;
//	  }
//	num_blocks_ = num_blocks;
//	max_block_size_ = max_block_size;
//	min_block_size_ = min_block_size;
//}


//size_t ArrayTableEntry::block_number(const sip::BlockId& block_id) const {
//	// Convert rank-dimensional index to 1-dimensional index
//	size_t block_num = 0;
//	size_t tmp = 1;
//	for (int pos = rank_ - 1; pos >= 0; pos--) {
//		int index_slot = index_selectors_[pos];
//		int num_segments = sip_tables_.num_segments(index_slot);
//		sip::check(num_segments >= 0, "num_segments is -ve", current_line());
//		block_num += bid.index_values(pos) * tmp;
//		tmp *= num_segments;
//	}
//	return block_num;
//}
void ArrayTableEntry::init_calculated_values(const IndexTable& index_table){
	size_t num_blocks = 1;
	size_t min_block = 1;
	size_t max_block = 1;
	size_t slice_size = 1;
	for (int pos=0; pos<rank_; pos++){
		int index_slot = index_selectors_[pos];
		int min, max, num_segments, lower;
		index_table.segment_info(index_slot, min, max, num_segments, lower);
		min_block *= min;
		max_block *= max;
		num_blocks *= num_segments;
		slice_sizes_.push_back(slice_size);
		slice_size *= num_segments;
		lower_.push_back(lower);
	}
	num_blocks_ = num_blocks;
	max_block_size_ = max_block;
	min_block_size_ = min_block;
}

size_t ArrayTableEntry::block_number(const BlockId& id) const{
//	std::cout << "in block_number with blockid " << id.str(Interpreter::global_interpreter->sip_tables());

	int res = 0;
	for (int i = rank_-1; i >=0; i--){
//		std::cout << "i=" << i;
//		std::cout << " slice_sizes_[i]=" << slice_sizes_[i];
//		std::cout << " id.index_values(i)=" << id.index_values(i);
//		std::cout << " lower_[i]=" << lower_[i];
		res += (slice_sizes_[i] * (id.index_values(i) - lower_[i]));
	}
//	std::cout << " returning res=" << res << std::endl << std::flush;
	return res;
}

//int ArrayTableEntry::block_number(const BlockId& id) const{
//	std::cout << "in block_number with blockid " << id.str(Interpreter::global_interpreter->sip_tables());
//
//	int res = 0;
//	for (int i = rank_-1; i >=0; i--){
//		std::cout << "i=" << i;
//		std::cout << " slice_sizes_[i]=" << slice_sizes_[i];
//		std::cout << " id.index_values_[i]=" << id.index_values_[i];
//		std::cout << " lower_[i]=" << lower_[i];
//		res += (slice_sizes_[i] * (id.index_values_[i] - lower_[i]));
//	}
//	std::cout << " returning res=" << res << std::endl << std::flush;
//	return res;
//}

std::ostream& operator<<(std::ostream& os,
                const sip::ArrayTableEntry& entry) {
	    os << entry.name_ << ": ";
        os << entry.rank_;
        os << ",[";
        for (int i=0;i < MAX_RANK; ++i){
        	os << entry.index_selectors_[i] << (i < MAX_RANK-1 ? "," : "]");
        }
        os << ',' << entry.array_type_;
        os << ',' << entry.scalar_selector_;
        os << ", num_block_=" << entry.num_blocks_;
        os << ", max_block_size=" << entry.max_block_size_;
        os << ", min_block_size=" << entry.min_block_size_;
        os << ",slice_sizes_(for id linearization)=[";
        for (std::vector<int>::const_iterator iter = entry.slice_sizes_.begin(); iter != entry.slice_sizes_.end(); ++iter){
        	os << *iter << (iter != entry.slice_sizes_.end()-1 ? "," : "");
        }
        os << ']';
        os << ",lower_=[";
        for (std::vector<int>::const_iterator iter = entry.lower_.begin(); iter != entry.lower_.end(); ++iter){
        	os << *iter << (iter != entry.lower_.end()-1 ? "," : "");
        }
        os << ']';
        return os;
}

ArrayTable::ArrayTable() {
}
ArrayTable::~ArrayTable() {
}

void ArrayTable::init(setup::InputStream &file) {
	int n = file.read_int(); //number of entries
	//read info from the .siox file
	for (int i = 0; i < n; ++i) {
		ArrayTableEntry entry;
		entry.init(file);
		entries_.push_back(entry);
		array_name_slot_map_[entry.name_] = i;
	}
}

//void ArrayTable::init_calculated_values(const IndexTable& index_table){
//	int n = entries_.size();
//	for (int array_id = 0; array_id < n; array_id++){
//		entries_[array_id].init_calculated_values(index_table);
//	}
//}

int ArrayTable::array_slot(const std::string & name) const {
	std::map<std::string, int>::const_iterator it = array_name_slot_map_.find(name);
	if (it == array_name_slot_map_.end()){
		throw std::out_of_range("Could not find array with name : " + name);
	}
	return it->second;

}

std::ostream& operator<<(std::ostream& os, const ArrayTable& arrayTableObj) {
	std::vector<ArrayTableEntry> local = arrayTableObj.entries_;
	std::vector<ArrayTableEntry>::iterator it;
	int i = 0;
	for (it = local.begin(); it != local.end(); ++it) {
		os << i++ << ":" <<*it << std::endl;
	}
    return os;
}

} /*namespace sip*/
