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
      subroutine compute_return_2el_grdints(
     *
     *                             integrals,
     *                             a1,a2,b1,b2,c1,c2,d1,d2,
     *                             scr, iscr, 
     *                             
     *                             coords, tcoords, 
     *
     *                             der_flags)
c---------------------------------------------------------------------------
!>   The block of integrals (a1:a2,b1:b2,c1:c2,d1:d2) is computed for the 
!>   following 'types' of integrals based on atomic labels.
c---------------------------------------------------------------------------

      use, intrinsic :: ISO_C_BINDING 
      implicit none
c---------------------------------------------------------------------------
c Interface added but should be included instead 

      include 'sip_interface.f'
      include 'imemory.h'
c---------------------------------------------------------------------------

      integer a1, a2, b1, b2, c1, c2, d1, d2 
      integer aa1,aa2,bb1,bb2,cc1,cc2,dd1,dd2
      integer adim, bdim, cdim, ddim  
      integer m1, m2, n1, n2, r1, r2, s1, s2
      integer i, j, n, m, r, s
      integer a,b,c,d
      integer der_flags(12)
      integer der_save(12)
      integer myder(4,3), mySder(4,3)

      integer nints 
      integer nalpha_pack, npcoeff_pack
      integer ncsum, next, nfirst
      integer ncder, k, wder 

      logical spherical

      double precision x1,y1,z1
      double precision x2,y2,z2
      double precision x3,y3,z3
      double precision x4,y4,z4
      double precision coords(3,*)
      double precision tcoords(3,*)

      double precision integrals(a1:a2,b1:b2,c1:c2,d1:d2) 
      double precision sum 

      double precision scr(*)   
      integer iscr(*)

      integer max_dim_coeff
      parameter (max_dim_coeff = 5000)
      integer ccbeg_pack(max_dim_coeff), ccend_pack(max_dim_coeff)
      double precision alpha_pack(max_dim_coeff), 
     *                 pcoeff_pack(max_dim_coeff)

      integer one 
      integer two
      integer (C_INT) intSpherical, nshells, ncenters

! one dimensional arrays containg ccbeg and ccend
C
c ----------------------------------------------------------------------
c
      TYPE(C_PTR) :: icc, ccbeg, ccend
      integer, pointer :: picc(:), pccbeg(:), pccend(:)

! one dimensional array containg alphas(exponents)
C
c ----------------------------------------------------------------------
c
      TYPE(C_PTR) :: ntot_alpha, ixalphas
      integer, pointer :: pntot_alpha(:), pixalphas(:)
      TYPE(C_PTR) :: alphas
      real(C_DOUBLE) , pointer :: palphas(:)

!  one dimensional array containing coefficients
C
c ----------------------------------------------------------------------
c
      TYPE(C_PTR) :: ntot_pcoeff, ixpcoeffs
      TYPE(C_PTR) :: pcoeffs
      integer, pointer :: pntot_pcoeff(:), pixpcoeffs(:)
      real(C_DOUBLE), pointer :: ppcoeffs(:)
C
c ----------------------------------------------------------------------
c
c  END_NFPS 
      TYPE(C_PTR) :: pnalpha, pend_nfps 
      integer, pointer :: nalpha(:) 
      integer, pointer :: end_nfps(:) 
c ----------------------------------------------------------------------

c ----------------------------------------------------------------------
c  NCFPS 
      TYPE(C_PTR) :: pishells, pncfps 
      integer, pointer :: ishells(:) 
      integer, pointer :: ncfps(:) 
c ----------------------------------------------------------------------

c ----------------------------------------------------------------------
c  NPFPS 
      TYPE(C_PTR) :: pnpfps 
      integer, pointer :: npfps(:) 
c ----------------------------------------------------------------------

c ----------------------------------------------------------------------
c  NPFPS 
      TYPE(C_PTR) :: pindex_cc  
      integer, pointer :: index_cc(:) 
c ----------------------------------------------------------------------

c ----------------------------------------------------------------------
c  INDX_CC   
      TYPE(C_PTR) :: pindx_cc    
      integer, pointer :: indx_cc(:) 
c ----------------------------------------------------------------------

c ----------------------------------------------------------------------
c  IVANGMOM  
      TYPE(C_PTR) :: pivangmom   
      integer, pointer :: ivangmom(:) 
c ----------------------------------------------------------------------
c ----------------------------------------------------------------------
c Mapping of atoms to shells

      TYPE(C_PTR) :: patom
      integer (C_INT), pointer :: atom(:)
C ---------------------------------------------------------------------

      adim = a2-a1+1
      bdim = b2-b1+1
      cdim = c2-c1+1
      ddim = d2-d1+1 

      one = 1 
      two = 2

c Get the predefined data

c ----------------------------------------------------------------------
c  CC_BEG
      call predefined_int_array("ccbeg"//C_NULL_CHAR, one, icc,
     *                                   ccbeg)
      call c_f_pointer(icc, picc,[1])
      call c_f_pointer(ccbeg,pccbeg,picc)
c     write(6,*) ' CC_BEG', picc(1), (pccbeg(i), i=1,
c    *                                           picc(1))
c ----------------------------------------------------------------------

c ----------------------------------------------------------------------
c  CC_END
      call predefined_int_array("ccend"//C_NULL_CHAR, one, icc,
     *                                   ccend)
      call c_f_pointer(icc, picc,[1])
      call c_f_pointer(ccend,pccend,picc)
c     write(6,*) ' CC_END', picc(1), (pccend(i), i=1,
c    *                                           picc(1))
c ----------------------------------------------------------------------

c ----------------------------------------------------------------------
c  IXALPHAS(exponents)
      call predefined_int_array("ixalphas"//C_NULL_CHAR, one,
     *                                   ntot_alpha, ixalphas)
      call c_f_pointer(ntot_alpha, pntot_alpha,[1])
      call c_f_pointer(ixalphas, pixalphas, pntot_alpha)
c     write(6,*) ' IXALPHAS', pntot_alpha(1), (pixalphas(i), i=1,
c    *                                         pntot_alpha(1))
c ----------------------------------------------------------------------

c ----------------------------------------------------------------------
c  IXCOEFFICIENTS
      call predefined_int_array("ixpcoeffs"//C_NULL_CHAR,one,
     *                                   ntot_pcoeff, ixpcoeffs)
      call c_f_pointer(ntot_pcoeff, pntot_pcoeff,[1])
      call c_f_pointer(ixpcoeffs, pixpcoeffs, pntot_pcoeff)
c     write(6,*) ' IXPCOEFFS', pntot_pcoeff(1), (pixpcoeffs(i), i=1,
c    *                                         pntot_pcoeff(1))
c ----------------------------------------------------------------------

c ----------------------------------------------------------------------
c  ALPHAS(exponents)
      call predefined_scalar_array("alphas"//C_NULL_CHAR,
     *                              one, ntot_alpha, alphas)
      call c_f_pointer(ntot_alpha, pntot_alpha,[1])
      call c_f_pointer(alphas, palphas, pntot_alpha)
c     write(6,*) ' ALPHAS', pntot_alpha(1) , (palphas(i), i=1,
c    *                pntot_alpha(1))
c ----------------------------------------------------------------------

c ----------------------------------------------------------------------
c  COEFFICIENTS
      call predefined_scalar_array("pcoeffs"//C_NULL_CHAR,
     *                               one, ntot_pcoeff,pcoeffs)
      call c_f_pointer(ntot_pcoeff, pntot_pcoeff,[1])
      call c_f_pointer(pcoeffs, ppcoeffs, pntot_pcoeff)
c     write(6,*) ' PCOEFFS', pntot_pcoeff(1) , (ppcoeffs(i), i=1,
c    *                pntot_pcoeff(1))
c ----------------------------------------------------------------------

c ----------------------------------------------------------------------
c END_NFPS 
      call predefined_int_array("end_nfps"//C_NULL_CHAR, one, 
     *                                   pnalpha, pend_nfps) 
      call c_f_pointer(pnalpha, nalpha, [1]) 
      call c_f_pointer(pend_nfps, end_nfps, nalpha) 
c     write(6,*) ' END_NFPS', nalpha(1), (end_nfps(i), i=1, 
c    *                                           nalpha(1))  
c ----------------------------------------------------------------------

c ----------------------------------------------------------------------
c NCFPS 
      call predefined_int_array("ncfps"//C_NULL_CHAR, one, 
     *                                   pishells, pncfps) 
      call c_f_pointer(pishells, ishells, [1]) 
      call c_f_pointer(pncfps, ncfps, ishells) 
c     write(6,*) ' NCFPS', ishells(1), (ncfps(i), i=1, 
c    *                                           ishells(1))  
c ----------------------------------------------------------------------

c ----------------------------------------------------------------------
c NPFPS 
      call predefined_int_array("npfps"//C_NULL_CHAR, one, 
     *                                   pishells, pnpfps) 
      call c_f_pointer(pishells, ishells, [1]) 
      call c_f_pointer(pnpfps, npfps, ishells) 
c     write(6,*) ' NCFPS', ishells(1), (npfps(i), i=1, 
c    *                                           ishells(1))  
c ----------------------------------------------------------------------

c ----------------------------------------------------------------------
c INDX_CC  
      call predefined_int_array("indx_cc"//C_NULL_CHAR, one, 
     *                                   pishells, pindx_cc) 
      call c_f_pointer(pishells, ishells, [1]) 
      call c_f_pointer(pindx_cc, indx_cc, ishells) 
c     write(6,*) ' INDX_CC', ishells(1), (indx_cc(i), i=1, 
c    *                                           ishells(1))  
c ----------------------------------------------------------------------

c ----------------------------------------------------------------------
c IVANGMOMO   
      call predefined_int_array("ivangmom"//C_NULL_CHAR, one, pishells, 
     *                                   pivangmom) 
      call c_f_pointer(pishells, ishells, [1]) 
      call c_f_pointer(pivangmom, ivangmom, ishells) 
c     write(6,*) ' IVANGMOMOM', ishells(1), (ivangmom(i), i=1, 
c    *                                           ishells(1))  
c ----------------------------------------------------------------------
c ----------------------------------------------------------------------
c  mapping of atoms to shells
      call predefined_int_array("atom"//C_NULL_CHAR, one,
     *                                   pnalpha, patom)
      call c_f_pointer(pnalpha, nalpha, [1])
      call c_f_pointer(patom, atom, nalpha)
C----------------------------------------------------------------------
      nshells = int_constant("nshells"//C_NULL_CHAR)

      intSpherical = int_constant("intspherical"//C_NULL_CHAR)
      spherical = .true.
      if (intSpherical .ne. 1) spherical = .false. 

#ifdef _DEBUG_LVL00
      Write(6,"(a,8i4)") " a1,a2,b1,b2,c1,c2,d1,d2",
     &                   a1,a2,b1,b2,c1,c2,d1,d2 
      Write(6,"(a,i4)") " The number of shells: ", nshells
      Write(6,"(a,i4)") " The number of atoms: ", ncenters
      Write(6,"(a,i4)") " The number primitive (exponents).: ",
     *                    nalpha(1)
      Write(6,"(a,l)") " The spherical flags.:",  spherical
      Write(6,"(a)") " The end_nfps array"
      Write(6,"(10i4)") (end_nfps(i), i=1,nalpha(1))
      write(*,*)
      Write(6,"(a)") " The npfps array"
      Write(6,"(10i4)") (npfps(i), i=1,nshells)
      write(*,*)
      Write(6,"(a)") " The ncfps array"
      Write(6,"(10i4)") (ncfps(i), i=1,nshells)
      write(*,*)
      Write(6,"(a)") " The indx_cc array"
      Write(6,"(10i4)") (indx_cc(i), i=1,nshells)
      write(*,*)
      Write(6,"(a)") " The angular momentum array"
      Write(6,"(10i4)") (ivangmom(i), i=1,nshells)
      write(*,*)
      Write(6,"(a)") " The ixalpha array"
      Write(6,"(10I4)") (pixalphas(i), i=1,pntot_alpha(1))
      write(*,*)
      Write(6,"(a)") " The ixpcoef array"
      Write(6,"(10I4)") (pixpcoeffs(i),i=1,pntot_pcoeff(1))
      Write(6,"(a,i4)") " The exponents array"
      Write(6,"(5F15.7)") (palphas(i), i=1,pntot_alpha(1))
      Write(6,"(a)") " The contraction coeffs. array"
      Write(6,"(5F15.7)") (ppcoeffs(i), i=1,pntot_pcoeff(1))
      write(*,*)
      Write(6,"(a)") " The cc_beg array"
      Write(6,"(10i4)") (pccbeg(i), i=1,picc(1))
      Write(6,"(a)") " The cc_end array"
      Write(6,"(10i4)") (pccend(i), i=1,picc(1))
      Write(6,"(a)") " map of atoms to shell"
      Write(6,"(10i4)") (atom(i), i=1,nalpha(1))
#endif

      do d = d1, d2 
      do c = c1, c2 
      do b = b1, b2 
      do a = a1, a2 
         integrals(a,b,c,d) = 0.0 
      enddo 
      enddo 
      enddo 
      enddo 

      do a = 1, 12
         der_save(a) = der_flags(a)
      enddo
c-----------------------------------------------------------------------
c   Find the shell blocks for which we shall loop through.
c-----------------------------------------------------------------------

      call lookup_shell(end_nfps, nshells, a1, m1)
      call lookup_shell(end_nfps, nshells, a2, m2)
      call lookup_shell(end_nfps, nshells, b1, n1)
      call lookup_shell(end_nfps, nshells, b2, n2)
      call lookup_shell(end_nfps, nshells, c1, r1)
      call lookup_shell(end_nfps, nshells, c2, r2)
      call lookup_shell(end_nfps, nshells, d1, s1)
      call lookup_shell(end_nfps, nshells, d2, s2)

      do m = m1, m2
         if (m .eq. 1) then
            aa1 = 1
         else
            aa1 = end_nfps(m-1) + 1
         endif
         aa2 = end_nfps(m)

         x1 = tcoords(1,m)
         y1 = tcoords(2,m)
         z1 = tcoords(3,m)
      do n = n1, n2
         if (n .eq. 1) then
             bb1 = 1
         else
             bb1 = end_nfps(n-1) + 1
         endif
         bb2 = end_nfps(n)

         x2 = tcoords(1,n)
         y2 = tcoords(2,n)
         z2 = tcoords(3,n)
      do r = r1, r2
         if (r .eq. 1) then
            cc1 = 1
         else
            cc1 = end_nfps(r-1) + 1
         endif
         cc2 = end_nfps(r)

         x3 = tcoords(1,r)
         y3 = tcoords(2,r)
         z3 = tcoords(3,r)
      do s = s1, s2
         x4 = tcoords(1,s)
         y4 = tcoords(2,s)
         z4 = tcoords(3,s)
C
         call pack_coeffs(palphas, pixalphas, ppcoeffs, 
     *                    pixpcoeffs,ncfps, npfps, m, n,
     *                    r, s, alpha_pack, nalpha_pack,
     *                    pcoeff_pack, npcoeff_pack,
     *                    pccbeg, pccend, indx_cc,
     *                    ccbeg_pack, ccend_pack,max_dim_coeff)

c----------------------------------------------------------------------------
c   Make sure the der_flags are set correctly for equivalent centers.
c----------------------------------------------------------------------------

            do a = 1, 12
               der_flags(a) = der_save(a)
            enddo

            if (atom(m) .eq. atom(n)) then
                if (der_flags(1) .eq. 1) der_flags(4) = 1
                if (der_flags(4) .eq. 1) der_flags(1) = 1
                if (der_flags(2) .eq. 1) der_flags(5) = 1
                if (der_flags(5) .eq. 1) der_flags(2) = 1
                if (der_flags(3) .eq. 1) der_flags(6) = 1
                if (der_flags(6) .eq. 1) der_flags(3) = 1
            endif

            if (atom(m) .eq. atom(r)) then
                if (der_flags(1) .eq. 1) der_flags(7) = 1
                if (der_flags(7) .eq. 1) der_flags(1) = 1
                if (der_flags(2) .eq. 1) der_flags(8) = 1
                if (der_flags(8) .eq. 1) der_flags(2) = 1
                if (der_flags(3) .eq. 1) der_flags(9) = 1
                if (der_flags(9) .eq. 1) der_flags(3) = 1
            endif

            if (atom(m) .eq. atom(s)) then
                if (der_flags(1) .eq. 1) der_flags(10) = 1
                if (der_flags(10).eq. 1) der_flags(1)  = 1
                if (der_flags(2) .eq. 1) der_flags(11) = 1
                if (der_flags(11).eq. 1) der_flags(2)  = 1
                if (der_flags(3 ).eq. 1) der_flags(12) = 1
                if (der_flags(12).eq. 1) der_flags(3)  = 1
            endif

            if (atom(n) .eq. atom(r)) then
                if (der_flags(4) .eq. 1) der_flags(7) = 1
                if (der_flags(7) .eq. 1) der_flags(4) = 1
                if (der_flags(5) .eq. 1) der_flags(8) = 1
                if (der_flags(8) .eq. 1) der_flags(5) = 1
                if (der_flags(6) .eq. 1) der_flags(9) = 1
                if (der_flags(9) .eq. 1) der_flags(6) = 1
            endif

            if (atom(n) .eq. atom(s)) then
                if (der_flags(4) .eq. 1) der_flags(10) = 1
                if (der_flags(10) .eq. 1) der_flags(4) = 1
                if (der_flags(5) .eq. 1) der_flags(11) = 1
                if (der_flags(11) .eq. 1) der_flags(5) = 1
                if (der_flags(6) .eq. 1) der_flags(12) = 1
                if (der_flags(12) .eq. 1) der_flags(6) = 1
            endif

            if (atom(r) .eq. atom(s)) then
                if (der_flags(7) .eq. 1) der_flags(10) = 1
                if (der_flags(10) .eq. 1) der_flags(7) = 1
                if (der_flags(8) .eq. 1) der_flags(11) = 1
                if (der_flags(11) .eq. 1) der_flags(8) = 1
                if (der_flags(9) .eq. 1) der_flags(12) = 1
                if (der_flags(12) .eq. 1) der_flags(9) = 1
            endif

c---------------------------------------------------------------------------
c   Check if you want to calculate the integral batch.
c---------------------------------------------------------------------------

            ncder   = 0
            k       = 0
            wder    = 0
            nfirst  = 0
C
            do i = 1, 4
               do j = 1, 3
                  k = k + 1
                  myder(i,j)  = der_flags(k)
                  mySder(i,j) = der_save(k)
                  if (der_flags(k) .eq. 1) wder  = j
                  if (der_flags(k) .eq. 1) ncder = ncder + 1
                enddo
            enddo

            if (ncder .eq. 2) then
               do i = 1, 4
                  if (myder(i,wder) .ne. 0) then
                      nfirst = i
                      go to 10
                  endif
               enddo
10          continue

            if (mySder(nfirst,wder) .ne. 1) go to 777
            endif ! ncder .eq. 2

            if (ncder .eq. 4) then
                do i = 1, 4
                   if (myder(i,wder) .ne. 0) then
                      nfirst = i
                      go to 12
                   endif
               enddo
12          continue
            if (mySder(nfirst,wder) .ne. 1) go to 777
            endif ! ncder .eq. 4

            if (ncder .eq. 3) then
                do i = 1, 4
                   if (myder(i,wder) .ne. 0) then
                       nfirst = i
                       go to 11
                   endif
                enddo
11          continue

            if (mySder(nfirst,wder) .ne. 1) go to 777
            endif ! ncder .eq. 3

c---------------------------------------------------------------------------
c   Calling sequence for ERD version 2.
c---------------------------------------------------------------------------

            ncsum = ncfps(m) + ncfps(n) + ncfps(r) + ncfps(s)

#ifdef _DEBUG_LVL0
      Write(6,*) "@-entry to ERI_DERV_batch"
      Write(6,"((a,4(1x,i4)))") "m, n, r, s:", m, n, r, s
      Write(6,"(a,2(1x,I4))") "The Nalpha_pack and Npcoeff_pack:",
     &                         nalpha_pack, npcoeff_pack
      Write(6,"((a,4(1x,i4)))") "Ivanagmom:", Ivangmom(m),
     &      Ivangmom(n), Ivangmom(r), Ivangmom(s)
      Write(6,"((a,4(1x,i4)))") "Ncfps:", Ncfps(m),
     &      Ncfps(n), Ncfps(r), Ncfps(s)
      Write(6,"((a,4(1x,i4)))") "Npfps:", Npfps(m),
     &      Npfps(n), Npfps(r), Npfps(s)
      Write(6,*)
      Write(6,"(4(1x,F15.7))") (Alpha_pack(i),i=1,Nalpha_pack)
      Write(6,*)
      Write(6,"(4(1x,F15.7))")(Pcoeff_pack(i),i=1,npcoeff_pack)
      Write(6,*)
      Write(6,"(4(1x,I4))")(CCbeg_pack(i),i=1,Nalpha_pack)
      Write(6,*)
      Write(6,"(4(1x,I4))")(CCend_pack(i),i=1,Nalpha_pack)
      Write(6,*)
      Write(6,"(a,3(1x,I10))") "The imax,Zmax,ncsum:",imax, zmax,
     &                       ncsum
      Write(6,*) "The coordiantes"
      Write(6,*)
      Write(6,"(3(1x,F10.5))") x1, y1, z1
      Write(6,"(3(1x,F10.5))") x2, y2, z2
      Write(6,"(3(1x,F10.5))") x3, y3, z3
      Write(6,"(3(1x,F10.5))") x4, y4, z4
      Write(6,"(a)") "The derivative flags"
      Write(6,"(3I4)") der_flags(1), der_flags(2), der_flags(3)
      Write(6,"(3I4)") der_flags(4), der_flags(5), der_flags(6)
      Write(6,"(3I4)") der_flags(7), der_flags(8), der_flags(9)
      Write(6,"(3I4)") der_flags(10), der_flags(11), der_flags(12)

#endif 
            call ERD__GENER_ERI_DERV_BATCH(
     &                imax, zmax,
     &                nalpha_pack, npcoeff_pack, ncsum,
     &                ncfps(m),ncfps(n), ncfps(r), ncfps(s),
     &                npfps(m),npfps(n), npfps(r), npfps(s),
     &                ivangmom(m), ivangmom(n),
     &                ivangmom(r), ivangmom(s), x1,y1,z1,
     &                x2,y2,z2,x3,y3,z3,x4,y4,z4,
     &                der_flags(1), der_flags(2), der_flags(3),
     &                der_flags(4), der_flags(5), der_flags(6),
     &                der_flags(7), der_flags(8), der_flags(9),
     &                der_flags(10), der_flags(11), der_flags(12),
     &                alpha_pack,
     &                pcoeff_pack, ccbeg_pack, ccend_pack,
     &                spherical, .true., iscr, nints,
     &                nfirst, scr)

c---------------------------------------------------------------------------
c   Move the integrals into the output block.
c---------------------------------------------------------------------------

            if (nints .gt. 0) then

               if (s .eq. 1) then
                  dd1 = 1
               else
                  dd1 = end_nfps(s-1) + 1
               endif
               dd2 = end_nfps(s)

               call move_integrals(integrals, a1,a2,b1,b2,c1,c2,d1,d2,
     *                             scr(nfirst),
     *                             aa1,aa2,bb1,bb2,cc1,cc2,dd1,dd2)
            endif

777         continue

         enddo   ! s
         enddo   ! r

         enddo   ! n
         enddo   ! m

C#ifdef _DEBUG_LVL0
      sum = 0.0d0 
      do d = d1, d2
      do c = c1, c2
      do b = b1, b2
      do a = a1, a2
         sum = sum + integrals(a,b,c,d) * integrals(a,b,c,d)
      enddo
      enddo
      enddo
      enddo
      write(6,"(a,F20.10)")" The 2-el der. integral checksum =",
     &                     sum
Cendif 
      return
      end
