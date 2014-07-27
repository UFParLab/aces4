/*
 * OpTable.h
 *
 *  Created on: Jul 15, 2013
 *      Author: basbas
 */

#ifndef OPTABLE_H_
#define OPTABLE_H_

#include <vector>
#include "sip.h"
#include "aces_defs.h"
#include "io_utils.h"
#include "opcode.h"


namespace sip {

class Tracer;

class OpTableEntry {
public:
	OpTableEntry();
	~OpTableEntry();

	static void read(OpTableEntry&, setup::InputStream&);
	friend class OpTable;
	friend std::ostream& operator<<(std::ostream&, const OpTableEntry &);
private:
	opcode_t opcode;
	int arg0;  //also user_sub, and rank in push_block_selector_op
	int arg1;
	int arg2;
	int selector[MAX_RANK];
	int line_number;
//	DISALLOW_COPY_AND_ASSIGN(OpTableEntry);
};

class OpTable{
public:
	OpTable();
	~OpTable();

	static void read(OpTable&, setup::InputStream&);


	/** inlined accessor functions for optable contents */
	opcode_t opcode(int pc) const{
		return entries_.at(pc).opcode;
	}

	int arg0(int pc) const{
		return entries_.at(pc).arg0;
	}

	int arg1(int pc){
		return entries_.at(pc).arg1;
	}

	int arg2(int pc) const{
		return entries_.at(pc).arg2;
	}

    sip::index_selector_t& index_selectors(int pc){
    	return entries_.at(pc).selector;
    }

	int line_number(int pc) const{
		return entries_.at(pc).line_number;
	}

	int size() const{
		return entries_.size();
	}

	int max_line_number() const{
		return entries_.at(size()-1).line_number;
	}



	/** a few useful accessor functions to somewhat abstract away optable layout
	 * probably, these should go away
	 * */
	int print_index(int pc) const{
		return entries_.at(pc).arg2;
	}

	int user_sub(int pc) const{
		return entries_.at(pc).arg0;  //shares slot
	}


	int go_to_target(int pc) const{
		return entries_.at(pc).arg2;
	}



	int num_indices(int pc) const{
		return entries_.at(pc).arg0;
	}



	int do_index_selector(int pc) const{
		return entries_.at(pc).selector[0];
	}

	friend std::ostream& operator<<(std::ostream&, const OpTable&);

	friend class SipTables;
	friend class Interpreter;
	friend class Tracer;

private:
	std::vector<OpTableEntry> entries_;
	DISALLOW_COPY_AND_ASSIGN(OpTable);
};

} /* namespace worker */
#endif /* OPTABLE_H_ */


