/*
 * sial_math.h
 *
 *  Created on: Jul 25, 2014
 *      Author: basbas
 */

#ifndef SIAL_MATH_H_
#define SIAL_MATH_H_



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
	int cast_double_to_int(double e);

	double pow(double e0, double e1);

	double sqrt(double e);


} /* namespace sial_math */

#endif /* SIAL_MATH_H_ */
