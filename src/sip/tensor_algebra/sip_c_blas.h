/*
 * sip_c_blas.h
 *
 *  Created on: Jan 28, 2015
 *      Author: njindal
 */

#ifndef SIP_C_BLAS_H_
#define SIP_C_BLAS_H_

// Interfaces to C blas routines used by C++ methods in SIP.
extern "C" {

void sip_blas_daxpy(int& N, double& alpha, double *X,
                  int& incX, double *Y, int& incY);

void sip_blas_dcopy(int& N, double *X, int& incX,
                 double *Y,  int& incY);

void sip_blas_dscal(int& N, double& alpha, double *X, int& incX);

}

#endif /* SIP_C_BLAS_H_ */
