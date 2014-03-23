/*
 * barrier_support.h
 *
 *  Created on: Mar 23, 2014
 *      Author: basbas
 */

#ifndef BARRIER_SUPPORT_H_
#define BARRIER_SUPPORT_H_

#include "sip_mpi_data.h";

namespace sip {

class BarrierSupport {
public:
	BarrierSupport();
	virtual ~BarrierSupport();
	/**
	 * Each tag (a 32 bit integer) contains these fields
	 */
	typedef struct {
		unsigned int message_type : 4;
		unsigned int section_number : 12;
		unsigned int message_number : 16;
	} SIPMPITagBitField;

	/**
	 * Convenience union to convert bits from the bitfield to an int and back
	 */
	typedef union {
		SIPMPITagBitField bf;
		int i;
	} SIPMPITagBitFieldConverter;

	/**
	 * Extracts the type of message from an MPI_TAG (right most byte).
	 * @param mpi_tag
	 * @return
	 */
	static SIPMPIData::MessageType_t extract_message_type(int mpi_tag){
		SIPMPITagBitFieldConverter bc;
	bc.i = mpi_tag;
	return SIPMPIData::intToMessageType(bc.bf.message_type);
	}

	/**
	 * Extracts the section number from an MPI_TAG.
	 * @param mpi_tag
	 * @return
	 */
	static int extract_section_number(int mpi_tag){
		SIPMPITagBitFieldConverter bc;
		bc.i = mpi_tag;
		return bc.bf.section_number;
	}

	/**
	 * Extracts the message number from an MPI_TAG.
	 * @param mpi_tag
	 * @return
	 */
	static int extract_message_number(int mpi_tag){
		SIPMPITagBitFieldConverter bc;
		bc.i = mpi_tag;
		return bc.bf.message_number;
	}

	/**
	 * Constructs a tag from its constituent parts
	 * @param message_type
	 * @param section_number
	 * @param message_number
	 * @return
	 */
	static int make_mpi_tag(SIPMPIData::MessageType_t message_type, int section_number, int message_number){
		SIPMPITagBitFieldConverter bc;
		bc.bf.message_type = message_type;
		bc.bf.section_number = section_number;
		bc.bf.message_number = message_number;
		return bc.i;
	}

	/**
	 * Called by the server loop for each message it receives.  Extracts message_type, section_number, and message_number,
	 * checks the invariant, and updates the section number.
	 *
	 * @param tag
	 * @param message_type
	 * @param section_number
	 * @param message_number
	 */
	void decode_tag_and_check_invariant(int mpi_tag, SIPMPIData::MessageType_t message_type, int section_number, int message_number){
		message_type = extract_message_type(mpi_tag);
		message_number = extract_message_number(mpi_tag);
		section_number = extract_section_number(mpi_tag);
		check (section_number >= this->section_number_, "Section number invariant violated. Received request from an older section !");
		this->section_number_ = section_number;
	}

	/**
	 * Called by worker to construct mpi tag for a GET transaction.
	 *
	 * The worker's message number is updated.
	 * @return tag
	 */
	int make_mpi_tag_for_GET(){
		return make_mpi_tag(SIPMPIData::GET, section_number_, message_number_++);
	}

	/**
	 * Called by the worker to construct mpi tags for a PUT transaction.
	 * The message number is the same in both messages.
	 *
	 * The worker's message number is updated.
	 *
	 * @param [out] put_data_tag contains the PUT_DATA tag
	 * @return PUT tag
	 */
	int make_mpi_tags_for_PUT(int& put_data_tag){
		int put_tag = make_mpi_tag(SIPMPIData::PUT, section_number_, message_number_);
		put_data_tag = make_mpi_tag(SIPMPIData::PUT_DATA, section_number_, message_number_++);
		return put_tag;
	}

	/**
	 * Constructs a PUT_DATA tag with the same section number and session number as the given tag
	 * Called by the server
	 *
	 * Requires:  the given tag is a PUT tag.
	 *
	 * @param put_tag
	 * @return
	 */
	int make_mpi_tag_for_PUT_DATA(int put_tag){
		SIPMPIData::MessageType_t message_type = extract_message_type(put_tag);
		int message_number = extract_message_number(put_tag);
		int section_number = extract_section_number(put_tag);
		return make_mpi_tag(SIPMPIData::PUT_DATA, section_number_, message_number_);
	}

	/**
	 * Called by the worker to construct mpi tags for a PUT_ACCUMULATE transaction.
	 * The message number is the same in both messages.
	 *
	 * The worker's message number is updated.
	 *
	 * @param [out] put_data_tag contains the PUT_ACCUMULATE_DATA tag
	 * @return PUT_ACCUMULATE tag
	 */
	int make_mpi_tags_for_PUT_ACCUMULATE(int& put_accumulate_data_tag){
		int put_accumulate_tag = make_mpi_tag(SIPMPIData::PUT_ACCUMULATE, section_number_, message_number_);
		put_accumulate_data_tag = make_mpi_tag(SIPMPIData::PUT_ACCUMULATE_DATA, section_number_, message_number_++);
		return put_accumulate_tag;
	}

	/**
	 * Constructs a PUT_ACCUMULATE DATA tag with the same section number and session number as the given tag
	 * Called by the server
	 *
	 * Requires:  the given tag is a PUT_ACCUMULATE tag.
	 *
	 * @param put_tag
	 * @return
	 */
	int make_mpi_tag_for_PUT_ACCUMULATE_DATA(int put_accumulate_tag){
		int message_number = extract_message_number(put_accumulate_tag);
		int section_number = extract_section_number(put_accumulate_tag);
		return make_mpi_tag(SIPMPIData::PUT_ACCUMULATE_DATA, section_number_, message_number_);
	}


	/**
	 * Called by worker to construct mpi tag for a DELETE transaction.
	 *
	 * The worker's message number is updated.
	 * @return tag
	 */
	int make_mpi_tag_for_DELETE(){
		return make_mpi_tag(SIPMPIData::DELETE, section_number_, message_number_++);
	}

	/**
	 * Called by worker to construct mpi tag for a END_PROGRAM transaction.
	 *
	 * @return tag
	 */
	int make_mpi_tag_for_END_PROGRAM(){
		return make_mpi_tag(SIPMPIData::DELETE, section_number_, message_number_++);
	}

	/**
	 * Called by worker to construct mpi tag for a SET_PERSISTENT transaction.
	 *
	 * The worker's message number is updated.
	 * @return tag
	 */
	int make_mpi_tag_for_SET_PERSISTENT(){
		return make_mpi_tag(SIPMPIData::SET_PERSISTENT, section_number_, message_number_++);
	}

	/**
	 * Called by worker to construct mpi tag for a RESTORE_PERSISTENT transaction.
	 *
	 * The worker's message number is updated.
	 * @return tag
	 */
	int make_mpi_tag_for_RESTORE_PERSISTENT(){
		return make_mpi_tag(SIPMPIData::RESTORE_PERSISTENT, section_number_, message_number_++);
	}


	/**
	 * Called by worker at barrier to increment section_number_ and reset message_number_;
	 */
	void barrier(){
		section_number_++;
		message_number_ = 0;
	}

private:
	/**
	 * Last pardo section for which a request was served.
	 * The invariant for correctness is
	 * section(any_incoming_request at server) >= section_number_
	 */
	int section_number_;
	int message_number_;

friend class SIPServer;
friend class Interpreter;
};

} /* namespace sip */

#endif /* BARRIER_SUPPORT_H_ */
