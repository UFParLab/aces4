/*
 * data_distribution.h
 *
 *  Created on: Jan 16, 2014
 *      Author: njindal
 */

#ifndef DATA_DISTRIBUTION_H_
#define DATA_DISTRIBUTION_H_

#include <list>

#include "block_id.h"
#include "sip_tables.h"
#include "sip_mpi_attr.h"
#include "sip_tables.h"

namespace sip {

/**
 * Decides distribution of block (statically)
 */
class DataDistribution {
public:
	DataDistribution(SipTables&, SIPMPIAttr&);

	/**
	 * Calculates and returns MPI rank of server that "owns" a given block.
	 * @param
	 * @return
	 */
	int get_server_rank(const sip::BlockId&) const;
	int block_cyclic_distribution_server_rank(const sip::BlockId& bid) const;
	int hashed_indices_based_server_rank(const sip::BlockId& bid) const;


	/** Generates a list of all blocks for a given array
	 * @param [in] global_server_rank
	 * @param [in] array_id
	 * @param [out] all_blocks
	 * @param [in] sip_tables
	 */
	void generate_server_blocks_list(int global_server_rank, int array_id,
											std::list<BlockId>& all_blocks,
											const SipTables& sip_tables) const;

private:

	SipTables& sip_tables_;
	SIPMPIAttr& sip_mpi_attr_;

	long block_position_in_array(const sip::BlockId& bid) const;
	void validate_block_position(const sip::BlockId& bid, long block_num) const;
	int server_rank_from_hash(std::size_t hash) const;


	bool increment_indices(int rank, index_value_array_t& upper,
			index_value_array_t& lower, index_value_array_t& current) const;

};

} /* namespace sip */

#endif /* DATA_DISTRIBUTION_H_ */
