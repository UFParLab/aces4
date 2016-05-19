/*
 * sip_interface.h
 *
 *  Created on: Aug 10, 2013
 *      Author: basbas
 */

#ifndef SIP_INTERFACE_H_
#define SIP_INTERFACE_H_
#include <string>

#ifdef __cplusplus
extern "C" {
#endif


void predefined_scalar_array(const char*aname, int& num_dims, int **dims, double **values);
void predefined_int_array(const char*aname, int& num_dims, int **dims, int **values);
void scratch_array(int& num_elements, double **array);
void delete_scratch_array(double **array);
void integer_scratch_array(int& num_elements, int **array);
void delete_integer_scratch_array(int **array);

int query_int_constant(const char*cname);
int query_scalar_constant(const char*cname);
int int_constant(const char*cname);
double scalar_constant(const char*cname);
void set_scalar_value(const char * name, double value);
double scalar_value(const char *);

int current_line();
void get_config_info(const char* sialfile, const char* key, const char* value);
int mpi_rank();


#ifdef __cplusplus
 }
#endif


//these routines are not callable from fortran.
namespace sip {

std::string index_name_value(int index_table_slot);
std::string array_name_value(int array_table_slot);
int get_line_number();
//array::Block::BlockPtr block(std::string name);
int get_mpi_rank();

} /* namespace sip */

#endif /* SIP_INTERFACE_H_ */
