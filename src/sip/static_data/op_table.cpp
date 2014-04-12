/*
 * OpTable.cpp
 *
 *  Created on: Jul 15, 2013
 *      Author: Beverly Sanders
 */

#include "op_table.h"
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
	return std::string("");
}

bool printableOpcode(opcode_t op){
	switch(op){
#define SIPOP(e,n,t,p) case e: return p;
#undef SIPOP
	}
	return false;
}

OpTableEntry::OpTableEntry() {}

OpTableEntry::~OpTableEntry() {}

void OpTableEntry::read(OpTableEntry &entry, setup::InputStream &file) {

	entry.opcode = intToOpcode(file.read_int());
	entry.op1_array = file.read_int();
	entry.op2_array = file.read_int();
	entry.result_array = file.read_int();
	for (int i = 0; i < MAX_RANK; ++i) { //the compiler generated sio format requires
									   //ints to be read individually rather than
									   //using read_int_array.  It does not write
									   //the array size first.
		entry.selector[i] = file.read_int();
	}
	entry.line_number = file.read_int();
}


std::ostream& operator<<(std::ostream& os, const OpTableEntry & entry) {
	os << opcodeToName(entry.opcode) << ':';
	os << entry.opcode << ',';
	os << entry.op1_array << ',';
	os << entry.op2_array << ',';
	os << entry.result_array << ",[";
	for (int i = 0; i < MAX_RANK; ++i) {
		os << entry.selector[i] << (i < MAX_RANK - 1 ? "," : "],");
	}
	os << entry.line_number;
	return os;
}


OpTable::OpTable() {
}
OpTable::~OpTable() {
}

void OpTable::read(OpTable& optable, setup::InputStream &file) {
	int n = file.read_int(); //number of entries
	for (int i = 0; i < n; ++i) {
		OpTableEntry entry;
		OpTableEntry::read(entry, file);
		optable.entries_.push_back(entry);
	}
}

std::ostream& operator<<(std::ostream& os, const OpTable & opTableObj) {
	std::vector<OpTableEntry>::iterator it;
	int i = 0;
	std::vector<OpTableEntry> entries = opTableObj.entries_;
	for (it = entries.begin(); it != entries.end(); ++it) {
		os << i++ << ":" << *it << std::endl;
	}
    return os;
}

} /* namespace worker */
