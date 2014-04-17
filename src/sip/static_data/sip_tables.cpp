/*! SipTables.cpp
 * 
 *
 *  Created on: Jun 6, 2013
 *      Author: Beverly Sanders
 */

#include "sip_tables.h"
#include <assert.h>
#include <cstddef>
#include "array_constants.h"
#include "interpreter.h"
#include "setup_reader.h"
#include <vector>

namespace sip {

SipTables::SipTables(setup::SetupReader& setup_reader, setup::InputStream& input_file):
	setup_reader_(setup_reader), siox_reader_(*this, input_file, setup_reader),	sialx_lines_(-1){
}

SipTables::~SipTables() {
}

int SipTables::max_timer_slots() const{
	if (sialx_lines_ > 0)
		return sialx_lines_;

	int max_line = 0;
	int size = op_table_.size();
	for (int i=0; i<size; i++){
		int line = op_table_.line_number(i);
		if (line > max_line)
			max_line = line;
	}
	sialx_lines_ = max_line;
	return max_line;
}

std::vector<std::string> SipTables::line_num_to_name() const{
	std::vector<std::string> lno2name(max_timer_slots()+1, "");
	int size = op_table_.size();
	for (int i=0; i<size; i++){
		int line = op_table_.line_number(i);
		opcode_t opcode = intToOpcode(op_table_.opcode(i));
		if (printableOpcode(opcode))
			lno2name[line] = opcodeToName(opcode);
	}
	return lno2name;
}


std::string SipTables::array_name(int array_table_slot) const{
	return array_table_.array_name(array_table_slot);
}
std::string SipTables::scalar_name(int array_table_slot) const{ //the same as array_table_name
	assert(is_scalar(array_table_slot));
	return array_table_.array_name(array_table_slot);
}
int SipTables::array_rank(int array_table_slot)  const{
	return array_table_.rank(array_table_slot);
}

int SipTables::array_rank(const sip::BlockId& id) const{
	return array_table_.rank(id.array_id());
}
bool SipTables::is_scalar(int array_table_slot)  const{
	int attr = array_table_.array_type(array_table_slot);
	return sip::is_scalar_attr(attr);
}
bool SipTables::is_auto_allocate(int array_table_slot) const {
	int attr = array_table_.array_type(array_table_slot);
	return sip::is_auto_allocate_attr(attr);
}

bool SipTables::is_scope_extent(int array_table_slot) const {
	int attr = array_table_.array_type(array_table_slot);
	return sip::is_scope_extent_attr(attr);
}

bool SipTables::is_contiguous(int array_table_slot) const {
	int attr = array_table_.array_type(array_table_slot);
	return sip::is_contiguous_attr(attr);
}

bool SipTables::is_predefined(int array_table_slot) const {
	int attr = array_table_.array_type(array_table_slot);
	return sip::is_predefined_attr(attr);
}

bool SipTables::is_distributed(int array_table_slot) const {
	int attr = array_table_.array_type(array_table_slot);
	return sip::is_sip_consistent_attr(attr);
}

bool SipTables::is_served(int array_table_slot) const {
	int attr = array_table_.array_type(array_table_slot);
	return sip::is_sip_consistent_attr(attr);
}

int SipTables::num_arrays() const{
	return array_table_.entries_.size();
}

int SipTables::int_value(int int_table_slot) const {
	return int_table_.value(int_table_slot);
}

std::string SipTables::string_literal(int slot) const {
	return string_literal_table_.at(slot);
}
std::string SipTables::index_name(int index_table_slot) const {
	return index_table_.index_name(index_table_slot);
}

sip::IndexType_t SipTables::index_type(int index_table_slot) const {
	return index_table_.index_type(index_table_slot);
}

int SipTables::block_size(const int array_id,
		const index_value_array_t& index_vals) const {
	int seg_sizes[MAX_RANK];
	calculate_seq_sizes(array_id, index_vals, seg_sizes);
	int num_elems = 1;
	for (int i = 0; i < MAX_RANK; i++)
		num_elems *= seg_sizes[i];
	return num_elems;
}

int SipTables::block_size(const BlockId& bid) const{
	const int array_id = bid.array_id_;
	const index_value_array_t & index_vals = bid.index_values_;
	int num_elems = block_size(array_id, index_vals);
	return num_elems;
}


void SipTables::calculate_seq_sizes(const int array_table_slot,
		const index_value_array_t& index_vals, int seg_sizes[MAX_RANK]) const {
	int rank = array_table_.rank(array_table_slot);
	const sip::index_selector_t& selectors = array_table_.index_selectors(
			array_table_slot);
	for (int i = 0; i < rank; ++i) {
		int index_id = selectors[i];
		if (is_subindex(index_id)) {
			int parent_value =
					sip::Interpreter::global_interpreter->data_manager_.index_value(
							parent_index(index_id));
			seg_sizes[i] = index_table_.subsegment_extent(index_id,
					parent_value, index_vals[i]);
		} else {
			seg_sizes[i] = index_table_.segment_extent(selectors[i],
					index_vals[i]);
		}
	}
	std::fill(seg_sizes + rank, seg_sizes + MAX_RANK, 1);
}

long SipTables::block_offset_in_array(const BlockId& bid) const{
	const int array_id = bid.array_id_;
	const index_value_array_t &index_vals = bid.index_values_;
	return block_indices_offset_in_array(array_id, index_vals);
}

long SipTables::array_num_elems(const int array_id) const{
	/* Index values are constructed for the upper limit of the array indices
	 * and passed to a helper method to count the "offset"
	 */
	const index_selector_t& selector = selectors(array_id);
	int rank = array_rank(array_id);
	index_value_array_t upper_index_vals;
	std::fill(upper_index_vals, upper_index_vals + MAX_RANK, unused_index_value);
	// Initialize
	for (int i = 0; i < rank; i++) {
		int selector_index = selector[i];
		int lower = lower_seg(selector_index);
		int upper = lower + num_segments(selector_index);
		upper_index_vals[i] = upper;
	}
	return block_indices_offset_in_array(array_id, upper_index_vals);

}


long SipTables::block_indices_offset_in_array(const int array_id,
		const index_value_array_t& index_vals) const {
	/*
	 * This method assumes that the blocks are laid out on disk
	 * in column-major oredring. i.e. A[0,0] is followed by A[1,0].
	 */

	/* This method iterates over all the indices, incrementing
	 * an "offset" count as it goes along. When the running indices
	 * equal to the "index_vals" argument, the loop exits and the
	 * calculated "offset" is returned.
	 */

	const index_selector_t& selector = selectors(array_id);
	int rank = array_rank(array_id);
	index_value_array_t upper_index_vals;
	std::fill(upper_index_vals, upper_index_vals + MAX_RANK, unused_index_value);
	index_value_array_t lower_index_vals;
	std::fill(lower_index_vals, lower_index_vals + MAX_RANK, unused_index_value);
	index_value_array_t current_index_vals;
	std::fill(current_index_vals, current_index_vals + MAX_RANK,
			unused_index_value);
	// Initialize
	for (int i = 0; i < rank; i++) {
		int selector_index = selector[i];
		int lower = lower_seg(selector_index);
		int upper = lower + num_segments(selector_index);
		lower_index_vals[i] = lower;
		current_index_vals[i] = lower_index_vals[i];
		upper_index_vals[i] = upper;
	}
	// Get the upper & lower ranges, increment the "current_seg_vals"
	// till it reaches the "block_seg_vals".
	// Sum up the block sizes into "tot_fp_elems"
	long tot_fp_elems = 0;
	int pos = 0;
	while (pos < rank) {
		// increment block_offset
		bool equal_to_input_blockid = true;
		for (int i = 0; i < MAX_RANK; i++)
			equal_to_input_blockid &= current_index_vals[i] == index_vals[i];
		if (equal_to_input_blockid)
			break;
		else {
			int elems = block_size(array_id, current_index_vals);
			tot_fp_elems += elems;
		}
		// Increment indices.
		current_index_vals[pos]++;
		if (current_index_vals[pos] < upper_index_vals[pos]) {
			break;
		} else {
			current_index_vals[pos] = lower_index_vals[pos];
			pos++;
		}
	}
	return tot_fp_elems;
}

//array::BlockShape& SipTables::shape(const array::BlockId& block_id) const{
//	int array_table_slot = block_id.array_id_;
//	int rank = array_table_.rank(array_table_slot);
//	array::index_selector_t& selectors = array_table_.index_selectors(array_table_slot);
//	int seg_sizes[MAX_RANK];
//	for (int i = 0; i < MAX_RANK; ++i) {
//		seg_sizes[i] = index_table_.segment_extent(selectors[i],
//				block_id.index_values_[i]);
//	}
////	std::cout << "seg_sizes: ";
////
////	for (int i = 0; i < MAX_RANK; ++i){
////		std::cout << (i==0?"":",") << seg_sizes[i];
////	}
////	std::cout << std::endl;
//	return array::BlockShape(seg_sizes);
//}

sip::BlockShape SipTables::shape(const sip::BlockId& block_id) const {
	const int array_table_slot = block_id.array_id();
	const index_value_array_t & index_vals = block_id.index_values_;

	int seg_sizes[MAX_RANK];
	calculate_seq_sizes(array_table_slot, index_vals, seg_sizes);
	return sip::BlockShape(seg_sizes);
}

sip::BlockShape SipTables::contiguous_array_shape(int array_id) const{
	int rank = array_table_.rank(array_id);
	const sip::index_selector_t& selector = array_table_.index_selectors(array_id);
	sip::segment_size_array_t seg_sizes;
	for (int i = 0; i < rank; ++i){
		seg_sizes[i] = index_table_.index_extent(selector[i]);
	}
	std::fill(seg_sizes+rank, seg_sizes + MAX_RANK, 1);
	return sip::BlockShape(seg_sizes);
}

int SipTables::offset_into_contiguous(int selector, int value) const{
	return index_table_.offset_into_contiguous(selector, value);
}
const sip::index_selector_t& SipTables::selectors(int array_id) const{
	return array_table_.index_selectors(array_id);
}
int SipTables::lower_seg(int index_table_slot) const {
	return index_table_.lower_seg(index_table_slot);
}
int SipTables::num_segments(int index_table_slot) const {
	return index_table_.num_segments(index_table_slot);
}
int SipTables::num_subsegments(int index_slot, int parent_segment_value) const {return index_table_.num_subsegments(index_slot, parent_segment_value);}
bool SipTables::is_subindex(int index_table_slot) const {return index_table_.is_subindex(index_table_slot);}
int SipTables::parent_index(int subindex_slot) const {return index_table_.parent(subindex_slot);}

void SipTables::print() const {
	std::cout << *this << std::endl;
}

std::ostream& operator<<(std::ostream& os, const SipTables& obj) {
	//index table
	os << "Index Table: " << std::endl;
	os << obj.index_table_;
	os << std::endl;
	//array table
	os << "Array Table: " << std::endl;
	os << obj.array_table_;
	os << std::endl;
	//op table
	os << "Op Table " << std::endl;
	os << obj.op_table_;
	os << std::endl;
	//scalar table
	os << "Scalar Table: (" << obj.scalar_table_.size() << " entries)" <<std::endl;
	for (int i = 0; i < obj.scalar_table_.size(); ++i) {
		double val = obj.scalar_table_[i];
		if (val != 0.0){
			os << i << ":" << obj.scalar_table_[i] << std::endl;
		}
	}
	os << std::endl;
	//int table
	os << "Int Table (predefined symbolic constants):" << std::endl;
	os << obj.int_table_;
	os << std::endl;
	// special instructions
	os << "Special Instructions: " << std::endl;
	os << obj.special_instruction_manager_;
	os << std::endl;
	//string literal table
	os << "String Literal Table:" << std::endl;
	for (int i = 0; i < obj.string_literal_table_.size(); ++i) {
		os << obj.string_literal_table_[i] << '\n';
	}
	os << std::endl;
	os << std::endl;
	return os;
}

} /* namespace sip */

