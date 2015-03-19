cmake_minimum_required (VERSION 2.8 FATAL_ERROR)

project(aces4)

enable_language(Fortran)

#----------------------------------------------------------------------
# Options, Settings, Libraries, Packages
#----------------------------------------------------------------------

# All custom find modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

# Find required packages for aces4.
find_package(LAPACK REQUIRED)
find_package(OpenMP)

##################
# HAVE_MPI Option
##################
option(HAVE_MPI "Whether to use MPI" ON)

# Check if MPI available on system
if (HAVE_MPI)
    find_package(MPI REQUIRED)
endif()

##################
# HAVE_TAU Option - TAU timers to be used instead of Linux / PAPI timers
##################
option(HAVE_TAU "Whether to use TAU timers for SIAL programs" OFF)

# Check if TAU available on system
if (HAVE_TAU)
    find_package(TAU REQUIRED)
endif()

####################
# HAVE_PAPI Options - PAPI Timers to be used instead of Linux Timers
####################
option(HAVE_PAPI "Whether to use PAPI high resolution timers" OFF)

# Check if PAPI available on system
if (HAVE_PAPI)
    find_package(PAPI REQUIRED)
endif()

####################
# HAVE_CUDA Options - Whether to use CUDA accelerated super instructions
####################
option(HAVE_CUDA "Whether to use CUDA accelerated super instructions" OFF)

# Check if CUDA available on system
if (HAVE_CUDA)
    find_package(CUDA REQUIRED)
endif()

####################
# SIP_DEVEL Options - Whether to enable extra logging for the entire SIP
####################
option(SIP_DEVEL "Whether to enable extra logging for the entire SIP" OFF)

if (SIP_DEVEL)
    message(STATUS "   SIP_DEVEL Flag enabled    ")
    message(STATUS "-----------------------------")
    message(STATUS "There will be lots of print !")
    message(STATUS "-----------------------------")
endif()



#Config file 
configure_file(${CMAKE_SOURCE_DIR}/config.h.cmake ${CMAKE_BINARY_DIR}/config.h)

#----------------------------------------------------------------------
# ACES4 sources, libraries, includes
#----------------------------------------------------------------------
set(ACES_SETUP_FILES 
    src/sip/setup/io_utils.h;
    src/sip/setup/setup_reader.h;
    src/sip/setup/setup_interface.h;
    src/sip/setup/setup_writer.h;
    src/sip/setup/setup_reader.cpp;
    src/sip/setup/setup_interface.cpp;
    src/sip/setup/setup_writer.cpp;
    src/sip/setup/io_utils.cpp)

set(ACES_CORE_FILES 
    src/sip/core/array_constants.h;
    src/sip/core/array_constants.cpp;
    src/sip/core/sip.h;
    src/sip/core/sip.cpp;
    src/sip/core/aces_defs.h)

# MPI - Conditional compile for MPI files
if (HAVE_MPI AND MPI_CXX_FOUND)
    set(ACES_MPI_FILES
        src/sip/mpi/rank_distribution.h ;
        src/sip/mpi/rank_distribution.cpp ;
        src/sip/mpi/data_distribution.cpp ;
        src/sip/mpi/data_distribution.h;
        src/sip/mpi/sip_mpi_utils.h;
        src/sip/mpi/sip_mpi_utils.cpp;
        src/sip/mpi/sip_mpi_constants.h;
        src/sip/mpi/sip_mpi_constants.cpp;
        src/sip/mpi/barrier_support.h;
        src/sip/mpi/async_acks.h;
        src/sip/mpi/async_acks.cpp;
        src/sip/mpi/sip_server.h;
        src/sip/mpi/sip_server.cpp;
        src/sip/worker/sial_ops_parallel.h;
        src/sip/worker/sial_ops_parallel.cpp;
        src/sip/mpi/disk_backed_arrays_io.h;
        src/sip/mpi/disk_backed_arrays_io.cpp;
        src/sip/mpi/server_block.h;
        src/sip/mpi/server_block.cpp;
        src/sip/mpi/disk_backed_block_map.h;
        src/sip/mpi/disk_backed_block_map.cpp;
        src/sip/mpi/server_persistent_array_manager.h;
        src/sip/mpi/server_persistent_array_manager.cpp;
        src/sip/mpi/server_timer.h;
        src/sip/mpi/server_timer.cpp;        
        src/sip/dynamic_data/mpi_state.h)
else ()
    set(ACES_MPI_FILES "")
endif()

# CUDA - Conditional compile for CUDA super instructions
if (HAVE_CUDA AND CUDA_FOUND)
    set(ACES_CUDA_FILES
        src/sip/cuda/gpu_super_instructions.cu
        src/sip/cuda/cuda_check.h)
else ()
    set(ACES_CUDA_FILES "")
endif()

set(ACES_DATA_FILES
    src/sip/static_data/array_table.h;
    src/sip/static_data/array_table.cpp;
    src/sip/static_data/index_table.h;
    src/sip/static_data/index_table.cpp;
    src/sip/static_data/int_table.h;
    src/sip/static_data/int_table.cpp;
    src/sip/static_data/sip_tables.h;
    src/sip/static_data/sip_tables.cpp;
    src/sip/static_data/opcode.h;
    src/sip/static_data/opcode.cpp;
    src/sip/static_data/op_table.h;
    src/sip/static_data/op_table.cpp;
    src/sip/dynamic_data/block_id.h;
    src/sip/dynamic_data/block_id.cpp;
    src/sip/dynamic_data/block_shape.h;
    src/sip/dynamic_data/block_shape.cpp;
    src/sip/dynamic_data/block_selector.h;
    src/sip/dynamic_data/block_selector.cpp;
    src/sip/dynamic_data/block.h;
    src/sip/dynamic_data/block.cpp;
    src/sip/dynamic_data/block_manager.h;
    src/sip/dynamic_data/block_manager.cpp;
    src/sip/dynamic_data/contiguous_array_manager.h;
    src/sip/dynamic_data/contiguous_array_manager.cpp;
    src/sip/dynamic_data/data_manager.h;
    src/sip/dynamic_data/data_manager.cpp;
    src/sip/dynamic_data/id_block_map.h;
    src/sip/dynamic_data/worker_persistent_array_manager.h;
    src/sip/dynamic_data/worker_persistent_array_manager.cpp;
    src/sip/dynamic_data/lru_array_policy.h;
    src/sip/dynamic_data/cached_block_map.h;
    src/sip/dynamic_data/cached_block_map.cpp;
    src/sip/dynamic_data/sip_timer.h;
	src/sip/dynamic_data/sip_timer.cpp;
    src/sip/dynamic_data/contiguous_local_array_manager.h;
    src/sip/dynamic_data/contiguous_local_array_manager.cpp;)
#need only header files for templates--these include .cpp

set(ACES_WORKER_FILES
    src/sip/worker/loop_manager.cpp;
    src/sip/worker/loop_manager.h;
    src/sip/worker/interpreter.cpp;
    src/sip/worker/interpreter.h;
    src/sip/worker/sialx_timer.cpp;
    src/sip/worker/sialx_timer.h;
    src/sip/worker/global_state.h;
    src/sip/worker/global_state.cpp;
    src/sip/worker/siox_reader.h;
    src/sip/worker/siox_reader.cpp;
    src/sip/worker/sial_ops_sequential.h;
    src/sip/worker/sial_ops_sequential.cpp;
    src/sip/worker/sial_printer.h;
    src/sip/worker/sial_printer.cpp;
    src/sip/worker/sial_math.h;
    src/sip/worker/sial_math.cpp;
    src/sip/sip_interface.h;
    src/sip/sip_interface.cpp;
    src/sip/tensor_algebra/tensor_ops_c_prototypes.h;
    src/sip/cuda/gpu_super_instructions.h;
    src/sip/mpi/sip_mpi_attr.h;
    src/sip/mpi/sip_mpi_attr.cpp)
    
#set(aces4_SRCS
#    ${ACES_SETUP_FILES};
#    ${ACES_CORE_FILES};
#    ${ACES_DATA_FILES};
#    ${ACES_MPI_FILES};
#    ${ACES_WORKER_FILES})

# Dmitry's Tensor library
set(tensordil_SRCS src/sip/tensor_algebra/tensor_dil_omp.F90)

set(INCLUDE_DIRS     
    ${CMAKE_BINARY_DIR};
    ${MPI_INCLUDE_PATH};
    ${CMAKE_SOURCE_DIR}/src/sip;
    ${CMAKE_SOURCE_DIR}/src/sip/super_instructions;
    ${CMAKE_SOURCE_DIR}/src/sip/tensor_algebra;
    ${CMAKE_SOURCE_DIR}/src/sip/core;
    ${CMAKE_SOURCE_DIR}/src/sip/setup;
    ${CMAKE_SOURCE_DIR}/src/sip/worker;
    ${CMAKE_SOURCE_DIR}/src/sip/static_data;
    ${CMAKE_SOURCE_DIR}/src/sip/dynamic_data;
    ${CMAKE_SOURCE_DIR}/src/sip/mpi;
    ${CMAKE_SOURCE_DIR}/src/sip/cuda;
    ${CMAKE_SOURCE_DIR}/test)

set (TOLINK_LIBRARIES
    ${TOLINK_LIBRARIES}
    tensordil;
    ${MPI_LIBRARIES};
    superinstructions # Points to : ${CMAKE_BINARY_DIR}/src/sip/super_instructions/libsuperinstructions.a;
    ${LAPACK_LIBRARIES})

# Append TAU include & library paths
if (TAU_FOUND)
    # TAU needs these defines to compile properly
    add_definitions(-DPROFILING_ON)
    add_definitions(-DTAU_DOT_H_LESS_HEADERS)
    set(INCLUDE_DIRS ${INCLUDE_DIRS} ${TAU_INCLUDE_PATH})
    set(TOLINK_LIBRARIES ${TOLINK_LIBRARIES} ${TAU_LIBRARY})
endif()

# Append PAPI include & library paths
if (PAPI_FOUND)
    set(INCLUDE_DIRS ${INCLUDE_DIRS} ${PAPI_INCLUDE_PATH})
    set(TOLINK_LIBRARIES ${TOLINK_LIBRARIES} ${PAPI_LIBRARY})
endif()

# CUDA Super instructions
if (CUDA_FOUND)
    set(CUDA_HOST_COMPILATION_CPP ON)
    CUDA_INCLUDE_DIRECTORIES(${INCLUDE_DIRS})
    
    # Support Double precision
    set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS} -arch=sm_13) 
    CUDA_ADD_LIBRARY(cudasuperinstructions ${ACES_CUDA_FILES})
    CUDA_ADD_CUBLAS_TO_TARGET(cudasuperinstructions)
    set(TOLINK_LIBRARIES ${TOLINK_LIBRARIES} cudasuperinstructions)
endif()

# Accumulating Compile & Link Flags

set(ACES4_COMPILE_FLAGS "")
set(LIBTENSORDIL_DEFINITIONS "")
set(ACES4_LINK_FLAGS "")

# Add LAPACK Linker Flags
set (ACES4_LINK_FLAGS ${ACES4_LINK_FLAGS} ${LAPACK_LINKER_FLAGS})

# Add OpenMP Flags
if (OPENMP_FOUND)
    set(LIBTENSORDIL_DEFINITIONS _OPENMP)
    set(ACES4_LINK_FLAGS ${ACES4_LINK_FLAGS} ${OpenMP_CXX_FLAGS})
endif()

# Add MPI Flags
if (MPI_CXX_FOUND)
    set(ACES4_COMPILE_FLAGS ${ACES4_COMPILE_FLAGS} ${MPI_COMPILE_FLAGS})
    set(ACES4_LINK_FLAGS ${ACES4_LINK_FLAGS} ${MPI_LINK_FLAGS})
endif()

#----------------------------------------------------------------------
# Testing Using Google Tests
#----------------------------------------------------------------------

##############
# Setup Tests
##############

add_subdirectory (test/gtest-1.7.0)
enable_testing()
set(INCLUDE_DIRS ${INCLUDE_DIRS} ${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
configure_file(${CMAKE_SOURCE_DIR}/test/ccsdpt_test.dat ${CMAKE_BINARY_DIR}/ COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/test/second_ccsdpt_test.dat ${CMAKE_BINARY_DIR}/ COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/test/cis_test.dat ${CMAKE_BINARY_DIR}/ COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/test/mcpt2_test.dat ${CMAKE_BINARY_DIR}/ COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/test/lindep_test.dat ${CMAKE_BINARY_DIR}/ COPYONLY)

#----------------------------------------------------------------------
# Build aces4 & print_siptables executable
#----------------------------------------------------------------------

include_directories(${INCLUDE_DIRS})

# Static Libraries for ACES4 
add_library(aces4_sip     
            ${ACES_SETUP_FILES}     
            ${ACES_CORE_FILES}
            ${ACES_WORKER_FILES}
            ${ACES_DATA_FILES}
            ${ACES_MPI_FILES})

set(TOLINK_LIBRARIES aces4_sip ${TOLINK_LIBRARIES})

# All Super instructions are in this directory
add_subdirectory(src/sip/super_instructions)

# The tensor library is added separately
add_library(tensordil ${tensordil_SRCS})
set_target_properties(tensordil PROPERTIES COMPILE_DEFINITIONS "${LIBTENSORDIL_DEFINITIONS}")

# aces4 executable
add_executable(aces4 src/aces_main/main.cpp)

# print_siptables executable
add_executable(print_siptables src/util/print_siptables.cpp)

# Setting Compile & link flags
set_target_properties(aces4 PROPERTIES COMPILE_FLAGS "${ACES4_COMPILE_FLAGS}")
set_target_properties(aces4 PROPERTIES LINK_FLAGS "${ACES4_LINK_FLAGS}")

set_target_properties(print_siptables PROPERTIES COMPILE_FLAGS "${ACES4_COMPILE_FLAGS}")
set_target_properties(print_siptables PROPERTIES LINK_FLAGS "${ACES4_LINK_FLAGS}")

# Libraries to link
target_link_libraries(aces4 ${TOLINK_LIBRARIES}) 
target_link_libraries(print_siptables ${TOLINK_LIBRARIES}) 

# Dependencies
if(HAVE_CUDA)
	add_dependencies(aces4 tensordil superinstructions cudasuperinstructions)
	add_dependencies(print_siptables tensordil superinstructions cudasuperinstructions)
else()
	add_dependencies(aces4 tensordil superinstructions)
	add_dependencies(print_siptables tensordil superinstructions)
endif()

add_dependencies(superinstructions aces4_core tensordil)

#----------------------------------------------------------------------
# Add Tests
#----------------------------------------------------------------------

add_library(test_controllers
            test/test_controller.h;
            test/test_controller.cpp;
            test/test_controller_parallel.h;
            test/test_controller_parallel.cpp;
            test/test_constants.h;
            test/test_constants.cpp)


if (HAVE_MPI) # Multi Node Version Tests
    add_executable(test_basic_sial      test/test_basic_sial.cpp;
                                        test/test_transpose_op.F; 
                                        test/test_transpose4d_op.F;
                                        test/test_contraction_small2.F)
    add_executable(test_sial  	        test/test_sial.cpp)
    #add_executable(test_mpi_qm          test/test_mpi_qm.cpp)
    add_executable(test_qm              test/test_qm.cpp)
    add_executable(test_unit            test/test_unit.cpp)

    # Link test executable against gtest & gtest_main
    set_target_properties(test_basic_sial   PROPERTIES COMPILE_FLAGS  "${ACES4_COMPILE_FLAGS}")
    set_target_properties(test_sial         PROPERTIES COMPILE_FLAGS  "${ACES4_COMPILE_FLAGS}")
    #set_target_properties(test_mpi_qm       PROPERTIES COMPILE_FLAGS  "${ACES4_COMPILE_FLAGS}")
    set_target_properties(test_qm           PROPERTIES COMPILE_FLAGS  "${ACES4_COMPILE_FLAGS}")
    set_target_properties(test_unit         PROPERTIES COMPILE_FLAGS  "${ACES4_COMPILE_FLAGS}")

    target_link_libraries(test_basic_sial   test_controllers gtest gtest_main  ${TOLINK_LIBRARIES})
    target_link_libraries(test_sial         test_controllers gtest gtest_main  ${TOLINK_LIBRARIES})
    #target_link_libraries(test_mpi_qm       test_controllers gtest gtest_main  ${TOLINK_LIBRARIES})
    target_link_libraries(test_qm           test_controllers gtest gtest_main  ${TOLINK_LIBRARIES})
    target_link_libraries(test_unit         test_controllers gtest gtest_main  ${TOLINK_LIBRARIES})

    set_target_properties(test_basic_sial   PROPERTIES LINK_FLAGS  "${ACES4_LINK_FLAGS}")
    set_target_properties(test_sial         PROPERTIES LINK_FLAGS  "${ACES4_LINK_FLAGS}")
    #set_target_properties(test_mpi_qm       PROPERTIES LINK_FLAGS  "${ACES4_LINK_FLAGS}")
    set_target_properties(test_qm           PROPERTIES LINK_FLAGS  "${ACES4_LINK_FLAGS}")
    set_target_properties(test_unit         PROPERTIES LINK_FLAGS  "${ACES4_LINK_FLAGS}")

    add_test(NAME test_basic_sial   COMMAND mpirun -np 2 test_basic_sial)
    add_test(NAME test_sial         COMMAND mpirun -np 2 test_sial)
    #add_test(NAME test_mpi_qm       COMMAND mpirun -np 2 test_mpi_qm)
    add_test(NAME test_qm           COMMAND mpirun -np 2 test_qm)
    add_test(NAME test_unit         COMMAND mpirun -np 2 test_unit)

    add_dependencies(test_basic_sial   gtest gtest_main test_contollers)
    add_dependencies(test_sial         gtest gtest_main test_contollers)
    #add_dependencies(test_mpi_qm       gtest gtest_main test_contollers)
    add_dependencies(test_qm           gtest gtest_main test_contollers)
    add_dependencies(test_unit         gtest gtest_main test_contollers)

else() # Single Node Version Tests
    add_executable(test_basic_sial   test/test_basic_sial.cpp    
                                     test/test_transpose_op.F 
                                     test/test_transpose4d_op.F
                                     test/test_contraction_small2.F)

    add_executable(test_sial  	     test/test_sial.cpp)
    add_executable(test_qm           test/test_qm.cpp)

    # Link test executable against gtest & gtest_main
    set_target_properties(test_basic_sial PROPERTIES COMPILE_FLAGS  "${ACES4_COMPILE_FLAGS}")
    set_target_properties(test_sial       PROPERTIES COMPILE_FLAGS  "${ACES4_COMPILE_FLAGS}")
    set_target_properties(test_qm         PROPERTIES COMPILE_FLAGS  "${ACES4_COMPILE_FLAGS}")

    target_link_libraries(test_basic_sial   test_controllers gtest gtest_main  ${TOLINK_LIBRARIES})
    target_link_libraries(test_sial         test_controllers gtest gtest_main  ${TOLINK_LIBRARIES})
    target_link_libraries(test_qm           test_controllers gtest gtest_main  ${TOLINK_LIBRARIES})

    set_target_properties(test_basic_sial   PROPERTIES LINK_FLAGS  "${ACES4_LINK_FLAGS}")
    set_target_properties(test_sial         PROPERTIES LINK_FLAGS  "${ACES4_LINK_FLAGS}")
    set_target_properties(test_qm           PROPERTIES LINK_FLAGS  "${ACES4_LINK_FLAGS}")

    add_test(NAME test_basic_sial   COMMAND test_basic_sial)
    add_test(NAME test_sial         COMMAND test_sial)
    add_test(NAME test_qm           COMMAND test_qm)

    add_dependencies(test_basic_sial gtest gtest_main test_contollers)
    add_dependencies(test_sial       gtest gtest_main test_contollers)
    add_dependencies(test_qm         gtest gtest_main test_contollers)

endif()


#----------------------------------------------------------------------
# Compile all SIALX files
#----------------------------------------------------------------------

set(SIAL_FILES
    src/sialx/test/empty.sialx;
    src/sialx/test/static_array_test.sialx;
    src/sialx/test/ifelse.sialx;
    src/sialx/test/put_test.sialx;
    src/sialx/test/index_decs.sialx;
    src/sialx/test/predef_scalars.sialx;
    src/sialx/test/print_block_test.sialx;
    src/sialx/test/tmp_arrays.sialx;
    src/sialx/test/scalars.sialx;
    src/sialx/test/insert_slice_test.sialx;
    src/sialx/test/print_scalar.sialx;
    src/sialx/test/where_clause.sialx;
    src/sialx/test/loop_over_simple_indices.sialx;
    src/sialx/test/exit_statement_test.sialx;
    src/sialx/test/no_arg_user_sub.sialx;
    src/sialx/test/helloworld.sialx;
    src/sialx/test/pardo_loop.sialx;
    src/sialx/test/subindex_test.sialx;
    src/sialx/test/local_arrays.sialx;
    src/sialx/test/scalarTable.sialx;
    src/sialx/test/stringLiterals.sialx;
    src/sialx/test/aces_defs.sialx;
    src/sialx/test/fill_sequential_test.sialx;
    src/sialx/test/local_arrays_wild.sialx;
    src/sialx/test/local_arrays_wild_fail.sialx;
    src/sialx/test/basic_assign_to_static_array_test.sialx;
    src/sialx/test/contiguous_local.sialx;
    src/sialx/test/contiguous_local_2.sialx;
    src/sialx/test/contiguous_local_allocate_twice_fail.sialx;
    src/sialx/test/block_scale_assign.sialx;
    src/sialx/test/return_sval_test.sialx;
    src/sialx/test/broadcast_static.sialx;
    src/sialx/test/sum_op_test.sialx;
    src/sialx/test/transpose_tmp.sialx;
    src/sialx/test/transpose4d_tmp.sialx;
    src/sialx/test/transpose4d_square_tmp.sialx;
    src/sialx/test/tmp_arrays_2.sialx;
    src/sialx/test/contraction_small_test.sialx;
    src/sialx/test/contraction_small_test2.sialx;
    src/sialx/test/get_int_array_test.sialx;
    src/sialx/test/get_scalar_array_test.sialx;
    src/sialx/test/get_scratch_array_test.sialx;
    src/sialx/test/gpu_contraction_small_test.sialx;
    src/sialx/test/gpu_ops.sialx;
    src/sialx/test/contract_to_scalar.sialx;
    src/sialx/test/gpu_sum_op_test.sialx;
    src/sialx/test/gpu_contract_to_scalar.sialx;
    src/sialx/test/gpu_transpose_tmp.sialx;
    src/sialx/test/self_multiply_test.sialx;
    src/sialx/test/gpu_self_multiply_test.sialx;
    src/sialx/test/simple_indices_assignments.sialx;
    src/sialx/test/add_and_subtract_scalars.sialx;
    src/sialx/test/gpu_contraction_predefined_test.sialx;
    src/sialx/test/assign_to_static_array_test.sialx;
    src/sialx/test/set_persistent_test.sialx;
    src/sialx/test/restore_persistent_test.sialx;
    src/sialx/test/persistent_static_array_test1.sialx;
    src/sialx/test/persistent_static_array_test2.sialx;
    src/sialx/test/persistent_distributed_array_test1.sialx;
    src/sialx/test/persistent_distributed_array_test2.sialx;
    src/sialx/test/put_test_mpi.sialx;
    src/sialx/test/put_increment.sialx;
    src/sialx/test/put_accumulate_mpi.sialx;
    src/sialx/test/delete_mpi.sialx;
    src/sialx/test/get_mpi.sialx;
    src/sialx/test/persistent_distributed_array_mpi1.sialx;
    src/sialx/test/persistent_distributed_array_mpi2.sialx;
    src/sialx/test/persistent_empty_mpi1.sialx;
    src/sialx/test/persistent_empty_mpi2.sialx;
    src/sialx/test/persistent_scalars_1.sialx;
    src/sialx/test/persistent_scalars_2.sialx;
    src/sialx/test/unmatched_get.sialx;
    src/sialx/test/all_rank_print_test.sialx;
    src/sialx/test/message_number_wraparound_test.sialx;
    src/sialx/test/pardo_loop_1d.sialx;
    src/sialx/test/pardo_loop_2d.sialx;
    src/sialx/test/pardo_loop_3d.sialx;
    src/sialx/test/pardo_loop_4d.sialx;
    src/sialx/test/pardo_loop_5d.sialx;
    src/sialx/test/pardo_loop_6d.sialx;
    src/sialx/test/scalar_ops.sialx;
    src/sialx/test/int_ops.sialx;
    src/sialx/test/int_self_ops.sialx;
    src/sialx/test/scalar_valued_blocks.sialx;
    src/sialx/test/broadcast_static.sialx;
    src/sialx/test/block_scale_assign.sialx;
    src/sialx/test/return_sval_test.sialx;
    src/sialx/test/index_scalar_cast.sialx;
    src/sialx/test/contig_local3.sialx;
    src/sialx/test/pardo_with_where.sialx;
    src/sialx/test/read_block_test.sialx;
    src/sialx/qm/scf_rhf_coreh.sialx;
    src/sialx/qm/scf_defs.sialx;
    src/sialx/qm/scf_frag_lowmem.sialx;
    src/sialx/qm/scf_frag_defs_lowmem.sialx;
    src/sialx/qm/frag_2order_corr_lowmem.sialx;
    src/sialx/qm/mcpt2_corr_lowmem.sialx;
    src/sialx/qm/mcpt2_singles_lowmem.sialx;
    src/sialx/qm/mcpt2_doubles_lowmem.sialx;
    src/sialx/qm/trans_frag_defs.sialx;
    src/sialx/qm/mp2_rhf_disc.sialx;
    src/sialx/qm/tran_rhf_no4v.sialx;
    src/sialx/qm/tran_uhf_no4v.sialx;
    src/sialx/qm/tran_rhf_no4v_defs.sialx;
    src/sialx/qm/tran_rhf_no3v.sialx;
    src/sialx/qm/rccsd_rhf.sialx;
    src/sialx/qm/rlccd_rhf.sialx;
    src/sialx/qm/rccsdpt_aaa.sialx;
    src/sialx/qm/rccsdpt_aab.sialx;
    src/sialx/qm/rcis_rhf.sialx;
    src/sialx/qm/rcis_d_rhf.sialx;
    src/sialx/qm/rcis_ccpt2_rhf.sialx;
    src/sialx/qm/reom_rhf_defs.sialx;
    src/sialx/qm/rccsd_rhf_defs.sialx)

set(SIAL_COMPILER_JAR ${CMAKE_SOURCE_DIR}/compiler/SialCompiler.jar)
set(SIAL_COMPILER java -jar ${SIAL_COMPILER_JAR})
set (SIAL_COMPILED_FILES "")
foreach(F ${SIAL_FILES})
    get_filename_component(F_WE ${CMAKE_SOURCE_DIR}/${F} NAME_WE)
    get_filename_component(SIALX_FILE_PATH ${CMAKE_SOURCE_DIR}/${F} PATH)
    get_filename_component(SIALX_FILE_LAST_PATH ${SIALX_FILE_PATH} NAME)

    set(SIAL_COMPILE_DIR "${CMAKE_BINARY_DIR}/src/sialx/${SIALX_FILE_LAST_PATH}")
    add_custom_command(
        OUTPUT ${SIAL_COMPILE_DIR}/${F_WE}.siox
        COMMAND ${SIAL_COMPILER} -sp ${SIALX_FILE_PATH} ${CMAKE_SOURCE_DIR}/${F}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${SIAL_COMPILE_DIR}
        COMMAND mv -f ${SIALX_FILE_PATH}/${F_WE}.siox ${SIAL_COMPILE_DIR} 
        DEPENDS ${SIAL_COMPILER_JAR} ${CMAKE_SOURCE_DIR}/${F}
        COMMENT "Compiling ${F} to ${SIAL_COMPILE_DIR}/${F_WE}.siox")
    list (APPEND SIAL_COMPILED_FILES "${SIAL_COMPILE_DIR}/${F_WE}.siox")
endforeach()

add_custom_target(SIAL_FILES_COMPILE ALL DEPENDS ${SIAL_COMPILED_FILES})



