/*
 * disable_debug_print.cpp
 *
 *	Calling this super instruction disables debug printing,
 *	if the aces4 build is built with the --enable-development flag
 *	Printing is enabled by default.
 *
 *  Created on: Apr 16, 2014
 *      Author: njindal
 */


#include <iostream>
#include "config.h"
#include "sip.h"


void disable_debug_print(){
	sip::_sip_debug_print = false;
}

