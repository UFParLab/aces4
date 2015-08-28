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


#include "array_file.h"
#include "chunk_manager.h"
#include "chunk.h"


#ifdef HAVE_TAU
#include <TAU.h>
#endif

//TEST(Sial_Unit,DISABLED_BlockLRUArrayPolicy){
//
//	sip::index_value_array_t index_values;
//	std::fill(index_values, index_values+MAX_RANK, sip::unused_index_value);
//
//	sip::BlockId bid0(0, index_values);
//	sip::BlockId bid1(1, index_values);
//	sip::BlockId bid2(2, index_values);
//	sip::BlockId bid3(3, index_values);
//	sip::BlockId bid4(4, index_values);
//	sip::BlockId bid5(5, index_values);
//
//	sip::Block *blk = NULL;
//
//	sip::IdBlockMap<sip::Block> block_map(6);
//	block_map.insert_block(bid0, blk);
//	block_map.insert_block(bid1, blk);
//	block_map.insert_block(bid2, blk);
//	block_map.insert_block(bid3, blk);
//	block_map.insert_block(bid4, blk);
//	block_map.insert_block(bid5, blk);
//
//	sip::LRUArrayPolicy<sip::Block> policy(block_map);
//	policy.touch(bid0);
//	policy.touch(bid1);
//	policy.touch(bid2);
//	policy.touch(bid3);
//	policy.touch(bid4);
//	policy.touch(bid5);
//
//	sip::Block* b;
//	sip::check(policy.get_next_block_for_removal(b).array_id() == 0, "Block to remove should be from array 0");
//	block_map.get_and_remove_block(bid0);
//	sip::check(policy.get_next_block_for_removal(b).array_id() == 1, "Block to remove should be from array 1");
//	block_map.get_and_remove_block(bid1);
//	sip::check(policy.get_next_block_for_removal(b).array_id() == 2, "Block to remove should be from array 2");
//	block_map.get_and_remove_block(bid2);
//	sip::check(policy.get_next_block_for_removal(b).array_id() == 3, "Block to remove should be from array 3");
//	block_map.get_and_remove_block(bid3);
//	sip::check(policy.get_next_block_for_removal(b).array_id() == 4, "Block to remove should be from array 4");
//	block_map.get_and_remove_block(bid4);
//	sip::check(policy.get_next_block_for_removal(b).array_id() == 5, "Block to remove should be from array 5");
//	block_map.get_and_remove_block(bid5);
//
//}
//
//
//TEST(Sial_Unit,DISABLED_ServerBlockLRUArrayPolicy){
//
//	sip::index_value_array_t index_values;
//	std::fill(index_values, index_values+MAX_RANK, sip::unused_index_value);
//
//	sip::BlockId bid0(0, index_values);
//	sip::BlockId bid1(1, index_values);
//	sip::BlockId bid2(2, index_values);
//	sip::BlockId bid3(3, index_values);
//	sip::BlockId bid4(4, index_values);
//	sip::BlockId bid5(5, index_values);
//
//	// Allocate dummy data for ServerBlock so that ServerBlock
//	// Specific LRUArrayPolicy processes it correctly.
//	sip::ServerBlock *sb = new sip::ServerBlock(1, true);
//
//	sip::IdBlockMap<sip::ServerBlock> server_block_map(6);
//	server_block_map.insert_block(bid0, sb);
//	server_block_map.insert_block(bid1, sb);
//	server_block_map.insert_block(bid2, sb);
//	server_block_map.insert_block(bid3, sb);
//	server_block_map.insert_block(bid4, sb);
//	server_block_map.insert_block(bid5, sb);
//
//	sip::LRUArrayPolicy<sip::ServerBlock> policy(server_block_map);
//	policy.touch(bid0);
//	policy.touch(bid1);
//	policy.touch(bid2);
//	policy.touch(bid3);
//	policy.touch(bid4);
//	policy.touch(bid5);
//
//	sip::ServerBlock* b;
//
//	sip::check(policy.get_next_block_for_removal(b).array_id() == 0, "Block to remove should be from array 0");
//	server_block_map.get_and_remove_block(bid0);
//	sip::check(policy.get_next_block_for_removal(b).array_id() == 1, "Block to remove should be from array 1");
//	server_block_map.get_and_remove_block(bid1);
//	sip::check(policy.get_next_block_for_removal(b).array_id() == 2, "Block to remove should be from array 2");
//	server_block_map.get_and_remove_block(bid2);
//	sip::check(policy.get_next_block_for_removal(b).array_id() == 3, "Block to remove should be from array 3");
//	server_block_map.get_and_remove_block(bid3);
//	sip::check(policy.get_next_block_for_removal(b).array_id() == 4, "Block to remove should be from array 4");
//	server_block_map.get_and_remove_block(bid4);
//	sip::check(policy.get_next_block_for_removal(b).array_id() == 5, "Block to remove should be from array 5");
//	server_block_map.get_and_remove_block(bid5);
//
//}
//

/** Just test opening, closing, and reopening the an ArrayFile.  This does write and read the index */
TEST(Server,array_file_0){
	std::string filename("array_file_0");
	typedef sip::ArrayFile::offset_val_t index_val_t;
	typedef sip::ArrayFile::header_val_t header_val_t;
	header_val_t chunk_size = 555;
	header_val_t num_blocks = 999;
	bool is_new = true;
	bool save_after_close = true;
	{
	sip::ArrayFile file0(num_blocks, chunk_size, filename, MPI_COMM_WORLD, is_new, save_after_close );
	file0.mark_persistent(filename);
	std::cout << "file0\n" << file0 << std::endl;
	}
	//file was closed when ArrayFile went out of scope
	//reopen it.
	{
	is_new = false;
	sip::ArrayFile file1(num_blocks, chunk_size, filename, MPI_COMM_WORLD, is_new);
	std::cout << "file1\n" << file1 << std::endl;
	}

}


/** Test opening, closing, and reopening the ArrayFile, plus writing and reading the index.*/
TEST(Server,array_file_1){
	typedef sip::ArrayFile::offset_val_t index_val_t;
	typedef sip::ArrayFile::header_val_t header_val_t;
	std::string filename("array_file_1.test.acf");
	int nprocs;
	int rank;
	MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	header_val_t chunk_size = 5;
	header_val_t num_blocks = nprocs * 2;
	bool is_new = true;
	bool save_after_close = true;
	//create and write the file
	{
	sip::ArrayFile file0(num_blocks, chunk_size, filename, MPI_COMM_WORLD, is_new, save_after_close );
	file0.mark_persistent(filename);
	index_val_t index[num_blocks];
	std::fill(index, index + num_blocks, 0);
	index[rank] = rank;
	index[nprocs+rank] = rank*2;
	file0.write_index(index, num_blocks);
	}

	//file was closed when ArrayFile went out of scope
	//reopen it.
	{
	is_new = false;
	sip::ArrayFile file1(num_blocks, chunk_size, filename, MPI_COMM_WORLD, is_new);
	index_val_t index2[num_blocks];
	std::fill(index2, index2 + num_blocks,0);
	file1.read_index(index2, num_blocks);
	ASSERT_EQ(rank, index2[rank]);
	ASSERT_EQ(rank*2, index2[nprocs+rank]);

	}

}

/** Test opening, closing, and reopening the ArrayFile, plus writing and reading the index, and some chunks*/
TEST(Server,array_file_3){
	std::string filename("array_file_3.test.acf");

	typedef sip::ArrayFile::offset_val_t index_val_t;
	typedef sip::ArrayFile::header_val_t header_val_t;

	int nprocs;
	int rank;
	MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);


	header_val_t chunk_size = 5;
	header_val_t num_blocks = nprocs * 2;

	MPI_Offset offset0 = rank*chunk_size;
	MPI_Offset offset1 = (nprocs + rank) *chunk_size;

	//create and write the file
	double val0 = rank;
	double val1 = rank + nprocs;
	{
	sip::ArrayFile file0(num_blocks, chunk_size, filename, MPI_COMM_WORLD, true, true);
	file0.mark_persistent(filename);
	//initialize and write some chunks
	double data0[chunk_size];

	std::fill(data0, data0+chunk_size, val0);

	double data1[chunk_size];
	std::fill(data1, data1+chunk_size, val1);

	for(int i = 0; i != chunk_size; ++i){
		ASSERT_EQ(val0, data0[i]);
		ASSERT_EQ(val1, data1[i]);
	}
//	sip::Chunk chunk0(data0, chunk_size, offset0, false);
//	sip::Chunk chunk1(data1, chunk_size, offset1, false);
	sip::Chunk chunk0(data0, offset0, false);
	sip::Chunk chunk1(data1, offset1, false);

//	//print the chunk values for ranks 0 and 1
//	if (rank == 0){
//		std::cout << "\nrank " << rank << " data0:" << std::endl;
//		for (int i = 0; i < chunk_size; ++i){
//			std::cout << data0[i] << ',';
//		}
//		std::cout << "\ndata1:" << std::endl;
//		for (int i = 0; i < chunk_size; ++i){
//			std::cout << data1[i] << ',';
//		}
//		std::cout << "\n--------------" << std::endl << std::flush;
//		MPI_Barrier(MPI_COMM_WORLD);
//	} else if (rank == 1){
//		MPI_Barrier(MPI_COMM_WORLD);
//		std::cout << "\nrank " << rank << " data0:" << std::endl;
//		for (int i = 0; i < chunk_size; ++i){
//			std::cout << data0[i] << ',';
//		}
//		std::cout << "\ndata1:"  << std::endl;
//		for (int i = 0; i < chunk_size; ++i){
//			std::cout << data1[i] << ',';
//		}
//		std::cout << "\n++++++++++++++++++=" << std::endl << std::flush;
//	}
//	else{
//		MPI_Barrier(MPI_COMM_WORLD);
//	}

	file0.chunk_write(chunk0);
	file0.chunk_write(chunk1);

	//initialize and write the index
	index_val_t index0[num_blocks];
	std::fill(index0, index0 + num_blocks, 0);
	index0[rank] = offset0;
	index0[nprocs+rank] = offset1;
	file0.write_index(index0, num_blocks);

//	file0.sync();
//	//try reading the file as one big chunk
//	if (rank==0){
//		size_t num_doubles = chunk_size * num_blocks;
//	double * datas = new double[num_doubles];
//	MPI_Offset file_start =0;
//	file0.read_doubles(datas, num_doubles, file_start);
//	std::cout << std::endl << "printing data part of file" << std::endl;
//	for (int i = 0; i < num_doubles; ++i){
//		std::cout << datas[i] << ',';
//	}
//	std::cout << std::endl << std::flush;
//	}


	}

	MPI_Barrier(MPI_COMM_WORLD);
	if(rank == 0){
		std::cout << "@@@@@@@@@@@ part one done" << std::endl;

	}
	MPI_Barrier(MPI_COMM_WORLD);
	//file was closed when ArrayFile went out of scope
	//reopen it.
	{

		sip::ArrayFile file1(num_blocks, chunk_size, filename, MPI_COMM_WORLD, false);

		//read the index
		index_val_t index1[num_blocks];
		std::fill(index1, index1 + num_blocks,0);
		file1.read_index(index1, num_blocks);
		ASSERT_EQ(offset0, index1[rank]);
		ASSERT_EQ(offset1, index1[nprocs+rank]);

//		if (rank == 0){
//			std::cout << "\nrank " << rank << " file1 state after reading index:" << file1 << std::endl;
//			std::cout << "index: ";
//			for (int i = 0; i < num_blocks; ++i){
//				std::cout << index1[i] << ',';
//			}
//			MPI_Barrier(MPI_COMM_WORLD);
//		} else if (rank == 1){
//			MPI_Barrier(MPI_COMM_WORLD);
//			std::cout << "\nrank " << rank << " file1 state after reading index:" << file1 << std::endl;
//			std::cout << "index: ";
//			for (int i = 0; i < num_blocks; ++i){
//				std::cout << index1[i] << ',';
//			}
//		} else {
//			MPI_Barrier(MPI_COMM_WORLD);
//		}
//
//		file1.sync();
//		//try reading the file as one big chunk
//		if (rank==0){
//			size_t num_doubles = chunk_size * num_blocks;
//		double * datas = new double[num_doubles];
//		MPI_Offset file_start =0;
//		file1.read_doubles(datas, num_doubles, file_start);
//		std::cout << std::endl << "printing data part of file" << std::endl;
//		for (int i = 0; i < num_doubles; ++i){
//			std::cout << datas[i] << ',';
//		}
//		std::cout << std::endl << std::flush;
//		}

		double datar0[chunk_size];
		double datar1[chunk_size];
//		sip::Chunk chunkr0(datar0, chunk_size, index1[rank], true);
//		sip::Chunk chunkr1(datar1, chunk_size, index1[nprocs+rank], true);
		sip::Chunk chunkr0(datar0, index1[rank], true);
		sip::Chunk chunkr1(datar1, index1[nprocs+rank], true);
		file1.chunk_read(chunkr0);
		file1.chunk_read(chunkr1);

//		//output the chunk values for ranks 0 and 1
//		if (rank == 0){
//			std::cout << "rank " << rank << " datar0:" << file1 << std::endl;
//			for (int i = 0; i < chunk_size; ++i){
//				std::cout << datar0[i] << ',';
//			}
//			std::cout << " datar1:" << file1 << std::endl;
//			for (int i = 0; i < chunk_size; ++i){
//				std::cout << datar1[i] << ',';
//			}
//			MPI_Barrier(MPI_COMM_WORLD);
//		} else if (rank == 1){
//			MPI_Barrier(MPI_COMM_WORLD);
//			std::cout << "rank " << rank << " datar0:" << file1 << std::endl;
//			for (int i = 0; i < chunk_size; ++i){
//				std::cout << datar0[i] << ',';
//			}
//			std::cout << " datar1:" << file1 << std::endl;
//			for (int i = 0; i < chunk_size; ++i){
//				std::cout << datar1[i] << ',';
//			}
//
//		} else {
//			MPI_Barrier(MPI_COMM_WORLD);
//		}
		for(int i = 0; i != chunk_size; ++i){
			ASSERT_EQ(val0, datar0[i]);
			ASSERT_EQ(val1, datar1[i]);
		}
	}

}



/** Same as array_file_3 except that it uses collective IO*/
TEST(Server,array_file_4){
	std::string filename("array_file_4.test.acf");

	typedef sip::ArrayFile::offset_val_t index_val_t;
	typedef sip::ArrayFile::header_val_t header_val_t;

	int nprocs;
	int rank;
	MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);


	header_val_t chunk_size = 5;
	header_val_t num_blocks = nprocs * 2;

	MPI_Offset offset0 = rank*chunk_size;
	MPI_Offset offset1 = (nprocs + rank) *chunk_size;

	//create and write the file
	double val0 = rank;
	double val1 = rank + nprocs;
	{
	sip::ArrayFile file0(num_blocks, chunk_size, filename, MPI_COMM_WORLD, true, true);
	file0.mark_persistent(filename);
	//initialize and write some chunks
	double data0[chunk_size];

	std::fill(data0, data0+chunk_size, val0);

	double data1[chunk_size];
	std::fill(data1, data1+chunk_size, val1);

	for(int i = 0; i != chunk_size; ++i){
		ASSERT_EQ(val0, data0[i]);
		ASSERT_EQ(val1, data1[i]);
	}
//	sip::Chunk chunk0(data0, chunk_size, offset0, false);
//	sip::Chunk chunk1(data1, chunk_size, offset1, false);
	sip::Chunk chunk0(data0, offset0, false);
	sip::Chunk chunk1(data1, offset1, false);

//	//print the chunk values for ranks 0 and 1
//	if (rank == 0){
//		std::cout << "\nrank " << rank << " data0:" << std::endl;
//		for (int i = 0; i < chunk_size; ++i){
//			std::cout << data0[i] << ',';
//		}
//		std::cout << "\ndata1:" << std::endl;
//		for (int i = 0; i < chunk_size; ++i){
//			std::cout << data1[i] << ',';
//		}
//		std::cout << "\n--------------" << std::endl << std::flush;
//		MPI_Barrier(MPI_COMM_WORLD);
//	} else if (rank == 1){
//		MPI_Barrier(MPI_COMM_WORLD);
//		std::cout << "\nrank " << rank << " data0:" << std::endl;
//		for (int i = 0; i < chunk_size; ++i){
//			std::cout << data0[i] << ',';
//		}
//		std::cout << "\ndata1:"  << std::endl;
//		for (int i = 0; i < chunk_size; ++i){
//			std::cout << data1[i] << ',';
//		}
//		std::cout << "\n++++++++++++++++++=" << std::endl << std::flush;
//	}
//	else{
//		MPI_Barrier(MPI_COMM_WORLD);
//	}

	file0.chunk_write_all(chunk0);
	file0.chunk_write_all(chunk1);

	//initialize and write the index
	index_val_t index0[num_blocks];
	std::fill(index0, index0 + num_blocks, 0);
	index0[rank] = offset0;
	index0[nprocs+rank] = offset1;
	file0.write_index(index0, num_blocks);

//	file0.sync();
//	//try reading the file as one big chunk
//	if (rank==0){
//		size_t num_doubles = chunk_size * num_blocks;
//	double * datas = new double[num_doubles];
//	MPI_Offset file_start =0;
//	file0.read_doubles(datas, num_doubles, file_start);
//	std::cout << std::endl << "printing data part of file" << std::endl;
//	for (int i = 0; i < num_doubles; ++i){
//		std::cout << datas[i] << ',';
//	}
//	std::cout << std::endl << std::flush;
//	}


	}

	MPI_Barrier(MPI_COMM_WORLD);
	if(rank == 0){
		std::cout << "@@@@@@@@@@@ part one done" << std::endl;

	}
	MPI_Barrier(MPI_COMM_WORLD);
	//file was closed when ArrayFile went out of scope
	//reopen it.
	{

		sip::ArrayFile file1(num_blocks, chunk_size, filename, MPI_COMM_WORLD, false);

		//read the index
		index_val_t index1[num_blocks];
		std::fill(index1, index1 + num_blocks,0);
		file1.read_index(index1, num_blocks);
		ASSERT_EQ(offset0, index1[rank]);
		ASSERT_EQ(offset1, index1[nprocs+rank]);

//		if (rank == 0){
//			std::cout << "\nrank " << rank << " file1 state after reading index:" << file1 << std::endl;
//			std::cout << "index: ";
//			for (int i = 0; i < num_blocks; ++i){
//				std::cout << index1[i] << ',';
//			}
//			MPI_Barrier(MPI_COMM_WORLD);
//		} else if (rank == 1){
//			MPI_Barrier(MPI_COMM_WORLD);
//			std::cout << "\nrank " << rank << " file1 state after reading index:" << file1 << std::endl;
//			std::cout << "index: ";
//			for (int i = 0; i < num_blocks; ++i){
//				std::cout << index1[i] << ',';
//			}
//		} else {
//			MPI_Barrier(MPI_COMM_WORLD);
//		}
//
//		file1.sync();
//		//try reading the file as one big chunk
//		if (rank==0){
//			size_t num_doubles = chunk_size * num_blocks;
//		double * datas = new double[num_doubles];
//		MPI_Offset file_start =0;
//		file1.read_doubles(datas, num_doubles, file_start);
//		std::cout << std::endl << "printing data part of file" << std::endl;
//		for (int i = 0; i < num_doubles; ++i){
//			std::cout << datas[i] << ',';
//		}
//		std::cout << std::endl << std::flush;
//		}

		double datar0[chunk_size];
		double datar1[chunk_size];
//		sip::Chunk chunkr0(datar0, chunk_size, index1[rank], true);
//		sip::Chunk chunkr1(datar1, chunk_size, index1[nprocs+rank], true);
		sip::Chunk chunkr0(datar0, index1[rank], true);
		sip::Chunk chunkr1(datar1, index1[nprocs+rank], true);
		file1.chunk_read_all(chunkr0);
		file1.chunk_read_all(chunkr1);

//		//output the chunk values for ranks 0 and 1
//		if (rank == 0){
//			std::cout << "rank " << rank << " datar0:" << file1 << std::endl;
//			for (int i = 0; i < chunk_size; ++i){
//				std::cout << datar0[i] << ',';
//			}
//			std::cout << " datar1:" << file1 << std::endl;
//			for (int i = 0; i < chunk_size; ++i){
//				std::cout << datar1[i] << ',';
//			}
//			MPI_Barrier(MPI_COMM_WORLD);
//		} else if (rank == 1){
//			MPI_Barrier(MPI_COMM_WORLD);
//			std::cout << "rank " << rank << " datar0:" << file1 << std::endl;
//			for (int i = 0; i < chunk_size; ++i){
//				std::cout << datar0[i] << ',';
//			}
//			std::cout << " datar1:" << file1 << std::endl;
//			for (int i = 0; i < chunk_size; ++i){
//				std::cout << datar1[i] << ',';
//			}
//
//		} else {
//			MPI_Barrier(MPI_COMM_WORLD);
//		}
		for(int i = 0; i != chunk_size; ++i){
			ASSERT_EQ(val0, datar0[i]);
			ASSERT_EQ(val1, datar1[i]);
		}
	}

}

/** Tests collective operations, with extra chunk at rank 0 to test noop commands.*/
TEST(Server,array_file_5){
	std::string filename("array_file_5.test.acf");

	typedef sip::ArrayFile::offset_val_t index_val_t;
	typedef sip::ArrayFile::header_val_t header_val_t;

	int nprocs;
	int rank;
	MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);


	header_val_t chunk_size = 5;
	header_val_t num_blocks = nprocs * 2 + 1;

	MPI_Offset offset0 = rank*chunk_size;
	MPI_Offset offset1 = (nprocs + rank) *chunk_size;
	MPI_Offset offset2 = ((2*nprocs) + rank) * chunk_size;

	//create and write the file
	double val0 = rank;
	double val1 = rank + nprocs;
	double val2 = rank + 2*nprocs;
	{
	sip::ArrayFile file0(num_blocks, chunk_size, filename, MPI_COMM_WORLD, true, true);
	file0.mark_persistent(filename);
	//initialize and write some chunks
	double data0[chunk_size];

	std::fill(data0, data0+chunk_size, val0);

	double data1[chunk_size];
	std::fill(data1, data1+chunk_size, val1);



	for(int i = 0; i != chunk_size; ++i){
		ASSERT_EQ(val0, data0[i]);
		ASSERT_EQ(val1, data1[i]);
	}
//	sip::Chunk chunk0(data0, chunk_size, offset0, false);
//	sip::Chunk chunk1(data1, chunk_size, offset1, false);
	sip::Chunk chunk0(data0, offset0, false);
	sip::Chunk chunk1(data1, offset1, false);


//	//print the chunk values for ranks 0 and 1
//	if (rank == 0){
//		std::cout << "\nrank " << rank << " data0:" << std::endl;
//		for (int i = 0; i < chunk_size; ++i){
//			std::cout << data0[i] << ',';
//		}
//		std::cout << "\ndata1:" << std::endl;
//		for (int i = 0; i < chunk_size; ++i){
//			std::cout << data1[i] << ',';
//		}
//		std::cout << "\n--------------" << std::endl << std::flush;
//		MPI_Barrier(MPI_COMM_WORLD);
//	} else if (rank == 1){
//		MPI_Barrier(MPI_COMM_WORLD);
//		std::cout << "\nrank " << rank << " data0:" << std::endl;
//		for (int i = 0; i < chunk_size; ++i){
//			std::cout << data0[i] << ',';
//		}
//		std::cout << "\ndata1:"  << std::endl;
//		for (int i = 0; i < chunk_size; ++i){
//			std::cout << data1[i] << ',';
//		}
//		std::cout << "\n++++++++++++++++++=" << std::endl << std::flush;
//	}
//	else{
//		MPI_Barrier(MPI_COMM_WORLD);
//	}

	file0.chunk_write_all(chunk0);
	file0.chunk_write_all(chunk1);

	if (rank==0){
		double data2[chunk_size];
		std::fill(data2, data2+chunk_size, val2);
//		sip::Chunk chunk2(data2, chunk_size, offset2, false);
		sip::Chunk chunk2(data2, offset2, false);
		file0.chunk_write_all(chunk2);
	}
	else {
		file0.chunk_write_all_nop();
	}

	//initialize and write the index
	index_val_t index0[num_blocks];
	std::fill(index0, index0 + num_blocks, 0);
	index0[rank] = offset0;
	index0[nprocs+rank] = offset1;
	if (rank == 0){
		index0[nprocs*2 + rank] = offset2;
	}
	file0.write_index(index0, num_blocks);

//	file0.sync();
//	//try reading the file as one big chunk
//	if (rank==0){
//		size_t num_doubles = chunk_size * num_blocks;
//	double * datas = new double[num_doubles];
//	MPI_Offset file_start =0;
//	file0.read_doubles(datas, num_doubles, file_start);
//	std::cout << std::endl << "printing data part of file" << std::endl;
//	for (int i = 0; i < num_doubles; ++i){
//		std::cout << datas[i] << ',';
//	}
//	std::cout << std::endl << std::flush;
//	}


	}

	MPI_Barrier(MPI_COMM_WORLD);
	if(rank == 0){
		std::cout << "@@@@@@@@@@@ part one done" << std::endl;

	}
	MPI_Barrier(MPI_COMM_WORLD);
	//file was closed when ArrayFile went out of scope
	//reopen it.
	{

		sip::ArrayFile file1(num_blocks, chunk_size, filename, MPI_COMM_WORLD, false);

		//read the index
		index_val_t index1[num_blocks];
		std::fill(index1, index1 + num_blocks,0);
		file1.read_index(index1, num_blocks);
		ASSERT_EQ(offset0, index1[rank]);
		ASSERT_EQ(offset1, index1[nprocs+rank]);

//		if (rank == 0){
//			std::cout << "\nrank " << rank << " file1 state after reading index:" << file1 << std::endl;
//			std::cout << "index: ";
//			for (int i = 0; i < num_blocks; ++i){
//				std::cout << index1[i] << ',';
//			}
//			MPI_Barrier(MPI_COMM_WORLD);
//		} else if (rank == 1){
//			MPI_Barrier(MPI_COMM_WORLD);
//			std::cout << "\nrank " << rank << " file1 state after reading index:" << file1 << std::endl;
//			std::cout << "index: ";
//			for (int i = 0; i < num_blocks; ++i){
//				std::cout << index1[i] << ',';
//			}
//		} else {
//			MPI_Barrier(MPI_COMM_WORLD);
//		}
//
//		file1.sync();
//		//try reading the file as one big chunk
//		if (rank==0){
//			size_t num_doubles = chunk_size * num_blocks;
//		double * datas = new double[num_doubles];
//		MPI_Offset file_start =0;
//		file1.read_doubles(datas, num_doubles, file_start);
//		std::cout << std::endl << "printing data part of file" << std::endl;
//		for (int i = 0; i < num_doubles; ++i){
//			std::cout << datas[i] << ',';
//		}
//		std::cout << std::endl << std::flush;
//		}

		double datar0[chunk_size];
		double datar1[chunk_size];
//		sip::Chunk chunkr0(datar0, chunk_size, index1[rank], true);
//		sip::Chunk chunkr1(datar1, chunk_size, index1[nprocs+rank], true);
		sip::Chunk chunkr0(datar0, index1[rank], true);
		sip::Chunk chunkr1(datar1, index1[nprocs+rank], true);
		file1.chunk_read_all(chunkr0);
		file1.chunk_read_all(chunkr1);
		if(rank == 0){
			double datar2[chunk_size];
//			sip::Chunk chunkr2(datar2, chunk_size, index1[nprocs*2 + rank], true);
			sip::Chunk chunkr2(datar2, index1[nprocs*2 + rank], true);
			file1.chunk_read_all(chunkr2);
			for(int i = 0; i != chunk_size; ++i){
				ASSERT_EQ(val0, datar0[i]);
				ASSERT_EQ(val1, datar1[i]);
				ASSERT_EQ(val2, datar2[i]);
			}
		}
		else {
			file1.chunk_read_all_nop();
			for(int i = 0; i != chunk_size; ++i){
				ASSERT_EQ(val0, datar0[i]);
				ASSERT_EQ(val1, datar1[i]);
			}
		}

//		//output the chunk values for ranks 0 and 1
//		if (rank == 0){
//			std::cout << "rank " << rank << " datar0:" << file1 << std::endl;
//			for (int i = 0; i < chunk_size; ++i){
//				std::cout << datar0[i] << ',';
//			}
//			std::cout << " datar1:" << file1 << std::endl;
//			for (int i = 0; i < chunk_size; ++i){
//				std::cout << datar1[i] << ',';
//			}
//			MPI_Barrier(MPI_COMM_WORLD);
//		} else if (rank == 1){
//			MPI_Barrier(MPI_COMM_WORLD);
//			std::cout << "rank " << rank << " datar0:" << file1 << std::endl;
//			for (int i = 0; i < chunk_size; ++i){
//				std::cout << datar0[i] << ',';
//			}
//			std::cout << " datar1:" << file1 << std::endl;
//			for (int i = 0; i < chunk_size; ++i){
//				std::cout << datar1[i] << ',';
//			}
//
//		} else {
//			MPI_Barrier(MPI_COMM_WORLD);
//		}

	}

}

/**
 * Creates chunks using ChunkManager class and writes to file using non-collective ops*/
TEST(Server,array_file_6){
	std::string filename("array_file_6.test.acf");

	typedef sip::ArrayFile::offset_val_t offset_val_t;
	typedef sip::ArrayFile::header_val_t header_val_t;

	int nprocs;
	int rank;
	MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);




	header_val_t chunk_size = 10;
	header_val_t num_blocks = nprocs * 6;

	sip::ArrayFile file0(num_blocks, chunk_size, filename, MPI_COMM_WORLD, true, true);
	sip::ChunkManager chunk_manager(chunk_size, &file0);

	int block_size[] = {5,5,4,4,7,5};
	double val[6];


	{

	//allocate and initialize data as if for serverblocks and write chunks to disk
	int chunk_number[6];
	offset_val_t offset[6];

	sip::ArrayFile file0(num_blocks, chunk_size, filename, MPI_COMM_WORLD, true);
	file0.mark_persistent(filename);

	for (int i = 0; i < 6; i++){
	    val[i] = rank + i*nprocs + 500;;
	    chunk_manager.assign_block_data_from_chunk(block_size[i], true, chunk_number[i], offset[i]);
	    double* data = chunk_manager.get_data(chunk_number[i], offset[i]);
	    std::fill(data, data + block_size[i], val[i]);
	    sip::Chunk* chunk = chunk_manager.chunk(chunk_number[i]);
	    file0.chunk_write(*chunk);  //same chunk may be written more than once by this proc.
	    chunk_manager.set_valid_on_disk(chunk_number[i],true);
	}
	std::cout << "offsets: " << std::endl;
	for (int i = 0; i < 6; ++i){
		std::cout <<  offset[i] << ',';
	}
	std::cout << "\nchunk_numbers: " << std::endl;
	for (int i = 0; i < 6; ++i){
		std::cout <<  chunk_number[i] << ',';
	}
	std::cout << std::endl << std::flush;

//	//construct local index array
//	offset_val_t index[num_blocks];
//	std::fill(index, index + num_blocks, 0);
//	for (int i = rank; i < num_blocks; i+= nprocs){
//		index[i] = offset[i];
//	}
//	file0.write_index(index, num_blocks);

//	//print the chunk managers for ranks 0 and 1
//	if (rank == 0){
//		std::cout << "\nrank " << rank <<  std::endl;
//		std::cout << chunk_manager;
//		std::cout << "\n--------------" << std::endl << std::flush;
//		MPI_Barrier(MPI_COMM_WORLD);
//	} else if (rank == 1){
//		MPI_Barrier(MPI_COMM_WORLD);
//		std::cout << "\nrank " << rank <<  std::endl;
//		std::cout << chunk_manager;
//		std::cout << "\n++++++++++++++++++=" << std::endl << std::flush;
//	}
//	else{
//		MPI_Barrier(MPI_COMM_WORLD);
//	}

	file0.sync();
	//delete all the data arrays, then restore them.

	size_t deleted = chunk_manager.delete_chunk_data_all();
	std::cout << std::flush;


	std::cout << "Number of doubles deleted at rank " <<  rank << ": " << deleted << std::endl << std::flush;


	std::cerr << "starting loop to read in data" << std::endl << std::flush;
	//now get the data for each block, which will require reading it from the file,
	size_t reallocated = 0;
	for (int i = 0; i < 6; i++){
		double * data = chunk_manager.get_data(chunk_number[i], offset[i]);
		if (data == NULL){
		    sip::Chunk* chunk = chunk_manager.chunk(chunk_number[i]);
		    reallocated += chunk_manager.reallocate_chunk_data(chunk);
		    std::cout << "getting ready to read chunk " << chunk_number[i] << ":  " << chunk << std::endl << std::flush;
		    file0.chunk_read(*chunk);
			data = chunk_manager.get_data(chunk_number[i], offset[i]);

		}
		std::cout << "data[0] for block " << i << "="<< data[0] << std::endl;
		for (int k = 0; k < block_size[i]; ++k){
			ASSERT_EQ(val[i], data[k]);
		}

	}
	std::cout << "Number of doubles reallocated " <<  reallocated << std::endl << std::flush;


	std::cerr << "finished loop to read in data" << std::endl << std::flush;




	MPI_Barrier(MPI_COMM_WORLD);
	chunk_manager.delete_chunk_data_all();

	}



} //end of test Server.array_file_6


/**
 * Creates chunks using ChunkManager class, sets and restores persistent */
TEST(Server,array_file_7){
	std::string filename("array_file_7");

	typedef sip::ArrayFile::offset_val_t offset_val_t;
	typedef sip::ArrayFile::header_val_t header_val_t;

	int nprocs;
	int rank;
	MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);


	bool new_file = true;
	bool save_after_close = true;


	header_val_t chunk_size = 10;
	header_val_t num_blocks = nprocs * 6;

	sip::ArrayFile file0(num_blocks, chunk_size, filename, MPI_COMM_WORLD, new_file, save_after_close);
	file0.mark_persistent("file0_P_");
	sip::ChunkManager chunk_manager(chunk_size, &file0);

	int block_size[] = {5,5,4,4,7,5};
	double val[6];


	{

	//allocate and initialize data as if for serverblocks and write chunks to disk
	int chunk_number[6];
	offset_val_t offset[6];


	for (int i = 0; i < 6; i++){
	    val[i] = rank + i*nprocs + 500;;
	    chunk_manager.assign_block_data_from_chunk(block_size[i], true, chunk_number[i], offset[i]);
	    double* data = chunk_manager.get_data(chunk_number[i], offset[i]);
	    std::fill(data, data + block_size[i], val[i]);
	}
	std::cout << "offsets: " << std::endl;
	for (int i = 0; i < 6; ++i){
		std::cout <<  offset[i] << ',';
	}
	std::cout << "\nchunk_numbers: " << std::endl;
	for (int i = 0; i < 6; ++i){
		std::cout <<  chunk_number[i] << ',';
	}
	std::cout << std::endl << std::flush;


	chunk_manager.collective_flush();

//	//construct local index array
//	offset_val_t index[num_blocks];
//	std::fill(index, index + num_blocks, 0);
//	for (int i = rank; i < num_blocks; i+= nprocs){
//		index[i] = offset[i];
//	}
//	file0.write_index(index, num_blocks);

//	//print the chunk managers for ranks 0 and 1
//	if (rank == 0){
//		std::cout << "\nrank " << rank <<  std::endl;
//		std::cout << chunk_manager;
//		std::cout << "\n--------------" << std::endl << std::flush;
//		MPI_Barrier(MPI_COMM_WORLD);
//	} else if (rank == 1){
//		MPI_Barrier(MPI_COMM_WORLD);
//		std::cout << "\nrank " << rank <<  std::endl;
//		std::cout << chunk_manager;
//		std::cout << "\n++++++++++++++++++=" << std::endl << std::flush;
//	}
//	else{
//		MPI_Barrier(MPI_COMM_WORLD);
//	}

	file0.sync();
	//delete all the data arrays, then restore them.

	size_t deleted = chunk_manager.delete_chunk_data_all();
	std::cout << std::flush;


	std::cout << "Number of doubles deleted at rank " <<  rank << ": " << deleted << std::endl << std::flush;


	std::cerr << "starting loop to read in data" << std::endl << std::flush;
	//now get the data for each block, which will require reading it from the file,
	size_t reallocated = 0;
	for (int i = 0; i < 6; i++){
		double * data = chunk_manager.get_data(chunk_number[i], offset[i]);
		if (data == NULL){
		    sip::Chunk* chunk = chunk_manager.chunk(chunk_number[i]);
		    reallocated += chunk_manager.reallocate_chunk_data(chunk);
		    std::cout << "getting ready to read chunk " << chunk_number[i] << ":  " << chunk << std::endl << std::flush;
		    file0.chunk_read(*chunk);
			data = chunk_manager.get_data(chunk_number[i], offset[i]);

		}
		std::cout << "data[0] for block " << i << "="<< data[0] << std::endl;
		for (int k = 0; k < block_size[i]; ++k){
			ASSERT_EQ(val[i], data[k]);
		}

	}
	std::cout << "Number of doubles reallocated " <<  reallocated << std::endl << std::flush;


	std::cerr << "finished loop to read in data" << std::endl << std::flush;




	MPI_Barrier(MPI_COMM_WORLD);
	chunk_manager.delete_chunk_data_all();

	}



} //end of test Server.array_file_7

/**
 * Creates chunks using ChunkManager class and writes to file using flush */
TEST(Server,DISABLED_array_file_8){
	std::string filename("array_file_8.test.acf");

	typedef sip::ArrayFile::offset_val_t offset_val_t;
	typedef sip::ArrayFile::header_val_t header_val_t;

	int nprocs;
	int rank;
	MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);




	header_val_t chunk_size = 10;
	header_val_t num_blocks = nprocs * 6;

	sip::ArrayFile file0(num_blocks, chunk_size, filename, MPI_COMM_WORLD, true, true);
	sip::ChunkManager chunk_manager(chunk_size, &file0);

	int block_size[] = {5,5,4,4,7,5};
	double val[6];


	{

	//allocate and initialize data as if for serverblocks and write chunks to disk
	int chunk_number[6];
	offset_val_t offset[6];

	sip::ArrayFile file0(num_blocks, chunk_size, filename, MPI_COMM_WORLD, true);

	for (int i = 0; i < 6; i++){
	    val[i] = rank + i*nprocs + 500;;
	    chunk_manager.assign_block_data_from_chunk(block_size[i], true, chunk_number[i], offset[i]);
	    double* data = chunk_manager.get_data(chunk_number[i], offset[i]);
	    std::fill(data, data + block_size[i], val[i]);
	}
	std::cout << "offsets: " << std::endl;
	for (int i = 0; i < 6; ++i){
		std::cout <<  offset[i] << ',';
	}
	std::cout << "\nchunk_numbers: " << std::endl;
	for (int i = 0; i < 6; ++i){
		std::cout <<  chunk_number[i] << ',';
	}
	std::cout << std::endl << std::flush;

	std::string label("persistent_array_file_8");
	file0.mark_persistent(label);
	chunk_manager.collective_flush();

//	//construct local index array
//	offset_val_t index[num_blocks];
//	std::fill(index, index + num_blocks, 0);
//	for (int i = rank; i < num_blocks; i+= nprocs){
//		index[i] = offset[i];
//	}
//	file0.write_index(index, num_blocks);

//	//print the chunk managers for ranks 0 and 1
//	if (rank == 0){
//		std::cout << "\nrank " << rank <<  std::endl;
//		std::cout << chunk_manager;
//		std::cout << "\n--------------" << std::endl << std::flush;
//		MPI_Barrier(MPI_COMM_WORLD);
//	} else if (rank == 1){
//		MPI_Barrier(MPI_COMM_WORLD);
//		std::cout << "\nrank " << rank <<  std::endl;
//		std::cout << chunk_manager;
//		std::cout << "\n++++++++++++++++++=" << std::endl << std::flush;
//	}
//	else{
//		MPI_Barrier(MPI_COMM_WORLD);
//	}

//	file0.sync();
//	//delete all the data arrays, then restore them.
//
//	size_t deleted = chunk_manager.delete_chunk_data_all();
//	std::cout << std::flush;
//
//
//	std::cout << "Number of doubles deleted at rank " <<  rank << ": " << deleted << std::endl << std::flush;
//
//
//	std::cerr << "starting loop to read in data" << std::endl << std::flush;
//	//now get the data for each block, which will require reading it from the file,
//	size_t reallocated = 0;
//	for (int i = 0; i < 6; i++){
//		double * data = chunk_manager.get_data(chunk_number[i], offset[i]);
//		if (data == NULL){
//		    sip::Chunk& chunk = chunk_manager.chunk(chunk_number[i]);
//		    reallocated += chunk_manager.reallocate_chunk_data(chunk);
//		    std::cout << "getting ready to read chunk " << chunk_number[i] << ":  " << chunk << std::endl << std::flush;
//		    file0.chunk_read(chunk);
//			data = chunk_manager.get_data(chunk_number[i], offset[i]);
//
//		}
//		std::cout << "data[0] for block " << i << "="<< data[0] << std::endl;
//		for (int k = 0; k < block_size[i]; ++k){
//			ASSERT_EQ(val[i], data[k]);
//		}
//
//	}
//	std::cout << "Number of doubles reallocated " <<  reallocated << std::endl << std::flush;
//
//
//	std::cerr << "finished loop to read in data" << std::endl << std::flush;
//
//
//
//
//	MPI_Barrier(MPI_COMM_WORLD);
	chunk_manager.delete_chunk_data_all();
//
//	}

	}

} //end of test Server.array_file_8

//	} //end of scope for file writing
//
//	MPI_Barrier(MPI_COMM_WORLD);
//	if(rank == 0){
//		std::cout << "@@@@@@@@@@@ part one done" << std::endl << std::flush;
//
//	}
//	MPI_Barrier(MPI_COMM_WORLD);
//
//
//	//file was closed when ArrayFile went out of scope
//	//reopen it.
//	{
//
//
//		sip::ArrayFile file1(num_blocks, chunk_size, filename, MPI_COMM_WORLD, true);
//		sip::ChunkManager chunk_manager(chunk_size, file1);
//
//
//		//read the index
//		offset_val_t index1[num_blocks];
//		std::fill(index1, index1 + num_blocks,0);
//		file1.read_index(index1, num_blocks);
//
//
//
//		offset_val_t offset0 = chunk_manager.chunk_offset(0);
//		offset_val_t offset1 = chunk_manager.chunk_offset(1);
//
//		ASSERT_EQ(offset0, index1[rank]);
//		ASSERT_EQ(offset1, index1[nprocs+rank]);
//
//		if (rank == 0){
//			std::cout << "\nrank " << rank << " file1 state after reading index:" << file1 << std::endl;
//			std::cout << "index: ";
//			for (int i = 0; i < num_blocks; ++i){
//				std::cout << index1[i] << ',';
//			}
//			MPI_Barrier(MPI_COMM_WORLD);
//		} else if (rank == 1){
//			MPI_Barrier(MPI_COMM_WORLD);
//			std::cout << "\nrank " << rank << " file1 state after reading index:" << file1 << std::endl;
//			std::cout << "index: ";
//			for (int i = 0; i < num_blocks; ++i){
//				std::cout << index1[i] << ',';
//			}
//		} else {
//			MPI_Barrier(MPI_COMM_WORLD);
//		}
////
//		file1.sync();
//		//try reading the file as one big chunk
//		if (rank==0){
//			size_t num_doubles = chunk_size * num_blocks;
//		double * datas = new double[num_doubles];
//		MPI_Offset file_start =0;
//		file1.read_doubles(datas, num_doubles, file_start);
//		std::cout << std::endl << "printing data part of file" << std::endl;
//		for (int i = 0; i < num_doubles; ++i){
//			std::cout << datas[i] << ',';
//		}
//		std::cout << std::endl << std::flush;
//		}
//
//		double datar0[chunk_size];
//		double datar1[chunk_size];
//		sip::Chunk chunkr0(datar0, chunk_size, index1[rank], true);
//		sip::Chunk chunkr1(datar1, chunk_size, index1[nprocs+rank], true);
//		file1.chunk_read_all(chunkr0);
//		file1.chunk_read_all(chunkr1);
//		if(rank == 0){
//			double datar2[chunk_size];
//			sip::Chunk chunkr2(datar2, chunk_size, index1[nprocs*2 + rank], true);
//			file1.chunk_read_all(chunkr2);
//			for(int i = 0; i != chunk_size; ++i){
//				ASSERT_EQ(val0, datar0[i]);
//				ASSERT_EQ(val1, datar1[i]);
//				ASSERT_EQ(val2, datar2[i]);
//			}
//		}
//		else {
//			file1.chunk_read_all_nop();
//			for(int i = 0; i != chunk_size; ++i){
//				ASSERT_EQ(val0, datar0[i]);
//				ASSERT_EQ(val1, datar1[i]);
//			}
//		}
//
////		//output the chunk values for ranks 0 and 1
////		if (rank == 0){
////			std::cout << "rank " << rank << " datar0:" << file1 << std::endl;
////			for (int i = 0; i < chunk_size; ++i){
////				std::cout << datar0[i] << ',';
////			}
////			std::cout << " datar1:" << file1 << std::endl;
////			for (int i = 0; i < chunk_size; ++i){
////				std::cout << datar1[i] << ',';
////			}
////			MPI_Barrier(MPI_COMM_WORLD);
////		} else if (rank == 1){
////			MPI_Barrier(MPI_COMM_WORLD);
////			std::cout << "rank " << rank << " datar0:" << file1 << std::endl;
////			for (int i = 0; i < chunk_size; ++i){
////				std::cout << datar0[i] << ',';
////			}
////			std::cout << " datar1:" << file1 << std::endl;
////			for (int i = 0; i < chunk_size; ++i){
////				std::cout << datar1[i] << ',';
////			}
////
////		} else {
////			MPI_Barrier(MPI_COMM_WORLD);
////		}
//
//	}





int main(int argc, char **argv) {

	MPI_Init(&argc, &argv);

#ifdef HAVE_TAU
	TAU_PROFILE_SET_NODE(0);
	TAU_STATIC_PHASE_START("SIP Main");
#endif

	sip::check(sizeof(int) >= 4, "Size of integer should be 4 bytes or more");
	sip::check(sizeof(double) >= 8, "Size of double should be 8 bytes or more");
	sip::check(sizeof(long long) >= 8, "Size of long long should be 8 bytes or more");

	int num_procs;
	int err = MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
	if (err != MPI_SUCCESS){
		std::cerr << "error obtaining number of procs in test_unit.cpp:main" << std::endl;
		MPI_Abort(MPI_COMM_WORLD, 55);
	}


	MPI_Errhandler_set(MPI_COMM_WORLD, MPI_ERRORS_RETURN);

	printf("Running main() from master_test_main.cpp\n");
	testing::InitGoogleTest(&argc, argv);
	int result = RUN_ALL_TESTS();

#ifdef HAVE_TAU
	TAU_STATIC_PHASE_STOP("SIP Main");
#endif

	 MPI_Finalize();

	return result;

}
