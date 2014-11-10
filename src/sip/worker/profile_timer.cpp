/*
 * profile_timer.cpp
 *
 *  Created on: Sep 30, 2014
 *      Author: njindal
 */

#include "profile_timer.h"
#include "sip_interface.h"
#include "global_state.h"
#include "print_timers.h"
#include "profile_timer_store.h"
#include <utility>
#include <iostream>
#include <iomanip>
#include <cassert>
#include <sstream>


namespace sip {

ProfileTimer::ProfileTimer(SialxTimer& sialx_timer) : sialx_timer_(sialx_timer){}

ProfileTimer::~ProfileTimer() {}


//void ProfileTimer::record_time(const ProfileTimer::Key& key, long long time, long long count){
//	TimerMap_t::iterator it = profile_timer_map_.find(key);
//	if (it == profile_timer_map_.end()){
//		profile_timer_map_.insert(std::make_pair(key, ProfileTimer::ValuePair(time, count)));
//	} else {
//		ValuePair& vpair = it->second;
//		std::stringstream err_ss;
//		err_ss << "Trying to overwrite key : " << key
//				<< ", prev [time, count] : [" << vpair.total_time << ", " << vpair.count << "]"
//				<< ", this [time, count] : [" << time << ", " << count << "]";
//		fail(err_ss.str());
//	}
//}

void ProfileTimer::record_line(const ProfileTimer::Key& key, int sialx_line){
	TimerMap_t::iterator it = profile_timer_map_.find(key);
	if (it == profile_timer_map_.end()){
		std::set<int> line_num_set;
		line_num_set.insert(sialx_line);
		profile_timer_map_.insert(std::make_pair(key, line_num_set));
	} else {
		std::set<int>& line_num_set = it->second;
		line_num_set.insert(sialx_line);
	}
}




ProfileTimer::BlockInfo::BlockInfo(int rank, const index_selector_t& index_ids, const segment_size_array_t& segment_sizes) :rank_(rank){
	std::copy(index_ids + 0, index_ids + rank, index_ids_+0);
	std::fill(index_ids_ + rank, segment_sizes_ + MAX_RANK, unused_index_slot);
	std::copy(segment_sizes + 0, segment_sizes + rank, segment_sizes_+0);
	std::fill(segment_sizes_ + rank, segment_sizes_ + MAX_RANK, 1);
}

ProfileTimer::BlockInfo::BlockInfo(const BlockInfo& rhs):
	rank_(rhs.rank_){
	std::copy(rhs.index_ids_ + 0, 		rhs.index_ids_ + MAX_RANK, 		this->index_ids_);
	std::copy(rhs.segment_sizes_ + 0, 	rhs.segment_sizes_ + MAX_RANK, 	this->segment_sizes_);
}
ProfileTimer::BlockInfo& ProfileTimer::BlockInfo::operator=(const BlockInfo& rhs){
	this->rank_ = rhs.rank_;
	std::copy(rhs.index_ids_ + 0, 		rhs.index_ids_ + MAX_RANK, 		this->index_ids_);
	std::copy(rhs.segment_sizes_ + 0, 	rhs.segment_sizes_ + MAX_RANK, 	this->segment_sizes_);
	return *this;
}

ProfileTimer::Key::Key(const std::string& opcode, const std::vector<BlockInfo>& blocks):
	opcode_(opcode), blocks_(blocks){

	// Reduce index_ids to a common comparable format
	// For a contraction with shape [12, 10, 2, 5] = [12, 10, 1, 6] * [1, 6, 2, 5]
	// The first encountered index is given the lowest number,
	// followed by the 2nd encountered index and so on.
	// So then the shape would be stored as
	// [1, 2, 3, 4] = [1, 2, 4, 5] * [4, 5, 3, 4]

	std::map<int, int> old2newIndex;
	int indices_seen = 0;

	// Iterate over all blocks in the Key
	std::vector<BlockInfo>::iterator it = blocks_.begin();
	for (; it != blocks_.end(); ++it){
		index_selector_t& indices = it->index_ids_;
		for (int i=0; i < it->rank_; i++){
			std::map<int, int>::iterator indexit = old2newIndex.find(indices[i]);
			int replace_index = -1;
			// If the index hasn't been encountered before, add it to the old2newIndex map.
			if (indexit == old2newIndex.end()){
				old2newIndex.insert(std::make_pair(indices[i], indices_seen));
				replace_index = indices_seen;
				indices_seen++;
			} else {
				replace_index = indexit->second;
			}
			check(replace_index >= 0, "Error in ProfileTimer::Key::Key", current_line());
			indices[i] = replace_index;
		}
	}
}


std::ostream& operator<<(std::ostream& os, const ProfileTimer::BlockInfo& obj) {
	// Operation Shape
	os << "(";
	os << "[";
	for (int i = 0; i < obj.rank_ ; ++i) {
		int id = obj.index_ids_[i];
		os << (i==0? "" : ",") << id;
	}
	os << ']';

	// Segment Sizes
	os << "{";
	for (int i = 0; i < obj.rank_; ++i) {
		os << (i == 0 ? "" : ",") << obj.segment_sizes_[i];
	}
	os << '}';
	os << ")";
	return os;
}

std::ostream& operator<<(std::ostream& os, const ProfileTimer::Key& obj) {
	os << obj.opcode_ << " ";
	std::vector<ProfileTimer::BlockInfo>::const_iterator it = obj.blocks_.begin();
	for (; it != obj.blocks_.end(); ++it){
		os << (it == obj.blocks_.begin() ? "" : ", ") << *it;
	}
	return os;
}


bool ProfileTimer::BlockInfo::operator==(const ProfileTimer::BlockInfo& rhs) const {
	if (this->rank_ != rhs.rank_)
		return false;

	bool is_eq = true;
	for (int i = 0; i < MAX_RANK; ++i) {
		is_eq = is_eq && (segment_sizes_[i] == rhs.segment_sizes_[i]);
		is_eq = is_eq && (index_ids_[i] == rhs.index_ids_[i]);
	}
	return is_eq;
}


bool ProfileTimer::BlockInfo::array1_lt_array2(int rank,
		const int (&array1)[MAX_RANK], const int (&array2)[MAX_RANK]) const {
	for (int i=0; i<rank; i++){
		if (array1[i] == array2[i]){
			continue;
		} else {
			return array1[i] < array2[i];
		}
	}
	return false;
}

bool ProfileTimer::BlockInfo::array1_eq_array2(int rank,
		const int (&array1)[MAX_RANK], const int (&array2)[MAX_RANK]) const {
	for (int i=0; i<rank; i++){
		if (array1[i] != array2[i]){
			return false;
		}
	}
	return true;
}

bool ProfileTimer::BlockInfo::operator<(const ProfileTimer::BlockInfo& rhs) const {
	if (rank_ != rhs.rank_){
		return rank_ - rhs.rank_ < 0 ? true : false;
	}

	// If segment sizes are equal, compare indices.
	// If indices are equal, return false (since this block is not smaller than rhs block).


	bool segment_sizes_equal = array1_eq_array2(rank_, segment_sizes_, rhs.segment_sizes_);
	if (!segment_sizes_equal){
		bool this_segment_smaller = array1_lt_array2(rank_, segment_sizes_, rhs.segment_sizes_);
		return this_segment_smaller;
	} else {
		bool indices_equal = array1_eq_array2(rank_, index_ids_, rhs.index_ids_);
		if (!indices_equal){
			bool this_indices_smaller = array1_lt_array2(rank_, index_ids_, rhs.index_ids_);
			return this_indices_smaller;
		} else {
			return false;
		}
	}
}

ProfileTimer::Key::Key(const ProfileTimer::Key& rhs):
		opcode_(rhs.opcode_), blocks_(rhs.blocks_){
}

ProfileTimer::Key& ProfileTimer::Key::operator=(const ProfileTimer::Key& rhs){
	this->opcode_ = rhs.opcode_;
	this->blocks_ = rhs.blocks_;	// Vector::operator=
	return *this;
}


bool ProfileTimer::Key::operator<(const ProfileTimer::Key& rhs) const{
	if (opcode_ != rhs.opcode_)
		return opcode_ < rhs.opcode_ ;

	int my_size = blocks_.size();
	int other_size = rhs.blocks_.size();
	if (my_size != other_size)
		return my_size - other_size < 0 ? true : false;


	for (int i=0; i<blocks_.size(); ++i){
		const BlockInfo & this_block = blocks_.at(i);
		const BlockInfo & other_block = rhs.blocks_.at(i);
		if (this_block == other_block)
			continue;
		else return this_block < other_block;
	}
	return false;

}


void ProfileTimer::save_to_store(ProfileTimerStore& profile_timer_store){
	TimerMap_t::const_iterator it = profile_timer_map_.begin();
	for (; it!= profile_timer_map_.end(); ++it){
		const Key &key = it->first;
		const std::set<int>& line_num_set = it->second;
		std::set<int>::const_iterator it = line_num_set.begin();
		long long total_computation_time = 0L;
		long long total_count = 0L;
		for (; it != line_num_set.end(); ++it){
			int line_number = *it;
			long long walltime = sialx_timer_.get_timer_value(line_number, SialxTimer::TOTALTIME);
			long long walltime_count = sialx_timer_.get_timer_count(line_number, SialxTimer::TOTALTIME);
			long long blockwait =  sialx_timer_.get_timer_value(line_number, SialxTimer::BLOCKWAITTIME);

			total_computation_time +=  walltime - blockwait;
			total_count += walltime_count;	// Number of times the line was invoked.
		}

		profile_timer_store.save_to_store(key, std::make_pair(total_computation_time, total_count));
	}
}

void ProfileTimer::print_timers(std::ostream& out) {
	const int CW = 18;	// Time
	const int LW = 40; 	// List of line numbers
	const int SW = 110;	// String
	const int NUM_LINES_TO_PRINT = 5;

	out<<"Timers"<<std::endl
		<<std::setw(SW)<<std::left<<"Type"
		<<std::setw(LW)<<std::left<<"Lines"
		<<std::setw(CW)<<std::left<<"AvgComputation"
		<<std::setw(CW)<<std::left<<"TotComputation"
		<<std::setw(CW)<<std::left<<"Count"
		<<std::endl;

	TimerMap_t::const_iterator it = profile_timer_map_.begin();
	for (; it!= profile_timer_map_.end(); ++it){
		const Key &kp = it->first;
		const std::set<int>& line_num_set = it->second;
		std::set<int>::const_iterator lineit = line_num_set.begin();
		long long total_walltime = 0L;
		long long total_blockwait = 0L;
		long long total_computation_time = 0L;
		long long total_count = 0L;
		std::stringstream line_nums_str;
		bool lines_printed = false;
		int lines = 0;
		int total_lines = line_num_set.size();
		for (; lineit != line_num_set.end(); ++lineit, ++lines){
			int line_number = *lineit;
			long long walltime = sialx_timer_.get_timer_value(line_number, SialxTimer::TOTALTIME);
			long long walltime_count = sialx_timer_.get_timer_count(line_number, SialxTimer::TOTALTIME);
			long long blockwait =  sialx_timer_.get_timer_value(line_number, SialxTimer::BLOCKWAITTIME);
			total_walltime += walltime;
			total_blockwait += blockwait;
			total_count += walltime_count; // Number of times the line was invoked.

			if (lines < NUM_LINES_TO_PRINT && !lines_printed){
				line_nums_str << line_number << ";";
			} else if (!lines_printed){
				line_nums_str << "..." << (total_lines - lines) << " more";
				lines_printed = true;
			}
		}
		total_computation_time =  total_walltime - total_blockwait;

		if (total_count > 0){
			out.precision(6); // Reset precision to 6 places.
			double to_print_tot_time = SipTimer_t::to_seconds(total_computation_time);
			double to_print_avg_time = to_print_tot_time / total_count;
			long to_print_count = total_count;

			std::stringstream ss;
			ss << it->first;

			out<<std::setw(SW)<< std::left << ss.str()
				<< std::setw(LW)<< std::left << line_nums_str.str()
				<< std::setw(CW)<< std::left << to_print_avg_time
				<< std::setw(CW)<< std::left << to_print_tot_time
				<< std::setw(CW)<< std::left << to_print_count
				<< std::endl;
		}

	}
	out<<std::endl;
}



} /* namespace sip */
