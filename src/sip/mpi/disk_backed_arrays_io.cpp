/*
 * disk_backed_arrays_io.cpp
 *
 *  Created on: Mar 17, 2014
 *      Author: njindal
 */

#include <disk_backed_arrays_io.h>

#include <cstdio>
#include <fstream>
#include <iostream>

#include "mpi.h"
#include "sip_mpi_utils.h"
#include "global_state.h"

namespace sip {

DiskBackedArraysIO::DiskBackedArraysIO(const SipTables& sip_tables,
		const SIPMPIAttr& sip_mpi_attr, const DataDistribution& data_distribution):
	sip_tables_(sip_tables), sip_mpi_attr_(sip_mpi_attr),
	data_distribution_(data_distribution){

	// Data type checks
	check_data_types();

	int num_arrays = sip_tables_.num_arrays();

	mpi_file_arr_ = new MPI_File[num_arrays];
	std::fill(mpi_file_arr_, mpi_file_arr_ + num_arrays, MPI_FILE_NULL);

	// Make a "delete on close" file for each distributed array.
	for (int i=0; i<num_arrays; ++i){
		bool is_remote = sip_tables_.is_distributed(i) || sip_tables_.is_served(i);
		if (is_remote){
			mpi_file_arr_[i] = create_file_for_array(i);
		}
	}
}


DiskBackedArraysIO::~DiskBackedArraysIO(){

	// Close each of the files.
	int num_arrays = sip_tables_.num_arrays();
	for (int i=0; i<num_arrays; i++){
		MPI_File mpif = mpi_file_arr_[i];
		if (mpif != MPI_FILE_NULL)
			SIPMPIUtils::check_err(MPI_File_close(&mpif));
	}
	delete [] mpi_file_arr_;
}


void DiskBackedArraysIO::read_block_from_disk(const BlockId& bid, ServerBlock::ServerBlockPtr bptr){
	SIP_LOG(std::cout << sip_mpi_attr_.global_rank()<< " : Reading block "<<bid<<" from disk..."<< std::endl);
	MPI_Offset block_offset = calculate_block_offset(bid);
	int array_id = bid.array_id();
	MPI_File fh = mpi_file_arr_[array_id];
	sip::check(fh != MPI_FILE_NULL, "Trying to read block from array file after closing it !");
	MPI_Offset header_offset = INTS_IN_FILE_HEADER * sizeof(int);
	MPI_Status read_status;
	double * data = bptr->get_data();
	SIPMPIUtils::check_err(
			MPI_File_read_at(fh, header_offset + block_offset, data,
					bptr->size(), MPI_DOUBLE, &read_status));
	bptr->data_ = data;
	SIP_LOG(std::cout << sip_mpi_attr_.global_rank()<< " : Done Reading block "<<bid<<" from disk !"<< std::endl);

}

// Blocks of the array are layed out on disk in COLUMN-MAJOR order.
// (Block [0,0] is followed by [1,0]).
void DiskBackedArraysIO::write_block_to_disk(const BlockId& bid, const ServerBlock::ServerBlockPtr bptr){
	SIP_LOG(std::cout << sip_mpi_attr_.global_rank()<< " : Writing block "<<bid<<" to disk..."<< std::endl);

	int array_id = bid.array_id();
	MPI_File fh = mpi_file_arr_[array_id];
	write_block_to_file(fh, bid, bptr);
	SIP_LOG(std::cout << sip_mpi_attr_.global_rank()<< " : Done Writing block "<<bid<<" to disk !"<< std::endl);

}

void DiskBackedArraysIO::delete_array(const int array_id){
	// Close the current file & delete it
	MPI_File fh = mpi_file_arr_[array_id];
	MPI_File_close(&fh);	// Close on delete

	char filename[MAX_FILE_NAME_SIZE];
	array_file_name(array_id, filename);
	int my_rank = sip_mpi_attr_.company_rank();

	// Delete the array file
	MPI_Comm server_comm = sip_mpi_attr_.company_communicator();
	SIPMPIUtils::check_err(MPI_Barrier(server_comm));
	if (my_rank == 0)
		std::remove(filename);
	SIPMPIUtils::check_err(MPI_Barrier(server_comm));

	// Create a new array file
	mpi_file_arr_[array_id] = create_file_for_array(array_id);

}

void DiskBackedArraysIO::zero_out_all_disk_blocks(const int array_id,
		MPI_File mpif) {
	long tot_elems = sip_tables_.array_num_elems(array_id);
	MPI_Offset end_array_offset = tot_elems * sizeof(double);
	MPI_Offset header_offset = INTS_IN_FILE_HEADER * sizeof(int);
	int num_servers = sip_mpi_attr_.company_size();
	int elems_per_server = tot_elems / num_servers;
	if (tot_elems % num_servers != 0)
		elems_per_server++;

	double* file_buf = new double[elems_per_server](); // 0-d out buffer
	const int server_rank = sip_mpi_attr_.company_rank();
	MPI_Offset offset = header_offset
			+ server_rank * elems_per_server * sizeof(double);
	// Collectively write 0s to the file to fill in all blocks.
	MPI_Status write_status;
	SIPMPIUtils::check_err(
			MPI_File_write_at_all(mpif, offset, file_buf, elems_per_server,
					MPI_DOUBLE, &write_status));
	SIPMPIUtils::check_err(MPI_File_sync(mpif));
}

void DiskBackedArraysIO::save_persistent_array(const int array_id, const std::string& array_label,
		IdBlockMap<ServerBlock>::PerArrayMap* array_blocks){
	/* This method renames the array file to a temporary persistent file and writes out
	 * all the dirty blocks of the given array.
	 * If a previous persistent array file by the same label exists, it deletes it and renames
	 * this temporary persistent file to the final file.
	 */

	// Construct File Names
	char persistent_filename[MAX_FILE_NAME_SIZE];
	persistent_array_file_name(array_label, persistent_filename);

	char temp_persistent_filename[MAX_FILE_NAME_SIZE];
	sprintf(temp_persistent_filename, "%s.tmp", persistent_filename);

	mpi_file_arr_[array_id] = MPI_FILE_NULL; // So that a "close" is not attempted on it.


	// On rank 0, Rename the array file.
	int my_rank = sip_mpi_attr_.company_rank();
	if (my_rank == 0){
		char arr_filename[MAX_FILE_NAME_SIZE];
		array_file_name(array_id, arr_filename);

		// Rename array file to this new temp persistent filename.
		std::rename(arr_filename, temp_persistent_filename);
	}

	const MPI_Comm server_comm = sip_mpi_attr_.company_communicator();
	SIPMPIUtils::check_err(MPI_Barrier(server_comm));


	// Collectively write out 0s to all blocks in the file.
	// Get number of elements in entire array

	// Collectively Open the file
	MPI_File mpif = MPI_FILE_NULL;
	SIPMPIUtils::check_err(
			MPI_File_open(server_comm, temp_persistent_filename,
					MPI_MODE_WRONLY | MPI_MODE_RDWR, MPI_INFO_NULL, &mpif));

	zero_out_all_disk_blocks(array_id, mpif);
	
	// Write all dirty blocks to temp persistent file.
	write_all_dirty_blocks(mpif, array_blocks);

	// Close the file
	MPI_File_close(&mpif);


	SIPMPIUtils::check_err(MPI_Barrier(server_comm));

	// If a previous persistent file already existed, get rid of it
	const std::string name(persistent_filename);
	if (my_rank == 0 && file_exists(name)){
		char persistent_filename_old[MAX_FILE_NAME_SIZE];
		sprintf(persistent_filename_old, "%s.old", persistent_filename);
		std::remove(persistent_filename_old);
	}

	SIPMPIUtils::check_err(MPI_Barrier(server_comm));

	// Rename temp file.
	if (my_rank == 0)
		std::rename (temp_persistent_filename, persistent_filename);

	SIPMPIUtils::check_err(MPI_Barrier(server_comm));
}


void DiskBackedArraysIO::restore_persistent_array(const int array_id, const std::string& array_label){
	
	/* This method copies all the blocks from the file with the persistent array
	 * labelled array_label into the array file for array id = array_id.
	 */

	// file handle for the array - array_id.
	MPI_File mpif_array = mpi_file_arr_[array_id];
	

	int my_rank = sip_mpi_attr_.company_rank();
	char persistent_filename[MAX_FILE_NAME_SIZE];
	persistent_array_file_name(array_label, persistent_filename);

	// Open the file
	MPI_File mpif_persistent;
	const MPI_Comm server_comm = sip_mpi_attr_.company_communicator();
	SIPMPIUtils::check_err(
			MPI_File_open(server_comm, persistent_filename,
					MPI_MODE_RDONLY, MPI_INFO_NULL, &mpif_persistent));


	// Collectively copy from persistent file and write to array file.
	MPI_Offset header_offset = INTS_IN_FILE_HEADER * sizeof(int);
	MPI_Offset file_size;
	SIPMPIUtils::check_err(MPI_File_get_size(mpif_persistent, &file_size));
	sip::check((file_size - header_offset) % sizeof(double) == 0, "Inconsistent persistent file !");
	int tot_elems = (file_size - header_offset) / sizeof(double);
	
	int num_servers = sip_mpi_attr_.company_size();
	int elems_per_server = tot_elems / num_servers;
	if (tot_elems % num_servers != 0)
		elems_per_server ++;
	double * file_buf = new double[elems_per_server];
	const int server_rank = sip_mpi_attr_.company_rank();
	MPI_Status read_status, write_status;
	MPI_Offset offset = header_offset + server_rank * elems_per_server * sizeof(double);
	SIPMPIUtils::check_err(
			MPI_File_read_at_all(mpif_persistent, offset, file_buf,
					elems_per_server, MPI_DOUBLE, &read_status));
	int elems_read;
	SIPMPIUtils::check_err(
			MPI_Get_count(&read_status, MPI_DOUBLE, &elems_read));

	SIPMPIUtils::check_err(
			MPI_File_write_at_all(mpif_array, offset, file_buf,
					elems_read, MPI_DOUBLE, &write_status));

	SIPMPIUtils::check_err(MPI_File_close(&mpif_persistent));

}

void DiskBackedArraysIO::write_block_to_file(MPI_File fh, const BlockId& bid,
		const ServerBlock::ServerBlockPtr bptr) {
	MPI_Offset block_offset = calculate_block_offset(bid);
	sip::check(fh != MPI_FILE_NULL,
			"Trying to write block to array file after closing it !");
	MPI_Offset header_offset = INTS_IN_FILE_HEADER * sizeof(int);
	MPI_Status read_status;
	SIPMPIUtils::check_err(
			MPI_File_write_at(fh, header_offset + block_offset,
					bptr->get_data(), bptr->size(), MPI_DOUBLE, &read_status));
}



MPI_Offset DiskBackedArraysIO::calculate_block_offset(const BlockId& bid){

	// Return the cached offset if available
	BlockIdOffsetMap::const_iterator it = block_offset_map_.find(bid);
	if (it != block_offset_map_.end())
		return it->second;

	// Otherwise calculate the offset, cache it & return it.
	long tot_fp_elems = sip_tables_.block_offset_in_array(bid);
	MPI_Offset block_offset = tot_fp_elems * sizeof(double);
	block_offset_map_.insert(std::pair<BlockId, MPI_Offset>(bid, block_offset));

	return block_offset;
}

void DiskBackedArraysIO::write_all_dirty_blocks(MPI_File fh, const IdBlockMap<ServerBlock>::PerArrayMap* array_blocks) {

	// Write out missing blocks into the array.
	if (array_blocks != NULL) {
		MPI_Offset header_offset = INTS_IN_FILE_HEADER * sizeof(int);
		IdBlockMap<ServerBlock>::PerArrayMap::const_iterator it = array_blocks->begin();
		for (; it != array_blocks->end(); ++it) {
			const BlockId& bid = it->first;
			const ServerBlock::ServerBlockPtr bptr = it->second;
			if (bptr->is_dirty())
				write_block_to_file(fh, bid, bptr);
		}
	}

}

void DiskBackedArraysIO::persistent_array_file_name(
		const std::string& array_label, char filename[MAX_FILE_NAME_SIZE]) {
	const char* array_label_cptr = array_label.c_str();
	sprintf(filename, "server.%s.persistarr", array_label_cptr);
}

void DiskBackedArraysIO::array_file_name(int array_id, char filename[MAX_FILE_NAME_SIZE]){
	const std::string& program_name_str = GlobalState::get_program_name();
	const std::string& arr_name_str = sip_tables_.array_name(array_id);
	const char * program_name = program_name_str.c_str();
	const char * arr_name = arr_name_str.c_str();
	sip::check(strlen(program_name) > 1,
			"Program name length is too short - " + program_name_str
					+ " !");
	// Each array is saved in a file called:
	// server.<program_name>.<array_name>.arr
	sprintf(filename, "server.%s.%s.arr", program_name, arr_name);
}

inline bool DiskBackedArraysIO::file_exists(const std::string& name) {
	std::ifstream f(name.c_str());
	bool exists = false;
	if (f.good()) {
		f.close();
		exists = true;
	} else {
		f.close();
		exists = false;
	}
	return exists;
}


MPI_File DiskBackedArraysIO::create_file_for_array(int array_id){

	int my_rank = sip_mpi_attr_.company_rank();
	const MPI_Comm server_comm = sip_mpi_attr_.company_communicator();

	char filename[MAX_FILE_NAME_SIZE];
	MPI_File mpif;

	// Get file name for array
	array_file_name(array_id, filename);

	SIPMPIUtils::check_err(
			MPI_File_open(server_comm, filename,
					MPI_MODE_CREATE | MPI_MODE_RDWR | MPI_MODE_DELETE_ON_CLOSE,
					MPI_INFO_NULL, &mpif));

	// Rank 0 of servers writes out the header.
	if (my_rank == 0) {
		int header[INTS_IN_FILE_HEADER] = {0};
		header[0] = BLOCKFILE_MAGIC;
		header[1] = BLOCKFILE_MAJOR_VERSION;
		header[2] = BLOCKFILE_MINOR_VERSION;
		header[3] = array_id;		// Array Number

		MPI_Status status;
		SIPMPIUtils::check_err(
				MPI_File_write_at(mpif, 0, header, INTS_IN_FILE_HEADER,
						MPI_INT, &status));
	}

	SIPMPIUtils::check_err(MPI_File_sync(mpif));

	zero_out_all_disk_blocks(array_id, mpif);


	return mpif;
}


void DiskBackedArraysIO::check_data_types() {
	// Data type checks
	int size_of_mpiint;
	SIPMPIUtils::check_err(MPI_Type_size(MPI_INT, &size_of_mpiint));
	check(sizeof(int) == size_of_mpiint,
			"Size of int and MPI_INT don't match !");

	int size_of_double;
	SIPMPIUtils::check_err(MPI_Type_size(MPI_DOUBLE, &size_of_double));
	check(sizeof(double) == size_of_double,
				"Size of double and MPI_DOUBLE don't match !");
}

std::ostream& operator<<(std::ostream& os, const DiskBackedArraysIO& obj){
	os << "block_offset_map : ";
	DiskBackedArraysIO::BlockIdOffsetMap::const_iterator it = obj.block_offset_map_.begin();
	for (; it != obj.block_offset_map_.end(); ++it){
		os << it->first << " : " << it->second << std::endl;
	}

	return os;
}


} /* namespace sip */
