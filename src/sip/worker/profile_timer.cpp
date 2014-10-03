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
#include <utility>
#include <iostream>
#include <iomanip>
#include <cassert>

namespace sip {

ProfileTimer::ProfileTimer(int sialx_lines) :
		sialx_lines_(sialx_lines), delegate_(sialx_lines),
		slot_to_assign_(0){
}

ProfileTimer::~ProfileTimer() {}

void ProfileTimer::start_timer(const ProfileTimer::Key& key){
	/** For the given key, find the slot in the profile_timer_map_
	 * If the key cannot be found, create a new entry in the profile_timer_map_
	 * and increment the slot_to_assign_.
	 * Then start the timer in the underlying delegate for the given slot.
	 */
	TimerMap_t::iterator it = profile_timer_map_.find(key);
	if (it == profile_timer_map_.end()){
		std::pair<TimerMap_t::iterator, bool> returned =
				profile_timer_map_.insert (std::make_pair(key, slot_to_assign_));
		slot_to_assign_ ++;
		check(slot_to_assign_ < sialx_lines_, "Out of space in the Profile timer !");
		it = returned.first;
	}
	int slot = it->second;
	delegate_.start_timer(slot);

}

void ProfileTimer::pause_timer(const ProfileTimer::Key& key){
	/** For the given key, find the slot in the profile_timer_map_
	 * If the key cannot be found, it is an error !!
	 * Pause the timer in the underlying delegate for the given slot.
	 */
	TimerMap_t::iterator it = profile_timer_map_.find(key);
	check (it != profile_timer_map_.end(), "Timer stopped for a key that hasn't been encountered yet !", current_line());
	int slot = it->second;
	delegate_.pause_timer(slot);
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

ProfileTimer::Key::Key(opcode_t opcode, const std::vector<BlockInfo>& blocks):
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
	os << "[" << obj.index_ids_[0];
	for (int i = 1; i < obj.rank_ ; ++i) {
		int id = obj.index_ids_[i];
		if (id != unused_index_slot)
			os << "," << id;
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
	os << opcodeToName(obj.opcode_) << " ";
	std::vector<ProfileTimer::BlockInfo>::const_iterator it = obj.blocks_.begin();
	os << *it; ++it;
	for (; it != obj.blocks_.end(); ++it){
		os << ", " << *it;
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

bool ProfileTimer::BlockInfo::operator<(const ProfileTimer::BlockInfo& rhs) const {
	bool is_eq = true;
	bool is_leq = true;
	for (int i = 0; is_leq && i < MAX_RANK; ++i) {
		is_leq = (segment_sizes_[i] <= rhs.segment_sizes_[i]);
		is_eq = is_eq && (segment_sizes_[i] == rhs.segment_sizes_[i]);
		is_leq = (index_ids_[i] <= rhs.index_ids_[i]);
		is_eq = is_eq && (index_ids_[i] == rhs.index_ids_[i]);
	}
	return (is_leq && !is_eq);
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
		return opcode_ - rhs.opcode_ < 0 ? true : false;

	int my_size = blocks_.size();
	int other_size = rhs.blocks_.size();
	if (my_size != other_size)
		return my_size - other_size < 0 ? true : false;

	std::vector<BlockInfo>::const_iterator it = blocks_.begin();
	bool is_eq = true;
	bool is_leq = true;
	for (int i=0; is_leq && i<blocks_.size(); i++){
		const BlockInfo & this_block = blocks_.at(i);
		const BlockInfo & other_block = rhs.blocks_.at(i);
		is_eq = is_eq &&  this_block == other_block;
		is_leq = this_block < other_block;
	}
	return (is_leq && !is_leq);
}

template <typename TIMER>
class SingleNodeProfilePrint : public PrintTimers<TIMER> {
private:
	const ProfileTimer::TimerMap_t& profile_timer_map_;
public:
	SingleNodeProfilePrint(const ProfileTimer::TimerMap_t &profile_timer_map)
		: profile_timer_map_(profile_timer_map) {}
	virtual ~SingleNodeProfilePrint(){}
	virtual void execute(TIMER& timer){

		//std::cout << "Timers for Program " << GlobalState::get_program_name() << std::endl;

		long long * timers = timer.get_timers();
		long long * timer_counts = timer.get_timer_count();
		const int CW = 15;	// Time
		const int SW = 40;	// String

		assert(timer.check_timers_off());
		std::cout<<"Timers"<<std::endl
			<<std::setw(SW)<<std::left<<"Type"
			<<std::setw(CW)<<std::left<<"Avg"
			<<std::setw(CW)<<std::left<<"Tot"
			<<std::endl;

		std::cout.precision(6); // Reset precision to 6 places.
		//for (int i=1; i<timer.max_slots - sialx_lines_; i++){
		ProfileTimer::TimerMap_t::const_iterator it = profile_timer_map_.begin();
		for (; it != profile_timer_map_.end(); ++it){
			int i = it->second;

			double tot_time = timer.to_seconds(timers[i]);	// Microsecond to second
			double avg_time = tot_time / timer_counts[i];

			std::cout<<std::setw(SW)<< std::left << it->first
					<< std::setw(CW)<< std::left << avg_time
					<< std::setw(CW)<< std::left << tot_time
					<< std::endl;
		}
		std::cout<<std::endl;
	}
};

void ProfileTimer::print_timers(){
	SingleNodeProfilePrint<TimerType_t> p(profile_timer_map_);
	delegate_.print_timers(p);
}


} /* namespace sip */
