/*
 * sial_printer.h
 *
 *  Created on: Jul 23, 2014
 *      Author: basbas
 */

#ifndef SIAL_PRINTER_H_
#define SIAL_PRINTER_H_


#include "block.h"

namespace sip {

class Interpreter;

/** Manages formatting and destination of SIAL print statements
 *
 */
class SialPrinter {
public:
	SialPrinter(std::ostream& out, int my_rank) :
			out_(out), my_mpi_rank_(my_rank), print_all_mpi_ranks_(0) {
	}

virtual ~SialPrinter() {
}

void print_string(std::string s) {
	do_print_string(s);
}
void print_int(std::string name, int value, int line_number) {
	do_print_int(name, value, line_number);
}
void print_int_value(int value) {
	do_print_int_value(value);
}
void print_scalar(std::string name, double value, int line_number) {
	do_print_scalar(name, value, line_number);
}
void print_scalar_value(double value) {
	do_print_scalar_value(value);
}
void print_index(std::string name, int value, int line_number) {
	do_print_index(name, value, line_number);
}
void print_block(BlockId id, Block::BlockPtr block, int line_number){
	do_print_block(id,  block, line_number);
}
void print_contiguous(int array_slot, Block::BlockPtr block, int line_number){
	 do_print_contiguous(array_slot,  block, line_number);
}

void set_print_all_ranks(bool setting) {
	print_all_mpi_ranks_ = setting;
}
void enable_all_rank_print(){set_print_all_ranks(true);}
void disable_all_rank_print(){set_print_all_ranks(false);}

void endl(){
	out_ << std::endl << std::flush;
}

std::ostream& get_ostream(){
	return out_;
}

protected:
std::ostream& out_;
bool print_all_mpi_ranks_;
int my_mpi_rank_;

virtual void do_print_string(std::string s)=0;
virtual void do_print_int(std::string name, int value, int line_number)=0;
virtual void do_print_int_value(int value)=0;
virtual void do_print_scalar(std::string name, double value, int line_number)=0;
virtual void do_print_scalar_value(double value)=0;
virtual void do_print_index(std::string name, int value, int line_number)=0;
virtual void do_print_block(const BlockId& id, Block::BlockPtr block, int line_number)=0;
virtual void do_print_contiguous(int array_slot, Block::BlockPtr block, int line_number)=0;
}
;

/** DO NOT CHANGE THIS CLASS!!!  T
 *  Some of the tests in the test suite compare expected output with
 *  output from this object, and changing the formatting will break
 *  those test cases.
 *
 *  This is currently the default for expediency only.  We can
 *  and should create a new subclass with your desired behavior and
 *  change the default.
 *  This is only the default for expediency.
 */

class SialPrinterForTests: public SialPrinter {
public:
SialPrinterForTests(std::ostream& out, int my_mpi_rank, const SipTables& sip_tables) :
	SialPrinter(out, my_mpi_rank),
	sip_tables_(sip_tables){
}
virtual ~SialPrinterForTests() {}

private:
const SipTables& sip_tables_;

std::string BlockId2String(const BlockId& id){

		std::stringstream ss;
		int rank = sip_tables_.array_rank(id.array_id());
		ss << sip_tables_.array_name(id.array_id()) ;
		ss << '[';
		int i;
		for (i = 0; i < rank; ++i) {
			ss << (i == 0 ? "" : ",") << id.index_values(i);
		}
		ss << ']';
		return ss.str();
	}


void do_print_string(std::string s) {
	if (my_mpi_rank_ == 0 || print_all_mpi_ranks_)
		out_ << s << std::flush;
}
void do_print_int(std::string name, int value, int line_number) {
	if (my_mpi_rank_ == 0 || print_all_mpi_ranks_) {
		out_ << line_number << ":  " << name << "=" << value << std::endl
				<< std::flush;
	}
}
void do_print_int_value(int value) {
	if (my_mpi_rank_ == 0 || print_all_mpi_ranks_) {
		out_ << value << std::flush;
	}
}
void do_print_scalar(std::string name, double value, int line_number) {
	if (my_mpi_rank_ == 0 || print_all_mpi_ranks_) {
		out_ << line_number << ":  " << name << "=";
		out_.precision(20);
		out_<< value << std::endl
				<< std::flush;
	}
}
void do_print_scalar_value(double value) {
	if (my_mpi_rank_ == 0 || print_all_mpi_ranks_) {
		out_ << value << std::flush;
	}
}
void do_print_index(std::string name, int value, int line_number) {
	if (my_mpi_rank_ == 0 || print_all_mpi_ranks_) {
		out_ << line_number << ":  " << name << "=" << value << std::endl
				<< std::flush;
	}
}
void do_print_block(const BlockId& id, Block::BlockPtr block, int line_number){
		int MAX_TO_PRINT = 1024;
		int size = block->size();
//		const int* extents = block->shape().segment_sizes_;
//		int OUTPUT_ROW_SIZE = extents[0];
		int OUTPUT_ROW_SIZE = block->shape().segment_sizes_[0];
		double* data = block->get_data();
		out_ << line_number << ":  ";
		out_ << "printing " << (size < MAX_TO_PRINT?size:MAX_TO_PRINT);
		out_ << " of " <<size << " elements of block " <<  BlockId2String(id);
		out_ << " in the order stored in memory ";
		int i;
	    for (i = 0; i < size && i < MAX_TO_PRINT; ++i){
	    	if (i%OUTPUT_ROW_SIZE == 0) out_ << std::endl;
	    //	std::cout << std::endl;
	    	out_.width(14);
	    	out_ << *(data+i) << " ";
	    }
	    if (i == MAX_TO_PRINT){
	    	out_ << "....";
	    }
	    out_ << std::endl;
	}

void do_print_contiguous(int array_slot, Block::BlockPtr block, int line_number){
	int MAX_TO_PRINT = 1024;
	int size = block->size();
//		const int* extents = block->shape().segment_sizes_;
//		int OUTPUT_ROW_SIZE = extents[0];
	int OUTPUT_ROW_SIZE = block->shape().segment_sizes_[0];
	double* data = block->get_data();
	out_ << line_number << ":  ";
	out_ << "printing " << (size < MAX_TO_PRINT?size:MAX_TO_PRINT);
	out_ << " of " <<size << " elements of contiguous array " <<  sip_tables_.array_name(array_slot);
	out_ << " in the order stored in memory ";
	int i;
    for (i = 0; i < size && i < MAX_TO_PRINT; ++i){
    	if (i%OUTPUT_ROW_SIZE == 0) out_ << std::endl;
    //	std::cout << std::endl;
    	out_.width(14);
    	out_ << *(data+i) << " ";
    }
    if (i == MAX_TO_PRINT){
    	out_ << "....";
    }
    out_ << std::endl;
}


friend class Interpreter;

};




}/* namespace sip */

#endif /* SIAL_PRINTER_H */
