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

ProfileTimer::ProfileTimer(int sialx_lines, ProfileTimerStore* profile_timer_store) :
		// An approximate maximum of the number of slots needed = sialx_lines_ * 3.
		// Increase this if needed.
		max_slots_(sialx_lines * 3), delegate_(sialx_lines * 3),
		profile_timer_store_(profile_timer_store),	// Can be NULL
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
		check(slot_to_assign_ < max_slots_, "Out of space in the Profile timer !");
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
		const int SW = 110;	// String

		assert(timer.check_timers_off());
		std::cout<<"Timers"<<std::endl
			<<std::setw(SW)<<std::left<<"Type"
			<<std::setw(CW)<<std::left<<"Avg"
			<<std::setw(CW)<<std::left<<"Tot"
			<<std::setw(CW)<<std::left<<"Count"
			<<std::endl;

		std::cout.precision(6); // Reset precision to 6 places.
		ProfileTimer::TimerMap_t::const_iterator it = profile_timer_map_.begin();
		for (; it != profile_timer_map_.end(); ++it){
			int i = it->second;

			double tot_time = timer.to_seconds(timers[i]);	// Microsecond to second
			double avg_time = tot_time / timer_counts[i];
			long count = timer_counts[i];

			std::stringstream ss;
			ss << it->first;

			std::cout<<std::setw(SW)<< std::left << ss.str()
					<< std::setw(CW)<< std::left << avg_time
					<< std::setw(CW)<< std::left << tot_time
					<< std::setw(CW)<< std::left << count
					<< std::endl;
		}
		std::cout<<std::endl;
	}
};


template <typename TIMER>
class SingleNodeProfileStore : public PrintTimers<TIMER> {
private:
	const ProfileTimer::TimerMap_t& profile_timer_map_;
	ProfileTimerStore* profile_timer_store_;
public:
	SingleNodeProfileStore(const ProfileTimer::TimerMap_t &profile_timer_map, ProfileTimerStore* profile_timer_store)
		: profile_timer_map_(profile_timer_map), profile_timer_store_(profile_timer_store) {
		check(profile_timer_store != NULL, "Assigning a NULL profile_timer_store to SingleNodeProfileStore instance");
	}
	virtual ~SingleNodeProfileStore(){}
	virtual void execute(TIMER& timer){

		// Do nothing if profile_timer_store is null
		if (profile_timer_store_ == NULL)
			return;

		long long * timers = timer.get_timers();
		long long * timer_counts = timer.get_timer_count();

		assert(timer.check_timers_off());

		ProfileTimer::TimerMap_t::const_iterator it = profile_timer_map_.begin();
		for (; it != profile_timer_map_.end(); ++it){
			const ProfileTimer::Key& key = it->first;
			int i = it->second;
			long tot_time = 	timers[i];
			long count 	= 	timer_counts[i];
			std::pair<long, long> time_count_pair = std::make_pair(tot_time, count);
			profile_timer_store_->save_to_store(key, time_count_pair);
		}
		std::cout<<std::endl;
	}
};


void ProfileTimer::print_timers(){
	SingleNodeProfilePrint<TimerType_t> print_to_stdout(profile_timer_map_);
	SingleNodeProfileStore<TimerType_t> save_to_store(profile_timer_map_, profile_timer_store_);
	delegate_.print_timers(print_to_stdout);
	delegate_.print_timers(save_to_store);
}


} /* namespace sip */
