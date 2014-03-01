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
      subroutine comp_return_ovl(z, da1, da2, db1, db2, ncenters, 
     *                           tcoords, zz, iscr) 
c---------------------------------------------------------------------------
c  Computes the set of OED integrals of types  
c
c   type = 1 --> NAI integrals
c          2 --> KIN integrals
c---------------------------------------------------------------------------

      use, intrinsic :: ISO_C_BINDING 
      implicit none
    
c---------------------------------------------------------------------------
c Interface added but should be included instead 

      include 'sip_interface.f' 

c---------------------------------------------------------------------------
      integer iscr(*) 
      integer intmax, zmax 

      integer da1, da2, db1, db2
      integer adim, bdim 
      double precision zsum(da1:da2, db1:db2)
      double precision z(*)  
      double precision zz(*)  

      integer i, ix, m, n, nfirst, nints, ncoeff, iatom, ncsum
      integer component
      integer a, b, a1,a2,b1,b2,lastao, igrad
      integer m1, m2, n1, n2
      integer der1x, der1y, der1z
      integer der2x, der2y, der2z
      integer dercx, dercy, dercz
      integer ixderc
      integer ind(2)
      integer ierr 
      integer two 

      integer flopcount
      logical spherical
      logical*8 l8true, l8spherical
   
      integer max_dim_coeff
      parameter (max_dim_coeff = 5000)
      integer ccbeg_pack(max_dim_coeff), ccend_pack(max_dim_coeff)
      integer nalpha_pack, ncoeff_pack 
      double precision alpha_pack(max_dim_coeff), 
     *                 pcoeff_pack(max_dim_coeff)
      double precision xn(ncenters), yn(ncenters), zn(ncenters)
      double precision x1,y1,z1,x2,y2,z2, y

      integer (C_INT) one  

c Nuclear coordinates 
c --------------------------------------------------------------------------- 
      integer ncenters 
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
c CCBEG/CCEND
      TYPE(C_PTR) :: icc, ccbeg, ccend  
      integer, pointer :: picc(:), pccbeg(:), pccend(:)

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
      call predefined_int_array("ivangmom"//C_NULL_CHAR, one, pishells, 
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

      intSpherical = int_constant("intspherical"//C_NULL_CHAR)
      spherical = .true.
      if (intSpherical .ne. 1) spherical = .false. 
c     write(6,*) ' Spherical = ', spherical 

      intmax = 100**4 ! VER BAD HACK  
      zmax = 2*intmax

c----------------------------------------------------------------------------
c   Clear the output array.
c----------------------------------------------------------------------------

c     ECP = iflags(71) 
      i = 0 
      do b = db1,db2
      do a = da1,da2
         i = i + 1 
         zsum(a,b) = 0.d0
         z(i) = 0.d0
c        IF (ECP .EQ. 1) z(a,b) = ecp_erd(a,b) 
      enddo
      enddo

c-------------------------------------------------------------------------
c   Find the shells we need to loop over for this block.
c--------------------------------------------------------------------------

      call lookup_shell(end_nfps, nshells, da1, m1)
      call lookup_shell(end_nfps, nshells, da2, m2)
      call lookup_shell(end_nfps, nshells, db1, n1)
      call lookup_shell(end_nfps, nshells, db2, n2)

c     write(6,*) ' M1 M2', m1, m2, da1, da2  
c     write(6,*) ' N1 N2', n1, n2, db1, db2  

c-------------------------------------------------------------------------
c   Calculate the integrals over the necessary shell blocks.
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

c           write(6,*) 'XYZ XYZ:', x1, y1, z1
c           write(6,*) 'XYZ XYZ:', x2,y2,z2
c           write(6,*) ' ' 

c Zero out the output array 

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

             i = 0 
             do b = b1,b2
             do a = a1,a2
                i = i + 1 
                z(i) = 0.0d0  
             enddo
             enddo

             call pack_coeffs_oed(palphas, pixalphas, ppcoeffs, 
     *                       pixpcoeffs, ncfps, npfps, m, n,
     *                       alpha_pack, nalpha_pack, pcoeff_pack,
     *                       ncoeff_pack, pccbeg, pccend, 
     *                       indx_cc,
     *                       ccbeg_pack, ccend_pack,
     *                       max_dim_coeff)

c      write(6,*) ' NALPHA_PACK:', nalpha_pack 
c      write(6,*) ' NCOEFF_PACK:', ncoeff_pack 
c ------------------------------------------------------------------------ 
c  KIN (type=2) integrals next.   
c ------------------------------------------------------------------------ 

              call oed__gener_ovl_batch(intmax, zmax, nalpha_pack,
     *                 ncoeff_pack,
     *                 ncsum, ncfps(m), ncfps(n), npfps(m),npfps(n),
     *                 ivangmom(m), ivangmom(n), x1,y1,z1,x2,y2,z2,
     *                 alpha_pack,
     *                 pcoeff_pack, ccbeg_pack, ccend_pack,
     *                 spherical, .true., iscr, nints, nfirst,
     *                 zz)

c           write(6,*) ' NINTS = ', nints 
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

                call add_integrals2(zsum, da1,da2,db1,db2,
     *                 zz(nfirst),a1,a2,b1,b2,1.0d0)

c Print out the integrals  

c               i = 0 
c               do b = b1,b2
c               do a = a1,a2
c                  i = i + 1 
c                  z(i) = zz(nfirst+i-1) 
c               enddo
c               enddo

             endif ! (nints .gt. 0) then

c Print out the integrals  

c     write(6,*) 'One electron integrals ' 

c     a2 = end_nfps(m)
c     if (m .eq. 1) then
c         a1 = 1 
c     else
c         a1 = end_nfps(m-1)+1
c     endif

c     b2 = end_nfps(n)
c     if (n .eq. 1) then
c         b1 = 1 
c     else
c         b1 = end_nfps(n-1)+1
c     endif

c               call add_integrals2(zsum, da1,da2,db1,db2,
c    *                 z(1),a1,a2,b1,b2,0.0d0)

c     i = 0 
c     do b = b1,b2
c     do a = a1,a2
c        i = i + 1 
c        write(6,*) ' A1 A2: B1 B2:', a1, a2, b1, b2
c        zsum(a,b) = z(i) 
c        write(6,*) a, b, z(i)
c     enddo
c     enddo

        enddo   ! n shells
        enddo   ! m shells 

c Print out the integrals  

c     write(6,*) 'One electron integrals ' 
      i = 0 
      do b = db1,db2
      do a = da1,da2
         i = i + 1 
         z(i) = zsum(a,b) 
c        write(6,*) a, b, zsum(a,b)
      enddo
      enddo
c     STOP 
      return
      end
