/*
 * sial_printer.h
 *
 *  Created on: Jul 23, 2014
 *      Author: basbas
 */

#ifndef SIAL_PRINTER_H_
#define SIAL_PRINTER_H_


#include "block.h"
#include "block_id.h"

namespace sip {

class Interpreter;

/** Manages formatting and destination of SIAL print statements
 *
 */
class SialPrinter {
public:
	SialPrinter(std::ostream& out, int my_rank);
	virtual ~SialPrinter();

	void print_string(std::string s);
	void print_int(std::string name, int value, int line_number);
	void print_int_value(int value);
	void print_scalar(std::string name, double value, int line_number);
	void print_scalar_value(double value);
	void print_index(std::string name, int value, int line_number);
	void print_block(BlockId id, Block::BlockPtr block, int line_number);
	void print_contiguous(int array_slot, Block::BlockPtr block, int line_number);
	void set_print_all_ranks(bool setting) ;
	void enable_all_rank_print();
	void disable_all_rank_print();
	void endl();
	std::ostream& get_ostream();

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
};

/** DO NOT CHANGE THIS CLASS!!!  T
 *  Some of the tests in the test suite compare expected output with
 *  output from this object, and changing the formatting will break
 *  those test cases.
 *
 *  This is currently the default for expediency only.  We can
 *  and should create a new subclass with your desired behavior and
 *  change the default.
 */

class SialPrinterForTests: public SialPrinter {
public:
	SialPrinterForTests(std::ostream& out, int my_mpi_rank, const SipTables& sip_tables);
	virtual ~SialPrinterForTests();
private:
	const SipTables& sip_tables_;

	std::string BlockId2String(const BlockId& id);
	void do_print_string(std::string s);
	void do_print_int(std::string name, int value, int line_number);
	void do_print_int_value(int value) ;
	void do_print_scalar(std::string name, double value, int line_number);
	void do_print_scalar_value(double value) ;
	void do_print_index(std::string name, int value, int line_number);
	void do_print_block(const BlockId& id, Block::BlockPtr block, int line_number);
	void do_print_contiguous(int array_slot, Block::BlockPtr block, int line_number);
	friend class Interpreter;

};




}/* namespace sip */

#endif /* SIAL_PRINTER_H */
