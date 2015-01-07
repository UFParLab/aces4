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
      subroutine place_scratch(
     * array_0, rank_0, index_values_0, size_0, extents_0, data_0,
     * array_1, rank_1, index_values_1, size_1, extents_1, data_1,
     * array_2, rank_2, index_values_2, size_2, extents_2, data_2,
     * array_3, rank_3, index_values_3, size_3, extents_3, data_3,
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

! static input array --> array_1
      integer(C_INT), intent(in)::array_1
      integer(C_INT), intent(in)::rank_1
      integer(C_INT), intent(in)::index_values_1(1:rank_1)
      integer(C_INT), intent(in)::size_1
      integer(C_INT), intent(in)::extents_1(1:rank_1)
      real(C_DOUBLE), intent(out)::data_1(1:size_1)

! scalar defining index types  
      integer(C_INT), intent(in)::array_2
      integer(C_INT), intent(in)::rank_2
      integer(C_INT), intent(in)::index_values_2(1:rank_2)
      integer(C_INT), intent(in)::size_2
      integer(C_INT), intent(in)::extents_2(1:rank_2)
      real(C_DOUBLE), intent(out)::data_2(1:size_2)

! scalar defining which fragment  
      integer(C_INT), intent(in)::array_3
      integer(C_INT), intent(in)::rank_3
      integer(C_INT), intent(in)::index_values_3(1:rank_3)
      integer(C_INT), intent(in)::size_3
      integer(C_INT), intent(in)::extents_3(1:rank_3)
      real(C_DOUBLE), intent(out)::data_3(1:size_3)

      integer(C_INT), intent(out)::ierr
c mosegments 
      TYPE(C_PTR) :: n_moa_segs, moasegs
      integer, pointer :: pn_moa_segs(:), pmoasegs(:) 
c aosegments 
      TYPE(C_PTR) :: n_ao_segs, aosegs
      integer, pointer :: pn_ao_segs(:), paosegs(:) 
c n_aocc
      TYPE(C_PTR) :: n_aocc, aocc
      integer, pointer :: pn_aocc(:), paocc(:)
c n_avirt
      TYPE(C_PTR) :: n_avirt, avirt
      integer, pointer :: pn_avirt(:), pavirt(:)
C map segments to fragments
      TYPE(C_PTR) :: n_frag_ao, frag_ao
      integer, pointer :: pn_frag_ao(:),  pfrag_ao(:)

      TYPE(C_PTR) :: n_frag_aocc, frag_aocc
      integer, pointer :: pn_frag_aocc(:), pfrag_aocc(:)

      TYPE(C_PTR) :: n_frag_avirt, frag_avirt
      integer, pointer :: pn_frag_avirt(:), pfrag_avirt(:)

c array defining which atoms are in which fragment 
      TYPE(C_PTR) :: sDimfrag
      TYPE(C_PTR) :: pwatom_frag
      integer, pointer :: pdim_frag(:)
      integer(C_INT), pointer :: watom_frag(:,:)
c array defining the number of atoms per fragment 
      TYPE(C_PTR) :: pnatoms_frag
      integer(C_INT), pointer :: natoms_frag(:)
c  CHARGES    
      TYPE(C_PTR) :: charge, atoms
      integer, pointer :: patoms(:)
      real(C_DOUBLE), pointer :: pcharge(:)

      integer(C_INT) offset_1, offset_2, offset_3, offset_4 
      integer(C_INT) offset_5, offset_6 
      integer(C_INT) start_1, start_2, start_3, start_4, start_5,start_6 
      integer(C_INT) i, i1, i2, i3, i4, m, n, ncenters, wfrag, n_apf
      integer(C_INT) one, two 
      integer(C_INT) nocc_a, nocc_b, nvirt_a, nvirt_b, tcharge, n_basis

      integer(C_INT) ishift

      integer iatom(1000) 

c Get the mo segment ranges  
      call predefined_int_array("moa_seg_ranges"//C_NULL_CHAR, one, 
     *                           n_moa_segs, moasegs)  
      call c_f_pointer(n_moa_segs,pn_moa_segs,[1]) 
      call c_f_pointer(moasegs,pmoasegs,pn_moa_segs) 


c Get the ao segment ranges  
      call predefined_int_array("ao_seg_ranges"//C_NULL_CHAR, one,
     *                           n_ao_segs, aosegs)
      call c_f_pointer(n_ao_segs,pn_ao_segs,[1])
      call c_f_pointer(aosegs,paosegs,pn_ao_segs) 

c get number of occupied fragment segments
      call predefined_int_array("n_aocc"//C_NULL_CHAR,one,
     *                           n_aocc,aocc)
      call c_f_pointer(n_aocc,pn_aocc,[1])
      call c_f_pointer(aocc,paocc,pn_aocc)

c get number of virtual fragment segments
      call predefined_int_array("n_avirt"//C_NULL_CHAR,one,
     *                           n_avirt,avirt)
      call c_f_pointer(n_avirt,pn_avirt,[1])
      call c_f_pointer(avirt,pavirt,pn_avirt)
C map segments to fragments
      call predefined_int_array("w_frag_aocc"//C_NULL_CHAR,one,
     *                      n_frag_aocc, frag_aocc)
      call c_f_pointer(n_frag_aocc,pn_frag_aocc,[1])
      call c_f_pointer(frag_aocc,pfrag_aocc,pn_frag_aocc)

      call predefined_int_array("w_frag_avirt"//C_NULL_CHAR,one,
     *                      n_frag_avirt, frag_avirt)
      call c_f_pointer(n_frag_avirt,pn_frag_avirt,[1])
      call c_f_pointer(frag_avirt,pfrag_avirt,pn_frag_avirt)

      call predefined_int_array("w_frag_ao"//C_NULL_CHAR,one,
     *                      n_frag_ao, frag_ao)
      call c_f_pointer(n_frag_ao,pn_frag_ao,[1])
      call c_f_pointer(frag_ao,pfrag_ao,pn_frag_ao)

      wfrag = data_3(1) ! Clean up  

      print *, "FRAG index values",
     *         wfrag,index_values_0(1),index_values_0(2)

      if (rank_0 .eq. 2) then 
         offset_1 = 0 
         offset_2 = 0 

         ishift = 0

         if (data_2(1) .eq. 1.0) then 
             if (index_values_0(1).gt.1) then
             do i = 1, index_values_0(1) - 1
                 if (pfrag_ao(i) .eq. wfrag) exit
                 offset_1 = -paosegs(i)
             enddo
             offset_1 = offset_1 + paosegs(index_values_0(1)-1)
             endif
C             do i = 1, index_values_0(1) - 1
C                 if (pfrag_ao(i) .eq. wfrag) then
C                     offset_1 = offset_1 + paosegs(i)
C                 endif
C             enddo
C            if (index_values_0(1).gt.1) then
C               offset_1 = paosegs(index_values_0(1)-1)
C            endif
         endif 

         if (data_2(1) .eq. 2.0) then 
            do i = 1, index_values_0(1) - 1
                if (i .le. pn_aocc(1)) then
                    if (pfrag_aocc(i) .eq. wfrag) then
                        offset_1  = offset_1 + pmoasegs(i)
                    endif
                endif
                if (i .gt. pn_aocc(1)) then
                    if (pfrag_avirt(i-pn_aocc(1)) .eq. wfrag) then
                        offset_1  = offset_1 + pmoasegs(i)
                    endif
                endif
            enddo
         endif 

         do i = 1, index_values_0(2) - 1
             if (i .le. pn_aocc(1)) then
                 if (pfrag_aocc(i) .eq. wfrag) then
                     offset_2  = offset_2 + pmoasegs(i)
                 endif
             endif
             if (i .gt. pn_aocc(1)) then
                 if (pfrag_avirt(i-pn_aocc(1)) .eq. wfrag) then
                     offset_2  = offset_2 + pmoasegs(i)
                 endif
             endif
         enddo

         write(6,*) 'SCRATCH TYPE = ', data_2(1), offset_1, offset_2 

      call place_scratch_2D(1, extents_0(1),offset_1, 
     *                      1, extents_0(2),offset_2, 
     *                      data_0, 
     *                      1, extents_1(1), 1, extents_1(2),
     *                      data_1) 

      endif  ! rank0 .eq. 2 

      return
      end


      subroutine place_scratch_2D(a1, a2, off_1, b1, b2, off_2, 
     *                      array, 
     *                      m1, m2, n1, n2, 
     *                      scratch) 
      implicit none 
      integer a, a1, a2, b, b1, b2, off_1, off_2, m1, m2, n1, n2 
      double precision array(a1:a2,b1:b2) 
      double precision scratch(m1:m2,n1:n2) 

      do b = b1, b2 
      do a = a1, a2 
         array(a,b) = scratch(a+off_1,b+off_2) 
      enddo 
      enddo 

      return 
      end 
