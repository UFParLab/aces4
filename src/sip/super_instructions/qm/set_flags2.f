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

!C  The GNU General Public License is included in this distribution
!C  in the file COPYRIGHT.
!      subroutine set_flags2(array_table, narray_table,
!     *                      index_table,
!     *                      nindex_table, segment_table, nsegment_table,
!     *                      block_map_table, nblock_map_table,
!     *                      scalar_table, nscalar_table,
!     *                      address_table, op)
!c---------------------------------------------------------------------------
!c   Sets the indices of a 3-d static array in common block values.
!c   The first index is assumed to be the atom, the second is the component
!c   index (i. e. x,y, or z), and the 3rd is the center.
!c
!c   These indices are stored in the d2int_com common block, and are meant
!c   to indicate the atom, component, and center on which to calculate a
!c   single block of derivative integrals.
!c
!c   Example:
!c      index jatom = 1, natoms
!c      index jx = 1,3
!c      static flags2(jatom, jx)
!c
!c      taoint(mu,nu,lambda, sigma) = 0.
!c      do jatom
!c      do jx
!c         execute set_flags2 flags2(jatom, jx)
!c         execute d2int aoint(mu, nu, lambda, sigma)
!c         taoint(mu,nu,lambda, sigma) += aoint(mu, nu, lambda, sigma)
!c      enddo jx
!c      enddo jatom
!c
!c----------------------------------------------------------------------------
!      implicit none
!      include 'interpreter.h'
!      include 'mpif.h'
!      include 'trace.h'
!      include 'parallel_info.h'
!
!      common /d2int_com/jatom, jx, jcenter
!      integer jatom, jx, jcenter
!      double precision flags_value
!
!      integer narray_table, nindex_table, nsegment_table,
!     *        nblock_map_table
!      integer op(loptable_entry)
!      integer array_table(larray_table_entry, narray_table)
!      integer index_table(lindex_table_entry, nindex_table)
!      integer segment_table(lsegment_table_entry, nsegment_table)
!      integer block_map_table(lblock_map_entry, nblock_map_table)
!      integer nscalar_table
!      double precision scalar_table(nscalar_table)
!      integer*8 address_table(narray_table)
!
!      integer ierr, array, array_type, ind, nind
!      integer i
!
!      array = op(c_result_array)
!      if (array .lt. 1 .or. array .gt. narray_table) then
!         print *,'Error: Invalid array in set_flags, line ',
!     *     current_line
!         print *,'Array index is ',array,' Allowable values are ',
!     *      ' 1 through ',narray_table
!         call abort_job()
!      endif
!
!      nind = array_table(c_nindex, array)
!      if (nind .ne. 2) then
!         print *,'Error: set_flags2 requires a 2-index array.'
!         call abort_job()
!      endif
!
!c-----------------------------------------------------------------------
!c   Atom, component, and center indices are determined from the c_current_seg
!c   field of the 1st and 2nd index of the array.
!c------------------------------------------------------------------------
!
!      ind   = array_table(c_index_array1,array)
!      jatom = index_table(c_current_seg, ind)
!      ind   = array_table(c_index_array1+1,array)
!      jx    = index_table(c_current_seg, ind)
!
!c-----------------------------------------------------------------------
!c   Set jcenter to 0 as it is not currently being used.
!c-----------------------------------------------------------------------
!
!      jcenter = 0
!
!      return
!      end
      subroutine set_flags2(
     c array_slot, rank, index_values, size, extents, data,
     c  ierr) BIND(C)
      use, intrinsic :: ISO_C_BINDING
      implicit none
      integer(C_INT), intent(in)::array_slot
      integer(C_INT), intent(in)::rank
      integer(C_INT), intent(in)::index_values(1:rank)
      integer(C_INT), intent(in)::size
      integer(C_INT), intent(in)::extents(1:rank)
      real(C_DOUBLE), intent(out)::data(1:size)
      integer(C_INT), intent(out)::ierr

      common /d2int_com/jatom, jx, jcenter
      integer jatom, jx, jcenter
      double precision flags_value

      jatom = index_values(1)
      jx = index_values(2)
      jcenter = 0

      return
      end
