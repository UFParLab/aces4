/*
 * array_file.h
 *
 *  Created on: Aug 10, 2015
 *      Author: basbas
 */

#ifndef ARRAY_FILE_H_
#define ARRAY_FILE_H_

#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <limits>
#include <mpi.h>
#include "sip.h"
#include "per_array_chunk_list.h"
namespace sip {

/** Encapsulates file IO for server disk backing and persistence
 *
 * The file format is
 *
 * <file> ::= <header> <index> <data>
 *
 *
 * <header> ::= <size_t num_blocks> <size_t chunk_size>
 * <index> ::=  <MPI_Offset><MPI_Offset>*  where the length of this list is num_blocks
 * <data> ::= <double>*
 *
 * Each server is allocated disk space in chunk_size sized chunks (chunk_size is in units of
 * double) in a round robin fashion.
 *
 * Invariant:  file view is set for reading data.  Thus methods that change the view must
 * reset it to the view for data.  Since read or write header is called in the constructor,
 * the invariant is established by the constructor.
 *
 */
class ArrayFile {
public:
	/**
	 * These definitions define the type of the values in the header.  The C++ type must
	 * match the MPI type.
	 */
	const static int NUM_VALS_IN_HEADER = 2;
	typedef int header_val_t;
#define  MPI_HEADER_VAL_T MPI_INT

/** these definitions define the types of values in the index.
 * The c++ type must match the mpi type.
 */
	typedef int index_val_t;
#define  MPI_INDEX_VAL_T  MPI_INT

	/**
	 * Opens the file.
	 *
	 * If new_file is true, a new file is created (or the old one will be truncated).
	 * If new_file is false, the file must exist.  The header is read by rank 0
	 * in the given communicator and broadcast to other processes.
	 *
	 * This is a collective operation among the given communicator.
	 *
	 * @param num_blocks
	 * @param chunk_size
	 * @param file_name
	 * @param comm
	 * @param new_file
	 *
	 * If new_file is true, a new file will be created, truncating the existing file.
	 * If not, a file with this name must exist.
	 *
	 * TODO add more info about array to header and check when reopened.
	 */

	ArrayFile(header_val_t num_blocks, header_val_t chunk_size, const std::string& file_name,
			const MPI_Comm& comm, bool new_file) :
			num_blocks_(num_blocks), chunk_size_(chunk_size), is_persistent_(
					false), label_(std::string("")), comm_(comm) {

		check_implementation_limits();


		if (new_file){
			//open file, creating if necessary.
			//write header with passed in info
		int err = MPI_File_open(comm_, const_cast<char *>(file_name.c_str()),
				 MPI_MODE_CREATE | MPI_MODE_RDWR, MPI_INFO_NULL,
				&fh_);
		check(err == MPI_SUCCESS, "failure opening new array file at server");

		write_header(); //currently, the header contains the number of blocks and chunk size

		}
		else {
			//file must exist.  Read and broadcast header and check that values are as expected.
			int err = MPI_File_open(comm_, const_cast<char *>(file_name.c_str()),
					 MPI_MODE_RDWR, MPI_INFO_NULL,
					&fh_);
			check(err == MPI_SUCCESS, "failure opening existing array file at server");
			read_header();
			check(num_blocks == num_blocks_, "number of blocks in reopened file is different than expected");
			check(chunk_size == chunk_size_, "chunk size in reopened file is different than expected");
		}

	}

	/**
	 * Closes the file. If it has not been marked persistent, it deletes it.
	 * If persistent, calls flush and renames it.
	 */
	~ArrayFile() {
		MPI_File_close(&fh_);
	}


	/**
	 * This is a collective operation to set the view to write unsigned values.
	 * Then the communicator's master writes it.
	 * Followed by a collective sync.
	 */
	void write_header() {
//			{//for gdb
//			    int i = 0;
//			    char hostname[256];
//			    gethostname(hostname, sizeof(hostname));
//			    printf("PID %d on %s ready for attach\n", getpid(), hostname);
//			    fflush(stdout);
//			    while (0 == i)
//			        sleep(5);
//			}
		int rank;
		MPI_Comm_rank(comm_, &rank);
		int err = MPI_File_set_view(fh_, 0, MPI_INDEX_VAL_T, MPI_INDEX_VAL_T,
				"native", MPI_INFO_NULL);
		if (rank == 0) {
			header_val_t header[NUM_VALS_IN_HEADER];
			header[0] = num_blocks_;
			header[1] = chunk_size_;
			int err = MPI_File_write_at(fh_, 0, header,
					NUM_VALS_IN_HEADER, MPI_INDEX_VAL_T, MPI_STATUS_IGNORE);
			check(err == MPI_SUCCESS, "failure writing array file header");
		}
		set_view_for_data();
	}

	void read_header() {
		int rank;
		MPI_Comm_rank(comm_, &rank);
		int err = MPI_File_set_view(fh_, 0, MPI_INDEX_VAL_T, MPI_INDEX_VAL_T,
				"native", MPI_INFO_NULL);
		header_val_t header[NUM_VALS_IN_HEADER];
		if (rank == 0) {
			int err = MPI_File_read_at(fh_, 0, header,
					NUM_VALS_IN_HEADER, MPI_INDEX_VAL_T, MPI_STATUS_IGNORE);
			check(err == MPI_SUCCESS, "failure reading array file header");
		}
		err = MPI_Bcast(header, NUM_VALS_IN_HEADER, MPI_INDEX_VAL_T, 0,
				comm_);
		check(err == MPI_SUCCESS, "header broadcast failed");
		num_blocks_ = header[0];
		chunk_size_ = header[1];
		set_view_for_data();
	}

	/**
	 * Causes failure if the number of blocks or chunk size exceed the current implementation limits.
	 */
	void check_implementation_limits() {
		check(
				num_blocks_
						< std::numeric_limits<index_val_t>::max() / chunk_size_,
				"implementation limit encountered:  type used to represent offsets in the index file may not be big enough");
		check(num_blocks_ < std::numeric_limits<int>::max(),
				"implemenation limit enountered:  number of blocks too large for int used when writing index file");
	}

	void set_view_for_data() {
		//displacement must be in bytes
		MPI_Offset displacement = (NUM_VALS_IN_HEADER * sizeof(header_val_t))
				+ (num_blocks_ * sizeof(index_val_t));
		//TODO add padding for alignment?
		int err = MPI_File_set_view(fh_, displacement, MPI_DOUBLE, MPI_DOUBLE,
				"native", MPI_INFO_NULL);
		check(err == MPI_SUCCESS, "setting view to write data failed");
	}

	/**
	 * Writes the given chunk to disk.
	 *
	 * This is not a collective operation.
	 *
	 * @param chunk
	 */
	void chunk_write_(Chunk & chunk) {
		MPI_Offset offset = chunk.file_offset_;
		MPI_Status status;
		int err = MPI_File_write_at(fh_, offset, chunk.data_, chunk_size_,
				MPI_DOUBLE, &status);
		check(err == MPI_SUCCESS, "write_chunk failed");
	}



	/**
	 * Write the given chunk to disk.
	 *
	 * This is a collective operation
	 *
	 * @param chunk
	 */
	void chunk_write_all(Chunk & chunk) {
		MPI_Offset offset = chunk.file_offset_;
		MPI_Status status;
		int err = MPI_File_write_at_all(fh_, offset, chunk.data_, chunk_size_,
				MPI_DOUBLE, &status);
		check(err == MPI_SUCCESS, "chunk_write_all write failed");
	}


	void chunk_write_all_nop() {
		MPI_Status status;
		int err = MPI_File_write_at_all(fh_, 0, NULL, 0, MPI_DOUBLE, &status);
		check(err == MPI_SUCCESS, "chunk_write_all_nop failed");
	}

	/**
	 * Reads the data for the given chunk from disk
	 *
	 * This is not a collective operation
	 *
	 * @param chunk
	 */
	void chunk_read(Chunk& chunk) {
		MPI_Offset offset = chunk.file_offset_;
		MPI_Status status;
		int err = MPI_File_read_at(fh_, offset, chunk.data_, chunk_size_,
				MPI_DOUBLE, &status);
		check(err == MPI_SUCCESS, "chunk_read failed");
	}

	/**
	 * Reads the data for the given chunk from disk
	 *
	 * This is a collective operation
	 *
	 * @param chunk
	 */
	void chunk_read_all(Chunk & chunk) {
		MPI_Offset offset = chunk.file_offset_;
		MPI_Status status;
		int err = MPI_File_read_at_all(fh_, offset, chunk.data_, chunk_size_,
				MPI_DOUBLE, &status);
		check(err == MPI_SUCCESS, "chunk_read_all failed");
	}

	/**
	 * Does nothing except participate in a collective read operation
	 */
	void chunk_read_all_nop() {
		MPI_Status status;
		int err = MPI_File_read_at_all(fh_, 0, NULL, 0, MPI_DOUBLE, &status);
		check(err == MPI_SUCCESS, "chunk_read_all_nop failed");
	}

	/**
	 * Takes an index containing the offsets for locally owned blocks (with value of 0 for
	 * blocks that have not been created by this server).
	 *
	 * Gathers the result
	 *
	 * The master server writes the index to the file. It is a collective operation since
	 * all procs will reset the view.
	 *
	 * @param index
	 */
	void write_index(index_val_t* index, header_val_t size) {
		check(size == num_blocks_, "array passed to write_index may not have correct size");
		int rank;
		MPI_Comm_rank(comm_, &rank);
		MPI_Offset displacement = NUM_VALS_IN_HEADER * sizeof(header_val_t);
		int err = MPI_File_set_view(fh_, displacement, MPI_INDEX_VAL_T,
				MPI_INDEX_VAL_T, "native", MPI_INFO_NULL);
		check(err == MPI_SUCCESS, "setting view to write index failed");
		MPI_Status status;
		if (rank == 0) {
			int err = MPI_Reduce(MPI_IN_PLACE, index, num_blocks_,
					MPI_INDEX_VAL_T, MPI_SUM, 0, comm_);
			check(err == MPI_SUCCESS, "failure reducing file index at root");

			err = MPI_File_write_at(fh_, 0, index, num_blocks_,
					MPI_INDEX_VAL_T, &status);
			check(err == MPI_SUCCESS, "failure writing file index");
		} else {
			int err = MPI_Reduce(index, NULL, num_blocks_, MPI_INDEX_VAL_T,
					MPI_SUM, 0, comm_);
			check(err == MPI_SUCCESS, "failure reducing file index at root");
		}
		set_view_for_data();
	}

	/**
	 * Reads the index of an array file and broadcast to other servers.
	 *
	 * Preconditions:
	 *
	 * the header has already been read, so num_blocks_ and chunk_size_ are
	 * initialized.
	 *
	 * index has been allocated.
	 *
	 *This is a collective operation
	 *
	 * @param index
	 */
	void read_index(index_val_t* index, header_val_t size) {
		check(size == num_blocks_, "array passed to read_index may not have correct size");
		int rank;
		MPI_Comm_rank(comm_, &rank);
		MPI_Offset displacement = NUM_VALS_IN_HEADER * sizeof(header_val_t);
		int err = MPI_File_set_view(fh_, displacement, MPI_INDEX_VAL_T,
				MPI_INDEX_VAL_T, "native", MPI_INFO_NULL);
		MPI_Status status;
		if (rank == 0) {
			err = MPI_File_read_at(fh_, 0, index, num_blocks_,
					MPI_INDEX_VAL_T, &status);
			check(err == MPI_SUCCESS, "failure reading file index");
		}
			err = MPI_Bcast(index, num_blocks_, MPI_INDEX_VAL_T, 0,
					comm_);
			check(err == MPI_SUCCESS, "index broadcast failed");
			set_view_for_data();
	}

	void set_persistent(const std::string& label) {
		is_persistent_ = true;
		label_ = label;
	}

	//read size values into the given buffer starting at offset doubles relative to the data view
	void read_doubles(double * data, size_t size, MPI_Offset offset){
		MPI_Status status;
		int err = MPI_File_read_at(fh_, offset, data, size,
				MPI_DOUBLE, &status);
		check(err == MPI_SUCCESS, "chunk_read_all failed");
	}

	void sync(){
		MPI_File_sync(fh_);
	}

	friend std::ostream& operator<<(std::ostream& os, const ArrayFile& obj);

private:
	MPI_File fh_;
	header_val_t num_blocks_;
	header_val_t chunk_size_;
	std::string name_;
	bool is_persistent_;
	std::string label_;
	const MPI_Comm& comm_;

	DISALLOW_COPY_AND_ASSIGN(ArrayFile);
};

} /* namespace sip */

#endif /* ARRAY_FILE_H_ */
