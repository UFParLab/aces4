/*
 * sip_mpi_utils.h
 *
 *  Created on: Jan 17, 2014
 *      Author: njindal
 */

#ifndef SIP_MPI_UTILS_H_
#define SIP_MPI_UTILS_H_

#include <mpi.h>
#include "block_id.h"



namespace sip {


/**
 * Utility methods for MPI communication
 */
class SIPMPIUtils {

public:
	SIPMPIUtils(){}
	~SIPMPIUtils(){}

	/**
	 * Sets the error handler to print the errors and exit.
	 */
	static void set_error_handler();

	/**
	 * Checks errors
	 * @param
	 */
	static void check_err(int errnum, int line, char* file);
	static void check_err(int errnum);

	static const int BLOCKID_BUFF_ELEMS = MAX_RANK + 3;  //this is the number of ints in the message
	/* buffer format is <array_id>, <dim_0> ...<dim_MAX_RANK-1> <pc> <pardo_section> */
	static void encode_BlockID_buff(int buff[], const BlockId& id, int pc, int pardo_section){
		int pos = 0;
		buff[pos++] = id.array_id();
		for (int i = 0;  i != MAX_RANK; ++i) buff[pos++] = id.index_values(i);
		buff[pos++]=pc;
		buff[pos]=pardo_section;
	}

	static void decode_BlockID_buff(int buff[], BlockId& id, int& pc, int& pardo_section){
		int pos = 0;
		id.array_id_ = buff[pos++];
		int index_values[MAX_RANK];
		for (int i = 0; i != MAX_RANK; ++i) id.index_values_[i]=buff[pos++];
		pc = buff[pos++];
		pardo_section = buff[pos];
	}






private:

	DISALLOW_COPY_AND_ASSIGN(SIPMPIUtils);
};

class MPIScalarOpType{

public:
	MPIScalarOpType();
	~MPIScalarOpType();

	struct scalar_op_message_t{
		double value_;
		int id_pc_section_buff_[SIPMPIUtils::BLOCKID_BUFF_ELEMS];
	};

	MPI_Datatype mpi_scalar_op_type_;
	void initialize_mpi_scalar_op_type();
private:
	DISALLOW_COPY_AND_ASSIGN(MPIScalarOpType);
};



} /* namespace sip */

#endif /* SIP_MPI_UTILS_H_ */
