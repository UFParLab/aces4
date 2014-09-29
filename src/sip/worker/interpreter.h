/*
 * interpreter.h
 *
 *  Created on: Sep 28, 2014
 *      Author: jindal
 */

#ifndef INTERPRETER_H_
#define INTERPRETER_H_

namespace sip {

class SialPrinter;

class Interpreter {
public:
	virtual ~Interpreter(){}

	/** main interpret function */
	void interpret() { do_interpret(); }

	/**
	 *  Should be called after the main interpret loop terminates to
	 *  cleanly terminate this program.  This is the responsibility of
	 *  the main program to allow more convenient testing of parallel
	 *  programs that don't use servers.
	 */
	void post_sial_program() { do_post_sial_program(); }

	/**
	 * Returns the line number in the SIAL program corresponding to the
	 * instruction at op_table[pc].
	 * @return -1 if pc is out of range of the op_table.
	 */
	int line_number() { return get_line_number(); }

	/** Allows superinstructions to use the same ostream as sial. */
	SialPrinter* printer() { return get_printer(); }

	/** Static pointer to the current Interpreter.  This is
	 * initialized in the Interpreter constructor and reset to NULL
	 * in its destructor.  There should be at most on Interpreter instance
	 * at any given time.
	 */
	static Interpreter* global_interpreter;

protected:
	Interpreter() : pc_(0){ global_interpreter = this; }
	int pc_; 		/*! the "program counter". Actually, the current location in the op_table_.	 */

	virtual SialPrinter* get_printer() = 0;
	virtual int get_line_number() = 0;
	virtual void do_post_sial_program() = 0;
	virtual void do_interpret() = 0;

};

} /* namespace sip */

#endif /* INTERPRETER_H_ */
