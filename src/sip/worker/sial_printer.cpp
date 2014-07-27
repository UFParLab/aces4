/*
 * sial_printer.cpp
 *
 *  Created on: Jul 23, 2014
 *      Author: basbas
 */

#include "sial_printer.h"

namespace sip {

void SialPrinter::print_to_ostream(std::ostream& out, std::string to_print){
	out << to_print << std::flush;
}
void SialPrinter::print_to_ostream(std::ostream& out, double value){
	std::stringstream ss;
	ss.precision(20);
	ss << value;
	out << ss.str() << std::flush;
}
void SialPrinter::print_to_ostream(std::ostream& out, int value){
	out << value << std::flush;
}
void SialPrinter::println_to_ostream(std::ostream& out){
	out << std::endl << std::flush;
}

} /* namespace sip */
