/*
 * sial_printer.cpp
 *
 *  Created on: Aug 12, 2014
 *      Author: njindal
 */

/*
 * sial_printer.h
 *
 *  Created on: Jul 23, 2014
 *      Author: basbas
 */

#include "sial_printer.h"

#include <sstream>
#include <iostream>

#include "block.h"
#include "block_id.h"
#include "sip_tables.h"

namespace sip {

/** Manages formatting and destination of SIAL print statements
 *
 */
SialPrinter::SialPrinter(std::ostream& out, int my_rank) :
			out_(out), my_mpi_rank_(my_rank), print_all_mpi_ranks_(0) {}

SialPrinter::~SialPrinter() {}

void SialPrinter::print_string(std::string s) {
	if (my_mpi_rank_==0 || print_all_mpi_ranks_) do_print_string(s);
}
void SialPrinter::print_int(std::string name, int value, int line_number) {
	if (my_mpi_rank_==0 || print_all_mpi_ranks_) do_print_int(name, value, line_number);
}
void SialPrinter::print_int_value(int value) {
	if (my_mpi_rank_==0 || print_all_mpi_ranks_) do_print_int_value(value);
}
void SialPrinter::print_scalar(std::string name, double value, int line_number) {
	if (my_mpi_rank_==0 || print_all_mpi_ranks_) do_print_scalar(name, value, line_number);
}
void SialPrinter::print_scalar_value(double value) {
	if (my_mpi_rank_==0 || print_all_mpi_ranks_) do_print_scalar_value(value);
}
void SialPrinter::print_index(std::string name, int value, int line_number) {
	if (my_mpi_rank_==0 || print_all_mpi_ranks_) do_print_index(name, value, line_number);
}
void SialPrinter::print_block(BlockId id, Block::BlockPtr block, int line_number){
	if (my_mpi_rank_==0 || print_all_mpi_ranks_) do_print_block(id,  block, line_number);
}
void SialPrinter::print_contiguous(int array_slot, Block::BlockPtr block, int line_number){
	if (my_mpi_rank_==0 || print_all_mpi_ranks_) do_print_contiguous(array_slot,  block, line_number);
}

void SialPrinter::set_print_all_ranks(bool setting) {
	print_all_mpi_ranks_ = setting;
}
void SialPrinter::enable_all_rank_print(){set_print_all_ranks(true);}
void SialPrinter::disable_all_rank_print(){set_print_all_ranks(false);}

void SialPrinter::endl(){
	out_ << std::endl << std::flush;
}

std::ostream& SialPrinter::get_ostream(){
	return out_;
}


/** DO NOT CHANGE THIS CLASS!!!  T
 *  Some of the tests in the test suite compare expected output with
 *  output from this object, and changing the formatting will break
 *  those test cases.
 *
 *  This is currently the default for expediency only.  We can
 *  and should create a new subclass with your desired behavior and
 *  change the default.
 */

SialPrinterForTests::SialPrinterForTests(std::ostream& out, int my_mpi_rank, const SipTables& sip_tables) :
	SialPrinter(out, my_mpi_rank),
	sip_tables_(sip_tables){
}
SialPrinterForTests::~SialPrinterForTests() {}


std::string SialPrinterForTests::BlockId2String(const BlockId& id){

		std::stringstream ss;
		bool contiguous_local = sip_tables_.is_contiguous_local(id.array_id());
		if (contiguous_local) ss << "contiguous local ";
		int rank = sip_tables_.array_rank(id.array_id());
		ss << sip_tables_.array_name(id.array_id()) ;
		ss << '[';
		int i;
		for (i = 0; i < rank; ++i) {
			ss << (i == 0 ? "" : ",") << id.index_values(i);
			if (contiguous_local) ss << ":" << id.upper_index_values(i);
		}
		ss << ']';
		return ss.str();
	}


void SialPrinterForTests::do_print_string(std::string s) {
	if (my_mpi_rank_ == 0 || print_all_mpi_ranks_)
		out_ << s << std::endl << std::flush;
}
void SialPrinterForTests::do_print_int(std::string name, int value, int line_number) {
	if (my_mpi_rank_ == 0 || print_all_mpi_ranks_) {
		out_ << line_number << ":  " << name << "=" << value << std::endl
				<< std::flush;
	}
}
void SialPrinterForTests::do_print_int_value(int value) {
	if (my_mpi_rank_ == 0 || print_all_mpi_ranks_) {
		out_ << value << std::flush;
	}
}
void SialPrinterForTests::do_print_scalar(std::string name, double value, int line_number) {
	if (my_mpi_rank_ == 0 || print_all_mpi_ranks_) {
		out_ << line_number << ":  " << name << "=";
		out_.precision(20);
		out_.setf(std::ios_base::fixed);
		out_<< value << std::endl
				<< std::flush;
	}
}
void SialPrinterForTests::do_print_scalar_value(double value) {
	if (my_mpi_rank_ == 0 || print_all_mpi_ranks_) {
		out_ << value << std::flush;
	}
}
void SialPrinterForTests::do_print_index(std::string name, int value, int line_number) {
	if (my_mpi_rank_ == 0 || print_all_mpi_ranks_) {
		out_ << line_number << ":  " << name << "=" << value << std::endl
				<< std::flush;
	}
}
void SialPrinterForTests::do_print_block(const BlockId& id, Block::BlockPtr block, int line_number){
		int MAX_TO_PRINT = 1024;
		int size = block->size();
		int OUTPUT_ROW_SIZE = block->shape().segment_sizes_[0];
		double* data = block->get_data();
	        out_.precision(14);
		out_.setf(std::ios_base::fixed);
		out_ << line_number << ":  ";
		if (size == 1) {
		    out_ << "printing " << id.str(sip_tables_) << " = ";
		    out_ << *(data);
		} else {
		    out_ << "printing " << (size < MAX_TO_PRINT?size:MAX_TO_PRINT);
		    out_ << " of " <<size << " elements of block " <<  id.str(sip_tables_);//BlockId2String(id);
		    out_ << " in the order stored in memory ";
		    int i;
		    for (i = 0; i < size && i < MAX_TO_PRINT; ++i){
			if (i%OUTPUT_ROW_SIZE == 0) out_ << std::endl;
			out_ << *(data+i) << " ";
		    }
		    if (i == MAX_TO_PRINT){
			out_ << "....";
		    }
		}
		out_ << std::endl;
	}

void SialPrinterForTests::do_print_contiguous(int array_slot, Block::BlockPtr block, int line_number){
	int MAX_TO_PRINT = 1024;
	int size = block->size();
	int OUTPUT_ROW_SIZE = block->shape().segment_sizes_[0];
	double* data = block->get_data();
	out_ << line_number << ":  ";
	out_ << "printing " << (size < MAX_TO_PRINT?size:MAX_TO_PRINT);
	out_ << " of " <<size << " elements of contiguous array " <<  sip_tables_.array_name(array_slot);
	out_ << " in the order stored in memory ";
	int i;
    for (i = 0; i < size && i < MAX_TO_PRINT; ++i){
    	if (i%OUTPUT_ROW_SIZE == 0) out_ << std::endl;
    	out_.width(14);
    	out_ << *(data+i) << " ";
    }
    if (i == MAX_TO_PRINT){
    	out_ << "....";
    }
    out_ << std::endl;
}



}/* namespace sip */



