#include <iostream>
#include "sip_c_blas.h"
#include "sip.h"

#define MAX_SIZE 128*128*128*128
#define START_AT 16
#define REPEAT 10

int main(int argc, char **argv){

	std::cout << "Section 1 : Time to add a block into another using DAXPY" << std::endl;

	std::cout << "Size(in bytes)\tTime(in secs)"<< std::endl;
	for (int i = START_AT; i<=(MAX_SIZE); i = i * 2){
		double * to_add = new double[i];
		double * data_ = new double[i];

		double total_time = 0.0;
		for (int j=0; j < REPEAT; ++j){
			double start_time = GETTIME;
			int i_one = 1;
			double d_one = 1.0;
			int size__ = i;
			sip_blas_daxpy(size__, d_one, to_add, i_one, data_, i_one);
			double end_time = GETTIME;
			total_time += end_time - start_time;
		}
		std::cout << i << "\t\t" << total_time / REPEAT << std::endl << std::flush;

		delete [] to_add;
		delete [] data_;
	}

}
