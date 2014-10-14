/*
 * sipmap_interpreter.cpp
 *
 *  Created on: Oct 9, 2014
 *      Author: njindal
 */

#include "sipmap_interpreter.h"
#include "profile_timer_store.h"
#include "sipmap_timer.h"

namespace sip {

SIPMaPInterpreter::SIPMaPInterpreter(const SipTables& sipTables, const ProfileTimerStore& profile_timer_store, SIPMaPTimer& sipmap_timer):
	SialxInterpreter(sipTables, NULL, NULL, NULL), profile_timer_store_(profile_timer_store), sipmap_timer_(sipmap_timer){
}

SIPMaPInterpreter::~SIPMaPInterpreter() {}


inline ProfileTimer::Key SIPMaPInterpreter::make_profile_timer_key(const std::string& opcode_name, const std::list<BlockSelector>& selector_list){
	std::vector<ProfileTimer::BlockInfo> block_infos;
	block_infos.reserve(selector_list.size());

	std::list<BlockSelector>::const_iterator it = selector_list.begin();
	for (; it != selector_list.end(); ++it){
		const BlockSelector &bsel = *it;
		BlockShape bs;
		int rank = bsel.rank_;
		if (sip_tables_.array_rank(bsel.array_id_) == 0) { //this "array" was declared to be a scalar.  Nothing to remove from selector stack.)
			continue;
		}
		else if (sip_tables_.is_contiguous(bsel.array_id_) && bsel.rank_ == 0) { //this is a static array provided without a selector, block is entire array
			Block::BlockPtr block = data_manager_.contiguous_array_manager_.get_array(bsel.array_id_);
			bs = block->shape();
			// Modify rank so that the correct segments & indices are stored
			// in the profile timer.
			rank = sip_tables_.array_rank(bsel.array_id_);
		} else {
			BlockId bid = get_block_id_from_selector(bsel);
			bs = sip_tables_.shape(bid);
		}
		ProfileTimer::BlockInfo bi (rank, bsel.index_ids_, bs.segment_sizes_);
		block_infos.push_back(bi);
	}
	ProfileTimer::Key key(opcode_name, block_infos);
	return key;
}

inline ProfileTimer::Key SIPMaPInterpreter::make_profile_timer_key(opcode_t opcode, const std::list<BlockSelector>& selector_list){
	return make_profile_timer_key(opcodeToName(opcode), selector_list);
}


} /* namespace sip */
