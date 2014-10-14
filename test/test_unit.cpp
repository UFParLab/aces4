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
#include "config.h"

#include "gtest/gtest.h"

#include "array_constants.h"
#include "id_block_map.h"
#include "server_block.h"
#include "block_id.h"
#include "block.h"
#include "lru_array_policy.h"
#include "sip_mpi_utils.h"
#include "sip_mpi_attr.h"

#include "sqlite3.h"
#include "profile_timer.h"
#include "profile_timer_store.h"

#ifdef HAVE_MPI
#include "mpi.h"
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

	sip::check(policy.get_next_block_for_removal().array_id() == 0, "Block to remove should be from array 0");
	block_map.get_and_remove_block(bid0);
	sip::check(policy.get_next_block_for_removal().array_id() == 1, "Block to remove should be from array 1");
	block_map.get_and_remove_block(bid1);
	sip::check(policy.get_next_block_for_removal().array_id() == 2, "Block to remove should be from array 2");
	block_map.get_and_remove_block(bid2);
	sip::check(policy.get_next_block_for_removal().array_id() == 3, "Block to remove should be from array 3");
	block_map.get_and_remove_block(bid3);
	sip::check(policy.get_next_block_for_removal().array_id() == 4, "Block to remove should be from array 4");
	block_map.get_and_remove_block(bid4);
	sip::check(policy.get_next_block_for_removal().array_id() == 5, "Block to remove should be from array 5");
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
	sip::ServerBlock *sb = new sip::ServerBlock(1);

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

	sip::check(policy.get_next_block_for_removal().array_id() == 0, "Block to remove should be from array 0");
	server_block_map.get_and_remove_block(bid0);
	sip::check(policy.get_next_block_for_removal().array_id() == 1, "Block to remove should be from array 1");
	server_block_map.get_and_remove_block(bid1);
	sip::check(policy.get_next_block_for_removal().array_id() == 2, "Block to remove should be from array 2");
	server_block_map.get_and_remove_block(bid2);
	sip::check(policy.get_next_block_for_removal().array_id() == 3, "Block to remove should be from array 3");
	server_block_map.get_and_remove_block(bid3);
	sip::check(policy.get_next_block_for_removal().array_id() == 4, "Block to remove should be from array 4");
	server_block_map.get_and_remove_block(bid4);
	sip::check(policy.get_next_block_for_removal().array_id() == 5, "Block to remove should be from array 5");
	server_block_map.get_and_remove_block(bid5);

}
#endif // HAVE_MPI


// Tests for sip::ProfileInterpreterStore

// Basic sanity check.
// Makes sure creation of tables doesnt cause problems
TEST(SialUnitProfileTimerStore, test_creation){
	sip::ProfileTimerStore profile_timer_store(":memory:");
}

// Just inserts a zero block operation.
TEST(SialUnitProfileTimerStore, insert_zero_block){
	sip::ProfileTimerStore profile_timer_store (":memory:");
	std::vector<sip::ProfileTimer::BlockInfo> blocks;
	sip::ProfileTimer::Key key1("test1", blocks);
	std::pair<double, int> time_count = std::make_pair(100, 10);
	profile_timer_store.save_to_store(key1, time_count);
}

// Inserts & retrieve a zero block operation
TEST(SialUnitProfileTimerStore, retrieve_zero_block){
	sip::ProfileTimerStore profile_timer_store (":memory:");
	std::vector<sip::ProfileTimer::BlockInfo> blocks;
	sip::ProfileTimer::Key key1("test1", blocks);
	std::pair<double, int> time_count = std::make_pair(100, 10);
	profile_timer_store.save_to_store(key1, time_count);

	std::pair<double, int> result = profile_timer_store.get_from_store(key1);
	ASSERT_EQ(time_count, result);
}

// Inserts a 0-block operation. Tries to get a different block with a different opcode
// expects an exception
TEST(SialUnitProfileTimerStore, invalid_retrieve_zero_block){
	sip::ProfileTimerStore profile_timer_store (":memory:");
	std::vector<sip::ProfileTimer::BlockInfo> blocks;
	sip::ProfileTimer::Key key1("test1", blocks);
	std::pair<double, int> time_count = std::make_pair(100, 10);
	profile_timer_store.save_to_store(key1, time_count);

	sip::ProfileTimer::Key key2("test2", blocks);
	ASSERT_THROW(profile_timer_store.get_from_store(key2),std::invalid_argument);
}

// Just inserts a one block operation.
TEST(SialUnitProfileTimerStore, insert_one_block){
	sip::ProfileTimerStore profile_timer_store (":memory:");
	sip::index_selector_t indices;
	sip::segment_size_array_t segments;

	for (int i=0; i<MAX_RANK; i++)
		indices[i] = (i + 10) % 3;
	std::fill(segments + 0, segments + MAX_RANK, 35);

	sip::ProfileTimer::BlockInfo b1(2, indices, segments);
	std::vector<sip::ProfileTimer::BlockInfo> blocks;
	blocks.push_back(b1);
	sip::ProfileTimer::Key key1("test1", blocks);
	std::pair<double, int> time_count = std::make_pair(100, 10);
	profile_timer_store.save_to_store(key1, time_count);
}

// Inserts & retrieve a one block operation
TEST(SialUnitProfileTimerStore, retrieve_one_block){
	sip::ProfileTimerStore profile_timer_store (":memory:");
	sip::index_selector_t indices;
	sip::segment_size_array_t segments;

	for (int i=0; i<MAX_RANK; i++)
		indices[i] = (i + 10) % 3;
	std::fill(segments + 0, segments + MAX_RANK, 35);

	sip::ProfileTimer::BlockInfo b1(2, indices, segments);
	std::vector<sip::ProfileTimer::BlockInfo> blocks;
	blocks.push_back(b1);
	sip::ProfileTimer::Key key1("test1", blocks);
	std::pair<double, int> time_count = std::make_pair(100, 10);
	profile_timer_store.save_to_store(key1, time_count);

	std::pair<double, int> result = profile_timer_store.get_from_store(key1);
	ASSERT_EQ(time_count, result);
}

// Inserts a 1 block operation.
// Tries to get a different 1 block operation with different rank.
// Expects exception
TEST(SialUnitProfileTimerStore, invalid_retrieve_one_block){
	sip::ProfileTimerStore profile_timer_store (":memory:");
	sip::index_selector_t indices;
	sip::segment_size_array_t segments;

	for (int i=0; i<MAX_RANK; i++)
		indices[i] = (i + 10) % 3;
	std::fill(segments + 0, segments + MAX_RANK, 35);

	sip::ProfileTimer::BlockInfo b1(2, indices, segments);
	std::vector<sip::ProfileTimer::BlockInfo> blocks;
	blocks.push_back(b1);
	sip::ProfileTimer::Key key1("test1", blocks);
	std::pair<double, int> time_count = std::make_pair(100, 10);
	profile_timer_store.save_to_store(key1, time_count);

	sip::ProfileTimer::BlockInfo b2(3, indices, segments);
	std::vector<sip::ProfileTimer::BlockInfo> blocks2;
	blocks2.push_back(b2);
	sip::ProfileTimer::Key key2("test1", blocks2);
	ASSERT_THROW(profile_timer_store.get_from_store(key2),std::invalid_argument);
}

// Inserts a 1 block operation.
// Tries to get a different 1 block operation with different segment size.
// Expects exception
TEST(SialUnitProfileTimerStore, invalid_retrieve_one_block_2){
	sip::ProfileTimerStore profile_timer_store (":memory:");
	sip::index_selector_t indices;
	sip::segment_size_array_t segments;

	for (int i=0; i<MAX_RANK; i++)
		indices[i] = (i + 10) % 3;
	std::fill(segments + 0, segments + MAX_RANK, 35);

	sip::ProfileTimer::BlockInfo b1(2, indices, segments);
	std::vector<sip::ProfileTimer::BlockInfo> blocks;
	blocks.push_back(b1);
	sip::ProfileTimer::Key key1("test1", blocks);
	std::pair<double, int> time_count = std::make_pair(100, 10);
	profile_timer_store.save_to_store(key1, time_count);

	segments[0] = 20;
	sip::ProfileTimer::BlockInfo b2(2, indices, segments);
	std::vector<sip::ProfileTimer::BlockInfo> blocks2;
	blocks2.push_back(b2);
	sip::ProfileTimer::Key key2("test1", blocks2);
	ASSERT_THROW(profile_timer_store.get_from_store(key2),std::invalid_argument);
}

// Inserts a 1 block operation.
// Tries to get a different 1 block operation with different indices (shape).
// Expects exception
TEST(SialUnitProfileTimerStore, invalid_retrieve_one_block_3){
	sip::ProfileTimerStore profile_timer_store (":memory:");
	sip::index_selector_t indices;
	sip::segment_size_array_t segments;

	for (int i=0; i<MAX_RANK; i++)
		indices[i] = (i + 10) % 3;
	std::fill(segments + 0, segments + MAX_RANK, 35);

	sip::ProfileTimer::BlockInfo b1(2, indices, segments);
	std::vector<sip::ProfileTimer::BlockInfo> blocks;
	blocks.push_back(b1);
	sip::ProfileTimer::Key key1("test1", blocks);
	std::pair<double, int> time_count = std::make_pair(100, 10);
	profile_timer_store.save_to_store(key1, time_count);

	indices[0] += 1;
	sip::ProfileTimer::BlockInfo b2(2, indices, segments);
	std::vector<sip::ProfileTimer::BlockInfo> blocks2;
	blocks2.push_back(b2);
	sip::ProfileTimer::Key key2("test1", blocks2);
	ASSERT_THROW(profile_timer_store.get_from_store(key2),std::invalid_argument);
}

// Just inserts a 2 block operation
TEST(SialUnitProfileTimerStore, insert_two_block){
	sip::ProfileTimerStore profile_timer_store (":memory:");
	sip::index_selector_t indices;
	sip::segment_size_array_t segments;

	for (int i=0; i<MAX_RANK; i++)
		indices[i] = (i + 10) % 3;
	std::fill(segments + 0, segments + MAX_RANK, 35);

	sip::ProfileTimer::BlockInfo b1(2, indices, segments);
	sip::ProfileTimer::BlockInfo b2(3, indices, segments);
	std::vector<sip::ProfileTimer::BlockInfo> blocks;
	blocks.push_back(b1);
	blocks.push_back(b2);
	sip::ProfileTimer::Key key1("test1", blocks);
	std::pair<double, int> time_count = std::make_pair(100, 10);
	profile_timer_store.save_to_store(key1, time_count);
}

// Inserts & retrieves a 2 block operation
TEST(SialUnitProfileTimerStore, retrieve_two_block){
	sip::ProfileTimerStore profile_timer_store (":memory:");
	sip::index_selector_t indices;
	sip::segment_size_array_t segments;

	for (int i=0; i<MAX_RANK; i++)
		indices[i] = (i + 10) % 3;
	std::fill(segments + 0, segments + MAX_RANK, 35);

	sip::ProfileTimer::BlockInfo b1(2, indices, segments);
	sip::ProfileTimer::BlockInfo b2(3, indices, segments);
	std::vector<sip::ProfileTimer::BlockInfo> blocks;
	blocks.push_back(b1);
	blocks.push_back(b2);
	sip::ProfileTimer::Key key1("test1", blocks);
	std::pair<double, int> time_count = std::make_pair(100, 10);
	profile_timer_store.save_to_store(key1, time_count);

	std::pair<double, int> result = profile_timer_store.get_from_store(key1);
	ASSERT_EQ(time_count, result);
}

// Just inserts a 3 block operation
TEST(SialUnitProfileTimerStore, insert_three_block){
	sip::ProfileTimerStore profile_timer_store (":memory:");
	sip::index_selector_t indices;
	sip::segment_size_array_t segments;

	for (int i=0; i<MAX_RANK; i++)
		indices[i] = (i + 10) % 3;
	std::fill(segments + 0, segments + MAX_RANK, 35);

	sip::ProfileTimer::BlockInfo b1(5, indices, segments);
	sip::ProfileTimer::BlockInfo b2(3, indices, segments);
	sip::ProfileTimer::BlockInfo b3(2, indices, segments);
	std::vector<sip::ProfileTimer::BlockInfo> blocks;
	blocks.push_back(b1);
	blocks.push_back(b2);
	blocks.push_back(b3);
	sip::ProfileTimer::Key key1("test1", blocks);
	std::pair<double, int> time_count = std::make_pair(100, 10);
	profile_timer_store.save_to_store(key1, time_count);
}

// Inserts & retrieves a 3 block operation
TEST(SialUnitProfileTimerStore, retrieve_three_block){
	sip::ProfileTimerStore profile_timer_store (":memory:");
	sip::index_selector_t indices;
	sip::segment_size_array_t segments;

	for (int i=0; i<MAX_RANK; i++)
		indices[i] = (i + 10) % 3;
	std::fill(segments + 0, segments + MAX_RANK, 35);

	sip::ProfileTimer::BlockInfo b1(5, indices, segments);
	sip::ProfileTimer::BlockInfo b2(3, indices, segments);
	sip::ProfileTimer::BlockInfo b3(2, indices, segments);
	std::vector<sip::ProfileTimer::BlockInfo> blocks;
	blocks.push_back(b1);
	blocks.push_back(b2);
	blocks.push_back(b3);
	sip::ProfileTimer::Key key1("test1", blocks);
	std::pair<double, int> time_count = std::make_pair(100, 10);
	profile_timer_store.save_to_store(key1, time_count);

	std::pair<double, int> result = profile_timer_store.get_from_store(key1);
	ASSERT_EQ(time_count, result);
}

// Just inserts a four block operation
TEST(SialUnitProfileTimerStore, insert_four_block){
	sip::ProfileTimerStore profile_timer_store (":memory:");
	sip::index_selector_t indices;
	sip::segment_size_array_t segments;

	for (int i=0; i<MAX_RANK; i++)
		indices[i] = (i + 10) % 3;
	std::fill(segments + 0, segments + MAX_RANK, 35);

	sip::ProfileTimer::BlockInfo b1(5, indices, segments);
	sip::ProfileTimer::BlockInfo b2(3, indices, segments);
	sip::ProfileTimer::BlockInfo b3(2, indices, segments);
	sip::ProfileTimer::BlockInfo b4(6, indices, segments);
	std::vector<sip::ProfileTimer::BlockInfo> blocks;
	blocks.push_back(b1);
	blocks.push_back(b2);
	blocks.push_back(b3);
	blocks.push_back(b4);
	sip::ProfileTimer::Key key1("test1", blocks);
	std::pair<double, int> time_count = std::make_pair(100, 10);
	profile_timer_store.save_to_store(key1, time_count);
}

// Inserts & retrieve a 4 block operation
TEST(SialUnitProfileTimerStore, retrieve_four_block){
	sip::ProfileTimerStore profile_timer_store (":memory:");
	sip::index_selector_t indices;
	sip::segment_size_array_t segments;

	for (int i=0; i<MAX_RANK; i++)
		indices[i] = (i + 10) % 3;
	std::fill(segments + 0, segments + MAX_RANK, 35);

	sip::ProfileTimer::BlockInfo b1(5, indices, segments);
	sip::ProfileTimer::BlockInfo b2(3, indices, segments);
	sip::ProfileTimer::BlockInfo b3(2, indices, segments);
	sip::ProfileTimer::BlockInfo b4(6, indices, segments);
	std::vector<sip::ProfileTimer::BlockInfo> blocks;
	blocks.push_back(b1);
	blocks.push_back(b2);
	blocks.push_back(b3);
	blocks.push_back(b4);
	sip::ProfileTimer::Key key1("test1", blocks);
	std::pair<double, int> time_count = std::make_pair(100, 10);
	profile_timer_store.save_to_store(key1, time_count);

	std::pair<double, int> result = profile_timer_store.get_from_store(key1);
	ASSERT_EQ(time_count, result);
}

// Just inserts a 5 block operation
TEST(SialUnitProfileTimerStore, insert_five_block){
	sip::ProfileTimerStore profile_timer_store (":memory:");
	sip::index_selector_t indices;
	sip::segment_size_array_t segments;

	for (int i=0; i<MAX_RANK; i++)
		indices[i] = (i + 10) % 3;
	std::fill(segments + 0, segments + MAX_RANK, 35);

	sip::ProfileTimer::BlockInfo b1(5, indices, segments);
	sip::ProfileTimer::BlockInfo b2(3, indices, segments);
	sip::ProfileTimer::BlockInfo b3(2, indices, segments);
	sip::ProfileTimer::BlockInfo b4(6, indices, segments);
	sip::ProfileTimer::BlockInfo b5(1, indices, segments);
	std::vector<sip::ProfileTimer::BlockInfo> blocks;
	blocks.push_back(b1);
	blocks.push_back(b2);
	blocks.push_back(b3);
	blocks.push_back(b4);
	blocks.push_back(b5);
	sip::ProfileTimer::Key key1("test1", blocks);
	std::pair<double, int> time_count = std::make_pair(100, 10);
	profile_timer_store.save_to_store(key1, time_count);
}

// Inserts & retrieve a 5 block operation
TEST(SialUnitProfileTimerStore, retrieve_five_block){
	sip::ProfileTimerStore profile_timer_store (":memory:");
	sip::index_selector_t indices;
	sip::segment_size_array_t segments;

	for (int i=0; i<MAX_RANK; i++)
		indices[i] = (i + 10) % 3;
	std::fill(segments + 0, segments + MAX_RANK, 35);

	sip::ProfileTimer::BlockInfo b1(5, indices, segments);
	sip::ProfileTimer::BlockInfo b2(3, indices, segments);
	sip::ProfileTimer::BlockInfo b3(2, indices, segments);
	sip::ProfileTimer::BlockInfo b4(6, indices, segments);
	sip::ProfileTimer::BlockInfo b5(1, indices, segments);
	std::vector<sip::ProfileTimer::BlockInfo> blocks;
	blocks.push_back(b1);
	blocks.push_back(b2);
	blocks.push_back(b3);
	blocks.push_back(b4);
	blocks.push_back(b5);
	sip::ProfileTimer::Key key1("test1", blocks);
	std::pair<double, int> time_count = std::make_pair(100, 10);
	profile_timer_store.save_to_store(key1, time_count);

	std::pair<double, int> result = profile_timer_store.get_from_store(key1);
	ASSERT_EQ(time_count, result);
}

// Inserts a 5 block operation
// Tries to retrieve an operation with different number of blocks (1 less)
// Expects an exception.
TEST(SialUnitProfileTimerStore, invalid_retrieve_five_block){
	sip::ProfileTimerStore profile_timer_store (":memory:");
	sip::index_selector_t indices;
	sip::segment_size_array_t segments;

	for (int i=0; i<MAX_RANK; i++)
		indices[i] = (i + 10) % 3;
	std::fill(segments + 0, segments + MAX_RANK, 35);

	sip::ProfileTimer::BlockInfo b1(5, indices, segments);
	sip::ProfileTimer::BlockInfo b2(3, indices, segments);
	sip::ProfileTimer::BlockInfo b3(2, indices, segments);
	sip::ProfileTimer::BlockInfo b4(6, indices, segments);
	sip::ProfileTimer::BlockInfo b5(1, indices, segments);
	std::vector<sip::ProfileTimer::BlockInfo> blocks;
	blocks.push_back(b1);
	blocks.push_back(b2);
	blocks.push_back(b3);
	blocks.push_back(b4);
	blocks.push_back(b5);
	sip::ProfileTimer::Key key1("test1", blocks);
	std::pair<double, int> time_count = std::make_pair(100, 10);
	profile_timer_store.save_to_store(key1, time_count);

	// Remove 1 block & try
	blocks.pop_back();
	sip::ProfileTimer::Key key2("test1", blocks);
	ASSERT_THROW(profile_timer_store.get_from_store(key2),std::invalid_argument);
	//std::pair<double, int> result = profile_timer_store.get_from_store(key2);
}


// Just inserts a 5 block operation
TEST(SialUnitProfileTimerStore, insert_six_block){
	sip::ProfileTimerStore profile_timer_store (":memory:");
	sip::index_selector_t indices;
	sip::segment_size_array_t segments;

	for (int i=0; i<MAX_RANK; i++)
		indices[i] = (i + 10) % 3;
	std::fill(segments + 0, segments + MAX_RANK, 35);

	sip::ProfileTimer::BlockInfo b1(5, indices, segments);
	sip::ProfileTimer::BlockInfo b2(3, indices, segments);
	sip::ProfileTimer::BlockInfo b3(2, indices, segments);
	sip::ProfileTimer::BlockInfo b4(6, indices, segments);
	sip::ProfileTimer::BlockInfo b5(1, indices, segments);
	sip::ProfileTimer::BlockInfo b6(4, indices, segments);
	std::vector<sip::ProfileTimer::BlockInfo> blocks;
	blocks.push_back(b1);
	blocks.push_back(b2);
	blocks.push_back(b3);
	blocks.push_back(b4);
	blocks.push_back(b5);
	blocks.push_back(b6);
	sip::ProfileTimer::Key key1("test1", blocks);
	std::pair<double, int> time_count = std::make_pair(100, 10);
	profile_timer_store.save_to_store(key1, time_count);
}

// Inserts & retrieve a 5 block operation
TEST(SialUnitProfileTimerStore, retrieve_sixls_block){
	sip::ProfileTimerStore profile_timer_store (":memory:");
	sip::index_selector_t indices;
	sip::segment_size_array_t segments;

	for (int i=0; i<MAX_RANK; i++)
		indices[i] = (i + 10) % 3;
	std::fill(segments + 0, segments + MAX_RANK, 35);

	sip::ProfileTimer::BlockInfo b1(5, indices, segments);
	sip::ProfileTimer::BlockInfo b2(3, indices, segments);
	sip::ProfileTimer::BlockInfo b3(2, indices, segments);
	sip::ProfileTimer::BlockInfo b4(6, indices, segments);
	sip::ProfileTimer::BlockInfo b5(1, indices, segments);
	sip::ProfileTimer::BlockInfo b6(4, indices, segments);
	std::vector<sip::ProfileTimer::BlockInfo> blocks;
	blocks.push_back(b1);
	blocks.push_back(b2);
	blocks.push_back(b3);
	blocks.push_back(b4);
	blocks.push_back(b5);
	blocks.push_back(b6);
	sip::ProfileTimer::Key key1("test1", blocks);
	std::pair<double, int> time_count = std::make_pair(100, 10);
	profile_timer_store.save_to_store(key1, time_count);

	std::pair<double, int> result = profile_timer_store.get_from_store(key1);
	ASSERT_EQ(time_count, result);
}



int main(int argc, char **argv) {

#ifdef HAVE_MPI
	MPI_Init(&argc, &argv);
	int num_procs;
	sip::SIPMPIUtils::check_err(MPI_Comm_size(MPI_COMM_WORLD, &num_procs));

	if (num_procs < 2){
		std::cerr<<"Please run this test with at least 2 mpi ranks"<<std::endl;
		return -1;
	}

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
