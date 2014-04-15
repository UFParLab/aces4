/*
 * server_block.h
 *
 *  Created on: Apr 7, 2014
 *      Author: basbas
 */

#ifndef SERVER_BLOCK_H_
#define SERVER_BLOCK_H_

#include "sip.h"

namespace sip {

/** This is a container for block data held by a server
 *
 */
class ServerBlock {
public:
	typedef double * dataPtr;
	typedef size_t size_type;

	ServerBlock():
		size_(0), data_(NULL){
	}

	ServerBlock(size_type size, bool initialize = true):
			size_(size){
		if (initialize) data_ = new double[size]();
		else data_ = new double[size];
	}

	ServerBlock(size_type size, dataPtr data):
				size_(size),
				data_(data){
	}

	~ServerBlock(){
		if (data_ != NULL) delete [] data_;
	}

	size_t size(){return size_;}
	dataPtr get_data(){return data_;}

    dataPtr accumulate_data(size_t size, dataPtr to_add){
    	check(size_ == size, "accumulating blocks of unequal size");
    	for (unsigned i = 0; i < size; ++i){
    		data_[i] += to_add[i];
    	}
    	return data_;
    }

    friend std::ostream& operator<<(std::ostream&, const ServerBlock &);

private:
	size_t size_;
	dataPtr data_;

	DISALLOW_COPY_AND_ASSIGN(ServerBlock);
};



} /* namespace sip */

#endif /* SERVER_BLOCK_H_ */
