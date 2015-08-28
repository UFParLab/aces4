include $(top_srcdir)/aminclude.am
EXTRA_DIST = doxygen.cfg compiler/SialCompiler.jar

ACLOCAL_AMFLAGS = -I m4

#######################################################################
# ACES4
#######################################################################

bin_PROGRAMS=aces4 print_siptables

noinst_LIBRARIES = libtensordil.a
libtensordil_a_SOURCES = ./src/sip/tensor_algebra/tensor_dil_omp.F90
libtensordil_a_FFLAGS = $(FLIBS) $(FCLIBS)  $(OPENMP_FFLAGS) $(OPENMP_FCFLAGS)

EXTRA_DIST+=\
README.md\
README.devel\
LICENSE\
src/sip/setup/setup_interface.f\
src/sip/sip_interface.f

ACES_SETUPFILES=\
./src/sip/setup/io_utils.h\
./src/sip/setup/setup_reader.h\
./src/sip/setup/setup_interface.h\
./src/sip/setup/setup_writer.h\
./src/sip/setup/setup_reader.cpp\
./src/sip/setup/setup_interface.cpp\
./src/sip/setup/setup_writer.cpp\
./src/sip/setup/io_utils.cpp

ACES_CORE_FILES=\
./src/sip/core/array_constants.h\
./src/sip/core/array_constants.cpp\
./src/sip/core/sip.h\
./src/sip/core/sip.cpp\
./src/sip/core/aces_defs.h


ACES_DATA_FILES=\
./src/sip/static_data/array_table.h\
./src/sip/static_data/array_table.cpp\
./src/sip/static_data/index_table.h\
./src/sip/static_data/index_table.cpp\
./src/sip/static_data/int_table.h\
./src/sip/static_data/int_table.cpp\
./src/sip/static_data/sip_tables.h\
./src/sip/static_data/sip_tables.cpp\
./src/sip/static_data/opcode.h\
./src/sip/static_data/opcode.cpp\
./src/sip/static_data/op_table.h\
./src/sip/static_data/op_table.cpp\
./src/sip/dynamic_data/block_id.h\
./src/sip/dynamic_data/block_id.cpp\
./src/sip/dynamic_data/block_shape.h\
./src/sip/dynamic_data/block_shape.cpp\
./src/sip/dynamic_data/block_selector.h\
./src/sip/dynamic_data/block_selector.cpp\
./src/sip/dynamic_data/block.h\
./src/sip/dynamic_data/block.cpp\
./src/sip/dynamic_data/block_manager.h\
./src/sip/dynamic_data/block_manager.cpp\
./src/sip/dynamic_data/contiguous_array_manager.h\
./src/sip/dynamic_data/contiguous_array_manager.cpp\
./src/sip/dynamic_data/contiguous_local_array_manager.h\
./src/sip/dynamic_data/contiguous_local_array_manager.cpp\
./src/sip/dynamic_data/data_manager.h\
./src/sip/dynamic_data/data_manager.cpp\
./src/sip/dynamic_data/id_block_map.h\
./src/sip/dynamic_data/worker_persistent_array_manager.h\
./src/sip/dynamic_data/worker_persistent_array_manager.cpp\
./src/sip/dynamic_data/lru_array_policy.h\
./src/sip/dynamic_data/cached_block_map.h\
./src/sip/dynamic_data/cached_block_map.cpp\
./src/sip/dynamic_data/sip_timer.h\
./src/sip/dynamic_data/sip_timer.cpp
#need only header files for templates--these include .cpp

ACES_SOURCEFILES=\
./src/sip/worker/loop_manager.cpp\
./src/sip/worker/loop_manager.h\
./src/sip/worker/interpreter.cpp\
./src/sip/worker/interpreter.h\
./src/sip/worker/sialx_timer.cpp\
./src/sip/worker/sialx_timer.h\
./src/sip/worker/global_state.h\
./src/sip/worker/global_state.cpp\
./src/sip/worker/siox_reader.h\
./src/sip/worker/siox_reader.cpp\
./src/sip/worker/sial_ops_sequential.h\
./src/sip/worker/sial_ops_sequential.cpp\
./src/sip/worker/sial_printer.h\
./src/sip/worker/sial_printer.cpp\
./src/sip/worker/sial_math.h\
./src/sip/worker/sial_math.cpp\
./src/sip/sip_interface.h\
./src/sip/sip_interface.cpp\
./src/sip/tensor_algebra/tensor_ops_c_prototypes.h\
./src/sip/cuda/gpu_super_instructions.h\
./src/sip/mpi/sip_mpi_attr.h\
./src/sip/mpi/sip_mpi_attr.cpp\
$(ACES_SETUPFILES)\
$(ACES_CORE_FILES)\
$(ACES_DATA_FILES)

# MPI
if WORKING_MPI
ACES_SOURCEFILES +=\
./src/sip/mpi/rank_distribution.h \
./src/sip/mpi/rank_distribution.cpp \
./src/sip/mpi/data_distribution.cpp \
./src/sip/mpi/data_distribution.h\
./src/sip/mpi/sip_mpi_utils.h\
./src/sip/mpi/sip_mpi_utils.cpp\
./src/sip/mpi/sip_mpi_constants.h\
./src/sip/mpi/sip_mpi_constants.cpp\
./src/sip/mpi/barrier_support.h\
./src/sip/mpi/async_acks.h\
./src/sip/mpi/async_acks.cpp\
./src/sip/mpi/sip_server.h\
./src/sip/mpi/sip_server.cpp\
./src/sip/worker/sial_ops_parallel.h\
./src/sip/worker/sial_ops_parallel.cpp\
./src/sip/mpi/disk_backed_arrays_io.h\
./src/sip/mpi/disk_backed_arrays_io.cpp\
./src/sip/mpi/server_block.h\
./src/sip/mpi/server_block.cpp\
./src/sip/mpi/disk_backed_block_map.h\
./src/sip/mpi/disk_backed_block_map.cpp\
./src/sip/mpi/server_persistent_array_manager.h\
./src/sip/mpi/server_persistent_array_manager.cpp\
./src/sip/mpi/server_timer.h\
./src/sip/mpi/server_timer.cpp\
./src/sip/dynamic_data/mpi_state.h


endif

# CUDA
if WORKING_NVCC
    ACES_SOURCEFILES += ./src/sip/cuda/gpu_super_instructions.cu ./src/sip/cuda/cuda_check.h
endif

AM_LDADD_LIBS= $(LAPACK_LIBS) $(BLAS_LIBS) $(LIBS) $(FLIBS) $(FCLIBS) $(PAPI_LIBS) $(TAU_LIBS) $(NVCCLDFLAGS)

aces4_SOURCES= \
    $(ACES_SOURCEFILES) \
    ./src/aces_main/main.cpp

print_siptables_SOURCES=\
	$(ACES_SOURCEFILES)\
	./src/util/print_siptables.cpp

aces4_LDADD = \
	libtensordil.a \
	src/sip/super_instructions/libinstr.la\
	$(AM_LDADD_LIBS)

print_siptables_LDADD=\
	libtensordil.a \
	src/sip/super_instructions/libinstr.la\
	$(AM_LDADD_LIBS)
	
aces4_LDFLAGS = \
	$(OPENMP_FFLAGS)\
	$(OPENMP_FCFLAGS)\
	$(OPENMP_CFLAGS)\
	$(OPENMP_CXXFLAGS)

ACES4_CPP_FLAGS = \
-I${top_srcdir}/src/sip\
-I${top_srcdir}/src/sip/super_instructions\
-I${top_srcdir}/src/sip/tensor_algebra\
-I${top_srcdir}/src/sip/core\
-I${top_srcdir}/src/sip/setup\
-I${top_srcdir}/src/sip/worker\
-I${top_srcdir}/src/sip/server\
-I${top_srcdir}/src/sip/static_data\
-I${top_srcdir}/src/sip/dynamic_data\
-I${top_srcdir}/src/sip/mpi\
-I${top_srcdir}/src/sip/cuda\
-I${top_srcdir}/test\
-I${top_srcdir}/test/gtest-1.7.0/include

AM_CPPFLAGS = \
    $(ACES4_CPP_FLAGS)\
    $(PAPI_INCLUDE)\
    $(TAU_CPP)



#######################################################################
# tests
#######################################################################



if WORKING_MPI
TESTS = test_qm test_qm_aux test_sial test_unit test_basic_sial
check_PROGRAMS = test_qm  test_unit test_basic_sial test_sial
LOG_COMPILER = mpirun 	# Run mpi test with mpirun
AM_LOG_FLAGS = -np 2		# Run locally with 2 mpi ranks
else
TESTS = test_basic_sial test_sial test_qm test_qm_aux
check_PROGRAMS = test_basic_sial test_sial test_qm
#TESTS = test_simple
#check_PROGRAMS = test_simple
endif

test_unit_SOURCES = \
	$(ACES_SOURCEFILES)\
	test/test_unit.cpp

#test_mpi_qm_SOURCES = \
#	$(ACES_SOURCEFILES)\
#	test/test_controller_parallel.h\
#	test/test_controller_parallel.cpp\
#	test/test_constants.h\
#	test/test_constants.cpp\
#	test/test_mpi_qm.cpp


test_sial_SOURCES = \
    $(ACES_SOURCEFILES) \
    test/test_controller.h\
    test/test_controller.cpp\
    test/test_controller_parallel.h\
    test/test_controller_parallel.cpp\
    test/test_constants.h\
	test/test_constants.cpp\
    test/test_sial.cpp

test_basic_sial_SOURCES = \
    $(ACES_SOURCEFILES) \
    test/test_controller.h\
    test/test_controller.cpp\
    test/test_controller_parallel.h\
    test/test_controller_parallel.cpp\
    test/test_constants.h\
	test/test_constants.cpp\
    test/test_basic_sial.cpp\
    test/test_transpose_op.F\
    test/test_transpose4d_op.F\
    test/test_contraction_small2.F
    
    
test_qm_SOURCES = \
	$(ACES_SOURCEFILES) \
    test/test_controller_parallel.h\
    test/test_controller_parallel.cpp\
	test/test_controller.h\
    test/test_controller.cpp\
    test/test_constants.h\
    test/test_constants.cpp\
    test/test_qm.cpp\
    test/test_qm_aux.cpp

TEST_LDADD_VARS = \
	test/libgtest.la \
	test/libgtest_main.la \
	libtensordil.a \
	src/sip/super_instructions/libinstr.la\
	$(AM_LDADD_LIBS) 
#	$(PTHREAD_LIBS) 
	
test_unit_LDADD = $(TEST_LDADD_VARS)	
test_sial_LDADD = $(TEST_LDADD_VARS)
test_qm_LDADD = $(TEST_LDADD_VARS)
test_qm_aux_LDADD = $(TEST_LDADD_VARS)
#test_mpi_qm_LDADD = $(TEST_LDADD_VARS)
test_mpi_simple_LDADD = $(TEST_LDADD_VARS)
test_basic_sial_LDADD = $(TEST_LDADD_VARS)
# Add any initialization files needed by tests here.
TEST_SUPPORT_FILES=\
test/ccsdpt_test.dat\
test/second_ccsdpt_test.dat\
test/cis_test.dat\
test/mcpt2_test.dat\
test/eom_test.dat\
test/eom_water-dimer_test.dat\
test/eom_lccsd_test.dat\
test/eom_lccd_test.dat\
test/eom_mp2_test.dat\
test/lindep_test.dat\
test/rlambda_test.dat\
test/lccd_test.dat\
test/lccsd_test.dat\
test/expected_output/helloworld1.txt\
test/expected_output/local_arrays_wild.txt\
test/expected_output/local_arrays.txt\
test/expected_output/loop_over_simple_indices.txt\
test/expected_output/scalar_valued_blocks.txt\
test/expected_output/scalars.txt\
test/expected_output/static_array_test.txt\
test/expected_output/static_array_test2.txt\
test/expected_output/tmp_arrays_2.txt\
test/expected_output/tmp_arrays.txt


EXTRA_DIST+= $(TEST_SUPPORT_FILES)

check_DATA = copy-data-files.stamp

copy-data-files.stamp:
	cp $(top_srcdir)/test/*.dat .
	cp $(top_srcdir)/test/expected_output/*.txt .


# Pthreads are not used, to enable, uncomment these. 
#if HAVE_PTHREADS
#	test_simple_LDADD += $(PTHREAD_LIBS)
#	test_simple_CFLAGS = $(PTHREAD_CFLAGS)
#	test_qm_LDADD += $(PTHREAD_LIBS)
#	test_qm_CFLAGS = $(PTHREAD_CFLAGS)
#	test_mpi_qm_LDADD += $(PTHREAD_LIBS)
#	test_mpi_qm_CFLAGS += $(PTHREAD_CFLAGS)
#	test_mpi_simple_LDADD += $(PTHREAD_LIBS)
#	test_mpi_simple_CFLAGS += $(PTHREAD_CFLAGS)
#endif

# Google TESTS
check_LTLIBRARIES = test/libgtest.la test/libgtest_main.la

nodist_test_libgtest_la_SOURCES = test/gtest-1.7.0/src/gtest-all.cc

nodist_test_libgtest_main_la_SOURCES = test/gtest-1.7.0/src/gtest_main.cc
test_libgtest_main_la_LIBADD = test/libgtest.la

GTEST_CPP_FLAG_VARS = -I$(top_srcdir)/test/gtest-1.7.0/include  -I$(top_srcdir)/test/gtest-1.7.0/ -I$(top_srcdir)/test/gtest-1.7.0/src
test_libgtest_la_CPPFLAGS = $(GTEST_CPP_FLAG_VARS)
test_libgtest_main_la_CPPFLAGS = $(GTEST_CPP_FLAG_VARS)

# Disable use of PTHREADS for Google tests for the time being
test_libgtest_la_CXXFLAGS = -DGTEST_HAS_PTHREAD=0
test_libgtest_main_la_CXXFLAGS = -DGTEST_HAS_PTHREAD=0



# To enable static linking and generating an executable in the tests directory.
# Has been disabled since lblas & llapack cannot be statically linked to an executable on Ubuntu
# Otherwise, while linker compalins of Multiple definitions of xerbla_
# test_LDFLAGS = -all-static	$(OPENMP_FFLAGS) $(OPENMP_FCFLAGS) 

# -no-install taken from https://www.flameeyes.eu/autotools-mythbuster/libtool/wrappers.html
TEST_LDFLAGS_VARS = $(OPENMP_FFLAGS) $(OPENMP_FCFLAGS) $(OPENMP_CFLAGS) $(OPENMP_CXXFLAGS) -no-install
test_unit_LDFLAGS = $(TEST_LDFLAGS_VARS)
test_sial_LDFLAGS = $(TEST_LDFLAGS_VARS)
test_qm_LDFLAGS = $(TEST_LDFLAGS_VARS)
test_qm_aux_LDFLAGS = $(TEST_LDFLAGS_VARS)
#test_mpi_qm_LDFLAGS = $(TEST_LDFLAGS_VARS)
test_mpi_simple_LDFLAGS = $(TEST_LDFLAGS_VARS)
test_basic_sial_LDFLAGS = $(TEST_LDFLAGS_VARS)

SUBDIRS = src/sip/super_instructions


#######################################################################
# SIALX Files
#######################################################################

# From http://stackoverflow.com/questions/8039196/automake-automatic-dependencies-custom-language-scripts
# Compile sialx files
SIAL_COMPILER_JAR=$(top_srcdir)/compiler/SialCompiler.jar

SIOX_FILES=\
./src/sialx/test/empty.siox\
./src/sialx/test/static_array_test.siox\
./src/sialx/test/ifelse.siox\
./src/sialx/test/put_test.siox\
./src/sialx/test/index_decs.siox\
./src/sialx/test/predef_scalars.siox\
./src/sialx/test/print_block_test.siox\
./src/sialx/test/tmp_arrays.siox\
./src/sialx/test/scalars.siox\
./src/sialx/test/insert_slice_test.siox\
./src/sialx/test/print_scalar.siox\
./src/sialx/test/where_clause.siox\
./src/sialx/test/loop_over_simple_indices.siox\
./src/sialx/test/exit_statement_test.siox\
./src/sialx/test/no_arg_user_sub.siox\
./src/sialx/test/helloworld.siox\
./src/sialx/test/pardo_loop.siox\
./src/sialx/test/subindex_test.siox\
./src/sialx/test/local_arrays.siox\
./src/sialx/test/scalarTable.siox\
./src/sialx/test/stringLiterals.siox\
./src/sialx/test/aces_defs.siox\
./src/sialx/test/fill_sequential_test.siox\
./src/sialx/test/local_arrays_wild.siox\
./src/sialx/test/local_arrays_wild_fail.siox\
./src/sialx/test/basic_assign_to_static_array_test.siox\
./src/sialx/test/sum_op_test.siox\
./src/sialx/test/transpose_tmp.siox\
./src/sialx/test/transpose4d_tmp.siox\
./src/sialx/test/transpose4d_square_tmp.siox\
./src/sialx/test/tmp_arrays_2.siox\
./src/sialx/test/contraction_small_test.siox\
./src/sialx/test/contraction_small_test2.siox\
./src/sialx/test/get_int_array_test.siox\
./src/sialx/test/get_scalar_array_test.siox\
./src/sialx/test/get_scratch_array_test.siox\
./src/sialx/test/gpu_contraction_small_test.siox\
./src/sialx/test/gpu_ops.siox\
./src/sialx/test/contract_to_scalar.siox\
./src/sialx/test/gpu_sum_op_test.siox\
./src/sialx/test/gpu_contract_to_scalar.siox\
./src/sialx/test/gpu_transpose_tmp.siox\
./src/sialx/test/self_multiply_test.siox\
./src/sialx/test/gpu_self_multiply_test.siox\
./src/sialx/test/simple_indices_assignments.siox\
./src/sialx/test/add_and_subtract_scalars.siox\
./src/sialx/test/gpu_contraction_predefined_test.siox\
./src/sialx/test/assign_to_static_array_test.siox\
./src/sialx/test/set_persistent_test.siox\
./src/sialx/test/restore_persistent_test.siox\
./src/sialx/test/persistent_static_array_test1.siox\
./src/sialx/test/persistent_static_array_test2.siox\
./src/sialx/test/persistent_distributed_array_test1.siox\
./src/sialx/test/persistent_distributed_array_test2.siox\
./src/sialx/test/put_test_mpi.siox\
./src/sialx/test/put_accumulate_mpi.siox\
./src/sialx/test/put_initialize.siox\
./src/sialx/test/put_increment.siox\
./src/sialx/test/delete_mpi.siox\
./src/sialx/test/get_mpi.siox\
./src/sialx/test/persistent_distributed_array_mpi1.siox\
./src/sialx/test/persistent_distributed_array_mpi2.siox\
./src/sialx/test/persistent_empty_mpi1.siox\
./src/sialx/test/persistent_empty_mpi2.siox\
./src/sialx/test/persistent_scalars_1.siox\
./src/sialx/test/persistent_scalars_2.siox\
./src/sialx/test/unmatched_get.siox\
./src/sialx/test/all_rank_print_test.siox\
./src/sialx/test/message_number_wraparound_test.siox\
./src/sialx/test/pardo_loop_1d.siox\
./src/sialx/test/pardo_loop_2d.siox\
./src/sialx/test/pardo_loop_3d.siox\
./src/sialx/test/pardo_loop_4d.siox\
./src/sialx/test/pardo_loop_5d.siox\
./src/sialx/test/pardo_loop_6d.siox\
./src/sialx/test/scalar_ops.siox\
./src/sialx/test/int_ops.siox\
./src/sialx/test/int_self_ops.siox\
./src/sialx/test/scalar_valued_blocks.siox\
./src/sialx/test/broadcast_static.siox\
./src/sialx/test/block_scale_assign.siox\
./src/sialx/test/return_sval_test.siox\
./src/sialx/test/index_scalar_cast.siox\
./src/sialx/test/contig_local3.siox\
./src/sialx/test/contiguous_local.siox\
./src/sialx/test/contiguous_local_2.siox\
./src/sialx/test/contiguous_local_allocate_twice_fail.siox\
./src/sialx/test/block_scale_assign.siox\
./src/sialx/test/return_sval_test.siox\
./src/sialx/test/broadcast_static.siox\
./src/sialx/test/cached_block_map_test.siox\
./src/sialx/test/read_block_test.siox\
./src/sialx/test/pardo_with_where.siox\
./src/sialx/test/pardo_load_balance_test.siox\
./src/sialx/test/put_initialize.siox\
./src/sialx/test/cast_indices_to_simple.siox\
./src/sialx/test/aoladder.siox\
./src/sialx/qm/scf_defs.siox\
./src/sialx/qm/scf_rhf_coreh.siox\
./src/sialx/qm/scf_frag_lowmem.siox\
./src/sialx/qm/scf_frag_defs_lowmem.siox\
./src/sialx/qm/drop_core_in_sial.siox\
./src/sialx/qm/frag_2order_corr_lowmem.siox\
./src/sialx/qm/mcpt2_corr_lowmem.siox\
./src/sialx/qm/mcpt2_singles_lowmem.siox\
./src/sialx/qm/mcpt2_doubles_lowmem.siox\
./src/sialx/qm/trans_frag_defs.siox\
./src/sialx/qm/mp2_rhf_disc.siox\
./src/sialx/qm/rcis_rhf.siox\
./src/sialx/qm/rcis_d_rhf.siox\
./src/sialx/qm/rcis_ccpt2_rhf.siox\
./src/sialx/qm/lr_eom_ccsd_rhf.siox\
./src/sialx/qm/lr_eom_linccsd_rhf.siox\
./src/sialx/qm/reom_rhf_defs.siox\
./src/sialx/qm/rlccd_rhf.siox\
./src/sialx/qm/rlccsd_rhf.siox\
./src/sialx/qm/tran_rhf_no4v.siox\
./src/sialx/qm/tran_uhf_no4v.siox\
./src/sialx/qm/tran_rhf_no3v.siox\
./src/sialx/qm/tran_eom.siox\
./src/sialx/qm/tran_rhf_no4v_defs.siox\
./src/sialx/qm/rccsd_rhf.siox\
./src/sialx/qm/rlambda_rhf.siox\
./src/sialx/qm/rccpt1p2_prop.siox\
./src/sialx/qm/rccsd_rhf_defs.siox\
./src/sialx/qm/rccsdpt_aaa.siox\
./src/sialx/qm/rccsdpt_aab.siox\
./src/sialx/qm/scf_rhf.siox\
./src/sialx/qm/scf_rhf_defs.siox

BUILT_SOURCES= $(SIOX_FILES)

EXTRA_DIST += \
./src/sialx/test/empty.sialx\
./src/sialx/test/static_array_test.sialx\
./src/sialx/test/ifelse.sialx\
./src/sialx/test/put_test.sialx\
./src/sialx/test/index_decs.sialx\
./src/sialx/test/predef_scalars.sialx\
./src/sialx/test/print_block_test.sialx\
./src/sialx/test/tmp_arrays.sialx\
./src/sialx/test/scalars.sialx\
./src/sialx/test/insert_slice_test.sialx\
./src/sialx/test/print_scalar.sialx\
./src/sialx/test/where_clause.sialx\
./src/sialx/test/loop_over_simple_indices.sialx\
./src/sialx/test/exit_statement_test.sialx\
./src/sialx/test/no_arg_user_sub.sialx\
./src/sialx/test/helloworld.sialx\
./src/sialx/test/pardo_loop.sialx\
./src/sialx/test/subindex_test.sialx\
./src/sialx/test/local_arrays.sialx\
./src/sialx/test/scalarTable.sialx\
./src/sialx/test/stringLiterals.sialx\
./src/sialx/test/aces_defs.sialx\
./src/sialx/test/fill_sequential_test.sialx\
./src/sialx/test/local_arrays_wild.sialx\
./src/sialx/test/local_arrays_wild_fail.sialx\
./src/sialx/test/basic_assign_to_static_array_test.sialx\
./src/sialx/test/sum_op_test.sialx\
./src/sialx/test/transpose_tmp.sialx\
./src/sialx/test/transpose4d_tmp.sialx\
./src/sialx/test/transpose4d_square_tmp.sialx\
./src/sialx/test/tmp_arrays_2.sialx\
./src/sialx/test/contraction_small_test.sialx\
./src/sialx/test/contraction_small_test2.sialx\
./src/sialx/test/get_int_array_test.sialx\
./src/sialx/test/get_scalar_array_test.sialx\
./src/sialx/test/get_scratch_array_test.sialx\
./src/sialx/test/gpu_contraction_small_test.sialx\
./src/sialx/test/gpu_ops.sialx\
./src/sialx/test/contract_to_scalar.sialx\
./src/sialx/test/gpu_sum_op_test.sialx\
./src/sialx/test/gpu_contract_to_scalar.sialx\
./src/sialx/test/gpu_transpose_tmp.sialx\
./src/sialx/test/self_multiply_test.sialx\
./src/sialx/test/gpu_self_multiply_test.sialx\
./src/sialx/test/simple_indices_assignments.sialx\
./src/sialx/test/add_and_subtract_scalars.sialx\
./src/sialx/test/gpu_contraction_predefined_test.sialx\
./src/sialx/test/assign_to_static_array_test.sialx\
./src/sialx/test/set_persistent_test.sialx\
./src/sialx/test/restore_persistent_test.sialx\
./src/sialx/test/persistent_static_array_test1.sialx\
./src/sialx/test/persistent_static_array_test2.sialx\
./src/sialx/test/persistent_distributed_array_test1.sialx\
./src/sialx/test/persistent_distributed_array_test2.sialx\
./src/sialx/test/put_test_mpi.sialx\
./src/sialx/test/put_accumulate_mpi.sialx\
./src/sialx/test/put_increment.sialx\
./src/sialx/test/delete_mpi.sialx\
./src/sialx/test/get_mpi.sialx\
./src/sialx/test/persistent_distributed_array_mpi1.sialx\
./src/sialx/test/persistent_distributed_array_mpi2.sialx\
./src/sialx/test/persistent_empty_mpi1.sialx\
./src/sialx/test/persistent_empty_mpi2.sialx\
./src/sialx/test/persistent_scalars_1.sialx\
./src/sialx/test/persistent_scalars_2.sialx\
./src/sialx/test/unmatched_get.sialx\
./src/sialx/test/all_rank_print_test.sialx\
./src/sialx/test/message_number_wraparound_test.sialx\
./src/sialx/test/pardo_loop_1d.sialx\
./src/sialx/test/pardo_loop_2d.sialx\
./src/sialx/test/pardo_loop_3d.sialx\
./src/sialx/test/pardo_loop_4d.sialx\
./src/sialx/test/pardo_loop_5d.sialx\
./src/sialx/test/pardo_loop_6d.sialx\
./src/sialx/test/scalar_ops.sialx\
./src/sialx/test/int_ops.sialx\
./src/sialx/test/int_self_ops.sialx\
./src/sialx/test/scalar_valued_blocks.sialx\
./src/sialx/test/broadcast_static.sialx\
./src/sialx/test/block_scale_assign.sialx\
./src/sialx/test/return_sval_test.sialx\
./src/sialx/test/index_scalar_cast.sialx\
./src/sialx/test/contig_local3.sialx\
./src/sialx/test/contiguous_local.sialx\
./src/sialx/test/contiguous_local_2.sialx\
./src/sialx/test/contiguous_local_allocate_twice_fail.sialx\
./src/sialx/test/block_scale_assign.sialx\
./src/sialx/test/return_sval_test.sialx\
./src/sialx/test/broadcast_static.sialx\
./src/sialx/test/cached_block_map_test.sialx\
./src/sialx/test/read_block_test.sialx\
./src/sialx/test/pardo_with_where.sialx\
./src/sialx/test/pardo_load_balance_test.sialx\
./src/sialx/test/put_initialize.sialx\
./src/sialx/test/cast_indices_to_simple.sialx\
./src/sialx/test/aoladder.sialx\
./src/sialx/qm/scf_rhf_coreh.sialx\
./src/sialx/qm/scf_defs.sialx\
./src/sialx/qm/scf_frag_lowmem.sialx\
./src/sialx/qm/scf_frag_defs_lowmem.sialx\
./src/sialx/qm/drop_core_in_sial.sialx\
./src/sialx/qm/trans_frag_defs.sialx\
./src/sialx/qm/frag_2order_corr_lowmem.sialx\
./src/sialx/qm/mcpt2_corr_lowmem.sialx\
./src/sialx/qm/mcpt2_singles_lowmem.sialx\
./src/sialx/qm/mcpt2_doubles_lowmem.sialx\
./src/sialx/qm/mp2_rhf_disc.sialx\
./src/sialx/qm/cis_rhf.sialx\
./src/sialx/qm/cis_rhf_defs.sialx\
./src/sialx/qm/rcis_rhf.sialx\
./src/sialx/qm/rcis_d_rhf.sialx\
./src/sialx/qm/rcis_ccpt2_rhf.sialx\
./src/sialx/qm/reom_rhf_defs.sialx\
./src/sialx/qm/lr_eom_ccsd_rhf.sialx\
./src/sialx/qm/lr_eom_linccsd_rhf.sialx\
./src/sialx/qm/rlccd_rhf.sialx\
./src/sialx/qm/rlccsd_rhf.sialx\
./src/sialx/qm/tran_rhf_no4v.sialx\
./src/sialx/qm/tran_uhf_no4v.sialx\
./src/sialx/qm/tran_rhf_no3v.sialx\
./src/sialx/qm/tran_eom.sialx\
./src/sialx/qm/tran_rhf_no4v_defs.sialx\
./src/sialx/qm/rccsd_rhf.sialx\
./src/sialx/qm/rlambda_rhf.sialx\
./src/sialx/qm/rccpt1p2_prop.sialx\
./src/sialx/qm/rccsd_rhf_defs.sialx\
./src/sialx/qm/rccsdpt_aaa_defs.sialx\
./src/sialx/qm/rccsdpt_aaa.sialx\
./src/sialx/qm/rccsdpt_aab_defs.sialx\
./src/sialx/qm/rccsdpt_aab.sialx\
./src/sialx/qm/scf_rhf.sialx\
./src/sialx/qm/scf_rhf_defs.sialx

#./src/sialx/qm/scf_mp2_rhf.sialx\
#./src/sialx/qm/scf_mp2_defs.sialx\
#./src/sialx/qm/rlambda_rhf.sialx

SIALX_DIR = ${top_srcdir}/src/sialx/
.sialx.siox: $(SIAL_COMPILER_JAR)
	java -jar $(SIAL_COMPILER_JAR) -sp $(<D) $<
	$(MKDIR_P) `dirname $@`
	-mv -f $(patsubst %.sialx, %.siox, $<) $@

#######################################################################
# Clean Compiled SIALX Files
#######################################################################

CLEANFILES = tensor_dil_omp.mod $(SIOX_FILES)


#######################################################################
# CUDA Compile Rule
#######################################################################

.cu.o:
	$(NVCC) -c $(AM_CPPFLAGS) $(CPPFLAGS) $(NVCCCPPFLAGS) -o $@ $<

#######################################################################
# CTAGS For SIALX
#######################################################################

AM_CTAGSFLAGS = --langdef=SIALX --langmap=SIALX:.sialx --regex-SIALX='/[\t ]*proc[ \t]*([a-zA-Z0-9_]+)/\1/p,procedure definition/i' --regex-SIALX='/[\t ]*(index|aoindex|moaindex|mobindex)[ \t]*([a-zA-Z0-9_]+)/\2/i,index definition/i' --regex-SIALX='/[\t ]*(distributed|served|static|temp|local)[ \t]*([a-zA-Z0-9_]+)/\2/a,array definition/i' --regex-SIALX='/[\t ]*(scalar)[ \t]*([a-zA-Z0-9_]+)/\2/s,scalar definition/i' -R $(top_srcdir)/src/sialx/


