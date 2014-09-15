/*
 * sial_math.cpp
 *
 *  Created on: Jul 25, 2014
 *      Author: basbas
 */


#include <cmath>
#include <cerrno>
//#include <cstdint>
#include "sip.h"
#include "sial_math.h"
#include "interpreter.h"
#include "sip_interface.h"
namespace sial_math {



/* from http://www.cplusplus.com/reference/cmath/lrint/
 *
 long int lrint  (double x);

 Rounds x to an integral value, using the rounding direction specified by fegetround, and returns it as a value of type long int.

 The value of x rounded to a nearby integral, casted to a value of type long int.
 If the rounded value is outside the range of the return type, the value returned is unspecified, and a domain error or an overflow range error may occur (or none, depending on implementation).

 If a domain error occurs:
 - And math_errhandling has MATH_ERRNO set: the global variable errno is set to EDOM.
 - And math_errhandling has MATH_ERREXCEPT set: FE_INVALID is raised.

 If an overflow range error occurs:
 - And math_errhandling has MATH_ERRNO set: the global variable errno is set to ERANGE.
 - And math_errhandling has MATH_ERREXCEPT set: FE_OVERFLOW is raised.

 TODO figure out what our error checking is for this and other math functions.
 */
int cast_double_to_int(double e) {
	long int long_val = lrint(e);

//		check( std::INT_MIN <= long_val && long_val <=std::INT_MAX,
//				"Conversion from scalar to int is invalid.  The magnitude of the scalar is too large.", line_number());

	return (int) long_val;
}

double pow(double e0, double e1) {
	errno = 0;
	double value = std::pow(e0, e1);
	sip::check(errno != EDOM, "domain error in exponent expression",
			sip::get_line_number());
	sip::check(errno != ERANGE,
			"pole or range error in raise-to-power expression",
			sip::get_line_number());
	return value;
}

double sqrt(double e){
	errno = 0;
	double value = std::sqrt(e);
	sip::check(errno != EDOM, "domain error in sqrt expression",
			sip::get_line_number());
	return value;
}
} /* namespace sip */
