/*
 * memory_tracker.h
 *
 *  Created on: Jan 27, 2016
 *      Author: basbas
 */

#ifndef MEMORY_TRACKER_H_
#define MEMORY_TRACKER_H_

#include <cstddef>
#include <iostream>

namespace sip {


class MemoryTracker {
public:
	MemoryTracker():allocated_bytes_(0) {
	}

	~MemoryTracker();

	static MemoryTracker* global;  //the globally accessible object.

	static void set_global_memory_tracker(MemoryTracker* memory_tracker){
		if (global != NULL) delete global;
		global = memory_tracker;
	}

	void inc_allocated(std::size_t size){
		allocated_bytes_ += size*sizeof(double);
	}

	void dec_allocated(std::size_t size){
		allocated_bytes_ -= size*sizeof(double);
	}

	void reset(){
		allocated_bytes_=0;
	}

	std::size_t get_allocated_bytes(){
		return allocated_bytes_;
	}

	friend std::ostream& operator <<(std::ostream&, const MemoryTracker&);
private:
	std::size_t allocated_bytes_;

	friend class CachedBlockMap;
};

} /* namespace sip */

#endif /* MEMORY_TRACKER_H_ */
