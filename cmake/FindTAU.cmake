# From https://github.com/schnorr/akypuera/blob/master/cmake/FindTAU.cmake
#
# Find the TAU libraries and include dir
#

# TAU_LIBRARY       - Path to TAU library
# TAU_INCLUDE_PATH  - Directories to include to use TAU
# TAU_LIBRARY_PATH    - Files to link against to use TAU
# TAU_FOUND        - When false, don't try to use TAU
#
# TAU_PATH can be used to make it simpler to find the various include
# directories and compiled libraries when TAU was not installed in the
# usual/well-known directories (e.g. because you made an in tree-source
# compilation or because you installed it in an "unusual" directory).
# Just set TAU_PATH it to your specific installation directory
#
FIND_LIBRARY(TAU_LIBRARY
  NAMES TAU TAU_tf
  PATHS /usr/lib /usr/local/lib ${TAU_PATH}/lib ${TAU_PATH}/x86_64/lib/ ${TAU_PATH}/i386_linux/lib)

IF(TAU_LIBRARY)
  MESSAGE ( STATUS "Found TAU: ${TAU_LIBRARY}" )
  GET_FILENAME_COMPONENT(TAU_LIBRARY_tmp "${TAU_LIBRARY}" PATH)
  SET (TAU_LIBRARY_PATH ${TAU_LIBRARY_tmp} CACHE PATH "")
ELSE(TAU_LIBRARY)
  SET (TAU_LIBRARY_PATH "TAU_LIBRARY_PATH-NOTFOUND")
  unset(LIBRARY_PATH CACHE)
ENDIF(TAU_LIBRARY)

FIND_PATH( TAU_INCLUDE_tmp TAU_tf.h
  PATHS
  ${TAU_GUESSED_INCLUDE_PATH}
  ${TAU_PATH}/include
  /usr/include
  /usr/local/include
)

IF(TAU_INCLUDE_tmp)
  SET (TAU_INCLUDE_PATH "${TAU_INCLUDE_tmp}" CACHE PATH "")
ELSE(TAU_INCLUDE_tmp)
  SET (TAU_INCLUDE_PATH "TAU_INCLUDE_PATH-NOTFOUND")
ENDIF(TAU_INCLUDE_tmp)

IF( TAU_INCLUDE_PATH )
  IF( TAU_LIBRARY_PATH )
    SET( TAU_FOUND TRUE )
  ENDIF ( TAU_LIBRARY_PATH )
ENDIF( TAU_INCLUDE_PATH )

IF( NOT TAU_FOUND )
  MESSAGE(STATUS "TAU installation was not found. Please provide TAU_PATH:")
  MESSAGE(STATUS "  - through the GUI when working with ccmake, ")
  MESSAGE(STATUS "  - as a command line argument when working with cmake e.g.")
  MESSAGE(STATUS "    cmake .. -DTAU_PATH:PATH=/usr/local/tau ")
  SET(TAU_PATH "" CACHE PATH "Root of TAU install tree." )
ENDIF( NOT TAU_FOUND )

unset(TAU_INCLUDE_tmp CACHE)
mark_as_advanced(TAU_LIBRARY)

