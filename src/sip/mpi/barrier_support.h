/**
 * This class manages tags in a way that supports the barrier implementation, and also
 * provides each transaction with a unique id.
 *
 * A barrier in SIAL is implemented with a distributed termination detection algorithm in the SIP.
 * This class attempts to encapsulate the implementation of the termination detection algorithm.
 *
 * Termination Detection algorithm:
 *
 * There are two types of processes involved--workers and servers.  A SIAL program proceeds in
 * a sequence of "sections" separated by barriers. The SIP maintains the current section number
 * at each worker, and the latest section seen at each server.  All messages from workers to
 * servers are acknowledged--either by replying to a request for data or with an explicit ack.
 * Termination of the current section follows from the condition that
 * for all workers:  worker is at the barrier and all messages sent by the worker have been acked.
 * Thus when a worker reaches a barrier, it waits until all of its messages have been acked and
 * invokes an MPI barrier.  Global termination of the section is indicated by termination of the
 * MPI barrier.
 *
 * The server updates its section number each time it receives a message, and checks that the sequence
 * of section numbers is monotone.
 *
 *
 *
 * Other considerations:
 *
 * It is useful for each "transaction" initiated by a worker to have a unique identifier. To
 * accomplish this, a transaction counter is also maintained.  Each message belonging to a transaction--
 * for example the put and corresponding put_data messages--has the same transaction number.
 * The section number, transaction number, and message type are combined to form the message tag.
 *
 * Because this class is entirely small inlined functions, it does not have a corresponding .cpp file.
 *
 */

#ifndef BARRIER_SUPPORT_H_
#define BARRIER_SUPPORT_H_

#include <sstream>
#include "sip_mpi_constants.h"
#include "sip_interface.h"

namespace sip {

class BarrierSupport {
public:

	const static int NUM_MESSAGE_TYPE_BITS = 4;
	const static int NUM_TRANSACTION_NUMBER_BITS = 26;
	const static int NUM_PADDING_BITS = 1;

	BarrierSupport():
		section_number_(0),
		transaction_number_(0){
	}
	~BarrierSupport(){}


/** Utility functions for dealing with tags **/

	/**
	 * Each tag (a 32 bit integer) contains these fields
	 */
	typedef struct {
        unsigned int :						NUM_PADDING_BITS;
		unsigned int message_type : 		NUM_MESSAGE_TYPE_BITS;
		unsigned int transaction_number : 	NUM_TRANSACTION_NUMBER_BITS;
        unsigned int :						NUM_PADDING_BITS;
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
	static SIPMPIConstants::MessageType_t extract_message_type(int mpi_tag){
		SIPMPITagBitFieldConverter bc;
        bc.i = 0;
		bc.i = mpi_tag;
		return SIPMPIConstants::intToMessageType(bc.bf.message_type);
	}

	/**
	 * Extracts the message number from an MPI_TAG.
	 * @param mpi_tag
	 * @return
	 */
	static int extract_transaction_number(int mpi_tag){
		SIPMPITagBitFieldConverter bc;
        bc.i = 0;
		bc.i = mpi_tag;
		return bc.bf.transaction_number;
	}

	/**
	 * Constructs a tag from its constituent parts
	 * @param message_type
	 * @param section_number
	 * @param transaction_number
	 * @return
	 */
	static int make_mpi_tag(SIPMPIConstants::MessageType_t message_type, int transaction_number){
		SIPMPITagBitFieldConverter bc;
        bc.i = 0;
		bc.bf.message_type = message_type;
		bc.bf.transaction_number = transaction_number;
		return bc.i;
	}


/** Routines called by the server */


	/**
	 * Called by the server loop for each message it receives.  Extracts message_type and transaction_number,
	 * @param tag
	 * @param message_type
	 * @param section_number
	 * @param transaction_number
	 * @return
	 */
	void decode_tag(int mpi_tag, SIPMPIConstants::MessageType_t& message_type, int& transaction_number){
		message_type = extract_message_type(mpi_tag);
		transaction_number = extract_transaction_number(mpi_tag);
	}

	/**
	 * Checks the invariant, and updates the section number.
	 * Returns true if the section number was updated.
	 * @param section_number
	 * @return true if section number was different that this->section_number_.
	 */
	bool check_section_number_invariant(int section_number) {
		bool section_number_changed = false;
		CHECK(section_number >= this->section_number_,
				"Section number invariant violated. Received request from an older section !");
		if (section_number > this->section_number_) {
			this->section_number_ = section_number;
			section_number_changed = true;
		}
		return section_number_changed;
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
		int transaction_number = extract_transaction_number(put_accumulate_tag);
		return make_mpi_tag(SIPMPIConstants::PUT_ACCUMULATE_DATA, transaction_number_);
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
		SIPMPIConstants::MessageType_t message_type = extract_message_type(put_tag);
		int transaction_number = extract_transaction_number(put_tag);
		return make_mpi_tag(SIPMPIConstants::PUT_DATA, transaction_number_);
	}


// Routines called by workers

	/**
	 * Called by worker to construct mpi tag for a GET transaction.
	 *
	 * The worker's transaction number is updated.
	 * @return tag
	 */
	int make_mpi_tag_for_GET(){
		int tag = make_mpi_tag(SIPMPIConstants::GET, transaction_number_);
		transaction_number_ = (transaction_number_ + 1) % (1 << NUM_TRANSACTION_NUMBER_BITS);
		return tag;
	}

	/**
	 * Called by the worker to construct mpi tags for a PUT transaction.
	 * The transaction number is the same in both messages.
	 *
	 * The worker's transaction number is updated.
	 *
	 * @param [out] put_data_tag contains the PUT_DATA tag
	 * @return PUT tag
	 */
	int make_mpi_tags_for_PUT(int& put_data_tag){
		int put_tag = make_mpi_tag(SIPMPIConstants::PUT, transaction_number_);
		put_data_tag = make_mpi_tag(SIPMPIConstants::PUT_DATA, transaction_number_);
		transaction_number_ = (transaction_number_ + 1) % (1 << NUM_TRANSACTION_NUMBER_BITS);
		return put_tag;
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
		int put_accumulate_tag = make_mpi_tag(SIPMPIConstants::PUT_ACCUMULATE, transaction_number_);
		put_accumulate_data_tag = make_mpi_tag(SIPMPIConstants::PUT_ACCUMULATE_DATA, transaction_number_);
		transaction_number_ = (transaction_number_ + 1) % (1 << NUM_TRANSACTION_NUMBER_BITS);
		return put_accumulate_tag;
	}

	/**
	 * Called by worker to construct mpi tag for a DELETE transaction.
	 *
	 * The worker's message number is updated.
	 * @return tag
	 */
	int make_mpi_tag_for_PUT_INITIALIZE(){
		int tag = make_mpi_tag(SIPMPIConstants::PUT_INITIALIZE, transaction_number_);
		transaction_number_ = (transaction_number_ + 1) % (1 << NUM_TRANSACTION_NUMBER_BITS);
		return tag;
	}

	/**
	 * Called by worker to construct mpi tag for a DELETE transaction.
	 *
	 * The worker's message number is updated.
	 * @return tag
	 */
	int make_mpi_tag_for_PUT_INCREMENT(){
		int tag = make_mpi_tag(SIPMPIConstants::PUT_INCREMENT, transaction_number_);
		transaction_number_ = (transaction_number_ + 1) % (1 << NUM_TRANSACTION_NUMBER_BITS);
		return tag;
	}

	/**
	 * Called by worker to construct mpi tag for a DELETE transaction.
	 *
	 * The worker's message number is updated.
	 * @return tag
	 */
	int make_mpi_tag_for_PUT_SCALE(){
		int tag = make_mpi_tag(SIPMPIConstants::PUT_SCALE, transaction_number_);
		transaction_number_ = (transaction_number_ + 1) % (1 << NUM_TRANSACTION_NUMBER_BITS);
		return tag;
	}


	/**
	 * Called by worker to construct mpi tag for a DELETE transaction.
	 *
	 * The worker's message number is updated.
	 * @return tag
	 */
	int make_mpi_tag_for_DELETE(){
		int tag = make_mpi_tag(SIPMPIConstants::DELETE, transaction_number_);
		transaction_number_ = (transaction_number_ + 1) % (1 << NUM_TRANSACTION_NUMBER_BITS);
		return tag;
	}

	/**
	 * Called by worker to construct mpi tag for a END_PROGRAM transaction.
	 *
	 * @return tag
	 */
	int make_mpi_tag_for_END_PROGRAM(){
		int tag = make_mpi_tag(SIPMPIConstants::END_PROGRAM, transaction_number_);
		transaction_number_ = (transaction_number_ + 1) % (1 << NUM_TRANSACTION_NUMBER_BITS);
		return tag;
	}

	/**
	 * Called by worker to construct mpi tag for a SET_PERSISTENT transaction.
	 *
	 * The worker's message number is updated.
	 * @return tag
	 */
	int make_mpi_tag_for_SET_PERSISTENT(){
		int tag = make_mpi_tag(SIPMPIConstants::SET_PERSISTENT, transaction_number_);
		transaction_number_ = (transaction_number_ + 1) % (1 << NUM_TRANSACTION_NUMBER_BITS);
		return tag;
	}

	/**
	 * Called by worker to construct mpi tag for a RESTORE_PERSISTENT transaction.
	 *
	 * The worker's message number is updated.
	 * @return tag
	 */
	int make_mpi_tag_for_RESTORE_PERSISTENT(){
		int tag = make_mpi_tag(SIPMPIConstants::RESTORE_PERSISTENT, transaction_number_);
		transaction_number_ = (transaction_number_ + 1) % (1 << NUM_TRANSACTION_NUMBER_BITS);
		return tag;
	}


	/**
	 * Called by worker at barrier to increment section_number_ and reset transaction_number_;
	 */
	void update_state_at_barrier(){
		section_number_++;
		transaction_number_ = 0;
	}

	static std::string tag_to_str(int tag){
		std::stringstream ss;
		ss <<  "get_tag: "
			<< " transaction number "
			<< extract_transaction_number(tag)
			<< " tag type "
			<< extract_message_type(tag)
			<< std::endl;
		return ss.str();
	}

	friend std::ostream& operator<<(std::ostream& os, const BarrierSupport& obj){
		os << "section number = " << obj.section_number_ << ": transaction number = " << obj.transaction_number_ << std::endl;
		return os;
	}

	int section_number() { return section_number_; }

private:
	/**
	 * At the worker, section_number is the current section (starting at 0) or the number of barriers that have been executed.
	 *
	 * At the server, this is the largest section number seen so far.  It is an error if a message arrives with a smaller section number.
	 * This condition is checked in method decode_tag_and_check_invariant.
	 */
	unsigned int section_number_;

	/**
	 * The number of the transaction in the current section.
	 * Transaction numbers wrap around after (2^NUM_TRANSACTION_NUMBER_BITS)
	 */
	unsigned int transaction_number_;

	DISALLOW_COPY_AND_ASSIGN(BarrierSupport);

};

} /* namespace sip */

#endif /* BARRIER_SUPPORT_H_ */
