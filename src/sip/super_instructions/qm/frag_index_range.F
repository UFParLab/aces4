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
      subroutine frag_index_range(
     * array_0, rank_0, index_values_0, size_0, extents_0, data_0,
     * array_1, rank_1, index_values_1, size_1, extents_1, data_1,
     * ierr) BIND(C)
      use, intrinsic :: ISO_C_BINDING
      implicit none

      include 'sip_interface.f'
c--------------------------------------------------------------------------
c Input: array_0=which fragment
c        array_1=type
c           1=aoindex 
c           2=moindex 
c           3=moindex occupied  
c           4=moindex virtual   
c           5=moaindex 
c           6=moaindex occupied  
c           7=moaindex virtual   
c           8=mobindex 
c           9=mobindex occupied  
c           10=mobindex virtual   
c Output: Beginning segment, Ending segment 
c--------------------------------------------------------------------------

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

! AO fragment mapping 
      TYPE(C_PTR) :: n_ao_segs, aosegs 
      integer, pointer :: pn_ao_segs(:), w_frag_AO(:) 
      integer, pointer :: w_frag_aocc(:) 
      integer, pointer :: w_frag_bocc(:) 
      integer, pointer :: w_frag_avirt(:) 
      integer, pointer :: w_frag_bvirt(:) 

      integer(C_INT), intent(out)::ierr
      integer(C_INT) one, nfrags, i, istart, iend    
      real(C_DOUBLE) x2, wfrag  

      wfrag = data_0(1)   
      x2    = data_1(1) 

      if (x2 .eq. 1.0) then ! get AO fragment ranges 
         call predefined_int_array("w_frag_ao"//C_NULL_CHAR, one,
     *                           n_ao_segs, aosegs)
         call c_f_pointer(n_ao_segs,pn_ao_segs,[1])
         call c_f_pointer(aosegs,w_frag_AO,pn_ao_segs)
      endif 

      if (x2 .eq. 6.0) then ! get alpha occ fragment ranges 
         call predefined_int_array("w_frag_aocc"//C_NULL_CHAR, one,
     *                           n_ao_segs, aosegs)
         call c_f_pointer(n_ao_segs,pn_ao_segs,[1])
         call c_f_pointer(aosegs,w_frag_aocc,pn_ao_segs)
      endif 

      if (x2 .eq. 9.0) then ! get beta occ fragment ranges 
         call predefined_int_array("w_frag_bocc"//C_NULL_CHAR, one,
     *                           n_ao_segs, aosegs)
         call c_f_pointer(n_ao_segs,pn_ao_segs,[1])
         call c_f_pointer(aosegs,w_frag_bocc,pn_ao_segs)
      endif 

      if (x2 .eq. 7.0) then ! get alpha virt fragment ranges 
         call predefined_int_array("w_frag_avirt"//C_NULL_CHAR, one,
     *                           n_ao_segs, aosegs)
         call c_f_pointer(n_ao_segs,pn_ao_segs,[1])
         call c_f_pointer(aosegs,w_frag_avirt,pn_ao_segs)
      endif 

      if (x2 .eq. 10.0) then ! get beta virt fragment ranges 
         call predefined_int_array("w_frag_bvirt"//C_NULL_CHAR, one,
     *                           n_ao_segs, aosegs)
         call c_f_pointer(n_ao_segs,pn_ao_segs,[1])
         call c_f_pointer(aosegs,w_frag_bvirt,pn_ao_segs)
      endif 

      nfrags = int_constant("nfrags"//C_NULL_CHAR)  
      write(6,*) 'VFL Nfrags = ', nfrags, wfrag, x2 
      return 

      if (x2 .eq. 1.0) then 
         istart = 0 
         iend   = 0 
         if (wfrag .eq. 1) then 
            istart = 1 
            go to 10 
         endif 
         do i = 2, nfrags  
            if ((w_frag_AO(i) .eq. wfrag) .and.
     *          (w_frag_AO(i-1) .ne. wfrag)) then 
                istart = i
                go to 10 
            endif 
         enddo 
10       continue 
         do i = istart, nfrags  
            if ((w_frag_AO(i-1) .eq. wfrag) .and.
     *          (w_frag_AO(i) .ne. wfrag)) then 
                iend = i-1 
                go to 20 
            endif 
         enddo 
20       continue 
      endif 

      if (x2 .eq. 6.0) then 
         istart = 0 
         iend   = 0 
         if (wfrag .eq. 1) then 
            istart = 1 
            go to 11 
         endif 
         do i = 2, nfrags  
            if ((w_frag_aocc(i) .eq. wfrag) .and.
     *          (w_frag_aocc(i-1) .ne. wfrag)) then 
                istart = i
                go to 11 
            endif 
         enddo 
11       continue 
         do i = istart, nfrags  
            if ((w_frag_aocc(i-1) .eq. wfrag) .and.
     *          (w_frag_aocc(i) .ne. wfrag)) then 
                iend = i-1  
                go to 21 
            endif 
         enddo 
21       continue 
      endif 

      if (x2 .eq. 9.0) then 
         istart = 0 
         iend   = 0 
         if (wfrag .eq. 1) then 
            istart = 1 
            go to 12 
         endif 
         do i = 2, nfrags  
            if ((w_frag_bocc(i) .eq. wfrag) .and.
     *          (w_frag_bocc(i-1) .ne. wfrag)) then 
                istart = i
                go to 12 
            endif 
         enddo 
12       continue 
         do i = istart, nfrags  
            if ((w_frag_bocc(i-1) .eq. wfrag) .and.
     *          (w_frag_bocc(i) .ne. wfrag)) then 
                iend = i-1  
                go to 22 
            endif 
         enddo 
22       continue 
      endif 

      if (x2 .eq. 7.0) then 
         istart = 0 
         iend   = 0 
         if (wfrag .eq. 1) then 
            istart = 1 
            go to 13 
         endif 
         do i = 2, nfrags  
            if ((w_frag_avirt(i) .eq. wfrag) .and.
     *          (w_frag_avirt(i-1) .ne. wfrag)) then 
                istart = i
                go to 13 
            endif 
         enddo 
13       continue 
         do i = istart, nfrags  
            if ((w_frag_avirt(i-1) .eq. wfrag) .and.
     *          (w_frag_avirt(i) .ne. wfrag)) then 
                iend = i-1  
                go to 23 
            endif 
         enddo 
23       continue 
      endif 

      if (x2 .eq. 10.0) then 
         istart = 0 
         iend   = 0 
         if (wfrag .eq. 1) then 
            istart = 1 
            go to 14 
         endif 
         do i = 2, nfrags  
            if ((w_frag_bvirt(i) .eq. wfrag) .and.
     *          (w_frag_bvirt(i-1) .ne. wfrag)) then 
                istart = i
                go to 14 
            endif 
         enddo 
14       continue 
         do i = istart, nfrags  
            if ((w_frag_bvirt(i-1) .eq. wfrag) .and.
     *          (w_frag_bvirt(i) .ne. wfrag)) then 
                iend = i-1  
                go to 24 
            endif 
         enddo 
24       continue 
      endif 

c     x1 = istart 
c     x2 = iend 

      data_0(1) = istart 
      data_1(1) = iend 

c     write(6,*) ' AOSTART AOEND IN FRAG:', x1, x2 

      return
      end

