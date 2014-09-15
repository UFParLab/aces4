#ifndef CONFIG_H
#define CONFIG_H

/* Define if you have a BLAS library. */
#cmakedefine HAVE_BLAS

/* Defined if CUDA & CUBLAS are present */
#cmakedefine HAVE_CUDA

/* Define if you have the MPI library. */
#cmakedefine HAVE_MPI

/* Define if you have a PAPI library. */
#cmakedefine HAVE_PAPI

/* Define if you have a TAU library. */
#cmakedefine HAVE_TAU

/* Development mode, extra logging and checking */
#cmakedefine SIP_DEVEL

#endif // CONFIG_H
