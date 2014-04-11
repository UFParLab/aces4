/*
 * sip_mpi_utils.h
 *
 *  Created on: Jan 17, 2014
 *      Author: njindal
 */

#ifndef SIP_MPI_UTILS_H_
#define SIP_MPI_UTILS_H_



namespace sip {


/**
 * Utility methods for MPI communication
 */
class SIPMPIUtils {

public:

	/**
	 * Sets the error handler to print the errors and exit.
	 */
	static void set_error_handler();

	/**
	 * Checks errors
	 * @param
	 */
	static void check_err(int);

private:


};

} /* namespace sip */

#endif /* SIP_MPI_UTILS_H_ */
