/*
 * test_unit.cpp
 *
 *  Created on: May 27, 2014
 *      Author: njindal
 */

#include <cstdio>
#include <algorithm>
#include <vector>
#include <stdexcept>
#include <map>
#include <istream>
#include "config.h"

#include "gtest/gtest.h"

#include "array_constants.h"
#include "id_block_map.h"
#include "block_id.h"
#include "block.h"
#include "lru_array_policy.h"
#include "sip_mpi_attr.h"
#include "cached_block_map.h"
#include "global_state.h"
#include "rank_distribution.h"


#ifdef HAVE_MPI
#include "mpi.h"
#include "server_block.h"
#include "sip_mpi_utils.h"
#endif

#ifdef HAVE_TAU
#include <TAU.h>
#endif

TEST(SialUnitLRU,BlockLRUArrayPolicy){

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

	sip::check(policy.get_next_block_for_removal(blk).array_id() == 0, "Block to remove should be from array 0");
	block_map.get_and_remove_block(bid0);
	sip::check(policy.get_next_block_for_removal(blk).array_id() == 1, "Block to remove should be from array 1");
	block_map.get_and_remove_block(bid1);
	sip::check(policy.get_next_block_for_removal(blk).array_id() == 2, "Block to remove should be from array 2");
	block_map.get_and_remove_block(bid2);
	sip::check(policy.get_next_block_for_removal(blk).array_id() == 3, "Block to remove should be from array 3");
	block_map.get_and_remove_block(bid3);
	sip::check(policy.get_next_block_for_removal(blk).array_id() == 4, "Block to remove should be from array 4");
	block_map.get_and_remove_block(bid4);
	sip::check(policy.get_next_block_for_removal(blk).array_id() == 5, "Block to remove should be from array 5");
	block_map.get_and_remove_block(bid5);

}

#ifdef HAVE_MPI // ServerBlocks are valid only in the MPI Build
TEST(SialUnitLRU,ServerBlockLRUArrayPolicy){

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
	sip::ServerBlock *sb = new sip::ServerBlock(1, false);

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

	sip::check(policy.get_next_block_for_removal(sb).array_id() == 0, "Block to remove should be from array 0");
	server_block_map.get_and_remove_block(bid0);
	sip::check(policy.get_next_block_for_removal(sb).array_id() == 1, "Block to remove should be from array 1");
	server_block_map.get_and_remove_block(bid1);
	sip::check(policy.get_next_block_for_removal(sb).array_id() == 2, "Block to remove should be from array 2");
	server_block_map.get_and_remove_block(bid2);
	sip::check(policy.get_next_block_for_removal(sb).array_id() == 3, "Block to remove should be from array 3");
	server_block_map.get_and_remove_block(bid3);
	sip::check(policy.get_next_block_for_removal(sb).array_id() == 4, "Block to remove should be from array 4");
	server_block_map.get_and_remove_block(bid4);
	sip::check(policy.get_next_block_for_removal(sb).array_id() == 5, "Block to remove should be from array 5");
	server_block_map.get_and_remove_block(bid5);

}
#endif // HAVE_MPI



// Tests for CachedBlockMap

// Sanity test
TEST(CachedBlockMap, only_insert){
	const int size_in_mb = 10;
	const std::size_t size_in_bytes = size_in_mb * 1024 * 1024;
	sip::CachedBlockMap cached_block_map(1);
	cached_block_map.set_max_allocatable_bytes(size_in_bytes);

	const int num_segments = 5;
	const int rank = 2;
	const int segment_size = 70;

	// Should not crash
	for (int i=1; i <= num_segments; ++i){
		for (int j=1; j <= num_segments; ++j){
			sip::index_value_array_t indices;
			indices[0] = i;
			indices[1] = j;
			for (int f=rank; f < MAX_RANK; f++) indices[f] = sip::unused_index_value;
			sip::BlockId block_id(0, indices);
			sip::segment_size_array_t segment_sizes;
			for (int s=0; s < rank; s++) segment_sizes[s] = segment_size;
			for (int s=rank; s<MAX_RANK; s++) segment_sizes[s] = sip::unused_index_segment_size;
			sip::BlockShape shape(segment_sizes, rank);
			sip::Block::BlockPtr block = new sip::Block(shape);
			cached_block_map.insert_block(block_id, block);
		}
	}
}


// Sanity test
TEST(CachedBlockMap, insert_more_than_2gb){
	const int size_in_gb = 3;
	const std::size_t size_in_bytes = size_in_gb * 1024L * 1024L * 1024L;
	sip::CachedBlockMap cached_block_map(1);
	cached_block_map.set_max_allocatable_bytes(size_in_bytes);

	const int num_segments = 4;
	const int rank = 4;
	const int segment_size = 35;

	// Should not crash
	for (int i=1; i <= num_segments; ++i){
		for (int j=1; j <= num_segments; ++j){
			for (int k=1; k <= num_segments; ++k){
				for (int l=1; l <= num_segments; ++l){
					sip::index_value_array_t indices;
					indices[0] = i;
					indices[1] = j;
					indices[2] = k;
					indices[3] = l;
					for (int f=rank; f < MAX_RANK; f++) indices[f] = sip::unused_index_value;
					sip::BlockId block_id(0, indices);
					sip::segment_size_array_t segment_sizes;
					for (int s=0; s < rank; s++) segment_sizes[s] = segment_size;
					for (int s=rank; s<MAX_RANK; s++) segment_sizes[s] = sip::unused_index_segment_size;
					sip::BlockShape shape(segment_sizes, rank);
					sip::Block::BlockPtr block = new sip::Block(shape);
					cached_block_map.insert_block(block_id, block);
				}
			}
		}
	}
}

// Max mem set to a low number so
// as to trigger a out_of_range exception
TEST(CachedBlockMap, exceed_insert){
	const int size_in_mb = 0.15;
	const std::size_t size_in_bytes = size_in_mb * 1024 * 1024;
	sip::CachedBlockMap cached_block_map(1);
	cached_block_map.set_max_allocatable_bytes(size_in_bytes);

	const int num_segments = 5;
	const int rank = 2;
	const int segment_size = 100;

	// Should throw exception
	int count = 0;
	for (int i=1; i <= num_segments; ++i){
		for (int j=1; j <= num_segments; ++j){
			sip::index_value_array_t indices;
			indices[0] = i;
			indices[1] = j;
			for (int f=rank; f < MAX_RANK; f++) indices[f] = sip::unused_index_value;
			sip::BlockId block_id(0, indices);
			sip::segment_size_array_t segment_sizes;
			for (int s=0; s < rank; s++) segment_sizes[s] = segment_size;
			for (int s=rank; s<MAX_RANK; s++) segment_sizes[s] = sip::unused_index_segment_size;
			sip::BlockShape shape(segment_sizes, rank);
			sip::Block::BlockPtr block = new sip::Block(shape);
			ASSERT_THROW(cached_block_map.insert_block(block_id, block), std::out_of_range);
			// Even if the exception is thrown at least once, this test passes.
		}
	}
}

// Checking if cached_delete-ed blocks
// are available again.
TEST(CachedBlockMap, cached_delete){
	const int size_in_mb = 10;
	const std::size_t size_in_bytes = size_in_mb * 1024 * 1024;
	sip::CachedBlockMap cached_block_map(1);
	cached_block_map.set_max_allocatable_bytes(size_in_bytes);

	const int num_segments = 5;
	const int rank = 2;
	const int segment_size = 70;

	// Should not crash
	for (int i=1; i <= num_segments; ++i){
		for (int j=1; j <= num_segments; ++j){
			sip::index_value_array_t indices;
			indices[0] = i;
			indices[1] = j;
			for (int f=rank; f < MAX_RANK; f++) indices[f] = sip::unused_index_value;
			sip::BlockId block_id(0, indices);
			sip::segment_size_array_t segment_sizes;
			for (int s=0; s < rank; s++) segment_sizes[s] = segment_size;
			for (int s=rank; s<MAX_RANK; s++) segment_sizes[s] = sip::unused_index_segment_size;
			sip::BlockShape shape(segment_sizes, rank);
			sip::Block::BlockPtr block = new sip::Block(shape);
			cached_block_map.insert_block(block_id, block);
		}
	}

	// Cached Delete the blocks
	for (int i=1; i <= num_segments; ++i){
		for (int j=1; j <= num_segments; ++j){
			sip::index_value_array_t indices;
			indices[0] = i;
			indices[1] = j;
			for (int f=rank; f < MAX_RANK; f++) indices[f] = sip::unused_index_value;
			sip::BlockId block_id(0, indices);
			cached_block_map.cached_delete_block(block_id);
		}
	}

	// Check if deleted blocks are available
	for (int i=1; i <= num_segments; ++i){
		for (int j=1; j <= num_segments; ++j){
			sip::index_value_array_t indices;
			indices[0] = i;
			indices[1] = j;
			for (int f=rank; f < MAX_RANK; f++) indices[f] = sip::unused_index_value;
			sip::BlockId block_id(0, indices);
			ASSERT_TRUE(cached_block_map.block(block_id) != NULL);
		}
	}

}

// deleted blocks should not be
// available again.
TEST(CachedBlockMap, regular_delete){
	const int size_in_mb = 10;
	const std::size_t size_in_bytes = size_in_mb * 1024 * 1024;
	sip::CachedBlockMap cached_block_map(1);
	cached_block_map.set_max_allocatable_bytes(size_in_bytes);

	const int num_segments = 5;
	const int rank = 2;
	const int segment_size = 70;

	// Should not crash
	for (int i=1; i <= num_segments; ++i){
		for (int j=1; j <= num_segments; ++j){
			sip::index_value_array_t indices;
			indices[0] = i;
			indices[1] = j;
			for (int f=rank; f < MAX_RANK; f++) indices[f] = sip::unused_index_value;
			sip::BlockId block_id(0, indices);
			sip::segment_size_array_t segment_sizes;
			for (int s=0; s < rank; s++) segment_sizes[s] = segment_size;
			for (int s=rank; s<MAX_RANK; s++) segment_sizes[s] = sip::unused_index_segment_size;
			sip::BlockShape shape(segment_sizes, rank);
			sip::Block::BlockPtr block = new sip::Block(shape);
			cached_block_map.insert_block(block_id, block);
		}
	}

	// Delete the blocks
	for (int i=1; i <= num_segments; ++i){
		for (int j=1; j <= num_segments; ++j){
			sip::index_value_array_t indices;
			indices[0] = i;
			indices[1] = j;
			for (int f=rank; f < MAX_RANK; f++) indices[f] = sip::unused_index_value;
			sip::BlockId block_id(0, indices);
			cached_block_map.delete_block(block_id);
		}
	}

	// Check if deleted blocks are available
	for (int i=1; i <= num_segments; ++i){
		for (int j=1; j <= num_segments; ++j){
			sip::index_value_array_t indices;
			indices[0] = i;
			indices[1] = j;
			for (int f=rank; f < MAX_RANK; f++) indices[f] = sip::unused_index_value;
			sip::BlockId block_id(0, indices);
			ASSERT_TRUE(cached_block_map.block(block_id) == NULL);
		}
	}
}


// Seeing if free space after block inserts & deletes
TEST(CachedBlockMap, insert_after_freeing_up_space){
	const int size_in_mb = 8;
	const std::size_t size_in_bytes = size_in_mb * 1024 * 1024;
	sip::CachedBlockMap cached_block_map(1);
	cached_block_map.set_max_allocatable_bytes(size_in_bytes);

	const int num_segments = 10;
	const int rank = 2;
	const int segment_size = 100;

	// Should not crash
	for (int i=1; i <= num_segments; ++i){
		for (int j=1; j <= num_segments; ++j){
			sip::index_value_array_t indices;
			indices[0] = i;
			indices[1] = j;
			for (int f=rank; f < MAX_RANK; f++) indices[f] = sip::unused_index_value;
			sip::BlockId block_id(0, indices);
			sip::segment_size_array_t segment_sizes;
			for (int s=0; s < rank; s++) segment_sizes[s] = segment_size;
			for (int s=rank; s<MAX_RANK; s++) segment_sizes[s] = sip::unused_index_segment_size;
			sip::BlockShape shape(segment_sizes, rank);
			sip::Block::BlockPtr block = new sip::Block(shape);
			cached_block_map.insert_block(block_id, block);
		}
	}

	// Delete the blocks
	for (int i=1; i <= num_segments; ++i){
		for (int j=1; j <= num_segments; ++j){
			sip::index_value_array_t indices;
			indices[0] = i;
			indices[1] = j;
			for (int f=rank; f < MAX_RANK; f++) indices[f] = sip::unused_index_value;
			sip::BlockId block_id(0, indices);
			cached_block_map.delete_block(block_id);
		}
	}

	// Insert blocks again
	for (int i=1; i <= num_segments; ++i){
		for (int j=1; j <= num_segments; ++j){
			sip::index_value_array_t indices;
			indices[0] = i;
			indices[1] = j;
			for (int f=rank; f < MAX_RANK; f++) indices[f] = sip::unused_index_value;
			sip::BlockId block_id(0, indices);
			sip::segment_size_array_t segment_sizes;
			for (int s=0; s < rank; s++) segment_sizes[s] = segment_size;
			for (int s=rank; s<MAX_RANK; s++) segment_sizes[s] = sip::unused_index_segment_size;
			sip::BlockShape shape(segment_sizes, rank);
			sip::Block::BlockPtr block = new sip::Block(shape);
			cached_block_map.insert_block(block_id, block);
		}
	}
}

// Seeing if free space after block inserts & cached deletes
TEST(CachedBlockMap, insert_after_cached_delete){
	const int size_in_mb = 8;
	const std::size_t size_in_bytes = size_in_mb * 1024 * 1024;
	sip::CachedBlockMap cached_block_map(1);
	cached_block_map.set_max_allocatable_bytes(size_in_bytes);

	const int num_segments = 10;
	const int rank = 2;
	const int segment_size = 100;

	// Should not crash
	for (int i=1; i <= num_segments; ++i){
		for (int j=1; j <= num_segments; ++j){
			sip::index_value_array_t indices;
			indices[0] = i;
			indices[1] = j;
			for (int f=rank; f < MAX_RANK; f++) indices[f] = sip::unused_index_value;
			sip::BlockId block_id(0, indices);
			sip::segment_size_array_t segment_sizes;
			for (int s=0; s < rank; s++) segment_sizes[s] = segment_size;
			for (int s=rank; s<MAX_RANK; s++) segment_sizes[s] = sip::unused_index_segment_size;
			sip::BlockShape shape(segment_sizes, rank);
			sip::Block::BlockPtr block = new sip::Block(shape);
			cached_block_map.insert_block(block_id, block);
		}
	}

	// Delete the blocks
	for (int i=1; i <= num_segments; ++i){
		for (int j=1; j <= num_segments; ++j){
			sip::index_value_array_t indices;
			indices[0] = i;
			indices[1] = j;
			for (int f=rank; f < MAX_RANK; f++) indices[f] = sip::unused_index_value;
			sip::BlockId block_id(0, indices);
			cached_block_map.cached_delete_block(block_id);
		}
	}

	// Insert blocks again
	for (int i=1; i <= num_segments; ++i){
		for (int j=1; j <= num_segments; ++j){
			sip::index_value_array_t indices;
			indices[0] = i;
			indices[1] = j;
			for (int f=rank; f < MAX_RANK; f++) indices[f] = sip::unused_index_value;
			sip::BlockId block_id(0, indices);
			sip::segment_size_array_t segment_sizes;
			for (int s=0; s < rank; s++) segment_sizes[s] = segment_size;
			for (int s=rank; s<MAX_RANK; s++) segment_sizes[s] = sip::unused_index_segment_size;
			sip::BlockShape shape(segment_sizes, rank);
			sip::Block::BlockPtr block = new sip::Block(shape);
			cached_block_map.insert_block(block_id, block);
		}
	}
}


TEST(ConfigurableRankDistribution, test1){
	const int num_processes = 10;
	const int num_workers = 9;
	const int num_servers = 1;
	bool reference_is_server_array[num_processes] = {
			false,	// 0
			false,	// 1
			false,	// 2
			false,	// 3
			false,	// 4
			false,	// 5
			false,	// 6
			false,	// 7
			false,	// 8
			true	// 9
	};
	bool local_worker_to_communicate[num_processes] = {
			true,	// 0
			false,	// 1
			false,	// 2
			false,	// 3
			false,	// 4
			false,	// 5
			false,	// 6
			false,	// 7
			false,	// 8
			false	// 9
	};

	sip::ConfigurableRankDistribution rd (num_workers, num_servers);
	const std::vector<bool> is_server_vector = rd.get_is_server_vector();

	for (int i=0; i<num_processes; ++i){
		ASSERT_EQ(reference_is_server_array[i], is_server_vector[i]);
	}

	for (int i=0; i<num_processes; ++i){
		ASSERT_EQ(local_worker_to_communicate[i], rd.is_local_worker_to_communicate(i) );
	}

	int local_servers_to_communicate[num_processes][num_processes] = { -1 };
	local_servers_to_communicate[0][0] = 9;

	for (int i=0; i<num_processes; ++i){
		if (rd.is_local_worker_to_communicate(i)){
			std::vector<int> servers = rd.local_servers_to_communicate(i);
			ASSERT_EQ(servers.size(), 1);
			for (int j=0; j<servers.size(); ++j){
				ASSERT_EQ(local_servers_to_communicate[i][j], servers[j]);
			}
		}
	}
}

TEST(ConfigurableRankDistribution, test2){
	const int num_processes = 10;
	const int num_workers = 8;
	const int num_servers = 2;
	bool reference_is_server_array[num_processes] = {
			false,	// 0
			false,	// 1
			false,	// 2
			false,	// 3
			true,	// 4
			false,	// 5
			false,	// 6
			false,	// 7
			false,	// 8
			true	// 9
	};
	bool local_worker_to_communicate[num_processes] = {
			true,	// 0
			false,	// 1
			false,	// 2
			false,	// 3
			false,	// 4
			true,	// 5
			false,	// 6
			false,	// 7
			false,	// 8
			false	// 9
	};

	sip::ConfigurableRankDistribution rd (num_workers, num_servers);
	const std::vector<bool> is_server_vector = rd.get_is_server_vector();

	for (int i=0; i<num_processes; ++i){
		ASSERT_EQ(reference_is_server_array[i], is_server_vector[i]);
	}

	for (int i=0; i<num_processes; ++i){
		ASSERT_EQ(local_worker_to_communicate[i], rd.is_local_worker_to_communicate(i) );
	}

	int local_servers_to_communicate[num_processes][num_processes] = { -1 };
	local_servers_to_communicate[0][0] = 4;
	local_servers_to_communicate[5][0] = 9;

	for (int i=0; i<num_processes; ++i){
		if (rd.is_local_worker_to_communicate(i)){
			std::vector<int> servers = rd.local_servers_to_communicate(i);
			ASSERT_EQ(servers.size(), 1);
			for (int j=0; j<servers.size(); ++j){
				ASSERT_EQ(local_servers_to_communicate[i][j], servers[j]);
			}
		}
	}
}

TEST(ConfigurableRankDistribution, test3){
	const int num_processes = 10;
	const int num_workers = 7;
	const int num_servers = 3;
	bool reference_is_server_array[num_processes] = {
			false,	// 0
			false,	// 1
			false,	// 2
			true,	// 3
			false,	// 4
			false,	// 5
			true,	// 6
			false,	// 7
			false,	// 8
			true	// 9
	};
	bool local_worker_to_communicate[num_processes] = {
			true,	// 0
			false,	// 1
			false,	// 2
			false,	// 3
			true,	// 4
			false,	// 5
			false,	// 6
			true,	// 7
			false,	// 8
			false	// 9
	};

	sip::ConfigurableRankDistribution rd (num_workers, num_servers);
	const std::vector<bool> is_server_vector = rd.get_is_server_vector();

	for (int i=0; i<num_processes; ++i){
		ASSERT_EQ(reference_is_server_array[i], is_server_vector[i]);
	}

	for (int i=0; i<num_processes; ++i){
		ASSERT_EQ(local_worker_to_communicate[i], rd.is_local_worker_to_communicate(i) );
	}
	int local_servers_to_communicate[num_processes][num_processes] = { -1 };
	local_servers_to_communicate[0][0] = 3;
	local_servers_to_communicate[4][0] = 6;
	local_servers_to_communicate[7][0] = 9;

	for (int i=0; i<num_processes; ++i){
		if (rd.is_local_worker_to_communicate(i)){
			std::vector<int> servers = rd.local_servers_to_communicate(i);
			ASSERT_EQ(servers.size(), 1);
			for (int j=0; j<servers.size(); ++j){
				ASSERT_EQ(local_servers_to_communicate[i][j], servers[j]);
			}
		}
	}
}

TEST(ConfigurableRankDistribution, test4){
	const int num_processes = 10;
	const int num_workers = 6;
	const int num_servers = 4;
	bool reference_is_server_array[num_processes] = {
			false,	// 0
			false,	// 1
			true,	// 2
			false,	// 3
			false,	// 4
			true,	// 5
			false,	// 6
			true,	// 7
			false,	// 8
			true	// 9
	};
	bool local_worker_to_communicate[num_processes] = {
			true,	// 0
			false,	// 1
			false,	// 2
			true,	// 3
			false,	// 4
			false,	// 5
			true,	// 6
			false,	// 7
			true,	// 8
			false	// 9
	};

	sip::ConfigurableRankDistribution rd (num_workers, num_servers);
	const std::vector<bool> is_server_vector = rd.get_is_server_vector();

	for (int i=0; i<num_processes; ++i){
		ASSERT_EQ(reference_is_server_array[i], is_server_vector[i]);
	}

	for (int i=0; i<num_processes; ++i){
		ASSERT_EQ(local_worker_to_communicate[i], rd.is_local_worker_to_communicate(i) );
	}

	int local_servers_to_communicate[num_processes][num_processes] = { -1 };
	local_servers_to_communicate[0][0] = 2;
	local_servers_to_communicate[3][0] = 5;
	local_servers_to_communicate[6][0] = 7;
	local_servers_to_communicate[8][0] = 9;

	for (int i=0; i<num_processes; ++i){
		if (rd.is_local_worker_to_communicate(i)){
			std::vector<int> servers = rd.local_servers_to_communicate(i);
			ASSERT_EQ(servers.size(), 1);
			for (int j=0; j<servers.size(); ++j){
				ASSERT_EQ(local_servers_to_communicate[i][j], servers[j]);
			}
		}
	}
}

TEST(ConfigurableRankDistribution, test5){
	const int num_processes = 10;
	const int num_workers = 5;
	const int num_servers = 5;
	bool reference_is_server_array[num_processes] = {
			false,	// 0
			true,	// 1
			false,	// 2
			true,	// 3
			false,	// 4
			true,	// 5
			false,	// 6
			true,	// 7
			false,	// 8
			true	// 9
	};
	bool local_worker_to_communicate[num_processes] = {
			true,	// 0
			false,	// 1
			true,	// 2
			false,	// 3
			true,	// 4
			false,	// 5
			true,	// 6
			false,	// 7
			true,	// 8
			false	// 9
	};

	sip::ConfigurableRankDistribution rd (num_workers, num_servers);
	const std::vector<bool> is_server_vector = rd.get_is_server_vector();

	for (int i=0; i<num_processes; ++i){
		ASSERT_EQ(reference_is_server_array[i], is_server_vector[i]);
	}

	for (int i=0; i<num_processes; ++i){
		ASSERT_EQ(local_worker_to_communicate[i], rd.is_local_worker_to_communicate(i) );
	}

	int local_servers_to_communicate[num_processes][num_processes] = { -1 };
	local_servers_to_communicate[0][0] = 1;
	local_servers_to_communicate[2][0] = 3;
	local_servers_to_communicate[4][0] = 5;
	local_servers_to_communicate[6][0] = 7;
	local_servers_to_communicate[8][0] = 9;

	for (int i=0; i<num_processes; ++i){
		if (rd.is_local_worker_to_communicate(i)){
			std::vector<int> servers = rd.local_servers_to_communicate(i);
			ASSERT_EQ(servers.size(), 1);
			for (int j=0; j<servers.size(); ++j){
				ASSERT_EQ(local_servers_to_communicate[i][j], servers[j]);
			}
		}
	}
}

TEST(ConfigurableRankDistribution, test6){
	const int num_processes = 10;
	const int num_workers = 4;
	const int num_servers = 6;
	bool reference_is_server_array[num_processes] = {
			false,	// 0
			true,	// 1
			true,	// 2
			false,	// 3
			true,	// 4
			true,	// 5
			false,	// 6
			true,	// 7
			false,	// 8
			true	// 9
	};
	bool local_worker_to_communicate[num_processes] = {
			true,	// 0
			false,	// 1
			false,	// 2
			true,	// 3
			false,	// 4
			false,	// 5
			true,	// 6
			false,	// 7
			true,	// 8
			false	// 9
	};

	sip::ConfigurableRankDistribution rd (num_workers, num_servers);
	const std::vector<bool> is_server_vector = rd.get_is_server_vector();

	for (std::vector<bool>::const_iterator it = is_server_vector.begin(); it!=is_server_vector.end(); ++it){
		std::cout << *it << "\t" ;
	}
	std::cout << std::endl;
	for (int i=0; i<num_processes; ++i){
		std::cout << rd.is_local_worker_to_communicate(i) << "\t";
	}
	std::cout << std::endl;

	for (int i=0; i<num_processes; ++i){
		ASSERT_EQ(reference_is_server_array[i], is_server_vector[i]);
	}

	for (int i=0; i<num_processes; ++i){
		ASSERT_EQ(local_worker_to_communicate[i], rd.is_local_worker_to_communicate(i) );
	}

	int local_servers_to_communicate[num_processes][num_processes] = { -1 };
	local_servers_to_communicate[0][0] = 1;
	local_servers_to_communicate[0][1] = 2;
	local_servers_to_communicate[3][0] = 4;
	local_servers_to_communicate[3][1] = 5;
	local_servers_to_communicate[6][0] = 7;
	local_servers_to_communicate[8][0] = 9;

	for (int i=0; i<num_processes; ++i){
		if (rd.is_local_worker_to_communicate(i)){
			std::vector<int> servers = rd.local_servers_to_communicate(i);
			ASSERT_GT(servers.size(), 0);
			for (int j=0; j<servers.size(); ++j){
				ASSERT_EQ(local_servers_to_communicate[i][j], servers[j]);
			}
		}
	}
}

TEST(ConfigurableRankDistribution, test7){
	const int num_processes = 10;
	const int num_workers = 3;
	const int num_servers = 7;
	bool reference_is_server_array[num_processes] = {
			false,	// 0
			true,	// 1
			true,	// 2
			true,	// 3
			false,	// 4
			true,	// 5
			true,	// 6
			false,	// 7
			true,	// 8
			true	// 9
	};
	bool local_worker_to_communicate[num_processes] = {
			true,	// 0
			false,	// 1
			false,	// 2
			false,	// 3
			true,	// 4
			false,	// 5
			false,	// 6
			true,	// 7
			false,	// 8
			false	// 9
	};

	sip::ConfigurableRankDistribution rd (num_workers, num_servers);
	const std::vector<bool> is_server_vector = rd.get_is_server_vector();

	for (std::vector<bool>::const_iterator it = is_server_vector.begin(); it!=is_server_vector.end(); ++it){
		std::cout << *it << "\t" ;
	}
	std::cout << std::endl;
	for (int i=0; i<num_processes; ++i){
		std::cout << rd.is_local_worker_to_communicate(i) << "\t";
	}
	std::cout << std::endl;

	for (int i=0; i<num_processes; ++i){
		ASSERT_EQ(reference_is_server_array[i], is_server_vector[i]);
	}

	for (int i=0; i<num_processes; ++i){
		ASSERT_EQ(local_worker_to_communicate[i], rd.is_local_worker_to_communicate(i) );
	}

	int local_servers_to_communicate[num_processes][num_processes] = { -1 };
	local_servers_to_communicate[0][0] = 1;
	local_servers_to_communicate[0][1] = 2;
	local_servers_to_communicate[0][2] = 3;
	local_servers_to_communicate[4][0] = 5;
	local_servers_to_communicate[4][1] = 6;
	local_servers_to_communicate[7][0] = 8;
	local_servers_to_communicate[7][1] = 9;

	for (int i=0; i<num_processes; ++i){
		if (rd.is_local_worker_to_communicate(i)){
			std::vector<int> servers = rd.local_servers_to_communicate(i);
			ASSERT_GT(servers.size(), 0);
			for (int j=0; j<servers.size(); ++j){
				ASSERT_EQ(local_servers_to_communicate[i][j], servers[j]);
			}
		}
	}
}


TEST(ConfigurableRankDistribution, test8){
	const int num_processes = 10;
	const int num_workers = 2;
	const int num_servers = 8;
	bool reference_is_server_array[num_processes] = {
			false,	// 0
			true,	// 1
			true,	// 2
			true,	// 3
			true,	// 4
			false,	// 5
			true,	// 6
			true,	// 7
			true,	// 8
			true	// 9
	};
	bool local_worker_to_communicate[num_processes] = {
			true,	// 0
			false,	// 1
			false,	// 2
			false,	// 3
			false,	// 4
			true,	// 5
			false,	// 6
			false,	// 7
			false,	// 8
			false	// 9
	};

	sip::ConfigurableRankDistribution rd (num_workers, num_servers);
	const std::vector<bool> is_server_vector = rd.get_is_server_vector();

	for (std::vector<bool>::const_iterator it = is_server_vector.begin(); it!=is_server_vector.end(); ++it){
		std::cout << *it << "\t" ;
	}
	std::cout << std::endl;
	for (int i=0; i<num_processes; ++i){
		std::cout << rd.is_local_worker_to_communicate(i) << "\t";
	}
	std::cout << std::endl;

	for (int i=0; i<num_processes; ++i){
		ASSERT_EQ(reference_is_server_array[i], is_server_vector[i]);
	}

	for (int i=0; i<num_processes; ++i){
		ASSERT_EQ(local_worker_to_communicate[i], rd.is_local_worker_to_communicate(i) );
	}

	int local_servers_to_communicate[num_processes][num_processes] = { -1 };
	local_servers_to_communicate[0][0] = 1;
	local_servers_to_communicate[0][1] = 2;
	local_servers_to_communicate[0][2] = 3;
	local_servers_to_communicate[0][3] = 4;
	local_servers_to_communicate[5][0] = 6;
	local_servers_to_communicate[5][1] = 7;
	local_servers_to_communicate[5][2] = 8;
	local_servers_to_communicate[5][3] = 9;

	for (int i=0; i<num_processes; ++i){
		if (rd.is_local_worker_to_communicate(i)){
			std::vector<int> servers = rd.local_servers_to_communicate(i);
			ASSERT_GT(servers.size(), 0);
			for (int j=0; j<servers.size(); ++j){
				ASSERT_EQ(local_servers_to_communicate[i][j], servers[j]);
			}
		}
	}
}

TEST(ConfigurableRankDistribution, test9){
	const int num_processes = 10;
	const int num_workers = 1;
	const int num_servers = 9;
	bool reference_is_server_array[num_processes] = {
			false,	// 0
			true,	// 1
			true,	// 2
			true,	// 3
			true,	// 4
			true,	// 5
			true,	// 6
			true,	// 7
			true,	// 8
			true	// 9
	};
	bool local_worker_to_communicate[num_processes] = {
			true,	// 0
			false,	// 1
			false,	// 2
			false,	// 3
			false,	// 4
			false,	// 5
			false,	// 6
			false,	// 7
			false,	// 8
			false	// 9
	};

	sip::ConfigurableRankDistribution rd (num_workers, num_servers);
	const std::vector<bool> is_server_vector = rd.get_is_server_vector();

	for (std::vector<bool>::const_iterator it = is_server_vector.begin(); it!=is_server_vector.end(); ++it){
		std::cout << *it << "\t" ;
	}
	std::cout << std::endl;
	for (int i=0; i<num_processes; ++i){
		std::cout << rd.is_local_worker_to_communicate(i) << "\t";
	}
	std::cout << std::endl;

	for (int i=0; i<num_processes; ++i){
		ASSERT_EQ(reference_is_server_array[i], is_server_vector[i]);
	}

	for (int i=0; i<num_processes; ++i){
		ASSERT_EQ(local_worker_to_communicate[i], rd.is_local_worker_to_communicate(i) );
	}

	int local_servers_to_communicate[num_processes][num_processes] = { -1 };
	local_servers_to_communicate[0][0] = 1;
	local_servers_to_communicate[0][1] = 2;
	local_servers_to_communicate[0][2] = 3;
	local_servers_to_communicate[0][3] = 4;
	local_servers_to_communicate[0][4] = 5;
	local_servers_to_communicate[0][5] = 6;
	local_servers_to_communicate[0][6] = 7;
	local_servers_to_communicate[0][7] = 8;
	local_servers_to_communicate[0][8] = 9;

	for (int i=0; i<num_processes; ++i){
		if (rd.is_local_worker_to_communicate(i)){
			std::vector<int> servers = rd.local_servers_to_communicate(i);
			ASSERT_GT(servers.size(), 0);
			for (int j=0; j<servers.size(); ++j){
				ASSERT_EQ(local_servers_to_communicate[i][j], servers[j]);
			}
		}
	}
}



int main(int argc, char **argv) {

#ifdef HAVE_MPI
	MPI_Init(&argc, &argv);
	int num_procs;
	sip::SIPMPIUtils::check_err(MPI_Comm_size(MPI_COMM_WORLD, &num_procs));

//	if (num_procs < 2){
//		std::cerr<<"Please run this test with at least 2 mpi ranks"<<std::endl;
//		return -1;
//	}
	sip::AllWorkerRankDistribution all_workers_rank_dist;
	sip::SIPMPIAttr::set_rank_distribution(&all_workers_rank_dist);

	sip::SIPMPIUtils::set_error_handler();
#endif // HAVE_MPI
	sip::SIPMPIAttr &sip_mpi_attr = sip::SIPMPIAttr::get_instance();

#ifdef HAVE_TAU
	TAU_PROFILE_SET_NODE(0);
	TAU_STATIC_PHASE_START("SIP Main");
#endif

	sip::check(sizeof(int) >= 4, "Size of integer should be 4 bytes or more");
	sip::check(sizeof(double) >= 8, "Size of double should be 8 bytes or more");
	sip::check(sizeof(long long) >= 8, "Size of long long should be 8 bytes or more");




	printf("Running main() from master_test_main.cpp\n");
	testing::InitGoogleTest(&argc, argv);
	int result = RUN_ALL_TESTS();

#ifdef HAVE_TAU
	TAU_STATIC_PHASE_STOP("SIP Main");
#endif

#ifdef HAVE_MPI
	MPI_Finalize();
#endif

	return result;

}
