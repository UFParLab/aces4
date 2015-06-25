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

#ifdef _OPENMP
bool Interpreter::done_once = false;
#endif

} /* namespace sip */
