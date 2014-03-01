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
      subroutine scf_frag(
     c array_slot_0, rank_0, index_values_0, size_0, extents_0, data_0,
     c array_slot_1, rank_1, index_values_1, size_1, extents_1, data_1,
     c array_slot_2, rank_2, index_values_2, size_2, extents_2, data_2,
     c array_slot_3, rank_3, index_values_3, size_3, extents_3, data_3,
     c array_slot_4, rank_4, index_values_4, size_4, extents_4, data_4,
     c  ierr) BIND(C)
c-------------------------------------------------------------------------- 
      use, intrinsic :: ISO_C_BINDING 
      implicit none

!     argument array
      integer(C_INT), intent(in)::array_slot_0
      integer(C_INT), intent(in)::rank_0
      integer(C_INT), intent(in)::index_values_0(1:rank_0)
      integer(C_INT), intent(in)::size_0
      integer(C_INT), intent(in)::extents_0(1:rank_0)
      real(C_DOUBLE), intent(out)::data_0(1:size_0)

!     should be static array  oed_kin
      integer(C_INT), intent(in)::array_slot_1
      integer(C_INT), intent(in)::rank_1
      integer(C_INT), intent(in)::index_values_1(1:rank_1)
      integer(C_INT), intent(in)::size_1
      integer(C_INT), intent(in)::extents_1(1:rank_1)
      real(C_DOUBLE), intent(in)::data_1(1:size_1)

!     should be static array oed_ovl
      integer(C_INT), intent(in)::array_slot_2
      integer(C_INT), intent(in)::rank_2
      integer(C_INT), intent(in)::index_values_2(1:rank_2)
      integer(C_INT), intent(in)::size_2
      integer(C_INT), intent(in)::extents_2(1:rank_2)
      real(C_DOUBLE), intent(in)::data_2(1:size_2)

!     should be static array  fockrohf_a
      integer(C_INT), intent(in)::array_slot_3
      integer(C_INT), intent(in)::rank_3
      integer(C_INT), intent(in)::index_values_3(1:rank_3)
      integer(C_INT), intent(in)::size_3
      integer(C_INT), intent(in)::extents_3(1:rank_3)
      real(C_DOUBLE), intent(in)::data_3(1:size_3)

!     should be static array fockrohf_b
      integer(C_INT), intent(in)::array_slot_4
      integer(C_INT), intent(in)::rank_4
      integer(C_INT), intent(in)::index_values_4(1:rank_4)
      integer(C_INT), intent(in)::size_4
      integer(C_INT), intent(in)::extents_4(1:rank_4)
      real(C_DOUBLE), intent(in)::data_4(1:size_4)

      integer(C_INT), intent(out)::ierr


c---------------------------------------------------------------------------
c Determine 
c 1. vmax: maximum number of virtuals on atom 
c 2. omax: maximum number of occupieds on atom 
c 3. norbmax: maximum number of AO's on atom 
c---------------------------------------------------------------------------

      write(6,*) ' Computing the SCF on fragment ', data_0 

c        call compute_fragscf(data_0, 
c    *                 dbuf(integral_scr),maxblk, ibuf(ipkgscr),
c    *                 dbuf(icoord),dbuf(ipcoeff),dbuf(ialpha), 
c    *                 ibuf(iccbeg), ibuf(iccend), 
c    *                 1, inda(1), 1,inda(2), 
c    *                 x(nai_array), x(kin_array), x(ovl_array),
c    *                 x(farray), x(fbarray), 
c    *                 x(ca_array), x(cb_array), 
c    *                 x(fa_array), x(fb_array), 
c
c    *                 vmax,omax,norbmax,maxhist,diis_start) 


      return
      end
