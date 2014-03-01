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
      subroutine stripi(
     * array_slot_0, rank_0, index_values_0, size_0, extents_0, data_0,
     # array_slot_1, rank_1, index_values_1, size_1, extents_1, data_1
     *                 ) BIND(C) 
c--------------------------------------------------------------------------
c  In this routine the index i is stripped from the array1 and the 
c  resulting array is returned as array2. This routine is VERY specific 
c in that ONLY the lasi index may be stripped. This is only out of
c convenience and easily modified to be general. However as this routine
c will probably not be extensively used it is left as is for now. 
c VFL 1/27/2014 
c--------------------------------------------------------------------------

      use, intrinsic :: ISO_C_BINDING 
      implicit none

      include 'sip_interface.f'

!     input array
      integer(C_INT), intent(in)::array_slot_0
      integer(C_INT), intent(in)::rank_0
      integer(C_INT), intent(in)::index_values_0(1:rank_0)
      integer(C_INT), intent(in)::size_0
      integer(C_INT), intent(in)::extents_0(1:rank_0)
      real(C_DOUBLE), intent(out)::data_0(1:size_0)

!     array returned with 'stripped' index  
      integer(C_INT), intent(in)::array_slot_1
      integer(C_INT), intent(in)::rank_1
      integer(C_INT), intent(in)::index_values_1(1:rank_1)
      integer(C_INT), intent(in)::size_1
      integer(C_INT), intent(in)::extents_1(1:rank_1)
      real(C_DOUBLE), intent(in)::data_1(1:size_1)

c aosegments 
c     integer(C_INT) n_ao_segs(1), aosegs(6)    
      TYPE(C_PTR) :: n_ao_segs, aosegs
      integer, pointer :: pn_ao_segs(:), paosegs(:)

      integer(C_INT) offset0_1, offset0_2, offset0_3, offset0_4  
      integer(C_INT) offset1_1, offset1_2, offset1_3, offset1_4  
      integer(C_INT) one, i   

c ----------------------------------------------------------------------
c Get the ao segment ranges  
      call predefined_int_array("moa_seg_ranges"//C_NULL_CHAR, one,
     *                           n_ao_segs, aosegs)
      call c_f_pointer(n_ao_segs,pn_ao_segs,[1])
      call c_f_pointer(aosegs,paosegs,pn_ao_segs)
c     if (rank_0 .eq. 2) 
c    *write(6,*) ' NAO_SEGS', pn_ao_segs(1), (paosegs(i), i=1, 
c    *                                             pn_ao_segs(1)) 
c ----------------------------------------------------------------------

      if (rank_0 .eq. 4) then 

c Check for simple index 
c     write(6,*) ' index_values(4):', index_values_1(4) 
c     write(6,*) ' extents(4):', extents_1(1), extents_1(2), 
c    *                           extents_1(3), extents_1(4)  

      offset0_1 = 0
      offset0_2 = 0
      offset0_3 = 0
      offset0_4 = 0

      if (index_values_0(1).gt.1) then 
          do i = 1, index_values_0(1) - 1 
             offset0_1 = offset0_1 + paosegs(i) 
          enddo 
      endif 

      if (index_values_0(2).gt.1) then 
          do i = 1, index_values_0(2) - 1 
             offset0_2 = offset0_2 + paosegs(i) 
          enddo 
      endif 

      if (index_values_0(3).gt.1) then 
          do i = 1, index_values_0(3) - 1 
             offset0_3 = offset0_3 + paosegs(i) 
          enddo 
      endif 

      if (index_values_0(4).gt.1) then 
          do i = 1, index_values_0(4) - 1 
             offset0_4 = offset0_4 + paosegs(i) 
          enddo 
      endif 

c     if (index_values_0(1).gt.1) offset0_1=paosegs(index_values_0(1)-1)
c     if (index_values_0(2).gt.1) offset0_2=paosegs(index_values_0(2)-1)
c     if (index_values_0(3).gt.1) offset0_3=paosegs(index_values_0(3)-1)
c     if (index_values_0(4).gt.1) offset0_4=paosegs(index_values_0(4)-1)

      offset1_1 = 0
      offset1_2 = 0
      offset1_3 = 0
      offset1_4 = 0

      if (index_values_1(1).gt.1) then 
          do i = 1, index_values_1(1) - 1 
             offset1_1 = offset1_1 + paosegs(i) 
          enddo 
      endif 

      if (index_values_1(2).gt.1) then 
          do i = 1, index_values_1(2) - 1 
             offset1_2 = offset1_2 + paosegs(i) 
          enddo 
      endif 

      if (index_values_1(3).gt.1) then 
          do i = 1, index_values_1(3) - 1 
             offset1_3 = offset1_3 + paosegs(i) 
          enddo 
      endif 

      call do_istrip4(data_0,  
     *                  1+offset0_1, extents_0(1)+offset0_1, 
     *                  1+offset0_2, extents_0(2)+offset0_2, 
     *                  1+offset0_3, extents_0(3)+offset0_3, 
     *                  1+offset0_4, extents_0(4)+offset0_4, 

     *                  data_1,   
     *                  1+offset1_1, extents_1(1)+offset1_1, 
     *                  1+offset1_2, extents_1(2)+offset1_2, 
     *                  1+offset1_3, extents_1(3)+offset1_3, 
     *                  index_values_1(4), index_values_1(4)
     *                  )
      endif 

      if (rank_0 .eq. 3) then 

      offset0_1 = 0
      offset0_2 = 0
      offset0_3 = 0

      if (index_values_0(1).gt.1) then 
          do i = 1, index_values_0(1) - 1 
             offset0_1 = offset0_1 + paosegs(i) 
          enddo 
      endif 

      if (index_values_0(2).gt.1) then 
          do i = 1, index_values_0(2) - 1 
             offset0_2 = offset0_2 + paosegs(i) 
          enddo 
      endif 

      if (index_values_0(3).gt.1) then 
          do i = 1, index_values_0(3) - 1 
             offset0_3 = offset0_3 + paosegs(i) 
          enddo 
      endif 

      offset1_1 = 0
      offset1_2 = 0
      offset1_3 = 0

      if (index_values_1(1).gt.1) then 
          do i = 1, index_values_1(1) - 1 
             offset1_1 = offset1_1 + paosegs(i) 
          enddo 
      endif 

      if (index_values_1(2).gt.1) then 
          do i = 1, index_values_1(2) - 1 
             offset1_2 = offset1_2 + paosegs(i) 
          enddo 
      endif 

      call do_istrip3(data_0,  
     *                  1+offset0_1, extents_0(1)+offset0_1, 
     *                  1+offset0_2, extents_0(2)+offset0_2, 
     *                  1+offset0_3, extents_0(3)+offset0_3, 

     *                  data_1,   
     *                  1+offset1_1, extents_1(1)+offset1_1, 
     *                  1+offset1_2, extents_1(2)+offset1_2, 
     *                  index_values_1(3), index_values_1(3)
     *                  )
      endif 

      if (rank_0 .eq. 2) then 

      offset0_1 = 0
      offset0_2 = 0

      if (index_values_0(1).gt.1) then 
          do i = 1, index_values_0(1) - 1 
             offset0_1 = offset0_1 + paosegs(i) 
          enddo 
      endif 

      if (index_values_0(2).gt.1) then 
          do i = 1, index_values_0(2) - 1 
             offset0_2 = offset0_2 + paosegs(i) 
          enddo 
      endif 

      offset1_1 = 0
      offset1_2 = 0

      if (index_values_1(1).gt.1) then 
          do i = 1, index_values_1(1) - 1 
             offset1_1 = offset1_1 + paosegs(i) 
          enddo 
      endif 

      call do_istrip2(data_0,  
     *                  1+offset0_1, extents_0(1)+offset0_1, 
     *                  1+offset0_2, extents_0(2)+offset0_2, 

     *                  data_1,   
     *                  1+offset1_1, extents_1(1)+offset1_1, 
     *                  index_values_1(2), index_values_1(2)
     *                  )
      endif 
 
      return
      end

      subroutine do_istrip4(x, a1, a2, b1, b2, c1, c2, d1, d2,
     *                      y, e1, e2, f1, f2, g1, g2, h1, h2
     *                      )
c--------------------------------------------------------------------------
c  Routine which performs the actual stripping. Only ONE index can be 
c  stripped and the arrays must be 4-D.   
c--------------------------------------------------------------------------
      implicit none

      integer a1,a2,b1,b2,c1,c2,d1,d2
      integer e1,e2,f1,f2,g1,g2,h1,h2
      integer sindex(4), findex(4), 
     *        match(4) 

      integer i, j, k, a, b, c, d 
      integer m, n, lda, ldb, ldc 
      integer strip(4), nstrip   
      integer astart, aend, bstart, bend, cstart, cend, dstart, dend 

      double precision x(a1:a2,b1:b2,c1:c2,d1:d2)
      double precision y(e1:e2,f1:f2,g1:g2,h1:h2)

c *********************************************************************************** 
c     Find the nonmatching indeces and make sure that array2 has simple indeces 
c     in those locations. 
c *********************************************************************************** 

      do i = 1, 4 
         sindex(i) = 0
         match(i) = 0
      enddo 
      if (e1 .eq. e2) sindex(1) = 1 
      if (f1 .eq. f2) sindex(2) = 1 
      if (g1 .eq. g2) sindex(3) = 1 
      if (h1 .eq. h2) sindex(4) = 1 

      nstrip = 0 
      do i = 1, 4 
         strip(i) = 0 
         if (sindex(i) .ne. 0) then 
             if (i .eq. 1) match(i) = e1 
             if (i .eq. 2) match(i) = f1 
             if (i .eq. 3) match(i) = g1 
             if (i .eq. 4) match(i) = h1 
             nstrip = nstrip + 1 
             strip(nstrip) = i 
         else 
             match(i) = 0 
         endif 
      enddo 

      if (match(1) .ne. 0) then 
         if ((e1 .lt. a1) .or. (e1 .gt. a2)) then 
            write(6,*) ' Mismatch trying to strip indices ' 
            call abort_job() 
         endif 
         astart = e1 
         aend   = e1 
      else 
         astart = a1 
         aend   = a2 
      endif 

      if (match(2) .ne. 0) then 
         if ((f1 .lt. b1) .or. (f1 .gt. b2)) then 
            write(6,*) ' Mismatch trying to strip indices ' 
            call abort_job() 
         endif 
         bstart = f1 
         bend   = f1 
      else 
         bstart = b1 
         bend   = b2 
      endif 

      if (match(3) .ne. 0) then 
         if ((g1 .lt. c1) .or. (g1 .gt. c2)) then 
            write(6,*) ' Mismatch trying to strip indices ' 
            call abort_job() 
         endif 
         cstart = g1 
         cend   = g1 
      else 
         cstart = c1 
         cend   = c2 
      endif 

      if (match(4) .ne. 0) then 
         if ((h1 .lt. d1) .or. (h1 .gt. d2)) then 
            write(6,*) ' Mismatch trying to strip indices in strip4' 
            write(6,*) ' H1 H2:', h1, h2 
            write(6,*) ' D1 D2:', d1, d2 
            call abort_job() 
         endif 
         dstart = h1 
         dend   = h1 
      else 
         dstart = d1 
         dend   = d2 
      endif 

      do d = dstart, dend  
      do c = cstart, cend  
      do b = bstart, bend  
      do a = astart, aend 

         y(a,b,c,d) = x(a,b,c,d) 

      enddo 
      enddo 
      enddo 
      enddo 
      return 


c *********************************************************************************** 
c Strip one index 
c *********************************************************************************** 
c
      if (nstrip .eq. 1) then 
c *********************************************************************************** 
c     The fourth index stripped 
c *********************************************************************************** 

      if ((strip(1) .eq. 4) .and. (h1 .ge. d1) .and. (h1 .le. d2)) then 

      do c = c1, c2 
      do b = b1, b2 
      do a = a1, a2 
c     do d = d1, d2 

         y(a,b,c,h1) = x(a,b,c,h1) 

c     enddo 
      enddo 
      enddo 
      enddo 

      go to 100 

      endif 

c *********************************************************************************** 
c     The Third index stripped 
c *********************************************************************************** 

      if ((strip(1) .eq. 3) .and. (g1 .ge. c1) .and. (g1 .le. c2)) then 

c     do c = c1, c2 
      do d = d1, d2 
      do b = b1, b2 
      do a = a1, a2 

         y(a,b,g1,d) = x(a,b,g1,d) 

      enddo 
      enddo 
      enddo 
c     enddo 

      go to 100 

      endif 

c *********************************************************************************** 
c     The Second index stripped 
c *********************************************************************************** 

      if ((strip(1) .eq. 2) .and. (f1 .ge. b1) .and. (f1 .le. b2)) then 

      do d = d1, d2 
      do c = c1, c2 
c     do b = b1, b2 
      do a = a1, a2 

         y(a,f1,c,d) = x(a,f1,c,d) 

      enddo 
      enddo 
c     enddo 
      enddo 

      go to 100 

      endif 

c *********************************************************************************** 
c     The First index stripped 
c *********************************************************************************** 

      if ((strip(1) .eq. 1) .and. (e1 .ge. a1) .and. (e1 .le. a2)) then 

      do d = d1, d2 
      do c = c1, c2 
      do b = b1, b2 
c     do a = a1, a2 

         y(e1,b,c,d) = x(e1,b,c,d) 

      enddo 
      enddo 
c     enddo 
      enddo 

      go to 100 

      endif 

      endif ! nstrip = 1 

c *********************************************************************************** 
c Strip two indices  
c *********************************************************************************** 
c
      if (nstrip .eq. 2) then 
c *********************************************************************************** 
c     The index 1 1nd 2 are stripped 
c *********************************************************************************** 

         if ((strip(1) .eq. 1) .and. (strip(2) .eq. 2)) then 
            do c = c1, c2 
            do d = d1, d2 
               y(e1,f1,c,d) = x(e1,f1,c,d) 
            enddo 
            enddo 
            return 
         endif 

      endif 


      write(6,*) ' No indeces were found to strip in strip4 '
      write(6,*) ' Strip = ', (strip(i), i=1, 4)  
      write(6,*) ' Match(i) = ', (match(i), i=1,4)  
      call abort_job() 

100   continue 

      return
      end


      subroutine do_istrip3(x, a1, a2, b1, b2, c1, c2,
     *                      y, e1, e2, f1, f2, g1, g2
     *                      )
c--------------------------------------------------------------------------
c  Routine which performs the actual stripping. Only ONE index can be 
c  stripped and the arrays must be 4-D.   
c--------------------------------------------------------------------------
      implicit none

      integer a1,a2,b1,b2,c1,c2,d1,d2
      integer e1,e2,f1,f2,g1,g2,h1,h2
      integer sindex(3), findex(3), 
     *        match(3) 

      integer i, j, k, a, b, c, d 
      integer m, n, lda, ldb, ldc 
      integer strip  

      double precision x(a1:a2,b1:b2,c1:c2)
      double precision y(e1:e2,f1:f2,g1:g2)

c *********************************************************************************** 
c     Find the nonmatching indeces and make sure that array2 has simple indeces 
c     in those locations. 
c *********************************************************************************** 

      do i = 1, 3 
         sindex(i) = 0
      enddo 
      if (e1 .eq. e2) sindex(1) = 1 
      if (f1 .eq. f2) sindex(2) = 1 
      if (g1 .eq. g2) sindex(3) = 1 

      do i = 1, 3 
         if (sindex(i) .ne. 0) then 
             if (i .eq. 1) match(i) = e1 
             if (i .eq. 2) match(i) = f1 
             if (i .eq. 3) match(i) = g1 
             strip = i 
         else 
             match(i) = 0 
         endif 
      enddo 

c *********************************************************************************** 
c     The Third index stripped 
c *********************************************************************************** 

      if ((strip .eq. 3) .and. (g1 .ge. c1) .and. (g1 .le. c2)) then 

c     do c = c1, c2 
      do b = b1, b2 
      do a = a1, a2 

         y(a,b,g1) = x(a,b,g1) 

      enddo 
      enddo 
c     enddo 

      go to 100 

      endif 

c *********************************************************************************** 
c     The Second index stripped 
c *********************************************************************************** 

      if ((strip .eq. 2) .and. (f1 .ge. b1) .and. (f1 .le. b2)) then 

      do c = c1, c2 
c     do b = b1, b2 
      do a = a1, a2 

         y(a,f1,c) = x(a,f1,c) 

      enddo 
      enddo 
c     enddo 

      go to 100 

      endif 

c *********************************************************************************** 
c     The First index stripped 
c *********************************************************************************** 

      if ((strip .eq. 1) .and. (e1 .ge. a1) .and. (e1 .le. a2)) then 

      do c = c1, c2 
      do b = b1, b2 
c     do a = a1, a2 

         y(e1,b,c) = x(e1,b,c) 

      enddo 
      enddo 
c     enddo 

      go to 100 

      endif 

      write(6,*) ' No indeces were found to strip in strip3 '
      write(6,*) ' Strip = ', strip  
      write(6,*) ' Match(i) = ', (match(i), i=1,3)  
      call abort_job() 

100   continue 

      return
      end


      subroutine do_istrip2(x, a1, a2, b1, b2, 
     *                      y, e1, e2, f1, f2   
     *                      )
c--------------------------------------------------------------------------
c  Routine which performs the actual stripping. Only ONE index can be 
c  stripped and the arrays must be 4-D.   
c--------------------------------------------------------------------------
      implicit none

      integer a1,a2,b1,b2
      integer e1,e2,f1,f2
      integer sindex(2), findex(2), 
     *        match(2) 

      integer i, j, k, a, b, c, d 
      integer m, n, lda, ldb, ldc 
      integer strip  

      double precision x(a1:a2,b1:b2)
      double precision y(e1:e2,f1:f2)

c *********************************************************************************** 
c     Find the nonmatching indeces and make sure that array2 has simple indeces 
c     in those locations. 
c *********************************************************************************** 

      do i = 1, 2 
         sindex(i) = 0
      enddo 
      if (e1 .eq. e2) sindex(1) = 1 
      if (f1 .eq. f2) sindex(2) = 1 

      do i = 1, 2 
         if (sindex(i) .ne. 0) then 
             if (i .eq. 1) match(i) = e1 
             if (i .eq. 2) match(i) = f1 
             strip = i 
         else 
             match(i) = 0 
         endif 
      enddo 

c *********************************************************************************** 
c     The Second index stripped 
c *********************************************************************************** 

      if ((strip .eq. 2) .and. (f1 .ge. b1) .and. (f1 .le. b2)) then 

      do a = a1, a2 

         y(a,f1) = x(a,f1) 

      enddo 

      go to 100 

      endif 

c *********************************************************************************** 
c     The first index stripped 
c *********************************************************************************** 

      if ((strip .eq. 1) .and. (e1 .ge. a1) .and. (e1 .le. a2)) then 

      do b = b1, b2 

         y(e1,b) = x(e1,b) 

      enddo 

      go to 100 

      endif 

      write(6,*) ' No indeces were found to strip in strip3 '
      write(6,*) ' Strip = ', strip  
      write(6,*) ' Match(i) = ', (match(i), i=1,3)  
      call abort_job() 

100   continue 

      return
      end

