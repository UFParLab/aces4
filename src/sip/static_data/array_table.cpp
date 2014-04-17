/*! ArrayDescriptor.cpp
 * 
 *
 *  Created on: Jun 5, 2013
 *      Author: Beverly Sanders
 */

#include "array_table.h"
#include "sip_tables.h"

namespace sip {

//const int ArrayTableEntry::unused_index_slot = -1;//chosen to throw out of bound exception if accessed.

ArrayTableEntry::ArrayTableEntry(){}

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

//void ArrayTable::init_num_blocks(){
//	int n = entries_.size();
//	SipTables& sip_tables = SipTables::instance();
//	for (unsigned array_id = 0; array_id < n; array_id++){
//		// Calculate total number of blocks
//		int num_blocks = 1;
//		for (int pos=0; pos<rank(array_id); pos++){
//			int index_slot = sip_tables.selectors(array_id)[pos];
//			int num_segments = sip_tables.num_segments(index_slot);
//			num_blocks *= num_segments;
//		}
//		entries_[array_id].num_blocks_ = num_blocks;
//	}
//}

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
