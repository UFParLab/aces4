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
      subroutine compute_aabb_batch(
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

c     include 'int_gen_parms.hf'

      integer*8 integral_scr
      integer maxblk , nsend

      ierr = 0
c---------------------------------------------------------------------------
c   Compute the block of integrals.
c---------------------------------------------------------------------------

!      call compute_aabb_integrals(bval(1),eval(1), bval(2),eval(2),
!     *                       bval(3),eval(3),bval(4),eval(4),
!     *                 dbuf(integral_scr),maxblk, ibuf(ipkgscr),
!     *                 dbuf(icoord),dbuf(ipcoeff),dbuf(ialpha),
!     *                 ibuf(iccbeg), ibuf(iccend),
!     *                 x, nsend)

c     call compute_aabb_integrals(1, extents(1),
c    *    1, extents(2),
c    *    1, extents(3),
c    *    1, extents(4),
c    *  dbuf(integral_scr),maxblk, ibuf(ipkgscr),
c    *  dbuf(icoord),dbuf(ipcoeff),dbuf(ialpha),
c    *  ibuf(iccbeg), ibuf(iccend),
c    *                 data, nsend)


      return
      end
