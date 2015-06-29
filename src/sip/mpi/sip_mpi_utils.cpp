/*
 * sip_mpi_utils.cpp
 *
 *  Created on: Jan 17, 2014
 *      Author: njindal
 */


#include "sip_mpi_utils.h"
#include <mpi.h>
#include <memory>
#include <cstdio>
#include <cstring>
#include "sip_mpi_attr.h"
namespace sip {


void SIPMPIUtils::set_error_handler(){
//	std::cout << "  in SIPMPIUtils::set_error_handler" << std::endl << std::flush;
	MPI_Errhandler_set(MPI_COMM_WORLD, MPI_ERRORS_RETURN);
	SIPMPIAttr & mpi_attr = SIPMPIAttr::get_instance();
//	std::cout << "Rank " << mpi_attr.global_rank()  << "  in SIPMPIUtils::set_error_handler:  mpi_attr.company_communicator()!=NULL " << (mpi_attr.company_communicator() != NULL) << std::endl << std::flush;
	MPI_Errhandler_set(mpi_attr.company_communicator(), MPI_ERRORS_RETURN);
}

void SIPMPIUtils::check_err(int err){

	if (err == MPI_SUCCESS){
		return;
	} else {
		int len, eclass;
	    char estring[MPI_MAX_ERROR_STRING];
	    MPI_Error_class(err, &eclass);
	    MPI_Error_string(err, estring, &len);
	    fprintf(stderr, "Error %d:%s\n", eclass, estring);
	    fflush(stdout);
        fail("MPI Error !\n");
	}
}
void SIPMPIUtils::check_err(int err, int line, char * file){

	if (err == MPI_SUCCESS){
		return;
	} else {
		int len, eclass;
	    char estring[MPI_MAX_ERROR_STRING];
	    MPI_Error_class(err, &eclass);
	    MPI_Error_string(err, estring, &len);
	    fprintf(stderr, "Error %d: %s at line %d in file %s\n", eclass, estring, line, file);
	    fflush(stdout);
        fail("MPI Error !\n");
	}
}

MPIScalarOpType::MPIScalarOpType(){
	initialize_mpi_scalar_op_type();
}

MPIScalarOpType::~MPIScalarOpType(){
	MPI_Type_free(&mpi_scalar_op_type_);
}

void MPIScalarOpType::initialize_mpi_scalar_op_type(){
	const int NUM_STRUCT_ITEMS = 2;
	MPI_Aint double_extent, offsets[NUM_STRUCT_ITEMS];
	int block_counts[NUM_STRUCT_ITEMS];
	MPI_Datatype struct_types[NUM_STRUCT_ITEMS];

	//initialize for the double field
	struct_types[0] = MPI_DOUBLE;
	offsets[0] = 0;
	block_counts[0] = 1;

	//get extend of the double
	MPI_Type_extent(MPI_DOUBLE, &double_extent);

	//initialize for the ints
	struct_types[1] = MPI_INT;
	offsets[1] = double_extent;
	block_counts[1] = SIPMPIUtils::BLOCKID_BUFF_ELEMS;

	MPI_Type_create_struct(NUM_STRUCT_ITEMS, block_counts, offsets, struct_types, &mpi_scalar_op_type_);

	//Double check that c++ and mpi agree on size of struct.
	//If not, MPI_Type_create_resized should be used.  This will not be implemented
	//now since there is no way to test it.  See examples in the MPI books.
//	MPI_Aint  s_lower, s_extent;
//	MPI_Type_get_extent(mpi_scalar_op_type_, &s_lower, &s_extent);
//	check(s_extent == sizeof(mpi_scalar_op_type_), "Need to use MPI_Type_create_resized.  See code comments");


	MPI_Type_commit(&mpi_scalar_op_type_);
}


} /* namespace sip */
