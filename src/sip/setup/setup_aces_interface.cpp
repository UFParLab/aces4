/*
 * setup_sip_interface.cpp
 *
 *  Created on: Oct 28, 2013
 *      Author: basbas
 */

#include "setup_aces_interface.h"
#include <stdexcept>
#include <string>
#include "interpreter.h"

#ifdef __cplusplus
extern "C" {
#endif


/**  Specify static predefined arrays
 *
 * @param aname [in]  Name of the array
 * @param num_dims [out]
 * @param dims [out] array of extents
 * @param values [out] the data contained in the array
 */

void get_predefined_array(const char*aname, int& num_dims, int *dims, double *values){
	try{
	std::pair<int, std::pair<int *, double *> > a =  sip::Interpreter::global_interpreter->sip_tables_.setup_reader_.predef_arr_.at(std::string(aname));
	num_dims = a.first;
	dims = a.second.first;
	values = a.second.second;
	return;
	}
	catch (const std::out_of_range& oor) {
		CHECK(false, "predefined array " + std::string("aname") + " not in predefined array map");
	    return;
	  }
}

//NOT YET IMPLEMENTED
//void get_config_info(const char* sialfile, const char* key, const char* value);


#ifdef __cplusplus
   }
#endif


