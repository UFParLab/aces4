/*
 * array_file.cpp
 *
 *  Created on: Aug 10, 2015
 *      Author: basbas
 */

#include "array_file.h"
#include <sys/stat.h>
#include <vector>
#include "job_control.h"

namespace sip {

ArrayFile::ArrayFile(header_val_t chunk_size, const std::string& name,
		const MPI_Comm& comm, bool save_after_close) :
		chunk_size_(chunk_size), name_(name), is_persistent_(false), save_after_close_(
				save_after_close), label_(""), comm_(comm), stats_(comm), was_restored_(
				false), index_file_name_("") {
//	check_implementation_limits();
//TODO
//create new file
	backing_file_name_ = make_temp_file_name(name_);
	int err = MPI_File_open(comm_,
			const_cast<char *>(backing_file_name_.c_str()),
			MPI_MODE_CREATE | MPI_MODE_EXCL | MPI_MODE_RDWR,
			MPI_INFO_NULL, &fh_);
	CHECK(err == MPI_SUCCESS,
			std::string("error creating new file for array ") + name
					+ std::string(" with filename " + backing_file_name_));
	write_header();
	//set view to skip header and count doubles.
	set_view_for_data();
}

ArrayFile::~ArrayFile() {
	if (!is_persistent_) MPI_File_close(&fh_);
	if (!is_persistent_ && !was_restored_ && !save_after_close_) {
		delete_file(backing_file_name_);
	}
}


const ArrayFile::offset_val_t ArrayFile::ABSENT_BLOCK_OFFSET=-1; //must be less than any valid offset

int ArrayFile::comm_rank() const {
	int rank;
	MPI_Comm_rank(comm_, &rank);
	return rank;
}

void ArrayFile::open_persistent_file(const std::string& label,
		offset_val_t& index_type, offset_val_t& index_size,
		std::vector<offset_val_t>& index) {

	//close and delete the original temp file
	MPI_File_close(&fh_);
	delete_file(backing_file_name_);

	label_ = label;
	was_restored_ = true;
	//open the persistent file.
	//first look for persistent file from current job
	//find the latest persistent file with the given label that was written during this job and open it.
	std::string jobid = JobControl::global->get_job_id();
	int current_prog_num = JobControl::global->get_program_num();
	backing_file_name_ = most_recent_matching_file(jobid, label, current_prog_num, PERSISTENT_SUFFIX);
	//if no file found from current job, try restart job.
	if (backing_file_name_=="" && JobControl::global->get_restart_id() != ""){
		backing_file_name_ = most_recent_matching_file(JobControl::global->get_restart_id(), label, current_prog_num, PERSISTENT_SUFFIX);
		//TODO  handle more than one restart job, probably the easiest way to do this is to create a
		//link with current jobid to each file used from the restart id.
	}
	CHECK(backing_file_name_ != "", "no persistent file for label" + label + " found.");
	int err = MPI_File_open(comm_,
			const_cast<char *>(backing_file_name_.c_str()),
			MPI_MODE_RDONLY, MPI_INFO_NULL, &fh_);
	CHECK(err == MPI_SUCCESS,
			std::string("failure opening persistent file") + label);
	//read the header.
	//this method checks that the chunk size and number of servers match.
	//TO DO handle the general case
	read_header();

	//open, read, and close the index file.

	std::string index_file_name = backing_file_name_ + std::string("_index");

	read_index(index_file_name, index_size,  index_type, index);
}

void ArrayFile::read_index(const std::string& index_file_name, offset_val_t& index_size,
		offset_val_t& index_type, std::vector<offset_val_t>& index) {

	offset_val_t index_header[NUM_VALS_IN_INDEX_HEADER];//buffer for index file header
	MPI_File index_fh = NULL; //handle for index file. Only rank 0 will change this value
	int err;
	//server rank 0 opens and reads index file header, then broadcasts to others
	if (comm_rank() == 0) {

		err = MPI_File_open(MPI_COMM_SELF,
				const_cast<char *>(index_file_name.c_str()),
				MPI_MODE_RDONLY, MPI_INFO_NULL, &index_fh);
		CHECK(err == MPI_SUCCESS,
				std::string("error opening index file ") + index_file_name);

		//set view to count offset type instead of byte
		MPI_Offset displacement = 0;
		err = MPI_File_set_view(index_fh, displacement,
		MPI_OFFSET_VAL_T,
		MPI_OFFSET_VAL_T, "native", MPI_INFO_NULL);
		MPI_Status status;
		err = MPI_File_read_at(index_fh, 0, index_header,
				NUM_VALS_IN_INDEX_HEADER,
				MPI_OFFSET_VAL_T, &status);
		CHECK(err == MPI_SUCCESS,
				std::string("failure reading file index header")
						+ index_file_name);

	}
	//everyone participates in broadcast of header
	MPI_Bcast(index_header, NUM_VALS_IN_INDEX_HEADER, MPI_OFFSET_VAL_T, 0,
			comm_);
	index_type = index_header[0];
	index_size = index_header[1];
	//Read the index data.
	//reserve space for the data in the index vector
	index.resize(index_size, ABSENT_BLOCK_OFFSET);

	//rank 0 reads index and closes the file
	if (comm_rank() == 0) {
		//set view to count offset type instead of byte and to skip over the header
//		MPI_Offset displacement = NUM_VALS_IN_INDEX_HEADER
//				* sizeof(offset_val_t);
//		int err = MPI_File_set_view(index_fh, displacement,
//		MPI_OFFSET_VAL_T,
//		MPI_OFFSET_VAL_T, "native", MPI_INFO_NULL);
		MPI_Status status;
		err = MPI_File_read_at(index_fh, NUM_VALS_IN_INDEX_HEADER, &index.front(), index_size,
		MPI_OFFSET_VAL_T, &status);
		CHECK(err == MPI_SUCCESS,
				std::string("failure reading file index") + index_file_name);
		MPI_File_close(&index_fh);
	}
	//everyone participates in the broadcast
	err = MPI_Bcast(&index.front(), index_size, MPI_OFFSET_VAL_T, 0, comm_);
	CHECK(err == MPI_SUCCESS, "index broadcast failed");
}

void ArrayFile::write_header() {
	int err = MPI_File_set_view(fh_, 0, MPI_HEADER_VAL_T,
	MPI_HEADER_VAL_T, "native", MPI_INFO_NULL);
	CHECK(err == MPI_SUCCESS,
			"failure setting view in write_header for file " + backing_file_name_);
	if (comm_rank() == 0) {
		header_val_t header[NUM_VALS_IN_HEADER];
		header[0] = chunk_size_;
		header[1] = comm_size();

		int err = MPI_File_write_at(fh_, 0, header, NUM_VALS_IN_HEADER,
		MPI_HEADER_VAL_T,
		MPI_STATUS_IGNORE);
		CHECK(err == MPI_SUCCESS, "failure writing array file header");
	}
	//technically should set view back to data, but we won't do that since we are just going to
	//close and rename the file next anyway.
}

void ArrayFile::read_header() {

	int err = MPI_File_set_view(fh_, 0, MPI_HEADER_VAL_T,
	MPI_HEADER_VAL_T, "native", MPI_INFO_NULL);
	header_val_t header[NUM_VALS_IN_HEADER];
	if (comm_rank() == 0) {
		int err = MPI_File_read_at(fh_, 0, header, NUM_VALS_IN_HEADER,
		MPI_HEADER_VAL_T,
		MPI_STATUS_IGNORE);
		CHECK(err == MPI_SUCCESS, "failure reading array file header");
	}
	err = MPI_Bcast(header, NUM_VALS_IN_HEADER,
	MPI_HEADER_VAL_T, 0, comm_);
	CHECK(err == MPI_SUCCESS, "header broadcast failed");
	int chunk_size = header[0];
	int file_comm_size = header[1];
	CHECK(chunk_size == chunk_size_,
			" unimplemented feature--currently chunks sizes must be consistent");

	CHECK(file_comm_size == comm_size(),
			"unimplemented feature--currently num servers must be same for reading and writing persisitent array");
	set_view_for_data();
}

void ArrayFile::set_view_for_data() {
//displacement must be in bytes
	MPI_Offset displacement = (NUM_VALS_IN_HEADER * sizeof(header_val_t));
//TODO add padding for alignment?
	int err = MPI_File_set_view(fh_, displacement, MPI_DOUBLE,
	MPI_DOUBLE, "native", MPI_INFO_NULL);
	CHECK(err == MPI_SUCCESS, "setting view to write data failed");
}

void ArrayFile::chunk_write(const Chunk & chunk) {

	MPI_Offset offset = chunk.file_offset_;
//DEBUG		std::cout << "writing chunk at offset " << offset << std::endl << std::flush;
	MPI_Status status;
	int err = MPI_File_write_at(fh_, offset, chunk.data_, chunk_size_,
	MPI_DOUBLE, &status);
	CHECK(err == MPI_SUCCESS, "write_chunk failed");
	stats_.chunks_written_.inc();
}

void ArrayFile::chunk_write_all(const Chunk & chunk) {
	MPI_Offset offset = chunk.file_offset_;
	MPI_Status status;
	int err = MPI_File_write_at_all(fh_, offset, chunk.data_, chunk_size_,
	MPI_DOUBLE, &status);
	if (err != MPI_SUCCESS){
		char string[MPI_MAX_ERROR_STRING];
		int resultlen;
		MPI_Error_string(err, string, &resultlen);
		CHECK(false, std::string("chunk_write_all write failed ") + string);
	}
//	CHECK(err == MPI_SUCCESS, "chunk_write_all write failed");
	stats_.chunks_written_.inc();
}

void ArrayFile::chunk_write_all_nop() const {
	MPI_Status status;
	int err = MPI_File_write_at_all(fh_, 0, NULL, 0, MPI_DOUBLE, &status);
	CHECK(err == MPI_SUCCESS, "chunk_write_all_nop failed");
}

void ArrayFile::chunk_read(Chunk& chunk) {
	MPI_Offset offset = chunk.file_offset_;
	MPI_Status status;
	int err = MPI_File_read_at(fh_, offset, chunk.data_, chunk_size_,
	MPI_DOUBLE, &status);
	CHECK(err == MPI_SUCCESS, "chunk_read failed");
	stats_.chunks_restored_.inc();
}

void ArrayFile::chunk_read_all(Chunk & chunk) {
	MPI_Offset offset = chunk.file_offset_;
	MPI_Status status;
	int err = MPI_File_read_at_all(fh_, offset, chunk.data_, chunk_size_,
	MPI_DOUBLE, &status);
	CHECK(err == MPI_SUCCESS, "chunk_read_all failed");
	stats_.chunks_restored_.inc();
}

void ArrayFile::chunk_read_all_nop() {
	MPI_Status status;
	int err = MPI_File_read_at_all(fh_, 0, NULL, 0, MPI_DOUBLE, &status);
	CHECK(err == MPI_SUCCESS, "chunk_read_all_nop failed");
}

void ArrayFile::write_sparse_index(std::vector<offset_val_t>& index) {
//	std::cerr << "writing sparse index " << make_index_file_name(label_) << std::endl << std::flush;
	MPI_Status status;
	std::vector<offset_val_t>::size_type index_size = index.size();
	CHECK(index_size < std::numeric_limits<int>::max(),
			"index size for persistent array exceeds mpi count limit. Fix to allow by writing in multiple parts");
	int err;
	//collectively compute total index size
	offset_val_t total_size;
	err = MPI_Reduce(&index_size, &total_size, 1,
			MPI_OFFSET_VAL_T, MPI_SUM, 0, comm_);
	CHECK(err == MPI_SUCCESS, "failure reducing sparse file index");

	//collectively compute offset for each server
	offset_val_t index_offset;
	err = MPI_Exscan(&index_size, &index_offset, 1, MPI_OFFSET_VAL_T, MPI_SUM, comm_);
	if (comm_rank()==0){
		index_offset = 0;  //root's val undefined after MPI_Exscan. Set to identity of operator, i.e. 0
	}

	//collectively open index file
	MPI_File index_fh;
	err = MPI_File_open(comm_,
			const_cast<char *>(make_index_file_name(label_).c_str()),
			MPI_MODE_CREATE | MPI_MODE_EXCL | MPI_MODE_WRONLY,
			MPI_INFO_NULL, &index_fh);
	CHECK(err == MPI_SUCCESS, "error opening file to write sparse index ");


	//set view to count MPI_OFFSET_VAL_T objects
	MPI_Offset displacement = 0;
	err = MPI_File_set_view(index_fh, displacement,
	MPI_OFFSET_VAL_T,
	MPI_OFFSET_VAL_T, "native", MPI_INFO_NULL);
	CHECK(err == MPI_SUCCESS, "setting view to write sparse index header failed");

	//rank 0 writes header
	if (comm_rank()==0){
	offset_val_t header[NUM_VALS_IN_INDEX_HEADER];
	header[0] = SPARSE_INDEX;
	header[1] = total_size;
	err = MPI_File_write_at(index_fh, 0, header, NUM_VALS_IN_HEADER,
	MPI_OFFSET_VAL_T, &status);
	CHECK(err == MPI_SUCCESS, "failure writing file index header");
	}
	//Collectively write the index.
	//but first, set view to skip the header
	//displacement must be in bytes
		displacement = (NUM_VALS_IN_INDEX_HEADER * sizeof(offset_val_t));
		err = MPI_File_set_view(index_fh, displacement, MPI_OFFSET_VAL_T,
				MPI_OFFSET_VAL_T, "native", MPI_INFO_NULL);
		CHECK(err == MPI_SUCCESS, "setting view to write index failed");

	err = MPI_File_write_at_all(index_fh, index_offset, &index.front(), index_size, MPI_OFFSET_VAL_T, &status);
		CHECK(err == MPI_SUCCESS, "write index data failed");
}

void ArrayFile::write_dense_index(std::vector<offset_val_t>& index) {
//	std::cerr << "writing dense index " << make_index_file_name(label_) << std::endl << std::flush;
	MPI_Status status;
	std::vector<offset_val_t>::size_type index_size = index.size();
	CHECK(index_size < std::numeric_limits<int>::max(),
			"index size for persistent array exceeds mpi count limit. Fix to allow by writing in multiple parts");
	int err;
	//TODO  check this
	if (comm_rank() == 0) {
		err = MPI_Reduce(MPI_IN_PLACE, &index.front(), index_size,
		MPI_OFFSET_VAL_T, MPI_MAX, 0, comm_);
		CHECK(err == MPI_SUCCESS, "failure reducing file index at root");
		//open index file and set view.  Only the master server writes, so use MPI_COMM_SELF
		MPI_File index_fh;
		err = MPI_File_open(MPI_COMM_SELF,
				const_cast<char *>(make_index_file_name(label_).c_str()),
				MPI_MODE_CREATE | MPI_MODE_EXCL | MPI_MODE_WRONLY,
				MPI_INFO_NULL, &index_fh);
		CHECK(err == MPI_SUCCESS, "error opening file to write index ");
		//change view to count MPI_OFFSET_VAL_T types instead of bytes
		MPI_Offset displacement = 0;
		err = MPI_File_set_view(index_fh, displacement,
		MPI_OFFSET_VAL_T,
		MPI_OFFSET_VAL_T, "native", MPI_INFO_NULL);
		CHECK(err == MPI_SUCCESS, "setting view to write index failed");
		//write header
		offset_val_t header[NUM_VALS_IN_INDEX_HEADER];
		header[0] = DENSE_INDEX;
		header[1] = index_size;
		err = MPI_File_write_at(index_fh, 0, header, NUM_VALS_IN_HEADER,
		MPI_OFFSET_VAL_T, &status);
		CHECK(err == MPI_SUCCESS, "failure writing file index header");
		//now write the index.
		err = MPI_File_write_at(index_fh, NUM_VALS_IN_HEADER, &index.front(), index_size,
		MPI_OFFSET_VAL_T, &status);
		CHECK(err == MPI_SUCCESS, "failure writing file index");
		MPI_File_close (&index_fh);
//		//DEBUG PRINT
//		std::cerr << "in write_dense_index " <<std::endl;
//		std::cerr << "index header:  index_type =  "<< header[0] <<
//		" index_size=";
//		std::cerr << header[1] << std::endl << std::flush;
//		for (std::vector<offset_val_t>::iterator it = index.begin(); it != index.end(); ++it){
//			std::cerr << *it << std::endl << std::flush;
//		}
	} else {
		int err = MPI_Reduce(&index.front(), NULL, index_size,
		MPI_OFFSET_VAL_T,
		MPI_MAX, 0, comm_);
		CHECK(err == MPI_SUCCESS, "failure reducing file index at non-root");
	}
}

//void ArrayFile::read_index_file(const std::string& file_name,
//		offset_val_t& index_type, offset_val_t& num_blocks,
//		std::vector<offset_val_t>& index) {
//
//	if (comm_rank() == 0) {
//		MPI_File_open(MPI_COMM_SELF, make_index_file_name(label_).c_str(),
//		MPI_MODE_RDONLY, MPI_INFO_NULL, &index_fh_);
//
//		MPI_Offset displacement = 0;
//		int err = MPI_File_set_view(index_fh_, displacement,
//		MPI_OFFSET_VAL_T,
//		MPI_OFFSET_VAL_T, "native", MPI_INFO_NULL);
//		MPI_Status status;
//		err = MPI_File_read_at(index_fh_, 0, index, num_blocks_,
//		MPI_OFFSET_VAL_T, &status);
//		CHECK(err == MPI_SUCCESS, "failure reading file index");
//	}
//	err = MPI_Bcast(index, num_blocks_, MPI_OFFSET_VAL_T, 0, comm_);
//	CHECK(err == MPI_SUCCESS, "index broadcast failed");
//}

void ArrayFile::mark_persistent(const std::string& label) {
	CHECK(is_persistent_ == false,
			"calling set_persistent on already persistent file");

	if (comm_rank() == 0) {
		std::cerr << "marking array with name " << name_
				<< " persistent with label " << label << std::endl
				<< std::flush;
	}
	is_persistent_ = true;
	label_ = label;
}

void ArrayFile::close_and_rename_persistent(){
	CHECK(is_persistent_, "attempting to treat non-persistent file as if it were persistent");
//	write_header();
	MPI_File_close(&fh_);
	if (comm_rank() == 0 && !was_restored_) {
		std::string new_file_name(make_persistent_file_name(label_));
		CHECK(rename(backing_file_name_, new_file_name),
				"error renaming persistent file");
//		std::cerr << "renamed " << backing_file_name_ << " to " << new_file_name << std::endl << std::flush;
		std::string index_file_name = make_index_file_name(label_);
		CHECK(file_exists(index_file_name),
				"index file has not been created " + index_file_name);
	}
}

///**
// * Reads the index of an array file and broadcast to other servers.
// *
// * Preconditions:
// *
// * the header has already been read, so num_blocks_ and chunk_size_ are
// * initialized.
// *
// * index has been allocated.
// *
// *This is a collective operation
// *
// * @param index
// */
//void read_dense_index(offset_val_t* index, header_val_t size) {
//	CHECK(size == num_blocks_,
//			"array passed to read_index may not have correct size");
//	MPI_Offset displacement = NUM_VALS_IN_HEADER * sizeof(header_val_t);
//	int err = MPI_File_set_view(fh_, displacement,
//	MPI_OFFSET_VAL_T,
//	MPI_OFFSET_VAL_T, "native", MPI_INFO_NULL);
//	MPI_Status status;
//	if (comm_rank() == 0) {
//		err = MPI_File_read_at(fh_, 0, index, num_blocks_,
//		MPI_OFFSET_VAL_T, &status);
//		CHECK(err == MPI_SUCCESS, "failure reading file index");
//	}
//	err = MPI_Bcast(index, num_blocks_, MPI_OFFSET_VAL_T, 0, comm_);
//	CHECK(err == MPI_SUCCESS, "index broadcast failed");
//	set_view_for_data();
//}

bool ArrayFile::file_exists(const std::string& file_name) {
	struct stat buf;
	return stat(file_name.c_str(), &buf) == 0;
}

std::ostream& operator<<(std::ostream& os, const ArrayFile& obj) {
	os << obj.name_
			<< ": " << obj.chunk_size_
			<< "," << obj.backing_file_name_
			<< "," << obj.was_restored_
			<< "," << obj.index_file_name_
			<< "," << obj.is_persistent_
			<< "," << obj.label_;
	os << std::endl;
	return os;
}



const std::string& ArrayFile::PERSISTENT_SUFFIX = "parr";
const std::string& ArrayFile::INDEX_SUFFIX = ArrayFile::PERSISTENT_SUFFIX + "_index";
const std::string& ArrayFile::TEMP_SUFFIX = "arr";

} /* namespace sip */
