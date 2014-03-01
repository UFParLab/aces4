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
      subroutine pack_coeffs_oed(alpha, ixalpha, pcoeff, ixpcoeff, 
     *                       ncfps, npfps, m, n,
     *                       alpha_pack, nalpha, pcoeff_pack, 
     *                       npcoeff, ccbeg, ccend, indx_cc,
     *                       ccbeg_pack, ccend_pack, 
     *                       maxpack)
c---------------------------------------------------------------------------
c   Formats the integral exponents and contraction coefficients for use
c   in the ERD integral package.
c---------------------------------------------------------------------------
      implicit none
c      include 'machine_types.h'
      
      integer ncfps(*), npfps(*), ixalpha(*), ixpcoeff(*)
      integer ccbeg(*), ccend(*), indx_cc(*)
      integer ccbeg_pack(*), ccend_pack(*)
      integer maxpack
      integer m, n
      integer nalpha, npcoeff
      integer ialpha
      integer ipcoeff
      integer quad(2)
      double precision alpha(*), pcoeff(*)
      double precision alpha_pack(*), pcoeff_pack(*)
      integer i, j, num, ishell
      integer k, l, icc

      quad(1) = m
      quad(2) = n

      ialpha = 1
      ipcoeff = 1
      nalpha = 0
      npcoeff = 0
      do j = 1, 2 
         ishell = quad(j)
         do i = 1, npfps(ishell)
            alpha_pack(ialpha+i-1) = alpha(ixalpha(ishell)+i-1)
         enddo

         ialpha = ialpha + npfps(ishell)
         nalpha = nalpha + npfps(ishell)
         if (nalpha .gt. maxpack) then
            print *,'Error: Pack buffer overflow, shell ',
     *           quad(1), quad(2),' nalpha ',nalpha,
     *           ' maxpack ',maxpack  
            call abort_job()
         endif
      enddo
  
      icc = 1
      do j = 1, 2
         ishell = quad(j)
         num    = npfps(ishell)*ncfps(ishell)
 
         do i = 1, num
            pcoeff_pack(ipcoeff+i-1) = pcoeff(ixpcoeff(ishell)+i-1)
         enddo

         ipcoeff = ipcoeff + num
         npcoeff = npcoeff + num
         if (npcoeff .gt. maxpack) then
            print *,'Error: pcoeff buffer overflow shell ',
     *            quad(1),quad(2)
            call abort_job()
         endif  
      enddo

         icc = 1
         do j = 1, 2
            ishell = quad(j)
            if (icc .gt. maxpack) then
               print *,'Error : ccbeg, ccend overflow shell ',
     *              quad(1),quad(2)
               call abort_job() 
            endif 

            do k = 1, ncfps(ishell)
               ccbeg_pack(icc) = ccbeg(indx_cc(ishell)+k-1)
               ccend_pack(icc) = ccend(indx_cc(ishell)+k-1)
               icc = icc + 1
            enddo
         enddo

      return
      end  
