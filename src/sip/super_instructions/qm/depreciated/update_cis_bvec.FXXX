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
      subroutine update_cis_bvec(
     * array_0, rank_0, index_values_0, size_0, extents_0, data_0,
     * array_1, rank_1, index_values_1, size_1, extents_1, data_1,
     * array_2, rank_2, index_values_2, size_2, extents_2, data_2,
     * array_3, rank_3, index_values_3, size_3, extents_3, data_3,
     * ierr) BIND(C)
      use, intrinsic :: ISO_C_BINDING
      implicit none

c--------------------------------------------------------------------------
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
      real(C_DOUBLE), intent(in)::data_1(1:size_1)

      integer(C_INT), intent(in)::array_2
      integer(C_INT), intent(in)::rank_2
      integer(C_INT), intent(in)::index_values_2(1:rank_2)
      integer(C_INT), intent(in)::size_2
      integer(C_INT), intent(in)::extents_2(1:rank_2)
      real(C_DOUBLE), intent(in)::data_2(1:size_2)

      integer(C_INT), intent(in)::array_3
      integer(C_INT), intent(in)::rank_3
      integer(C_INT), intent(in)::index_values_3(1:rank_3)
      integer(C_INT), intent(in)::size_3
      integer(C_INT), intent(in)::extents_3(1:rank_3)
      real(C_DOUBLE), intent(in)::data_3(1:size_3)

      integer(C_INT), intent(out)::ierr

      if (rank_0.ne.3 .and. rank_1.ne.3 .and. 
     *    rank_2.ne.3 .and. rank_3.ne.3) then
          write(6,*) "rank missmatch in update_cis_bvec"
          ierr = ierr + 1
      endif
         
      if ((3*extents_1(1) .ne. extents_0(1)) .or.
     *    (3*extents_2(1) .ne. extents_0(1)) .or.
     *    (3*extents_3(1) .ne. extents_0(1))) then
          write(6,*) "subspace size missmatch in update_cis_bvec"
          ierr = ierr + 1
      endif

      if (ierr .ne. 0) call abort_job()

      call cisbvecupdate(extents_0(1),extents_1(1),extents_1(2),
     *                   extents_1(3),data_0,data_1,data_2,data_3)

      return
      end

c---------------------------------------------------------------------------

      subroutine cisbvecupdate(nsub,nroot,nvir,nocc,bvec,ck0,ck1,rk)
      use, intrinsic :: ISO_C_BINDING 
      implicit none
      integer nsub,nroot,nvir,nocc
      double precision bvec(nsub,nvir,nocc)
      double precision ck0(nroot,nvir,nocc)
      double precision ck1(nroot,nvir,nocc)
      double precision rk(nroot,nvir,nocc)
      double precision val1, val2

      integer nroot2, k,a,i,j


      nroot2 = 2*nroot

C form new bvec
      do k = 1,nroot
          do a = 1,nvir
              do i = 1,nocc
                  bvec(k,a,i)        = ck0(k,a,i)
                  bvec(k+nroot,a,i)  = ck1(k,a,i)
                  bvec(k+nroot2,a,i) = rk(k,a,i)
              enddo 
C              write(6,100) k,a,(ck0(k,a,i),i=1,nocc)
          enddo
      enddo
C normalize
      do k=1,nsub
          val1 = 0.d0
          do a = 1,nvir
              do i = 1,nocc
                  val1 = val1 + bvec(k,a,i)*bvec(k,a,i)
              enddo
          enddo
          val2 = dsqrt(val1)
          do a = 1,nvir
              do i = 1,nocc
                  bvec(k,a,i) = bvec(k,a,i)/val2
              enddo
          enddo
      enddo
C do GS orthogonalization
      do k = 2,nsub !I
      do j = 1,k-1    !j
          val1 = 0.d0
          val2 = 0.d0
C form dotproducts
          do a = 1,nvir
              do i = 1,nocc
                  val1 = val1 + bvec(k,a,i)*bvec(j,a,i)
                  val2 = val2 + bvec(j,a,i)*bvec(j,a,i)
              enddo
          enddo
          do a = 1,nvir
              do i = 1,nocc
                  bvec(k,a,i) = bvec(k,a,i) - val1*bvec(j,a,i)/val2
              enddo
          enddo
      enddo
      enddo
C normalize
      do k=1,nsub
          val1 = 0.d0
          do a = 1,nvir
              do i = 1,nocc
                  val1 = val1 + bvec(k,a,i)*bvec(k,a,i)
              enddo
          enddo
          val2 = dsqrt(val1)
          do a = 1,nvir
              do i = 1,nocc
                  bvec(k,a,i) = bvec(k,a,i)/val2
              enddo
          enddo
      enddo
C      do a = 1,nvir
C          do i = 1,nocc
C              write(6,100) a,i,(bvec(k,a,i),k=1,nsub)
C          enddo
C      enddo
C
  100 format(2I2,24F14.6)

      return
      end
