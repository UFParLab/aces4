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

#include "sip.h"
#include "opcode.h"
#include "sip_timer.h"

namespace sip {

class ProfileTimerStore;

class ProfileTimer {
public:

	ProfileTimer(int sialx_lines, ProfileTimerStore* profile_timer_store);
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

	typedef std::map<ProfileTimer::Key, int> TimerMap_t;

	void start_timer(const ProfileTimer::Key& key);
	void pause_timer(const ProfileTimer::Key& key);

	void print_timers();

private:

	ProfileTimerStore* profile_timer_store_;	//! Non-volatile store for profile timers
	TimerMap_t profile_timer_map_; 	//! Key -> index slot in underlying timer
	int slot_to_assign_;			//! Next slot to assign in the delegate_

	/** Underlying timer either Linux, PAPI or TAU timers */
#ifdef HAVE_TAU
	COMPILER ERROR - NOT SUPPORTED  // FIXME TODO
	typedef TAUSIPTimers TimerType_t;
#elif defined HAVE_PAPI
	typedef PAPISIPTimers TimerType_t;
#else
	typedef LinuxSIPTimers TimerType_t;
#endif

	TimerType_t delegate_;
	const int max_slots_;

	DISALLOW_COPY_AND_ASSIGN(ProfileTimer);
};

} /* namespace sip */

#endif /* PROFILE_TIMER_H_ */
