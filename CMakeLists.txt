project(aces4)

enable_language(Fortran)

cmake_minimum_required (VERSION 2.8 FATAL_ERROR)

find_package(MPI)
find_package(LAPACK REQUIRED)

option(HAVE_MPI "Whether to use MPI" ON)

if (HAVE_MPI AND !MPI_CXX_FOUND)
    message(FATAL_ERROR "MPI could not be found, consider building with USE_MPI = off")
endif()

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

# MPI
if (HAVE_MPI AND MPI_CXX_FOUND)
    set(ACES_MPI_FILES
        src/sip/mpi/sip_mpi_attr.h;
        src/sip/mpi/sip_mpi_attr.cpp;
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
        src/sip/dynamic_data/mpi_state.h)
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
    src/sip/dynamic_data/cached_block_map.cpp)
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
    src/sip/sip_interface.h;
    src/sip/sip_interface.cpp;
    src/sip/tensor_algebra/tensor_ops_c_prototypes.h;
    src/sip/cuda/gpu_super_instructions.h)
    
set(aces4_SRCS
    ${ACES_SETUP_FILES};
    ${ACES_CORE_FILES};
    ${ACES_DATA_FILES};
    ${ACES_MPI_FILES};
    ${ACES_WORKER_FILES})


set(tensordil_SRCS src/sip/tensor_algebra/tensor_dil_omp.F90)

configure_file(${CMAKE_SOURCE_DIR}/config.h.cmake ${CMAKE_BINARY_DIR}/config.h)

include_directories(
    ${CMAKE_BINARY_DIR};
    ${MPI_INCLUDE_PATH};
    src/sip;
    src/sip/super_instructions;
    src/sip/tensor_algebra;
    src/sip/core;
    src/sip/setup;
    src/sip/worker;
    src/sip/static_data;
    src/sip/dynamic_data;
    src/sip/mpi;
    src/sip/cuda;
    test)

add_subdirectory(src/sip/super_instructions)

add_library(tensordil ${tensordil_SRCS})

add_executable(aces4 src/aces_main/main.cpp ${aces4_SRCS})
set_target_properties(aces4 PROPERTIES COMPILE_FLAGS "${MPI_COMPILE_FLAGS}")
set_target_properties(aces4 PROPERTIES LINK_FLAGS "${MPI_LINK_FLAGS} ${LAPACK_LINKER_FLAGS}")

target_link_libraries(aces4 tensordil ${MPI_LIBRARIES} ${CMAKE_BINARY_DIR}/src/sip/super_instructions/libsuperinstructions.a ${LAPACK_LIBRARIES}) 



