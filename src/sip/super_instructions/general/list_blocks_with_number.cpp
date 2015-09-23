/*
 * list_blocks_with_number.cpp
 *
 *  Created on: Sep 3, 2015
 *      Author: basbas
 */


#include <iostream>
#include "sip_interface.h"
#include "interpreter.h"
#include "data_manager.h"
#include "sial_printer.h"


void list_blocks_with_number(){
	sip::Interpreter* interpreter = sip::Interpreter::global_interpreter;
	std::ostream& out = interpreter->printer()->get_ostream();
	out <<"LISTING Blocks with their number" << std::endl;
	std::vector<std::pair<sip::BlockId,size_t> > vec;
	interpreter->data_manager().block_manager().block_map().c_list_blocks(interpreter->sip_tables(), vec);
	out << "size of block vector " << vec.size() << std::endl;
	std::vector<std::pair<sip::BlockId,size_t> >::iterator it;
	for (it = vec.begin(); it != vec.end(); ++it){
		out << it->first << ":" << it->second  << std::endl;
	}
    out << std::endl;

}


