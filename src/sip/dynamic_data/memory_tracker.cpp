/*
 * memory_tracker.cpp
 *
 *  Created on: Jan 27, 2016
 *      Author: basbas
 */

#include "memory_tracker.h"

namespace sip {

MemoryTracker* MemoryTracker::global = NULL;

MemoryTracker::~MemoryTracker(){

}


std::ostream& operator <<(std::ostream& os, const MemoryTracker& obj){
	os << "*************************************" << std::endl;
    os << "allocated_bytes_: " << obj.allocated_bytes_ << std::endl;
    os << "**************************************" << std::endl;
    return os;
}
} /* namespace sip */
