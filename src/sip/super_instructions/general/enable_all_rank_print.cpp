/*
 * enable_all_rank_print.cpp
 *
 *  Created on: Jun 2, 2014
 *      Author: njindal
 */

#include "config.h"
#include "sip.h"
#include "interpreter.h"
#include "sial_printer.h"

void enable_all_rank_print(){
	sip::Interpreter::global_interpreter->printer_->enable_all_rank_print();
}


