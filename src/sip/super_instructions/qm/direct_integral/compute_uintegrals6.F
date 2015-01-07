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
      subroutine compute_uintegrals6(a1,a2,b1,b2,c1,c2,d1,d2,aoint, 
     *                 scr, iscr, coords, tcoords ,coeffs, 
     *                 ixpcoef,alphas, 
     *                 ixalpha, ccbeg, ccend,
     *                 ca, cb, nc1,nc2, nd1, nd2,
     *                 fa,fb) 
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

      integer num_to_do
      integer nints 
      integer nalpha_pack, npcoeff_pack
      integer ncsum, next, nfirst
      integer me, ierr
      integer nc1, nc2, nd1, nd2 

      integer imin, zmin, iblk, zblk

      logical skip
      logical mn_symmetry
      logical rs_symmetry
      logical mn_rs_symmetry
      logical*8 l8true, l8spherical
      logical spherical

      double precision x1,y1,z1
      double precision x2,y2,z2
      double precision x3,y3,z3
      double precision x4,y4,z4

      double precision coords(3,*), coeffs(*), alphas(*)
      double precision tcoords(3,*)
      double precision ca(nc1:nc2,nd1:nd2)
      double precision cb(nc1:nc2,nd1:nd2)
      double precision fa(nc1:nc2,nc1:nc2)
      double precision fb(nc1:nc2,nc1:nc2)
      double precision integrals(a1:a2,b1:b2,c1:c2,d1:d2) 
      double precision aoint(*) 

      double precision scr(*)   
      integer iscr(*)

      integer ccbeg(*), ccend(*)
      integer ixalpha(*), ixpcoef(*) 

      integer max_dim_coeff
      parameter (max_dim_coeff = 5000)
      integer ccbeg_pack(max_dim_coeff), ccend_pack(max_dim_coeff)
      integer*8 ccbeg_pack64(max_dim_coeff), ccend_pack64(max_dim_coeff)
      double precision alpha_pack(max_dim_coeff), 
     *                 pcoeff_pack(max_dim_coeff)
      integer*8 arg64(25)

c VFL TO BE COMPLETED       common /Imax_com/sz_max(nshells,nshells), delta 
      double precision sz_max, delta
      double precision itol, bmax, dtemp, emax    

      common /d2int_com/jatom, jx, jcenter
      integer jatom, jx, jcenter  
      integer one 

      integer (C_INT) nshells, ncenters, nalpha_occupied, nbeta_occupied
      integer (C_INT) nalpha_virtual, nbeta_virtual
      integer (C_INT) intSpherical  


c ----------------------------------------------------------------------
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

      adim = a2-a1+1
      bdim = b2-b1+1
      cdim = c2-c1+1
      ddim = d2-d1+1 
      l8true = .true.
      jx = 2 
c     spherical = (ispherical .eq. 1)

c Get the predefined data 

      one = 1 

c ----------------------------------------------------------------------
c  END_NFPS 
      call predefined_int_array("end_nfps"//C_NULL_CHAR, one, 
     *                                   pnalpha, pend_nfps) 
      call c_f_pointer(pnalpha, nalpha, [1]) 
      call c_f_pointer(pend_nfps, end_nfps, nalpha) 
c     write(6,*) ' END_NFPS', nalpha(1), (end_nfps(i), i=1, 
c    *                                           nalpha(1))  
c ----------------------------------------------------------------------

c ----------------------------------------------------------------------
c  NCFPS 
      call predefined_int_array("ncfps"//C_NULL_CHAR, one, 
     *                                   pishells, pncfps) 
      call c_f_pointer(pishells, ishells, [1]) 
      call c_f_pointer(pncfps, ncfps, ishells) 
c     write(6,*) ' NCFPS', ishells(1), (ncfps(i), i=1, 
c    *                                           ishells(1))  
c ----------------------------------------------------------------------

c ----------------------------------------------------------------------
c  NPFPS 
      call predefined_int_array("npfps"//C_NULL_CHAR, one, 
     *                                   pishells, pnpfps) 
      call c_f_pointer(pishells, ishells, [1]) 
      call c_f_pointer(pnpfps, npfps, ishells) 
c     write(6,*) ' NCFPS', ishells(1), (npfps(i), i=1, 
c    *                                           ishells(1))  
c ----------------------------------------------------------------------

c ----------------------------------------------------------------------
c  INDX_CC  
      call predefined_int_array("indx_cc"//C_NULL_CHAR, one, 
     *                                   pishells, pindx_cc) 
      call c_f_pointer(pishells, ishells, [1]) 
      call c_f_pointer(pindx_cc, indx_cc, ishells) 
c     write(6,*) ' INDX_CC', ishells(1), (indx_cc(i), i=1, 
c    *                                           ishells(1))  
c ----------------------------------------------------------------------

c ----------------------------------------------------------------------
c  IVANGMOMO   
      call predefined_int_array("ivangmom"//C_NULL_CHAR, one, pishells, 
     *                                   pivangmom) 
      call c_f_pointer(pishells, ishells, [1]) 
      call c_f_pointer(pivangmom, ivangmom, ishells) 
c     write(6,*) ' IVANGMOMOM', ishells(1), (ivangmom(i), i=1, 
c    *                                           ishells(1))  
c ----------------------------------------------------------------------

      ncenters = int_constant("ncenters"//C_NULL_CHAR)
      nshells = int_constant("nshells"//C_NULL_CHAR)
      nalpha_occupied = int_constant("nalpha_occupied"//C_NULL_CHAR)
      nbeta_occupied = int_constant("nbeta_occupied"//C_NULL_CHAR)
      nalpha_virtual = int_constant("nalpha_virtual"//C_NULL_CHAR)
      nbeta_virtual = int_constant("nbeta_virtual"//C_NULL_CHAR)

      intSpherical = int_constant("intspherical"//C_NULL_CHAR)
      spherical = .true.
      if (intSpherical .ne. 1) spherical = .false. 

c     intmax = 100**4 ! VER BAD HACK  
c     zmax = 2*intmax  
 
      i = 0
      do d = d1, d2 
      do c = c1, c2 
      do b = b1, b2 
      do a = a1, a2 
         i = i + 1 
         integrals(a,b,c,d) = 0.0 
      enddo 
      enddo 
      enddo 
      enddo 

c Set the integral tolerance 

c Skip screening for now 
c     call set_itol(delta,itol)
      itol = -1000. 

c     write(6,*) ' DELTA = ', delta, itol  
c     itol = delta ! 1.0d-9 
  
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

c        write(6,*) 'VFL ', a1,a2,':',m1,m2 
c        write(6,*) 'VFL ', b1,b2,':',n1,n2 
c        write(6,*) 'VFL ', c1,c2,':',r1,r2 
c        write(6,*) 'VFL ', d1,d2,':',s1,s2 
c        write(6,*) 'VFL ncemters = ', ncenters 
c        write(6,*) 'VFL '
c        write(6,*) ' COORDS :', (coords(1,i),coords(2,i),coords(3,i), 
c    *                i=1, ncenters)  
c        return 

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
            if (s .eq. 1) then
               dd1 = 1
            else
               dd1 = end_nfps(s-1) + 1
            endif
            dd2 = end_nfps(s)

c-----------------------------------------------------------------------
c   Determine the largest density element.
c-----------------------------------------------------------------------

            emax = 1.0 
            bmax = 1000000.0 ! VFL TO DO 
c           bmax = sz_max(m,n)*sz_max(r,s) 
c           bmax = dsqrt(bmax)*emax  

            if (bmax .lt. itol) go to 30 
               x4 = tcoords(1,s)
               y4 = tcoords(2,s)
               z4 = tcoords(3,s)
               call pack_coeffs(alphas, ixalpha, coeffs, ixpcoef, 
     *                          ncfps, npfps, m, n, 
     *                          r, s, alpha_pack, nalpha_pack, 
     *                          pcoeff_pack, npcoeff_pack, 
     *                          ccbeg, ccend, indx_cc,
     *                          ccbeg_pack, ccend_pack) 

c---------------------------------------------------------------------------
c   Calling sequence for ERD version 2.
c---------------------------------------------------------------------------

               ncsum = ncfps(m) + ncfps(n) + ncfps(r) + ncfps(s)

              call ERD__GENER_ERI_BATCH(imax, zmax,
     *                nalpha_pack, npcoeff_pack, ncsum, 
     *                ncfps(m),ncfps(n), ncfps(r), ncfps(s),
     *                npfps(m),npfps(n), npfps(r), npfps(s),
     *                ivangmom(m), ivangmom(n), 
     *                ivangmom(r), ivangmom(s), x1,y1,z1,
     *                x2,y2,z2,x3,y3,z3,x4,y4,z4, alpha_pack,
     *                pcoeff_pack, ccbeg_pack, ccend_pack,
     *                spherical, .true., iscr, nints, 
     *                nfirst, scr)    

c---------------------------------------------------------------------------
c   Move the integrals into the output block.  (For the GAMESS integral 
c   package, this has been already performed.)
c---------------------------------------------------------------------------

           if (nints .gt. 0) then
 
              jx = 2 
              call uscf_tp6a(nalpha_occupied, nbeta_occupied, jx,  
     *                             nc1,nc2,nd1,nd2,ca,cb,a1,a2,b1,b2,
     *                             c1,c2,d1,d2,
     *                             integrals, 
     *                             scr(nfirst),   
     *                             aa1,aa2,bb1,bb2,cc1,cc2,dd1,dd2,
     *                             Fa,Fb)

            endif

30       continue 
         enddo   ! s
20       continue 
         enddo   ! r

10       continue 
         enddo   ! n
100      continue 
         enddo   ! m
 
               i = 0 
               do d = d1, d2  
               do c = c1, c2  
               do b = b1, b2  
               do a = a1, a2  
                  i = i + 1 
c                 if ((a .ge. aa1 .and. a .le. aa2) .and.
c    *                (b .ge. bb1 .and. b .le. bb2) .and.
c    *                (c .ge. cc1 .and. c .le. cc2) .and.
c    *                (d .ge. dd1 .and. d .le. dd2)) then  
                      aoint(i) = integrals(a,b,c,d)  
c                 endif 
               enddo 
               enddo 
               enddo 
               enddo 

 
c     i = 0
c     do d = d1, d2 
c     do c = c1, c2 
c     do b = b1, b2 
c     do a = a1, a2 
c        i = i + 1 
c        write(6,*) a, b, c, d, aoint(i) 
c     enddo 
c     enddo 
c     enddo 
c     enddo 
c     stop 

c     write(6,*) ' Fock_a inside' 
c     write(6,*) ' -------------' 
c     do b = nc1, nc2
c     do a = nc1, nc2
c        write(6,*) a, b, Fa(a,b) 
c     enddo
c     enddo 

      return
      end
