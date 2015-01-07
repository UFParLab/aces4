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
      subroutine compute_aabb_batch(
     c array_slot_1, rank_1, index_values_1, size_1, extents_1, data_1,
     c array_slot_2, rank_2, index_values_2, size_2, extents_2, data_2,
     c  ierr) BIND(C)
      use, intrinsic :: ISO_C_BINDING
      implicit none

      include 'sip_interface.f' 
      include 'imemory.h'

c Input array used as scratch space 
      integer(C_INT), intent(in)::array_slot_1
      integer(C_INT), intent(in)::rank_1
      integer(C_INT), intent(in)::index_values_1(1:rank_1)
      integer(C_INT), intent(in)::size_1
      integer(C_INT), intent(in)::extents_1(1:rank_1)
      real(C_DOUBLE), intent(out)::data_1(1:size_1)

c Output array holding screening array  
      integer(C_INT), intent(in)::array_slot_2
      integer(C_INT), intent(in)::rank_2
      integer(C_INT), intent(in)::index_values_2(1:rank_2)
      integer(C_INT), intent(in)::size_2
      integer(C_INT), intent(in)::extents_2(1:rank_2)
      real(C_DOUBLE), intent(out)::data_2(2:size_2)

      integer(C_INT), intent(out)::ierr

!     one dimensional array of atomic coordinates  
      TYPE(C_PTR) :: sDimCoords 
      TYPE(C_PTR) :: coords 
      integer, pointer :: pdim_coords(:)  
      real(C_DOUBLE), pointer :: pcoords(:,:)

!     tw0 dimensional array mapping atomic coordinates to shells  
      TYPE(C_PTR) :: sDimTCoords 
      TYPE(C_PTR) :: Tcoords 
      integer, pointer :: pdim_Tcoords(:)  
      real(C_DOUBLE), pointer :: pTcoords(:,:)

!     one dimensional arrays containg ccbeg and ccend  
      TYPE(C_PTR) :: icc, ccbeg, ccend  
      integer, pointer :: picc(:), pccbeg(:), pccend(:)  

!     one dimensional array containg alphas(exponents)   
      TYPE(C_PTR) :: ntot_alpha, ixalphas 
      integer, pointer :: pntot_alpha(:), pixalphas(:) 
      TYPE(C_PTR) :: alphas
      real(C_DOUBLE) , pointer :: palphas(:)

!     one dimensional array containing coefficients  
      TYPE(C_PTR) :: ntot_pcoeff, ixpcoeffs 
      TYPE(C_PTR) :: pcoeffs  
      integer, pointer :: pntot_pcoeff(:), pixpcoeffs(:) 
      real(C_DOUBLE), pointer :: ppcoeffs(:)

!     integer scratch array. HARD coded size should be changed  
c     integer(C_INT) iscr(5000000)   
c     real(C_DOUBLE) scr(5000000)   

      real(C_DOUBLE), pointer :: scr(:) 
      integer, pointer :: iscr(:)
      TYPE(C_PTR) :: cscr
      TYPE(C_PTR) :: ciscr
!     offsets of atomic indeces  
      integer(C_INT) offset_1, offset_2, offset_3, offset_4
      integer(C_INT) i, one, two

c aosegments 
c     integer(C_INT) n_ao_segs(1), aosegs(6)    
      TYPE(C_PTR) :: n_ao_segs, aosegs
      integer, pointer :: pn_ao_segs(:), paosegs(:)

c ----------------------------------------------------------------------
c  END_NFPS 
      TYPE(C_PTR) :: pnalpha, pend_nfps
      integer, pointer :: nalpha(:)
      integer, pointer :: end_nfps(:)
c ----------------------------------------------------------------------

      integer(C_INT) norb, nscr, niscr
c
      one = 0 ! 1
      two = 0 ! 2
      ierr = 0

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
c  ATOMIC COORDINATES  
      call predefined_scalar_array("coords"//C_NULL_CHAR,
     *                                  two, sDimCoords,  coords)
      call c_f_pointer(sDimCoords, pdim_coords,[2])
      call c_f_pointer(coords, pcoords, pdim_coords)
c     write(6,*) ' COORDS',  (pcoords(1,i), i=1, pdim_coords(2),
c    *                pntot_pcoeff(1))  
c ----------------------------------------------------------------------

c ----------------------------------------------------------------------
c  ATOMIC COORDINATES MAPPED TO SHELLS   
      call predefined_scalar_array("tcoords"//C_NULL_CHAR,
     *                                  two, sDimTCoords, tcoords)
      call c_f_pointer(sDimTCoords, pdim_Tcoords,[2])
      call c_f_pointer(tcoords, ptcoords, pdim_tcoords)
c ----------------------------------------------------------------------

c ----------------------------------------------------------------------
c Get the ao segment ranges  
      call predefined_int_array("ao_seg_ranges"//C_NULL_CHAR, one,
     *                           n_ao_segs, aosegs)
      call c_f_pointer(n_ao_segs,pn_ao_segs,[1])
      call c_f_pointer(aosegs,paosegs,pn_ao_segs)
c     write(6,*) ' NAO_SEGS', pn_ao_segs(1), (paosegs(i), i=1, 
c    *                                             pn_ao_segs(1)) 
c ----------------------------------------------------------------------

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
c
c  Set scratch space  
      nscr  = zmax ! 5000000 
      niscr = imax ! 5000000 
c     nscr  = 5000000 
c     niscr = 5000000 

      call scratch_array(nscr, cscr)
      call c_f_pointer(cscr, scr, [nscr])

      call scratch_array(niscr, ciscr)
      call c_f_pointer(ciscr, iscr, [niscr])

      offset_1 = 0
      offset_2 = 0
      offset_3 = 0
      offset_4 = 0
      if (index_values_1(1).gt.1) offset_1=paosegs(index_values_1(1)-1)
      if (index_values_1(2).gt.1) offset_2=paosegs(index_values_1(2)-1)
      if (index_values_1(3).gt.1) offset_3=paosegs(index_values_1(3)-1)
      if (index_values_1(4).gt.1) offset_4=paosegs(index_values_1(4)-1)

      call compute_aabb_integrals(1+offset_1, extents_1(1)+offset_1,
     *                       1+offset_2, extents_1(2)+offset_2,
     *                       1+offset_3, extents_1(3)+offset_3,
     *                       1+offset_4, extents_1(4)+offset_4,
     *                       1, extents_2(2),  
     *                       1, extents_2(2),  
     *                       data_1, !integrals data_1,  
     *                       data_2, ! secreening array   
     *                       scr, ! scratch spaces for integrals data_0,  
c 
     *                       iscr, ! integer scratch spaces for integrals 
     *                       pcoords, ptcoords,
     *                       ppcoeffs, pixpcoeffs, palphas, pixalphas,
     *                       pccbeg, pccend)
c 
c ----------------------------------------------------------------------
c
c  Remove scratch space  
      call delete_scratch_array(cscr)
      call delete_scratch_array(ciscr)

      return
      end

      subroutine compute_aabb_integrals(a1,a2,b1,b2,c1,c2,d1,d2,sa1,sa2,
     *          da1,da2,aoint, screening_array, 
     *          scr, iscr, coords, tcoords, coeffs, ixpcoef, alphas, 
     *          ixalpha, ccbeg, ccend) 
c---------------------------------------------------------------------------
c   The block of integrals (a1:a2,b1:b2,c1:c2,d1:d2) is computed for the 
c   following 'types' of integrals based on atomic labels.
c
c   1. AABB 
c---------------------------------------------------------------------------

      use, intrinsic :: ISO_C_BINDING
      implicit none

c---------------------------------------------------------------------------
c Interface added but should be included instead 

      include 'sip_interface.f' 
      include 'imemory.h'

c---------------------------------------------------------------------------

      integer a1, a2, b1, b2, c1, c2, d1, d2, sa1, sa2, da1, da2    
      integer aa1,aa2,bb1,bb2,cc1,cc2,dd1,dd2
      integer adim, bdim, cdim, ddim  
      integer m1, m2, n1, n2, r1, r2, s1, s2
      integer i, j, k, n, m, r, s
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
      double precision integrals(a1:a2,b1:b2,c1:c2,d1:d2) 
c     double precision screening_array(sa1:sa2,da1:da2) 
      double precision screening_array(sa2*da2), temp(sa2,da2)  
      double precision aoint(*) 

      double precision scr(*)   
      integer iscr(*)

      integer ccbeg(*), ccend(*)
      integer ixalpha(*), ixpcoef(*) 
      integer max_dim_coeff
      parameter (max_dim_coeff = 5000)
      integer ccbeg_pack(max_dim_coeff), ccend_pack(max_dim_coeff)
      double precision alpha_pack(max_dim_coeff)
      double precisionpcoeff_pack(max_dim_coeff)

c VFL TO BE COMPLETED       common /Imax_com/sz_max(nshells,nshells), delta 
      double precision sz_max, delta
      double precision itol, bmax, dtemp, emax

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

c-----------------------------------------------------------------------
c   Find the shell blocks for which we shall loop through.
c-----------------------------------------------------------------------

         call lookup_shell(end_nfps, nshells, a1, m1)
         call lookup_shell(end_nfps, nshells, a2, m2)
         call lookup_shell(end_nfps, nshells, b1, n1)
         call lookup_shell(end_nfps, nshells, b2, n2)
c        call lookup_shell(end_nfps, nshells, c1, r1)
c        call lookup_shell(end_nfps, nshells, c2, r2)
c        call lookup_shell(end_nfps, nshells, d1, s1)
c        call lookup_shell(end_nfps, nshells, d2, s2)

         do m = 1,sa2
         do n = 1,da2
         temp(m,n) = 0.d0
         enddo 
         enddo

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
         do r = m, m
            if (r .eq. 1) then
               cc1 = 1
            else
               cc1 = end_nfps(r-1) + 1
            endif
            cc2 = end_nfps(r)

            x3 = tcoords(1,r)
            y3 = tcoords(2,r)
            z3 = tcoords(3,r)
         do s = n, n
               x4 = tcoords(1,s)
               y4 = tcoords(2,s)
               z4 = tcoords(3,s)

c---------------------------------------------------------------------------
c   Calling sequence for ERD version 2.
c---------------------------------------------------------------------------

               ncsum = ncfps(m) + ncfps(n) + ncfps(r) + ncfps(s)

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
c   Move the integrals into the output block.  
c---------------------------------------------------------------------------

            if (nints .gt. 0) then
               
               if (s .eq. 1) then
                  dd1 = 1
               else
                  dd1 = end_nfps(s-1) + 1
               endif
               dd2 = end_nfps(s)

               call find_Imax(scr(nfirst), nints, 1, sa2, 1, da2, 
     *                        temp, m,n) 

            else 
               
               call zero_Imax(1, sa2, 1, da2, temp, m,n) 

            endif

30       continue 
         enddo   ! s
20       continue 
         enddo   ! r

c        write(6,*) ' sa1, sa2, M N IMAX :', sa1, sa2, m, n,   
c    *        screening_array(m,n) 

c        write(6,*) ' M N :', m, n 
c        do r = nfirst, nfirst+nints-1 
c           write(6,*) ' VFL ', r, scr(r) 
c        enddo 

10       continue 
         enddo   ! n
100      continue 
         enddo   ! m

c        write(6,*) ' SSAA:', sa1, sa2, da1, da2, m1,m2,n1,n2

         k = 0 
         do i = sa1, sa2 
         do j = da1, da2 
            k = k + 1 

               if (i.ge.n1 .and. i.le.n2 .and. j.ge.m1 .and. j.le.m2) 
     *            then   
                 screening_array(k) = temp(j,i) 
c                write(6,*) ' K M N :', temp(j,i), screening_array(k)
               endif 

         ENDDO 
         ENDDO 

33    FORMAT(5I4,F12.8) 

      return
      end

      subroutine find_Imax(intblk,nints,sa1,sa2,da1,da2,sz_max,m,n)

      implicit none

      integer m, n, nints, d, sa1, sa2, da1, da2  

      double precision intblk(*)
      double precision Int_max, tmax
      double precision sz_max(sa1:sa2,da1:da2)  

      Int_max = sz_max(m,n)
      if (sz_max(m,n) .gt. sz_max(n,m)) Int_max = sz_max(n,m)
      do d = 1, nints  
         tmax = dabs(intblk(d))
         if (tmax .gt. Int_max) Int_max = tmax
      enddo
      sz_max(m,n) = Int_max 
c     if (sz_max(n,m) .gt. Int_max) sz_max(m,n) = sz_max(n,m)  

c     write(6,*) ' M N Imax ', m, n, Int_max

      return
      end


      subroutine zero_Imax(sa1,sa2,da1,da2, sz_max, m,n)

      implicit none

      integer m, n, sa1, sa2, da1, da2

      double precision Int_max, tmax
      double precision sz_max(sa1:sa2,da1:da2)  

      sz_max(m,n) = 0.0 
      sz_max(n,m) = 0.0  

c     write(6,*) ' SKIPPING M N Imax ', m, n, Int_max

      return
      end


