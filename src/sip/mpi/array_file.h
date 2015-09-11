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
#include <sstream>
#include <mpi.h>
#include "sip.h"
#include "chunk.h"
#include "data_distribution.h"

namespace sip {

class Chunk;
class ChunkManager;
/** Encapsulates file IO for server disk backing and persistence.
 *
 * Responsible for converting array names and persistent array labels to file names.
 *
 * The file format is
 *
 * <file> ::= <header> <index> <data>
 *
 *
 * <header> ::= <header_val_t num_blocks> <header_val_t chunk_size> <header_val_t num_servers>
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

	/** these definitions define the types of values in the header and index.
	 *
	 * Note header_val_t and index_val_t are defined to be c++ types,
	 * as is MPI_Offset  The latter's size is supposed to be large enough to handle
	 * the largest files supported on the given system.
	 *
	 * In contrast MPI_OFFSET_VAL_T and MPI_HEADER_VAL_T are MPI_Datatype (and
	 * can be passed as type arguments to MPI routines.)
	 *
	 * The c++ types must match the MPI_Datatype.  For example, if the c++
	 * type is int, the MPI_Datatype should be MPI_INT.
	 *
	 * WARNING:  MPI_Offset is implementation/configuration specific.
	 * TODO fix more generally
	 */

	typedef int header_val_t;
#define  MPI_HEADER_VAL_T MPI_INT

	typedef MPI_Offset offset_val_t;
#define  MPI_OFFSET_VAL_T  MPI_LONG_LONG

	const static int NUM_VALS_IN_HEADER = 3;
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
	 *
	 * TODO add more info about array to header and check when reopened.
	 */

	ArrayFile(header_val_t num_blocks, header_val_t chunk_size, const std::string& name,
			const MPI_Comm& comm, bool new_file, bool save_after_close = false) :
			num_blocks_(num_blocks), chunk_size_(chunk_size), name_(name), is_persistent_(
					false), save_after_close_(save_after_close), label_(std::string(name)), comm_(comm) {

		check_implementation_limits();


		if (new_file){
			//open file, creating if necessary.
			//write header with passed in info
		int err = MPI_File_open(comm_, const_cast<char *>(file_name().c_str()),
				 MPI_MODE_CREATE | MPI_MODE_EXCL | MPI_MODE_RDWR, MPI_INFO_NULL,
				&fh_);
		if (err != MPI_SUCCESS){
			//a file with this name already exists.  It might contain persistent data
			//or it might just be left over from an aborted previous job.
			//In any case, we will try to create a unique name.
			//If the existing file contains persistent data, then this new file will
			// be deleted when the existing file is restored, and the existing file
			// will be deleted at the end of the sial program unless it has been marked
			// as persistent again.
			name_ = name + std::string("_");
			err = MPI_File_open(comm_, const_cast<char *>(file_name().c_str()),
					 MPI_MODE_CREATE | MPI_MODE_EXCL | MPI_MODE_RDWR, MPI_INFO_NULL,
					&fh_);
			if (err != MPI_SUCCESS){
				check(fail, "error creating new files " + name + " and " + name_);
			}
		}

//		std::cout << "creating file " << file_name() << std::endl <<std::flush;
		write_header(); //currently, the header contains the number of blocks, chunk size, and number of servers

		}
		else {
			//file must exist.  Read and broadcast header and check that values are as expected.
			int err = MPI_File_open(comm_, const_cast<char *>(file_name().c_str()),
					 MPI_MODE_RDWR, MPI_INFO_NULL,
					&fh_);
//			std::cout << "opening " << file_name() << std::endl << std::flush;
			check(err == MPI_SUCCESS, "failure opening existing array file " + file_name() + " at server");
			read_header();
			check(num_blocks == num_blocks_, "number of blocks in reopened file is different than expected");
			check(chunk_size == chunk_size_, "chunk size in reopened file is different than expected");
		}

	}

	/**
	 *
	 * If the file is not persistent, it is closed and deleted.
	 * If it persistent, it is closed and renamed to a filename constructed from the label.
	 *
	 * This is a collective operation
	 */
	~ArrayFile() {
//		std::cout << "in ~ArrayFile for " << *this << std::endl;
		MPI_File_close(&fh_);
//		std::cout << "closed file " << file_name() << std::endl <<std::flush;
		if (is_persistent_){
//			std::cout << "renaming file " <<file_name() << std::endl <<std::flush;
			rename_persistent();
		}
		else {
			if (!save_after_close_) {
//				std::cout << "deleting file " << file_name() << std::endl <<std::flush;
				delete_file();
			}
		}
	}


	/**
	 * This is a collective operation to set the view to write unsigned values.
	 * Then the communicator's master writes it.
	 * Followed by a collective sync.
	 */
	void write_header() {
		int err = MPI_File_set_view(fh_, 0, MPI_OFFSET_VAL_T, MPI_OFFSET_VAL_T,
				"native", MPI_INFO_NULL);
		if (comm_rank() == 0) {
			header_val_t header[NUM_VALS_IN_HEADER];
			header[0] = num_blocks_;
			header[1] = chunk_size_;
			header[2] = comm_size();
			int err = MPI_File_write_at(fh_, 0, header,
					NUM_VALS_IN_HEADER, MPI_OFFSET_VAL_T, MPI_STATUS_IGNORE);
			check(err == MPI_SUCCESS, "failure writing array file header");
		}
		set_view_for_data();
	}

	void read_header() {
		int rank;
		MPI_Comm_rank(comm_, &rank);
		int err = MPI_File_set_view(fh_, 0, MPI_OFFSET_VAL_T, MPI_OFFSET_VAL_T,
				"native", MPI_INFO_NULL);
		header_val_t header[NUM_VALS_IN_HEADER];
		if (rank == 0) {
			int err = MPI_File_read_at(fh_, 0, header,
					NUM_VALS_IN_HEADER, MPI_OFFSET_VAL_T, MPI_STATUS_IGNORE);
			check(err == MPI_SUCCESS, "failure reading array file header");
		}
		err = MPI_Bcast(header, NUM_VALS_IN_HEADER, MPI_OFFSET_VAL_T, 0,
				comm_);
		check(err == MPI_SUCCESS, "header broadcast failed");
		num_blocks_ = header[0];
		chunk_size_ = header[1];
		int file_comm_size = header[2];
		check(file_comm_size == comm_size(), "unimplemented feature--currently num servers must be same for reading and writing persisitent array");
		set_view_for_data();
	}

	/**
	 * Causes failure if the number of blocks or chunk size exceed the current implementation limits.
	 */
	void check_implementation_limits() {
		check(
				num_blocks_
						< std::numeric_limits<offset_val_t>::max() / chunk_size_,
				"implementation limit encountered:  type used to represent offsets in the index file may not be big enough");
		check(num_blocks_ < std::numeric_limits<int>::max(),
				"implemenation limit enountered:  number of blocks too large for int used when writing index file");
	}

	void set_view_for_data() {
		//displacement must be in bytes
		MPI_Offset displacement = (NUM_VALS_IN_HEADER * sizeof(header_val_t))
				+ (num_blocks_ * sizeof(offset_val_t));
		//TODO add padding for alignment?
//DEBUG		std::cout << "displacement to data view=" << displacement << std::endl;
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
	void chunk_write(const Chunk & chunk) {

		MPI_Offset offset = chunk.file_offset_;
//DEBUG		std::cout << "writing chunk at offset " << offset << std::endl << std::flush;
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
	void chunk_write_all(const Chunk & chunk) const {
		MPI_Offset offset = chunk.file_offset_;
//DEBUG		std::cout << "rank " << comm_rank() << " collective writing chunk at offset " << offset << std::endl << std::flush;
		MPI_Status status;
		int err = MPI_File_write_at_all(fh_, offset, chunk.data_, chunk_size_,
				MPI_DOUBLE, &status);
		check(err == MPI_SUCCESS, "chunk_write_all write failed");
	}


	void chunk_write_all_nop() const{
		MPI_Status status;
//DEBUG		std::cout << "rank " << comm_rank() << " noop collective write " <<  std::endl << std::flush;
		int err = MPI_File_write_at_all(fh_, 0, NULL, 0, MPI_DOUBLE, &status);
		check(err == MPI_SUCCESS, "chunk_write_all_nop failed");
	}

	/**
	 * Reads the data for the given chunk from disk.
	 * Precondition:  chunk memory has been allocated
	 *
	 * This is not a collective operation
	 *
	 * @param chunk
	 */
	void chunk_read(Chunk& chunk) {
//DEBUG		std::cerr << "in chunk_read " << chunk << std::endl << std::flush;
		MPI_Offset offset = chunk.file_offset_;
		MPI_Status status;
		int err = MPI_File_read_at(fh_, offset, chunk.data_, chunk_size_,
				MPI_DOUBLE, &status);
		check(err == MPI_SUCCESS, "chunk_read failed");
	}

	/**
	 * Reads the data for the given chunk from disk
	 * Precondition:  chunk memory has been allocated
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
	void write_index(offset_val_t* index, header_val_t size) {
		check(size == num_blocks_, "array passed to write_index may not have correct size");
		MPI_Offset displacement = NUM_VALS_IN_HEADER * sizeof(header_val_t);
		int err = MPI_File_set_view(fh_, displacement, MPI_OFFSET_VAL_T,
				MPI_OFFSET_VAL_T, "native", MPI_INFO_NULL);
		check(err == MPI_SUCCESS, "setting view to write index failed");
		MPI_Status status;


//		if(comm_rank() == 0){
//			std::cerr << "local index for rank 0: ";
//			for (int i = 0; i < size; ++i){
//				std::cerr <<index[i] << ',';
//			}
//			std::cerr << std::endl << std::flush;
//			MPI_Barrier(comm_);
//			MPI_Barrier(comm_);
//		}
//		else {
//			MPI_Barrier(comm_);
//			std::cerr << "local index for rank 1: ";
//			for (int i = 0; i < size; ++i){
//				std::cerr <<index[i] << ',';
//			}
//			std::cerr << std::endl << std::flush;
//			MPI_Barrier(comm_);
//		}

		if (comm_rank() == 0) {
			int err = MPI_Reduce(MPI_IN_PLACE, index, num_blocks_,
					MPI_OFFSET_VAL_T, MPI_MAX, 0, comm_);
			check(err == MPI_SUCCESS, "failure reducing file index at root");
			err = MPI_File_write_at(fh_, 0, index, num_blocks_,
					MPI_OFFSET_VAL_T, &status);
			check(err == MPI_SUCCESS, "failure writing file index");

//			std::cerr << "combined index : ";
//			for (int i = 0; i < size; ++i){
//				std::cerr <<index[i] << ',';
//			}
//			std::cerr << std::endl << std::flush;




		} else {
			int err = MPI_Reduce(index, NULL, num_blocks_, MPI_OFFSET_VAL_T,
					MPI_MAX, 0, comm_);
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
	void read_index(offset_val_t* index, header_val_t size) {
		check(size == num_blocks_,
				"array passed to read_index may not have correct size");
		MPI_Offset displacement = NUM_VALS_IN_HEADER * sizeof(header_val_t);
		int err = MPI_File_set_view(fh_, displacement, MPI_OFFSET_VAL_T,
		MPI_OFFSET_VAL_T, "native", MPI_INFO_NULL);
		MPI_Status status;
		if (comm_rank() == 0) {
			err = MPI_File_read_at(fh_, 0, index, num_blocks_,
			MPI_OFFSET_VAL_T, &status);
			check(err == MPI_SUCCESS, "failure reading file index");
		}
		err = MPI_Bcast(index, num_blocks_, MPI_OFFSET_VAL_T, 0, comm_);
		check(err == MPI_SUCCESS, "index broadcast failed");
		set_view_for_data();
	}

	void mark_persistent(const std::string& label) {
		check(is_persistent_==false, "calling set_persistent on already persistent file");
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

	int comm_rank() const{
		int rank;
		MPI_Comm_rank(comm_, &rank);
		return rank;
	}

	int comm_size() const{
		int size;
		MPI_Comm_size(comm_, &size);
		return size;
	}



	//const_cast<char *>(file_name.c_str())
	bool rename_persistent(){
		if (comm_rank() == 0){

		std::string name = file_name();
		std::string new_name = persistent_file_name();
		if (name.compare(new_name)==0) return false;
		int err = std::remove(const_cast<char *>(new_name.c_str()));
		//note that here the error is that the remove succeeded.
		check_and_warn(err != 0, std::string("Deleted existing persistent file ") + new_name);
//		std::cout << "renaming " << name << " to " << new_name << std::endl << std::flush;
		err = std::rename(const_cast<char*>(name.c_str()),
				const_cast<char*>(new_name.c_str()));
		if (err != 0){
			perror("Error renaming persistent array");
			check(false,"");
		}
		return true;
		}
		return false;
	}

	friend ChunkManager;
	friend std::ostream& operator<<(std::ostream& os, const ArrayFile& obj);


private:
	MPI_File fh_;
	header_val_t num_blocks_;
	header_val_t chunk_size_;
	std::string name_;
//	std::string system_file_name_;  //may be obtained from file_name, or persistent_file_name. Used to close.
	bool is_persistent_;
	std::string label_;
	const MPI_Comm& comm_;
	bool save_after_close_;  //used in tests.

	std::string file_name() const{
		std::stringstream ss;
		ss << "server." << name_ << ".arr";
		return ss.str();
	}

	std::string persistent_file_name() const{
		std::stringstream ss;
		ss << "server." << label_ << ".arr";
		return ss.str();
	}

	void delete_file(){
		if (comm_rank() == 0){
		std::string name = file_name();
		int err = std::remove(const_cast<char *>(name.c_str()));
		if (err != 0){
			perror ("Error deleting non-persistent array");
			check_and_warn(false, std::string("file ") + name);
		}
		}
	}

//	void init_offset_block_map_from_index(offset_val_t* index, header_val_t size){
//		int j = 0;
//		offset_val_t start_offset =
//		for(int i = 0; i < size; ++i){
//
//		}
//	}

	friend class DiskBackedBlockMap;

	DISALLOW_COPY_AND_ASSIGN(ArrayFile);
};

} /* namespace sip */

#endif /* ARRAY_FILE_H_ */
