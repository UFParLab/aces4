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
C-------------------------------------------------
C computes the unit vector guess to the singles vector based on the
C GAMESS "algorithm"
      subroutine cis_unit_guess(
     c array_slot_0, rank_0, index_values_0, size_0, extents_0, data_0,
     c array_slot_1, rank_1, index_values_1, size_1, extents_1, data_1,
C     c array_slot_2, rank_2, index_values_2, size_2, extents_2, data_2,
     c  ierr) BIND(C)
      use, intrinsic :: ISO_C_BINDING
      implicit none
      integer(C_INT), intent(in)::array_slot_0
      integer(C_INT), intent(in)::rank_0
      integer(C_INT), intent(in)::index_values_0(1:rank_0)
      integer(C_INT), intent(in)::size_0
      integer(C_INT), intent(in)::extents_0(1:rank_0)
      real(C_DOUBLE), intent(inout)::data_0(1:size_0)

      integer(C_INT), intent(in)::array_slot_1
      integer(C_INT), intent(in)::rank_1
      integer(C_INT), intent(in)::index_values_1(1:rank_1)
      integer(C_INT), intent(in)::size_1
      integer(C_INT), intent(in)::extents_1(1:rank_1)
      real(C_DOUBLE), intent(inout)::data_1(1:size_1)

C      integer(C_INT), intent(in)::array_slot_2
C      integer(C_INT), intent(in)::rank_2
C      integer(C_INT), intent(in)::index_values_2(1:rank_2)
C      integer(C_INT), intent(in)::size_2
C      integer(C_INT), intent(in)::extents_2(1:rank_2)
C      real(C_DOUBLE), intent(inout)::data_2(1:size_2)

      integer(C_INT), intent(out)::ierr

      integer current_line, isub, ivir, iocc

      ierr = 0
      if (rank_0 .ne. 3) then
         ierr = ierr + 1
         print *,'Error in cis_unit_guess, line ',current_line,
     *           ' Number of array',
     *           ' indices must be 3, actual number is ',rank_0
      endif

      if (rank_1 .ne. 2) then
         ierr = ierr + 1
         print *,'Error in cis_unit_guess, line ',current_line,
     *           ' Number of array',
     *           ' indices must be 1, actual number is ',rank_0
      endif

C      if (rank_2 .ne. 1) then
C         ierr = ierr + 1
C         print *,'Error in cis_unit_guess, line ',current_line,
C     *           ' Number of array',
C     *           ' indices must be 1, actual number is ',rank_0
C      endif

      if (ierr .ne. 0) call abort_job()

      isub = extents_0(1)
      ivir = extents_0(2)
      iocc = extents_0(3)
C      call unitvecguess(data_0,data_1,isub,ivir,iocc,ivir+iocc)
      call unitvecguess(data_0,data_1,isub,ivir,iocc)
C      call unitvecguess(data_0,data_1,data_2,isub,ivir,iocc)

      return
      end
C
C      subroutine unitvecguess(Bvec,fock,isub,ivir,iocc,no)
      subroutine unitvecguess(Bvec,Hdiag,isub,ivir,iocc)
C      subroutine unitvecguess(Bvec,Ovec,Vvec,isub,ivir,iocc)
      use, intrinsic :: ISO_C_BINDING 
      implicit none 

      integer isub, ivir, iocc, no
C      double precision Bvec(isub,ivir,iocc), fock(no,no)
      double precision Bvec(isub,ivir,iocc), Hdiag(ivir,iocc)
C      double precision Bvec(isub,ivir,iocc), Ovec(iocc), Vvec(ivir)

      integer i, a, k, isav, asav
      double precision EE

      do k = 1,isub
          do i = 1,iocc
              do a = 1,ivir
                  Bvec(k,a,i) = 0.d0
              enddo
          enddo
      enddo 

      do i = 1,iocc
          do a = 1,ivir
C              Bvec(isub,a,i) = fock(iocc + a,iocc + a) - fock(i,i)
              Bvec(isub,a,i) = Hdiag(a,i)
          enddo
      enddo

      do k = 1,isub-1
          EE = 1.d8
          do i = 1,iocc
              do a = 1,ivir
                  if(Bvec(isub,a,i) .le. EE) then
                      EE = Bvec(isub,a,i)
                      isav = i
                      asav = a
                  endif
              enddo
          enddo
          Bvec(k,asav,isav) = 1.d0
          Bvec(isub,asav,isav) = 1.d9
      enddo 
C
C complicated because we are using the last subspace column of Bvec as scratch
      EE = 1.d8
      do i = 1,iocc
          do a = 1,ivir
              if(Bvec(isub,a,i) .le. EE) then
                  EE = Bvec(isub,a,i)
                  isav = i
                  asav = a
              endif
          enddo
      enddo
      do i = 1,iocc
          do a = 1,ivir
              Bvec(isub,a,i) = 0.d0
          enddo
      enddo
      Bvec(isub,asav,isav) = 1.d0

C      do a = 1,ivir
C          do i = 1,iocc
CC              write(6,100) a,i,fock(iocc + a,iocc + a) - fock(i,i) !Hdiag(a,i)
C              write(6,100) a,i,Hdiag(a,i)
C          enddo
C      enddo
C      do a = 1,ivir
C          do i = 1,iocc
C              write(6,100) a,i,(bvec(k,a,i),k=1,isub)
C          enddo
C      enddo
  100 format(2I2,24F14.6)

      return
      end
