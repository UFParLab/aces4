/*
 * array_constants.h
 *
 *  Created on: Aug 31, 2013
 *      Author: Beverly Sanders
 */

#ifndef ARRAY_CONSTANTS_H_
#define ARRAY_CONSTANTS_H_

#include <string>
#include "aces_defs.h"

#define DEFAULT_NUM_SUBSEGMENTS 4

namespace sip{
const int unused_index_slot = -1;
const int unused_index_value = 1;
const int unused_index_segment_size = 1;
const int wild_card_slot = 90909;
const int wild_card_value = -2;

/*! typedefs for small arrays of size MAX_RANK*/

//TODO change to size_t instead of int???
typedef int index_selector_t[MAX_RANK];
typedef int index_value_array_t[MAX_RANK];
typedef int segment_size_array_t[MAX_RANK];
typedef int offset_array_t[MAX_RANK];

/*! IndexType_t.  An enum with values for the index types defined in Sial.
 *
 * These constants are also defined in the compiler in sial.codegen.SipConstants.java
 */

#define INDEX_TYPES_T\
	INDEX_TYPE_T(aoindex, 	1001, "aoindex")\
	INDEX_TYPE_T(moindex, 	1002, "moindex")\
	INDEX_TYPE_T(moaindex, 	1003, "moaindex")\
	INDEX_TYPE_T(mobindex, 	1004, "mobindex")\
	INDEX_TYPE_T(simple, 	1005, "simple")\
	INDEX_TYPE_T(laindex, 	1006, "laindex")\
	INDEX_TYPE_T(subindex, 	1007, "subindex")\

enum IndexType_t {
#define INDEX_TYPE_T(e,n,s) e = n,
	INDEX_TYPES_T
#undef INDEX_TYPE_T
	unused=1008
};

//enum IndexType_t {
//	aoindex=1001,
//	moindex=1002,
//	moaindex=1003,
//	mobindex=1004,
//	simple=1005,
//	laindex=1006,
//	subindex=1007,
//	unused=1008
//};

/*! returns the index type given the lower case name of the index ("aoindex", "index", etc.) */
IndexType_t map_index_type(std::string name);

std::string index_type_name(IndexType_t t);

int indexTypeToInt(IndexType_t);

IndexType_t intToIndexType_t(int);

IndexType_t map_index_type(std::string name);


/*! bit flags representing constants for new array attributes these can be combined
 * with bitwise or as below
 * individual flags can be tested with bitwise and--attribute &
 * attr_contiguous is non zero if attribute has attr_contiguous set.
 *
 * Note:  these values must match those in the SIAL compiler
 */
enum ArrayAttribute {
	/*! the modified object is an integer */
	attr_integer = 0x01, //0x01 ==   1 == "00000001"

	/*! a sip consistent object may be replicated in the system, but has a single conceptual "home" that
	 *  represents the object. At certain points in a computation, such as immediately after a barrier,
	 *  all copies of a sip consistent object should contain the same value.
	 */
	attr_sip_consistent = 0x02,     //0x02 ==   2 == "00000010"

	/*! an object is contiguous if all of its elements are laid out linearly in memory
	 * according to their conceptual values.  For example distribute array is not contiguous, but
	 * is divided into blocks which are contiguous.  A static array is contiguous, but its blocks
	 * are not.
	 */
	attr_contiguous =0x04,  //0x04 ==   4 == "00000100"

	/*! auto allocate means that the object is created automatically when needed, rather than explicitly
	 * via a command in a SIAL program
	 */
	attr_auto_allocate = 0x08,  //0x08 ==   8 == "00001000"

	/*! scope extent means that the lifetime of the object is limited to the
	 * scope in which it was created. Scopes are delineated in SIAL program by do and
	 * pardo loop boundaries.  For example, temp arrays in SIAL programs are
	 * auto_allocate, scope_extent objects--they are created automatically when used
	 * and automatically destroyed when the program leaves the scope in which they
	 * were created.
	 */
	attr_scope_extent = 0x10, //0x10 ==  16 == "00010000"
	/*! predefined means that the value of the object was set before the
	 * SIAL program begins execution.
	 */

	attr_predefined = 0x20,  //0x20 ==  32 == "00100000"

	/*! persistent means that the object persists after completion of the
	 * SIAL program and can be referenced in the following program or
	 * accessed by the ACES4 driver for output, etc.
	 */
	attr_persistent = 0x40,  //0x40 ==  64 == "01000000"

	/*! the object has rank 0.  If the integer attribute it set, it is an integer, otherwise it holds a double*/
	attr_scalar = 0x80,  //0x80 == 128 == "10000000"

	/*! distributed or served array is sparse */
	attr_sparse = 0x100 //0x100 == 256 == "100000000"
};


/*! attributes for aces3 SIAL types (served, distribute, temp, local, etc. */
enum ArrayType_t {
	served_array_t = attr_sip_consistent, // ==2
    static_array_t = (attr_contiguous | attr_auto_allocate),  //==12  //TODO CHECK auto_allocate??
    predefined_static_array_t = (static_array_t | attr_predefined), //==44
    distributed_array_t = served_array_t, //==2
    temp_array_t = (attr_auto_allocate | attr_scope_extent), //== 24
    scalar_value_t = (attr_contiguous | attr_auto_allocate | attr_scalar), //==140
    predefined_scalar_value_t = (scalar_value_t | attr_predefined), //=172
    local_array_t = 0, //all defaults give a local_array
    int_value_t = (attr_integer | scalar_value_t),
    contiguous_local_t = (attr_contiguous)
};

bool is_integer_attr(int attr);
bool is_sip_consistent_attr(int attr);
bool is_contiguous_attr(int attr);
bool is_auto_allocate_attr(int attr);
bool is_scope_extent_attr(int attr);
bool is_predefined_attr(int attr);
bool is_persistent_attr(int attr);
bool is_scalar_attr(int attr);
bool is_predefined_scalar_attr(int attr);
bool is_sparse_attr(int attr);
bool is_contiguous_local_attr(int attr);
bool is_temp_attr(int attr);

ArrayType_t intToArrayType_t(int);

}//namespace array

#endif /* ARRAY_CONSTANTS_H_ */
