/*! C++ prototypes allowing the fortran routines in tensor_dil_omp.f90
 *
 *
 *    #include in the C++ files that call these routines.
 *
 *    @see tensor_dil_omp.f90
 */

#ifndef TENSOR_OPS_C_PROTOTYPES_H_
#define TENSOR_OPS_C_PROTOTYPES_H_

extern "C" {


//these are the prototypes that go with tensor_dil_omp_13_9_4.f90 as changed by me to pass the size to tensor_block_init, and with appropriate params made const.

//long long int tensor_size_by_shape_(long int &, long int *, long int &);
//void get_contraction_ptrn_(long int &, long int &, long int &, long int *, long int *, long int &);
//void tensor_block_init__(const long int &, double *, const long int &, const long int &, const double &, long int &);
//void tensor_block_scale__(long int &, double *, long int &, long int *, double &, long int &);
//double tensor_block_norm2__(long int &, double *, long int &, long int *, long int &);
//void tensor_block_slice__(long int &, long int &, double *, long int *, double *, long int *, long int *, long int &);
//void tensor_block_insert__(long int &, long int &, double *, long int *, double *, long int *, long int *, long int &);
//void tensor_block_add__(long int &, long int &, long int *, double *, double *, double &, long int &);
//void tensor_block_copy__(long int &, long int &, long int *, long int *, double *, double *, long int &);
//void tensor_block_contract__(long int &, long int *, double *, long int &, long int *, double *, long int &, long int *, double *, long int &, long int *, long int &);

//
////These are the prototypes that go with tensor_dil_omp.f90, the "original" with changes for ISO binding
//void tensor_block_init_(double*,int&,int&,double&,int&); //changed to take size instead of segments
//void tensor_block_scale_(double*,int&,int*,double&,int&);
//double tensor_block_norm2_(double*,int&,int*,int&);
////void tensor_block_add_(int&,int*,double*,double*,double&,int&);
//void tensor_block_copy_(int&,int*,int*,double*,double*,int&);
//void tensor_block_contract_(int*, double*,int&,int*, double*,int&,int*, double*,int&,int*, int&);




//PROTOTYPES by REFERENCE:
long long int tensor_size_by_shape_(int&, int*, int&);


/* subroutine get_contraction_ptrn(drank,lrank,rrank,aces_ptrn,my_ptrn,ierr) bind(c,name='get_contraction_ptrn_') !SERIAL
!This subroutine converts an ACESIII contraction pattern specification into my internal contraction pattern specification.
!INPUT:
! - drank - rank of the destination tensor block;
! - lrank - rank of the left tensor operand;
! - rrank - rank of the right tensor operand;
! - aces_ptrn(1:drank+lrank+rrank) - a union of three integer ACESIII arrays which uniquely label the indices (by integers);
!OUTPUT:
! - my_ptrn(1:lrank+rrank) - my format of the contraction pattern specification (supplied to tensor_block_contract__);
! - ierr - error code (0:success).
 */
void get_contraction_ptrn_(int& drank, int& lrank, int& rrank, int* aces_ptrn, int* my_ptrn, int& ierr);


/*    subroutine tensor_block_init_(nthreads,tens,tens_rank,tens_dim_extents,sval,ierr) bind(c,name='tensor_block_init__') !PARALLEL
!This subroutine initializes a tensor block <tens> with some scalar value <sval>.
!INPUT:
! - nthreads - number of threads requested;
! - tens - tensor block;
! - tens_rank - tensor rank;
! - tens_dim_extents(1:tens_rank) - tensor dimension extents;
! - sval - initialization scalar value <val>;
!OUTPUT:
! - tens - filled tensor block;
! - ierr - error code (0: success):
*/
void tensor_block_init__(int&, double*, int&, int*, double&, int&);

/**    subroutine tensor_block_scale_(nthreads,tens,tens_rank,tens_dim_extents,scale_fac,ierr) bind(c,name='tensor_block_scale__') !PARALLEL
!This subroutine multiplies a tensor block <tens> by <scale_fac>.
!INPUT:
! - nthreads - number of threads requested;
! - tens - tensor block;
! - tens_rank - tensor rank;
! - tens_dim_extents(1:tens_rank) - tensor dimension extents;
! - scale_fac - scaling factor;
!OUTPUT:
! - tens - scaled tensor block;
! - ierr - error code (0:success).
*/
void tensor_block_scale__(int&, double*, int&, int*, double&, int&);

double tensor_block_norm2__(int&, double*, int&, int*, int&);

/**  subroutine tensor_block_slice_(nthreads,dim_num,tens,tens_ext,slice,slice_ext,ext_beg,ierr) bind(c,name='tensor_block_slice__') !PARALLEL
!This subroutine extracts a slice from a tensor block.
!INPUT:
! - nthreads - number of threads requested;
! - dim_num - number of tensor dimensions;
! - tens(0:) - tensor block (array);
! - tens_ext(1:dim_num) - dimension extents for <tens>;
! - slice_ext(1:dim_num) - dimension extents for <slice>;
! - ext_beg(1:dim_num) - beginning dimension offsets for <tens> (numeration starts at 0);
!OUTPUT:
! - slice(0:) - slice (array);
! - ierr - error code (0:success).
*/
void tensor_block_slice__(int&, int&, double*, int*, double*, int*, int*, int&);


/**subroutine tensor_block_insert_(nthreads,dim_num,tens,tens_ext,slice,slice_ext,ext_beg,ierr) bind(c,name='tensor_block_insert__') !PARALLEL
!This subroutine inserts a slice into a tensor block.
!INPUT:
! - nthreads - number of threads requested;
! - dim_num - number of tensor dimensions;
! - tens_ext(1:dim_num) - dimension extents for <tens>;
! - slice(0:) - slice (array);
! - slice_ext(1:dim_num) - dimension extents for <slice>;
! - ext_beg(1:dim_num) - beginning dimension offsets for <tens> (numeration starts at 0);
!OUTPUT:
! - tens(0:) - tensor block (array);
! - ierr - error code (0:success).
*/
void tensor_block_insert__(int&, int&, double*, int*, double*, int*, int*, int&);

/*
subroutine tensor_block_add_(nthreads,dim_num,dim_extents,tens0,tens1,scale_fac,ierr) bind(c,name='tensor_block_add__') !PARALLEL
!This subroutine adds tensor block <tens1> to tensor block <tens0>:
!tens0(:)+=tens1(:)*scale_fac
!INPUT:
! - nthreads - number of threads requested;
! - dim_num - number of dimensions;
! - dim_extents(1:dim_num) - dimension extents;
! - tens0, tens1 - tensor blocks;
! - scale_fac - scaling factor;
!OUTPUT:
! - tens0 - modified tensor block;
! - ierr - error code (0:success);
*/
void tensor_block_add__(const int&, const int&, int*, double*, double*, const double&, int&);


/*subroutine tensor_block_copy_(nthreads,dim_num,dim_extents,dim_transp,tens_in,tens_out,ierr) bind(c,name='tensor_block_copy__') !PARALLEL
!Given a dense tensor block, this subroutine makes a copy of it, permuting the indices according to the <dim_transp>.
!The algorithm is expected to be cache-efficient (Author: Dmitry I. Lyakh: quant4me@gmail.com).
!In practice, regardless of the index permutation, it should be only two times slower than the direct copy.
!INPUT:
! - nthreads - number of threads requested;
! - dim_num - number of dimensions (>0);
! - dim_extents(1:dim_num) - dimension extents;
! - dim_transp(0:dim_num) - index permutation (old-to-new);
! - tens_in(0:) - input tensor block;
!OUTPUT:
! - tens_out(0:) - output (possibly transposed) tensor block;
! - ierr - error code (0:success).
!NOTES:
! - The output tensor block is expected to have the same size as the input tensor block.
 */
void tensor_block_copy__(int&, int&, int*, int*, double*, double*, int&);




/*  subroutine tensor_block_contract_(nthreads,contr_ptrn,ltens,ltens_num_dim,ltens_dim_extent, &   !PARALLEL
                                                          rtens,rtens_num_dim,rtens_dim_extent, &
                                                          dtens,dtens_num_dim,dtens_dim_extent,ierr) bind(c,name='tensor_block_contract__')
!This subroutine contracts two arbitrary dense tensor blocks and accumulates the result into another tensor block:
!dtens(:)+=ltens(:)*rtens(:)
!The algorithm is based on cache-efficient tensor transposes (Author: Dmitry I. Lyakh: quant4me@gmail.com)
!Possible cases:
! A) tensor+=tensor*tensor (no traces!): all tensor operands can be transposed;
! B) scalar+=tensor*tensor (no traces!): only the left tensor operand can be transposed;
! C) tensor+=tensor*scalar OR tensor+=scalar*tensor (no traces!): no transpose;
! D) scalar+=scalar*scalar: no transpose.
!INPUT:
! - nthreads - number of threads requested;
! - contr_ptrn(1:left_rank+right_rank) - contraction pattern;
! - ltens/ltens_num_dim/ltens_dim_extent - left tensor block / its rank / and dimension extents;
! - rtens/rtens_num_dim/rtens_dim_extent - right tensor block / its rank / and dimension extents;
! - dtens/dtens_num_dim/dtens_dim_extent - destination tensor block (initialized!) / its rank / and dimension extents;
!OUTPUT:
! - dtens - modified destination tensor block;
! - ierr - error code (0: success);
 */
void tensor_block_contract__(int&, int*, double*, int&, int*, double*, int&, int*, double*, int&, int*, int&);

}


#endif /* TENSOR_OPS_C_PROTOTYPES_H_ */
