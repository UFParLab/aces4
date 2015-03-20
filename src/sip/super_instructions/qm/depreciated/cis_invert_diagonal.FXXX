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
      subroutine cis_invert_diagonal(
     * array_0, rank_0, index_values_0, size_0, extents_0, data_0,
     * array_1, rank_1, index_values_1, size_1, extents_1, data_1,
     * ierr) BIND(C)
      use, intrinsic :: ISO_C_BINDING
      implicit none

c--------------------------------------------------------------------------
c Usage: execute cis_invert_diagonal T1 T2
C returns element by element t1/t2
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

      integer(C_INT), intent(in)::array_1
      integer(C_INT), intent(in)::rank_1
      integer(C_INT), intent(in)::index_values_1(1:rank_1)
      integer(C_INT), intent(in)::size_1
      integer(C_INT), intent(in)::extents_1(1:rank_1)
      real(C_DOUBLE), intent(out)::data_1(1:size_1)

      integer(C_INT), intent(out)::ierr

      call do_return_inv3(1, extents_0(1),
     *                    1, extents_0(2),
     *                    1, extents_0(3),
     *                    data_0,data_1)

      return
      end

      subroutine do_return_inv3( a1, a2, b1, b2, c1, c2, array1,array2)
      
c---------------------------------------------------------------------------
c
c
c---------------------------------------------------------------------------

      implicit none
      integer a, a1, a2, b, b1, b2, c, c1, c2, d, d1, d2, m1, m2, n1, n2
      double precision array1(a1:a2,b1:b2,c1:c2) 
      double precision array2(a1:a2,b1:b2,c1:c2) 
      double precision val

      val = 0.0 

      do c = c1,c2
      do b = b1,b2
      do a = a1,a2
          if(array2(a,b,c) .gt. 1.d-12) then
              val = array1(a,b,c)/array2(a,b,c)
          else
              val = 0.d0
          endif
          array1(a,b,c) = val
C          write(6,100) a,b,c,array1(a,b,c),array2(a,b,c),val
      enddo 
      enddo 
      enddo 
  100 format(3I3,24E14.6)

      return
      end
