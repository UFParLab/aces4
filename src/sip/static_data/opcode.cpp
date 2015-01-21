/*
 * opcode.cpp
 *
 *  Created on: Jun 22, 2014
 *      Author: basbas
 */

#include "opcode.h"
#include <stdexcept>
#include <cstdio>
#include <iostream>

namespace sip {

opcode_t intToOpcode(int opcode) {
	if (goto_op <= opcode && opcode < invalid_op) {
//		return (opcode_t) opcode;
		return static_cast<opcode_t>(opcode);
	}
	std::cout << "intToOpcode called with param " << opcode << std::endl;
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

int  opcodeToInt(opcode_t op){
	switch(op){
#define SIPOP(e,n,t,p) case e: return n;
	SIP_OPCODES
#undef SIPOP
	}
	return 0;  //this shoudln't happen
}

} /* namespace sip */
