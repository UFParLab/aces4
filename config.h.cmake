#ifndef CONFIG_H
#define CONFIG_H

/* Define if you have a BLAS library. */
#cmakedefine HAVE_BLAS

/* Defined if CUDA & CUBLAS are present */
#cmakedefine HAVE_CUDA

/* Define if you have the MPI library. */
#cmakedefine HAVE_MPI

/* Development mode, extra logging and checking */
#cmakedefine SIP_DEVEL

#endif // CONFIG_H
