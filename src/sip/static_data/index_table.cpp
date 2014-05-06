/*! index_table.cpp
 * 
 *
 *  Created on: Jun 5, 2013
 *      Author: Beverly Sanders
 */

#include "index_table.h"
#include "assert.h"
#include <sstream>
#include <stdexcept>
#include "int_table.h"
#include "array_table.h"
#include "setup_reader.h"
#include "interpreter.h"

namespace sip {
SimpleIndexSegmentDescriptor::~SimpleIndexSegmentDescriptor() {}
std::string SimpleIndexSegmentDescriptor::to_string() const {
	return std::string("simple");
}
int SimpleIndexSegmentDescriptor::get_extent(int segment) const {
	return 1;
}
int SimpleIndexSegmentDescriptor::get_offset(int beg_segment, int segment) const {
	return segment - beg_segment;
}

NonuniformSegmentDescriptor::NonuniformSegmentDescriptor(
		std::vector<int> seg_extents) :
		seg_extents_(seg_extents) {
}
NonuniformSegmentDescriptor::~NonuniformSegmentDescriptor() {
}


std::string NonuniformSegmentDescriptor::to_string() const {
	std::stringstream ss;
	std::vector<int>::const_iterator it;
	ss << '[';
	for (it = seg_extents_.begin(); it != seg_extents_.end(); ++it) {
		if (it != seg_extents_.begin())
			ss << ',';
		ss << *it;
	}
	ss << ']';
	return ss.str();
}

int NonuniformSegmentDescriptor::get_extent(int segment) const {
	try {
		return seg_extents_.at(segment - 1); //convert to c index before retrieving value
	} catch (const std::out_of_range& oor) {
		sip::check(false,
				"SIAL program/ setup data incompatibility.  Not enough segments for index type");
	}
	return 0;  //should not get here
}

int NonuniformSegmentDescriptor::get_offset(int beg_segment, int segment) const{
	sip::check(beg_segment <= segment,
			"SIP or Compiler bug.  Illegal segment value in NonuniformSegmentDescriptor::get_offset");
	int offset = 0;
	for (int i = beg_segment; i < segment; ++i){
		offset += extent(i);
	}
	return offset;
}


SubindexSegmentDescriptor::SubindexSegmentDescriptor(int parent_index_slot,
		int num_subsegments) :
		parent_index_slot_(parent_index_slot), num_subsegments_(num_subsegments) {
}

SubindexSegmentDescriptor::~SubindexSegmentDescriptor() {
}
int SubindexSegmentDescriptor::get_subsegment_offset(int parent_segment,
		int segment) {
	int subsegment_offset = 0;
	for (int i = 1; i < segment; ++i) {
		subsegment_offset += get_subsegment_extent(parent_segment, i);
	}
	return subsegment_offset;
}

int SubindexSegmentDescriptor::get_subsegment_extent(int parent_segment,
		int segment) {
	int parent_extent = sip::Interpreter::global_interpreter->segment_extent(
			parent_index_slot_, parent_segment);
	int nominal_size = (parent_extent / num_subsegments_);
	int rem = parent_extent % num_subsegments_;
	int subsegment_extent = nominal_size + (segment <= rem ? 1 : 0);
	return subsegment_extent;
}

int SubindexSegmentDescriptor::get_extent(int segment) const {
//	int parent_segment_value =
//			sip::Interpreter::global_interpreter->index_value(
//					parent_index_slot_);
//	int extent = get_subsegment_extent(parent_segment_value, segment);
//	std::cout
//			<< "SubindexSegmentDescriptor::  segment, parent_segment_value, get_extent(parent,seg) = "
//			<< segment << "," << parent_segment_value << "," << extent
//			<< std::endl;
//	return extent;
	sip::check(false, "Sip bug:  SubindexSegmentDescriptor::get_extent is unsupported operation");
	return -1;
}

int SubindexSegmentDescriptor::get_offset(int beg_segment, int segment) const{
	sip::check(false,
			"SIP or Compiler bug:  calling get_offset in SubindexSegmentDescriptor");
	return -1;
}

std::string SubindexSegmentDescriptor::to_string() const {
	std::stringstream ss;
	ss << "parent_index_slot_=" << parent_index_slot_;
	ss << " num_subsegments_=" << num_subsegments_;
	return ss.str();
}

IndexTableEntry::IndexTableEntry() {
}
IndexTableEntry::~IndexTableEntry() {
 //   if (index_type_ == subindex && segment_descriptor_ptr_) delete segment_descriptor_ptr_;
}

int IndexTableEntry::segment_extent(int index_value) const {
//	std::cout << "IndexTableEntry::segment_extent " << *this << std::endl;
	assert(
			index_type_ == subindex
					|| lower_seg_ <= index_value
							&& index_value < lower_seg_ + num_segments_);
	return segment_descriptor_ptr_->extent(index_value);
}


int IndexTableEntry::index_extent() const {
	int result = 0;
	for (int i = lower_seg_; i < lower_seg_ + num_segments_; ++i) {
		result += segment_descriptor_ptr_->extent(i);
	}
	return result;
}

int IndexTableEntry::parent_index() const {
	sip::check(index_type_ == subindex,
			"attempting to get parent of an index that is not a subindex");
	return lower_seg_;
}

bool IndexTableEntry::operator==(const IndexTableEntry& rhs) const {
	sip::check(index_type_ != subindex, "== unsupported for subindex");
     return (index_type_ == rhs.index_type_) &&
    		 (lower_seg_ == rhs.lower_seg_) &&
    		 (num_segments_ == rhs.num_segments_);
}

bool IndexTableEntry::operator< (const IndexTableEntry& rhs) const {
	sip::check(index_type_ != subindex, "< unsupported for subindex");
     return (index_type_ == rhs.index_type_) &&
    		 (rhs.lower_seg_ <= lower_seg_) &&
    		 (lower_seg_ + num_segments_ <= rhs.lower_seg_ + rhs.num_segments_);
}

/**
 *
 *  Reads values from .siox file and initializes predefined constants
 */
//TODO  rework this to just create a Segment descriptor when it isn't in the table.
void IndexTableEntry::init(const std::string& name, IndexTableEntry& entry,
		IndexTable& table, setup::InputStream &siox_file,
		setup::SetupReader& setup_info, sip::IntTable& int_table) {
	int bseg = siox_file.read_int();
	int bsegIsSymbolic = siox_file.read_int();
	if (bsegIsSymbolic) {  //this is a symbolic constant, so bseg is actually index in intTable
		bseg = int_table.value(bseg); //lookup and replace value
	}
	int eseg = siox_file.read_int();
	int esegIsSymbolic = siox_file.read_int();
	if (esegIsSymbolic) {  //this is a symbolic constant, so eseg is actually index in intTable
		eseg = int_table.value(eseg); //lookup and replace value
	}
	entry.name_ = name;
	entry.lower_seg_ = bseg;
	entry.num_segments_ = (eseg - bseg) + 1;
	entry.index_type_ = intToIndexType_t(siox_file.read_int());
	if (entry.index_type_ != subindex) { //set subindex_descriptor in IndexTable::init rather than here
		entry.segment_descriptor_ptr_ = table.segment_descriptors_.at(
				entry.index_type_);
		;
	}

}

//void IndexTableEntry::init_symbolic(sip::IntTable& intTable) {
//	int bseg = lower_seg_;
//	int eseg = num_segments_;  //num_segements_ was briefly set to eseg in init
//	if (bseg <= 0) {  //this is a symbolic constant  TODO check == 0
////		int int_table_index = (bseg<0? (-bseg) - 1 : 0); //the intTable uses C indexing
//		int int_table_index = -bseg;
//		bseg = intTable.value(int_table_index);
//	}
//	if (eseg <= 0) {
////		int int_table_index = (eseg<0? (-eseg) - 1 : 0); //the intTable uses C indexing
//		int int_table_index = -eseg;
//		eseg = intTable.value(int_table_index);
//	}
//	lower_seg_ = bseg;
//	num_segments_ = (eseg - bseg) + 1;  //now it is really the size
//	assert(num_segments_ >= 0);
//}

std::ostream& operator<<(std::ostream& os, const IndexTableEntry &entry) {
	os << entry.name_;
	os << ',' << entry.index_type_;
	os << ',' << entry.lower_seg_;
	os << ',' << entry.num_segments_;
	os << ',' << *entry.segment_descriptor_ptr_;
	os << '@' << entry.segment_descriptor_ptr_;
	return os;
}

IndexTable::IndexTable() {
}
IndexTable::~IndexTable() {
	SegmentDescriptorMap::iterator it;
	for (it = segment_descriptors_.begin(); it != segment_descriptors_.end();
			++it) {
		delete it->second;
	}
}

void IndexTable::init(setup::InputStream &siox_file,
		setup::SetupReader& setup_info, sip::IntTable& int_table) {
	//set up the segment info map
	segment_descriptors_[simple] = new SimpleIndexSegmentDescriptor;
	setup::SetupReader::SetupSegmentInfoMap::iterator it;
	for (it = setup_info.segment_map_.begin();
			it != setup_info.segment_map_.end(); ++it) {
		IndexType_t index_type = it->first;
		segment_descriptors_[index_type] = new NonuniformSegmentDescriptor(
				it->second);
	}

	int n = siox_file.read_int(); //number of entries
	for (int i = 0; i < n; ++i) {
		std::string name = siox_file.read_string();
		IndexTableEntry entry;
		IndexTableEntry::init(name, entry, *this, siox_file, setup_info,
				int_table);
//		std::cout << "index table entry[" << i << "]=" << entry << std::endl;
		name_entry_map_[name] = i;
		if (entry.index_type_ == subindex) {
			int parent = entry.lower_seg_;
			entry.segment_descriptor_ptr_ = new SubindexSegmentDescriptor(
					parent); //assuming default #segments
		}
		entries_.push_back(entry);
	}
}

int IndexTable::segment_extent(int index_id, int index_value) const {
	if (index_id == unused_index_slot)
		return unused_index_segment_size;
	const IndexTableEntry &entry = entries_.at(index_id);
	return entry.segment_extent(index_value);
}

int IndexTable::index_extent(int index_slot) const{
	return entries_.at(index_slot).index_extent();
}

int IndexTable::offset_into_contiguous(int index_slot, int index_value) const{
	const IndexTableEntry entry = entries_.at(index_slot);
	return entry.segment_descriptor_ptr_->offset(entry.lower_seg_, index_value);
}
int IndexTable::lower_seg(int index_slot) const {
	return entries_.at(index_slot).lower_seg_;
}

int IndexTable::num_segments(int index_slot) const {
	return entries_.at(index_slot).num_segments_;
}

std::string IndexTable::index_name(int index_slot) const {
	return entries_.at(index_slot).name_;
}

IndexType_t IndexTable::index_type(int index_slot) const {
	return entries_.at(index_slot).index_type_;
}

bool IndexTable::is_subindex(int index_slot) const {
	return entries_.at(index_slot).index_type_ == subindex;
}

int IndexTable::parent(int subindex_slot) const {
	return entries_.at(subindex_slot).parent_index();
}

/*! returns the offset of this subsegment within the parent segment */
int IndexTable::subsegment_offset(int subindex_slot, int parent_segment_value,
		int subsegment_value) const {
	sip::check(is_subindex(subindex_slot),
			"attempting to get subsegment offset of an index that is not a subindex");
	SubindexSegmentDescriptor * desc =
			static_cast<SubindexSegmentDescriptor*>(entries_.at(subindex_slot).segment_descriptor_ptr_);
	return desc->subsegment_offset(parent_segment_value, subsegment_value);
}

int IndexTable::subsegment_extent(int subindex_slot, int parent_segment_value,
		int subsegment_value) const {
//	std::cout << "IndexTable::subsegment_extent(int subindex_slot, int parent_segment_value, int subsegment_value)" <<
//			subindex_slot << ',' << parent_segment_value << ',' << subsegment_value << std::endl;
	SubindexSegmentDescriptor * desc =
			static_cast<SubindexSegmentDescriptor*>(entries_.at(subindex_slot).segment_descriptor_ptr_);
//	std::cout << "*desc=" << *desc << std::endl;
	int tmp = desc->subsegment_extent(parent_segment_value, subsegment_value);
//	std::cout << "returning IndexTable::subsegment_extent " << tmp << std::endl;
	return tmp;
}

int IndexTable::num_subsegments(int index_slot, int parent_segment_value) const {
//	std::cout << "DEBUG IndexTable::num_subsegments:: index_slot=" << index_slot
//			<< " parent_segment_value=" << parent_segment_value << std::endl;
	IndexTableEntry subindex_entry = entries_.at(index_slot);
//	std::cout << "DEBUG IndexTable::num_subsegments:: entry= " << subindex_entry
//			<< std::endl;
//	std::cout << "desc=" << *subindex_entry.segment_descriptor_ptr_
//			<< " at address " << subindex_entry.segment_descriptor_ptr_
//			<< std::endl;
	SegmentDescriptor * desc = entries_.at(index_slot).segment_descriptor_ptr_;
//	std::cout << "DEBUG IndexTable::num_subsegments:: desc= " << *desc
//			<< std::endl;
	SubindexSegmentDescriptor * subdesc =
			static_cast<SubindexSegmentDescriptor*>(desc);
//	std::cout << "DEBUG IndexTable::num_subsegments:: parent's desc= " << entries_.at(index_slot). << std::endl;
//	std::cout
//			<< "DEBUG IndexTable::SubindexSegmentDescriptor subdesc after static cast tostring\n"
//			<< subdesc->to_string() << std::endl;
//	std::cout << "returned from subdesc->to_string()" << std::endl;
	int toreturn = subdesc->num_subsegments(parent_segment_value);
//	std::cout << "DEBUG IndexTable::SubindexSegmentDescriptor  returning"
//			<< std::endl;
	return toreturn;
}

std::ostream& operator<<(std::ostream& os, const IndexTable& indexTableObj) {
	std::vector<IndexTableEntry>::const_iterator it;
	const std::vector<IndexTableEntry> &table = indexTableObj.entries_;
	int i = 0;
	for (it = table.begin(); it != table.end(); ++it, ++i) {
		os << i << ":";
		os << *it;
		os << std::endl;
	}

	return os;
}

} /*namespace sip*/
