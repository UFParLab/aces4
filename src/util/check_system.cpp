/*
 * print_init_file.cpp
 *
 *  Created on: Oct 16, 2014
 *      Author: njindal
 */

#include <mpi.h>
#include "barrier_support.h"




int main(int argc, char* argv[]) {


	MPI_Init(&argc, &argv);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0){
    int error, flag = 0;
    MPI_Aint mpi_tag_ub_ptr = 0;
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    error = MPI_Comm_get_attr(MPI_COMM_WORLD, MPI_TAG_UB, &mpi_tag_ub_ptr, &flag);
    int mpi_tag_ub = *(int*)mpi_tag_ub_ptr;
    if (error == MPI_SUCCESS && flag) std::cerr << "MPI_TAG_UB  = " << mpi_tag_ub << std::endl << std::flush;
    else std::cerr << "MPI_Comm_get_attr(MPI_COMM_WORLD, MPI_TAG_UB, &mpi_tag_ub, &flag); failed at rank " << rank << std::endl << std::flush;

    sip::BarrierSupport barrier_support;
    int max_tag = barrier_support.get_max_tag();
    std::cerr << "Max tag in barrier support = " << max_tag << std::endl;
    if (max_tag < 0) std::cerr << "Max tag in barrier support is negative" << std::endl;
    if (max_tag > mpi_tag_ub) std::cerr << "Max tag in barrier support  exceeds MPI_TAG_UB  for current system" << std::endl;
    if (max_tag > 32767) std::cerr << "Max tag in barrier support exceeds minimum MPI_TAG_UB in MPI standard" << std::endl;
    }

    MPI_Finalize();


	return 0;
}


