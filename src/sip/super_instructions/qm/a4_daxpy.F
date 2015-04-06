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
c---------------------------------------------------------------------------
      subroutine A4_daxpy(
     * array_0, rank_0, index_values_0, size_0, extents_0, data_0,
     * array_1, rank_1, index_values_1, size_1, extents_1, data_1,
     * array_2, rank_2, index_values_2, size_2, extents_2, data_2,
     * ierr) BIND(C)
      use, intrinsic :: ISO_C_BINDING

      implicit none

C-----------------------------------------------------------------
C This does exactly the serial blas routine daxpy does. That 
C is A = C A  + B  where A, B are vectors and C is a scalar.
C-----------------------------------------------------------------

      include 'sip_interface.h'

C input array 

      integer(C_INT), intent(in)::array_0
      integer(C_INT), intent(in)::rank_0
      integer(C_INT), intent(in)::index_values_0(1:rank_0)
      integer(C_INT), intent(in)::size_0
      integer(C_INT), intent(in)::extents_0(1:rank_0)
      real(C_DOUBLE), intent(in)::data_0(1:size_0)

C The second array (both input and output)

      integer(C_INT), intent(in)::array_1
      integer(C_INT), intent(in)::rank_1
      integer(C_INT), intent(in)::index_values_1(1:rank_1)
      integer(C_INT), intent(in)::size_1
      integer(C_INT), intent(in)::extents_1(1:rank_1)
      real(C_DOUBLE), intent(in)::data_1(1:size_1)

C The input Scalar  

      integer(C_INT), intent(in)::array_2
      integer(C_INT), intent(in)::rank_2
      integer(C_INT), intent(in)::index_values_2(1:rank_2)
      integer(C_INT), intent(in)::size_2
      integer(C_INT), intent(in)::extents_2(1:rank_2)
      real(C_DOUBLE), intent(out)::data_2(1:size_2)

      integer(C_INT), intent(out)::ierr


      call lt_212(xxx(isource1),xxx(isource2),inda(1),
     *                    inda(2),indb(1))

      return
      end
C-------------------------------------------------------------------------

      subroutine mult_212(xxx,yyy,pa1,pa2,pb1)

      implicit none
      include 'int_gen_parms.hf'
C
      integer pa1,pa2,pb1,pb2,atype,btype,i,j,ierr,n,k
      integer company_comm, pst_get_company_comm,master,pst_get_master
      double precision xxx(1:pa1,1:pa2)
      double precision yyy(1:pb1)
      double precision z
C
#ifdef _DEBUG_LVL0
      if (me .eq. master) then
      write(6,"(a)") "Checking the input arrays"
      Write(*,"(a,1x,i4,1x,i4)") "The input dimensions: ", pb1, pa2
      call output(xxx, 1, pa1, 1, pa2, pa1, pa2,1)
      write(*,*)
      write(*,"(a)") "The occupation vector"
      Write(*,"(6(1x,F3.1))") (yyy(i),i=1,pa1)
      write(6,"(a)") "End checking input array"
      endif 
#endif 
   
      Do j = 1, pa2
         Z = yyy(j) 
         Do i = 1, pa1
            xxx(i,j) = xxx(i,j) * Z
         Enddo 

      Enddo

#ifdef _DEBUG_LVL0
      if (me .eq. master) then
      write(*,*)
      write(*,"(a)") "The scalled array"
      call output(xxx, 1, pa1, 1, pa2, pa1, pa2, 1)
      Write(*,*)
      endif
#endif

      return
      end
