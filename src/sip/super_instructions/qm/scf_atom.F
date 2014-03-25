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
      subroutine scf_atom(
     c array_slot_0, rank_0, index_values_0, size_0, extents_0, data_0,
     c array_slot_1, rank_1, index_values_1, size_1, extents_1, data_1,
     c array_slot_2, rank_2, index_values_2, size_2, extents_2, data_2,
     c array_slot_3, rank_3, index_values_3, size_3, extents_3, data_3,
     c array_slot_4, rank_4, index_values_4, size_4, extents_4, data_4,
     c array_slot_5, rank_5, index_values_5, size_5, extents_5, data_5,
     c  ierr) BIND(C)

      use, intrinsic :: ISO_C_BINDING
      implicit none

c-------------------------------------------------------------------------- 
      include 'sip_interface.f'
      include 'imemory.h' 

!     which fragment  
      integer(C_INT), intent(in)::array_slot_0
      integer(C_INT), intent(in)::rank_0
      integer(C_INT), intent(in)::index_values_0(1:rank_0)
      integer(C_INT), intent(in)::size_0
      integer(C_INT), intent(in)::extents_0(1:rank_0)
      real(C_DOUBLE), intent(out)::data_0(1:size_0)

!     should be static array alpha hf coeff ca  
      integer(C_INT), intent(in)::array_slot_1
      integer(C_INT), intent(in)::rank_1
      integer(C_INT), intent(in)::index_values_1(1:rank_1)
      integer(C_INT), intent(in)::size_1
      integer(C_INT), intent(in)::extents_1(1:rank_1)
      real(C_DOUBLE), intent(in)::data_1(1:size_1)

!     should be static array beta hf coeff cb  
      integer(C_INT), intent(in)::array_slot_2
      integer(C_INT), intent(in)::rank_2
      integer(C_INT), intent(in)::index_values_2(1:rank_2)
      integer(C_INT), intent(in)::size_2
      integer(C_INT), intent(in)::extents_2(1:rank_2)
      real(C_DOUBLE), intent(in)::data_2(1:size_2)

!     should be static array  alpha fock matrix fa  
      integer(C_INT), intent(in)::array_slot_3
      integer(C_INT), intent(in)::rank_3
      integer(C_INT), intent(in)::index_values_3(1:rank_3)
      integer(C_INT), intent(in)::size_3
      integer(C_INT), intent(in)::extents_3(1:rank_3)
      real(C_DOUBLE), intent(in)::data_3(1:size_3)

!     should be static array  beta fock matrix fb  
      integer(C_INT), intent(in)::array_slot_4
      integer(C_INT), intent(in)::rank_4
      integer(C_INT), intent(in)::index_values_4(1:rank_4)
      integer(C_INT), intent(in)::size_4
      integer(C_INT), intent(in)::extents_4(1:rank_4)
      real(C_DOUBLE), intent(in)::data_4(1:size_4)

!     should be static array  of overlap integrals  
      integer(C_INT), intent(in)::array_slot_5
      integer(C_INT), intent(in)::rank_5
      integer(C_INT), intent(in)::index_values_5(1:rank_5)
      integer(C_INT), intent(in)::size_5
      integer(C_INT), intent(in)::extents_5(1:rank_5)
      real(C_DOUBLE), intent(in)::data_5(1:size_5)

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

!     Number of atomic centers 
      integer (C_INT) ncenters        

      integer norbmax, vmax, omax, maxblk, maxhist, diis_start 
      integer one, two, i, j  
      integer iscr(500000) 
      double precision scr(500000) 
      double precision scr1(500000) 
      double precision scr2(500000) 
      double precision scr3(500000) 
      double precision scr4(500000) 
      double precision scr5(500000) 

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
c  ATOMIC COORDINATES  
      call predefined_scalar_array("tcoords"//C_NULL_CHAR,
     *                                  two, sDimCoords,  tcoords)
      call c_f_pointer(sDimCoords, pdim_coords,[2])
      call c_f_pointer(tcoords, ptcoords, pdim_coords)
c     do i = 1, pdim_coords(2) 
c         write(6,*) 'I = ', i 
c     write(6,*) ' TCOORDS',  ptcoords(1,i), ptcoords(2,i),
c    *                        ptcoords(3,i) 
c     enddo 
c ----------------------------------------------------------------------


      ncenters        = int_constant("ncenters"//C_NULL_CHAR)

c ----------------------------------------------------------------------

c----------------------------------------------------------------------------
c   Set up the integral pointer 
c----------------------------------------------------------------------------

c     integral_scr = dpkgscr
c     maxblk  = zmax

         maxblk = 5000000

c---------------------------------------------------------------------------

         norbmax = extents_1(1) ! inda(1) 
         vmax    = extents_1(1) ! inda(1) 
         omax    = extents_1(1) ! inda(1) 
         maxhist = 8 
         diis_start = 3 

c---------------------------------------------------------------------------
c Determine 
c 1. vmax: maximum number of virtuals on atom 
c 2. omax: maximum number of occupieds on atom 
c 3. norbmax: maximum number of AO's on atom 
c---------------------------------------------------------------------------

c        if (frag_calc) then 

         call compute_fragscf(data_0,     ! OK  
     *                 scr, ! dbuf(integral_scr),
     *                 maxblk, 
     *                 iscr, ! ibuf(ipkgscr),
     *                 pcoords, ! dbuf(icoord),
     *                 ptcoords, ! dbuf(icoord),
     *                 ppcoeffs, !  dbuf(ipcoeff),
     *                 palphas, ! dbuf(ialpha), 
     *                 pccbeg, ! ibuf(iccbeg), 
     *                 pccend, ! ibuf(iccend), 
     *                 1, extents_1(1), ! 1, inda(1), 
     *                 1, extents_1(1), ! inda(2), 
     *                 scr1, ! x(nai_array), 
     *                 scr2, ! x(kin_array), 
     *                 ncenters, 
     *                 scr4, ! x(farray), 
     *                 scr5, ! x(fbarray), 
     *                 data_1, !x(ca_array), 
     *                 data_2, ! x(cb_array), 
     *                 data_3, ! x(fa_array), 
     *                 data_4, ! x(fb_array), 
     *                 data_5, ! overlap array  
     *                 vmax,omax,norbmax,maxhist,diis_start) 

c        call compute_fragscf(scalar_table(sind), 
c    *                 dbuf(integral_scr),maxblk, ibuf(ipkgscr),
c    *                 dbuf(icoord),dbuf(ipcoeff),dbuf(ialpha), 
c    *                 ibuf(iccbeg), ibuf(iccend), 
c    *                 1, inda(1), 1,inda(2), 
c    *                 x(nai_array), x(kin_array), x(ovl_array),
c    *                 x(farray), x(fbarray), 
c    *                 x(ca_array), x(cb_array), 
c    *                 x(fa_array), x(fb_array), 
c
c    *                 vmax,omax,norbmax,maxhist,diis_start) 

c        return 
c        endif 

      return
      end
C
      subroutine compute_fragscf(current_frag, scr,
     *                 maxblk, iscr, coords, tcoords ,coeffs,alphas, 
     *                 ccbeg, ccend,
     *                 nc1,nc2, nd1, nd2,
     *                 nai, kin, ncenters, fa, fb, 
     *                 ca_perm, cb_perm, fa_perm, fb_perm, ovl,  
c
     *                 vmax,omax,norb,maxhist,diis_start)  
c---------------------------------------------------------------------------

      use, intrinsic :: ISO_C_BINDING
      implicit none

c---------------------------------------------------------------------------
c Interface added but should be included instead 

      include 'sip_interface.f'
      include 'imemory.h'

      integer (C_INT) intSpherical
      integer (C_INT) nshells         
      integer (C_INT) nfrags         
      integer (C_INT) nalpha_occupied 
      integer (C_INT) nbeta_occupied  
      integer (C_INT) nalpha_virtual  
      integer (C_INT) nbeta_virtual   

c ----------------------------------------------------------------------
c  END_NFPS 
      TYPE(C_PTR) :: pnalpha, pend_nfps 
      integer, pointer :: nalpha(:) 
      integer, pointer :: end_nfps(:) 
c ----------------------------------------------------------------------

c array defining which atoms are in which fragment 
      TYPE(C_PTR) :: sDimfrag  
      TYPE(C_PTR) :: pwatom_frag 
      integer, pointer :: pdim_frag(:)  
      real(C_DOUBLE), pointer :: watom_frag(:,:)
c array defining the number of atoms per fragment 
      TYPE(C_PTR) :: pnatoms_frag 
      real(C_DOUBLE), pointer :: natoms_frag(:)
c     mapping of atoms to shells 
      TYPE(C_PTR) :: patom  
      integer, pointer :: atom(:)
c  CHARGES    
      TYPE(C_PTR) :: charge, atoms
      integer, pointer :: patoms(:)
      real(C_DOUBLE), pointer :: pcharge(:)
c scratch space 
      real(C_DOUBLE), pointer :: scr_i(:)
      real(C_DOUBLE), pointer :: iscr_i(:)
      TYPE(C_PTR) :: cscr
      TYPE(C_PTR) :: ciscr

c arrays defining fragment to otbital mapping  
      real(C_DOUBLE), pointer :: baocc_frag(:,:)
      real(C_DOUBLE), pointer :: bbocc_frag(:,:)
      real(C_DOUBLE), pointer :: bavirt_frag(:,:)
      real(C_DOUBLE), pointer :: bbvirt_frag(:,:)
c ----------------------------------------------------------------------


      integer aa1,aa2,bb1,bb2,cc1,cc2,dd1,dd2
      integer adim, bdim, cdim, ddim  
      integer m1, m2, n1, n2, r1, r2, s1, s2
      integer i, j, n, m, r, s
      integer a,b,c,d
      integer ncenters  
      integer iatom(ncenters), n_basis, n_apf, nf   
      double precision current_frag 
      integer icurrent_frag 

      integer num_to_do, nsend
      integer nints, maxblk
      integer nalpha_pack, npcoeff_pack
      integer ncsum, next, nfirst
      integer me, ierr
      integer nc1, nc2, nd1, nd2 

      integer imin, zmin, iblk, zblk
      integer one, two 
      integer nscr, niscr 

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

      double precision coords(3,*), tcoords(3,*), coeffs(*), alphas(*)
      double precision nai(nc1:nc2,nd1:nd2)
      double precision kin(nc1:nc2,nd1:nd2)
      double precision ovl(nc1:nc2,nd1:nd2)
      double precision H0T(nc1:nc2,nd1:nd2)

      double precision ca(nc1:nc2,nd1:nd2)
      double precision cb(nc1:nc2,nd1:nd2)

      double precision fa(nc1:nc2,nd1:nd2)
      double precision fb(nc1:nc2,nd1:nd2)

      double precision ca_perm(nc1:nc2,nd1:nd2)
      double precision cb_perm(nc1:nc2,nd1:nd2)
      double precision fa_perm(nc1:nc2,nd1:nd2)
      double precision fb_perm(nc1:nc2,nd1:nd2)

      integer map(nc1:nc2) 
      integer umap(nc1:nc2) 

      integer max_shells 
      parameter (max_shells = 5000)
      integer beg_anfps(max_shells)  
      integer end_anfps(max_shells)  
      double precision scr(*)   
      integer iscr(*)

      integer ccbeg(*), ccend(*)

      integer max_dim_coeff
      parameter (max_dim_coeff = 25000)
      integer ccbeg_pack(max_dim_coeff), ccend_pack(max_dim_coeff)
      double precision alpha_pack(max_dim_coeff), 
     *                 pcoeff_pack(max_dim_coeff)
      integer n_max, nout  
c
c ---------------------------------------------------------------------- 
c Arrays used in the DIIS procedure 
c ---------------------------------------------------------------------- 

      integer vmax,omax,norb,maxhist,diis_start  
      double precision Fai(vmax,omax,maxhist+1) 
      double precision Fbj(vmax,omax,maxhist+1) 
      double precision Ea(norb,norb,maxhist+1) 
      double precision Eb(norb,norb,maxhist+1) 
      double precision BM(maxhist+1,maxhist+1) 
      double precision TBM(maxhist+1,maxhist+1) 
      double precision CM(maxhist+1) 
c
c ---------------------------------------------------------------------- 
c Get predefined data 
c ---------------------------------------------------------------------- 
      intSpherical    = int_constant("intspherical"//C_NULL_CHAR)
      nshells         = int_constant("nshells"//C_NULL_CHAR)
      nalpha_occupied = int_constant("nalpha_occupied"//C_NULL_CHAR)
      nbeta_occupied  = int_constant("nbeta_occupied"//C_NULL_CHAR)
      nalpha_virtual  = int_constant("nalpha_virtual"//C_NULL_CHAR)
      nbeta_virtual   = int_constant("nbeta_virtual"//C_NULL_CHAR)
      nfrags          = int_constant("nfrags"//C_NULL_CHAR)

c array defining which atoms are in which fragments 
c ----------------------------------------------------------------------
      call predefined_int_array("watom_frag"//C_NULL_CHAR,
     *                                  two, sDimfrag,  pwatom_frag)
      call c_f_pointer(sDimfrag, pdim_frag,[2])
      call c_f_pointer(pwatom_frag, watom_frag, pdim_frag)
c array defining the number of atoms in each fragment 
c ----------------------------------------------------------------------
      call predefined_int_array("natoms_frag"//C_NULL_CHAR,
     *                                  one, sDimfrag,  pnatoms_frag)
      call c_f_pointer(sDimfrag, pdim_frag,[1])
      call c_f_pointer(pnatoms_frag, natoms_frag, pdim_frag)
c ----------------------------------------------------------------------
c  END_NFPS 
      call predefined_int_array("end_nfps"//C_NULL_CHAR, one,
     *                                   pnalpha, pend_nfps)
      call c_f_pointer(pnalpha, nalpha, [1])
      call c_f_pointer(pend_nfps, end_nfps, nalpha)
c ----------------------------------------------------------------------
c  mapping of atoms to shells  
      call predefined_int_array("atom"//C_NULL_CHAR, one,
     *                                   pnalpha, patom)
      call c_f_pointer(pnalpha, nalpha, [1])
      call c_f_pointer(patom, atom, nalpha)
c ----------------------------------------------------------------------
c  NUCLEAR CHARGES    
c ----------------------------------------------------------------------
      call predefined_scalar_array("charge"//C_NULL_CHAR,
     *                               one, atoms ,charge)
      call c_f_pointer(atoms, patoms,[1])
      call c_f_pointer(charge, pcharge, patoms)

c retrive alpha occupied mapping 
      call predefined_int_array("baocc_frag"//C_NULL_CHAR,
     *                                  two, sDimfrag,  pwatom_frag)
      call c_f_pointer(sDimfrag, pdim_frag,[2])
      call c_f_pointer(pwatom_frag, baocc_frag, pdim_frag)
c retrive beta occupied mapping 
      call predefined_int_array("bbocc_frag"//C_NULL_CHAR,
     *                                  two, sDimfrag,  pwatom_frag)
      call c_f_pointer(sDimfrag, pdim_frag,[2])
      call c_f_pointer(pwatom_frag, bbocc_frag, pdim_frag)
c retrive alpha virtual mapping 
      call predefined_int_array("bavirt_frag"//C_NULL_CHAR,
     *                                  two, sDimfrag,  pwatom_frag)
      call c_f_pointer(sDimfrag, pdim_frag,[2])
      call c_f_pointer(pwatom_frag, bavirt_frag, pdim_frag)
c retrive beta virtual mapping 
      call predefined_int_array("bbvirt_frag"//C_NULL_CHAR,
     *                                  two, sDimfrag,  pwatom_frag)
      call c_f_pointer(sDimfrag, pdim_frag,[2])
      call c_f_pointer(pwatom_frag, bbvirt_frag, pdim_frag)

      spherical = .true.
      if (intSpherical .ne. 1) spherical = .false.
c
c ---------------------------------------------------------------------- 

      write(6,*) ' NFRAGS = ', nfrags 
      icurrent_frag = current_frag 
      call return_napf(n_apf, icurrent_frag, nfrags, natoms_frag) 
c     n_apf = natoms_frag(icurrent_frag) 
      do m = 1, n_apf 
         call return_predef_2d_int(nfrags, ncenters, icurrent_frag, m, 
     *                             watom_frag, nout) 
         iatom(m) = nout ! watom_frag(icurrent_frag, m) 
      enddo 
c     iatom = wfrag 
      write(6,*) ' Entering H0 on frag:', icurrent_frag, ':', 
     *            n_apf, (iatom(m), m=1, n_apf)  

c   Set scratch space 

      nscr  = 5000000 ! zmax  
      niscr = 5000000 ! imax 
      call scratch_array(nscr, cscr)
      call c_f_pointer(cscr, scr_i, [nscr])
      call scratch_array(niscr, ciscr)
      call c_f_pointer(ciscr, iscr_i, [niscr])

      call comp_return_frag_h0(H0T, n_apf, iatom, nc1, nc2, nd1, nd2,
     *                         ncenters, 
     *                         coords, tcoords, 
     *                         pcharge, atom,  
     *                         scr_i, iscr_i) 
c  Remove scratch space  
c     call delete_scratch_array(cscr)
c     call delete_scratch_array(ciscr)

c-----------------------------------------------------------------------
c   Find the shell blocks for which we shall loop through.
c-----------------------------------------------------------------------

         m1 = 1 
         n1 = 1 
         r1 = 1 
         s1 = 1 

         m2 = (nshells)   
         n2 = (nshells)  
         r2 = (nshells) 
         s2 = (nshells)  

c-----------------------------------------------------------------------
c   Find the number of basis functions and shells in the atom.  
c-----------------------------------------------------------------------

         n_basis = 0 
         do m = m1, m2 
         do n = 1, n_apf 
            if(atom(m) .eq. iatom(n)) n_basis = n_basis + 
     *                end_nfps(m) - end_nfps(m-1)   
         enddo 
         enddo 

         write(6,*) ' The number of basis functions on frag', 
     *               icurrent_frag, '=', n_basis  
c        do m = 1, n_basis 
c        do n = 1, n_basis 
c           write(6,*) ' VFL:', m,n, h0t(m,n) 
c        enddo 
c        enddo 

c-----------------------------------------------------------------------
c   Find the mapping from atom <--> molecule.  
c-----------------------------------------------------------------------

         do n = nc1, nc2 
            map(n) = 0 
            umap(n) = 0 
         enddo 

         n_basis = 0 
         n_max = 0 
         do m = m1, m2 
            beg_anfps(m) = 0   
            end_anfps(m) = 0   

          do nf = 1, n_apf 
            if(atom(m) .eq. iatom(nf)) then 
               beg_anfps(m) = n_basis + 1  

               if (m .eq. 1) then
                  DO n = 1, end_nfps(m) 
                     n_basis = n_basis + 1 
                     map(n_basis) = n 
                     umap(n) = n_basis  
                     if (n .gt. n_max) n_max = n 
                  enddo 
               else 
                  DO n = end_nfps(m-1) + 1, end_nfps(m) 
                     n_basis = n_basis + 1 
                     map(n_basis) = n 
                     umap(n) = n_basis  
                     if (n .gt. n_max) n_max = n 
                  enddo 
               endif 

               end_anfps(m) = n_basis 

            endif 
          enddo 

         enddo 

         if ((n_max .lt. nc1) .or. (n_max .gt. nc2)) then 
           write(66,*) ' UMapping of atom dens wrong:' 
           write(66,*) ' Correct range :', nc1, nc2, 'Computed range:', 
     *                 '1', n_max  
         endif 

         if ((n_basis .lt. nc1) .or. (n_basis .gt. nc2)) then 
           write(66,*) ' Mapping of atom dens wrong:' 
           write(66,*) ' Correct range :', nc1, nc2, 'Computed range:', 
     *                 '1', n_basis   
         endif 

c        do j = nd1, nd2 
c        do i = nc1, nc2 
c           ca_perm(i,j) = 0.0 
c           cb_perm(i,j) = 0.0 
c           fa_perm(i,j) = 0.0 
c           fb_perm(i,j) = 0.0 
c        enddo 
c        enddo 

c   Set scratch space 

      nscr  = 5000000 ! zmax  
      niscr = 5000000 ! imax 
      imax = niscr 
      zmax = nscr 
c     call scratch_array(nscr, cscr)
c     call c_f_pointer(cscr, scr_i, [nscr])
c     call scratch_array(niscr, ciscr)
c     call c_f_pointer(ciscr, iscr_i, [niscr])

         call do_fragscf(icurrent_frag, scr,
     *               maxblk, iscr, tcoords,coeffs,alphas, ccbeg, ccend,
     *               nc1,nc2, nd1, nd2,
     *               H0T, nai, kin, ovl, ncenters, nshells, spherical,  
     *               n_basis, beg_anfps, end_anfps, map, umap, 
c
     *               ca_perm, cb_perm, fa_perm, fb_perm, 
     *               natoms_frag, 
     *               watom_frag, 
     *               pcharge, 
     *               atom, 
     *                         nfrags, 
     *                         baocc_frag,
     *                         bbocc_frag,
     *                         bavirt_frag,
     *                         bbvirt_frag, 
c 
     *               vmax,omax,norb,maxhist,diis_start) 
c  Remove scratch space  
      call delete_scratch_array(cscr)
      call delete_scratch_array(ciscr)

      return 
      end 

      subroutine do_fragscf(current_frag, scr,
     *               maxblk, iscr, tcoords,coeffs,alphas, ccbeg, ccend,
     *               nc1,nc2, nd1, nd2,
     *               H0T, nai, kin, ovl, ncenters, nshells, spherical,
     *               n_basis, beg_anfps, end_anfps, map, umap, 
c
     *               ca_perm, cb_perm, fa_perm, fb_perm, 
     *               natoms_frag, 
     *               watom_frag, 
     *               charge, 
     *               atom, 
     *                         nfrags, 
     *                         baocc_frag,
     *                         bbocc_frag,
     *                         bavirt_frag,
     *                         bbvirt_frag, 
c
     *               vmax,omax,norb,maxhist,diis_start) 
c---------------------------------------------------------------------------

      use, intrinsic :: ISO_C_BINDING
      implicit none

c-------------------------------------------------------------------------- 
      include 'sip_interface.f'
      include 'imemory.h' 

!     one dimensional array containg alphas(exponents)   
      TYPE(C_PTR) :: ntot_alpha, ixalphas 
      integer, pointer :: pntot_alpha(:), pixalphas(:) 

!     one dimensional array containg coefficients 
      TYPE(C_PTR) :: ntot_pcoeff, ixpcoeffs 
      TYPE(C_PTR) :: pcoeffs  
      integer, pointer :: pntot_pcoeff(:), pixpcoeffs(:) 
      real(C_DOUBLE), pointer :: ppcoeffs(:) 
c ----------------------------------------------------------------------
c  NCFPS 
      TYPE(C_PTR) :: pishells, pncfps 
      integer, pointer :: ishells(:) 
      integer, pointer :: ncfps(:) 
c  NPFPS 
      TYPE(C_PTR) :: pnpfps
      integer, pointer :: npfps(:)
c  INDX_CC   
      TYPE(C_PTR) :: pindx_cc
      integer, pointer :: indx_cc(:)
c  IVANGMOM  
      TYPE(C_PTR) :: pivangmom
      integer, pointer :: ivangmom(:)
c ----------------------------------------------------------------------

      integer a1, a2, b1, b2, c1, c2, d1, d2 
      integer aa1,aa2,bb1,bb2,cc1,cc2,dd1,dd2
      integer adim, bdim, cdim, ddim  
      integer m1, m2, n1, n2, r1, r2, s1, s2
      integer i, j, n, m, r, s, l, mn, rs  
      integer a,b,c,d
      integer ncenters, nshells  
      integer iatom(ncenters), n_basis, n_basis_org, n_apf, nf   
      integer current_frag, nfrags  

      integer num_to_do, nsend
      integer nints, maxblk
      integer nalpha_pack, npcoeff_pack
      integer ncsum, next, nfirst
      integer me, ierr
      integer nc1, nc2, nd1, nd2 

      integer imin, zmin, iblk, zblk
      integer one, two 

      logical dom, don, dor, dos 
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

      double precision tcoords(3,*), coeffs(*), alphas(*)
      double precision nai(nc1:nc2,nd1:nd2)
      double precision kin(nc1:nc2,nd1:nd2)
      double precision ovl(nc1:nc2,nd1:nd2)
      double precision H0T(nc1:nc2,nd1:nd2)

      double precision ca_perm(nc1:nc2,nd1:nd2)
      double precision cb_perm(nc1:nc2,nd1:nd2)
      double precision fa_perm(nc1:nc2,nd1:nd2)
      double precision fb_perm(nc1:nc2,nd1:nd2)

      double precision h0(n_basis,n_basis) 
      double precision aovl(n_basis,n_basis) 
      double precision sos(n_basis,n_basis) 
      double precision Qxx(n_basis,n_basis) 

      double precision HFD_A(n_basis,n_basis) 
      double precision HFD_B(n_basis,n_basis) 
      double precision HFDOLD_A(n_basis,n_basis) 
      double precision HFDOLD_B(n_basis,n_basis) 

      double precision ca(n_basis,n_basis) 
      double precision cb(n_basis,n_basis) 
      double precision cba(n_basis,n_basis) 
      double precision cbb(n_basis,n_basis) 
      double precision FTa(n_basis,n_basis) 
      double precision FTb(n_basis,n_basis) 
      double precision Fa(n_basis,n_basis) 
      double precision Fb(n_basis,n_basis) 

      integer baocc_frag(nfrags,*)
      integer bbocc_frag(nfrags,*)
      integer bavirt_frag(nfrags,*)
      integer bbvirt_frag(nfrags,*)

      double precision temp, tempa, tempb   
      integer doit, itemp, jtemp, p, p1  

      integer tcharge, ma_offset, na_offset 
      integer mb_offset, nb_offset, nstart  
      integer nocc_a, nocc_b, nvirt_a, nvirt_b  
      integer nocc_a_org, nocc_b_org 
      integer iter, max_iter 
c
c ---------------------------------------------------------------------- 
c Arrays used in the DIIS procedure 
c ---------------------------------------------------------------------- 

      integer vmax,omax,norb,nhist,maxhist,diis_start   
      double precision Fai(vmax,omax,maxhist+1) 
      double precision Fbj(vmax,omax,maxhist+1) 
      double precision Ea(norb,norb,maxhist+1) 
      double precision Eb(norb,norb,maxhist+1) 
      double precision BM(maxhist+1,maxhist+1) 
      double precision TBM(maxhist+1,maxhist+1) 
      double precision CM(maxhist+1) 
c ---------------------------------------------------------------------- 

      integer max_dim_coeff
      parameter (max_dim_coeff = 25000)

      integer max_shells 
      parameter (max_shells = 5000)
      integer natoms_frag(current_frag) 
      integer watom_frag(current_frag, max_shells) 
      integer atom(nshells) 

      integer map(nc1:nc2) 
      integer umap(nc1:nc2) 
      integer beg_anfps(max_shells)  
      integer end_anfps(max_shells)  
      double precision scr(*)   
      integer iscr(*)

      integer ccbeg(*), ccend(*)

      integer ccbeg_pack(max_dim_coeff), ccend_pack(max_dim_coeff)
      double precision alpha_pack(max_dim_coeff), 
     *                 pcoeff_pack(max_dim_coeff)
      double precision charge(ncenters) 

c ----------------------------------------------------------------------
c Get prdefined data 
c ----------------------------------------------------------------------
c  IXALPHAS(exponents)     
      call predefined_int_array("ixalphas"//C_NULL_CHAR, one,
     *                                   ntot_alpha, ixalphas)
      call c_f_pointer(ntot_alpha, pntot_alpha,[1])
      call c_f_pointer(ixalphas, pixalphas, pntot_alpha)
c ----------------------------------------------------------------------
c  IXPCOEFFS  
c ----------------------------------------------------------------------
      call predefined_int_array("ixpcoeffs"//C_NULL_CHAR,one,
     *                                   ntot_pcoeff, ixpcoeffs)
      call c_f_pointer(ntot_pcoeff, pntot_pcoeff,[1])
      call c_f_pointer(ixpcoeffs, pixpcoeffs, pntot_pcoeff)
c ----------------------------------------------------------------------
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
c ----------------------------------------------------------------------

      n_apf = natoms_frag(current_frag) 
      do m = 1, n_apf 
         iatom(m) = watom_frag(current_frag, m) 
      enddo 

      nocc_a = 0 
      nocc_b = 0 
      tcharge = 0 
      do m = 1, ncenters 
       do n = 1, n_apf 
         if (m .eq. iatom(n)) then 
c           nocc_b = nocc_b + charge(m)/2 
c           nocc_a = nocc_a + charge(m) - nocc_b 
            tcharge = tcharge + charge(m)  
         endif 
       enddo 
      enddo 
      nocc_a = tcharge/2 
      nocc_b = tcharge - nocc_a 

      nvirt_a = n_basis - nocc_a  
      nvirt_b = n_basis - nocc_b  

c TEST !!!! 

      write(6,*) ' Performing an SCF calculation on frag:', 
     *  current_frag, 
     * 'in a basis of', n_basis, 'functions with', nocc_a, nocc_b, 
     * 'alpha and beta occupied electrons and ', nvirt_a, nvirt_b, 
     * ' alpha and beta virtual electrons'    
      if (nocc_a .eq. 0) return  

c-----------------------------------------------------------------------
c   Find the shell blocks for which we shall loop through.
c-----------------------------------------------------------------------

      m1 = 1 
      n1 = 1 
      r1 = 1 
      s1 = 1 

      m2 = (nshells)   
      n2 = (nshells)  
      r2 = (nshells) 
      s2 = (nshells)  

c-----------------------------------------------------------------------
c Sum nai and kin into small array and copy ovl there too. 
c --> initial guess   
c-----------------------------------------------------------------------

      do n = 1, n_basis  
      do m = 1, n_basis  
         h0(m,n) = 0.0  
      enddo  
      enddo  

      write(6,*) ' NC = ', nc1, nc2, ':', nd1, nd2 

      itemp = 0 
      do n = nc1, nc2 
         if (umap(n).ne.0) then 
            itemp = itemp + 1 
            jtemp = 0 
      do m = nc1, nc2 
         if (umap(m).ne.0) then 
            aovl(umap(m),umap(n)) = ovl(m,n) 
            h0(umap(m),umap(n))   = H0T(m,n)   
            jtemp = jtemp + 1 
         endif 
      enddo  
         if (jtemp .ne. n_basis) then 
            write(6,*) ' Jtemp .ne. n_basis ', jtemp, n_basis 
            call abort_job() 
         endif 
         endif 
      enddo  
      if (itemp .ne. n_basis) then 
         write(6,*) ' itemp .ne. n_basis ', itemp, n_basis 
         call abort_job() 
      endif 

      if (itemp*jtemp .ne. n_basis**2) then 
         write(6,*) ' Something possibly wrong with umap ', 
     *                itemp, n_basis 
         call abort_job()
      endif 

c-----------------------------------------------------------------------
c Construct the hcore initial guess  
c-----------------------------------------------------------------------

      do n = 1, n_basis  
      do m = 1, n_basis  
         FA(m,n)    = 0.0  
         FB(m,n)    = 0.0  
         FTA(m,n)   = 0.0  
         FTB(m,n)   = 0.0  
         HFD_a(m,n) = 0.0 
         HFD_b(m,n) = 0.0 
         ca(m,n)    = 0.0 
         cb(m,n)    = 0.0 
      enddo  
      enddo  

      do n = 1, n_basis  
      do m = 1, n_basis  
         FA(m,n) = h0(m,n) 
         FB(m,n) = h0(m,n) 
      enddo  
      enddo  

c-----------------------------------------------------------------------
c Construct U*S**(-1/2)  
c-----------------------------------------------------------------------

      call diag(aovl,sos,m,n_basis,0,1,1) 
      do m = 1, n_basis  
      do n = 1, n_basis  
         temp = 0.0 
         do l = 1, n_basis  
            temp = temp + sos(m,l)*aovl(l,n) 
         enddo 
         Qxx(m,n) = temp 
       enddo  
       enddo  

c-----------------------------------------------------------------------
c Transpose the Fock matrix -> Construct S^(-1/2) F S^(-1/2)  
c-----------------------------------------------------------------------

       call fock_transpose(FA,FB,Qxx,FTa,FTb,n_basis) 

c-----------------------------------------------------------------------
c Diagonalize the transposed Fock matrix  
c-----------------------------------------------------------------------

       call diag(FTa,ca,m,n_basis,0,0,0) 
       call diag(FTb,cb,m,n_basis,0,0,0) 

       if (nocc_a .eq. nocc_b) then 
          do n = 1, n_basis
          do m = 1, n_basis
                 cb(m,n) = ca(m,n)  
          enddo 
          enddo 
       endif 

c-----------------------------------------------------------------------
c Back transform the coefficient array  
c-----------------------------------------------------------------------

       call c_backtran(Qxx,ca,cb,cba,cbb,n_basis) 

c-----------------------------------------------------------------------
c Compute the HF density  
c-----------------------------------------------------------------------

       call hfdensity(ca,cb,HFD_A,HFD_B,n_basis,nocc_a,nocc_b) 

c-----------------------------------------------------------------------
c Compute the HF energy   
c-----------------------------------------------------------------------

       call hfenergy(HFD_A,HFD_B,FA,FB,h0,n_basis) 

c-----------------------------------------------------------------------
c Copy the HF Density into the old Density. 
c-----------------------------------------------------------------------

      call hfdensity_copy(HFD_A,HFD_B,HFDOLD_A,HFDOLD_B,n_basis)  

c     if (nocc_a .ne. nocc_b) go to 100 

c     if (nocc_a .eq. 1) go to 100 

c-----------------------------------------------------------------------
c Start the SCF iterations  
c-----------------------------------------------------------------------

      max_iter = 100  
      DO iter = 1, max_iter 

c-----------------------------------------------------------------------
c       Construct the new Fock matrix  
c-----------------------------------------------------------------------

c-----------------------------------------------------------------------
c       One-electron piece  
c-----------------------------------------------------------------------

        do m = 1, n_basis 
        do n = 1, n_basis 
           FA(m,n) = h0(m,n)  
           FB(m,n) = h0(m,n)  
        enddo 
        enddo 

c-----------------------------------------------------------------------
c       Two-electron piece  
c-----------------------------------------------------------------------

         do m = m1, m2
            dom = .false. 
            do nf = 1, n_apf 
               if(atom(m) .eq. iatom(nf)) dom = .true.   
            enddo    
            if(dom) then  
            aa1 = beg_anfps(m)
            aa2 = end_anfps(m)

            x1 = tcoords(1,m)
            y1 = tcoords(2,m)
            z1 = tcoords(3,m)
         do n = n1, n2
            if (m .le. n) then 
            don = .false. 
            do nf = 1, n_apf 
               if(atom(n) .eq. iatom(nf)) don = .true.   
            enddo    
            if(don) then  
            bb1 = beg_anfps(n)
            bb2 = end_anfps(n)

            x2 = tcoords(1,n)
            y2 = tcoords(2,n)
            z2 = tcoords(3,n)
         do r = r1, r2
            dor = .false. 
            do nf = 1, n_apf 
               if(atom(r) .eq. iatom(nf)) dor = .true.   
            enddo    
            if(dor) then  
            cc1 = beg_anfps(r)
            cc2 = end_anfps(r)

            x3 = tcoords(1,r)
            y3 = tcoords(2,r)
            z3 = tcoords(3,r)
         do s = s1, s2
            if (r .le. s) then 
            dos = .false. 
            do nf = 1, n_apf 
               if(atom(s) .eq. iatom(nf)) dos = .true.   
            enddo    
            if(dos) then  
            dd1 = beg_anfps(s)
            dd2 = end_anfps(s)
c
c-----------------------------------------------------------------------
c   Determine the largest density element.
c-----------------------------------------------------------------------

               x4 = tcoords(1,s)
               y4 = tcoords(2,s)
               z4 = tcoords(3,s)
               call pack_coeffs(alphas, pixalphas, coeffs, pixpcoeffs, 
     *                          ncfps, npfps, m, n, 
     *                          r, s, alpha_pack, nalpha_pack, 
     *                          pcoeff_pack, npcoeff_pack, 
     *                          ccbeg, ccend, indx_cc,
     *                          ccbeg_pack, ccend_pack) 

c---------------------------------------------------------------------------
c   Calling sequence for ERD version 2.
c---------------------------------------------------------------------------

               ncsum = ncfps(m) + ncfps(n) + ncfps(r) + ncfps(s)

c              if (doit .eq. 1) then 

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

c               endif 

c---------------------------------------------------------------------------
c   Move the integrals into the output block.  
c---------------------------------------------------------------------------

           if (nints .gt. 0) then

               call form_ss1fock(scr(nfirst), n_basis,
     *                        aa1,aa2,bb1,bb2,cc1,cc2,dd1,dd2,
     *                        HFD_A,HFD_B,FA,FB)
               call form_ss2fock(scr(nfirst), n_basis,
     *                        aa1,aa2,bb1,bb2,cc1,cc2,dd1,dd2,
     *                        HFD_A,HFD_B,FA,FB)

               call form_ss3fock(scr(nfirst), n_basis,
     *                        aa1,aa2,bb1,bb2,cc1,cc2,dd1,dd2,
     *                        HFD_A,HFD_B,FA,FB)

               call form_ss4fock(scr(nfirst), n_basis,
     *                        aa1,aa2,bb1,bb2,cc1,cc2,dd1,dd2,
     *                        HFD_A,HFD_B,FA,FB)

               call form_ss5fock(scr(nfirst), n_basis,
     *                        aa1,aa2,bb1,bb2,cc1,cc2,dd1,dd2,
     *                        HFD_A,HFD_B,FA,FB)

               call form_ss6fock(scr(nfirst), n_basis,
     *                        aa1,aa2,bb1,bb2,cc1,cc2,dd1,dd2,
     *                        HFD_A,HFD_B,FA,FB)

               call form_ss7fock(scr(nfirst), n_basis,
     *                        aa1,aa2,bb1,bb2,cc1,cc2,dd1,dd2,
     *                        HFD_A,HFD_B,FA,FB)

               call form_ss8fock(scr(nfirst), n_basis,
     *                        aa1,aa2,bb1,bb2,cc1,cc2,dd1,dd2,
     *                        HFD_A,HFD_B,FA,FB)

           endif
c 
            endif 
            endif 
         enddo   ! s
            endif 
         enddo   ! r
            endif 
            endif 
         enddo   ! n
            endif 
         enddo   ! m

c-----------------------------------------------------------------------
c       Done computing the Fock matrix   
c-----------------------------------------------------------------------

c-----------------------------------------------------------------------
c       Compute the new HF energy  
c-----------------------------------------------------------------------

        call hfenergy(HFD_A,HFD_B,FA,FB,h0,n_basis) 

c-----------------------------------------------------------------------
c       Compute the DIIS coefficients  
c-----------------------------------------------------------------------

        call gen_hist(iter,maxhist,diis_start,n_basis,nvirt_a,
     *                nocc_a,nvirt_b,nocc_b,ca,cb,FA,FB,Fai,Fbj,Ea,Eb)

        if ((iter .lt. maxhist+diis_start-1) .and. 
     *      (iter .gt. diis_start)) then 
           nhist = iter - (diis_start-1)  
           call do_diis(nhist,nvirt_a,nocc_a,nvirt_b,nocc_b,n_basis,
     *                  Fai,Ea, Fbj,Eb, FA, FB) 
        endif 

        if (iter .ge. maxhist+diis_start-1) then 
           nhist = maxhist   
           call do_diis(nhist,nvirt_a,nocc_a,nvirt_b,nocc_b,n_basis,
     *                  Fai,Ea, Fbj,Eb, FA, FB) 
           call move_hist(nhist,diis_start,n_basis,Fa,Fb,Ea,Eb) 
        endif 

c-----------------------------------------------------------------------
c       Transpose the new Fock Matrix   
c-----------------------------------------------------------------------

        call fock_transpose(FA,FB,Qxx,FTa,FTb,n_basis) 

c-----------------------------------------------------------------------
c       Diagonalize the new Transposed Fock Matrix   
c-----------------------------------------------------------------------

        call diag(FTa,ca,m,n_basis,0,0,0) 
        call diag(FTb,cb,m,n_basis,0,0,0) 

        if (nocc_a .eq. nocc_b) then 
           do n = 1, n_basis
           do m = 1, n_basis
                  cb(m,n) = ca(m,n)  
           enddo 
           enddo 
        endif 

c-----------------------------------------------------------------------
c       Back Transform the coefficient array  
c-----------------------------------------------------------------------

        call c_backtran(Qxx,ca,cb,cba,cbb,n_basis) 

c-----------------------------------------------------------------------
c       Check on convergence and replace the old density with the new   
c-----------------------------------------------------------------------

c-----------------------------------------------------------------------
c       Compute the new HF density  
c-----------------------------------------------------------------------

        call hfdensity(ca,cb,HFD_A,HFD_B,n_basis,nocc_a,nocc_b) 

c-----------------------------------------------------------------------
c       Check for convergence    
c-----------------------------------------------------------------------

        call check_conv(HFD_A,HFD_B,HFDOLD_A,HFDOLD_B,n_basis,doit)  
        if (doit .eq. 1) go to 100 

c-----------------------------------------------------------------------
c Copy the HF Density into the old Density. 
c-----------------------------------------------------------------------

        call hfdensity_copy(HFD_A,HFD_B,HFDOLD_A,HFDOLD_B,n_basis)  


      ENDDO ! iter = 1, max_iter 
c If no convergence set contribution to density to zero???  
c     HFD_a(m,n) = 0.0  
c     HFD_b(m,n) = 0.0   
100   continue 

      if (current_frag .eq. 111111111111) then 
       write(6,*) ' Alpha Orbital energies ' 
       write(6,*) ' -----------------------------------------------' 
       do m = 1, nocc_a 
          write(6,*) '  ', m, FTA(m,m), FTA(m,m)*27.21138386 
       enddo  
       write(6,*) ' -----------------------------------------------' 
       do m = nocc_a + 1, n_basis  
          write(6,*) '  ', m, FTA(m,m), FTA(m,m)*27.21138386  
       enddo  
       write(6,*) ' -----------------------------------------------' 
       write(6,*) ' ' 

       write(6,*) ' Beta Orbital energies ' 
       write(6,*) ' -----------------------------------------------' 
       do m = 1, nocc_b 
          write(6,*) '  ', m, FTB(m,m), FTB(m,m)*27.21138386 
       enddo  
       write(6,*) ' -----------------------------------------------' 
       do m = nocc_b + 1, n_basis  
          write(6,*) '  ', m, FTB(m,m), FTB(m,m)*27.21138386  
       enddo  
       write(6,*) ' -----------------------------------------------' 
       write(6,*) ' ' 

       endif 
          write(6,*) ' Done SCF calculation of ATOM :', iatom 

c-----------------------------------------------------------------------
c     Put the transformation coefficients to the proper spot  
c-----------------------------------------------------------------------

c-----------------------------------------------------------------------
c        Alpha 
c-----------------------------------------------------------------------

      na_offset = baocc_frag(current_frag, 1) 
      do n = 1, nocc_a 
      do m = 1, n_basis
         if ((map(m) .ne. 0) .and. (map(n) .ne. 0)) then  
            ca_perm(map(m),na_offset+n-1) = Ca(m,n)  
         endif 
      enddo 
      enddo 

      na_offset = bavirt_frag(current_frag, 1) 
      do n = 1, n_basis - nocc_a  
      do m = 1, n_basis
         if ((map(m) .ne. 0) .and. (map(n) .ne. 0)) then  
            ca_perm(map(m),na_offset+n-1) = Ca(m,n+nocc_a)  
         endif 
      enddo 
      enddo 

c-----------------------------------------------------------------------
c        Beta 
c-----------------------------------------------------------------------

      nb_offset = bbocc_frag(current_frag, 1) 
      do n = 1, nocc_b 
      do m = 1, n_basis
         if ((map(m) .ne. 0) .and. (map(n) .ne. 0)) then  
            cb_perm(map(m),nb_offset+n-1) = Cb(m,n)  
         endif 
      enddo 
      enddo 

      nb_offset = bbvirt_frag(current_frag, 1) 
      do n = 1, n_basis - nocc_b  
      do m = 1, n_basis
         if ((map(m) .ne. 0) .and. (map(n) .ne. 0)) then  
            cb_perm(map(m),nb_offset+n-1) = Cb(m,n+nocc_b)  
         endif 
      enddo 
      enddo 


c-----------------------------------------------------------------------
c     Done transfering coefficients  
c-----------------------------------------------------------------------

c-----------------------------------------------------------------------
c     Put the Fock matrix to the proper spot  
c-----------------------------------------------------------------------

c-----------------------------------------------------------------------
c        ALPHA 
c-----------------------------------------------------------------------

      na_offset = baocc_frag(current_frag, 1) 
      ma_offset = baocc_frag(current_frag, 1) 
      do n = 1, nocc_a 
      do m = 1, nocc_a  
         fa_perm(ma_offset+m-1,na_offset+n-1) = 0.0  
         if ((map(m) .ne. 0) .and. (map(n) .ne. 0)) then  
            fa_perm(ma_offset+m-1,na_offset+n-1) = FTa(m,n)  
         endif 
      enddo 
      enddo 

      na_offset = baocc_frag(current_frag, 1) 
      ma_offset = bavirt_frag(current_frag, 1)  
      do n = 1, nocc_a 
      do m = 1, n_basis - nocc_a   
         fa_perm(ma_offset+m-1,na_offset+n-1) = 0.0  
         if ((map(m) .ne. 0) .and. (map(n) .ne. 0)) then  
            fa_perm(ma_offset+m-1,na_offset+n-1) = 
     *          FTa(m+nocc_a,n)  
         endif 
      enddo 
      enddo 

      ma_offset = baocc_frag(current_frag, 1) 
      na_offset = bavirt_frag(current_frag, 1)  
      do n = 1, n_basis - nocc_a    
      do m = 1, nocc_a 
         fa_perm(ma_offset+m-1,na_offset+n-1) = 0.0  
         if ((map(m) .ne. 0) .and. (map(n) .ne. 0)) then  
            fa_perm(ma_offset+m-1,na_offset+n-1) = 
     *          FTa(m,n+nocc_a)  
         endif 
      enddo 
      enddo 

      na_offset = bavirt_frag(current_frag, 1)  
      ma_offset = bavirt_frag(current_frag, 1)  
      do n = 1, n_basis - nocc_a    
      do m = 1, n_basis - nocc_a   
         fa_perm(ma_offset+m-1,na_offset+n-1) = 0.0  
         if ((map(m) .ne. 0) .and. (map(n) .ne. 0)) then  
            fa_perm(ma_offset+m-1,na_offset+n-1) =
     *          FTa(m+nocc_a,n+nocc_a)  
         endif 
      enddo 
      enddo 

c-----------------------------------------------------------------------
c        BETA 
c-----------------------------------------------------------------------
99    continue 

      nb_offset = bbocc_frag(current_frag, 1) 
      mb_offset = bbocc_frag(current_frag, 1) 
      do n = 1, nocc_b 
      do m = 1, nocc_b  
         fb_perm(mb_offset+m-1,nb_offset+n-1) = 0.0  
         if ((map(m) .ne. 0) .and. (map(n) .ne. 0)) then  
            fb_perm(mb_offset+m-1,nb_offset+n-1) = FTb(m,n)  
         endif 
      enddo 
      enddo 

      nb_offset = bbocc_frag(current_frag, 1) 
      mb_offset = bbvirt_frag(current_frag, 1)  
      do n = 1, nocc_b 
      do m = 1, n_basis - nocc_b    
         fb_perm(mb_offset+m-1,nb_offset+n-1) = 0.0  
         if ((map(m) .ne. 0) .and. (map(n) .ne. 0)) then  
            fb_perm(mb_offset+m-1,nb_offset+n-1) = 
     *          FTb(m+nocc_b,n)  
         endif 
      enddo 
      enddo 

      mb_offset = bbocc_frag(current_frag, 1) 
      nb_offset = bbvirt_frag(current_frag, 1)  
      do n = 1, n_basis - nocc_b   
      do m = 1, nocc_b 
         fb_perm(mb_offset+m-1,nb_offset+n-1) = 0.0  
         if ((map(m) .ne. 0) .and. (map(n) .ne. 0)) then  
            fb_perm(mb_offset+m-1,nb_offset+n-1) = 
     *          FTb(m,n+nocc_b)  
         endif 
      enddo 
      enddo 

      nb_offset = bbvirt_frag(current_frag, 1)  
      mb_offset = bbvirt_frag(current_frag, 1)  
      do n = 1, n_basis - nocc_b    
      do m = 1, n_basis - nocc_b   
         fb_perm(ma_offset+m-1,na_offset+n-1) = 0.0  
         if ((map(m) .ne. 0) .and. (map(n) .ne. 0)) then  
            fb_perm(mb_offset+m-1,nb_offset+n-1) =
     *          FTb(m+nocc_b,n+nocc_b)  
         endif 
      enddo 
      enddo 

      end
 

 


      subroutine form_fock(int_block,n_basis,aa1,aa2,bb1,bb2,cc1,cc2,
     *                     dd1,dd2,HFD_A,HFD_B,F_A,F_B)
      implicit none 
      integer aa1, aa2, bb1, bb2, cc1, cc2, dd1, dd2, n_basis 
      integer a, b, c, d 
      integer m, n, r, s 
      double precision int_block(aa1:aa2,bb1:bb2,cc1:cc2,dd1:dd2)  
      double precision HFD_A(n_basis,n_basis) 
      double precision HFD_B(n_basis,n_basis) 
      double precision F_A(n_basis,n_basis) 
      double precision F_B(n_basis,n_basis) 
      double precision tempa, tempb, temp_rs, temp_mn   

      do s = dd1, dd2 
      do r = cc1, cc2 
         temp_rs = HFD_A(r,s) + HFD_B(r,s) 
      do n = bb1, bb2 
      do m = aa1, aa2 
           temp_mn  = int_block(m,n,r,s)*temp_rs 
           F_A(m,n) = F_A(m,n) + temp_mn 
           F_B(m,n) = F_B(m,n) + temp_mn 
      enddo 
      enddo 
      enddo 
      enddo 

      do s = dd1, dd2 
      do r = cc1, cc2 
      do n = bb1, bb2 
         tempa = HFD_A(n,s) 
         tempb = HFD_B(n,s) 
      do m = aa1, aa2 
           F_A(m,r) = F_A(m,r) - int_block(m,n,r,s)*tempa ! HFD_A(n,s)  
           F_B(m,r) = F_B(m,r) - int_block(m,n,r,s)*tempb ! HFD_B(n,s)  
      enddo 
      enddo 
      enddo 
      enddo 

      return 
      end 

      subroutine form_sfock(int_block,n_basis,aa1,aa2,bb1,bb2,cc1,cc2,
     *                      dd1,dd2,HFD_A,HFD_B,F_A,F_B)
      implicit none 
      integer aa1, aa2, bb1, bb2, cc1, cc2, dd1, dd2, n_basis 
      integer a, b, c, d 
      integer m, n, r, s 
      double precision int_block(aa1:aa2,bb1:bb2,cc1:cc2,dd1:dd2)  
      double precision HFD_A(n_basis,n_basis) 
      double precision HFD_B(n_basis,n_basis) 
      double precision F_A(n_basis,n_basis) 
      double precision F_B(n_basis,n_basis) 
      double precision F1A(n_basis) 
      double precision F1B(n_basis) 
      double precision tempa, tempb, temp_rs, temp_mn   

c-----------------------------------------------------------------------
c m=n=r=s  
c-----------------------------------------------------------------------

c     if (m.eq.n .and. r.eq.s .and. n.eq.r) then 

         do s = dd1, dd2 
         do r = cc1, cc2 
            temp_rs = HFD_A(r,s) + HFD_B(r,s) 
         do n = bb1, bb2 
         do m = aa1, aa2 
              temp_mn  = int_block(m,n,r,s)*temp_rs 
              F_A(m,n) = F_A(m,n) + temp_mn 
              F_B(m,n) = F_B(m,n) + temp_mn 
         enddo 
         enddo 
         enddo 
         enddo 

         do s = dd1, dd2 
         do r = cc1, cc2 
            do m = aa1, aa2 
               F1A(m) = 0.0 
               F1B(m) = 0.0 
            enddo 
         do n = bb1, bb2 
            tempa = HFD_A(n,s) 
            tempb = HFD_B(n,s) 
         do m = aa1, aa2 
              F1A(m) = F1A(m) - int_block(m,n,r,s)*tempa ! HFD_A(n,s)  
              F1B(m) = F1B(m) - int_block(m,n,r,s)*tempb ! HFD_B(n,s)  
         enddo 
         enddo 
            do m = aa1, aa2 
               F_A(m,r) = F_A(m,r) + F1A(m)  
               F_B(m,r) = F_A(m,r) + F1B(m)  
            enddo 
         enddo 
         enddo 

c     endif 

c-----------------------------------------------------------------------
c  m.lt.n .and. r.lt.s .and. m.lt.r .and. n.ne.s .and. n.ne.r .and. m.ne.s   
c-----------------------------------------------------------------------

      if(m.lt.n .and. r.lt.s .and. m.lt.r .and. n.ne.s .and. 
     *   n.ne.r .and. m.ne.s) then  

      endif 

      return 
      end 

      subroutine fock_transpose(Fa,FB,Qxx,FTa,FTb,n_basis) 
      implicit none 
      integer n_basis, m, n, l, s  
      double precision Fa(n_basis,n_basis)  
      double precision Fb(n_basis,n_basis)  
      double precision FTa(n_basis,n_basis)  
      double precision FTb(n_basis,n_basis)  
      double precision Qxx(n_basis,n_basis)  
      double precision tempa, tempb 

      do m = 1, n_basis 
      do n = 1, n_basis 
         FTa(m,n) = 0.0  
         FTb(m,n) = 0.0  
      enddo 
      enddo 

      do m = 1, n_basis  
      do s = 1, n_basis  
         tempa = 0.0 
         tempb = 0.0 
         do l = 1, n_basis  
            tempa = tempa + Qxx(l,m)*FA(l,s) 
            tempb = tempb + Qxx(l,m)*FB(l,s) 
         enddo 

         do n = 1, n_basis  
            FTa(m,n) = FTa(m,n) + tempa*Qxx(s,n) 
            FTb(m,n) = FTb(m,n) + tempb*Qxx(s,n) 
         enddo 
      enddo 
      enddo 

      return 
      end 

      subroutine c_backtran(Qxx,ca,cb,cba,cbb,n_basis) 
      implicit none 
      integer n_basis 
      integer m, p, n  
      double precision Qxx(n_basis,n_basis)  
      double precision ca(n_basis,n_basis)  
      double precision cb(n_basis,n_basis)  
      double precision cba(n_basis,n_basis)  
      double precision cbb(n_basis,n_basis) 
      double precision tempa, tempb  

      do p = 1, n_basis  
      do m = 1, n_basis  
         tempa = 0.0  
         tempb = 0.0  
         do n = 1, n_basis  
            tempa = tempa + Qxx(m,n)*ca(n,p) 
            tempb = tempb + Qxx(m,n)*cb(n,p) 
         enddo 
         cba(m,p) = tempa
         cbb(m,p) = tempb
      enddo 
      enddo 

      do p = 1, n_basis  
      do m = 1, n_basis  
         ca(m,p) = cba(m,p) 
         cb(m,p) = cbb(m,p) 
      enddo 
      enddo 

      return 
      end 


      subroutine hfdensity(ca,cb,HFD_A,HFD_B,n_basis,nocc_a,nocc_b) 
      implicit none 
      integer n_basis, nocc_a, nocc_b 
      integer m, n, i, j 
      double precision ca(n_basis,n_basis) 
      double precision cb(n_basis,n_basis) 
      double precision HFD_A(n_basis,n_basis) 
      double precision HFD_B(n_basis,n_basis) 
      double precision tempa, tempb  

      do m = 1, n_basis  
      do n = 1, n_basis  
         tempa = 0.0 
         tempb = 0.0 
         do i = 1, nocc_a 
            tempa = tempa + ca(m,i)*ca(n,i) 
         enddo 
         do j = 1, nocc_b 
            tempb = tempb + cb(m,j)*cb(n,j) 
         enddo 
         HFD_A(m,n) = tempa 
         HFD_B(m,n) = tempb 
      enddo 
      enddo 

      return 
      end 

      subroutine hfenergy(HFD_A,HFD_B,FA,FB,h0,n_basis) 
      implicit none 
      integer n_basis 
      integer m, n 
      double precision HFD_A(n_basis,n_basis) 
      double precision HFD_B(n_basis,n_basis) 
      double precision FA(n_basis,n_basis) 
      double precision FB(n_basis,n_basis) 
      double precision h0(n_basis,n_basis) 
      double precision ea, eb, etotal 

      ea = 0.0 
      eb = 0.0 
      etotal = 0.0 

      do n = 1, n_basis  
      do m = 1, n_basis  
         ea = ea + (h0(m,n)+FA(m,n))*HFD_A(m,n)   
         eb = eb + (h0(m,n)+FB(m,n))*HFD_B(m,n)   
      enddo  
      enddo  

      etotal = 0.5d0*(ea + eb) 
      write(6,*) ' Total SCF energy(-NN) = ', etotal 

      return 
      end 
   
c
c ---------------------------------------------------------------------------- 

      subroutine form_ss1fock(int_block,n_basis,aa1,aa2,bb1,bb2,cc1,cc2,
     *                      dd1,dd2,HFD_A,HFD_B,F_A,F_B)
      implicit none 
      integer aa1, aa2, bb1, bb2, cc1, cc2, dd1, dd2, n_basis 
      integer a, b, c, d 
      integer m, n, r, s, l  
      double precision int_block(aa1:aa2,bb1:bb2,cc1:cc2,dd1:dd2)  
      double precision HFD_A(n_basis,n_basis) 
      double precision HFD_B(n_basis,n_basis) 
      double precision F_A(n_basis,n_basis) 
      double precision F_B(n_basis,n_basis) 
      double precision F1A(n_basis) 
      double precision F1B(n_basis) 
      double precision tempa, tempb, temp_rs, temp_mn   

      double precision wint  
      double precision t1xx
      double precision t2xx
      double precision t3xx
      double precision t4xx
      double precision t5xx
      double precision t6xx
      double precision t7xx
      double precision t8xx
      double precision t9xx
      double precision t10xx
      double precision t11xx
      double precision t12xx
      double precision t13xx
      double precision t14xx
      double precision t15xx
      double precision t16xx
      double precision t17xx
      double precision t18xx
      double precision t19xx
      double precision t20xx
      double precision t21xx
      double precision t22xx
c
c
c############ CLASS 6  ############
c SS1 
c#  Do four-center part (mu nu |la si)->(mu nu |si la ),(nu mu|la si),(nu mu|si la)
c#                      (la si |mu nu),(si la |mu nu),(la si |nu mu),(si la |nu mu)
c
      do s = dd1, dd2 
      do l = cc1, cc2 
         if ( l .lt. s) then 
            T1xx = HFD_A(l,s) + HFD_B(l,s)
            T5xx = 0.0 
      do n = bb1, bb2 
         if ( n .ne. s) then 
         if ( n .ne. l) then 
            T11xx = 0.0 
            T13xx = 0.0 
            T19xx = 0.0 
            T21xx = 0.0 
      do m = aa1, aa2 
c
         if ( m .lt. n) then 
         if ( m .lt. l) then 
         if ( m .ne. s) then 

            wint      = int_block(m,n,l,s) 
            T4xx      = HFD_A(m,n) + HFD_B(m,n)
            T2xx      = 2.0*wint*T1xx
            T5xx      = T5xx + 2.0*wint*T4xx
            T7xx      = wint*HFD_A(n,s)
            T9xx      = wint*HFD_A(n,l)
            T11xx     = T11xx + wint*HFD_A(m,s)
            T13xx     = T13xx + wint*HFD_A(m,l)
            T19xx     = T19xx + wint*HFD_B(m,s)
            T21xx     = T21xx + wint*HFD_b(m,l)

            F_a(m,n)  = F_a(m,n) + T2xx
            F_b(m,n)  = F_b(m,n) + T2xx
            F_a(n,m)  = F_a(n,m) + T2xx
            F_b(n,m)  = F_b(n,m) + T2xx

            F_A(m,l) = F_a(m,l) - T7xx
            F_A(l,m) = F_a(l,m) - T7xx

            F_a(m,s) = F_a(m,s) - T9xx
            F_A(s,m) = F_a(s,m) - T9xx

            T15xx    = wint*HFD_B(n,s)
            F_B(m,l) = F_b(m,l) - T15xx
            F_B(l,m) = F_b(l,m) - T15xx

            T17xx    = wint*HFD_B(n,l)
            F_b(m,s) = F_b(m,s) - T17xx
            F_b(s,m) = F_b(s,m) - T17xx

         endif 
         endif 
         endif 

      enddo ! m  
            F_A(n,l) = F_a(n,l) - T11xx
            F_A(l,n) = F_a(l,n) - T11xx
            F_A(n,s) = F_a(n,s) - T13xx
            F_A(s,n) = F_a(s,n) - T13xx
            F_b(n,l) = F_b(n,l) - T19xx
            F_b(l,n) = F_b(l,n) - T19xx
            F_b(n,s) = F_b(n,s) - T21xx
            F_b(s,n) = F_b(s,n) - T21xx
         endif 
         endif 
      enddo ! n  
            F_a(l,s) = F_a(l,s) + T5xx
            F_b(l,s) = F_b(l,s) + T5xx
            F_a(s,l) = F_a(s,l) + T5xx
            F_b(s,l) = F_b(s,l) + T5xx
         endif 
      enddo ! l  
      enddo ! s  
c
      return 
      end 
c
c ---------------------------------------------------------------------------- 

      subroutine form_ss2fock(int_block,n_basis,aa1,aa2,bb1,bb2,cc1,cc2,
     *                      dd1,dd2,HFD_A,HFD_B,F_A,F_B)
      implicit none 
      integer aa1, aa2, bb1, bb2, cc1, cc2, dd1, dd2, n_basis 
      integer a, b, c, d 
      integer m, n, r, s, l  
      double precision int_block(aa1:aa2,bb1:bb2,cc1:cc2,dd1:dd2)  
      double precision HFD_A(n_basis,n_basis) 
      double precision HFD_B(n_basis,n_basis) 
      double precision F_A(n_basis,n_basis) 
      double precision F_B(n_basis,n_basis) 
      double precision F1A(n_basis) 
      double precision F1B(n_basis) 
      double precision tempa, tempb, temp_rs, temp_mn   

      double precision wint  
      double precision t1xx
      double precision t2xx
      double precision t3xx
      double precision t4xx
      double precision t5xx
      double precision t6xx
      double precision t7xx
      double precision t8xx
      double precision t9xx
      double precision t10xx
      double precision t11xx
      double precision t12xx
      double precision t13xx
      double precision t14xx
      double precision t15xx
      double precision t16xx
      double precision t17xx
      double precision t18xx
      double precision t19xx
      double precision t20xx
      double precision t21xx
      double precision t22xx
c
c########### CLASS 3  ############
c SS2 
c  Do two-center part (m m |m n)->(m n |m m ),(m m|n m),(n m|m m)
c  Do three-center part (m m |n l)->(m m |l n ),(n l|m m),(l n|m m)
c
      do s = dd1, dd2 
      do l = cc1, cc2 
         if (l .lt. s) then 
             T1xx = HFD_a(l,s) + HFD_b(l,s)
             T4xx = 0.0  
      do n = bb1, bb2 
      do m = aa1, aa2 
         if (m .eq. n) then 

             wint     = int_block(m,n,l,s) 
             T3xx     = HFD_a(m,n) + HFD_b(m,n)
             T2xx     = 2.0*wint*T1xx
             T4xx     = T4xx + wint*T3xx
             T6xx     = wint*HFD_a(n,s)
             T8xx     = wint*HFD_a(n,l)
             T10xx    = wint*HFD_b(n,s)
             T12xx    = wint*HFD_b(n,l)

             F_a(m,n) = F_a(m,n) + T2xx
             F_b(m,n) = F_b(m,n) + T2xx

             F_a(m,l) = F_a(m,l) - T6xx
             F_a(l,m) = F_a(l,m) - T6xx

             F_a(m,s) = F_a(m,s) - T8xx
             F_a(s,m) = F_a(s,m) - T8xx

             F_b(m,l) = F_b(m,l) - T10xx
             F_b(l,m) = F_b(l,m) - T10xx

             F_b(m,s) = F_b(m,s) - T12xx
             F_b(s,m) = F_b(s,m) - T12xx

         endif 
      enddo 
      enddo 
             F_a(l,s) = F_a(l,s) + T4xx
             F_b(l,s) = F_b(l,s) + T4xx
             F_a(s,l) = F_a(s,l) + T4xx
             F_b(s,l) = F_b(s,l) + T4xx
         endif 
      enddo 
      enddo 

c ---------------------------------------------------------------------------- 
c
      return 
      end 
c
c ---------------------------------------------------------------------------- 

      subroutine form_ss3fock(int_block,n_basis,aa1,aa2,bb1,bb2,cc1,cc2,
     *                      dd1,dd2,HFD_A,HFD_B,F_A,F_B)
      implicit none 
      integer aa1, aa2, bb1, bb2, cc1, cc2, dd1, dd2, n_basis 
      integer a, b, c, d 
      integer m, n, r, s, l  
      double precision int_block(aa1:aa2,bb1:bb2,cc1:cc2,dd1:dd2)  
      double precision HFD_A(n_basis,n_basis) 
      double precision HFD_B(n_basis,n_basis) 
      double precision F_A(n_basis,n_basis) 
      double precision F_B(n_basis,n_basis) 
      double precision F1A(n_basis) 
      double precision F1B(n_basis) 
      double precision tempa, tempb, temp_rs, temp_mn   

      double precision wint  
      double precision t1xx
      double precision t2xx
      double precision t3xx
      double precision t4xx
      double precision t5xx
      double precision t6xx
      double precision t7xx
      double precision t8xx
      double precision t9xx
      double precision t10xx
      double precision t11xx
      double precision t12xx
      double precision t13xx
      double precision t14xx
      double precision t15xx
      double precision t16xx
      double precision t17xx
      double precision t18xx
      double precision t19xx
      double precision t20xx
      double precision t21xx
      double precision t22xx
c
c########### CLASS 5  ############
c##################  CLASS A ##############
c SS3 
c  Do three-center part (m n |m la)->(m n |la m ),(n m|m la),(n m|la m)
c                       (m la |m n),(la m |m n ),(m la |n m),(la m |n m)
c
      do s = dd1, dd2 
      do l = cc1, cc2 
         if ( l .lt. s) then 
            T1xx = HFD_a(l,s) + HFD_b(l,s)
            T5xx = 0.0 
      do n = bb1, bb2 
         if ( n .lt. s) then 
            T19xx = 0.0 
            T21xx = 0.0 
            T11xx = 0.0 
            T13xx = 0.0 
      do m = aa1, aa2 
         if ( m .lt. n) then 
         if ( m .eq. l) then 

                wint     = int_block(m,n,l,s) 
                T4xx     = HFD_a(m,n) + HFD_b(m,n)
                T2xx     = 2.0*wint*T1xx
                T5xx     = T5xx + 2.0*wint*T4xx
                T7xx     = wint*HFD_a(n,s)
                T9xx     = wint*HFD_a(n,l)
                T11xx    = T11xx + wint*HFD_a(m,s)
                T13xx    = T13xx + wint*HFD_a(m,l)
                T15xx    = wint*HFD_b(n,s)
                T17xx    = wint*HFD_b(n,l)
                T19xx    = T19xx + wint*HFD_b(m,s)
                T21xx    = T21xx + wint*HFD_b(m,l)

                F_a(m,n) = F_a(m,n) + T2xx
                F_b(m,n) = F_b(m,n) + T2xx
                F_a(n,m) = F_a(n,m) + T2xx
                F_b(n,m) = F_b(n,m) + T2xx

                F_a(m,l) = F_a(m,l) - T7xx
                F_a(l,m) = F_a(l,m) - T7xx
                F_a(m,s) = F_a(m,s) - T9xx
                F_a(s,m) = F_a(s,m) - T9xx

                F_b(m,l) = F_b(m,l) - T15xx
                F_b(l,m) = F_b(l,m) - T15xx
                F_b(m,s) = F_b(m,s) - T17xx
                F_b(s,m) = F_b(s,m) - T17xx
c
         endif 
         endif 
      enddo ! m  
            F_b(n,l) = F_b(n,l) - T19xx
            F_b(l,n) = F_b(l,n) - T19xx
            F_b(n,s) = F_b(n,s) - T21xx
            F_b(s,n) = F_b(s,n) - T21xx
            F_a(n,l) = F_a(n,l) - T11xx
            F_a(l,n) = F_a(l,n) - T11xx
            F_a(n,s) = F_a(n,s) - T13xx
            F_a(s,n) = F_a(s,n) - T13xx
         endif 
      enddo ! n  
            F_a(l,s) = F_a(l,s) + T5xx
            F_b(l,s) = F_b(l,s) + T5xx
            F_a(s,l) = F_a(s,l) + T5xx
            F_b(s,l) = F_b(s,l) + T5xx
         endif 
      enddo ! l  
      enddo ! s  
c
      return 
      end 
c
c ---------------------------------------------------------------------------- 

      subroutine form_ss4fock(int_block,n_basis,aa1,aa2,bb1,bb2,cc1,cc2,
     *                      dd1,dd2,HFD_A,HFD_B,F_A,F_B)
      implicit none 
      integer aa1, aa2, bb1, bb2, cc1, cc2, dd1, dd2, n_basis 
      integer a, b, c, d 
      integer m, n, r, s, l  
      double precision int_block(aa1:aa2,bb1:bb2,cc1:cc2,dd1:dd2)  
      double precision HFD_A(n_basis,n_basis) 
      double precision HFD_B(n_basis,n_basis) 
      double precision F_A(n_basis,n_basis) 
      double precision F_B(n_basis,n_basis) 
      double precision F1A(n_basis) 
      double precision F1B(n_basis) 
      double precision tempa, tempb, temp_rs, temp_mn   

      double precision wint  
      double precision t1xx
      double precision t2xx
      double precision t3xx
      double precision t4xx
      double precision t5xx
      double precision t6xx
      double precision t7xx
      double precision t8xx
      double precision t9xx
      double precision t10xx
      double precision t11xx
      double precision t12xx
      double precision t13xx
      double precision t14xx
      double precision t15xx
      double precision t16xx
      double precision t17xx
      double precision t18xx
      double precision t19xx
      double precision t20xx
      double precision t21xx
      double precision t22xx
c
c##################  CLASS B ##############
c SS4 
c  Do three-center part (m n |m la)->(m n |la m ),(n m|m la),(n m|la m)
c                       (m la |m n),(la m |m n ),(m la |n m),(la m |n m)
c
      do s = dd1, dd2 
      do l = cc1, cc2 
         if ( l .lt. s) then 
            T1xx = HFD_a(l,s) + HFD_b(l,s)
            T11xx = 0.0 
            T13xx = 0.0 
            T5xx  = 0.0 
      do n = bb1, bb2 
         if ( n .eq. l) then 
            T19xx = 0.0 
            T21xx = 0.0 
      do m = aa1, aa2 
         if ( m .lt.  n) then 
         if ( m .lt. s) then 

             wint     = int_block(m,n,l,s) 
             T4xx     = HFD_a(m,n) + HFD_b(m,n)
             T2xx     = 2.0*wint*T1xx
             T7xx     = wint*HFD_a(n,s)
             T9xx     = wint*HFD_a(n,l)
             T15xx    = wint*HFD_b(n,s)
             T17xx    = wint*HFD_b(n,l)

             T19xx    = T19xx + wint*HFD_b(m,s)
             T21xx    = T21xx + wint*HFD_b(m,l)

             T5xx     = T5xx + 2.0*wint*T4xx
             T11xx    = T11xx + wint*HFD_a(m,s)
             T13xx    = T13xx + wint*HFD_a(m,l)

             F_a(m,n) = F_a(m,n) + T2xx
             F_b(m,n) = F_b(m,n) + T2xx
             F_a(n,m) = F_a(n,m) + T2xx
             F_b(n,m) = F_b(n,m) + T2xx

             F_a(m,l) = F_a(m,l) - T7xx
             F_a(l,m) = F_a(l,m) - T7xx

             F_a(m,s) = F_a(m,s) - T9xx
             F_a(s,m) = F_a(s,m) - T9xx

             F_b(m,l) = F_b(m,l) - T15xx
             F_b(l,m) = F_b(l,m) - T15xx

             F_b(m,s) = F_b(m,s) - T17xx
             F_b(s,m) = F_b(s,m) - T17xx
c
         endif 
         endif 
      enddo ! m  
             F_a(n,l) = F_a(n,l) - T11xx
             F_a(l,n) = F_a(l,n) - T11xx
             F_a(n,s) = F_a(n,s) - T13xx
             F_a(s,n) = F_a(s,n) - T13xx
             F_b(n,l) = F_b(n,l) - T19xx
             F_b(l,n) = F_b(l,n) - T19xx
             F_b(n,s) = F_b(n,s) - T21xx
             F_b(s,n) = F_b(s,n) - T21xx
         endif 
      enddo ! n  
             F_a(l,s) =  F_a(l,s) + T5xx
             F_b(l,s) =  F_b(l,s) + T5xx
             F_a(s,l) =  F_a(s,l) + T5xx
             F_b(s,l) =  F_b(s,l) + T5xx
         endif 
      enddo ! l  
      enddo ! s  
c
      return 
      end 
c
c ---------------------------------------------------------------------------- 

      subroutine form_ss5fock(int_block,n_basis,aa1,aa2,bb1,bb2,cc1,cc2,
     *                      dd1,dd2,HFD_A,HFD_B,F_A,F_B)
      implicit none 
      integer aa1, aa2, bb1, bb2, cc1, cc2, dd1, dd2, n_basis 
      integer a, b, c, d 
      integer m, n, r, s, l  
      double precision int_block(aa1:aa2,bb1:bb2,cc1:cc2,dd1:dd2)  
      double precision HFD_A(n_basis,n_basis) 
      double precision HFD_B(n_basis,n_basis) 
      double precision F_A(n_basis,n_basis) 
      double precision F_B(n_basis,n_basis) 
      double precision F1A(n_basis) 
      double precision F1B(n_basis) 
      double precision tempa, tempb, temp_rs, temp_mn   

      double precision wint  
      double precision t1xx
      double precision t2xx
      double precision t3xx
      double precision t4xx
      double precision t5xx
      double precision t6xx
      double precision t7xx
      double precision t8xx
      double precision t9xx
      double precision t10xx
      double precision t11xx
      double precision t12xx
      double precision t13xx
      double precision t14xx
      double precision t15xx
      double precision t16xx
      double precision t17xx
      double precision t18xx
      double precision t19xx
      double precision t20xx
      double precision t21xx
      double precision t22xx
c
c##################  CLASS C ##############
c SS5 
c  Do three-center part (m n |m la)->(m n |la m ),(n m|m la),(n m|la m)
c                       (m la |m n),(la m |m n ),(m la |n m),(la m |n m)
c
      do s = dd1, dd2 
      do l = cc1, cc2 
         if ( l .lt. s) then 
            T1xx = HFD_a(l,s) + HFD_b(l,s)
            T5xx = 0.0 
      do n = bb1, bb2 
         if ( n .eq. s) then 
            T11xx = 0.0 
            T13xx = 0.0 
            T19xx = 0.0 
            T21xx = 0.0 
      do m = aa1, aa2 
         if ( m .lt. n) then 
         if ( m .lt. l) then 
c 
             wint     = int_block(m,n,l,s) 
             T4xx     = HFD_a(m,n) + HFD_b(m,n)
             T2xx     = 2.0*wint*T1xx
             T7xx     = wint*HFD_a(n,s)
             T9xx     = wint*HFD_a(n,l)
             T15xx    = wint*HFD_b(n,s)
             T17xx    = wint*HFD_b(n,l)

             T11xx    = T11xx + wint*HFD_a(m,s)
             T13xx    = T13xx + wint*HFD_a(m,l)
             T5xx     = T5xx + 2.0*wint*T4xx
             T19xx    = T19xx + wint*HFD_b(m,s)
             T21xx    = T21xx + wint*HFD_b(m,l)

             F_a(m,n) = F_a(m,n) + T2xx
             F_a(n,m) = F_a(n,m) + T2xx
             F_b(m,n) = F_b(m,n) + T2xx
             F_b(n,m) = F_b(n,m) + T2xx

             F_a(m,l) = F_a(m,l) - T7xx
             F_a(l,m) = F_a(l,m) - T7xx

             F_a(m,s) = F_a(m,s) - T9xx
             F_a(s,m) = F_a(s,m) - T9xx

             F_b(m,l) = F_b(m,l) - T15xx
             F_b(l,m) = F_b(l,m) - T15xx

             F_b(m,s) = F_b(m,s) - T17xx
             F_b(s,m) = F_b(s,m) - T17xx
c
         endif 
         endif 
      enddo ! m  
            F_b(n,l) = F_b(n,l) - T19xx
            F_b(l,n) = F_b(l,n) - T19xx
            F_a(n,l) = F_a(n,l) - T11xx
            F_a(l,n) = F_a(l,n) - T11xx
            F_a(n,s) = F_a(n,s) - T13xx
            F_a(s,n) = F_a(s,n) - T13xx
            F_b(n,s) = F_b(n,s) - T21xx
            F_b(s,n) = F_b(s,n) - T21xx
         endif 
      enddo ! n  
            F_a(l,s) = F_a(l,s) + T5xx
            F_b(l,s) = F_b(l,s) + T5xx
            F_a(s,l) = F_a(s,l) + T5xx
            F_b(s,l) = F_b(s,l) + T5xx
         endif 
      enddo ! l  
      enddo ! s  
c
      return 
      end 
c
c ---------------------------------------------------------------------------- 

      subroutine form_ss6fock(int_block,n_basis,aa1,aa2,bb1,bb2,cc1,cc2,
     *                      dd1,dd2,HFD_A,HFD_B,F_A,F_B)
      implicit none 
      integer aa1, aa2, bb1, bb2, cc1, cc2, dd1, dd2, n_basis 
      integer a, b, c, d 
      integer m, n, r, s, l  
      double precision int_block(aa1:aa2,bb1:bb2,cc1:cc2,dd1:dd2)  
      double precision HFD_A(n_basis,n_basis) 
      double precision HFD_B(n_basis,n_basis) 
      double precision F_A(n_basis,n_basis) 
      double precision F_B(n_basis,n_basis) 
      double precision F1A(n_basis) 
      double precision F1B(n_basis) 
      double precision tempa, tempb, temp_rs, temp_mn   

      double precision wint  
      double precision t1xx
      double precision t2xx
      double precision t3xx
      double precision t4xx
c
c########### CLASS 1  ############
c SS6 
c  Do one-center part (m m |m m)
c
      do s = dd1, dd2 
      do l = cc1, cc2 
            T1xx = HFD_a(l,s) + HFD_b(l,s)
      do n = bb1, bb2 
      do m = aa1, aa2 
         if ( n .eq. m ) then 
         if ( s .eq. m ) then 
         if ( l .eq. m ) then 
        
            wint     = int_block(m,n,l,s) 
            T2xx     = wint*T1xx
            T3xx     = wint*HFD_a(n,s)
            T4xx     = wint*HFD_b(n,s)

            F_a(m,n) = F_a(m,n) + T2xx
            F_b(m,n) = F_b(m,n) + T2xx
            F_a(m,l) = F_a(m,l) - T3xx
            F_b(m,l) = F_b(m,l) - T4xx
c
         endif 
         endif 
         endif 
      enddo ! m  
      enddo ! n  
      enddo ! l  
      enddo ! s   
c
      return 
      end 
c
c ---------------------------------------------------------------------------- 

      subroutine form_ss7fock(int_block,n_basis,aa1,aa2,bb1,bb2,cc1,cc2,
     *                      dd1,dd2,HFD_A,HFD_B,F_A,F_B)
      implicit none 
      integer aa1, aa2, bb1, bb2, cc1, cc2, dd1, dd2, n_basis 
      integer a, b, c, d 
      integer m, n, r, s, l  
      double precision int_block(aa1:aa2,bb1:bb2,cc1:cc2,dd1:dd2)  
      double precision HFD_A(n_basis,n_basis) 
      double precision HFD_B(n_basis,n_basis) 
      double precision F_A(n_basis,n_basis) 
      double precision F_B(n_basis,n_basis) 
      double precision F1A(n_basis) 
      double precision F1B(n_basis) 
      double precision tempa, tempb, temp_rs, temp_mn   

      double precision wint  
      double precision t1xx
      double precision t2xx
      double precision t3xx
      double precision t4xx
      double precision t5xx
      double precision t6xx
      double precision t7xx
c
c########### CLASS 2  ############
c SS7 
c  Do two-center part (m m |n n)->(n n |m m )
c
      do s = dd1, dd2 
      do l = cc1, cc2 
         if ( l .eq. s) then 
            T1xx = HFD_a(l,s) + HFD_b(l,s)
            T4xx = 0.0 
      do n = bb1, bb2 
      do m = aa1, aa2 
         if ( m .eq. n) then 
         if ( m .lt. l) then 
 
             wint     = int_block(m,n,l,s) 
             T3xx     = HFD_a(m,n) + HFD_b(m,n)
             T2xx     = wint*T1xx
             T4xx     = T4xx + wint*T3xx
             T5xx     = wint*HFD_a(n,s)
             T7xx     = wint*HFD_b(n,s)

             F_a(m,n) = F_a(m,n) + T2xx
             F_b(m,n) = F_b(m,n) + T2xx

             F_a(m,l) = F_a(m,l) - T5xx
             F_a(l,m) = F_a(l,m) - T5xx

             F_b(m,l) = F_b(m,l) - T7xx
             F_b(l,m) = F_b(l,m) - T7xx
c
         endif 
         endif 
c
      enddo ! m  
      enddo ! n  
            F_a(l,s) = F_a(l,s) + T4xx
            F_b(l,s) = F_b(l,s) + T4xx
         endif 
      enddo ! l  
      enddo ! s  
c
c########### END CLASS 2  ############
c
      return 
      end 
c
c ---------------------------------------------------------------------------- 

      subroutine form_ss8fock(int_block,n_basis,aa1,aa2,bb1,bb2,cc1,cc2,
     *                      dd1,dd2,HFD_A,HFD_B,F_A,F_B)
      implicit none 
      integer aa1, aa2, bb1, bb2, cc1, cc2, dd1, dd2, n_basis 
      integer a, b, c, d 
      integer m, n, r, s, l  
      double precision int_block(aa1:aa2,bb1:bb2,cc1:cc2,dd1:dd2)  
      double precision HFD_A(n_basis,n_basis) 
      double precision HFD_B(n_basis,n_basis) 
      double precision F_A(n_basis,n_basis) 
      double precision F_B(n_basis,n_basis) 
      double precision F1A(n_basis) 
      double precision F1B(n_basis) 
      double precision tempa, tempb, temp_rs, temp_mn   

      double precision wint  
      double precision t1xx
      double precision t2xx
      double precision t3xx
      double precision t4xx
      double precision t5xx
      double precision t6xx
      double precision t7xx
      double precision t8xx
      double precision t9xx
      double precision t10xx
      double precision t11xx
c
c########### CLASS 4  ############
c SS8 
c  Do two-center part (m n |m n)->(m n |n m ),(n m|m n),(n m|n m)
c
      do s = dd1, dd2 
      do l = cc1, cc2 
         if ( l .lt. s ) then 
            T1xx = HFD_a(l,s) + HFD_b(l,s)
      do n = bb1, bb2 
         if ( n .eq. s) then 
            T6xx  = 0.0 
            T7xx  = 0.0 
            T10xx = 0.0 
            T11xx = 0.0 
      do m = aa1, aa2 
         if ( m .lt. n) then 
         if ( m .eq. l) then 
 
             wint     = int_block(m,n,l,s) 
             T2xx     = 2.0*wint*T1xx

             T4xx     = wint*HFD_a(n,s)
             T5xx     = wint*HFD_a(n,l)
             T8xx     = wint*HFD_b(n,s)
             T9xx     = wint*HFD_b(n,l)

             T6xx     = T6xx  + wint*HFD_a(m,s)
             T7xx     = T7xx  + wint*HFD_a(m,l)
             T10xx    = T10xx + wint*HFD_b(m,s)
             T11xx    = T11xx + wint*HFD_b(m,l)
 
             F_a(m,n) = F_a(m,n) + T2xx
             F_b(m,n) = F_b(m,n) + T2xx
             F_a(n,m) = F_a(n,m) + T2xx
             F_b(n,m) = F_b(n,m) + T2xx

             F_a(m,l) = F_a(m,l) - T4xx
             F_a(m,s) = F_a(m,s) - T5xx
             F_b(m,l) = F_b(m,l) - T8xx
             F_b(m,s) = F_b(m,s) - T9xx
c
         endif 
         endif 
      enddo ! m  
            F_a(n,l) = F_a(n,l) - T6xx
            F_a(n,s) = F_a(n,s) - T7xx
            F_b(n,l) = F_b(n,l) - T10xx
            F_b(n,s) = F_b(n,s) - T11xx
         endif 
      enddo ! n  
         endif 
      enddo ! l  
      enddo ! s  
c
c ---------------------------------------------------------------------------- 
c
      return 
      end 

      subroutine form_ss(int_block,m,n,r,s,n_basis,aa1,aa2,bb1,bb2,
     *                   cc1,cc2,dd1,dd2)
      implicit none 
      integer aa1, aa2, bb1, bb2, cc1, cc2, dd1, dd2, n_basis 
      integer a, b, c, d 
      integer m, n, r, s   
      double precision int_block(aa1:aa2,bb1:bb2,cc1:cc2,dd1:dd2)  
      double precision temp_block(aa1:aa2,bb1:bb2,cc1:cc2,dd1:dd2)  

      if ((m .lt. n) .and. (r .lt. s)) then 
         do a = aa1, aa2 
         do b = bb1, bb2 
         do c = cc1, cc2 
         do d = dd1, dd2 
            int_block(b,a,c,d) = int_block(a,b,c,d)  
            int_block(a,b,d,c) = int_block(a,b,c,d)  
            int_block(b,a,d,c) = int_block(a,b,c,d)  
         enddo 
         enddo 
         enddo 
         enddo 
      endif 

      if ((m .lt. n) .and. (r .eq. s)) then 
         do a = aa1, aa2 
         do b = bb1, bb2 
         do c = cc1, cc2 
         do d = dd1, dd2 
            int_block(b,a,c,d) = int_block(a,b,c,d)  
            int_block(a,b,d,c) = int_block(a,b,c,d)  
         enddo 
         enddo 
         enddo 
         enddo 
      endif 

      if ((m .eq. n) .and. (r .lt. s)) then 
         do a = aa1, aa2 
         do b = bb1, bb2 
         do c = cc1, cc2 
         do d = dd1, dd2 
            int_block(a,b,d,c) = int_block(a,b,c,d)  
            int_block(b,a,d,c) = int_block(a,b,c,d)  
         enddo 
         enddo 
         enddo 
         enddo 
      endif 

      return 
      end 

      subroutine hfdensity_copy(HFD_A,HFD_B,HFDOLD_A,HFDOLD_B,n_basis)  
      implicit none 

      integer n_basis, a, b 
      double precision HFD_A(n_basis,n_basis) 
      double precision HFD_B(n_basis,n_basis) 
      double precision HFDOLD_A(n_basis,n_basis) 
      double precision HFDOLD_B(n_basis,n_basis) 

      do a = 1, n_basis 
      do b = 1, n_basis 
         HFDOLD_A(b,a) = HFD_A(b,a) 
         HFDOLD_B(b,a) = HFD_B(b,a) 
      enddo 
      enddo 

      return 
      end 


      subroutine check_conv(HFD_A,HFD_B,HFDOLD_A,HFDOLD_B,n_basis,flag)  
      implicit none 

      integer n_basis, a, b, flag  
      double precision HFD_A(n_basis,n_basis) 
      double precision HFD_B(n_basis,n_basis) 
      double precision HFDOLD_A(n_basis,n_basis) 
      double precision HFDOLD_B(n_basis,n_basis) 
      double precision max, diffa, diffb, thresh    

      max = 0.0 
      do a = 1, n_basis 
      do b = 1, n_basis 
         diffa = dabs(HFDOLD_A(b,a) - HFD_A(b,a))  
         diffb = dabs(HFDOLD_B(b,a) - HFD_B(b,a))  
         if (diffa .gt. max) max = diffa 
         if (diffb .gt. max) max = diffb 
      enddo 
      enddo 

      thresh = 1.0d-8 
      flag = 0 
      if (max .lt. thresh) flag = 1 
c      write(6,*) '      Max density difference :', max 

c     if (flag .eq. 0) then 
c        do a = 1, n_basis 
c        do b = 1, n_basis 
c           HFDOLD_A(b,a) = HFD_A(b,a) 
c           HFDOLD_B(b,a) = HFD_B(b,a) 
c        enddo 
c        enddo 
c     endif 

      return 
      end 

      subroutine do_diis(nhist_old,navirt,naocc,nbvirt,nbocc,norb,
     *                   Fai,Ea,Fbj,Eb,FA,FB) 

      Implicit none 
      integer nhist, nhist_old, norb, navirt, naocc, nbvirt, nbocc 
      double precision CM(nhist_old+1),BM(nhist_old+1,nhist_old+1) 
      double precision R(nhist_old+1,nhist_old+1) 
      double precision Fai(navirt,naocc,nhist_old+1), 
     *                 Ea(norb,norb,nhist_old+1)  
      double precision Fbj(nbvirt,nbocc,nhist_old+1), 
     *                 Eb(norb,norb,nhist_old+1)  
      double precision FA(norb,norb), FB(norb,norb) 
      double precision itemp, jtemp, atemp, btemp 
      double precision rpack(nhist_old*nhist_old+2*nhist_old+1) 
      integer iP(nhist_old+1) 
      integer m, n, a, i, b, j, k, npack  

c---------------------------------------------------------------------------
c Zero out B and C matrices 
c---------------------------------------------------------------------------

      nhist = nhist_old ! - 1  

      do n = 1, nhist+1  
         CM(n) = 0.0 
         do m = 1, nhist+1  
            BM(m,n)  = 0.0 
         enddo ! m 
      enddo ! n  

c---------------------------------------------------------------------------
c Form the B matrix 
c---------------------------------------------------------------------------

c     alpha-spin 
      do n = 1, nhist 
         do m = 1, nhist   

            btemp = 0.0 

            do i = 1, naocc 
            do a = 1, navirt 

               btemp = btemp + Fai(a,i,m)*Fai(a,i,n)  

            enddo ! a 
            enddo ! i 

            BM(m,n) = btemp 

          enddo ! m 
      enddo ! n 

c     beta-spin 
      do n = 1, nhist 
         do m = 1, nhist   

            btemp = 0.0 

            do j = 1, nbocc 
            do b = 1, nbvirt 

               btemp = btemp + Fbj(b,j,m)*Fbj(b,j,n)  

            enddo ! b 
            enddo ! j 

            BM(m,n) = BM(m,n) + btemp 

          enddo ! m 
       enddo ! n 

c---------------------------------------------------------------------------
c Done forming the B matrix 
c---------------------------------------------------------------------------

      do n = 1, nhist 
      do m = n, nhist   
         R(n,m) = BM(n,m) 
         if (m .ne. n) R(m,n) = BM(n,m) 
      enddo 
      enddo 
    
      do m = 1, nhist 
         R(m,nhist+1) = -1.0 
         R(nhist+1,m) = -1.0 
      enddo 
      R(nhist+1,nhist+1) = 0.0 

c     write(6,*) ' R vector ' 
c     do m = 1, nhist + 1  
c     do n = 1, nhist + 1 
c        write(6,*) '  ', m, n, R(m,n) 
c     enddo 
c     enddo 

      npack = 0 
      do n = 1, nhist+1  
      do m = 1, nhist+1  
         npack = npack + 1 
         Rpack(npack) = R(m,n) 
      enddo 
      enddo 

      do m = 1, nhist  
         CM(m) = 0.0 
      enddo 
      CM(nhist+1) = -1.0 

      call dgesv(nhist+1,1,Rpack,nhist+1,iP,CM,nhist+1,i) 
      if (i .ne. 0) then 
         write(6,*) ' Error in atom DIIS ', i
         call abort_job() 
      endif 

c     write(6,*) ' C vector ', nhist + 1  
c     do m = 1, nhist+1 
c        write(6,*) '  ', m,  CM(M) 
c     enddo 

      CM(nhist+1) = 0.0 

c---------------------------------------------------------------------------
c Form the updated Fock matrix   
c---------------------------------------------------------------------------

      nhist = nhist_old 

c     write(6,*) ' Updating F ' 
      do n = 1, norb  
      do m = 1, norb  
         itemp = 0.0 
         do k = 1, nhist  
             itemp = itemp + Ea(m,n,k)*CM(k)  
         enddo ! k  
         FA(m,n) = itemp 
      enddo ! m 
      enddo ! n 

c     write(6,*) ' Updating F ' 
      do m = 1, norb  
      do n = 1, norb  
         jtemp = 0.0 
         do k = 1, nhist  
             jtemp = jtemp + Eb(m,n,k)*CM(k)  
         enddo ! k  
         FB(m,n) = jtemp 
      enddo ! m 
      enddo ! n 

c     write(6,*) ' Finished DIIS of order', nhist 

      return 
      end 

c       call gen_hist(iter,maxhist,diis_start,n_basis,nvirt_a,
c    *                nocc_a,nvirt_b,nocc_b,ca,cb,FA,FB,Fai,Fbj,Ea,Eb)
      subroutine gen_hist(kiter,maxhist,diis_start,norb,navirt,naocc,
     *                    nbvirt,nbocc,ca,cb,Fpq_a,Fpq_b,Fai,Fbj,Ea,Eb)

      Implicit none 
      integer kiter, maxhist,diis_start,norb,navirt,naocc,nbvirt,nbocc
      integer m, n, a, i, b, j 
      Double precision Fpq_a(norb,norb), Fpq_b(norb,norb) 
      double precision ca(norb,norb), cb(norb,norb) 
      double precision Fai(navirt,naocc,maxhist+1) 
      double precision FTai(navirt,naocc) 
      double precision FTbj(nbvirt,nbocc) 
      double precision Fbj(nbvirt,nbocc,maxhist+1) 
      double precision Ea(norb,norb,maxhist+1) 
      double precision Eb(norb,norb,maxhist+1) 
      double precision Ti(norb,naocc), Tj(norb,nbocc)  
      double precision itemp, jtemp  

c---------------------------------------------------------------------------
c Form Fai and Fbj 
c---------------------------------------------------------------------------

      do m = 1, norb 
      do i = 1, naocc 
         itemp= 0.0 
         do n = 1, norb 
            itemp = itemp + Fpq_a(m,n)*ca(n,i) 
         enddo  
         Ti(m,i) = itemp 
      enddo  
      enddo  

      do a = naocc+1, norb  
      do i = 1, naocc 
         itemp = 0.0 
         do m = 1, norb 
            itemp = itemp + Ti(m,i)*ca(m,a)  
         enddo 
         FTai(a-naocc,i) = itemp 
      enddo 
      enddo 

      do m = 1, norb 
      do j = 1, nbocc 
         jtemp= 0.0 
         do n = 1, norb 
            jtemp = jtemp + Fpq_b(m,n)*cb(n,j) 
         enddo  
         Tj(m,j) = jtemp 
      enddo  
      enddo  

      do b = nbocc+1, norb  
      do j = 1, nbocc 
         jtemp = 0.0 
         do m = 1, norb 
            jtemp = jtemp + Tj(m,j)*cb(m,b)  
         enddo 
         FTbj(b-nbocc,j) = jtemp 
      enddo 
      enddo 

c---------------------------------------------------------------------------
c Done Forming Fai and Fbj 
c---------------------------------------------------------------------------

c---------------------------------------------------------------------------
c Store Fock matrix history  
c---------------------------------------------------------------------------

      if ((kiter .ge. diis_start) .and. 
     *    (kiter .lt. maxhist+diis_start-1)) then 

            do n = 1, norb 
            do m = 1, norb 
               Ea(m,n,kiter-(diis_start-1)) = Fpq_a(m,n) 
               Eb(m,n,kiter-(diis_start-1)) = Fpq_b(m,n) 
            enddo 
            enddo 
           
            do i = 1, naocc 
            do a = 1, navirt   
               Fai(a,i,kiter-(diis_start-1)) = FTai(a,i) 
c              write(6,*) ' XXXXX', a, i, kiter-2, Fai(a,i,kiter-2)
            enddo 
            enddo 
           
            do j = 1, nbocc 
            do b = 1, nbvirt  
               Fbj(b,j,kiter-(diis_start-1)) = FTbj(b,j) 
            enddo 
            enddo 

      endif 

      if (kiter .ge. maxhist+diis_start-1) then 

            do n = 1, norb 
            do m = 1, norb 
               Ea(m,n,maxhist) = Fpq_a(m,n) 
               Eb(m,n,maxhist) = Fpq_b(m,n) 
            enddo 
            enddo 
           
            do i = 1, naocc 
            do a = 1, navirt  
               Fai(a,i,maxhist) = FTai(a,i) 
            enddo 
            enddo 
           
            do j = 1, nbocc 
            do b = 1, nbvirt 
               Fbj(b,j,maxhist) = FTbj(b,j) 
            enddo 
            enddo 

      endif 

      return 
      end 

      subroutine move_hist(nhist,diis_start,norb,Fa,Fb,Ea,Eb) 
      Implicit none 
      integer norb, nhist, diis_start, m, n, k 
      Double precision Ea(norb,norb,nhist), Eb(norb,norb,nhist) 
      Double precision Fa(norb,norb), Fb(norb,norb) 

c Shift histories 

      do k = 2, nhist 
         do n = 1, norb 
         do m = 1, norb 
            Ea(m,n,k-1) = Ea(m,n,k) 
            Eb(m,n,k-1) = Eb(m,n,k) 
         enddo 
         enddo 
      enddo 

c Current Fock 

      do n = 1, norb 
      do m = 1, norb 
         Ea(m,n,nhist) = Fa(m,n) 
         Eb(m,n,nhist) = Fb(m,n) 
      enddo 
      enddo 

      return 
      end 

      subroutine return_napf(n_apf, icurrent_frag, nfrags, natoms_frag) 
      implicit none 
      integer n_apf, icurrent_frag, nfrags 
      integer natoms_frag(nfrags) 
      integer i 

      n_apf = natoms_frag(icurrent_frag) 

      end 

      subroutine return_predef_2d_int(nfrags, ncenters, icurrent_frag, 
     *                             m, watom_frag, nout) 
      implicit none 
      integer nfrags, ncenters, icurrent_frag, m, nout 
      integer watom_frag(nfrags, ncenters) 

      nout = watom_frag(icurrent_frag, m) 

      end  
