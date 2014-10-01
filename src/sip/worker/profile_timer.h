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

class ProfileTimer {
public:

	ProfileTimer(int sialx_lines);
	~ProfileTimer();


	/** Info about each block involved in the operation */
	struct BlockInfo {
		const int rank_;
		index_selector_t index_ids_;			//! For block selector or operation shape
		segment_size_array_t segment_sizes_; 	//! For block segment sizes
		BlockInfo(int, const index_selector_t&, const segment_size_array_t&);
		bool operator==(const BlockInfo& rhs) const;
		bool operator<(const BlockInfo& rhs) const;
		friend std::ostream& operator<<(std::ostream&, const ProfileTimer::BlockInfo&);
	};

	/** Each operation will be profiled based on opcode & list of arguments
	 * The list of arguments captures only
	 * 1. the shape of the blocks &
	 * 2. the shape of the operation - encoded as indices 1, 2, 3, 4...
	 * For a contraction C[a,b] = A [a,i] * B[b,j],
	 * The shape of the operation is [1,2], [1,3], [2,4]
	 * For a super instruction "execute usersuperinst A[i,j] B[q,f] C[j,i]
	 * The shape of the operation is [1,2], [3,4], [2,1]
	 */
	struct Key {
		const opcode_t opcode_;
		std::vector<BlockInfo> blocks_;
		Key(opcode_t, const std::vector<BlockInfo>&);
		bool operator<(const Key& rhs) const;
		friend std::ostream& operator<<(std::ostream&, const ProfileTimer::Key&);
	};

	typedef std::map<ProfileTimer::Key, int> TimerMap_t;

	void start_timer(const ProfileTimer::Key& key);
	void pause_timer(const ProfileTimer::Key& key);

	void print_timers();

private:

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
	const int sialx_lines_;

	DISALLOW_COPY_AND_ASSIGN(ProfileTimer);
};

} /* namespace sip */

#endif /* PROFILE_TIMER_H_ */
