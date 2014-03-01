#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])
AC_AUTOCONF_VERSION
AC_INIT([ACES4], 
	[1.0], 
	[sanders@cise.ufl.edu])
AC_CONFIG_MACRO_DIR([m4])
m4_include([m4/ax_blas.m4])
m4_include([m4/ax_lapack.m4])
m4_include([m4/ax_prog_cc_mpi.m4])
m4_include([m4/ax_prog_cxx_mpi.m4])
m4_include([m4/ax_prog_f77_mpi.m4])
m4_include([m4/ax_prog_fc_mpi.m4])
m4_include([m4/ax_append_flag.m4])
m4_include([m4/ax_check_compile_flag.m4])
m4_include([m4/ax_append_compile_flags.m4])
m4_include([m4/ax_config_feature.m4])
m4_include([m4/ax_compiler_vendor.m4])
m4_include([m4/ax_gcc_archflag.m4])
m4_include([m4/ax_gcc_x86_cpuid.m4])
m4_include([m4/ax_cc_maxopt.m4])
m4_include([m4/ax_ext.m4])
m4_include([m4/acx_pthread.m4])
m4_include([m4/ax_openmp.m4])
m4_include([m4/ax_prog_doxygen.m4])
m4_include([m4/ax_compare_version.m4])

AC_CONFIG_SRCDIR([src/])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
#AC_PROG_CXX([icpc g++])
#AC_PROG_CC([icc gcc])
#AC_PROG_F77([ifort gfortran])
#AC_PROG_FC([ifort gfortran])

# Check for MPI Compiler
# If --with-mpi=auto is used, try to find MPI, but use standard C compiler if it is not found.
# If --with-mpi=yes is used, try to find MPI and fail if it isn't found.
# If --with-mpi=no is used, use a standard C compiler instead.
AC_ARG_WITH(mpi, [AS_HELP_STRING([--with-mpi],
    [compile with MPI (parallelization) support. If none is found,
    MPI is not used. Default: auto])
],,[with_mpi=auto])
#
use_mpi="yes"
AX_PROG_CC_MPI([test x"$with_mpi" != xno],[],[use_mpi="no"
  if test x"$with_mpi" = xyes; then
    AC_MSG_FAILURE([C MPI compiler requested, but couldn't use MPI.])
  else
    AC_MSG_WARN([No C MPI compiler found, won't use MPI.])
  fi
])
AX_PROG_CXX_MPI([test x"$with_mpi" != xno],[],[use_mpi="no"
  if test x"$with_mpi" = xyes; then
    AC_MSG_FAILURE([C++ MPI compiler requested, but couldn't use MPI.])
  else
    AC_MSG_WARN([No C++ MPI compiler found, won't use MPI.])
  fi
])
AX_PROG_F77_MPI([test x"$with_mpi" != xno],[],[use_mpi="no"
  if test x"$with_mpi" = xyes; then
    AC_MSG_FAILURE([Fortran 77 MPI compiler requested, but couldn't use MPI.])
  else
    AC_MSG_WARN([No Fortran 77 MPI compiler found, won't use MPI.])
  fi
])
AX_PROG_FC_MPI([test x"$with_mpi" != xno],[],[use_mpi="no"
  if test x"$with_mpi" = xyes; then
    AC_MSG_FAILURE([Fortran MPI compiler requested, but couldn't use MPI.])
  else
    AC_MSG_WARN([No Fortran MPI compiler found, won't use MPI.])
  fi
])

AM_CONDITIONAL([WORKING_MPI], [test "x$use_mpi" = "xyes"])
if test "x$use_mpi" = "xyes"; then
    AC_DEFINE([HAVE_MPI])
fi


# Doxygen
DX_HTML_FEATURE(ON)
DX_CHM_FEATURE(OFF)
DX_CHI_FEATURE(OFF)
DX_MAN_FEATURE(OFF)
DX_RTF_FEATURE(OFF)
DX_XML_FEATURE(OFF)
DX_PDF_FEATURE(ON)
DX_PS_FEATURE(OFF)

DX_INIT_DOXYGEN([$PACKAGE_NAME],[doxygen.cfg])


# Enabling compiler vendor specific flags for which 
# there was no easily available macro
AX_COMPILER_VENDOR
case $ax_cv_c_compiler_vendor in
	gnu)
		AC_LANG([C])
		AC_LANG([C++])
		AC_LANG([Fortran])
	        AX_APPEND_COMPILE_FLAGS([-finit-local-zero])
	        AX_APPEND_COMPILE_FLAGS([-Wuninitialized])
		AC_LANG([Fortran 77])
	        AX_APPEND_COMPILE_FLAGS([-finit-local-zero])
	        AX_APPEND_COMPILE_FLAGS([-Wuninitialized])
	;;
	intel)
		AC_LANG([C])
		AC_LANG([C++])
		AC_LANG([Fortran])
		AX_APPEND_COMPILE_FLAGS([-zero])		  # init saved vars to zero
		AX_APPEND_COMPILE_FLAGS([-Zp8])
		AC_LANG([Fortran 77])
		AX_APPEND_COMPILE_FLAGS([-zero])		  # init saved vars to zero
		AX_APPEND_COMPILE_FLAGS([-Zp8])
	;;
	portland)
		AC_LANG([C])
		AC_LANG([C++])
		AC_LANG([Fortran])
		AC_LANG([Fortran 77])
	;;
esac

AC_LANG([C])

# For Libtool
AC_PROG_LIBTOOL

# For using target specific Primaries in Makefile.am
AM_PROG_CC_C_O

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h fcntl.h limits.h netdb.h stddef.h stdlib.h string.h strings.h sys/time.h sys/timeb.h unistd.h wchar.h wctype.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_CHECK_TYPES([ptrdiff_t])

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_MMAP
AC_FUNC_STRERROR_R
AC_CHECK_FUNCS([dup2 fchdir getcwd getpagesize gettimeofday memset mkdir munmap regcomp rmdir socket strcasecmp strchr strdup strerror strrchr strstr strtol strtoull])


# Attempts to conditionally run autoconf macros have failed using these
# snippets of code.
# I am just doing what works on the UF HiperGator - NJ

#newac=m4_version_compare(m4_defn([AC_AUTOCONF_VERSION]), [2.63])
#if test ${newac} -gt 0 ; then
#    AC_MSG_NOTICE([Will use stock OpenMP macro])
#    AC_LANG([Fortran])
#    AC_OPENMP
#    AC_LANG([Fortran 77])
#    AC_OPENMP
#    #Archiver Required in Automake > 1.12
#    AM_PROG_AR
#else
#    AC_MSG_NOTICE([Will use third party OpenMP macro])
#    AC_LANG([Fortran])
#    AX_OPENMP
#    AC_LANG([Fortran 77])
#    AX_OPENMP
#fi


#AX_COMPARE_VERSION([m4_defn([AC_AUTOCONF_VERSION])], [gt], [2.69],
#    [AC_MSG_NOTICE([Will use stock OpenMP macro])
#    AC_LANG([Fortran])
#    AC_OPENMP
#    AC_LANG([Fortran 77])
#    AC_OPENMP
#    #Archiver Required in Automake > 1.12
#    AM_PROG_AR],
#    [AC_MSG_NOTICE([Will use third party OpenMP macro])
#    AC_LANG([Fortran])
#    AX_OPENMP
#    AC_LANG([Fortran 77])
#    AX_OPENMP])

AC_MSG_NOTICE([Not using OpenMP])
# These don't work on UF HiperGator and are commented out
#AC_LANG([Fortran])
#AC_OPENMP([AX_APPEND_COMPILE_FLAGS([-D_OPENMP])], [])
#AC_LANG([Fortran 77])
#AC_OPENMP([AX_APPEND_COMPILE_FLAGS([-D_OPENMP])], [])
#AC_LANG([C])
#AC_OPENMP
#AC_LANG([C++])
#AC_OPENMP





AC_LANG([C])
# Search for the BLAS & LAPACK libraries on the system
#AX_BLAS(AC_MSG_RESULT([found BLAS library $BLAS_LIBS]), 
#	AC_MSG_ERROR([could not find a BLAS library]))   
lapack_present=true
AX_LAPACK(AC_MSG_RESULT([found LAPACK library $LAPACK_LIBS]), 
	[AC_MSG_ERROR([could not find a LAPACK library]); lapack=false])

# If we could provide our own lapack library, this would be a good way to link to it.
#AM_CONDITIONAL([LAPACK_PRESENT], [test x$lapack_present = xtrue])


# For pthread library linking
#AC_LANG([C])
#ACX_PTHREAD
#AC_SUBST(PTHREAD_LIBS)
#AC_SUBST(PTHREAD_CFLAGS)
#AC_SUBST(PTHREAD_CC)

# Configure pthreads.
# Taken from Google Test 1.7's configure.ac
AC_ARG_WITH([pthreads],
	    [AS_HELP_STRING([--with-pthreads],
	       [use pthreads (default is yes)])],
	    [with_pthreads=$withval],
	    [with_pthreads=check])

have_pthreads=no
AS_IF([test "x$with_pthreads" != "xno"],
      [ACX_PTHREAD(
	[],
	[AS_IF([test "x$with_pthreads" != "xcheck"],
	       [AC_MSG_FAILURE(
		 [--with-pthreads was specified, but unable to be used])])])
       have_pthreads="$acx_pthread_ok"])
AM_CONDITIONAL([HAVE_PTHREADS],[test "x$have_pthreads" = "xyes"])
AC_SUBST(PTHREAD_CFLAGS)
AC_SUBST(PTHREAD_LIBS)


# PAPI
dnl Location of PAPI
have_papi=no
AC_ARG_WITH(papi,AC_HELP_STRING([--with-papi=PATH],[Location of PAPI]),
    [AC_SUBST(PAPI_LIBS,[${withval}/lib/libpapi.a]), 
    AC_SUBST(PAPI_INCLUDE,[-I${withval}/include]), 
    have_papi="xyes", 
    AC_DEFINE(HAVE_PAPI,1,[Define if you have a PAPI library.])],
	AC_MSG_WARN([Not using PAPI!! The timers may not be accurate!]))

# DEVELEOPMENT / LOGGING MODE
AC_ARG_ENABLE(development, AS_HELP_STRING([--enable-development], [Enable development mode; extra logging and checks]),
	[AC_DEFINE(SIP_DEVEL, 1, [Development mode, extra logging and checking])], [])

# TAU
have_tau=no
AC_ARG_WITH(tau,AC_HELP_STRING([--with-tau=PATH],[Location of TAU]),
    [AC_SUBST(TAU_LIBS,["-L${withval}/x86_64/lib/ -lTAU"]), 
    AC_SUBST(TAU_CPP,["-I${withval}/include -DPROFILING_ON -DTAU_DOT_H_LESS_HEADERS"]), 
    have_tau="xyes", 
    AC_DEFINE(HAVE_TAU,1,[Define if you have a TAU library.])],
	AC_MSG_WARN([Not using TAU!]))


# Defines C macros to properly mangle the names of C/C++ identifiers, 
# and identifiers with underscores so that they 
# match the name-mangling scheme used by the Fortran compiler. 
AC_F77_WRAPPERS
AC_FC_WRAPPERS

# To search for stuff like ifcore, etc
AC_F77_LIBRARY_LDFLAGS
AC_FC_LIBRARY_LDFLAGS

# Takes care of certain compilers trying to declare their own MAIN_
AC_F77_MAIN
AC_FC_MAIN

# To deal with the ACES2 part in the initialization executable
# AC_LANG([Fortran])
# AX_APPEND_COMPILE_FLAGS([-D__fortran -D__fortran77])
# AC_LANG([Fortran 77])
# AX_APPEND_COMPILE_FLAGS([-D__fortran -D__fortran77])

# CUDA
# largely adapted from http://wili.cc/blog/cuda-m4.html
AC_ARG_WITH(cuda, AC_HELP_STRING([--with-cuda=PATH],[Enable CUDA]), 
            [cuda_enable="yes"; cuda_prefix=$withval], 
            [cuda_enable="no"])
if test "x$cuda_enable" == "xyes"; then
    NVCCLDFLAGS=
    NVCCCPPFLAGS=
    # Save the current flags"
    ax_save_CFLAGS="${CFLAGS}"
    ax_save_LDFLAGS="${LDFLAGS}"
    if test "x$cuda_prefix" != "xyes"; then
        # assign CFLAGS & LDFLAGS for header & library checks
        CUDA_CFLAGS="-I$cuda_prefix/include"
        CFLAGS="$CUDA_CFLAGS $CFLAGS"
        CUDA_LDFLAGS="-L$cuda_prefix/lib64"
        LDFLAGS="$CUDA_LDFLAGS $LDFLAGS"
        NVCCLDFLAGS=" -L${cuda_prefix}/lib64 -lcudart -lcublas"
        NVCCCPPFLAGS=" -I${cuda_prefix}/include -arch=sm_13"
    else
        NVCCLDFLAGS=" -lcudart -lcublas "
        NVCCCPPFLAGS=" -arch=sm_13 "
    fi
    AC_ARG_VAR([NVCC], [nvcc compiler to use])
    AC_PATH_PROG([NVCC], [nvcc], [no])
    AC_CHECK_HEADER([cuda.h], [], AC_MSG_FAILURE([Couldnt find cuda.h]), [#include <cuda.h>])
    AC_CHECK_LIB([cudart], [cudaDeviceReset], [AC_MSG_NOTICE([Founc cudart])], AC_MSG_FAILURE([Couldnt find libcudart]))
    AC_CHECK_LIB([cublas], [cublasInit], [AC_MSG_NOTICE([found cublas])], AC_MSG_FAILURE([Couldnt find libcublas]))
    AC_DEFINE(HAVE_CUDA, 1, [Defined if CUDA & CUBLAS are present])
    AC_MSG_NOTICE([Using CUDA])
    AC_SUBST(NVCCLDFLAGS)
    AC_SUBST(NVCCCPPFLAGS)
    CFLAGS=${ax_save_CFLAGS}
    LDFLAGS=${ax_save_LDFLAGS}
fi

# For Google Tests
PYTHON=  # We *do not* allow the user to specify a python interpreter
AC_PATH_PROG([PYTHON],[python],[:])
AS_IF([test "$PYTHON" != ":"],
      [AM_PYTHON_CHECK_VERSION([$PYTHON],[2.3],[:],[PYTHON=":"])])
AM_CONDITIONAL([HAVE_PYTHON],[test "$PYTHON" != ":"])


AM_CONDITIONAL([WORKING_NVCC], [test "x$cuda_enable" = "xyes"])


AM_INIT_AUTOMAKE([foreign -Wall tar-ustar])
AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([src/sip/super_instructions/Makefile])
AC_OUTPUT

