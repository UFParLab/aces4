/*
 * disk_backed_arrays_io.cpp
 *
 *  Created on: Mar 17, 2014
 *      Author: njindal
 */

#include <time.h>
#include <unistd.h>

#include <disk_backed_arrays_io.h>

#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>

#include "mpi.h"
#include "sip_mpi_utils.h"
#include "global_state.h"

//#include "TAU.h"

namespace sip {

DiskBackedArraysIO::DiskBackedArraysIO(const SipTables& sip_tables
                                     , const SIPMPIAttr& sip_mpi_attr
                                     , const DataDistribution& data_distribution)
                                     : sip_tables_(sip_tables)
                                     , sip_mpi_attr_(sip_mpi_attr)
                                     , data_distribution_(data_distribution)
                                     , max_array_size(0){

    // Data type checks
    check_data_types();

    int num_arrays = sip_tables_.num_arrays();

    mpi_file_arr_  = new MPI_File[num_arrays];
    is_persistent_ = new bool[num_arrays];
    my_blocks_     = new std::list<BlockId>[num_arrays];
    persistent_array_name_ = new std::string[num_arrays];
    
    std::fill(mpi_file_arr_, mpi_file_arr_ + num_arrays, MPI_FILE_NULL);

    // Make a "delete on close" file for each distributed array.
    for (int array_id = 0; array_id < num_arrays; array_id ++) {
        bool is_remote = sip_tables_.is_distributed(array_id) 
                      || sip_tables_.is_served(array_id);
        if (is_remote) {
            mpi_file_arr_[array_id] = create_uninitialized_file_for_array(array_id);
        }
        is_persistent_[array_id] = false;
    }
    
}

DiskBackedArraysIO::~DiskBackedArraysIO() {
    // Close each of the files.
    int num_arrays = sip_tables_.num_arrays();
    
    for (int array_id = 0; array_id < num_arrays; array_id ++){
        MPI_File mpi_file = mpi_file_arr_[array_id];
        
        if (mpi_file != MPI_FILE_NULL) {
            SIPMPIUtils::check_err(MPI_File_close(&mpi_file),__LINE__,__FILE__);
            
            char arr_filename[MAX_FILE_NAME_SIZE];
            array_file_name(array_id, arr_filename);
            std::remove(arr_filename);
            
            delete [] block_bit_map_[array_id];
        }
    }
    
    delete [] mpi_file_arr_;
    delete [] is_persistent_;
    delete [] my_blocks_;
    delete [] persistent_array_name_;

    std::stringstream ss;
    ss << "Server: " << sip_mpi_attr_.global_rank() << std::endl;
    ss << "Max array size: " << max_array_size << std::endl;
    //std::cout << ss.str() << std::endl;    
}

void DiskBackedArraysIO::read_block_from_disk(const BlockId& bid, ServerBlock::ServerBlockPtr bptr) {
    //return;

    SIP_LOG(std::cout << sip_mpi_attr_.global_rank() << " : Reading block " << bid << " from disk..." << std::endl);

    int array_id = bid.array_id();
    MPI_File mpi_file = mpi_file_arr_[array_id];
    sip::check(mpi_file != MPI_FILE_NULL, "Trying to read block from array file after closing it !");
    
    MPI_Offset header_offset = INTS_IN_FILE_HEADER * sizeof(int);
    MPI_Offset block_offset = calculate_block_offset(bid);
/*
    TAU_START("Simulated Read");

    unsigned long long nano = 1000000000;
    unsigned long long t1, t2;

    struct timespec tm;

    clock_gettime(CLOCK_REALTIME, &tm);
    t1 = tm.tv_nsec + tm.tv_sec * nano;

    int block_size = bptr->size();
    double *block_pointer = bptr->get_data();

    //for (int i = 0; i < bptr->size(); i ++) {
        //block_pointer[i] = 0.0f;
    //}

    clock_gettime(CLOCK_REALTIME, &tm);
    t2 = tm.tv_nsec + tm.tv_sec * nano;

    unsigned long long expected_time = block_size * sizeof(double) / (1024*1024*1024*1.0) * 1000000;
    unsigned long long gone_time = (t2 - t1) / 1000;

    if (gone_time < expected_time) {
        usleep(expected_time - gone_time);
    }

    TAU_STOP("Simulated Read");
*/

    MPI_Status read_status;
    SIPMPIUtils::check_err(
            MPI_File_read_at(mpi_file, header_offset + block_offset, bptr->get_data(),
                             bptr->size(), MPI_DOUBLE, &read_status),
            __LINE__,__FILE__);

    /*
    SIPMPIUtils::check_err(
            MPI_File_read_at(mpi_file, header_offset + block_offset, bptr->get_data(),
                             bptr->size(), MPI_DOUBLE, &read_status),
            __LINE__,__FILE__);
    */
    SIP_LOG(std::cout << sip_mpi_attr_.global_rank() << " : Done Reading block " << bid << " from disk !" << std::endl);
}

void DiskBackedArraysIO::iread_block_from_disk(const BlockId& bid, ServerBlock::ServerBlockPtr bptr) {
    //return;

    //TAU_START("IREAD");

    int array_id = bid.array_id();
    MPI_File mpi_file = mpi_file_arr_[array_id];
    sip::check(mpi_file != MPI_FILE_NULL, "Trying to read block from array file after closing it !");
    
    MPI_Offset header_offset = INTS_IN_FILE_HEADER * sizeof(int);
    MPI_Offset block_offset = calculate_block_offset(bid);
/*
    TAU_START("Simulated Read");

    unsigned long long nano = 1000000000;
    unsigned long long t1, t2;

    struct timespec tm;

    clock_gettime(CLOCK_REALTIME, &tm);
    t1 = tm.tv_nsec + tm.tv_sec * nano;

    int block_size = bptr->size();
    double *block_pointer = bptr->get_data();

    //for (int i = 0; i < bptr->size(); i ++) {
    //    block_pointer[i] = 0.0f;
    //}

    clock_gettime(CLOCK_REALTIME, &tm);
    t2 = tm.tv_nsec + tm.tv_sec * nano;

    unsigned long long expected_time = block_size * sizeof(double) / (1024*1024*1024*1.0) * 1000000;
    unsigned long long gone_time = (t2 - t1) / 1000;

    if (gone_time < expected_time) {
        usleep(expected_time - gone_time);
    }

    TAU_STOP("Simulated Read");
*/
/*
    MPI_Request mpi_request;

    SIPMPIUtils::check_err(
            MPI_File_iread_at(mpi_file, header_offset + block_offset, bptr->get_data(),
                             bptr->size(), MPI_DOUBLE, &mpi_request),
            __LINE__,__FILE__);

    bptr->set_on_mpi(true);
	bptr->set_mpi_type(ServerBlock::MPI_READ);
	bptr->set_mpi_request(mpi_request);
*/
        
    MPI_Status read_status;
    SIPMPIUtils::check_err(
            MPI_File_read_at(mpi_file, header_offset + block_offset, bptr->get_data(),
                             bptr->size(), MPI_DOUBLE, &read_status),
            __LINE__,__FILE__);


    //TAU_STOP("IREAD");
}

void DiskBackedArraysIO::iwrite_block_to_disk(const BlockId& bid, ServerBlock::ServerBlockPtr bptr) {
    int array_id = bid.array_id();
    MPI_File mpi_file = mpi_file_arr_[array_id];
    sip::check(mpi_file != MPI_FILE_NULL, "Trying to read block from array file after closing it !");
    
    MPI_Offset header_offset = INTS_IN_FILE_HEADER * sizeof(int);
    MPI_Offset block_offset = calculate_block_offset(bid);
    MPI_Request mpi_request;
    SIPMPIUtils::check_err(
            MPI_File_iwrite_at(mpi_file, header_offset + block_offset,
                              bptr->get_data(), bptr->size(), MPI_DOUBLE, &mpi_request),
            __LINE__,__FILE__);
    
	//bptr->set_on_mpi(true);
	//bptr->set_mpi_type(ServerBlock::MPI_WRITE);
	//bptr->set_mpi_request(mpi_request);
}

// Blocks of the array are layed out on disk in COLUMN-MAJOR order.
// (Block [0,0] is followed by [1,0]).
void DiskBackedArraysIO::write_block_to_disk(const BlockId& bid, const ServerBlock::ServerBlockPtr bptr) {
    SIP_LOG(std::cout << sip_mpi_attr_.global_rank() << " : Writing block " << bid << " to disk..." << std::endl);

    int array_id = bid.array_id();
    MPI_File mpi_file = mpi_file_arr_[array_id];
    write_block_to_file(mpi_file, bid, bptr);
    SIP_LOG(std::cout << sip_mpi_attr_.global_rank() << " : Done Writing block " << bid << " to disk !" << std::endl);
}

void DiskBackedArraysIO::delete_array(const int array_id, IdBlockMap<ServerBlock>::PerArrayMap* per_array_map){
//    // Close the current file & delete it
//    MPI_File fh = mpi_file_arr_[array_id];
//    MPI_File_close(&fh);    // Close on delete
//
//    char filename[MAX_FILE_NAME_SIZE];
//    array_file_name(array_id, filename);
//    int my_rank = sip_mpi_attr_.company_rank();
//
//    // Delete the array file
//    MPI_Comm server_comm = sip_mpi_attr_.company_communicator();
//    SIPMPIUtils::check_err(MPI_Barrier(server_comm));
//    if (my_rank == 0)
//        std::remove(filename);
//    SIPMPIUtils::check_err(MPI_Barrier(server_comm));
//    // Create a new array file
//    mpi_file_arr_[array_id] = create_initialized_file_for_array(array_id);


    /** Zero out blocks on disk that this server owns
     */
    /*
    MPI_File my_file = mpi_file_arr_[array_id];
    IdBlockMap<ServerBlock>::PerArrayMap::iterator it;
    for (it = per_array_map->begin(); it != per_array_map->end(); it ++){
        ServerBlock *sb = it->second;
        BlockId bid = it->first;
        int block_size = sb->size();
        double * zero_block = new double[block_size]();
        
        MPI_Offset block_offset = calculate_block_offset(bid);
        MPI_Offset header_offset = INTS_IN_FILE_HEADER * sizeof(int);
        MPI_Status status;
        SIPMPIUtils::check_err(
                MPI_File_write_at(my_file, header_offset + block_offset,
                                  zero_block, block_size, MPI_DOUBLE, &status),
                __LINE__,__FILE__);

        delete [] zero_block;
    }
    */
    
    return;
    
    int server_rank        = sip_mpi_attr_.company_rank();
    int server_number      = sip_mpi_attr_.num_servers();
    const MPI_Comm server_comm = sip_mpi_attr_.company_communicator();
    
    MPI_File mpi_file = mpi_file_arr_[array_id];
    
    unsigned long long file_block_size = array_file_block_size_[array_id];
    double *writing_buffer = new double[file_block_size]();
    
    unsigned long long file_block_offset = 0, next_file_block_offset = 0;
    int file_block_index = 0;
    
    file_block_offset = data_start_offset_[array_id] + server_rank * file_block_size * sizeof(double);
    next_file_block_offset =  file_block_offset + server_number * file_block_size * sizeof(double);
    
    while (file_block_index < file_blocks_num_[array_id]) {
        MPI_Status status;
        MPI_Offset writing_offset = file_block_offset;
        
        SIPMPIUtils::check_err(
                MPI_File_write_at_all(mpi_file, writing_offset, writing_buffer, file_block_size, MPI_DOUBLE, &status),
                __LINE__,__FILE__);
        file_block_offset = next_file_block_offset;
        next_file_block_offset = file_block_offset + server_number * file_block_size * sizeof(double);
        file_block_index ++;
    }
    
    delete [] writing_buffer;
    
    for (int i = 0; i < block_bit_map_size_[array_id]; i ++) {
        block_bit_map_[array_id][i] = 0;
    }
    
    MPI_Status status;
        SIPMPIUtils::check_err(
                MPI_File_write_at_all(mpi_file, 
                block_bit_map_offset_[array_id],
                block_bit_map_[array_id],
                block_bit_map_size_[array_id],
                MPI_INT, &status),
                __LINE__,__FILE__);
    
}

void DiskBackedArraysIO::save_persistent_array(const int array_id, const std::string& array_label,
        IdBlockMap<ServerBlock>::PerArrayMap* array_blocks){
    /* This method renames the array file to a temporary persistent file and writes out
     * all the dirty blocks of the given array. It also zeroes out blocks on disk,
     * those that have not been formed in memory.
     * If a previous persistent array file by the same label exists, it deletes it and renames
     * this temporary persistent file to the final file.
     */

    is_persistent_[array_id] = true;
    persistent_array_name_[array_id] = array_label;
    
    int server_rank        = sip_mpi_attr_.company_rank();
    int server_global_rank = sip_mpi_attr_.global_rank();
    int server_number      = sip_mpi_attr_.num_servers();
    const MPI_Comm server_comm = sip_mpi_attr_.company_communicator();
    
    MPI_File mpi_file = mpi_file_arr_[array_id];
    
    std::list<BlockId> &server_blocks = my_blocks_[array_id];
    std::list<BlockId>::iterator blocks_it;
    
    unsigned long long file_block_size = array_file_block_size_[array_id];
    
    double *writing_buffer = new double[file_block_size];
    unsigned long long file_block_offset = 0, buffer_offset = 0, next_file_block_offset = 0;
    unsigned long long buffer_start_offset = 0;
    unsigned long long buffer_size = file_block_size;
    int file_block_index = 0;
    bool is_writing_continuous = true;
    
    MPI_Offset writing_offset;
    
    file_block_offset = data_start_offset_[array_id] + server_rank * file_block_size * sizeof(double);
    next_file_block_offset =  file_block_offset + server_number * file_block_size * sizeof(double);
    
    for (std::list<BlockId>::iterator blocks_it = server_blocks.begin(); blocks_it != server_blocks.end(); ++blocks_it) {
        BlockId& bid = *blocks_it;
        IdBlockMap<ServerBlock>::PerArrayMap::iterator found_it = array_blocks->find(bid);
        
        if (array_blocks->end() != found_it) {
            ServerBlock* sb = found_it->second;
            bool dirty     = sb->is_dirty();
            bool on_disk   = sb->is_on_disk();
            bool in_memory = sb->is_in_memory();
            // Error cases
            if (!on_disk && !in_memory) {
                sip::fail("Invalid block state ! - neither on disk or memory");
            } else if (dirty && on_disk && !in_memory) {
                sip::fail("Invalid block state ! - dirty but not in memory ");
            }
            
            if (!on_disk && dirty) {
                unsigned int block_size = sb->size();
                unsigned long long block_offset = block_offset_map_[bid];
                if (block_offset >= next_file_block_offset) {
                    MPI_Status status;
                    writing_offset = file_block_offset + buffer_start_offset * sizeof(double);
                    SIPMPIUtils::check_err(
                            MPI_Barrier(server_comm),
                            __LINE__,__FILE__);
                    //std::cout << "Buffer size: " << buffer_size << std::endl;
                    //std::cout << "buffer_start_offset: " << buffer_start_offset << std::endl;
                    //std::cout << "Server " << server_rank 
                    //          << " : writing at: " << writing_offset 
                    //          << " with size of " << (buffer_size-buffer_start_offset) << std::endl;
                    if (is_writing_continuous) {
                        if (buffer_offset != 0) {
                            SIPMPIUtils::check_err(
                                    MPI_File_write_at_all(mpi_file, writing_offset,
                                    writing_buffer+buffer_start_offset, buffer_size-buffer_start_offset, MPI_DOUBLE, &status),
                                    __LINE__,__FILE__);
                        } else {
                            SIPMPIUtils::check_err(
                                    MPI_File_write_at_all(mpi_file, writing_offset,
                                    writing_buffer, 0, MPI_DOUBLE, &status),
                                    __LINE__,__FILE__);
                        }
                    }
                    buffer_offset = 0;
                    buffer_start_offset = 0;
                    file_block_offset = next_file_block_offset;
                    next_file_block_offset = file_block_offset + server_number * file_block_size * sizeof(double);
                    file_block_index ++;
                    is_writing_continuous = true;
                }
                
                if (block_size == buffer_size) {
                    MPI_Status status;
                    writing_offset = file_block_offset;
                    SIPMPIUtils::check_err(
                            MPI_Barrier(server_comm),
                            __LINE__,__FILE__);
                    //std::cout << "Buffer size: " << buffer_size << std::endl;
                    //std::cout << "buffer_start_offset: " << buffer_start_offset << std::endl;
                    //std::cout << "Server " << server_rank 
                    //          << " : writing at: " << writing_offset 
                    //          << " with size of " << (buffer_size-buffer_start_offset) << std::endl;
                    SIPMPIUtils::check_err(
                            MPI_File_write_at_all(mpi_file, writing_offset,
                            sb->get_data(), block_size, MPI_DOUBLE, &status),
                            __LINE__,__FILE__);
                    buffer_offset = 0;
                    buffer_start_offset = 0;
                    file_block_offset = next_file_block_offset;
                    next_file_block_offset = file_block_offset + server_number * file_block_size * sizeof(double);
                    file_block_index ++;
                } else if (!is_writing_continuous || buffer_offset != (block_offset - file_block_offset) / sizeof(double)) {
                    MPI_Status status;
                    if (is_writing_continuous) {
                        SIPMPIUtils::check_err(
                                MPI_File_write_at_all(mpi_file, writing_offset,
                                writing_buffer+buffer_start_offset, buffer_offset-buffer_start_offset, MPI_DOUBLE, &status),
                                __LINE__,__FILE__);
                        is_writing_continuous = false;
                    }
                    SIPMPIUtils::check_err(
                            MPI_File_write_at(mpi_file, block_offset,
                            sb->get_data(), block_size, MPI_DOUBLE, &status),
                            __LINE__,__FILE__);
                } else {
                    if (buffer_offset == 0 && buffer_start_offset == 0 &&
                            block_offset != file_block_offset) {
                        buffer_start_offset = (block_offset - file_block_offset) / sizeof(double);
                        buffer_offset = buffer_start_offset;
                    } else {
                        buffer_offset = (block_offset - file_block_offset) / sizeof(double);
                    }
                    memcpy(writing_buffer+buffer_offset, sb->get_data(), block_size * sizeof(double));
                    buffer_offset += block_size;
                    sb->unset_dirty();
                }
            }
            block_bit_map_[array_id][block_index_[bid]/32] |= (1<<(block_index_[bid]%32));
        } else {
            //block_bit_map_[array_id][block_index_[bid]/32] &= (~(1<<(block_index_[bid]%32)));
        }
    }
    
    if (buffer_offset != 0) {
        MPI_Status status;
        writing_offset = file_block_offset + buffer_start_offset * sizeof(double);
        SIPMPIUtils::check_err(
                MPI_Barrier(server_comm),
                __LINE__,__FILE__);
        SIPMPIUtils::check_err(
                MPI_File_write_at_all(mpi_file, writing_offset,
                writing_buffer+buffer_start_offset, buffer_size-buffer_start_offset, MPI_DOUBLE, &status),
                __LINE__,__FILE__);
        //std::cout << "Server " << server_rank 
        //          << " : writing at: " << writing_offset 
        //          << " with size of " << (buffer_size-buffer_start_offset) << std::endl;
        //std::cout << "Server " << my_rank << " : writing at: " << writing_offset << " with size of " << buffer_offset << std::endl;
        buffer_offset = 0;
        buffer_start_offset = 0;
        file_block_offset = next_file_block_offset;
        next_file_block_offset = file_block_offset + server_number * file_block_size * sizeof(double);
        file_block_index ++;
    }
    
    while (file_block_index < file_blocks_num_[array_id]) {
        MPI_Status status;
        writing_offset = file_block_offset;
        
        SIPMPIUtils::check_err(
                MPI_Barrier(server_comm),
                __LINE__,__FILE__);
        SIPMPIUtils::check_err(
                MPI_File_write_at_all(mpi_file, writing_offset, writing_buffer, 0, MPI_DOUBLE, &status),
                __LINE__,__FILE__);
        //std::cout << "Server " << server_rank 
        //          << " : writing at: " << writing_offset 
        //          << " with size of " << buffer_size << std::endl;
        //std::cout << "Server " << my_rank << " : writing at: " << writing_offset << " with size of " << buffer_offset << std::endl;
        file_block_offset = next_file_block_offset;
        next_file_block_offset = file_block_offset + server_number * file_block_size * sizeof(double);
        file_block_index ++;
    }
    
    delete [] writing_buffer;
    
    SIPMPIUtils::check_err(
            MPI_Barrier(server_comm),
            __LINE__,__FILE__);
            
    MPI_Status status;
        SIPMPIUtils::check_err(
                MPI_File_write_at_all(mpi_file, 
                block_bit_map_offset_[array_id],
                block_bit_map_[array_id],
                block_bit_map_size_[array_id],
                MPI_INT, &status),
                __LINE__,__FILE__);
    
    /*
    typename IdBlockMap<ServerBlock>::PerArrayMap::iterator it;
    for (it = array_blocks->begin(); it != array_blocks->end(); ++it) {
        if (it->second != NULL) {
            ServerBlock* sb = it->second;
            bool dirty = sb->is_dirty();
            bool on_disk = sb->is_on_disk();
            bool in_memory = sb->is_in_memory();
            // Error cases
            if (!on_disk && !in_memory)
                sip::fail("Invalid block state ! - neither on disk or memory");
            else if (dirty && on_disk && !in_memory)
                sip::fail("Invalid block state ! - dirty but not in memory ");
            
            if (on_disk) {
                continue;
            }
            
            if (dirty) {
                write_block_to_disk(it->first, sb);
                sb->unset_dirty();
            }
        }
    }
    */
    
    if (mpi_file_arr_[array_id] != MPI_FILE_NULL) {
        SIPMPIUtils::check_err(
                MPI_File_close(&mpi_file_arr_[array_id]),
                __LINE__,__FILE__);
        mpi_file_arr_[array_id] = MPI_FILE_NULL;
    }
    
    SIPMPIUtils::check_err(MPI_Barrier(server_comm),__LINE__,__FILE__);

    if (server_rank == 0)
    {
        char persistent_filename[MAX_FILE_NAME_SIZE], arr_filename[MAX_FILE_NAME_SIZE];
        
        persistent_array_file_name(array_label, persistent_filename);
        array_file_name(array_id, arr_filename);
        std::rename(arr_filename, persistent_filename);
        
        SIP_LOG(std::cout << sip_mpi_attr_.global_rank() << " : Renamed file " << arr_filename 
                          << " to " << persistent_filename << std::endl);
    }
    
    return;
}

void DiskBackedArraysIO::restore_persistent_array(const int array_id, const std::string& array_label){
    SIP_LOG(std::cout << sip_mpi_attr_.global_rank() << " : Restoring label \"" 
                      << array_label << "\" into array " << sip_tables_.array_name(array_id) 
                      << " from disk" << std::endl);
    /* This method removes the array file
     * and renames the persistent file to the array file.
     */

    if (mpi_file_arr_[array_id] != MPI_FILE_NULL) {
        SIPMPIUtils::check_err(
                MPI_File_close(&mpi_file_arr_[array_id]),
                __LINE__,__FILE__);
    }
    
    char persistent_filename[MAX_FILE_NAME_SIZE], arr_filename[MAX_FILE_NAME_SIZE];
    
    persistent_array_file_name(array_label, persistent_filename);
    array_file_name(array_id, arr_filename);
    
    
    // Copies data from MPI_File handle "mpif_array" to file - persistent_filename.
    // collectively_copy_block_data(persistent_filename, mpif);

    // On rank 0, Rename the array file.
    int server_rank = sip_mpi_attr_.company_rank();
    const MPI_Comm server_comm = sip_mpi_attr_.company_communicator();
    
    if (server_rank == 0){
        std::remove(arr_filename);    //Needed // Not needed since file is DELETE_ON_CLOSE.
        // Rename persistent file to array file.
        std::rename(persistent_filename, arr_filename);
        SIP_LOG(std::cout << "S " << sip_mpi_attr_.global_rank() << " : Renamed file " 
                          << persistent_filename << " to " << arr_filename << std::endl);
    }

    SIPMPIUtils::check_err(
            MPI_Barrier(server_comm),
            __LINE__,__FILE__);

    MPI_File new_mpi_file;

    // Get file name for array into "filename"
    SIPMPIUtils::check_err(
            MPI_File_open(server_comm, arr_filename,
                          MPI_MODE_RDWR /*| MPI_MODE_DELETE_ON_CLOSE*/,
                          MPI_INFO_NULL, &new_mpi_file),
                          __LINE__,__FILE__);

    //MPI_File_set_atomicity(new_mpi_file, true);
    mpi_file_arr_[array_id] = new_mpi_file;

    MPI_Status status;
        SIPMPIUtils::check_err(
                MPI_File_read_at_all(new_mpi_file, 
                block_bit_map_offset_[array_id],
                block_bit_map_[array_id],
                block_bit_map_size_[array_id],
                MPI_INT, &status),
                __LINE__,__FILE__);
    
    SIPMPIUtils::check_err(
            MPI_Barrier(server_comm),
            __LINE__,__FILE__);
}

unsigned int * DiskBackedArraysIO::get_array_bit_map(int array_id) {
    return block_bit_map_[array_id];
}

void DiskBackedArraysIO::write_block_to_file(MPI_File mpi_file, const BlockId& bid,
                                             const ServerBlock::ServerBlockPtr bptr) {
    sip::check(mpi_file != MPI_FILE_NULL,
            "Trying to write block to array file after closing it !");
    
    MPI_Offset header_offset = INTS_IN_FILE_HEADER * sizeof(int);
    MPI_Offset block_offset = calculate_block_offset(bid);
    MPI_Status status;
    SIPMPIUtils::check_err(
            MPI_File_write_at(mpi_file, header_offset + block_offset,
                              bptr->get_data(), bptr->size(), MPI_DOUBLE, &status),
            __LINE__,__FILE__);
}

MPI_Offset DiskBackedArraysIO::calculate_block_offset(const BlockId& bid){

    // Return the cached offset if available
    BlockIdOffsetMap::const_iterator it = block_offset_map_.find(bid);
    if (it != block_offset_map_.end()) {
        return it->second;
    }
    
    sip::check(false, "Requested block does not have calculated file offset!");
    
    return -1;
}

void DiskBackedArraysIO::persistent_array_file_name(
        const std::string& array_label, char filename[MAX_FILE_NAME_SIZE]) {
    const char* array_label_cptr = array_label.c_str();
    sprintf(filename, "server.%s.persist.arr", array_label_cptr);
}

void DiskBackedArraysIO::array_file_name(int array_id, char filename[MAX_FILE_NAME_SIZE]){
    const std::string& program_name_str = GlobalState::get_program_name();
    const std::string& arr_name_str = sip_tables_.array_name(array_id);
    const char * program_name = program_name_str.c_str();
    const char * arr_name = arr_name_str.c_str();
    sip::check(program_name_str.length() > 1,
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

MPI_File DiskBackedArraysIO::create_uninitialized_file_for_array(int array_id) {

    const MPI_Comm& server_comm = sip_mpi_attr_.company_communicator();

    char filename[MAX_FILE_NAME_SIZE];
    MPI_File mpi_file;

    // Get file name for array into "filename"
    array_file_name(array_id, filename);

    //std::cout << "creating uninitialized file for array " << array_id << " with filename " << std::string(filename) << std::endl << std::flush;

    SIPMPIUtils::check_err(
            MPI_File_open(server_comm, filename,
                            MPI_MODE_EXCL | MPI_MODE_CREATE |
                            MPI_MODE_RDWR /*| MPI_MODE_DELETE_ON_CLOSE*/,
                            MPI_INFO_NULL, &mpi_file),
            __LINE__,__FILE__);

    MPI_File_set_atomicity(mpi_file, true);    

    // Rank 0 of servers writes out the header.
    int server_rank = sip_mpi_attr_.company_rank();
    if (server_rank == 0) {
        int header[INTS_IN_FILE_HEADER] = { 0 };
        header[0] = BLOCKFILE_MAGIC;
        header[1] = BLOCKFILE_MAJOR_VERSION;
        header[2] = BLOCKFILE_MINOR_VERSION;
        header[3] = array_id;        // Array Number

        MPI_Status status;
        SIPMPIUtils::check_err(
                MPI_File_write_at(mpi_file, 0, header, INTS_IN_FILE_HEADER,
                MPI_INT, &status),
                __LINE__,__FILE__);
    }
    
    SIPMPIUtils::check_err(
            MPI_File_sync(mpi_file),
            __LINE__,__FILE__);
    
    std::list<BlockId> server_blocks;
    int server_global_rank = sip_mpi_attr_.global_rank();
    data_distribution_.generate_server_blocks_list(server_global_rank, array_id, server_blocks, sip_tables_);
    my_blocks_[array_id] = server_blocks;
    
    unsigned long long file_block_size = min_file_block_size_;
    
    for (std::list<BlockId>::iterator blocks_it = server_blocks.begin(); blocks_it != server_blocks.end(); blocks_it ++) {
        unsigned int block_size = sip_tables_.block_size(*blocks_it);
        
        while (file_block_size < block_size) {
            file_block_size *= 2;
        }
    }
    array_file_block_size_[array_id] = file_block_size;
    
    int server_number = sip_mpi_attr_.num_servers();

    unsigned long long array_size = 0;
    unsigned long long file_group_offset_ = server_rank * file_block_size, file_block_offset_ = 0;
    int block_index = 0;
    int file_block_index_ = 1;

    for (std::list<BlockId>::iterator blocks_it = server_blocks.begin(); blocks_it != server_blocks.end(); blocks_it ++) {
        BlockId& bid = *blocks_it;
        block_index_.insert(std::pair<BlockId, int>(bid, block_index++));
        unsigned int block_size = sip_tables_.block_size(bid);
        array_size += block_size;

        if (file_block_offset_ + block_size > file_block_size) {
            file_group_offset_ += server_number * file_block_size;
            file_block_offset_ = 0;
            file_block_index_ ++;
        }
        block_offset_map_.insert(std::pair<BlockId, MPI_Offset>(bid, (file_group_offset_ + file_block_offset_) * sizeof(double)));
        file_block_offset_ += block_size;
    }
    
    if (array_size > max_array_size) {
        max_array_size = array_size;
    }
    
    unsigned long long temp[server_number], block_numbers[server_number];
    
    int max_file_block_number;
    
    SIPMPIUtils::check_err(
            MPI_Allreduce(&file_block_index_, &max_file_block_number, 1, MPI_INT, MPI_MAX, server_comm),
            __LINE__,__FILE__);
    
    file_blocks_num_.insert(std::pair<int,int>(array_id, max_file_block_number));
    
    for (int i = 0; i < server_number; i ++)
    {
        temp[i] = 0;
    }

    temp[server_rank] = server_blocks.size() / 8 / sizeof(int) + 1;

    SIPMPIUtils::check_err(
            MPI_Allreduce(temp, block_numbers, server_number, MPI_UINT64_T, MPI_SUM, server_comm),
            __LINE__,__FILE__);
    
    unsigned long long server_bit_map_offset = 0, servers_bit_map_sum = 0;
    
    for (int i = 0; i < server_number; i ++) {
        if (i < server_rank) {
            server_bit_map_offset += block_numbers[i];
        }
        servers_bit_map_sum += block_numbers[i];
    }
    
    unsigned long long server_writing_offset 
            = INTS_IN_FILE_HEADER * sizeof(int) + servers_bit_map_sum * sizeof(int);
    
    //align the offset to file_block_size_.
    server_writing_offset = (server_writing_offset / (file_block_size * sizeof(double)) + 1) * file_block_size * sizeof(double);
    //std::cout << "Server: " << server_rank << ", Data start offset: " << server_writing_offset << std::endl;

    for (std::list<BlockId>::iterator blocks_it = server_blocks.begin(); blocks_it != server_blocks.end(); blocks_it ++) {
        BlockId& bid = *blocks_it;
        block_offset_map_[bid] += server_writing_offset;
        //std::cout << "Server: " << server_rank << ", Block: " << bid << ", Offset: " << block_offset_map_[bid] << std::endl;
    }
    
    data_start_offset_.insert(std::pair<int,unsigned long long>(array_id, server_writing_offset));
    
    block_bit_map_.insert(std::pair<int, unsigned int*>(array_id, new unsigned int[block_numbers[server_rank]]()));
    
    block_bit_map_size_.insert(std::pair<int, int>(array_id, block_numbers[server_rank]));

    block_bit_map_offset_.insert(std::pair<int, unsigned long long>
                (array_id, INTS_IN_FILE_HEADER * sizeof(int) + server_bit_map_offset * sizeof(int)));
    
    SIPMPIUtils::check_err(
            MPI_Barrier(server_comm),
            __LINE__,__FILE__);
    
    return mpi_file;
}

void DiskBackedArraysIO::write_persistent_array_blocks(
        MPI_File mpif,
        std::list<BlockId> my_blocks,
        IdBlockMap<ServerBlock>::PerArrayMap* array_blocks) {
    // To save the persistent array, write out zero for blocks
    // that have not been formed yet.
    // Write the block if dirty or not on disk.
    /**
     * When to write a block to disk
     * Dirty    OnDisk    InMemory    WhatToWriteToDisk
     * 1        1        0            X - Error Condition
     * 1        0        0            X - Error Condition
     *
     * 0        0        0            Zeroes (Should never happen !)
     *
     * 0        1        0            Nothing
     * 0        1        1            Nothing
     *
     * 0        0        1            Block
     * 1        0        1            Block
     * 1        1        1            Block
     */
    std::list<BlockId>::iterator blocks_it = my_blocks.begin();
    for (; blocks_it != my_blocks.end(); ++blocks_it) {
        BlockId& bid = *blocks_it;
        IdBlockMap<ServerBlock>::PerArrayMap::iterator found_it =
                array_blocks->find(bid);
        if (array_blocks->end() != found_it) {

            bool to_write_dirty_block = false;

            ServerBlock* sb = found_it->second;
            bool dirty = sb->is_dirty();
            bool on_disk = sb->is_on_disk();
            bool in_memory = sb->is_in_memory();
            // Error cases
            if (!on_disk && !in_memory)
                sip::fail("Invalid block state ! - neither on disk or memory");
            else if (dirty && on_disk && !in_memory)
                sip::fail("Invalid block state ! - dirty but not in memory ");
            else
            // Don't write anything to disk
            if (!dirty && on_disk)
                to_write_dirty_block = false;
            else
            // Write the block to disk
            if (dirty)
                to_write_dirty_block = true;
            else if (!on_disk && in_memory)
                to_write_dirty_block = true;


            if (to_write_dirty_block)
                write_block_to_file(mpif, bid, sb);

        } else {
            // Write zero block

            std::size_t block_size = sip_tables_.block_size(bid);
            ServerBlock::dataPtr zero_data = new double[block_size]();
            ServerBlock *zero_sb = new ServerBlock(block_size, zero_data);
            write_block_to_file(mpif, bid, zero_sb);
            delete zero_sb;

        }

    }
}

void DiskBackedArraysIO::check_data_types() {
    // Data type checks
    int size_of_mpiint;
    SIPMPIUtils::check_err(MPI_Type_size(MPI_INT, &size_of_mpiint),__LINE__,__FILE__);
    check(sizeof(int) == size_of_mpiint,
            "Size of int and MPI_INT don't match !");

    int size_of_double;
    SIPMPIUtils::check_err(MPI_Type_size(MPI_DOUBLE, &size_of_double),__LINE__,__FILE__);
    check(sizeof(double) == size_of_double,
                "Size of double and MPI_DOUBLE don't match !");
}

/******************** BEGIN UNUSED ********************/

void DiskBackedArraysIO::write_all_dirty_blocks(MPI_File fh,
        const IdBlockMap<ServerBlock>::PerArrayMap* array_blocks) {

    // Write out missing blocks into the array.
    if (array_blocks != NULL) {
        MPI_Offset header_offset = INTS_IN_FILE_HEADER * sizeof(int);
        IdBlockMap<ServerBlock>::PerArrayMap::const_iterator it =
                array_blocks->begin();
        for (; it != array_blocks->end(); ++it) {
            const BlockId& bid = it->first;
            const ServerBlock::ServerBlockPtr bptr = it->second;
            if (bptr->is_dirty())
                write_block_to_file(fh, bid, bptr);
        }
    }

}

void DiskBackedArraysIO::collectively_zero_out_all_disk_blocks(
        const int array_id, MPI_File mpif) {
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
            MPI_DOUBLE, &write_status),__LINE__,__FILE__);
    SIPMPIUtils::check_err(MPI_File_sync(mpif),__LINE__,__FILE__);
    delete[] file_buf;
}

MPI_File DiskBackedArraysIO::create_initialized_file_for_array(int array_id) {

    MPI_File mpif = create_uninitialized_file_for_array(array_id);
    collectively_zero_out_all_disk_blocks(array_id, mpif);

    return mpif;
}

void DiskBackedArraysIO::collectively_copy_block_data(
        char persistent_filename[MAX_FILE_NAME_SIZE], MPI_File mpif_array) {
    // Open the file
    MPI_File mpif_persistent;
    const MPI_Comm server_comm = sip_mpi_attr_.company_communicator();
    SIPMPIUtils::check_err(MPI_File_open(server_comm, persistent_filename,
    MPI_MODE_RDONLY, MPI_INFO_NULL, &mpif_persistent),__LINE__,__FILE__);
    // Collectively copy from persistent file and write to array file.
    MPI_Offset header_offset = INTS_IN_FILE_HEADER * sizeof(int);
    MPI_Offset file_size;
    SIPMPIUtils::check_err(MPI_File_get_size(mpif_persistent, &file_size),__LINE__,__FILE__);
    sip::check((file_size - header_offset) % sizeof(double) == 0,
            "Inconsistent persistent file !");
    int tot_elems = (file_size - header_offset) / sizeof(double);
    int num_servers = sip_mpi_attr_.company_size();
    int elems_per_server = tot_elems / num_servers;
    if (tot_elems % num_servers != 0)
        elems_per_server++;

    double* file_buf = new double[elems_per_server];
    const int server_rank = sip_mpi_attr_.company_rank();
    MPI_Status read_status, write_status;
    MPI_Offset offset = header_offset
            + server_rank * elems_per_server * sizeof(double);
    SIPMPIUtils::check_err(
            MPI_File_read_at_all(mpif_persistent, offset, file_buf,
                    elems_per_server, MPI_DOUBLE, &read_status),__LINE__,__FILE__);
    int elems_read;
    SIPMPIUtils::check_err(
            MPI_Get_count(&read_status, MPI_DOUBLE, &elems_read),__LINE__,__FILE__);
    SIPMPIUtils::check_err(
            MPI_File_write_at_all(mpif_array, offset, file_buf, elems_read,
            MPI_DOUBLE, &write_status),__LINE__,__FILE__);
    SIPMPIUtils::check_err(MPI_File_close(&mpif_persistent),__LINE__,__FILE__);

    delete[] file_buf;
}

/******************** END OF UNUSED ********************/

std::ostream& operator<<(std::ostream& os, const DiskBackedArraysIO& obj){
    os << "block_offset_map : ";
    DiskBackedArraysIO::BlockIdOffsetMap::const_iterator it = obj.block_offset_map_.begin();
    for (; it != obj.block_offset_map_.end(); ++it){
        os << it->first << " : " << it->second << std::endl;
    }

    return os;
}


} /* namespace sip */
