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
		rank_(rank), array_type_(array_type), scalar_selector_(scalar_selector)
		,max_block_size_(0)//will be reinitialized in init_calculated_values method
		,min_block_size_(0)//will be reinitialized in init_calculated_valuesmethod
		,num_blocks_(0)//will be reinitialized in init_calculated_values method
		{
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

void ArrayTableEntry::init_calculated_values(const IndexTable& index_table){
	size_t num_blocks = 1;
	size_t min_block = 1;
	size_t max_block = 1;
	size_t slice_size = 1;
	slice_sizes_.resize(rank_,-1);
	lower_.resize(rank_,-1);
	for (int pos = rank_-1; pos >= 0; pos--){
		int index_slot = index_selectors_[pos];
		int min, max, num_segments, lower;
		index_table.segment_info(index_slot, min, max, num_segments, lower);
		min_block *= min;
		max_block *= max;
		num_blocks *= num_segments;
		slice_sizes_[pos] = slice_size;
		slice_size *= num_segments;
		lower_[pos] = lower;
	}
	num_blocks_ = num_blocks;
	max_block_size_ = max_block;
	min_block_size_ = min_block;
}

size_t ArrayTableEntry::block_number(const BlockId& id) const{
	size_t res = 0;
	for (int i = 0; i < rank_; i++){
		res += (slice_sizes_[i] * (id.index_values(i) - lower_[i]));
	}
	return res;
}

BlockId ArrayTableEntry::num2id(int array_id, size_t block_number) const{
	index_value_array_t index_array;
	size_t num = block_number;
	for (int i = 0; i < rank_; i++){
		int q = num/slice_sizes_[i];
		index_array[i] = q + lower_[i];
		num -= (q*slice_sizes_[i]);
	}
	for (int j = rank_; j< MAX_RANK; j++){
		index_array[j] = unused_index_value;
	}
	return BlockId(array_id, index_array);
}
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
        for (std::vector<long>::const_iterator iter = entry.slice_sizes_.begin(); iter != entry.slice_sizes_.end(); ++iter){
        	os << *iter << (iter != entry.slice_sizes_.end()-1 ? "," : "");
        }
        os << ']';
        os << ",lower_=[";
        for (std::vector<long>::const_iterator iter = entry.lower_.begin(); iter != entry.lower_.end(); ++iter){
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
