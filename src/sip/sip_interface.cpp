/*
 * interpreter_interface.cpp
 *
 * Wrapper to allow routines to be called from fortran and/or provide a result
 * without requiring the caller to include interpreter.h or deal with namespaces
 * or classes.
 *
 *  Created on: Aug 10, 2013
 *      Author: basbas
 */

#include "config.h"
#include "sip_interface.h"
#include "interpreter.h"
#include "index_table.h"
#include "block.h"
#include <stdexcept>

#ifdef HAVE_MPI
#include "sip_mpi_attr.h"
#include "sip_server.h"
#endif // HAVE_MPI

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Gets a currently active scalar value
 * @param name
 * @return
 */
double scalar_value(const char * name) {
	return sip::Interpreter::global_interpreter->scalar_value(std::string(name));
}

/**
 * Sets the value of a currently active scalar
 * @param name
 * @param value
 */
void set_scalar_value(const char * name, double value) {
	sip::Interpreter::global_interpreter->set_scalar_value(std::string(name),
			value);
}

/**
 * Get a double constant from initialization data
 * @param cname [in]  Name of the constant
 * @return Value of the constant
 */
double scalar_constant(const char*cname) {
	return sip::Interpreter::global_interpreter->sip_tables().setup_reader().predefined_scalar(
			std::string(cname));
}

/**
 * Get an integer constant from initialization data
 * @param cname [in]  Name of the constant
 * @return Value of the constant
 */
int int_constant(const char*cname) {
	return sip::Interpreter::global_interpreter->sip_tables().setup_reader().predefined_int(
			std::string(cname));
}

/**
 * Get predefined scalar array from initialization data
 * @param aname [in]  Name of the array
 * @param num_dims [out]
 * @param dims [out] array of extents
 * @param values [out] the data contained in the array
 */


void predefined_scalar_array(const char*aname, int& num_dims, int **dims,
		double **values) {
	try {
//		std::pair<int, std::pair<int *, double *> > a =
//				sip::Interpreter::global_interpreter->sip_tables().setup_reader().predef_arr_.at(
//						std::string(aname));
//		num_dims = a.first;
//		*dims = a.second.first;
//		*values = a.second.second;
		setup::PredefContigArray rankblock = sip::Interpreter::global_interpreter->sip_tables().setup_reader().predefined_contiguous_array(std::string(aname));
		num_dims = rankblock.first;
		sip::Block::BlockPtr block = rankblock.second;
		*dims = const_cast<sip::segment_size_array_t&>(block->shape().segment_sizes_);
		*values = block->get_data();
		return;
	} catch (const std::out_of_range& oor) {
		sip::check(false,
				"predefined array " + std::string(aname)
						+ " not in predefined array map\n");
		return;
	}
}

void scratch_array(int& num_elements, double **array){
    try{
        double * scratch = new double[num_elements]();  //initialize to zero
        *array = scratch;
    } catch (const std::bad_alloc& ba) {
        std::cerr << "Got bad alloc ! Memory being requestd : " << num_elements << std::endl;
        throw ba;
    }
}

void delete_scratch_array(double **array){
	delete [] *array;
}

void integer_scratch_array(int& num_elements, int **array){
	int * scratch = new int[num_elements]();  //initialize to zero
	*array = scratch;
}

void delete_integer_scratch_array(int **array){
	delete [] *array;
}

/**
 * Get predefined integer array from initialization data
 *
 * @param aname [in] Name of the array
 * @param [out] num_dims
 * @param [out] dims array of extents
 * @param [out] values 1-d array containing the data contained in the array
 */
void predefined_int_array(const char*aname, int& num_dims, int **dims,
		int **values) {
	try {
		setup::PredefIntArray a =
				sip::Interpreter::global_interpreter->sip_tables().setup_reader().predefined_integer_array(std::string(aname));
		//std::cout << "aname= " << aname << ", a=" << a.first << std::endl;
		num_dims = a.rank;
		*dims = a.dims;
		*values = a.data;
        /*
		std::cout << num_dims << "," << *dims << "," << *values << std::endl;
        std::cout<<"Dims :"<<std::endl;
        for(int i=0; i<num_dims; i++){
            std::cout<<(*dims)[i]<< " ";
        }
        std::cout<<std::endl<<"Values :"<<std::endl;
        for (int i=0; i< 10; i++){
            std::cout<<(*values)[i]<<" ";
        }
        */
		return;
	} catch (const std::out_of_range& oor) {
		sip::check(false,
				"predefined array " + std::string(aname)
						+ " not in predefined array map\n");
		return;
	}
}

/**
 * Gets the current SIALX line number begin executed
 * @return
 */
int current_line() {
	return sip::get_line_number();
}

//NOT YET IMPLEMENTED
//void get_config_info(const char* sialfile, const char* key, const char* value);

#ifdef __cplusplus
}
#endif

namespace sip {
std::string index_name_value(int slot) {
	return sip::Interpreter::global_interpreter->index_value_to_string(slot);
}
std::string array_name_value(int array_table_slot) {
	return sip::Interpreter::global_interpreter->array_name(array_table_slot);
}
int get_line_number() {
#ifdef HAVE_MPI
	sip::Interpreter *interpreter = sip::Interpreter::global_interpreter;
	sip::SIPServer * server = sip::SIPServer::global_sipserver;
	sip::SIPMPIAttr &mpiattr = sip::SIPMPIAttr::get_instance();
	if (mpiattr.is_worker()){
		if (interpreter != NULL)
			return interpreter->line_number();
		else
			return 0;
	} else {
		if (server != NULL)
			return server->last_seen_line();
		else
			return 0;
	}

#else	// HAVE_MPI
	if (sip::Interpreter::global_interpreter != NULL) {
		return sip::Interpreter::global_interpreter->line_number();
	} else
		return 0;
#endif	// HAVE_MPI
}

}
