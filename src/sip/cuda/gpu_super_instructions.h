/*
 * gpu_super_instructions.h
 *
 *  Created on: Nov 4, 2013
 *      Author: jindal
 */

#ifndef GPU_SUPER_INSTRUCTIONS_H_
#define GPU_SUPER_INSTRUCTIONS_H_

#include "aces_defs.h"
#include <stdio.h>

//**************************************************
// Configuration
//**************************************************
//#define MAX_DIMS MAX_RANK
#define SCRATCH_BUFFER_SIZE_MB 40
#define REORDER_BLOCKS 48
#define REORDER_THREADS 512

/**
 * Initialization routine for CUDA
 * @param [out] device id (not used)
 */
void _init_gpu(int* devid, int* myRank);

/**
 * Implements Y = Y + alpha * X.
 * @param y         address of y
 * @param x        	address of x
 * @param alpha 	alpha
 * @param numElems  number of elements
 */
void _gpu_axpy(double* y, double* x, const double alpha, int numElems);


/**
 * Sets all the values in a block to a given value
 * @param g_addr
 * @param value
 * @param numElems
 */
void _gpu_double_memset(double * g_addr, double value, int numElems);


/**
 * Implements X = X * alpa (scalar)
 * @param x         address of x2
 * @param alpha     scalar value
 * @param numElems  number of elements
 */
void _gpu_selfmultiply(double* x, const double alpha, int numElems);

/**
 * implements y = x1 * x2 (contraction)
 * @param y         address of block y on GPU
 * @param ny        number of indices in y
 * @param yDims     ranges of indices of y
 * @param yInds     labels of y
 * @param x1        address of block x1 on GPU
 * @param n1        number of indices in x1
 * @param x1Dims    ranges of indices of x1
 * @param x1Inds    labels of x1
 * @param x2        address of block x2 on GPU
 * @param n2        number of indices in x2
 * @param x2Dims    ranges of indices of x2
 * @param x2Inds    labels of x2
 */
void _gpu_contract(double* y, int ny, int* yDims, int* yInds, double* x1,
		int n1, int* x1Dims, int* x1Inds, double* x2, int n2, int* x2Dims,
		int *x2Inds);

/**
 * implements y = x1 (permutation / assignment / transpose)
 * implements y = x1 * x2 (contraction)
 * @param y         address of block y on GPU
 * @param ny        number of indices in y
 * @param yDims     ranges of indices of y
 * @param yInds     labels of y
 * @param x        address of block x1 on GPU
 * @param nx        number of indices in x1
 * @param xDims    ranges of indices of x1
 * @param xInds    labels of x1
 */
void _gpu_permute(double* y, int ny, int* yDims, int* yInds,
		double* x, int nx, int* xDims, int* xInds);

/**
 * Copies a block of size numElems * sizeof(double)
 * bytes on GPU from contents of h_addr there
 * @param [in]  h_adr address of block on host
 * @param [in]  g_adr address of block on gpu
 * @param [in]  numElems number of elements
 */
void _gpu_host_to_device(double* c_addr, double* g_addr, int numElems);

/**
 * Mallocs a block of size numElems bytes on GPU
 * @param [in] numElems number of elements
 * @return pointer to allocation
 */
double* _gpu_allocate(int numElems);

/**
 * Copies block on GPU back to CPU
 * @param [in]  h_adr address of block on host (Already allocated on CPU)
 * @param [in]  g_adr address of block on gpu
 * @param [in]  numElems number of elements
 */
void _gpu_device_to_host(double* c_addr, double* g_addr, int numElems);

/**
 * Copies a block of size numElems * sizeof(double)
 * bytes from a memory location dst to src, both on device
 * @param [in]  dst address of destination on device
 * @param [in]  src address of source on device
 * @param [in]  numElems number of elements
 */
void _gpu_device_to_device(double* dst, double *src, int numElems);

/**
 * Frees block on the GPU
 * @param [in] block to be freed
 */
void _gpu_free(double* g_addr);

#endif /* GPU_SUPER_INSTRUCTIONS_H_ */
