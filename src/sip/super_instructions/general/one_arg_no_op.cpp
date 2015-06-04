/*
 * one_arg_no_op.cpp
 *
 * This is primarily used in test programs to ensure that a "get" on a block has
 * a subsequent use.
 *
 *  Created on: Jun 24, 2015
 *      Author: basbas
 */




void one_arg_no_op(int& array_slot, int& rank, int* index_values, int& size, int* extents,  double* data, int& ierr){
 //since this is called via a pointer, it's call should not be optimized away.
  }


