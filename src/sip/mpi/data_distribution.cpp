/*
 * data_distribution.cpp
 *
 *  Created on: Jan 16, 2014
 *      Author: njindal
 */

#include <data_distribution.h>
#include <sstream>

namespace sip {

DataDistribution::DataDistribution(SipTables& sip_tables, SIPMPIAttr& sip_mpi_attr):
		sip_tables_(sip_tables), sip_mpi_attr_(sip_mpi_attr) {
}

DataDistribution::~DataDistribution() {}

int DataDistribution::block_cyclic_distribution_server_rank(
		const sip::BlockId& bid) const {
	// Convert rank-dimensional index to 1-dimensional index
	long block_position = block_position_in_array(bid);
	validate_block_position(bid, block_position);
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

	//int server_global_rank = block_cyclic_distribution_server_rank(bid);
	int server_global_rank = hashed_indices_based_server_rank(bid);

    return server_global_rank;

}



void DataDistribution::validate_block_position(const sip::BlockId& bid,
		long block_num) const {
	int array_id = bid.array_id();
	int array_rank = sip_tables_.array_rank(array_id);
	std::stringstream ss1;
	ss1 << " Block num is -ve : " << block_num << " Block Id : " << bid;
	if (block_num < 0) {
		for (int pos = array_rank - 1; pos >= 0; pos--) {
			int index_slot = sip_tables_.selectors(array_id)[pos];
			int num_segments = sip_tables_.num_segments(index_slot);
			std::cerr << " index_slot : " << index_slot << " pos : " << pos
					<< " nseg : " << num_segments << std::endl;
		}
		sip::check(block_num >= 0, ss1.str(), current_line());
	}
}

int DataDistribution::server_rank_from_hash(std::size_t hash) const {
	// Cyclic distribution
	int num_servers = sip_mpi_attr_.num_servers();
	int server_rank_slot = hash % num_servers;
	const std::vector<int>& server_ranks = sip_mpi_attr_.server_ranks();

	// Validation
	{
		std::stringstream ss2;
		ss2 << " Server rank slot is -ve : " << server_rank_slot;
		sip::check(server_rank_slot >= 0, ss2.str(), current_line());
	}

	int server_global_rank = server_ranks.at(server_rank_slot);
	return server_global_rank;
}

long DataDistribution::block_position_in_array(const sip::BlockId& bid) const {
	int array_rank = sip_tables_.array_rank(bid);
	int array_id = bid.array_id();
	// Convert rank-dimensional index to 1-dimensional index
	long block_num = 0;
	long tmp = 1;
	for (int pos = array_rank - 1; pos >= 0; pos--) {
		int index_slot = sip_tables_.selectors(array_id)[pos];
		int num_segments = sip_tables_.num_segments(index_slot);
		sip::check(num_segments >= 0, "num_segments is -ve", current_line());
		block_num += bid.index_values(pos) * tmp;
		tmp *= num_segments;
	}
	return block_num;
}

} /* namespace sip */
