#include <sip_mpi_data.h>


namespace sip{

SIPMPIData::MessageType_t SIPMPIData::intToMessageType(int msgtype) {
	if (0 <= msgtype && msgtype < LAST_MESSAGE_TYPE) {
		return (MessageType_t) msgtype;
	}
	throw std::domain_error("illegal msgtype value");
}

};
