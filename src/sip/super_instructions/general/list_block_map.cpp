/*
 * DO NOT MODIFY.  THIS CLASS IS USED IN SEVERAL TEST PROGRAMS
 *
 *
 * Prints the ids of currently defined blocks.  This is primarily intended for
 * testing and debugging purposes.  It violates all the normal encapsulation
 * rules for super instructions
 *
 *  Created on: Aug 25, 2013
 *      Author: Beverly Sanders
 */


#include <iostream>
#include "sip_interface.h"
#include "interpreter.h"
#include "sialx_interpreter.h"
#include "data_manager.h"
#include "sial_printer.h"


// Super instruction built specifically for tests
// Use only with SialxInterpreter.
void list_block_map(){
	std::ostream& out = sip::Interpreter::global_interpreter()->printer()->get_ostream();
	out <<"LISTING CURRENT BLOCKS" << std::endl;
	out <<  static_cast<sip::SialxInterpreter*>(sip::Interpreter::global_interpreter())->data_manager().block_manager();
	out << std::endl;
//   std::cout <<"LISTING CURRENT BLOCKS" << std::endl;
//   std::cout <<  sip::Interpreter::global_interpreter->data_manager_.block_manager_;
//   std::cout << std::endl;

}
