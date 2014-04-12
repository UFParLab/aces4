/*! aces_defs.h  Problem specific constants
 * 
 *
 *  Created on: May 31, 2013
 *      Author: Beverly Sanders
 */

#ifndef PROBLEM_ACES_DEFS_H_
#define PROBLEM_ACES_DEFS_H_

/** Maximum number of dimensions */

const int MAX_RANK = 6;

/** Macro for a 6-d  array of all ones, for convenience.
 * This needs to change if MAX_RANK does
 */
#define ONE_ARRAY {1,1,1,1,1,1}


/** Maximum number of arguments to a special instruction */
const int MAXARGS = 3;


#endif /* PROBLEM_ACES_DEFS_H_ */
