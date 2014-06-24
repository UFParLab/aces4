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



class OpTableEntry {
public:
	OpTableEntry();
	~OpTableEntry();

	static void read(OpTableEntry&, setup::InputStream&);
	friend class OpTable;
	friend std::ostream& operator<<(std::ostream&, const OpTableEntry &);
private:
	opcode_t opcode;
	int op1_array;  //also user_sub, and rank in push_block_selector_op
	int op2_array;
	int result_array;
	int selector[MAX_RANK];
//	int user_sub;
	int line_number;
//	DISALLOW_COPY_AND_ASSIGN(OpTableEntry);
};

class OpTable{
public:
	OpTable();
	~OpTable();

	static void read(OpTable&, setup::InputStream&);
	friend std::ostream& operator<<(std::ostream&, const OpTable&);
	friend class SipTables;
	friend class Interpreter;


	//* inlined accesor functions for optable contents
	opcode_t opcode(int pc) const{
		return entries_.at(pc).opcode;
	}

	int op1_array(int pc) const{
		return entries_.at(pc).op1_array;
	}

	int op2_array(int pc){
		return entries_.at(pc).op2_array;
	}

	int result_array(int pc) const{
		return entries_.at(pc).result_array;
	}

	int print_index(int pc) const{
		return entries_.at(pc).result_array;
	}

	int user_sub(int pc) const{
		return entries_.at(pc).op1_array;  //shares slot
	}

	int line_number(int pc) const{
		return entries_.at(pc).line_number;
	}
	int go_to_target(int pc) const{
		return entries_.at(pc).result_array;
	}

	int size() const{
		return entries_.size();
	}

	int num_indices(int pc) const{
		return entries_.at(pc).op1_array;
	}

    sip::index_selector_t& index_selectors(int pc){
    	return entries_.at(pc).selector;
    }

	int do_index_selector(int pc) const{
		return entries_.at(pc).selector[0];
	}

private:
	std::vector<OpTableEntry> entries_;
	DISALLOW_COPY_AND_ASSIGN(OpTable);
};

} /* namespace worker */
#endif /* OPTABLE_H_ */


