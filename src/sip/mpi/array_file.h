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
 * double) in a round robin fashion. These chunks map into allocated memory.
 *
 */
class ArrayFile {
public:

	/**
	 * Opens a file using MPI_IO.
	 *
	 * This is a collective operation among the given communicator.
	 *
	 * @param num_blocks
	 * @param chunk_size
	 * @param file_name
	 * @param comm
	 */

	ArrayFile(size_t num_blocks, size_t chunk_size, std::string file_name,
			const MPI_Comm& comm) :
			num_blocks_(num_blocks), chunk_size_(chunk_size), is_persistent_(
					false), label_(std::string("")), comm_(comm) {

		check_implementation_limits();

		//open file
		int err = MPI_File_open(comm_, file_name,
				MPI_MODE_EXCL | MPI_MODE_CREATE | MPI_MODE_RDWR, MPI_INFO_NULL,
				&fh_);
		check(err == MPI_SUCCESS, "failure opening array file at server");

		write_header(); //currently, the header contains the number of blocks and chunk size

		set_view_for_data(); //leave file in state to read and write chunks
	}

	/**
	 * Closes the file. If it has not been marked persistent, it deletes it.
	 * If persistent, calls flush and renames it.
	 */
	~ArrayFile() {
		MPI_File_close(fh_);
	}

	/**
	 * These constants define the type of the values in the header.  The C++ types must
	 * be compatible with the MPI type.
	 */
	const static int NUM_VALS_IN_HEADER = 2;
	typedef unsigned long header_val_t;
	typedef unsigned long index_val_t;
#define MPI_UNSIGNED_LONG MPI_HEADER_TYPE
#define MPI_UNSIGNED_LONG MPI_INDEX_TYPE
	typedef std::vector<index_val_t> index_t;
	/**
	 * This is a collective operation to set the view to write unsigned values.
	 * Then the communicator's master writes it.
	 * Followed by a collective sync.
	 */
	void write_header() {
		int rank;
		MPI_Comm_rank(comm_, &rank);
		int err = MPI_File_set_view(fh_, 0, MPI_HEADER_TYPE, MPI_HEADER_TYPE,
				"native", MPI_INFO_NULL);
		if (rank == 0) {
			std::vector<header_val_t> header(NUM_VALS_IN_HEADER);
			header.push_back(num_blocks_);
			header.push_back(chunk_size_);
			int err = MPI_File_write_at(fh_, 0, header.data(),
					NUM_VALS_IN_HEADER, MPI_HEADER_TYPE, MPI_STATUS_IGNORE);
			check(err == MPI_SUCCESS, "failure writing array file header");
		}
		MPI_File_sync (fh);
	}

	void read_header() {
		int rank;
		MPI_Comm_rank(comm_, &rank);
		int err = MPI_File_set_view(fh_, 0, MPI_HEADER_TYPE, MPI_HEADER_TYPE,
				"native", MPI_INFO_NULL);
		std::vector<size_t> header(NUM_VALS_IN_HEADER);
		if (rank == 0) {
			int err = MPI_File_read_at(fh_, 0, header.data(),
					NUM_VALS_IN_HEADER, MPI_HEADER_TYPE, MPI_STATUS_IGNORE);
			check(err == MPI_SUCCESS, "failure reading array file header");
		}
		MPI_File_sync (fh);
		err = MPI_Bcast(header.data(), NUM_VALS_IN_HEADER, MPI_HEADER_TYPE, 0,
				comm_);
		check(err == MPI_SUCCESS, "header broadcast failed");
		num_blocks_ = header.at(0)
		chunk_size_ = header.at(1);
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
		MPI_Offset displacement = NUM_VALS_IN_HEADER * sizeof(header_val_t)
				+ num_blocks_ * sizeof(index_val_t);
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
	void chunk_write_(ChunkListEntry & chunk) {
		MPI_Offset offset = chunk.file_offset_;
		MPI_Status status;
		int err = MPI_File_write_at(fh_, offset, chunk.data_, chunk_size_,
				MPI_DOUBLE, &status);
		check(err == MPI_SUCCESS, "write_chunk_from_disk failed");
	}

	/**
	 * Write the given chunk to disk.
	 *
	 * This is a collective operation
	 *
	 * @param chunk
	 */
	void chunk_write_all(ChunkListEntry & chunk) {
		MPI_Offset offset = chunk.file_offset_;
		MPI_Status status;
		int err = MPI_File_write_at_all(fh_, offset, chunk.data_, chunk_size_,
				MPI_DOUBLE, &status);
		check(err == MPI_SUCCESS, "chunk_write_all write failed");
	}

	void chunk_write_all_nop() {
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
	void chunk_read(ChunkListEntry& chunk) {
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
	void chunk_read_all(ChunkListEntry & chunk) {
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
	void write_index(const index_t& index) {
		int rank;
		MPI_Comm_rank(comm_, &rank);
		check(index.size() == num_blocks_, "problem with size of file index");
		MPI_Offset displacement = NUM_VALS_IN_HEADER * sizeof(header_val_t);
		int err = MPI_File_set_view(fh_, displacement, MPI_INDEX_TYPE,
				MPI_INDEX_TYPE, "native", MPI_INFO_NULL);
		check(err == MPI_SUCCESS, "setting view to write index failed");

		if (rank == 0) {
			int err = MPI_Reduce(MPI_IN_PLACE, index.data(), num_blocks,
					MPI_INDEX_TYPE, MPI + SUM, 0, comm_);
			check(err == MPI_SUCCESS, "failure reducing file index at root");
			MPI_Status status;
			int err = MPI_File_write_at(fh_, 0, index_.data(), num_blocks_,
					MPI_INDEX_TYPE, &status);
			check(err == MPI_SUCCESS, "failure writing file index");
		} else {
			int err = MPI_Reduce(index.data(), NULL, num_blocks, MPI_INDEX_TYPE,
					MPI + SUM, 0, comm_);
			check(err == MPI_SUCCESS, "failure reducing file index at root");
		}
	}

	/**
	 * Reads the index of an array file and broadcast to other servers.
	 *
	 * Precondition:  the header has already been read, so num_blocks_ and chunk_size_ are
	 * initialized.
	 *
	 * @param index
	 */
	void read_index(index_t& index) {
		index.resize(num_blocks_);
		int rank;
		MPI_Comm_rank(comm_, &rank);
		MPI_Offset displacement = NUM_VALS_IN_HEADER * sizeof(header_val_t);
		int err = MPI_File_set_view(fh_, displacement, MPI_INDEX_TYPE,
				MPI_INDEX_TYPE, "native", MPI_INFO_NULL);
		if (rank == 0) {
			int err = MPI_File_read_at(fh_, 0, index_.data(), num_blocks_,
					MPI_INDEX_TYPE, &status);
			check(err == MPI_SUCCESS, "failure reading file index");
			err = MPI_Bcast(index.data(), num_blocks_, MPI_INDEX_TYPE, 0,
					comm_);
			check(err == MPI_SUCCESS, "index broadcast failed at root");
		} else {
			int err = MPI_Bcast(index.data(), num_blocks_, MPI_INDEX_TYPE, 0,
					comm_);
			check(err == MPI_SUCCESS, "index broadcast failed at non-root");
		}
	}

	void set_persistent(const std::string& label) {
		set_persistent_ = true;
		label_ = label;
	}

	friend std::ostream& operator<<(std::ostream& os, const ArrayFile& obj);

private:
	MPI_File fh_;
	size_t num_blocks_;
	size_t chunk_size_;
	std::string name_;
	bool is_persistent_;
	std::string label_;
	const MPI_Comm& comm_;

	const static int MAX_FILE_NAME_SIZE = 256;

	DISALLOW_COPY_AND_ASSIGN(ArrayFile);
};

} /* namespace sip */

#endif /* ARRAY_FILE_H_ */
