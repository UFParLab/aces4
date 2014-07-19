/*
 * sip_mpi_data.h
 *
 *  Created on: Jan 14, 2014
 *      Author: njindal
 */

#ifndef SIP_MPI_DATA_H_
#define SIP_MPI_DATA_H_

#include <stdexcept>

namespace sip{

class SIPMPIConstants {
public:

/* We are using the same tag for
 * a message an its ack. There is no ambiguity since
 * messages go from worker to server, and acks from server to
 * worker.
 */
/** Message tags.  Only four bits available, thus max 15 different tags
 *
 * The same tag is used for a message and its ack.*/
#define SIP_MESSAGE_TYPES \
SIP_MESSAGE(DELETE, 0, "DELETE")\
SIP_MESSAGE(GET, 1, "GET")\
SIP_MESSAGE(PUT, 2, "PUT")\
SIP_MESSAGE(PUT_DATA, 3, "PUT_DATA")\
SIP_MESSAGE(PUT_ACCUMULATE, 4, "PUT_ACCUMULATE")\
SIP_MESSAGE(PUT_ACCUMULATE_DATA, 5, "PUT_ACCUMULATE_DATA")\
SIP_MESSAGE(BARRIER, 6, "BARRIER")\
SIP_MESSAGE(END_PROGRAM, 7, "END_PROGRAM")\
SIP_MESSAGE(SET_PERSISTENT, 8, "SET_PERSISTENT")\
SIP_MESSAGE(RESTORE_PERSISTENT, 9, "RESTORE_PERSISTENT")

	enum MessageType_t {
	#define SIP_MESSAGE(e,n,s) e = n,
		SIP_MESSAGE_TYPES
	#undef SIP_MESSAGE
		LAST_MESSAGE_TYPE
	};

	/**
	 * Returns a MessageType_t for an integer.
	 * @param msgtype
	 * @return
	 */
	static MessageType_t intToMessageType(int msgtype);

	/**
	 * Converts a message type to it's string equivalent
	 * @param
	 * @return
	 */
	static std::string messageTypeToName(MessageType_t);

};

}



#endif /* SIP_MPI_DATA_H_ */
