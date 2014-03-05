/*
 * sip_mpi_utils.h
 *
 *  Created on: Jan 17, 2014
 *      Author: njindal
 */

#ifndef SIP_MPI_UTILS_H_
#define SIP_MPI_UTILS_H_

#include "blocks.h"
#include "mpi.h"

namespace sip {

/**
 * Utility methods for MPI communication
 */
class SIPMPIUtils {

public:

	/**
	 * Sets the error handler to print the errors and exit.
	 */
	static void set_error_handler();

	/**
	 * Checks errors
	 * @param
	 */
	static void check_err(int);

	/**
	 * Gets array::BlockId from another MPI rank and returns it.
	 * @param rank
	 * @param block_id_tag
	 * @return
	 */
	static sip::BlockId get_block_id_from_rank(int rank, int block_id_tag);

	/**
	 * Gets double precision data from another MPI rank and copies it into the passed BlockPtr
	 * @param rank
	 * @param size_tag
	 * @param data_tag
	 * @param bid
	 * @return
	 */
	static void get_bptr_from_rank(int rank, int data_tag, int size, sip::Block::BlockPtr bptr);

	/**
	 * Send a block to another MPI rank
	 * @param bid
	 * @param bptr
	 * @param rank
	 * @param size_tag
	 * @param data_tag
	 */
	static void send_bptr_to_rank(const sip::BlockId& bid, sip::Block::BlockPtr bptr, int rank, int size_tag, int data_tag);

//	/**
//	 * Sends a string to another rank
//	 * @param rank
//	 * @param str
//	 * @param len
//	 * @param tag
//	 */
//	static void send_str_to_rank(const int rank, char *str, const int len, const int tag);


	/**
	 * Gets the block id, shape & data size from another MPI Rank
	 * @param rank
	 * @param size_tag
	 * @param bid
	 * @param shape
	 * @param data_size
	 */
	static void get_block_params(const int rank, const int size_tag, sip::BlockId& bid, sip::BlockShape& shape, int &data_size);


	/**
	 * Sends array::BlockId from another MPI rank.
	 * @param id
	 * @param server_rank
	 * @param tag
	 */
	static void send_block_id_to_rank(const sip::BlockId& id, int server_rank, int tag);

	/**
	 * Sends an ack to a given node (an integer value).
	 * @param rank
	 * @param ack
	 * @param tag
	 */
	static void send_ack_to_rank(const int rank, int ack, const int tag);

	/**
	 * Expects an ack message with a given tag from a specific rank
	 * @param rank
	 * @param ack
	 * @param tag
	 */
	static void expect_ack_from_rank(const int rank, int ack, const int tag);


};

} /* namespace sip */

#endif /* SIP_MPI_UTILS_H_ */
