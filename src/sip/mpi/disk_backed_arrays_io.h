/*
 * disk_backed_arrays_io.h
 *
 *  Created on: Mar 17, 2014
 *      Author: njindal
 */

#ifndef DISK_BACKED_ARRAYS_IO_H_
#define DISK_BACKED_ARRAYS_IO_H_

#include <vector>
#include <map>

#include "mpi.h"
#include "id_block_map.h"
#include "block_id.h"
#include "block_shape.h"
#include "sip_mpi_attr.h"
#include "sip_tables.h"
#include "server_block.h"
#include "data_distribution.h"

//namespace sip {
//
//
///**
// * Provides disk backed IO for distributed and served arrays.
// * Reads and writes blocks to disk.
// */
//class DiskBackedArraysIO {
//public:
//
//	const static int MAX_FILE_NAME_SIZE = 256;
//	const static int INTS_IN_FILE_HEADER = 16;
//	const static int BLOCKFILE_MAGIC = 70209;
//	const static int BLOCKFILE_MAJOR_VERSION = 1;
//	const static int BLOCKFILE_MINOR_VERSION = 1;
//
//	DiskBackedArraysIO(const SipTables&, const SIPMPIAttr&, const DataDistribution&);
//	~DiskBackedArraysIO();
//
//	/**
//	 * Reads a block from disk into BlockPtr instance
//	 * @param bid  [in]
//	 * @param bptr [inout]
//	 */
//	void read_block_from_disk(const BlockId& bid, ServerBlock::ServerBlockPtr bptr);
//
//	/**
//	 * Writes a block to disk.
//	 * @param bid  [in]
//	 * @param bptr [in]
//	 */
//	void write_block_to_disk(const BlockId& bid, const ServerBlock::ServerBlockPtr bptr);
//
//	/**
//	 * Deletes the array
//	 * @param array_id
//	 */
//	void delete_array(const int array_id, IdBlockMap<ServerBlock>::PerArrayMap* per_array_map);
//
//	/**
//	 * Writes out all the blocks of an array to disk
//	 *
//	 * It renames the array file to server.<label>.persistarr and writes out all the blocks
//	 * that are marked dirty.
//	 * @param array_id
//	 * @param array_blocks
//	 * @param array_label
//	 */
//	void save_persistent_array(const int array_id,
//			const std::string& array_label,
//			sip::IdBlockMap<ServerBlock>::PerArrayMap* array_blocks);
//
//	/**
//	 * Reads all the blocks of the array with given label.
//	 * @param array_id
//	 * @param array_label
//	 */
//	void restore_persistent_array(const int array_id, const std::string& array_label);
//
//	friend std::ostream& operator<<(std::ostream& os, const DiskBackedArraysIO& obj);
//
//
//private:
//	typedef std::map<BlockId, MPI_Offset> BlockIdOffsetMap;
//
//	BlockIdOffsetMap block_offset_map_;			/**< Recently calculated offsets */
//	const SipTables& sip_tables_;				/**< Access to static SIP Data */
//	const SIPMPIAttr& sip_mpi_attr_;			/**< Access to MPI Attributes for this rank */
//	const DataDistribution data_distribution_;	/**< Data distribution scheme */
//	MPI_File *mpi_file_arr_;					/**< MPI File handles, one per sip array */
//
//
//	/**
//	 * Creates a MPI_File for an array with a given id/slot number.
//	 * Zeroes out all elements
//	 * @param array_id
//	 * @return
//	 */
//	MPI_File create_initialized_file_for_array(int array_id);
//
//	/**
//	 * Creates a MPI File for an array with a given id/slot number.
//	 * Does not zero out all elements.
//	 * @param array_id
//	 * @return
//	 */
//	MPI_File create_uninitialized_file_for_array(int array_id);
//
//	/**
//	 * Returns the offset on where to write block data in the file.
//	 * @param bid
//	 * @return
//	 */
//	MPI_Offset calculate_block_offset(const BlockId& bid);
//
//	/**
//	 * Makes sure that sizeof(MPI_INT) == sizeof(int) and
//	 * sizeof(MPI_DOUBLE) == sizeof(double)
//	 */
//	void check_data_types();
//
//	/**
//	 * Constructs file name for a distributed array
//	 * @param array_id [in]
//	 * @param filename [out]
//	 */
//	void array_file_name(int array_id, char filename[MAX_FILE_NAME_SIZE]);
//
//	/**
//	 * Writes a block to a MPI_File
//	 * @param fh
//	 * @param bid
//	 * @param bptr
//	 */
//	void write_block_to_file(MPI_File fh, const BlockId& bid,
//			const ServerBlock::ServerBlockPtr bptr);
//
//	/**
//	 * Constructs file name for a persistent array
//	 * @param array_label [in]
//	 * @param filename [out]
//	 */
//	void persistent_array_file_name(const std::string& array_label,
//			char filename[MAX_FILE_NAME_SIZE]);
//
//	/**
//	 * Returns true if a file with the given name exists
//	 * @param name
//	 * @return
//	 */
//	bool file_exists(const std::string& name);
//
////	/**
////	 * Writes out all the dirty blocks of an array to a given persistent file.
////	 * @param fh
////	 * @param array_blocks
////	 */
////	void write_all_dirty_blocks(MPI_File fh, const IdBlockMap<ServerBlock>::PerArrayMap* array_blocks);
////
////	/**
////	 * Zeroes out all blocks on disk for a given array.
////	 * @param array_id
////	 * @param mpif
////	 */
////	void collectively_zero_out_all_disk_blocks(const int array_id, MPI_File mpif);
////
////	/**
////	 * Collectively (Using MPIIO) copies data from the given MPI File handle
////	 * to a new file.
////	 *
////	 * Not used in current implementation. Kept around for future use.
////	 * Please verify correctness before using.
////	 *
////	 * @param persistent_filename
////	 * @param mpif_array
////	 */
////	void collectively_copy_block_data(
////				char persistent_filename[MAX_FILE_NAME_SIZE], MPI_File mpif_array);
////
//	/**
//	 * To save the persistent array,
//	 * write out zero for blocks that have not been formed yet.
//	 * Write the block if dirty or not on disk.
//	 * @param my_blocks
//	 * @param array_blocks
//	 */
//	void write_persistent_array_blocks(MPI_File mpif, std::list<BlockId> my_blocks,
//				IdBlockMap<ServerBlock>::PerArrayMap* array_blocks);
//
//};

//} /* namespace sip */

#endif /* DISK_BACKED_ARRAYS_IO_H_ */
