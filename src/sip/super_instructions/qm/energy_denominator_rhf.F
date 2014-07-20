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
      subroutine energy_denominator_rhf(
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

! scalar array --> array_1
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

c Get the mo segment ranges  
      call predefined_int_array("moa_seg_ranges"//C_NULL_CHAR, one, 
     *                           n_ao_segs, aosegs)  
      call c_f_pointer(n_ao_segs,pn_ao_segs,[1]) 
      call c_f_pointer(aosegs,paosegs,pn_ao_segs) 

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

      call do_rhf_den4(1, extents_0(1),offset_1, 
     *                 1, extents_0(2),offset_2, 
     *                 1, extents_0(3),offset_3, 
     *                 1, extents_0(4),offset_4, 
     *                 data_0, 
     *                 1, extents_1(1), 1, extents_1(2),
     *                 data_1) 

      end if  ! rank0 .eq. 4 

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

      call do_rhf_den2(1, extents_0(1),offset_1, 
     *                 1, extents_0(2),offset_2, 
     *                 data_0, 
     *                 1, extents_1(1), 1, extents_1(2),
     *                 data_1) 

      end if  ! rank0 .eq. 2 

      if (rank_0 .eq. 6) then 

c Find the offset index ranges 
      start_1 = 1 
      start_2 = 1 
      start_3 = 1 
      start_4 = 1 
      start_5 = 1 
      start_6 = 1 

      offset_1 = 0 
      offset_2 = 0 
      offset_3 = 0 
      offset_4 = 0 
      offset_5 = 0 
      offset_6 = 0 

      if (extents_0(1) .eq. 1) then 
         offset_1 = index_values_0(1) - 1  
      else
          if (index_values_0(1).gt.1) then  
          do i = 1, index_values_0(1) - 1 
             offset_1 = offset_1 + paosegs(i) 
          enddo 
          endif 
      endif 

      if (extents_0(2) .eq. 1) then 
         offset_2 = index_values_0(2) - 1  
      else 
          if (index_values_0(2).gt.1) then 
          do i = 1, index_values_0(2) - 1 
             offset_2 = offset_2 + paosegs(i) 
          enddo 
      endif 
      endif 
      if (extents_0(3) .eq. 1) then 
         offset_3 = index_values_0(3) - 1  
      else
          if (index_values_0(3).gt.1) then 
          do i = 1, index_values_0(3) - 1 
             offset_3 = offset_3 + paosegs(i) 
          enddo 
      endif 
      endif 
      if (extents_0(4) .eq. 1) then 
         offset_4 = index_values_0(4) - 1  
      else
          if (index_values_0(4).gt.1) then 
          do i = 1, index_values_0(4) - 1 
             offset_4 = offset_4 + paosegs(i) 
          enddo 
      endif 
      endif 
      if (extents_0(5) .eq. 1) then 
         offset_5 = index_values_0(5) - 1  
      else
          if (index_values_0(5).gt.1) then 
          do i = 1, index_values_0(5) - 1 
             offset_5 = offset_5 + paosegs(i) 
          enddo 
      endif 
      endif 
      if (extents_0(6) .eq. 1) then 
         offset_6 = index_values_0(6) - 1  
      else
          if (index_values_0(6).gt.1) then 
          do i = 1, index_values_0(6) - 1 
             offset_6 = offset_6 + paosegs(i) 
          enddo 
      endif 
      endif 

c Check for simple ineces 


      call do_rhf_den6(start_1, extents_0(1),offset_1, 
     *                 start_2, extents_0(2),offset_2, 
     *                 start_3, extents_0(3),offset_3, 
     *                 start_4, extents_0(4),offset_4, 
     *                 start_5, extents_0(5),offset_5, 
     *                 start_6, extents_0(6),offset_6, 
     *                 data_0, 
     *                 1, extents_1(1), 1, extents_1(2),
     *                 data_1) 

      end if  ! rank0 .eq. 6 


      return
      end

      subroutine do_rhf_den6(a1, a2, off_1, b1, b2, off_2, 
     *                       c1, c2, off_3, d1, d2, off_4, 
     *                       e1, e2, off_5, f1, f2, off_6, 
     *                       array, 
     *                       m1, m2, n1, n2, fock)  
    
c---------------------------------------------------------------------------
c
c
c---------------------------------------------------------------------------

      implicit none
      integer a, a1, a2, b, b1, b2, c, c1, c2, d, d1, d2, m1, m2, n1, n2
      integer e, e1, e2, f, f1, f2 
      integer off_1, off_2, off_3, off_4, off_5, off_6  
      double precision array(a1:a2,b1:b2,c1:c2,d1:d2,e1:e2,f1:f2) 
      double precision fock(m1:m2,n1:n2) 
      double precision val, eps, epsa, epsb, epsc, epsd, epse, epsf    

      val = 0.0 
      do f = f1, f2  
         epsf = fock(f+off_6,f+off_6) 
      do e = e1, e2  
         epse = fock(e+off_5,e+off_5) 
      do d = d1, d2  
         epsd = fock(d+off_4,d+off_4) 
      do c = c1, c2  
         epsc = fock(c+off_3,c+off_3) 
      do b = b1, b2  
         epsb = fock(b+off_2,b+off_2) 
      do a = a1, a2  
         epsa = fock(a+off_1,a+off_1) 

         eps = epsb + epsd + epsf - epsa - epsc - epse  

c        val = array(a,b,c,d) 
         array(a,b,c,d,e,f) = array(a,b,c,d,e,f)/eps  
c         write(6,*) ' EPS:', a, b, c, d, eps, array(a,b,c,d) 
      enddo 
      enddo 
      enddo 
      enddo 
      enddo 
      enddo 
10    format(4I3,3F10.8) 

      return
      end 

