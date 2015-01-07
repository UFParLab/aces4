C  Copyright (c) 2003-2010 University of Florida
C
C  This program is free software; you can redistribute it and/or modify
C  it under the terms of the GNU General Public License as published by
C  the Free Software Foundation; either version 2 of the License, or
C  (at your option) any later version.

C  This program is distributed in the hope that it will be useful,
C  but WITHOUT ANY WARRANTY; without even the implied warranty of
C  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
C  GNU General Public License for more details.

C  The GNU General Public License is included in this distribution
C  in the file COPYRIGHT.
!      subroutine eig_sr_inv(x, nindex, type, bval,
!     *                              eval, bdim, edim,
!     *                      x2, nindex2, type2, bval2,
!     *                              eval2, bdim2, edim2)
c--------------------------------------------------------------------------
!> special eig_sr_inv uu
!>   Calculates the eigenvalues and eigenvectors of a 2-d square matrix.
!>   The matrix is also diagonalized on output and inverted. 
!>
!>   The eigenvalues are stored in a common block.  
!>   The instruction is executed as follows:
!>   
!>   execute eig Array1 Array2
!>
!>   where Array1 is the input matrix that is to be diagonalized.
!>         Array2 is the matrix whose columns are the eigenvectors of Array1.
!>
!>   Both Array1 and Array2 must be "static" arrays.
c--------------------------------------------------------------------------
!
!      implicit none
!C#include "interpreter.h"
!C      include 'trace.h'
!
!      double precision x(*)
!      integer nindex, type(*), bval(*), eval(*)
!      integer bdim(*), edim(*)
!      double precision x2(*)
!      integer nindex2, type2(*), bval2(*), eval2(*)
!      integer bdim2(*), edim2(*)

      subroutine eig_sr_inv(
     c array_slot_0, rank_0, index_values_0, size_0, extents_0, data_0,
     c array_slot_1, rank_1, index_values_1, size_1, extents_1, data_1,
     c  ierr) BIND(C)
      use, intrinsic :: ISO_C_BINDING
      implicit none
      integer(C_INT), intent(in)::array_slot_0
      integer(C_INT), intent(in)::rank_0
      integer(C_INT), intent(in)::index_values_0(1:rank_0)
      integer(C_INT), intent(in)::size_0
      integer(C_INT), intent(in)::extents_0(1:rank_0)
      real(C_DOUBLE), intent(out)::data_0(1:size_0)

      integer(C_INT), intent(in)::array_slot_1
      integer(C_INT), intent(in)::rank_1
      integer(C_INT), intent(in)::index_values_1(1:rank_1)
      integer(C_INT), intent(in)::size_1
      integer(C_INT), intent(in)::extents_1(1:rank_1)
      real(C_DOUBLE), intent(out)::data_1(1:size_1)

      integer(C_INT), intent(out)::ierr
      integer i, j, n, current_line, junk

      ierr = 0
      if (rank_0 .ne. 2) then
         ierr = ierr + 1
         print *,'Error in eig_sr_inv, line ',current_line,
     *           ' Number of array',
     *           ' indices must be 2, actual number is ',rank_0
      endif

      if (rank_1 .ne. 2) then
         ierr = ierr + 1
         print *,'Error in eig_sr_inv, line ',current_line,
     *           ' Number of array',
     *           ' indices must be 2, actual number is ',rank_0
      endif

      if (ierr .ne. 0) call abort_job()

      n = extents_0(1)

      call diag(data_0, data_1, junk, n, 1, 1, 1)

c     n = 0 
c     do i = 1, extents_0(1) 
c     do j = 1, extents_0(1) 
c        n = n + 1 
c        write(6,*) ' SR_INV:', j,i, data_1(n) 
c     enddo 
c     enddo 

      return
      end
