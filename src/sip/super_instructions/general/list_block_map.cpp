/*
 * list_block_map.pcc
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
#include "data_manager.h"

void list_block_map(){
   std::cout <<"LISTING CURRENT BLOCKS" << std::endl;
   std::cout <<  sip::Interpreter::global_interpreter->data_manager_.block_manager_;
   std::cout << std::endl;

}
