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
      subroutine return_sval( 
     *  array_0, rank_0, index_values_0, size_0, extents_0, data_0,  
     *  array_1, rank_1, index_values_1, size_1, extents_1, data_1,  
     *  ierr) BIND(C) 
      use, intrinsic :: ISO_C_BINDING 
      implicit none 

c--------------------------------------------------------------------------
!> special return_sval rw
!>   Usage: execute return_sval array scalar
!>   The scalar is set to the A(i,j) array element. 
!>   This is depreciated for cases where the array A is a static.
c--------------------------------------------------------------------------

      include 'sip_interface.f' 

!     output array --> array_0 
      integer(C_INT), intent(in)::array_0
      integer(C_INT), intent(in)::rank_0
      integer(C_INT), intent(in)::index_values_0(1:rank_0)
      integer(C_INT), intent(in)::size_0
      integer(C_INT), intent(in)::extents_0(1:rank_0)
      real(C_DOUBLE), intent(out)::data_0(1:size_0) 

!     scalar array --> array_1 
      integer(C_INT), intent(in)::array_1
      integer(C_INT), intent(in)::rank_1
      integer(C_INT), intent(in)::index_values_1(1:rank_1)
      integer(C_INT), intent(in)::size_1
      integer(C_INT), intent(in)::extents_1(1:rank_1)
      real(C_DOUBLE), intent(out)::data_1(1:size_1) 
      real(C_DOUBLE) :: sval

      integer(C_INT), intent(out)::ierr 

      sval = 0.0

      if (rank_0 .gt. 2) then
         print *,'Error: return_sval at line '!,current_line
         print *,'First arg. must be a block of a 1 or 2-index array.'
c        call abort_job()
      endif

      if (rank_1 .ne. 0) then
         print *,'Error: return_sval at line '!,current_line,' not ',
         print *,'called something other than scalar in 2nd arg.'
c        call abort_job()
      endif

      if (rank_0 .eq. 2) then 

         if (extents_0(1) .ne. 1) then  
             write(6,*) ' return sval called using array without 
     *                    simple indeces'
         endif 

         if (extents_0(2) .ne. 1) then  
             write(6,*) ' return sval called using array without 
     *                    simple indeces'
         endif 

         call doreturn2(data_0, sval,
     *              1, extents_0(1), 1, extents_0(2)) 
	 data_1(1) = sval

         return 
      endif 

      if (rank_0 .eq. 1) then 

         call doreturn1(data_0, sval,
     *              1, extents_0(1)) 
	 data_1(1) = sval

         return 
      endif 



      end 

      subroutine doreturn2(array1,sval,a1,a2,b1,b2) 

c---------------------------------------------------------------------------
c
c 
c---------------------------------------------------------------------------

      implicit none
      integer a, a1, a2, b, b1, b2   
      double precision array1(a1:a2,b1:b2), sval   

      sval = 0.0d+0 
c     write(6,*) ' RANGE :', a1, a2, b1, b2 
       do a = a1, a2 
       do b = b1, b2 
            sval = array1(a,b) 
c          write(6,*) ' SVAL ', a, b, sval 
       enddo ! b 
       enddo ! a 
      sval = array1(a2,b2)
c
      return
      end


      subroutine doreturn1(array1,sval,a1,a2) 

c---------------------------------------------------------------------------
c
c 
c---------------------------------------------------------------------------

      implicit none
      integer a, a1, a2
      double precision array1(a1:a2), sval   

      sval = 0.0d+0 
      sval = array1(a2)
c
      return
      end

