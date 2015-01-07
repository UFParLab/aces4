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
      subroutine erd_scratch_mem_calc(nshells, ncenters, 
     *                                npcoeff, nalphas,  
     *                                shtype, ncfps, npfps, 
     *                                atom, coords, alpha, pcoeff,
     *                                ixalpha, ixpcoeff, ccbeg, ccend,
     *                                indx_cc, spherical,
     *                                nuclei, calc_2der, imax, zmax, 
     *                                m1, m2)
c-------------------------------------------------------------------------
c   Calculates the maximum amount (in words) of scratch memory required
c   by the Flocke integral package.  
c
c   Arguments:
c	nshells			Number of shells 
c       shtype			Array of shell types.
c	ncfps			Number of contracted functions per shell.
c	npfps			Number of primitive functions per shell.
c       atom                    Array of atomic labels (one for each shell).
c       coords                  Array of coords per shell.
c       calc_2der               Logical variable set .true. if we need the
c                               2nd derivative integrals in this job..  
c	imax			Returned number of integer words required.
c	zmax 			Returned number of fl. pt. words required.
c       me                      Rank of current processor.
c       nprocs                  Number of processors.
c--------------------------------------------------------------------------

      use, intrinsic :: ISO_C_BINDING
      implicit none

c Interface added BUT should be include NOT added in the future

      include 'sip_interface.f'

C      integer (C_INT) nfrags 

      integer nshells, ncenters, npcoeff, nalphas   
      integer shtype(nshells), ncfps(nshells), npfps(nshells)
      double precision atom(3,ncenters)  
      integer satom(nshells) 
      integer ixalpha(nshells), ixpcoeff(nshells)
      integer ccbeg(*), ccend(*), indx_cc(*)
      integer imax, zmax
      integer nuclei, nalpha, ncoeff 

      logical calc_2der

      double precision coords(3,nshells)
      double precision alpha(*), pcoeff(*)
c     integer max_dim_coeff
c     parameter (max_dim_coeff = 5000)
c
c These 4 arrays pack the coefficients/exponents linearly with n_shell
c for each shell --> 2 or 4. (one_el two-el) integrals. They thus need
c at most 4*n. 
      double precision alpha_pack(4*nalphas), 
     *                 pcoeff_pack(4*npcoeff)
      integer ccbeg_pack(4*npcoeff), ccend_pack(4*npcoeff)

      integer i,j
      integer m, n, r, s
      integer iblk, zblk
      integer imin, zmin
      integer iopt, zopt
      integer ixderc
      integer der1x, der1y, der1z
      integer der2x, der2y, der2z
      integer dercx, dercy, dercz
      integer der_flags(12), iflag, jflag 
      integer lmax, ncmax, npmax
      integer m1, m2, mrange, mleft
      integer ihrr, zhrr 

      logical spherical

C      nfrags          = int_constant("nfrags"//C_NULL_CHAR)

      do m = 1, nshells 
      do i = 1, ncenters 
         if ((dabs(atom(1,i)-coords(1,m)) .lt. 1.0d-10) .and. 
     *       (dabs(atom(2,i)-coords(2,m)) .lt. 1.0d-10) .and. 
     *       (dabs(atom(3,i)-coords(3,m)) .lt. 1.0d-10))then 
             satom(m) = i 
         endif 
      enddo 
      enddo 

c     imax = 500000
c     zmax = 500000
c     return

c     imax = 5000000 
c     zmax = 5000000 
c     return 
      imax = 0 
      zmax = 0
      ihrr = 0 
      zhrr = 0 

      do m = m1, m2
       n = m
       r = m
       s = m
         call pack_coeffs(alpha, ixalpha, pcoeff, ixpcoeff,
     *                       ncfps, npfps, m, n, r, s,
     *                       alpha_pack, nalpha, pcoeff_pack,
     *                       ncoeff, ccbeg, ccend, indx_cc,
     *                       ccbeg_pack, ccend_pack) 
c    *                       nalphas, npcoeff)
c     write(6,*) ' NALPHA:', m, n, r, s, nalpha 

c-------------------------------------------------------------------------
c   ERD version 2
c-------------------------------------------------------------------------
c        write(6,*) 'SHTYPE:', shtype(m), shtype(n), shtype(r),
c    *   shtype(s)

            call ERD__MEMORY_ERI_BATCH(
     *                   nalpha, ncoeff,                 
     *                   ncfps(m), ncfps(n), ncfps(r), ncfps(s),
     *                   npfps(m), npfps(n), npfps(r), npfps(s),
     *                   shtype(m), shtype(n), shtype(r), shtype(s),
     *                   coords(1,m),coords(2,m),coords(3,m),
     *                   coords(1,n),coords(2,n),coords(3,n), 
     *                   coords(1,r),coords(2,r),coords(3,r), 
     *                   coords(1,s),coords(2,s),coords(3,s), 
     *                   alpha_pack, pcoeff_pack, spherical,
     *                   imin, iblk, zmin, zblk)
         imax = max0(imax, iblk)
         zmax = max0(zmax, zblk)
      enddo

C      if (nfrags .gt. 1) go to 999 

      go to 999 ! SKIP DERIVATIVE FOR NOW 

      do m = m1, m2
      do n = m, nshells
      do r = 1, nshells
      do s = r, nshells

c-------------------------------------------------------------------------
c   Derivative integrals.   Perform loops the same as they will be run
c   at execution time.
c-------------------------------------------------------------------------

c         do iflag = 1, 12
          do iflag = 1, 3

            do j = 1, 12
               der_flags(j) = 0
            enddo
            der_flags(iflag) = 1

               if (satom(m) .eq. satom(n)) then
                  if (der_flags(1) .eq. 1) der_flags(4) = 1
                  if (der_flags(4) .eq. 1) der_flags(1) = 1
                  if (der_flags(2) .eq. 1) der_flags(5) = 1
                  if (der_flags(5) .eq. 1) der_flags(2) = 1
                  if (der_flags(3) .eq. 1) der_flags(6) = 1
                  if (der_flags(6) .eq. 1) der_flags(3) = 1
               endif

               if (satom(m) .eq. satom(r)) then
                  if (der_flags(1) .eq. 1) der_flags(7) = 1
                  if (der_flags(7) .eq. 1) der_flags(1) = 1
                  if (der_flags(2) .eq. 1) der_flags(8) = 1
                  if (der_flags(8) .eq. 1) der_flags(2) = 1
                  if (der_flags(3) .eq. 1) der_flags(9) = 1
                  if (der_flags(9) .eq. 1) der_flags(3) = 1
               endif

               if (satom(m) .eq. satom(s)) then
                  if (der_flags(1) .eq. 1) der_flags(10) = 1
                  if (der_flags(10).eq. 1) der_flags(1)  = 1
                  if (der_flags(2) .eq. 1) der_flags(11) = 1
                  if (der_flags(11).eq. 1) der_flags(2)  = 1
                  if (der_flags(3 ).eq. 1) der_flags(12) = 1
                  if (der_flags(12).eq. 1) der_flags(3)  = 1
               endif
                   
               if (satom(n) .eq. satom(r)) then
                  if (der_flags(4) .eq. 1) der_flags(7) = 1
                  if (der_flags(7) .eq. 1) der_flags(4) = 1
                  if (der_flags(5) .eq. 1) der_flags(8) = 1
                  if (der_flags(8) .eq. 1) der_flags(5) = 1
                  if (der_flags(6) .eq. 1) der_flags(9) = 1
                  if (der_flags(9) .eq. 1) der_flags(6) = 1
               endif

               if (satom(n) .eq. satom(s)) then
                  if (der_flags(4) .eq. 1) der_flags(10) = 1
                  if (der_flags(10) .eq. 1) der_flags(4) = 1
                  if (der_flags(5) .eq. 1) der_flags(11) = 1
                  if (der_flags(11) .eq. 1) der_flags(5) = 1
                  if (der_flags(6) .eq. 1) der_flags(12) = 1
                  if (der_flags(12) .eq. 1) der_flags(6) = 1
               endif
                   
               if (satom(r) .eq. satom(s)) then
                  if (der_flags(7) .eq. 1) der_flags(10) = 1
                  if (der_flags(10) .eq. 1) der_flags(7) = 1
                  if (der_flags(8) .eq. 1) der_flags(11) = 1
                  if (der_flags(11) .eq. 1) der_flags(8) = 1
                  if (der_flags(9) .eq. 1) der_flags(12) = 1
                  if (der_flags(12) .eq. 1) der_flags(9) = 1
               endif

               call pack_coeffs(alpha, ixalpha, pcoeff, ixpcoeff,
     *                       ncfps, npfps, m, n, r, s,
     *                       alpha_pack, nalpha, pcoeff_pack,
     *                       ncoeff, ccbeg, ccend, indx_cc,
     *                       ccbeg_pack, ccend_pack) 
c    *                       nalphas, npcoeff)
c     write(6,*) ' NALPHA:', m, n, r, s, nalpha 

c--------------------------------------------------------------------------
c   Set the derivative flag arguments.
c--------------------------------------------------------------------------
 
               call erd__memory_eri_derv_batch(nalpha, ncoeff, 
     *                 ncfps(m), ncfps(n), ncfps(r), ncfps(s),
     *                 npfps(m), npfps(n), npfps(r), npfps(s),
     *                 shtype(m), shtype(n), shtype(r), shtype(s),
     *                   coords(1,m),coords(2,m),coords(3,m),
     *                   coords(1,n),coords(2,n),coords(3,n),
     *                   coords(1,r),coords(2,r),coords(3,r),
     *                   coords(1,s),coords(2,s),coords(3,s),
     *                 der_flags(1), der_flags(2), der_flags(3),
     *                 der_flags(4), der_flags(5), der_flags(6),
     *                 der_flags(7), der_flags(8), der_flags(9),
     *                 der_flags(10),der_flags(11), der_flags(12),    
     *                   alpha_pack, pcoeff_pack, spherical,
     *                   imin, iblk, zmin, zblk)   
            imax = max0(imax, iblk)
            zmax = max0(zmax, zblk)

         enddo
 
c----------------------------------------------------------------------------
c   Second-derivative integrals. Loop through all possible flag combinations.  
c----------------------------------------------------------------------------

         if (.not. calc_2der) go to 2000
         do iflag = 1, 12
         do jflag = iflag, 12

            do j = 1, 12
               der_flags(j) = 0
            enddo

            der_flags(iflag) = 1
            der_flags(jflag) = der_flags(jflag) + 1

               if (satom(m) .eq. satom(n)) then
                  if (der_flags(1) .ne. 0) der_flags(4) = der_flags(1)
                  if (der_flags(4) .ne. 0) der_flags(1) = der_flags(4)
                  if (der_flags(2) .ne. 0) der_flags(5) = der_flags(2)
                  if (der_flags(5) .ne. 0) der_flags(2) = der_flags(5)
                  if (der_flags(3) .ne. 0) der_flags(6) = der_flags(3)
                  if (der_flags(6) .ne. 0) der_flags(3) = der_flags(6)
               endif

               if (satom(m) .eq. satom(r)) then
                  if (der_flags(1) .ne. 0) der_flags(7) = der_flags(1)
                  if (der_flags(7) .ne. 0) der_flags(1) = der_flags(7)
                  if (der_flags(2) .ne. 0) der_flags(8) = der_flags(2)
                  if (der_flags(8) .ne. 0) der_flags(2) = der_flags(8)
                  if (der_flags(3) .ne. 0) der_flags(9) = der_flags(3)
                  if (der_flags(9) .ne. 0) der_flags(3) = der_flags(9)
               endif

               if (satom(m) .eq. satom(s)) then
                  if (der_flags(1) .ne. 0) der_flags(10) = der_flags(1)
                  if (der_flags(10).ne. 0) der_flags(1)  = der_flags(10)
                  if (der_flags(2) .ne. 0) der_flags(11) = der_flags(2)
                  if (der_flags(11).ne. 0) der_flags(2)  = der_flags(11)
                  if (der_flags(3 ).ne. 0) der_flags(12) = der_flags(3)
                  if (der_flags(12).ne. 0) der_flags(3)  = der_flags(12)
               endif

               if (satom(n) .eq. satom(r)) then
                  if (der_flags(4) .ne. 0) der_flags(7) = der_flags(4)
                  if (der_flags(7) .ne. 0) der_flags(4) = der_flags(7)
                  if (der_flags(5) .ne. 0) der_flags(8) = der_flags(5)
                  if (der_flags(8) .ne. 0) der_flags(5) = der_flags(8)
                  if (der_flags(6) .ne. 0) der_flags(9) = der_flags(6)
                  if (der_flags(9) .ne. 0) der_flags(6) = der_flags(9)
               endif

               if (satom(n) .eq. satom(s)) then
                  if (der_flags(4)  .ne. 0) der_flags(10)= der_flags(4)
                  if (der_flags(10) .ne. 0) der_flags(4) = der_flags(10)
                  if (der_flags(5)  .ne. 0) der_flags(11)= der_flags(5)
                  if (der_flags(11) .ne. 0) der_flags(5) = der_flags(11)
                  if (der_flags(6)  .ne. 0) der_flags(12)= der_flags(6)
                  if (der_flags(12) .ne. 0) der_flags(6) = der_flags(12)
               endif

               if (satom(r) .eq. satom(s)) then
                  if (der_flags(7)  .ne. 0) der_flags(10)= der_flags(7)
                  if (der_flags(10) .ne. 0) der_flags(7) = der_flags(10)
                  if (der_flags(8)  .ne. 0) der_flags(11)= der_flags(8)
                  if (der_flags(11) .ne. 0) der_flags(8) = der_flags(11)
                  if (der_flags(9)  .ne. 0) der_flags(12)= der_flags(9)
                  if (der_flags(12) .ne. 0) der_flags(9) = der_flags(12)
               endif 

c--------------------------------------------------------------------------
c   Set the derivative flag arguments.
c--------------------------------------------------------------------------
 
               call erd__memory_eri_derv_batch(nalpha, ncoeff, 
     *                 ncfps(m), ncfps(n), ncfps(r), ncfps(s),
     *                 npfps(m), npfps(n), npfps(r), npfps(s),
     *                 shtype(m), shtype(n), shtype(r), shtype(s),
     *                   coords(1,m),coords(2,m),coords(3,m),
     *                   coords(1,n),coords(2,n),coords(3,n),
     *                   coords(1,r),coords(2,r),coords(3,r),
     *                   coords(1,s),coords(2,s),coords(3,s),
     *                 der_flags(1), der_flags(2), der_flags(3),
     *                 der_flags(4), der_flags(5), der_flags(6),
     *                 der_flags(7), der_flags(8), der_flags(9),
     *                 der_flags(10),der_flags(11), der_flags(12),    
     *                   alpha_pack, pcoeff_pack, spherical,
     *                   imin, iblk, zmin, zblk)   

            imax = max0(imax, iblk)
            zmax = max0(zmax, zblk)
         enddo
         enddo
 2000    continue

      enddo
      enddo
      enddo
      enddo

999   continue 

c------------------------------------------------------------------------
c   Now calculate the memory for the one-electron integrals.
c------------------------------------------------------------------------

      do m = m1, m2
      do n = 1, nshells
         call pack_coeffs_oed(alpha, ixalpha, pcoeff, ixpcoeff,
     *                        ncfps, npfps, m, n, 
     *                       alpha_pack, nalpha, pcoeff_pack,
     *                       ncoeff, ccbeg, ccend, indx_cc,
     *                       ccbeg_pack, ccend_pack,
     *                       npcoeff*4)
            call oed__memory_nai_batch(nalpha, ncoeff,
     *                   ncfps(m), ncfps(n), 
     *                   npfps(m), npfps(n), 
     *                   shtype(m), shtype(n),
     *                   coords(1,m),coords(2,m),coords(3,m),
     *                   coords(1,n),coords(2,n),coords(3,n),
     *                   nuclei, alpha_pack, pcoeff_pack, spherical, 
     *                   imin, iopt, zmin, zopt)

         imax = max0(iopt, imax)
         zmax = max0(zopt, zmax)

            call oed__memory_kin_batch(nalpha, ncoeff,
     *                    ncfps(m), ncfps(n), 
     *                    npfps(m), npfps(n), 
     *                    shtype(m), shtype(n),
     *                    coords(1,m),coords(2,m),coords(3,m),
     *                    coords(1,n),coords(2,n),coords(3,n),
     *                    alpha_pack, pcoeff_pack, spherical, 
     *                    imin, iopt, zmin, zopt)
         imax = max0(iopt, imax)
         zmax = max0(zopt, zmax)

            call oed__memory_ovl_batch(nalpha, ncoeff,
     *                      ncfps(m), ncfps(n), 
     *                      npfps(m), npfps(n), 
     *                      shtype(m), shtype(n),
     *                      coords(1,m),coords(2,m),coords(3,m),
     *                      coords(1,n),coords(2,n),coords(3,n),
     *                      alpha_pack, pcoeff_pack, spherical, 
     *                      imin, iopt, zmin, zopt)

         imax = max0(iopt, imax)
         zmax = max0(zopt, zmax)

         go to 1000
 
            call oed__memory_nai_derv_batch(nalpha, ncoeff,
     *                   ncfps(m), ncfps(n), 
     *                   npfps(m), npfps(n), 
     *                   shtype(m), shtype(n),
     *                   coords(1,m),coords(2,m),coords(3,m),
     *                   coords(1,n),coords(2,n),coords(3,n),
     *                   nuclei, ixderc, 
     *                   der1x, der1y, der1z,
     *                   der2x, der2y, der2z,
     *                   dercx, dercy, dercz,
     *                   alpha_pack, pcoeff_pack, spherical, 
     *                   imin, iopt, zmin, zopt)

         imax = max0(iopt, imax)
         zmax = max0(zopt, zmax)

            call oed__memory_kin_derv_batch(nalpha, ncoeff,
     *                   ncfps(m), ncfps(n), 
     *                   npfps(m), npfps(n), 
     *                   shtype(m), shtype(n),
     *                   coords(1,m),coords(2,m),coords(3,m),
     *                   coords(1,n),coords(2,n),coords(3,n),
     *                   der1x, der1y, der1z,
     *                   der2x, der2y, der2z,
     *                   alpha_pack, pcoeff_pack, spherical, 
     *                   imin, iopt, zmin, zopt)

         imax = max0(iopt, imax)
         zmax = max0(zopt, zmax)

            call oed__memory_ovl_derv_batch(nalpha, ncoeff,
     *                   ncfps(m), ncfps(n), 
     *                   npfps(m), npfps(n), 
     *                   shtype(m), shtype(n),
     *                   coords(1,m),coords(2,m),coords(3,m),
     *                   coords(1,n),coords(2,n),coords(3,n),
     *                   der1x, der1y, der1z,
     *                   der2x, der2y, der2z,
     *                   alpha_pack, pcoeff_pack, spherical, 
     *                   imin, iopt, zmin, zopt)

         imax = max0(iopt, imax)
         zmax = max0(zopt, zmax)

 1000    continue
      enddo
      enddo

      imax = 2*imax
      zmax = 2*zmax

c     write(6,*) ' IMAX = ', imax 
c     write(6,*) ' ZMAX = ', zmax 

c-------------------------------------------------------------------------
c   Add correction for memory needed in the HRR transformation.
c-------------------------------------------------------------------------

      call erd__memory_hrr_correction(shtype, nshells, 
     *                  spherical, ihrr, zhrr)
      imax = imax + ihrr
      zmax = zmax + zhrr  

      write(6,*) ' F_IMAX = ', ihrr 
      write(6,*) ' F_ZMAX = ', zhrr 

      return
      end
