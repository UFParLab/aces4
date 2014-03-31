C Copyright (c) 2003-2010 University of Florida
C
C This program is free software; you can redistribute it and/or modify
C it under the terms of the GNU General Public License as published by
C the Free Software Foundation; either version 2 of the License, or
C (at your option) any later version.

C This program is distributed in the hope that it will be useful,
C but WITHOUT ANY WARRANTY; without even the implied warranty of
C MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
C GNU General Public License for more details.

C The GNU General Public License is included in this distribution
C in the file COPYRIGHT.
      subroutine return_diagonal(
     * array_0, rank_0, index_values_0, size_0, extents_0, data_0,
     * ierr) BIND(C)
      use, intrinsic :: ISO_C_BINDING
      implicit none

c--------------------------------------------------------------------------
c Usage: execute check_dconf array scalar
c
c--------------------------------------------------------------------------
      include 'sip_interface.f'

! output array --> array_0
      integer(C_INT), intent(in)::array_0
      integer(C_INT), intent(in)::rank_0
      integer(C_INT), intent(in)::index_values_0(1:rank_0)
      integer(C_INT), intent(in)::size_0
      integer(C_INT), intent(in)::extents_0(1:rank_0)
      real(C_DOUBLE), intent(out)::data_0(1:size_0)

      integer(C_INT), intent(out)::ierr
c mosegments 
      TYPE(C_PTR) :: n_ao_segs, aosegs
      integer, pointer :: pn_ao_segs(:), paosegs(:) 
      integer(C_INT) offset_1, offset_2, offset_3, offset_4 
      integer(C_INT) offset_5, offset_6 
      integer(C_INT) start_1, start_2, start_3, start_4, start_5,start_6 
      integer(C_INT) i, i1, i2, i3, i4, one  

c Get the mo segment ranges  
      call predefined_int_array("moa_seg_ranges"//C_NULL_CHAR, one, 
     *                           n_ao_segs, aosegs)  
      call c_f_pointer(n_ao_segs,pn_ao_segs,[1]) 
      call c_f_pointer(aosegs,paosegs,pn_ao_segs) 

      if (rank_0 .eq. 2) then 

c Find the offset index ranges 
      offset_1 = 0 
      offset_2 = 0 
      if (index_values_0(1).gt.1) then 
          do i = 1, index_values_0(1) - 1 
             offset_1 = offset_1 + paosegs(i) 
          enddo 
      endif 
      if (index_values_0(2).gt.1) then 
          do i = 1, index_values_0(2) - 1 
             offset_2 = offset_2 + paosegs(i) 
          enddo 
      endif 

      call do_diag2(1, extents_0(1), offset_1,  
     *              1, extents_0(2), offset_2,  
     *              data_0) 

      end if  ! rank0 .eq. 2 

      return 
      end 

      subroutine do_diag2(a1, a2, offset_1, b1, b2, offset_2, array) 
      
c---------------------------------------------------------------------------
c
c
c---------------------------------------------------------------------------

      implicit none
      integer a, a1, a2, b, b1, b2
      integer m1, m2, n1, n2 
      integer offset_1, offset_2 
      double precision array(a1:a2,b1:b2) 

      do b = b1, b2 
      do a = a1, a2 
         if (a+offset_1 .ne. b+offset_2) array(a,b) = 0.0  
      enddo 
      enddo 

      return
      end
