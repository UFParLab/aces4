
#include <sip_mpi_constants.h>

#include <iostream>
#include "sip.h"

namespace sip{

SIPMPIConstants::MessageType_t SIPMPIConstants::intToMessageType(int msgtype) {
	if (0 <= msgtype && msgtype < LAST_MESSAGE_TYPE) {
		return (MessageType_t) msgtype;
	}
	std::cerr << "illegal argument to SIPMPIConstants::intToMessageType " << msgtype << std::endl << std::flush;
	throw std::domain_error("illegal msgtype value");
}

std::string SIPMPIConstants::messageTypeToName(SIPMPIConstants::MessageType_t m){
	switch(m){
	#define SIP_MESSAGE(e,n,s) case e: return std::string(s);
		SIP_MESSAGE_TYPES
	#undef SIP_MESSAGE
	}
	sip::fail("Interal error ! message type not recognized !");
	return std::string("");
}

};
