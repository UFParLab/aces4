/*
 * lru_array_policy.h
 *
 *  Created on: May 15, 2014
 *      Author: njindal
 */

#ifndef LRU_ARRAY_POLICY_H_
#define LRU_ARRAY_POLICY_H_

#include "id_block_map.h"
#include "server_block.h"


#include <list>

namespace sip {

class BlockId;

/**
 * Policy for block replacement on servers.
 * This policy maintains an LRU data structure on least recently used arrays.
 * From the array, an arbitrary block is offered up for removal.
 */
class LRUArrayPolicy {
public:
	LRUArrayPolicy(IdBlockMap<ServerBlock>&);
    
    /*! Mark a block as recently used  */
	void touch(const BlockId& bid);

    /*! Get the next block for removal */
	BlockId get_next_block_for_removal();

private:
	std::list<int> lru_list_; /*! List of least recently ysed arrays  */
    IdBlockMap<ServerBlock>& block_map_;

};

} /* namespace sip */

#endif /* LRU_ARRAY_POLICY_H_ */
