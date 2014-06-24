/*
 * opcode.cpp
 *
 *  Created on: Jun 22, 2014
 *      Author: basbas
 */

#include <opcode.h>
#include <stdexcept>

namespace sip {

opcode_t intToOpcode(int opcode) {
	if (101 <= opcode && opcode < last_opcode) {
		return (opcode_t) opcode;
	}
	throw std::domain_error("illegal opcode value");
}

std::string opcodeToName(opcode_t op){
	switch(op){
#define SIPOP(e,n,t,p) case e: return std::string(t);
	SIP_OPCODES
#undef SIPOP
	}
	sip::fail("Interal error ! opcode not recognized !");
	return std::string("");
}

bool printableOpcode(opcode_t op){
	switch(op){
#define SIPOP(e,n,t,p) case e: return p;
	SIP_OPCODES
#undef SIPOP
	}
	return false;
}

} /* namespace sip */
