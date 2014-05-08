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


int DataDistribution::get_server_rank(const sip::BlockId& bid) const{
	int array_id = bid.array_id();
	int array_rank = sip_tables_.array_rank(array_id);

//	// Calculate total number of blocks
//	int num_blocks = 1;
//	for (int pos=0; pos<array_rank; pos++){
//		int index_slot = sip_tables_.selectors(array_id)[pos];
//		int num_segments = sip_tables_.num_segments(index_slot);
//		num_blocks *= num_segments;
//	}

	//int num_blocks = sip_tables_.num_block_in_array(array_id);

    std::stringstream ss;
    ss << " Array id is -ve " << bid;
    sip::check(array_id >= 0, ss.str(), current_line());
//	int num_blocks = sip_tables_.num_block_in_array(array_id);

//    std::stringstream ss1;
//    ss1 << " Number of blocks is -ve " << num_blocks;
//    sip::check(num_blocks >= 0, ss.str(), current_line());

	// Convert rank-dimensional index to 1-dimensional index
	long block_num = 0;
	long tmp = 1;
	for (int pos=array_rank-1; pos>=0; pos--){
		int index_slot = sip_tables_.selectors(array_id)[pos];
		int num_segments = sip_tables_.num_segments(index_slot);
        sip::check (num_segments >= 0, "num_segments is -ve", current_line());
		block_num += bid.index_values(pos) * tmp;
		tmp *= num_segments;
	}

	// Cyclic distribution
	int num_servers = sip_mpi_attr_.num_servers();
	int server_rank = block_num % num_servers;

	const std::vector<int> &server_ranks = sip_mpi_attr_.server_ranks();

    std::stringstream ss1;
    ss1 << " Block num is -ve : "<< block_num << " Block Id : " << bid;
    if (block_num < 0){
        for (int pos=array_rank-1; pos>=0; pos--){
            int index_slot = sip_tables_.selectors(array_id)[pos];
            int num_segments = sip_tables_.num_segments(index_slot);
            std :: cerr << " index_slot : " << index_slot 
                        << " pos : " << pos 
                        << " nseg : " << num_segments 
                        << std::endl;
	}
    sip::check(block_num >= 0, ss1.str(), current_line());

    }

    std::stringstream ss2;
    ss2 << " Server rank is -ve : "<< server_rank;
    sip::check(server_rank >= 0, ss2.str(), current_line());

    return server_ranks.at(server_rank);

}



} /* namespace sip */
