#ifndef __GPU_SUPER_INSTRUCTIONS_CU__
#define __GPU_SUPER_INSTRUCTIONS_CU__

#include "gpu_super_instructions.h"
#include "sip_interface.h"
#include <cublas_v2.h>
#include <cuda_check.h>
#include <cuda_runtime.h>
#include <assert.h>
#include <iostream>

// Internal helper functions
void __gpu_contract_helper(double* y, int ny, int* yDims, int* yInds,
		double* x1, int n1, int* x1Dims, int* x1Inds, double* x2, int n2,
		int* x2Dims, int* x2Inds);
void __gpu_permute_helper(double* y, int ny, int* yDims, int* yInds, double* x1,
		int n1, int* x1Dims, int* x1Inds);

void __gpu_matplus_helper(double* p_y, double* p_x1, double* p_x2, int numElems,
		const double alpha);

void printHostArray(double*, int);
void printGPUArray(double*, int);

__constant__ int dimsDev[MAX_RANK];
__constant__ int stepsDev[MAX_RANK];

cublasHandle_t cublasHandle;

/**
 * Initialization routine for CUDA
 * @param [out] device id (not used)
 */
void _init_gpu(int* devid, int* myRank) {
	int devCnt = 0;
	cudaDeviceProp deviceProp;

	cudaError_t err = cudaGetDeviceCount(&devCnt);

	int myDevice = -1;
	//CUDA_CHECK(cudaGetDevice(&myDevice));

	if (err == cudaSuccess) {
		//if (myDevice < 0 || myDevice > devCnt){
		//    printf ("Task %d : cudaGetDevice did not return a device (device id = %d)\n", *myRank,myDevice);
		myDevice = (*myRank) % devCnt;
		CUDA_CHECK(cudaSetDevice(myDevice));
		*devid = myDevice;
		//}
		printf("Task %d set device %d out of %d GPUs\n", *myRank, myDevice,
				devCnt);
		CUDA_CHECK(cudaGetDeviceProperties(&deviceProp, myDevice));
		printf("GPU Device %d: \"%s\" with compute capability %d.%d\n\n",
				myDevice, deviceProp.name, deviceProp.major, deviceProp.minor);
		CUBLAS_CHECK(cublasCreate(&cublasHandle));
	} else {
		*devid = -1;
		printf("Task %d not using GPUs, error returned :%s\n", *myRank,
				cudaGetErrorString(err));
	}

}

/**
 * Any cleanup that needs to be done on the GPU
 * is done here.
 */
void _finalize_gpu() {
//    CUBLAS_CHECK(cublasDestroy(cublasHandle));
}

///**
// * Implements Y = X1 + X2.
// * Does either Y+=X1 or Y+=X2
// * @param y         address of y (passed as double pointer since passed in from fortran)
// * @param x1        address of x1
// * @param x2        address of x2
// * @param numElems  number of elements
// */
//void _gpu_matplus(double* y, double* x1, double* x2, int numElems) {
//	double alpha = 1.0;
//	__gpu_matplus_helper(y, x1, x2, numElems, alpha);
//}
//
///**
// * Implements Y = X1 - X2.
// * To do Y-=X, pass  y=Y, x1=x, x2=y
// * @param y         address of y (passed as double pointer since passed in from fortran)
// * @param x1        address of x1
// * @param x2        address of x2
// * @param numElems  number of elements
// */
//void _gpu_matminus(double* y, double* x1, double* x2, int numElems) {
//	double alpha = -1.0;
//	__gpu_matplus_helper(y, x1, x2, numElems, alpha);
//}

/**
 * Implements X = X * alpa (scalar)
 * @param x         address of x2
 * @param alpha     scalar value
 * @param numElems  number of elements
 */
void _gpu_selfmultiply(double* p_x, const double alpha, int numElems) {
	double *x = p_x;
	CUBLAS_CHECK(cublasDscal(cublasHandle, numElems, &alpha, x, 1));
	CUDA_CHECK(cudaDeviceSynchronize());
}

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
void _gpu_contract(double* y, int ny, int* yDims, int* yInds,
		double* x1, int n1, int* x1Dims, int* x1Inds,
		double* x2, int n2, int* x2Dims, int *x2Inds) {

//	cudaPointerAttributes ptrAttr;
//	CUDA_CHECK(cudaPointerGetAttributes(&ptrAttr, y));
//	assert(ptrAttr.memoryType == cudaMemoryTypeDevice);
//	CUDA_CHECK(cudaPointerGetAttributes(&ptrAttr, x1));
//	assert(ptrAttr.memoryType == cudaMemoryTypeDevice);
//	CUDA_CHECK(cudaPointerGetAttributes(&ptrAttr, x2));
//	assert (ptrAttr.memoryType == cudaMemoryTypeDevice);


//        int yDims[MAX_RANK], x1Dims[MAX_RANK], x2Dims[MAX_RANK];
//        int i;
//        for(i = 0; i < MAX_RANK; i++) {
//            yDims[i] = nyb[i] - nya[i] + 1;
//            x1Dims[i] = nx1b[i] - nx1a[i] + 1;
//            x2Dims[i] = nx2b[i] - nx2a[i] + 1;
//        }
/************************************************************************/
//printf("X1\n");
//printGPUArray(x1, 10);
//printf("X2\n");
//printGPUArray(x2, 10);
/************************************************************************/
	__gpu_contract_helper(y, ny, yDims, yInds, x1, n1, x1Dims, x1Inds, x2, n2,
			x2Dims, x2Inds);
}

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
		double* x, int nx, int* xDims, int* xInds) {
//	int yDims[MAX_RANK], x1Dims[MAX_RANK];
//	int i;
//	for (i = 0; i < MAX_RANK; i++) {
//		yDims[i] = nyb[i] - nya[i] + 1;
//		x1Dims[i] = nx1b[i] - nx1a[i] + 1;
//	}
	__gpu_permute_helper(y, ny, yDims, yInds, x, nx, xDims, xInds);
}

/**
 * Mallocs a block of size numElems bytes on GPU
 * @param [in] numElems number of elements
 * @return pointer to allocation
 */
double* _gpu_allocate(int numElems) {

//	std::cout<< "_gpu_allocate called from "<<current_line()<<std::endl;

	double *gpuAddr = NULL;
	//printf ("load_temp : h_addr= %u, h_addr[0] = %lf\n", h_addr, h_addr[0]);
	CUDA_CHECK(cudaMalloc((void**)&gpuAddr, (numElems) * sizeof(double)));
	//CUDA_CHECK(cudaMemcpy(gpuAddr, h_addr, (*numElems)*sizeof(double), cudaMemcpyHostToDevice));
	//*g_addr = gpuAddr;
	CUDA_CHECK(cudaMemset((void*)gpuAddr, 0,(numElems) * sizeof(double)));
	CUDA_CHECK(cudaDeviceSynchronize());

	assert (gpuAddr != NULL);
//std::cout<<"Allocated on the GPU ..."<<gpuAddr<<std::endl;
//	cudaPointerAttributes ptrAttr;
//	CUDA_CHECK(cudaPointerGetAttributes(&ptrAttr, (void*)gpuAddr));
//	assert(ptrAttr.memoryType == cudaMemoryTypeDevice);
//	CUDA_CHECK(cudaDeviceSynchronize());

	//printf("load_temp : gpuAddr=%u, *g_addr=%u\n", gpuAddr, *g_addr);
	return gpuAddr;
}

/**
 * Copies block on GPU back to CPU
 * @param [in]  h_adr address of block on host (Already allocated on CPU)
 * @param [in]  g_adr address of block on gpu
 * @param [in]  numElems number of elements
 */
void _gpu_device_to_host(double* h_addr, double* g_addr, int numElems) {
	double *gpuAddr = g_addr;

//	cudaPointerAttributes ptrAttr;
//	CUDA_CHECK(cudaPointerGetAttributes(&ptrAttr, g_addr));
//	assert(ptrAttr.memoryType == cudaMemoryTypeDevice);
	//printf("unload : gpuAddr=%u, *g_addr=%u\n", gpuAddr, *g_addr);
	//printf("\nunloading gpuaddr :\n");
	//printArray(gpuAddr, 10);
	//printf ("unload : h_addr= %u, h_addr[0] = %lf\n", h_addr, h_addr[0]);
	//CUDA_CHECK(cudaMemcpy(gpuAddr, h_addr, (*numElems)*sizeof(double), cudaMemcpyHostToDevice));
	CUDA_CHECK(
			cudaMemcpy(h_addr, gpuAddr, (numElems) * sizeof(double),
					cudaMemcpyDeviceToHost));
	//CUDA_CHECK(cudaFree(gpuAddr)); //$$$$$ TODO GET RID OF THIS $$$$$
	CUDA_CHECK(cudaDeviceSynchronize());
}

/**
 * Copies a block of size numElems * sizeof(double)
 * bytes from Host to GPU
 * @param [in]  h_adr address of block on host
 * @param [in]  g_adr address of block on gpu
 * @param [in]  numElems number of elements
 */
void _gpu_host_to_device(double* h_addr, double* g_addr, int numElems) {

//	cudaPointerAttributes ptrAttr;
//	CUDA_CHECK(cudaPointerGetAttributes(&ptrAttr, g_addr));
//	assert(ptrAttr.memoryType == cudaMemoryTypeDevice);

	//double *gpuAddr = NULL;
	//printf ("load_input : h_addr= %u, h_addr[0] = %lf\n", h_addr, h_addr[0]);
	//CUDA_CHECK(cudaMalloc(&gpuAddr, (numElems)*sizeof(double)));
	CUDA_CHECK(cudaMemcpy(g_addr, h_addr, (numElems) * sizeof(double), cudaMemcpyHostToDevice));
	CUDA_CHECK(cudaDeviceSynchronize());
	//*g_addr = gpuAddr;
	//printf("load_input : gpuAddr=%u, *g_addr=%u\n", gpuAddr, *g_addr);
/***********************************************************************************************/
//printf("h_addr before copy");
//printHostArray(h_addr, 10);
//printf("g_addr after copy");
//printGPUArray(g_addr, 10);
/***********************************************************************************************/
}

/**
 * Copies a block of size numElems * sizeof(double)
 * bytes from a memory location dst to src, both on device
 * @param [in]  dst address of destination on device
 * @param [in]  src address of source on device
 * @param [in]  numElems number of elements
 */
void _gpu_device_to_device(double* dst, double *src, int numElems){
	CUDA_CHECK(cudaMemcpy(dst, src, (numElems) * sizeof(double), cudaMemcpyDeviceToDevice));
	CUDA_CHECK(cudaDeviceSynchronize());
}


/**
 * Frees block on the GPU
 * @param [in] block to be freed
 */
void _gpu_free(double* g_addr) {
	//double *gpuAddr = g_addr;

//	cudaPointerAttributes ptrAttr;
//	CUDA_CHECK(cudaPointerGetAttributes(&ptrAttr, g_addr));
//	assert(ptrAttr.memoryType == cudaMemoryTypeDevice);

	//printf("unload : gpuAddr=%u, *g_addr=%u\n", gpuAddr, *g_addr);
	CUDA_CHECK(cudaFree(g_addr));
	CUDA_CHECK(cudaDeviceSynchronize());
}

/**
 * Sets the value of all the doubles int a double block
 */
__global__ void doubleMemSet(double * x, double value, size_t count )
{
    size_t tid = threadIdx.x + blockIdx.x * blockDim.x;
    size_t stride = blockDim.x * gridDim.x;

    for(int i=tid; i<count; i+=stride) {
        x[i] = value;
    }
}

void _gpu_double_memset(double * g_addr, double value, int numElems){
	doubleMemSet<<<REORDER_BLOCKS, REORDER_THREADS>>>(g_addr, value, numElems);
	CUDA_CHECK(cudaGetLastError());
}



__global__ void reorderScatter(double* newX, double* oldX, int ndims,
		int size) {
	int blockstep = gridDim.x * blockDim.x;
	int oldIndex = blockIdx.x * blockDim.x + threadIdx.x;
	int newIndex;
	int t;
	int i;

	while (oldIndex < size) {
		t = oldIndex;

		newIndex = t % dimsDev[0] * stepsDev[0];
		t /= dimsDev[0];

		for (i = 1; i < ndims; i++) {
			newIndex += t % dimsDev[i] * stepsDev[i];
			t /= dimsDev[i];
		}

		newX[newIndex] = oldX[oldIndex];
		oldIndex += blockstep;
	}
}

__global__ void reorderGather(double* newX, double* oldX, int ndims, int size) {
	int blockstep = gridDim.x * blockDim.x;
	int newIndex = blockIdx.x * blockDim.x + threadIdx.x;
	int oldIndex;
	int t;
	int i;

	while (newIndex < size) {
		t = newIndex;
		oldIndex = t % dimsDev[0] * stepsDev[0];
		t /= dimsDev[0];

		for (i = 1; i < ndims; i++) {
			oldIndex += t % dimsDev[i] * stepsDev[i];
			t /= dimsDev[i];
		}

		newX[newIndex] = oldX[oldIndex];
		newIndex += blockstep;
	}
}

void printGPUArray(double* g_addr, int size) {
	double * h_addr = (double*) malloc(size * sizeof(double));
	CUDA_CHECK(cudaMemcpy(h_addr, g_addr, size * sizeof(double), cudaMemcpyDeviceToHost));
	printf("\n");
	for (int i = 0; i < size; i++)
		printf("g[%d]=%lf  ", i, h_addr[i]);
	printf("\n");
}

void printHostArray(double* h_addr, int size) {
	printf("\n");
	for (int i = 0; i < size; i++)
		printf("h[%d]=%lf  ", i, h_addr[i]);
	printf("\n");
}

void __gpu_contract_helper(double* p_y, int ny, int* yDims, int* yInds,
		double* p_x1, int n1, int* x1Dims, int* x1Inds, double* p_x2, int n2,
		int* x2Dims, int* x2Inds) {
	double *y = p_y;
	double *x1 = p_x1;
	double *x2 = p_x2;

	double* scratch1;
	double* scratch2;
	double* scratch3;


    // Determine number of elements in the largest of the 3 blocks
    int max_elems = 0;
    int elems = 1;
    for (int i=0; i<n1; i++)
        elems *= x1Dims[i];
    if (elems > max_elems)
        max_elems = elems;
    elems = 0;
    for (int i=0; i<n2; i++)
        elems *= x2Dims[i];
    if (elems > max_elems)
        max_elems = elems;
    elems = 0;
    for (int i=0; i<ny; i++)
        elems *= yDims[i];
    if (elems > max_elems)
        max_elems = elems;


	CUDA_CHECK(cudaMalloc(&scratch1, SCRATCH_BUFFER_SIZE_MB * 1024 * 1024));
	CUDA_CHECK(cudaMalloc(&scratch2, SCRATCH_BUFFER_SIZE_MB * 1024 * 1024));
	CUDA_CHECK(cudaMalloc(&scratch3, SCRATCH_BUFFER_SIZE_MB * 1024 * 1024));

    //long BUFF_SIZE = max_elems * sizeof(double); 
    //std::cout<<"Allocating " << BUFF_SIZE << " Memory on GPU for Scratch Blocks" <<std::endl;
	//CUDA_CHECK(cudaMalloc(&scratch1, BUFF_SIZE));
	//CUDA_CHECK(cudaMalloc(&scratch2, BUFF_SIZE));
	//CUDA_CHECK(cudaMalloc(&scratch3, BUFF_SIZE));

	//printf ("p_y=%u p_x1=%u, p_x2=%u\n", p_y, p_x1, p_x2);
	//printf ("y=%u x1=%u x2=%u \n", y, x1, x2);

	int steps[MAX_RANK];
	int yIndsP[MAX_RANK], yDimsP[MAX_RANK];
	int x1IndsP[MAX_RANK], x1DimsP[MAX_RANK];
	int x2IndsP[MAX_RANK], x2DimsP[MAX_RANK];
	int step;
	int lda, ldb;
	int i, j;
	int c, k;
	int size;
	int nc = (n1 + n2 - ny) / 2;
	bool isContractedIndex;

	// determine permutations of x1, x2, and y
	c = 0;
	k = 0;
	for (i = 0; i < n1; i++) {
		isContractedIndex = false;

		for (j = 0; j < n2; j++) {
			if (x1Inds[i] == x2Inds[j]) {
				isContractedIndex = true;
				x1IndsP[n1 - nc + c] = x1Inds[i];
				x1DimsP[n1 - nc + c] = x1Dims[i];
				x2IndsP[c] = x2Inds[j];
				x2DimsP[c] = x2Dims[j];
				c++;
				break;
			}
		}

		if (!isContractedIndex) {
			x1IndsP[k] = x1Inds[i];
			x1DimsP[k] = x1Dims[i];
			yIndsP[k] = x1Inds[i];
			yDimsP[k] = x1Dims[i];
			k++;
		}
	}

	c = 0;
	for (i = 0; i < n2; i++) {
		for (j = 0; j < ny; j++) {
			if (x2Inds[i] == yInds[j]) {
				x2IndsP[nc + c] = x2Inds[i];
				x2DimsP[nc + c] = x2Dims[i];
				yIndsP[k] = yInds[j];
				yDimsP[k] = yDims[j];
				k++;
				c++;
			}
		}
	}

	// copy x1 into scratch3 and then reorder into scratch1
	step = 1;
	for (i = 0; i < n1; i++) {
		for (j = 0; j < n1; j++)
			if (x1Inds[j] == x1IndsP[i]) {
				steps[j] = step;
				break;
			}
		step *= x1DimsP[i];
	}
	size = step;

	CUDA_CHECK(cudaMemcpyToSymbol(dimsDev, x1Dims, sizeof(int) * n1));
	CUDA_CHECK(cudaMemcpyToSymbol(stepsDev, steps, sizeof(int) * n1));

	CUDA_CHECK(
			cudaMemcpy(scratch3, x1, size * sizeof(double),
					cudaMemcpyDeviceToDevice));
	CUDA_CHECK(cudaDeviceSynchronize());
reorderScatter<<<REORDER_BLOCKS, REORDER_THREADS>>>(scratch1, scratch3, n1, size);

    	CUDA_CHECK(cudaGetLastError());
	CUDA_CHECK(cudaDeviceSynchronize());
	CUDA_CHECK(cudaGetLastError());

///***********************************************************************************************/
//printf("scratch1 after reorder\n");
//printGPUArray(scratch1, 10);
///***********************************************************************************************/
	// copy x2 into scratch3 and then reorder into scratch2
	step = 1;
	for (i = 0; i < n2; i++) {
		for (j = 0; j < n2; j++)
			if (x2Inds[j] == x2IndsP[i]) {
				steps[j] = step;
				break;
			}
		step *= x2DimsP[i];
	}
	size = step;

	CUDA_CHECK(cudaMemcpyToSymbol(dimsDev, x2Dims, sizeof(int) * n2));
	CUDA_CHECK(cudaMemcpyToSymbol(stepsDev, steps, sizeof(int) * n2));

	CUDA_CHECK(
			cudaMemcpy(scratch3, x2, size * sizeof(double),
					cudaMemcpyDeviceToDevice));
	CUDA_CHECK(cudaDeviceSynchronize());
reorderScatter<<<REORDER_BLOCKS, REORDER_THREADS>>>(scratch2, scratch3, n2, size);

    	CUDA_CHECK(cudaGetLastError());
	CUDA_CHECK(cudaDeviceSynchronize());
///***********************************************************************************************/
//printf("scratch2 after reorder\n");
//printGPUArray(scratch2, 10);
///***********************************************************************************************/
	// dGemm scratch1 and scratch2 into scratch 3
	double alpha = 1.0;
	double beta = 0.0;

	lda = 1;
	for (i = 0; i < n1 - nc; i++)
		lda *= x1DimsP[i];

	ldb = 1;
	for (i = 0; i < nc; i++)
		ldb *= x2DimsP[i];

	//printf ("Now doing cublasDgemm\n");
	CUBLAS_CHECK(
			cublasDgemm(cublasHandle, CUBLAS_OP_N, CUBLAS_OP_N, lda, size / ldb, ldb, &alpha, scratch1, lda, scratch2, ldb, &beta, scratch3, lda));

	CUDA_CHECK(cudaDeviceSynchronize());
///***********************************************************************************************/
//printf("scratch3 after dgemm\n");
//printGPUArray(scratch3, 10);
///***********************************************************************************************/
	//printf ("Now doing cublasDcopy\n");
	//CUBLAS_CHECK(cublasDcopy(cublasHandle, 100, scratch1, 1, scratch3, 1));

	CUDA_CHECK(cudaDeviceSynchronize());
	CUBLAS_CHECK(
			cublasDgemm(cublasHandle, CUBLAS_OP_N, CUBLAS_OP_N, lda, size / ldb, ldb, &alpha, scratch1, lda, scratch2, ldb, &beta, scratch3, lda));
	CUDA_CHECK(cudaDeviceSynchronize());
	CUDA_CHECK(cudaGetLastError());

	// reorder y from scratch3 to scratch1 and copy back from GPU
	step = 1;
	for (i = 0; i < ny; i++) {
		for (j = 0; j < ny; j++)
			if (yInds[j] == yIndsP[i]) {
				steps[j] = step;
				break;
			}
		step *= yDimsP[i];
	}
	size = step;

	CUDA_CHECK(cudaMemcpyToSymbol(dimsDev, yDims, sizeof(int) * ny));
	CUDA_CHECK(cudaMemcpyToSymbol(stepsDev, steps, sizeof(int) * ny));

reorderGather<<<REORDER_BLOCKS, REORDER_THREADS>>>(scratch1, scratch3, ny, size);
    	CUDA_CHECK(cudaGetLastError());
	CUDA_CHECK(cudaDeviceSynchronize());
	//printf ("y=%u x1=%u x2=%u \n", y, x1, x2);
	CUDA_CHECK(
			cudaMemcpy(y, scratch1, size * sizeof(double),
					cudaMemcpyDeviceToDevice));

//printf("y after reorder\n");
//printArray(y, 10);

	//printf ("end y=%u x1=%u x2=%u \n", y, x1, x2);

	CUDA_CHECK(cudaFree(scratch1));
	CUDA_CHECK(cudaFree(scratch2));
	CUDA_CHECK(cudaFree(scratch3));

	CUDA_CHECK(cudaDeviceSynchronize());

}

void __gpu_permute_helper(double* p_y, int ny, int* yDims, int* yInds,
		double* p_x1, int n1, int* x1Dims, int* x1Inds) {
	double *y = p_y;
	double *x1 = p_x1;

	double* scratch1;
	double* scratch3;

    // Determine number of elements in the larger of the 2 blocks
    int max_elems = 0;
    int elems = 1;
    for (int i=0; i<n1; i++)
        elems *= x1Dims[i];
    if (elems > max_elems)
        max_elems = elems;
    elems = 0;
    for (int i=0; i<ny; i++)
        elems *= yDims[i];
    if (elems > max_elems)
        max_elems = elems;



	CUDA_CHECK(cudaMalloc(&scratch1, SCRATCH_BUFFER_SIZE_MB * 1024 * 1024)); 
	CUDA_CHECK(cudaMalloc(&scratch3, SCRATCH_BUFFER_SIZE_MB * 1024 * 1024)); 
    //int BUFF_SIZE = max_elems * sizeof(double); 
	//CUDA_CHECK(cudaMalloc(&scratch1, BUFF_SIZE));
	//CUDA_CHECK(cudaMalloc(&scratch3, BUFF_SIZE));

	//printf ("permute p_y=%u p_x1=%u \n", p_y, p_x1);
	//printf ("permute y=%u x1=%u\n", y, x1);

	int steps[MAX_RANK];
	int yIndsP[MAX_RANK], yDimsP[MAX_RANK];
	int x1IndsP[MAX_RANK], x1DimsP[MAX_RANK];
	int step;
	int i, j;
	int c, k;
	int size;
	bool isContractedIndex;

	c = 0;
	k = 0;
	for (i = 0; i < n1; i++) {

		x1IndsP[i] = x1Inds[i];
		x1DimsP[i] = x1Dims[i];
		yIndsP[i] = x1Inds[i];
		yDimsP[i] = x1Dims[i];
	}

	// copy x1 into scratch3
	step = 1;
	for (i = 0; i < n1; i++) {
		for (j = 0; j < n1; j++)
			if (x1Inds[j] == x1IndsP[i]) {
				steps[j] = step;
				break;
			}
		step *= x1DimsP[i];
	}
	size = step;

	CUDA_CHECK(cudaMemcpyToSymbol(dimsDev, x1Dims, sizeof(int) * n1));
	CUDA_CHECK(cudaMemcpyToSymbol(stepsDev, steps, sizeof(int) * n1));

	CUDA_CHECK(cudaMemcpy(scratch3, x1, size * sizeof(double), cudaMemcpyDeviceToDevice));
	CUDA_CHECK(cudaDeviceSynchronize());

	// copy scratch1 into scratch3
	step = 1;
	for (i = 0; i < ny; i++) {
		for (j = 0; j < ny; j++)
			if (yInds[j] == x1IndsP[i]) {
				steps[j] = step;
				break;
			}
		step *= x1DimsP[i];
	}
	size = step;

	CUDA_CHECK(cudaMemcpyToSymbol(dimsDev, yDims, sizeof(int) * ny));
	CUDA_CHECK(cudaMemcpyToSymbol(stepsDev, steps, sizeof(int) * ny));

reorderGather<<<REORDER_BLOCKS, REORDER_THREADS>>>(scratch1, scratch3, ny, size);
    	CUDA_CHECK(cudaGetLastError());
	// copy scratch1 into y  and copy back from GPU
	CUDA_CHECK(cudaMemcpy(y, scratch1, size * sizeof(double), cudaMemcpyDeviceToDevice));

	//printf ("end permute y=%u x1=%u\n", y, x1);
	//printf ("end permute : p_y=%u p_x1=%u \n", p_y, p_x1);

	CUDA_CHECK(cudaFree(scratch1));
	CUDA_CHECK(cudaFree(scratch3));
	CUDA_CHECK(cudaDeviceSynchronize());
}

/**
 * Implements Y = Y + alpha * X.
 * @param y         address of y
 * @param x        	address of x
 * @param alpha 	alpha
 * @param numElems  number of elements
 */
void _gpu_axpy(double *p_y, double *p_x, const double alpha, int numElems){
	double *y = p_y;
	double *x = p_x;
	CUBLAS_CHECK(cublasDaxpy(cublasHandle, numElems, &alpha, x, 1, y, 1));
	CUDA_CHECK(cudaDeviceSynchronize());
}

///**
// * Does Y = alpha * X1 + X2
// * @param y         address of array y on GPU
// * @param x1        address of array x1 on GPU
// * @param x2        address of array x2 on GPU
// * @param numElems  number of elements in y, x1 & x2
// * @param alpha     scalar multiplier
// */
//void __gpu_matplus_helper(double* p_y, double* p_x1, double* p_x2, int numElems,
//		const double alpha) {
//
//	double *y = p_y;
//	double *x1 = p_x1;
//	double *x2 = p_x2;
//	if (y == x1) {
//		CUBLAS_CHECK(cublasDaxpy(cublasHandle, numElems, &alpha, x2, 1, x1, 1));
//	} else if (y == x2) {
//		CUBLAS_CHECK(cublasDaxpy(cublasHandle, numElems, &alpha, x1, 1, x2, 1));
//	} else {
//		CUDA_CHECK(
//				cudaMemcpy(y, x2, (numElems) * sizeof(double),
//						cudaMemcpyDeviceToDevice));
//		CUBLAS_CHECK(cublasDaxpy(cublasHandle, numElems, &alpha, x1, 1, y, 1));
//		//CUBLAS_CHECK(cublasDswap(cublasHandle, numElems, x2, 1, y, 1));
//	}
//	CUDA_CHECK(cudaDeviceSynchronize());
//}

#endif // __GPU_SUPER_INSTRUCTIONS_CU__

