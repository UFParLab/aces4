/*
 * chunk.cpp
 *
 *  Created on: Jul 21, 2015
 *      Author: basbas
 */

#include "chunk_manager.h"
#include "server_block.h"

namespace sip {

void Chunk::wait_all(){
	for (std::vector<ServerBlock*>::iterator it = blocks_.begin();
			it != blocks_.end(); ++it){
		(*it)->wait();
	}
}

std::ostream& operator<<(std::ostream& os, const Chunk& obj){
	os << "data_: " << obj.data_;
	os << ", file_offset_: " << obj.file_offset_;
	os << " num_assigned_doubles_: "<< obj.num_assigned_doubles_;
	os << " valid_on_disk_: " << obj. valid_on_disk_;
	os << std::endl;
	return os;
}



} /* namespace sip */
