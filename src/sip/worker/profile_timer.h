/*
 * profile_timer.h
 *
 *  Created on: Sep 30, 2014
 *      Author: njindal
 */

#ifndef PROFILE_TIMER_H_
#define PROFILE_TIMER_H_

#include <map>
#include <vector>
#include <set>
#include <iostream>
#include "sip.h"
#include "opcode.h"
#include "sip_timer.h"
#include "sialx_timer.h"

class ProfileInterpreterTestControllerParallel;

namespace sip {
class ProfileTimerStore;

class ProfileTimer {
public:

	ProfileTimer(SialxTimer& sialx_timer);
	~ProfileTimer();


	/** Info about each block involved in the operation */
	struct BlockInfo {
		int rank_;
		index_selector_t index_ids_;			//! For block selector or operation shape
		segment_size_array_t segment_sizes_; 	//! For block segment sizes
		BlockInfo(int, const index_selector_t&, const segment_size_array_t&);
		BlockInfo(const BlockInfo& rhs);
		BlockInfo& operator=(const BlockInfo& rhs);
		bool operator==(const BlockInfo& rhs) const;
		bool operator<(const BlockInfo& rhs) const;
		friend std::ostream& operator<<(std::ostream&, const ProfileTimer::BlockInfo&);

	private:
		/**
		 * Is array1 less than array2
		 * @param rank
		 * @param array1
		 * @param array2
		 * @return
		 */
		bool array1_lt_array2(int rank, const int (&array1)[MAX_RANK],
				const int (&array2)[MAX_RANK]) const;

		/**
		 * Is array1 equal to array2
		 * @param rank
		 * @param array1
		 * @param array2
		 * @return
		 */
		bool array1_eq_array2(int rank, const int (&array1)[MAX_RANK],
						const int (&array2)[MAX_RANK]) const;
	};

	/** Each operation will be profiled based on opcode & list of arguments
	 * The list of arguments captures only
	 * 1. the shape of the blocks &
	 * 2. the shape of the operation - encoded as indices 1, 2, 3, 4...
	 * For a contraction C[a,b] = A [a,i] * B[b,j],
	 * The shape of the operation is [1,2], [1,3], [2,4]
	 * For a super instruction "execute usersuperinst A[i,j] B[q,f] C[j,i]"
	 * The shape of the operation is [1,2], [3,4], [2,1]
	 */
	struct Key {
		std::string opcode_;
		std::vector<BlockInfo> blocks_;
		Key() : opcode_("invalid_op") {}
		Key(const std::string&, const std::vector<BlockInfo>&);
		Key(const Key& rhs);
		Key& operator=(const Key& rhs);
		bool operator<(const Key& rhs) const;
		friend std::ostream& operator<<(std::ostream&, const ProfileTimer::Key&);
	};

//	struct ValuePair{
//		long long total_time;
//		long long count;
//		ValuePair(long long tt, long long c) : total_time(tt), count(c) {}
//	};
//
//	typedef std::map<Key, ValuePair> TimerMap_t;

	//void record_time(const ProfileTimer::Key& key, long long time, long long count);


	typedef std::map<Key, std::set<int> > TimerMap_t;	// Key -> Set of program counters

	/**
	 * Records occurrence of operation & operand type at given program counter
	 * @param key
	 * @param pc
	 */
	void record_line(const ProfileTimer::Key& key, int pc);

	/** Prints timers to an ostream */
	void print_timers(std::ostream& out);

	/** Saves timer to underlying ProfileTimerStore */
	void save_to_store(ProfileTimerStore& profile_timer_store);

	friend class ::ProfileInterpreterTestControllerParallel;

private:
	SialxTimer& sialx_timer_;
	TimerMap_t profile_timer_map_; 				//! Key -> Set of program counters


	DISALLOW_COPY_AND_ASSIGN(ProfileTimer);

};

} /* namespace sip */

#endif /* PROFILE_TIMER_H_ */
