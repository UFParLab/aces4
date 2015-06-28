/*
 * one_arg_no_op.cpp
 *
 * This is used in test programs to ensure that a "get" on a block has
 * a subsequent use.
 *
 * Usage:  execute one_arg_no_op a[i,j,k,l]
 *
 * Declaration:  special one_arg_no_op r
 *
 *  Created on: Jun 24, 2015
 *      Author: basbas
 */




void one_arg_no_op(int& array_slot, int& rank, int* index_values, int& size, int* extents,  double* data, int& ierr){
 //since this is called via a pointer, its call should not be optimized away.
  }


