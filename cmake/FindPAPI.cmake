# From https://github.com/schnorr/akypuera/blob/master/cmake/FindTAU.cmake
# Modified for PAPI
#
# Find the PAPI libraries and include dir
#

# PAPI_LIBRARY       - Path to PAPI library
# PAPI_INCLUDE_PATH  - Directories to include to use PAPI
# PAPI_LIBRARY_PATH    - Files to link against to use PAPI
# PAPI_FOUND        - When false, don't try to use PAPI
#
# PAPI_PATH can be used to make it simpler to find the various include
# directories and compiled libraries when PAPI was not installed in the
# usual/well-known directories (e.g. because you made an in tree-source
# compilation or because you installed it in an "unusual" directory).
# Just set PAPI_PATH it to your specific installation directory
#
FIND_LIBRARY(PAPI_LIBRARY
  NAMES PAPI papi libpapi.a
  PATHS /usr/lib /usr/local/lib ${PAPI_PATH}/lib ${PAPI_PATH}/x86_64/lib/ ${PAPI_PATH}/i386_linux/lib)

IF(PAPI_LIBRARY)
  MESSAGE ( STATUS "Found PAPI: ${PAPI_LIBRARY}" )
  GET_FILENAME_COMPONENT(PAPI_LIBRARY_tmp "${PAPI_LIBRARY}" PATH)
  SET (PAPI_LIBRARY_PATH ${PAPI_LIBRARY_tmp} CACHE PATH "")
ELSE(PAPI_LIBRARY)
  SET (PAPI_LIBRARY_PATH "PAPI_LIBRARY_PATH-NOTFOUND")
  unset(LIBRARY_PATH CACHE)
ENDIF(PAPI_LIBRARY)

FIND_PATH( PAPI_INCLUDE_tmp papi.h
  PATHS
  ${PAPI_GUESSED_INCLUDE_PATH}
  ${PAPI_PATH}/include
  /usr/include
  /usr/local/include
)

IF(PAPI_INCLUDE_tmp)
  SET (PAPI_INCLUDE_PATH "${PAPI_INCLUDE_tmp}" CACHE PATH "")
ELSE(PAPI_INCLUDE_tmp)
  SET (PAPI_INCLUDE_PATH "PAPI_INCLUDE_PATH-NOTFOUND")
ENDIF(PAPI_INCLUDE_tmp)

IF( PAPI_INCLUDE_PATH )
  IF( PAPI_LIBRARY_PATH )
    SET( PAPI_FOUND TRUE )
  ENDIF ( PAPI_LIBRARY_PATH )
ENDIF( PAPI_INCLUDE_PATH )

IF( NOT PAPI_FOUND )
  MESSAGE(STATUS "PAPI installation was not found. Please provide PAPI_PATH:")
  MESSAGE(STATUS "  - through the GUI when working with ccmake, ")
  MESSAGE(STATUS "  - as a command line argument when working with cmake e.g.")
  MESSAGE(STATUS "    cmake .. -DPAPI_PATH:PATH=/usr/local/papi ")
  SET(PAPI_PATH "" CACHE PATH "Root of PAPI install tree." )
ENDIF( NOT PAPI_FOUND )

unset(PAPI_INCLUDE_tmp CACHE)
mark_as_advanced(PAPI_LIBRARY)

