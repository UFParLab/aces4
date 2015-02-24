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

C Shell to drive LAPACK algorithm.

C A    - REAL SYMMETRIC MATRIX TO BE DIAGONALIZED (E.VALUES IN DIAGS AFTERWARDS)
C B    - EIGENVECTORS RETURNED IN COLUMNS
C JUNK - not used
C N    - SIZE OF MATRIX
C SORT - EIGENVECTORS AND EIGENVALUES ARE REORDERED, with eigenvalues:
c      0     - ascending
c      1     - unordered
c      other - descending

c#include "aces.h"

      subroutine eig(a,b,junk,n,sort)
      use, intrinsic :: ISO_C_BINDING 
      implicit none 

      integer junk, n, sort
      double precision a(n,n),b(n,n),c(n*n) 

      integer i, j, k, index, ierr, lda
      double precision dtmp, reserve(9)

      double precision tolerance
      parameter (tolerance = 1.0d-5)

c ----------------------------------------------------------------------

      if (n.lt.1) return
      if (n.eq.1) then
         b(1,1) = 1.0d0
         return
      end if

      do j = 1, n
         do i = 1, n
            b(i,j) = a(i,j) 
         end do
      end do
c#ifdef _ASSERT
      dtmp = 0.d0
      do j = 2, n
         do i = 1, j-1
            dtmp = dtmp + dabs(a(j,i)-a(i,j))
         end do
      end do
      if (dtmp.gt.1.d-8) then
         print *, '@EIG: WARNING - the matrix is not symmetric'
         print *, '      sum of differences is ',dtmp
      end if
c#endif

      do i = 1, n
      do j = 1, n
         a(i,j) = 0.d0
      enddo
      enddo

c     print *, "Commented out call to dsyev in eig.F to allow compile"
      ierr = sort
      if (n.lt.4) then
         do i = 1, 9
            reserve(i) = 0.d0
         enddo

         call dsyev('V','L',n,b,n,a,reserve,9,      ierr)
      else

c     do i = 1, n 
c     do j = 1, n 
c        write(6,*) ' EIG BEFORE:', j,i, b(j,i) 
c     enddo 
c     enddo 
         call dsyev('V','L',n,b,n,a,a(1,2), (n*n-n),ierr)
      end if
      if (ierr.ne.0) then
         write(*,*) '@EIG: Eigenvalue not found. Error code: ',ierr
c         call errex
         call abort_job()
      end if
c     if ((sort.ne.0).and.(sort.ne.1)) return  
      

c   o make first significant element of each eigenvector positive
      do j = 1, n
         index = 0
         do i = 1, n
            if (index.eq.0) then
               if (abs(b(i,j)).gt.tolerance) index = i
            end if
         end do
         if (index.ne.0) then
            if (b(index,j).lt.0.0d0) then
               do i = 1, n
                  b(i,j) = -b(i,j)
               end do
            end if
         end if
      end do

      if ((sort.ne.0).and.(sort.ne.1)) then
         do i = 1, n-1
            do j = i+1, n
               if ( a(i,1) .lt. a(j,1) ) then
                  dtmp   = a(i,1)
                  a(i,1) = a(j,1)
                  a(j,1) = dtmp
                  do k = 1, n
                     dtmp   = b(k,i)
                     b(k,i) = b(k,j)
                     b(k,j) = dtmp
                  end do
               end if
            end do
         end do
      end if

c   o put the eigenvalues along the diagonal of A and zero the rest
      do j = 2, n
         do i = 1, n
            a(i,j) = 0.0d0
         end do
      end do
      do i = 1, n
         a(i,i) = a(i,1)
      end do
      do i = 2, n
         a(i,1) = 0.0d0
      end do

c     do i = 1, n 
c     do j = 1, n 
c        write(6,*) ' EIG AFTER:', j,i, b(j,i)
c     enddo 
c     enddo 

      return
      end

