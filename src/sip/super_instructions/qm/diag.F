c  Copyright (c) 2003-2010 University of Florida
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
C DOSQ - THE SQUARE ROOT OF THE DIAGONALIZED MATRIX IS TAKEN 
C      0     - no 
C      1     - yes  
C DOINV- TAKE THE INVERSE OF THE DIAGONALIZED MATRIX
C      0     - no 
C      1     - yes  


      subroutine diag(a,b,junk,n,sort,dosq,doinv)
      use, intrinsic :: ISO_C_BINDING 
      implicit none 
C     include 'machine_types.h'
C#include "parallel_info.h"
C      include 'trace.h'

C TODO TODO
      integer lindep_tol  

      integer junk, n, sort, dosq, doinv,NLINDEP  
      double precision a(n,n),b(n,n),tresh,qqq

      integer i, j

c ----------------------------------------------------------------------
c   Call the ACESII "eig" routine.
c-----------------------------------------------------------------------
      lindep_tol = 10 ! TODO  
      tresh = 10**(-1.d0*lindep_tol)

      call eig(a,b,junk,n,sort)
c
c take the square root if desired 
c ------------------------------- 
c
      if (dosq .eq. 1) then  
c  

         do i = 1, n
            a(i,i) = dsqrt(a(i,i))
         enddo 
c
      endif 
c
c take the inverse if desired 
c --------------------------- 
c
      if (doinv .eq. 1) then  


c      write(6,*) '-------------------------------------------'
c      write(6,*) 'Linear dependency threshold',tresh
c      write(6,*) '-------------------------------------------'

      NLINDEP = 0
c------------------------------------------------------------------
         do i = 1, n
            qqq = a(i,i)*a(i,i) 
            IF(dabs(qqq).ge.tresh) then 
            a(i,i) = 1.0d0/a(i,i) 
            ELSE
            a(i,i) = 0.d0
            NLINDEP = NLINDEP + 1
            ENDIF
         enddo 
c------------------------------------------------------------------

c         write(6,*)'==================================================='
c         write(6,*) 'Basis of atomic orbitals contains', NLINDEP,
c     & 'linear dependecies'
c        write(6,*)'===================================================='

      endif 
c 
      return
      end

