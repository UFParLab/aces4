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
      subroutine check_dconf(
     *  array_0, rank_0, index_values_0, size_0, extents_0, data_0,  
     *  array_1, rank_1, index_values_1, size_1, extents_1, data_1,  
     *  ierr) BIND(C) 
      use, intrinsic :: ISO_C_BINDING 
      implicit none 

c--------------------------------------------------------------------------
!> special check_dconf rw
!>  takes an incoming two dimensional block and returns the largest element of that block
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

      integer(C_INT), intent(out)::ierr 
      real(C_DOUBLE) :: val

c     if (nindex .ne. 2) then
c        print *,'Error: check_dconf at line ',current_line
c        print *,'First arg. must be a block of a 2-index array.'
c        call abort_job()
c     endif

c     if (nindex2 .ne. 0) then
c        print *,'Error: check_dconf at line ',current_line,' not ',
c    *           'called with scalar in 2nd arg.'
c        call abort_job()      
c     endif
 
      val = 0.0
      call docheck(data_0, val, 1, extents_0(1), 1, extents_0(2))  
      data_1(1) = val

      return 
      end 

      subroutine docheck(array1,val,a1,a2,b1,b2) 

c---------------------------------------------------------------------------
c
c 
c---------------------------------------------------------------------------

      implicit none
      integer a, a1, a2, b, b1, b2   
      double precision array1(a1:a2,b1:b2), val, eval   

      val = 0.0 
      do a = a1, a2 
      do b = b1, b2 
           eval = dabs(array1(a,b)) 
           if (eval .gt. val) val = eval  
      enddo ! b 
      enddo ! a 
c
      return
      end

