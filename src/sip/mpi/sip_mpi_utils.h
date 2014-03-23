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
#include "sip_mpi_data.h"

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
	 * @param rank [in]
	 * @param tag [in]
	 * @return
	 */
	static sip::BlockId get_block_id_from_rank(int rank, int tag);

	/**
	 * Gets double precision data from another MPI rank and copies it into the passed Block
	 * @param rank
	 * @param tag [int]
	 * @param size
	 * @param bptr [inout]
	 */
	static void get_bptr_data_from_rank(int rank, int tag, int size, Block* bptr);

	/**
	 * Gets double precision data from another MPI rank and copies it into the passed ServerBlock
	 * @param rank
	 * @param tag [int]
	 * @param size
	 * @param bptr [inout]
	 */
	static void get_bptr_data_from_rank(int rank, int tag, int size, ServerBlock* bptr);

	/**
	 * Asynchronously sends a block id and a block to another MPI rank
	 * @param bid
	 * @param bptr
	 * @param rank
	 * @param size_tag
	 * @param data_tag
	 * @param request
	 */
	static void isend_bid_and_bptr_to_rank(const sip::BlockId& bid, sip::Block::BlockPtr bptr, int rank, int size_tag, int data_tag, MPI_Request *request);

	/**
	 * Asynchronously send a double array to another MPI rank.
	 * @param data
	 * @param size
	 * @param rank
	 * @param tag
	 */
	static MPI_Request isend_block_data_to_rank(Block::dataPtr data, int size, int rank, int tag);


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
	 * @param rank [in]
	 * @param tag [in]
	 * @param bid [out]
	 * @param shape [out]
	 * @param data_size [out]
	 */
	static void get_block_params(const int rank, int tag, sip::BlockId *bid, sip::BlockShape *shape, int *data_size);


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
	 * @param tag
	 */
	static void expect_ack_from_rank(const int rank, const int tag);


	/**
	 * Post an asynchronous receive for an ack message with a given tag from a specific rank
	 * @param rank
	 * @param ack
	 * @param tag
	 * @param request
	 */
	static void expect_async_ack_from_rank(const int rank, int ack, const int tag, MPI_Request *request);

//	/**
//	 * Each tag (a 32 bit integer) contains these fields
//	 */
//	typedef struct {
//		unsigned int message_type : 4;
//		unsigned int section_number : 12;
//		unsigned int message_number : 16;
//	} SIPMPITagBitField;
//
//	/**
//	 * Convenience union to convert bits from the bitfield to an int and back
//	 */
//	typedef union {
//		SIPMPITagBitField bf;
//		int i;
//	} SIPMPITagBitFieldConverter;
//
//	/**
//	 * Extracts the type of message from an MPI_TAG (right most byte).
//	 * @param mpi_tag
//	 * @return
//	 */
//	static SIPMPIData::MessageType_t get_message_type(int mpi_tag);
//
//	/**
//	 * Gets the section number from an MPI_TAG.
//	 * @param mpi_tag
//	 * @return
//	 */
//	static int get_section_number(int mpi_tag);
//
//	/**
//	 * Extracts the message number from an MPI_TAG.
//	 * @param mpi_tag
//	 * @return
//	 */
//	static int get_message_number(int mpi_tag);
//
//	/**
//	 * Constructs a tag from its constituent parts
//	 * @param message_type
//	 * @param section_number
//	 * @param message_number
//	 * @return
//	 */
//	static int make_mpi_tag(SIPMPIData::MessageType_t message_type, int section_number, int message_number);

private:


};

} /* namespace sip */

#endif /* SIP_MPI_UTILS_H_ */
