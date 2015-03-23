/*
 * interpreter.cpp
 *
 *  Created on: Sep 28, 2014
 *      Author: jindal
 */

#include "interpreter.h"
#include <cstddef>

namespace sip {

std::vector<Interpreter*> Interpreter::global_interpreters_;
bool Interpreter::done_once = false;

} /* namespace sip */
