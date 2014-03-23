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

class SIPMPIData {
public:

	//TODO  rename this to SIPMPIConstants

/*Note that if we need to add more message types while keeping
 * the tag length to four bits, we can use the same tag for
 * the message and its ack.  There is no ambiguity since
 * messages go from worker to server, and acks from server to
 * worker.
 */

#define SIP_MESSAGE_TYPES \
SIP_MESSAGE(DELETE, 0, "DELETE")\
SIP_MESSAGE(DELETE_ACK, 1, "DELETE_ACK")\
SIP_MESSAGE(GET, 2, "GET")\
SIP_MESSAGE(GET_DATA, 3, "GET_DATA")\
SIP_MESSAGE(PUT, 4, "PUT")\
SIP_MESSAGE(PUT_DATA, 5, "PUT_DATA")\
SIP_MESSAGE(PUT_ACCUMULATE, 6, "PUT_ACCUMULATE")\
SIP_MESSAGE(PUT_ACCUMULATE_DATA, 7, "PUT_ACCUMULATE_DATA")\
SIP_MESSAGE(BARRIER, 8, "BARRIER")\
SIP_MESSAGE(END_PROGRAM, 9, "END_PROGRAM")\
SIP_MESSAGE(SET_PERSISTENT, 10, "SET_PERSISTENT")\
SIP_MESSAGE(RESTORE_PERSISTENT, 11, "RESTORE_PERSISTENT")\
SIP_MESSAGE(PUT_DATA_ACK, 12, "PUT_DATA_ACK")\
SIP_MESSAGE(PUT_ACCUMULATE_DATA_ACK, 13, "PUT_ACCUMULATE_DATA_ACK")\
SIP_MESSAGE(SET_PERSISTENT_ACK, 14, "SET_PERSISTENT_ACK")\
SIP_MESSAGE(RESTORE_PERSISTENT_ACK, 15, "RESTORE_PERSISTENT_ACK")
// Type of message allocated 4 bits only.

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

};

}



#endif /* SIP_MPI_DATA_H_ */
