/*
 * server_block.h
 *
 *  Created on: Mar 25, 2014
 *      Author: njindal
 */

#ifndef SERVER_BLOCK_H_
#define SERVER_BLOCK_H_

#include <bitset>
#include <new>
#include "id_block_map.h"

namespace sip {

class SIPServer;
class DiskBackedArraysIO;
class DiskBackedBlockMap;

class ServerBlock {
public:
	typedef double * dataPtr;
	typedef ServerBlock* ServerBlockPtr;

	~ServerBlock();

	void set_dirty() { is_dirty_ = true; }
	void set_clean() { is_dirty_ = false; }
	bool is_dirty() { return is_dirty_; }

	dataPtr get_data() { return data_; }
	void set_data(dataPtr data) { data_ = data; }

	int size() { return size_; }

    dataPtr accumulate_data(size_t size, dataPtr to_add);


	static bool limit_reached();

	friend std::ostream& operator<< (std::ostream& os, const ServerBlock& block);

private:

	explicit ServerBlock(int size, bool initialize=true);
	explicit ServerBlock(int size, dataPtr data);

    const int size_;/**< Number of elements in block */
	dataPtr data_;	/**< Pointer to block of data */
	bool is_dirty_; /**< Is the block more up to date than on disk */

	const static std::size_t field_members_size_;

	const static std::size_t max_allocated_bytes_;
	static std::size_t allocated_bytes_;

	friend SIPServer;
	friend DiskBackedArraysIO;
	friend DiskBackedBlockMap;
	friend IdBlockMap<ServerBlock>;
};

} /* namespace sip */

#endif /* SERVER_BLOCK_H_ */
