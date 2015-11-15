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
C
      subroutine compute_return_nai_hessints(
     &
     &                              der_ints, density, hessian,

     &                              da1, da2, db1, db2,
     &                              dc1, dc2, dd1, dd2,
     & 
     &                              ncenters, coords, tcoords,
     &
     &                              scr,iscr)
c---------------------------------------------------------------------------
c  Computes the set of OED derivative of nuclear attraction integrals matching 
c  and contracts them with density block (da1:da2, db1:db2).
c---------------------------------------------------------------------------

      use, intrinsic :: ISO_C_BINDING
      implicit none

      include 'sip_interface.f'
      include 'imemory.h'
c---------------------------------------------------------------------------
      integer type
      integer da1, da2, db1, db2, dc1, dc2, dd1, dd2
      double precision der_ints(da1:da2, db1:db2)
      double precision density(dc1:dc2, dd1:dd2)
      double precision hessian(3*ncenters,3*ncenters)
      double precision scr(*) 
      integer iscr(*)
      Integer (C_INT) one

      integer i, ix, m, n, nfirst, nints
      integer ncoeff, iatom, jatom, ncsum
      integer component, icomponent, jcomponent
      integer a, b, a1,a2,b1,b2,lastao, igrad
      integer m1, m2, n1, n2
      integer der1x, der1y, der1z
      integer der2x, der2y, der2z
      integer dercx, dercy, dercz
      integer ixderc, jxderc
      integer ihess, jhess 
      integer ind(2)
      integer flopcount 
      logical spherical
   
      integer max_dim_coeff
      parameter (max_dim_coeff = 5000)
      integer ccbeg_pack(max_dim_coeff), ccend_pack(max_dim_coeff)
      double precision alpha_pack(max_dim_coeff), 
     *                 pcoeff_pack(max_dim_coeff)
      double precision xn(ncenters), yn(ncenters), 
     *                 zn(ncenters)
      double precision hess
      double precision x1,y1,z1,x2,y2,z2, y
      double precision sumxx, sumxy, sumxz
      double precision sumyx, sumyy, sumyz
      double precision sumzx, sumzy, sumzz

      integer nalpha_pack, ncoeff_pack
c
c Nuclear coordinates
c ---------------------------------------------------------------------------
      integer ncenters
      double precision coords(3,ncenters)
      double precision tcoords(3,*)
c ---------------------------------------------------------------------------

      integer (C_INT) nshells, intSpherical
c ----------------------------------------------------------------------
c  END_NFPS
      TYPE(C_PTR) :: pnalpha, pend_nfps

      integer, pointer :: nalpha(:)
      integer, pointer :: end_nfps(:)
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
c  IVANGMOM
      TYPE(C_PTR) :: pivangmom
      integer, pointer :: ivangmom(:)
c ----------------------------------------------------------------------

c ----------------------------------------------------------------------
c  ALPHAS
      TYPE(C_PTR) :: ntot_alpha, ixalphas
      integer, pointer :: pntot_alpha(:), pixalphas(:)
      TYPE(C_PTR) :: alphas
      real(C_DOUBLE) , pointer :: palphas(:)
c ----------------------------------------------------------------------

c ----------------------------------------------------------------------
c  COEFFS
      TYPE(C_PTR) :: ntot_pcoeff, ixpcoeffs
      TYPE(C_PTR) :: pcoeffs
      integer, pointer :: pntot_pcoeff(:), pixpcoeffs(:)
      real(C_DOUBLE), pointer :: ppcoeffs(:)
c ----------------------------------------------------------------------

c ----------------------------------------------------------------------
c  CHARGES
      TYPE(C_PTR) :: charge, atoms
      integer, pointer :: patoms(:)
      real(C_DOUBLE), pointer :: pcharge(:)
c ----------------------------------------------------------------------
      TYPE(C_PTR) :: icc, ccbeg, ccend
      integer, pointer :: picc(:), pccbeg(:), pccend(:)
c----------------------------------------------------------------------
c Mapping of atoms to shells

      TYPE(C_PTR) :: patom
      integer (C_INT), pointer :: atom(:)
C ---------------------------------------------------------------------

c Get data needed to compute integrals
c ---------------------------------------------------------------------------
c  NSHELLS
       nshells = int_constant("nshells"//C_NULL_CHAR)

c ----------------------------------------------------------------------
c  END_NFPS
c ----------------------------------------------------------------------
      call predefined_int_array("end_nfps"//C_NULL_CHAR, one,
     *                                   pnalpha, pend_nfps)
      call c_f_pointer(pnalpha, nalpha, [1])
      call c_f_pointer(pend_nfps, end_nfps, nalpha)
c     write(6,*) ' END_NFPS', nalpha(1), (end_nfps(i), i=1,
c    *                                           nalpha(1))
c ----------------------------------------------------------------------
c  NCFPS
c ----------------------------------------------------------------------
      call predefined_int_array("ncfps"//C_NULL_CHAR, one,
     *                                   pishells, pncfps)
      call c_f_pointer(pishells, ishells, [1])
      call c_f_pointer(pncfps, ncfps, ishells)
c     write(6,*) ' NCFPS', ishells(1), (ncfps(i), i=1,
c    *                                           ishells(1))
c ----------------------------------------------------------------------
c  NPFPS
c ----------------------------------------------------------------------
      call predefined_int_array("npfps"//C_NULL_CHAR, one,
     *                                   pishells, pnpfps)
      call c_f_pointer(pishells, ishells, [1])
      call c_f_pointer(pnpfps, npfps, ishells)
c     write(6,*) ' NPFPS', ishells(1), (npfps(i), i=1,
c    *                                           ishells(1))
c ----------------------------------------------------------------------
c  INDX_CX
c ----------------------------------------------------------------------
      call predefined_int_array("indx_cc"//C_NULL_CHAR, one,
     *                                   pishells, pindx_cc)
      call c_f_pointer(pishells, ishells, [1])
      call c_f_pointer(pindx_cc, indx_cc, ishells)
c     write(6,*) ' INDX_CC', ishells(1), (indx_cc(i), i=1,
c    *                                           ishells(1))
c ----------------------------------------------------------------------
c  IVANGMOM
c ----------------------------------------------------------------------
      call predefined_int_array("ivangmom"//C_NULL_CHAR, one,pishells,
     *                                   pivangmom)
      call c_f_pointer(pishells, ishells, [1])
      call c_f_pointer(pivangmom, ivangmom, ishells)
c     write(6,*) ' IVANGMOMOM', ishells(1), (ivangmom(i), i=1,
c    *                                           ishells(1))

c ----------------------------------------------------------------------
c  IXALPHAS
c ----------------------------------------------------------------------
      call predefined_int_array("ixalphas"//C_NULL_CHAR, one,
     *                                   ntot_alpha, ixalphas)
      call c_f_pointer(ntot_alpha, pntot_alpha,[1])
      call c_f_pointer(ixalphas, pixalphas, pntot_alpha)
c     write(6,*) ' IXALPHAS', pntot_alpha(1), (pixalphas(i), i=1,
c    *                                         pntot_alpha(1))

c ----------------------------------------------------------------------
c  IXPCOEFFS
c ----------------------------------------------------------------------
      call predefined_int_array("ixpcoeffs"//C_NULL_CHAR,one,
     *                                   ntot_pcoeff, ixpcoeffs)
      call c_f_pointer(ntot_pcoeff, pntot_pcoeff,[1])
      call c_f_pointer(ixpcoeffs, pixpcoeffs, pntot_pcoeff)
c     write(6,*) ' IXPCOEFFS', pntot_pcoeff(1), (pixpcoeffs(i), i=1,
c    *                                         pntot_pcoeff(1))

c ----------------------------------------------------------------------
c  ALPHAS
c ----------------------------------------------------------------------
      call predefined_scalar_array("alphas"//C_NULL_CHAR,
     *                              one, ntot_alpha, alphas)
      call c_f_pointer(ntot_alpha, pntot_alpha,[1])
      call c_f_pointer(alphas, palphas, pntot_alpha)
c     write(6,*) ' ALPHAS', pntot_alpha(1) , (palphas(i), i=1,
c    *                pntot_alpha(1))

c ----------------------------------------------------------------------
c  PCOEFFS
c ----------------------------------------------------------------------
      call predefined_scalar_array("pcoeffs"//C_NULL_CHAR,
     *                               one, ntot_pcoeff,pcoeffs)
      call c_f_pointer(ntot_pcoeff, pntot_pcoeff,[1])
      call c_f_pointer(pcoeffs, ppcoeffs, pntot_pcoeff)
c     write(6,*) ' PCOEFFS', pntot_pcoeff(1) , (ppcoeffs(i), i=1,
c    *                pntot_pcoeff(1))
c ----------------------------------------------------------------------
c  CCBEG
c ----------------------------------------------------------------------
      call predefined_int_array("ccbeg"//C_NULL_CHAR, one, icc,
     *                                   ccbeg)
      call c_f_pointer(icc, picc,[1])
      call c_f_pointer(ccbeg,pccbeg,picc)
c     write(6,*) ' CC_BEG', picc(1), (pccbeg(i), i=1,
c    *                                           picc(1))

c ----------------------------------------------------------------------
c  CCEND
c ----------------------------------------------------------------------
      call predefined_int_array("ccend"//C_NULL_CHAR, one, icc,
     *                                   ccend)
      call c_f_pointer(icc, picc,[1])
      call c_f_pointer(ccend,pccend,picc)
c     write(6,*) ' CC_END', picc(1), (pccend(i), i=1,
c    *                                           picc(1))

c ----------------------------------------------------------------------
c  NUCLEAR CHARGES
c ----------------------------------------------------------------------
      call predefined_scalar_array("charge"//C_NULL_CHAR,
     *                               one, atoms ,charge)
      call c_f_pointer(atoms, patoms,[1])
      call c_f_pointer(charge, pcharge, patoms)
c     write(6,*) ' CHARGEs', patoms(1) , (pcharge(i), i=1,
c    *                patoms(1))
c ----------------------------------------------------------------------
c  mapping of atoms to shells
      call predefined_int_array("atom"//C_NULL_CHAR, one,
     *                                   pnalpha, patom)
      call c_f_pointer(pnalpha, nalpha, [1])
      call c_f_pointer(patom, atom, nalpha)
C----------------------------------------------------------------------

      intSpherical = int_constant("intspherical"//C_NULL_CHAR)
      spherical = .true.
      if (intSpherical .ne. 1) spherical = .false.

C#ifdef _DEBUG_LVL0
      Write(6,"(a,8i4)") " da1,da2,db1,db2,dc1,dc2,dd1,dd2",
     &                    da1,da2,db1,db2,dc1,dc2,dd1,dd2
      Write(6,"(a,i4)") " The number of shells: ", nshells
      Write(6,"(a,i4)") " The number of atoms: ", ncenters
      Write(6,"(a,i4)") " The number primitive (exponents).: ",
     *                    nalpha(1)
      Write(6,"(a,l)") " The spherical flags.:",  spherical
      Write(6,"(a)") " The end_nfps array"
      Write(6,"(10i4)") (end_nfps(i), i=1,nalpha(1))
      write(*,*)
      Write(6,"(a)") " The atom charges"
      Write(6,"(5F6.3)") (pcharge(i), i=1,ncenters)
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
C#endif
C#ifdef _DEBUG_LVL0
      sumxx=0.0d0
      sumxy=0.0d0
      sumxz=0.0d0
      sumyx=0.0d0
      sumyy=0.0d0
      sumyz=0.0d0
      sumzx=0.0d0
      sumzy=0.0d0
      sumzz=0.0d0
C#endif

      Ind(1) = 1
      Ind(2) = 2
C
c-------------------------------------------------------------------------
c Find the shells we need to loop over for this block of density data.
c--------------------------------------------------------------------------
C
      call lookup_shell(end_nfps, nshells, da1, m1)
      call lookup_shell(end_nfps, nshells, da2, m2)
      call lookup_shell(end_nfps, nshells, db1, n1)
      call lookup_shell(end_nfps, nshells, db2, n2)
      
c-------------------------------------------------------------------------
c Build an array of coordinates per atom.
c-------------------------------------------------------------------------
      do m = 1, ncenters
         xn(m) = coords(1,m)
         yn(m) = coords(2,m)
         zn(m) = coords(3,m)
      enddo

c-------------------------------------------------------------------------
c Calculate the integrals over the necessary shell blocks.
c-------------------------------------------------------------------------

       do m = m1, m2

          x1 = tcoords(1,m)
          y1 = tcoords(2,m)
          z1 = tcoords(3,m)

       do n = n1, n2

            ncsum = ncfps(m) + ncfps(n) 

            x2 = tcoords(1,n)
            y2 = tcoords(2,n)
            z2 = tcoords(3,n)

            call pack_coeffs_oed(palphas, pixalphas, ppcoeffs,
     &                       pixpcoeffs, ncfps, npfps, m, n,
     &                       alpha_pack, nalpha_pack, pcoeff_pack,
     &                       ncoeff_pack, pccbeg, pccend,
     &                       indx_cc,
     &                       ccbeg_pack, ccend_pack,
     &                       max_dim_coeff)

c loop over derivatives on centers 
    
            do ixderc = 1, ncenters 

               iatom = ixderc 
               do icomponent = 1, 3   ! component is x, y, or z term of gradient
               do jcomponent = 1, 3   ! component is x, y, or z term of gradient
  
c-------------------------------------------------------------------------
c   Clear the integral block.
c-------------------------------------------------------------------------

               do b = db1, db2
               do a = da1, da2
                  der_ints(a,b) = 0.0d0
               enddo
               enddo
c--------------------------------------------------------------------------

c-------------------------------------------------------------------------
c   Initialize the derivative array.
c-------------------------------------------------------------------------

               der1x = 0 
               der1y = 0 
               der1z = 0 
               der2x = 0 
               der2y = 0 
               der2z = 0 

               dercx  = 0 
               dercy  = 0 
               dercz  = 0 
C XX-derivatives

               If (icomponent.eq.1.and.jcomponent.eq.1) then

                  dercx = 2
                  If (ixderc .eq. atom(m) .and. 
     &                ixderc .eq. atom(n)) Then
                     der1x = 2
                     der2x = 2
                  Else
                     If (ixderc .eq. atom(m)) then
                        der1x = 2
                     Elseif (ixderc .eq. atom(n)) then
                        der2x = 2
                     Endif 
                  Endif
C XY-derivatives
                 
                Elseif (icomponent.eq.1.and.jcomponent.eq.2) then

                  dercx = 1
                  dercy = 1
                  If (ixderc .eq. atom(m) .and. 
     &                ixderc .eq. atom(n)) Then
                     der1x = 1 
                     der1y = 1
                     der2x = 1
                     der2y = 1
                  Else
                     If (ixderc .eq. atom(m)) then
                        der1x = 1
                        der1y = 1
                     Elseif (ixderc .eq. atom(n)) then
                        der2x = 1
                        der2y = 1
                     Endif
                  Endif 
  
C XZ-derivatives

                Elseif (icomponent.eq.1.and.jcomponent.eq.3) then

                  dercx = 1
                  dercz = 1
                  If (ixderc .eq. atom(m) .and. 
     &                ixderc .eq. atom(n)) Then
                     der1x = 1 
                     der1z = 1
                     der2x = 1
                     der2z = 1
                  Else
                     If (ixderc .eq. atom(m)) then
                        der1x = 1
                        der1z = 1
                     Elseif (ixderc .eq. atom(n)) then
                        der2x = 1
                        der2z = 1
                     Endif
                  Endif 
C YX-derivatives

                Elseif (icomponent.eq.2.and.jcomponent.eq.1) then

                  dercx = 1
                  dercy = 1
                  If (ixderc .eq. atom(m) .and.
     &                ixderc .eq. atom(n)) Then
                     der1x = 1
                     der1y = 1
                     der2x = 1
                     der2y = 1
                  Else 
                     If (ixderc .eq. atom(m)) then
                        der1x = 1
                        der1y = 1
                     Elseif (ixderc .eq. atom(n)) then
                        der2x = 1
                        der2y = 1
                     Endif
                  Endif
C YY-Derivatives

                Elseif (icomponent.eq.2.and.jcomponent.eq.2) then

                  dercy = 2
                  If (ixderc .eq. atom(m) .and. 
     &                ixderc .eq. atom(n)) Then
                     der1y = 1 
                     der2y = 1
                  Else
                     If (ixderc .eq. atom(m)) then
                        der1y = 2
                     Elseif (ixderc .eq. atom(n)) then
                        der2y = 2
                     Endif
                  Endif

C YZ-Derivatives

                Elseif (icomponent.eq.2.and.jcomponent.eq.3) then

                  dercy = 1
                  dercz = 1
                  If (ixderc .eq. atom(m) .and.
     &                ixderc .eq. atom(n)) Then
                     der1y = 1
                     der1z = 1
                     der2y = 1
                     der2z = 1
                  Else 
                     If (ixderc .eq. atom(m)) then
                        der1y = 1
                        der1z = 1
                     Elseif (ixderc .eq. atom(n)) then
                        der2y = 1
                        der2z = 1
                     Endif
                  Endif
C ZX-Derivatives

                Elseif (icomponent.eq.3.and.jcomponent.eq.1) then

                  dercx = 1
                  dercz = 1
                  If (ixderc .eq. atom(m) .and.
     &                ixderc .eq. atom(n)) Then
                     der1x = 1
                     der1z = 1
                     der2x = 1
                     der2z = 1
                  Else  
                     If (ixderc .eq. atom(m)) then
                        der1x = 1
                        der1z = 1
                     Elseif (ixderc .eq. atom(n)) then
                        der2x = 1
                        der2z = 1
                     Endif
                  Endif 

C ZY-Derivatives
                Elseif (icomponent.eq.3.and.jcomponent.eq.2) then

                  dercy = 1
                  dercz = 1
                  If (ixderc .eq. atom(m) .and.
     &                ixderc .eq. atom(n)) Then
                     der1y = 1
                     der1z = 1
                     der2y = 1
                     der2z = 1
                  Else
                     If (ixderc .eq. atom(m)) then
                        der1y = 1
                        der1z = 1
                     Elseif (ixderc .eq. atom(n)) then
                        der2y = 1
                        der2z = 1
                     Endif
                  Endif

C ZZ-Derivatives

                Elseif (icomponent.eq.3.and.jcomponent.eq.3) then

                  dercz = 2
                  If (ixderc .eq. atom(m) .and.
     &                ixderc .eq. atom(n)) Then
                     der1z = 1
                     der2z = 1
                  Else
                     If (ixderc .eq. atom(m)) then
                        der1z = 2
                     Elseif (ixderc .eq. atom(n)) then
                        der2z = 2
                     Endif 
                  Endif 

               Endif 
#ifdef _DEBUG_LVL0
      Write(6,*)
      write(6,"(a,3I2)") "dercx,dercy,dercz : ", dercx,dercy,dercz
      write(6,"(a,3I2)") "der1x,der1y,der1z : ", der1x,der1y,der1z
      write(6,"(a,3I2)") "der2x,der2y,der2z : ", der2x,der2y,der2z
#endif 

               call oed__gener_nai_derv_batch(imax,zmax,
     *                 nalpha_Pack,ncoeff_pack,
     *                 ncsum, ncfps(m), ncfps(n), npfps(m),npfps(n),
     *                 ivangmom(m), ivangmom(n), x1,y1,z1,x2,y2,z2,
     *                 ncenters, xn,yn,zn, pcharge, ixderc,
     *                 der1x, der1y, der1z,
     *                 der2x, der2y, der2z, 
     *                 dercx, dercy, dercz, alpha_pack,
     *                 pcoeff_pack, ccbeg_pack, ccend_pack,
     *                 spherical, .true., iscr, nints, nfirst, 
     *                 scr)     

               if (nints .gt. 0) then
                  a2 = end_nfps(m)
                  if (m .eq. 1) then
                     a1 = 1
                  else
                     a1 = end_nfps(m-1)+1
                  endif

                  b2 = end_nfps(n)
                  if (n .eq. 1) then
                     b1 = 1
                  else
                     b1 = end_nfps(n-1)+1
                  endif
                 
                 call move_integrals2(der_ints, da1,da2,db1,db2,
     *                 scr(nfirst),a1,a2,b1,b2)

c--------------------------------------------------------------------------
c   Contract the density block with the block of integrals.
c--------------------------------------------------------------------------

                 call cwork0222(hess,
     *                          density,dc1,dc2,dd1,dd2,
     *                          dc1,dc2,dd1,dd2,ind,
     *                          der_ints,da1,da2,db1,db2,
     *                          da1,da2,db1,db2,1,2,flopcount)

c---------------------------------------------------------------------------
c   Sum into the gradient.
c---------------------------------------------------------------------------
#ifdef _DEBUG_LVL0
       if (type .eq. 1) then
       Write(6,*)
       Write(6,"(a)") "The nuclear attraction gradient"
       Write(6,"(2I4,F15.10)") iatom, component, grad 
       write(6,"(3(F15.10))") (Gradient(i), i=1,3*ncenters)
       endif 
#endif
                 ihess = (iatom-1)*3 + icomponent
                 jhess = (jatom-1)*3 + jcomponent

                 hessian(jhess,ihess) = hessian(jhess,ihess) + hess

               endif ! nints 
C#ifdef _DEBUG_LVL0
      do b = da1, da2
      do a = db1, db2
         if (icomponent .eq. 1 .and. jcomponent .eq. 1)
     &       sumxx = sumxx + der_ints(a,b) *  der_ints(a,b)
         if (icomponent .eq. 1 .and. jcomponent .eq. 2)
     &       sumxy = sumxy + der_ints(a,b) *  der_ints(a,b)
         if (icomponent .eq. 1 .and. jcomponent .eq. 3)
     &       sumxz = sumxz + der_ints(a,b) *  der_ints(a,b)
         if (icomponent .eq. 2 .and. jcomponent .eq. 1)
     &       sumyx = sumyx + der_ints(a,b) *  der_ints(a,b)
         if (icomponent .eq. 2 .and. jcomponent .eq. 2)
     &       sumyy = sumyy + der_ints(a,b) *  der_ints(a,b)
         if (icomponent .eq. 2 .and. jcomponent .eq. 3)
     &       sumyz = sumyz + der_ints(a,b) *  der_ints(a,b)
         if (icomponent .eq. 3 .and. jcomponent .eq. 1)
     &       sumzx = sumzx + der_ints(a,b) *  der_ints(a,b)
         if (icomponent .eq. 3 .and. jcomponent .eq. 2)
     &       sumzy = sumzy + der_ints(a,b) *  der_ints(a,b)
         if (icomponent .eq. 3 .and. jcomponent .eq. 3)
     &       sumzz = sumzz + der_ints(a,b) *  der_ints(a,b)
      enddo
      enddo

C#endif
            enddo ! jcomponent 
            enddo ! icomponent 
            enddo ! ncenters 
     
         enddo   ! n shells
         enddo   ! m shells

C#ifdef _DEBUG_LVL0
      write(6,"(a,F20.10)")" The 1-el (nai) hes. integral checksumxx =",
     &sumxx
      write(6,"(a,F20.10)")" The 1-el (nai) hes. integral checksumxy =",
     &sumxy
      write(6,"(a,F20.10)")" The 1-el (nai) hes. integral checksumxz =",
     &sumxz
      write(6,"(a,F20.10)")" The 1-el (nai) hes. integral checksumyx =",
     &sumyx
      write(6,"(a,F20.10)")" The 1-el (nai) hes. integral checksumyy =",
     &sumyy
      write(6,"(a,F20.10)")" The 1-el (nai) hes. integral checksumyz =",
     &sumyz
      write(6,"(a,F20.10)")" The 1-el (nai) hes. integral checksumzx =",
     &sumzx
      write(6,"(a,F20.10)")" The 1-el (nai) hes. integral checksumzy =",
     &sumzy
      write(6,"(a,F20.10)")" The 1-el (nai) hes. integral checksumzz =",
     &sumzz
      write(6,*)
C#endif

      return
      end
