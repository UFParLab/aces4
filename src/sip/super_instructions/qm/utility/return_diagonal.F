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
     * array_1, rank_1, index_values_1, size_1, extents_1, data_1,
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

! output scalar --> array_1
      integer(C_INT), intent(in)::array_1
      integer(C_INT), intent(in)::rank_1
      integer(C_INT), intent(in)::index_values_1(1:rank_1)
      integer(C_INT), intent(in)::size_1
      integer(C_INT), intent(in)::extents_1(1:rank_1)
      real(C_DOUBLE), intent(out)::data_1(1:size_1)

      integer(C_INT), intent(out)::ierr
c mosegments 
      TYPE(C_PTR) :: n_ao_segs, aosegs
      integer, pointer :: pn_ao_segs(:), paosegs(:) 
      integer(C_INT) offset_1, offset_2, offset_3, offset_4 
      integer(C_INT) offset_5, offset_6 
      integer(C_INT) start_1, start_2, start_3, start_4, start_5,start_6 
      integer(C_INT) i, i1, i2, i3, i4, one  

      double precision diag_sum 

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
     *              data_0, diag_sum) 

      end if  ! rank0 .eq. 2 

      if (rank_0 .eq. 4) then 

c Find the offset index ranges 
      offset_1 = 0 
      offset_2 = 0 
      offset_3 = 0 
      offset_4 = 0 
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
      if (index_values_0(3).gt.1) then 
          do i = 1, index_values_0(3) - 1 
             offset_3 = offset_3 + paosegs(i) 
          enddo 
      endif 
      if (index_values_0(4).gt.1) then 
          do i = 1, index_values_0(4) - 1 
             offset_4 = offset_4 + paosegs(i) 
          enddo 
      endif 

      call do_diag4(1, extents_0(1), offset_1,  
     *              1, extents_0(2), offset_2,  
     *              1, extents_0(3), offset_3,  
     *              1, extents_0(4), offset_4,  
     *              data_0, diag_sum) 

      end if  ! rank0 .eq. 2 

      data_1(1) = diag_sum  

      return 
      end 

      subroutine do_diag2(a1, a2, offset_1, b1, b2, offset_2, array,
     *                    diag_sum) 
      
c---------------------------------------------------------------------------
c
c
c---------------------------------------------------------------------------

      implicit none
      integer a, a1, a2, b, b1, b2
      integer m1, m2, n1, n2 
      integer offset_1, offset_2 
      double precision array(a1:a2,b1:b2), diag_sum  

      diag_sum = 0.0 
      do b = b1, b2 
      do a = a1, a2 
         if (a+offset_1 .ne. b+offset_2) then 
             array(a,b) = 0.0  
         endif 
         if (a+offset_1 .eq. b+offset_2) then 
             diag_sum = diag_sum + array(a,b) 
c            write(6,*) ' Diag elements:', a+offset_1,b+offset_2,
c    *                    array(a,b), diag_sum  
         endif 
      enddo 
      enddo 

      return
      end


      subroutine do_diag4(a1, a2, offset_1, b1, b2, offset_2, 
     *                    c1, c2, offset_3, d1, d2, offset_4, array, 
     *                    diag_sum) 
      
c---------------------------------------------------------------------------
c
c
c---------------------------------------------------------------------------

      implicit none
      integer a, a1, a2, b, b1, b2, c, c1, c2, d, d1, d2
      integer m1, m2, n1, n2 
      integer offset_1, offset_2, offset_3, offset_4  
      double precision array(a1:a2,b1:b2,c1:c2,d1:d2), diag_sum  

      diag_sum = 0.0 
      do d = d1, d2 
      do c = c1, c2 
      do b = b1, b2 
      do a = a1, a2 
         if ((a+offset_1 .ne. b+offset_2) .and. 
     *       (c+offset_3 .ne. d+offset_4))  then 
             array(a,b,c,d) = 0.0  
         endif 
         if ((a+offset_1 .eq. b+offset_2) .and. 
     *       (c+offset_3 .eq. d+offset_4))  then 
             diag_sum = diag_sum + array(a,b,c,d) 
         endif 
      enddo 
      enddo 
      enddo 
      enddo 

      return
      end

