/*
 * data_distribution.cpp
 *
 *  Created on: Jan 16, 2014
 *      Author: njindal
 */

#include <data_distribution.h>
#include <sstream>

namespace sip {

DataDistribution::DataDistribution(const SipTables& sip_tables, SIPMPIAttr& sip_mpi_attr):
		sip_tables_(sip_tables), sip_mpi_attr_(sip_mpi_attr) {
}

int DataDistribution::block_cyclic_distribution_server_rank(
		const sip::BlockId& bid) const {
	// Convert rank-dimensional index to 1-dimensional index
	size_t block_position = sip_tables_.block_number(bid);
	BlockId id2 = sip_tables_.block_id(bid.array_id(),block_position);
	check(bid==id2, "problem computing block position in block_cyclic_distribution_server_rank");
	// Cyclic distribution
	int server_global_rank = server_rank_from_hash(block_position);
	return server_global_rank;
}

int DataDistribution::hashed_indices_based_server_rank(
		const sip::BlockId& bid) const {

	std::size_t seed = 0;
	const index_value_array_t& indices = bid.index_values_;
	const int array_rank = sip_tables_.array_rank(bid);

	// hash calculation based on boost hash_combine
	for (int i = 0; i < array_rank; i++) {
		seed ^= indices[i] + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	}

	int server_global_rank = server_rank_from_hash(seed);
	return server_global_rank;
}

int DataDistribution::get_server_rank(const sip::BlockId& bid) const{
// TODO
	int server_global_rank = block_cyclic_distribution_server_rank(bid);
	//int server_global_rank = hashed_indices_based_server_rank(bid);

    return server_global_rank;

}


bool DataDistribution::is_my_block(size_t block_number) const{
	return server_rank_from_hash(block_number) == sip_mpi_attr_.global_rank();

}


int DataDistribution::server_rank_from_hash(std::size_t hash) const {
	// Cyclic distribution
	int num_servers = sip_mpi_attr_.num_servers();
	int server_rank_slot = hash % num_servers;
	const std::vector<int>& server_ranks = sip_mpi_attr_.server_ranks();

	int server_global_rank = server_ranks.at(server_rank_slot);
	return server_global_rank;
}


} /* namespace sip */
