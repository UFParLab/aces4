/**
 * setup_sip_interface.h
 *
 * Provides access to data read from the setup file to fortran programs.
 * This should only be used for basis set information and other data that
 * is not part of the sial program.  If it is part of the sial program, it
 * should be passed as an argument to the superinstruction rather than
 * accessed here.
 *
 *  Created on: Oct 28, 2013
 *      Author: basbas
 */

#ifndef SETUP_ACES_INTERFACE_H_
#define SETUP_ACES_INTERFACE_H_

/** The following routines are callable from Fortran */

#ifdef __cplusplus
extern "C" {
#endif

/**  Specify static predefined arrays
 *
 * @param aname  Name of the array
 * @param num_dims
 * @param dims array of extents
 * @param values 1-d array containing the data contained in the array
 */

void get_predefined_array(const char*aname, int& num_dims, int *dims, double *values);

/**
 *
 * @param sialfile
 * @param key
 * @param value
 */
void get_config_info(const char* sialfile, const char* key, const char* value);

#ifdef __cplusplus
   }
#endif
#endif /* SETUP_ACES_INTERFACE_H_ */
