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
      subroutine return_pairs(
     * array_1, rank_1, index_values_1, size_1, extents_1, data_1,
     * array_2, rank_2, index_values_2, size_2, extents_2, data_2,
     * array_3, rank_3, index_values_3, size_3, extents_3, data_3,
     * array_4, rank_4, index_values_4, size_4, extents_4, data_4,
     * ierr) BIND(C)
      use, intrinsic :: ISO_C_BINDING
      implicit none 
c--------------------------------------------------------------------------
      include 'sip_interface.f' 

! scalar array_1 --> fragment 1  
      integer(C_INT), intent(in)::array_1
      integer(C_INT), intent(in)::rank_1
      integer(C_INT), intent(in)::index_values_1(1:rank_1)
      integer(C_INT), intent(in)::size_1
      integer(C_INT), intent(in)::extents_1(1:rank_1)
      real(C_DOUBLE), intent(out)::data_1(1:size_1) 

! scalar array_2 --> fragment 2  
      integer(C_INT), intent(in)::array_2
      integer(C_INT), intent(in)::rank_2
      integer(C_INT), intent(in)::index_values_2(1:rank_2)
      integer(C_INT), intent(in)::size_2
      integer(C_INT), intent(in)::extents_2(1:rank_2)
      real(C_DOUBLE), intent(out)::data_2(1:size_2) 

! scalar array_3 --> pair distance threshold  
      integer(C_INT), intent(in)::array_3
      integer(C_INT), intent(in)::rank_3
      integer(C_INT), intent(in)::index_values_3(1:rank_3)
      integer(C_INT), intent(in)::size_3
      integer(C_INT), intent(in)::extents_3(1:rank_3)
      real(C_DOUBLE), intent(out)::data_3(1:size_3) 

! scalar array_4 --> indicator whether to perform pair cal or not   
      integer(C_INT), intent(in)::array_4
      integer(C_INT), intent(in)::rank_4
      integer(C_INT), intent(in)::index_values_4(1:rank_4)
      integer(C_INT), intent(in)::size_4
      integer(C_INT), intent(in)::extents_4(1:rank_4)
      real(C_DOUBLE), intent(out)::data_4(1:size_4) 

      integer(C_INT), intent(out)::ierr
      integer one, two 
      integer ifrag, jfrag 
      double precision dmin, dcrit   
c
c ---------------------------------------------------------------------- 
c predefined data declarations  
c ---------------------------------------------------------------------- 
      integer(C_INT) nfrags 
      real(C_DOUBLE) r_thresh  
!     two dimensional array of atomic coordinates  
      TYPE(C_PTR) :: sDimCoords
      TYPE(C_PTR) :: coords
      integer, pointer :: pdim_coords(:)
      real(C_DOUBLE), pointer :: pcoords(:,:)
c array defining which atoms are in which fragment 
      TYPE(C_PTR) :: sDimfrag
      TYPE(C_PTR) :: pwatom_frag
      integer, pointer :: pdim_frag(:)
      integer(C_INT), pointer :: watom_frag(:,:)
c array defining the number of atoms per fragment 
      TYPE(C_PTR) :: pnatoms_frag
      integer(C_INT), pointer :: natoms_frag(:)
c
c ---------------------------------------------------------------------- 
c Get predefined data 
c ----------------------------------------------------------------------

      nfrags   = int_constant("nfrags"//C_NULL_CHAR) 

c array defining which atoms are in which fragments 
c ----------------------------------------------------------------------
      call predefined_int_array("watom_frag"//C_NULL_CHAR,
     *                                  two, sDimfrag,  pwatom_frag)
      call c_f_pointer(sDimfrag, pdim_frag,[2])
      call c_f_pointer(pwatom_frag, watom_frag, pdim_frag)
c array defining the number of atoms in each fragment 
c ----------------------------------------------------------------------
      call predefined_int_array("natoms_frag"//C_NULL_CHAR,
     *                                  one, sDimfrag,  pnatoms_frag)
      call c_f_pointer(sDimfrag, pdim_frag,[1])
      call c_f_pointer(pnatoms_frag, natoms_frag, pdim_frag)
c ----------------------------------------------------------------------
c  ATOMIC COORDINATES  
      call predefined_scalar_array("coords"//C_NULL_CHAR,
     *                                  two, sDimCoords,  coords)
      call c_f_pointer(sDimCoords, pdim_coords,[2])
      call c_f_pointer(coords, pcoords, pdim_coords) 

      ifrag = data_1(1)  
      jfrag = data_2(1)  
      r_thresh = data_3(1) 
      call return_dmin(ifrag,jfrag,nfrags,natoms_frag,watom_frag,
     *                 pcoords,r_thresh, data_4(1)) 

      return
      end

      subroutine return_dmin(ifrag,jfrag,nfrags,natoms_frag,watom_frag,
     *                       coords, dcrit, pair_flag)
      implicit none 

      integer ifrag, jfrag, nfrags, iatom, jatom, i, j   
      integer natoms_frag(*), watom_frag(nfrags,*) 
      double precision coords(3,*) 
      double precision dist, dmin, dcrit, pair_flag   

      if (ifrag .eq. jfrag) then 
         pair_flag = 2 
         write(6,*) ' IFRAG JFRAG ', ifrag, jfrag, pair_flag  
         return 
      endif 

      dmin = 1000.0 

      do i = 1, natoms_frag(ifrag) 
         iatom = watom_frag(ifrag,i) 
      do j = 1, natoms_frag(jfrag) 
         jatom = watom_frag(jfrag,j) 

         dist = (coords(1,iatom) - coords(1,jatom))**2  
     *        + (coords(2,iatom) - coords(2,jatom))**2  
     *        + (coords(3,iatom) - coords(3,jatom))**2  

         dist = dsqrt(dist) 
         if (dist .lt. dmin) dmin = dist 

      enddo 
      enddo 

c     dcrit = 7.0 

      if (dmin .le. dcrit) pair_flag = 2.0 
      if (dmin .gt. dcrit) pair_flag = 0.0 

c     write(6,*) ' IFRAG JFRAG ', ifrag, jfrag, dcrit, pair_flag  

      return
      end 

