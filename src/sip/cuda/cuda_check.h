// From https://bitbucket.org/seanmauch/stlib/src/77b6f65e53c8/src/cuda/check.h?at=default


// -*- C++ -*-

/*!
  \file cuda/check.h
  \brief Check CUDA error codes.
*/

#if !defined(__cuda_check_h__)
#define __cuda_check_h__

#ifdef __CUDA_ARCH__

//! No checks within device code.
#define CUDA_CHECK(err) (err)

//! No checks within device code.
#define CUBLAS_CHECK(err) (err)

#else

#include <cuda_runtime_api.h>
#include <cublas_v2.h>
#include <iostream>

/*!
\page cudaCheck Check CUDA error codes.

Check CUDA error codes with cudaCheck() or the CUDA_CHECK macro.
*/

//! Check the CUDA error code.
inline
void
cudaCheck(cudaError_t err, const char *file, const int line) {
   if (err != cudaSuccess) {
      std::cout << cudaGetErrorString(err) << " in " << file << " at line "
                << line << ".\n";
      exit(EXIT_FAILURE);
   }
}

//! Check the CUDA error code.
#define CUDA_CHECK(err) (cudaCheck(err, __FILE__, __LINE__))

// From 
// http://stackoverflow.com/questions/13041399/equivalent-of-cudageterrorstring-for-cublas    
inline 
const char* 
cublasGetErrorString(cublasStatus_t status)
{
    switch(status)
    {
        case CUBLAS_STATUS_SUCCESS: return "CUBLAS_STATUS_SUCCESS";
        case CUBLAS_STATUS_NOT_INITIALIZED: return "CUBLAS_STATUS_NOT_INITIALIZED";
        case CUBLAS_STATUS_ALLOC_FAILED: return "CUBLAS_STATUS_ALLOC_FAILED";
        case CUBLAS_STATUS_INVALID_VALUE: return "CUBLAS_STATUS_INVALID_VALUE"; 
        case CUBLAS_STATUS_ARCH_MISMATCH: return "CUBLAS_STATUS_ARCH_MISMATCH"; 
        case CUBLAS_STATUS_MAPPING_ERROR: return "CUBLAS_STATUS_MAPPING_ERROR";
        case CUBLAS_STATUS_EXECUTION_FAILED: return "CUBLAS_STATUS_EXECUTION_FAILED"; 
        case CUBLAS_STATUS_INTERNAL_ERROR: return "CUBLAS_STATUS_INTERNAL_ERROR"; 
    }
    return "unknown error";
}

inline 
void 
cublasCheck(cublasStatus_t err, const char *file, const int line){
    if (err != CUBLAS_STATUS_SUCCESS){
        std::cout << cublasGetErrorString(err) << " in " << file << " at line "
                  << line << ".\n";
        exit(EXIT_FAILURE);          
    }
}


#define CUBLAS_CHECK(err) (cublasCheck(err, __FILE__, __LINE__))

#endif

#endif
