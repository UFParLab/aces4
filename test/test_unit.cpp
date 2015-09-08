/*
 * test_unit.cpp
 *
 *  Created on: May 27, 2014
 *      Author: njindal
 */

#include <cstdio>
#include <algorithm>

#include "mpi.h"
#include "gtest/gtest.h"

#include "id_block_map.h"
#include "server_block.h"
#include "block_id.h"
#include "block.h"
#include "lru_array_policy.h"
#include "sip_mpi_utils.h"
#include "sip_mpi_attr.h"

TEST(Sial_Unit,BlockLRUArrayPolicy){

	sip::index_value_array_t index_values;
	std::fill(index_values, index_values+MAX_RANK, sip::unused_index_value);

	sip::BlockId bid0(0, index_values);
	sip::BlockId bid1(1, index_values);
	sip::BlockId bid2(2, index_values);
	sip::BlockId bid3(3, index_values);
	sip::BlockId bid4(4, index_values);
	sip::BlockId bid5(5, index_values);

	sip::Block *blk = NULL;

	sip::IdBlockMap<sip::Block> block_map(6);
	block_map.insert_block(bid0, blk);
	block_map.insert_block(bid1, blk);
	block_map.insert_block(bid2, blk);
	block_map.insert_block(bid3, blk);
	block_map.insert_block(bid4, blk);
	block_map.insert_block(bid5, blk);

	sip::LRUArrayPolicy<sip::Block> policy(block_map);
	policy.touch(bid0);
	policy.touch(bid1);
	policy.touch(bid2);
	policy.touch(bid3);
	policy.touch(bid4);
	policy.touch(bid5);

	sip::Block* b;
	sip::check(policy.get_next_block_for_removal(b).array_id() == 0, "Block to remove should be from array 0");
	block_map.get_and_remove_block(bid0);
	sip::check(policy.get_next_block_for_removal(b).array_id() == 1, "Block to remove should be from array 1");
	block_map.get_and_remove_block(bid1);
	sip::check(policy.get_next_block_for_removal(b).array_id() == 2, "Block to remove should be from array 2");
	block_map.get_and_remove_block(bid2);
	sip::check(policy.get_next_block_for_removal(b).array_id() == 3, "Block to remove should be from array 3");
	block_map.get_and_remove_block(bid3);
	sip::check(policy.get_next_block_for_removal(b).array_id() == 4, "Block to remove should be from array 4");
	block_map.get_and_remove_block(bid4);
	sip::check(policy.get_next_block_for_removal(b).array_id() == 5, "Block to remove should be from array 5");
	block_map.get_and_remove_block(bid5);

}


TEST(Sial_Unit,ServerBlockLRUArrayPolicy){

	sip::index_value_array_t index_values;
	std::fill(index_values, index_values+MAX_RANK, sip::unused_index_value);

	sip::BlockId bid0(0, index_values);
	sip::BlockId bid1(1, index_values);
	sip::BlockId bid2(2, index_values);
	sip::BlockId bid3(3, index_values);
	sip::BlockId bid4(4, index_values);
	sip::BlockId bid5(5, index_values);

	// Allocate dummy data for ServerBlock so that ServerBlock
	// Specific LRUArrayPolicy processes it correctly.
	sip::ServerBlock *sb = new sip::ServerBlock(1, true);

	sip::IdBlockMap<sip::ServerBlock> server_block_map(6);
	server_block_map.insert_block(bid0, sb);
	server_block_map.insert_block(bid1, sb);
	server_block_map.insert_block(bid2, sb);
	server_block_map.insert_block(bid3, sb);
	server_block_map.insert_block(bid4, sb);
	server_block_map.insert_block(bid5, sb);

	sip::LRUArrayPolicy<sip::ServerBlock> policy(server_block_map);
	policy.touch(bid0);
	policy.touch(bid1);
	policy.touch(bid2);
	policy.touch(bid3);
	policy.touch(bid4);
	policy.touch(bid5);

	sip::ServerBlock* b;

	sip::check(policy.get_next_block_for_removal(b).array_id() == 0, "Block to remove should be from array 0");
	server_block_map.get_and_remove_block(bid0);
	sip::check(policy.get_next_block_for_removal(b).array_id() == 1, "Block to remove should be from array 1");
	server_block_map.get_and_remove_block(bid1);
	sip::check(policy.get_next_block_for_removal(b).array_id() == 2, "Block to remove should be from array 2");
	server_block_map.get_and_remove_block(bid2);
	sip::check(policy.get_next_block_for_removal(b).array_id() == 3, "Block to remove should be from array 3");
	server_block_map.get_and_remove_block(bid3);
	sip::check(policy.get_next_block_for_removal(b).array_id() == 4, "Block to remove should be from array 4");
	server_block_map.get_and_remove_block(bid4);
	sip::check(policy.get_next_block_for_removal(b).array_id() == 5, "Block to remove should be from array 5");
	server_block_map.get_and_remove_block(bid5);

}

int main(int argc, char **argv) {

	MPI_Init(&argc, &argv);

	sip::check(sizeof(int) >= 4, "Size of integer should be 4 bytes or more");
	sip::check(sizeof(double) >= 8, "Size of double should be 8 bytes or more");
	sip::check(sizeof(long long) >= 8, "Size of long long should be 8 bytes or more");

	int num_procs;
	sip::SIPMPIUtils::check_err(MPI_Comm_size(MPI_COMM_WORLD, &num_procs));

	if (num_procs < 2){
		std::cerr<<"Please run this test with at least 2 mpi ranks"<<std::endl;
		return -1;
	}

	sip::SIPMPIUtils::set_error_handler();
	sip::SIPMPIAttr &sip_mpi_attr = sip::SIPMPIAttr::get_instance();


	printf("Running main() from master_test_main.cpp\n");
	testing::InitGoogleTest(&argc, argv);
	int result = RUN_ALL_TESTS();

	MPI_Finalize();

	return result;

}
