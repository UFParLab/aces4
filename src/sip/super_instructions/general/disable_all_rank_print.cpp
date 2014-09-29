/*
 * disable_all_rank_print.cpp
 *
 *  Created on: Jun 2, 2014
 *      Author: njindal
 */

#include "config.h"
#include "sip.h"
#include "interpreter.h"
#include "sial_printer.h"

void disable_all_rank_print(){
//	sip::_all_rank_print = false;
	sip::SialxInterpreter::global_interpreter->printer()->disable_all_rank_print();
}


