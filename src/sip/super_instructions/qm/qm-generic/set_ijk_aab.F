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
      subroutine set_ijk_aab(
     * array_slot_0, rank_0, index_values_0, size_0, extents_0, data_0, 
     * ierr) BIND(C) 
       use, intrinsic :: ISO_C_BINDING
      implicit none
      include 'sip_interface.f'  
c---------------------------------------------------------------------------
c----------------------------------------------------------------------------

!     The old Xijk(NT,7) array wich contains the ijk triplet information.  
      integer(C_INT), intent(in)::array_slot_0
      integer(C_INT), intent(in)::rank_0
      integer(C_INT), intent(in)::index_values_0(1:rank_0)
      integer(C_INT), intent(in)::size_0
      integer(C_INT), intent(in)::extents_0(1:rank_0)
      real(C_DOUBLE), intent(out)::data_0(1:size_0)

      integer(C_INT), intent(out)::ierr 

      TYPE(C_PTR) :: pn_mao, pmo_ranges  
      integer, pointer :: n_mao(:), mo_ranges(:)  

      integer maxi, bseg, eseg, nseg 
      integer naocc 
      integer one 
      integer i, j, k, Nt, e, ie, ii, is, je, jj, js
      integer np(10), n_s(100,10), start(100), end(100) 
      integer Xijk(10000,7) 

c------------------------------------------------------------------------

      maxi  = 5 ! HARD CODED VFL. Must be changed  
c     maxi = sip_sub_occ_segsize
c     write(6,*) ' MAXI IN SET_IJK_AAA = ', maxi
 
c-----------------------------------------------------------------------
c Determine the index 
c------------------------------------------------------------------------
  
      bseg = int_constant("baocc"//C_NULL_CHAR) 
      eseg = int_constant("eaocc"//C_NULL_CHAR) 
      nseg = eseg - bseg + 1 
 
c     write(6,*) ' ------------------------------- '  
c     write(6,*) ' IND :', ind, nseg 
 
c-----------------------------------------------------------------------
c Determine the number of occupied orbitals  
c------------------------------------------------------------------------
  
      naocc = int_constant("naocc"//C_NULL_CHAR) 
 
c-----------------------------------------------------------------------
c Determine the index ranges  
c np(i) = number of partitions of segment i 
c ns(i,k) =  
c------------------------------------------------------------------------

      call predefined_int_array("moa_seg_ranges"//C_NULL_CHAR, one, 
     *                          pn_mao, pmo_ranges) 
      call c_f_pointer(pn_mao, n_mao, [1]) 
      call c_f_pointer(pmo_ranges, mo_ranges, n_mao)  

c Make sure maxi < mo_ranges(i) 

      do i = 1, nseg 
         if (maxi .ge. mo_ranges(bseg + i - 1)) maxi = maxi - 1   
      enddo 

      start(1) = 1 
      end(1)   = mo_ranges(1) 
      if (bseg .gt. 1) then
	  do i = 2, bseg
             start(1) = end(1) + 1  
             end(1) = start(1) + mo_ranges(i) - 1 
	  enddo
      endif
      do i = 2, nseg 
         start(i) = end(i-1) + 1  
         end(i) = start(i) + mo_ranges(bseg + i - 1) - 1 
      enddo 

c      write(6,*) ' NSEG = ', nseg 
c      write(6,*) '   ', ((start(i),end(i)), i=1,nseg) 

      do i = 1, nseg 
         np(i) = (end(i) - start(i) + 1)/maxi 
         if (maxi*np(i) .lt. end(i) - start(i) + 1)
     *       np(i) = np(i) + 1 
      enddo 

c     write(6,*) ' N(I):', (np(i), i=1,nseg) 

      do i = 1, nseg 
         e = 0 
         do k = 1, np(i) - 1 
            n_s(i,k) = (end(i) - start(i) + 1)/np(i) 
            e = e + n_s(i,k) 
         enddo 
         n_s(i,np(i)) = (end(i) - start(i) + 1) - e 
      enddo 

      Nt = 0 
      ie = bseg - 1
      do i = 1, nseg 
      do ii = 1, np(i) 

         is = ie + 1 
         ie = is + n_s(i,ii) - 1 

          je = bseg - 1
          do j = 1, nseg 
          do jj = 1, np(j) 

             js = je + 1 
             je = js + n_s(j,jj) - 1 

          if (i .le. j) then 
             do k = 1, nseg 

                Nt = Nt + 1 
                Xijk(Nt,1) = i 
                Xijk(Nt,2) = is  
                Xijk(Nt,3) = ie  
                Xijk(Nt,4) = j 
                Xijk(Nt,5) = js 
                Xijk(Nt,6) = je
                Xijk(Nt,7) = k 

             enddo 
           endif 
     
           enddo 
           enddo 

       enddo 
       enddo 

       do i = 1, 7 
       Xijk(Nt+1,i) = -1 
       enddo 

c     write(6,*) ' Total number of combinations:', Nt 
c     do k = 1, Nt 
c     write(6,*) '  ', (Xijk(k,i), i=1, 7) 
c     enddo 

c-----------------------------------------------------------------------
c Put the result in the output array.  
c-----------------------------------------------------------------------

      i = 7 
      call update_xijk(Nt+1, i, Xijk, data_0) 

      return
      end

