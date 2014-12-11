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
      subroutine scf_atom_lowmem(
     c array_slot_0, rank_0, index_values_0, size_0, extents_0, data_0,
     c array_slot_1, rank_1, index_values_1, size_1, extents_1, data_1,
     c array_slot_2, rank_2, index_values_2, size_2, extents_2, data_2,
     c array_slot_3, rank_3, index_values_3, size_3, extents_3, data_3,
     c array_slot_4, rank_4, index_values_4, size_4, extents_4, data_4,
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

      integer(C_INT), intent(out)::ierr
      integer(C_INT)  nscr  

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

c ----------------------------------------------------------------------
c  END_NFPS 
      TYPE(C_PTR) :: pnalpha, pend_nfps 
      integer, pointer :: nalpha(:) 
      integer, pointer :: end_nfps(:) 
c ----------------------------------------------------------------------
c     mapping of atoms to shells 
      TYPE(C_PTR) :: patom  
      integer, pointer :: atom(:)

!     Number of atomic centers 
      integer (C_INT) ncenters        
!     Number of coeffs  
      integer (C_INT) npcoeffs  
!     Number of shells  
      integer (C_INT) nshells  

      integer norbmax, vmax, omax, maxblk, maxhist, diis_start 
      integer one, two, i, j  

      TYPE(C_PTR) :: cscr
      TYPE(C_PTR) :: ciscr
      TYPE(C_PTR) :: cscr1
      TYPE(C_PTR) :: cscr2
      TYPE(C_PTR) :: cscr4
      TYPE(C_PTR) :: cscr5

      real(C_DOUBLE), pointer :: scr(:)
      integer(C_INT), pointer :: iscr(:)
      real(C_DOUBLE), pointer :: scr1(:)
      real(C_DOUBLE), pointer :: scr2(:)
      real(C_DOUBLE), pointer :: scr4(:)
      real(C_DOUBLE), pointer :: scr5(:)

      integer icurrent_frag 

c Allocate scratch arrays 

c     nscr = (extents_1(1) + 1)**2  

      write(6,*) ' ZMAX IMAX NSCR :', zmax, imax, 
     *             (extents_1(1) + 1)**2 
      nscr = zmax ! 5000000
      call scratch_array(nscr, cscr)
      call c_f_pointer(cscr,   scr, [nscr])

      nscr = imax ! 5000000
      call integer_scratch_array(nscr, ciscr) 
      call c_f_pointer(ciscr, iscr, [nscr])

      nscr = (extents_1(1) + 1)**2  

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
c     call predefined_int_array("ixalphas"//C_NULL_CHAR, one,
c    *                                   ntot_alpha, ixalphas)
c     call c_f_pointer(ntot_alpha, pntot_alpha,[1])
c     call c_f_pointer(ixalphas, pixalphas, pntot_alpha)
c     write(6,*) ' IXALPHAS', pntot_alpha(1), (pixalphas(i), i=1, 
c    *                                         pntot_alpha(1)) 
c ----------------------------------------------------------------------

c ----------------------------------------------------------------------
c  IXCOEFFICIENTS   
c     call predefined_int_array("ixpcoeffs"//C_NULL_CHAR,one,
c    *                                   ntot_pcoeff, ixpcoeffs)
c     call c_f_pointer(ntot_pcoeff, pntot_pcoeff,[1])
c     call c_f_pointer(ixpcoeffs, pixpcoeffs, pntot_pcoeff)
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


      ncenters        = int_constant("ncenters"//C_NULL_CHAR)
      npcoeffs        = int_constant("npcoeffs"//C_NULL_CHAR)
      nshells         = int_constant("nshells"//C_NULL_CHAR)

c ----------------------------------------------------------------------

c----------------------------------------------------------------------------
c   Set up the integral pointer 
c----------------------------------------------------------------------------

c     integral_scr = dpkgscr
c     maxblk  = zmax

         maxblk = zmax ! 5000000

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

         icurrent_frag = data_0(1) 
         call compute_fragscf_lowmem(icurrent_frag, ! OK  
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
c    *                 scr1, ! x(nai_array), 
c    *                 scr2, ! x(kin_array), 
     *                 ncenters, npcoeffs, nshells,  
c    *                 scr4, ! x(farray), 
c    *                 scr5, ! x(fbarray), 
     *                 data_1, ! x(ca_array), 
     *                 data_2, ! x(cb_array), 
     *                 data_3, ! x(fa_array), 
     *                 data_4, ! x(fb_array), 
c  
     *                 atom, end_nfps, 
c 
     *                 vmax,omax,norbmax,maxhist,diis_start) 
 

c deAllocate scratch arrays 

      call delete_scratch_array(cscr)
c     call delete_scratch_array(cscr1)
c     call delete_scratch_array(cscr2)
c     call delete_scratch_array(cscr4)
c     call delete_scratch_array(cscr5)

      call delete_integer_scratch_array(ciscr) 


      return
      end
C
      subroutine compute_fragscf_lowmem(icurrent_frag, scr,
     *                 maxblk, iscr, coords, tcoords ,coeffs,alphas, 
     *                 ccbeg, ccend,
     *                 nc1,nc2, nd1, nd2,
c    *                 nai, kin, ncenters, npcoeffs, nshells, fa, fb, 
     *                 ncenters, npcoeffs, nshells,  
     *                 ca_perm, cb_perm, fa_perm, fb_perm, 
c  
     *                 atom, end_nfps, 
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

c array defining which atoms are in which fragment 
      TYPE(C_PTR) :: sDimfrag  
      TYPE(C_PTR) :: pwatom_frag 
      integer, pointer :: pdim_frag(:)  
      integer(C_INT), pointer :: watom_frag(:,:)
c array defining the number of atoms per fragment 
      TYPE(C_PTR) :: pnatoms_frag 
      integer(C_INT), pointer :: natoms_frag(:)
c  CHARGES    
      TYPE(C_PTR) :: charge, atoms
      integer, pointer :: patoms(:)
      real(C_DOUBLE), pointer :: pcharge(:)
c scratch space 
      real(C_DOUBLE), pointer :: scr_i(:)
      integer(C_INT), pointer :: iscr_i(:)
c     real(C_DOUBLE), pointer :: iscr_i(:)
      TYPE(C_PTR) :: cscr
      TYPE(C_PTR) :: ciscr

c arrays defining fragment to orbital mapping  
      integer(C_INT), pointer :: baocc_frag(:,:)
      integer(C_INT), pointer :: bbocc_frag(:,:)
      integer(C_INT), pointer :: bavirt_frag(:,:)
      integer(C_INT), pointer :: bbvirt_frag(:,:)


c     real(C_DOUBLE), pointer :: baocc_frag(:,:)
c     real(C_DOUBLE), pointer :: bbocc_frag(:,:)
c     real(C_DOUBLE), pointer :: bavirt_frag(:,:)
c     real(C_DOUBLE), pointer :: bbvirt_frag(:,:)
c ----------------------------------------------------------------------

      integer atom(*), end_nfps(*)  

      integer aa1,aa2,bb1,bb2,cc1,cc2,dd1,dd2
      integer adim, bdim, cdim, ddim  
      integer m1, m2, n1, n2, r1, r2, s1, s2
      integer i, j, n, m, r, s
      integer a,b,c,d
      integer ncenters, npcoeffs   
      integer iatom(ncenters), n_basis, n_apf, nf   
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
c     double precision nai(nc1:nc2,nd1:nd2)
c     double precision kin(nc1:nc2,nd1:nd2)
      double precision ovl(nd1:nd2,nd1:nd2)
c     double precision H0T(nc1:nc2,nd1:nd2)

c     double precision ca(nc1:nc2,nd1:nd2)
c     double precision cb(nc1:nc2,nd1:nd2)

c     double precision fa(nc1:nc2,nd1:nd2)
c     double precision fb(nc1:nc2,nd1:nd2)

      double precision ca_perm(nc1:nc2,nd1:nd2)
      double precision cb_perm(nc1:nc2,nd1:nd2)
      double precision fa_perm(nc1:nc2,nd1:nd2)
      double precision fb_perm(nc1:nc2,nd1:nd2)

c     integer max_shells 
c     parameter (max_shells = 5000)
      integer beg_anfps(nshells)  
      integer end_anfps(nshells)  
      double precision scr(*)   
      integer iscr(*)

      integer ccbeg(*), ccend(*)

c     integer max_dim_coeff
c     parameter (max_dim_coeff = 5000)
      integer ccbeg_pack(npcoeffs*4), ccend_pack(npcoeffs*4)
      double precision alpha_pack(npcoeffs*4),
     *                 pcoeff_pack(npcoeffs*4)
      integer n_max, nout  
c
c ---------------------------------------------------------------------- 
c Arrays used in the DIIS procedure 
c ---------------------------------------------------------------------- 

      integer vmax,omax,norb,maxhist,diis_start  
c     double precision Fai(vmax,omax,maxhist+1) 
c     double precision Fbj(vmax,omax,maxhist+1) 
c     double precision Ea(norb,norb,maxhist+1) 
c     double precision Eb(norb,norb,maxhist+1) 
c     double precision BM(maxhist+1,maxhist+1) 
c     double precision TBM(maxhist+1,maxhist+1) 
c     double precision CM(maxhist+1) 
      double precision etemp  
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
      write(6,*) ' CURRENT_FRAGS = ', icurrent_frag 

      write(6,*) 'N_APF = ', natoms_frag(icurrent_frag)
C      do m = 1, natoms_frag(icurrent_frag) 
C         write(6,*) '  ', m, watom_frag(icurrent_frag, m)
C      enddo 

      n_apf = natoms_frag(icurrent_frag) 
      do m = 1, n_apf 
c        call return_predef_2d_int(nfrags, ncenters, icurrent_frag, m, 
c    *                             watom_frag, nout) 
c        iatom(m) = nout ! watom_frag(icurrent_frag, m) 
         iatom(m) = watom_frag(icurrent_frag, m) 
      enddo 
c     call return_napf(n_apf, icurrent_frag, nfrags, natoms_frag) 

123   continue 

      write(6,*) ' Entering H0 on frag:', icurrent_frag, ':', 
     *            n_apf, (iatom(m), m=1, n_apf)  


c   Set scratch space 

c     nscr  = zmax  
c     niscr = imax 
      nscr = (nc2-nc1+1)*(nd2-nd1+1) 
      call scratch_array(nscr, cscr)
      call c_f_pointer(cscr, scr_i, [nscr])
c     call integer_scratch_array(niscr, ciscr)
c     call c_f_pointer(ciscr, iscr_i, [niscr])

      call comp_return_frag_h0_lowmem(scr_i, n_apf, iatom, 
     *                         nc1, nc2, nd1, nd2,
     *                         npcoeffs, ncenters, 
     *                         coords, tcoords, 
     *                         pcharge, atom,  
     *                         scr, iscr) 

      call comp_return_frag_ovl_lowmem(ovl, n_apf, iatom, 
     *                         nc1, nc2, nd1, nd2,
     *                         npcoeffs, ncenters, 
     *                         coords, tcoords, 
     *                         pcharge, atom,  
     *                         scr, iscr) 

c  Remove scratch space  
c     call delete_integer_scratch_array(ciscr)
      write(6,*) ' Finished H0 on frag:', icurrent_frag

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

         write(6,*) ' M1 M2', m1, m2 
         write(6,*) ' N_APF ', n_apf, (iatom(m), m=1, n_apf) 
C         do m = m1, m2 
C            write(6,*) ' ', m, atom(m), end_nfps(m) 
C         enddo 

         n_basis = 0 
         do m = m1, m2 
         do n = 1, n_apf 
            if (m .gt. 1) then 
               if(atom(m) .eq. iatom(n)) n_basis = n_basis + 
     *                           end_nfps(m) - end_nfps(m-1)   
            else 
               if(atom(m) .eq. iatom(n)) n_basis = n_basis + 
     *                           end_nfps(m)    
            endif 
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
                     if (n .gt. n_max) n_max = n 
                  enddo 
               else 
                  DO n = end_nfps(m-1) + 1, end_nfps(m) 
                     n_basis = n_basis + 1 
                     if (n .gt. n_max) n_max = n 
                  enddo 
               endif 

               end_anfps(m) = n_basis 

            endif 
          enddo 

         enddo 

c        beg_anfps(1) = 1   
c        end_anfps(1) = end_nfps(1)    
c        do m = 2, m2 
c           beg_anfps(m) = end_nfps(m-1) + 1    
c           end_anfps(m) = end_nfps(m)    
c        enddo 

C         if ((n_max .lt. nc1) .or. (n_max .gt. nc2)) then 
C           write(6,*) ' UMapping of atom dens wrong:' 
C           write(6,*) ' Correct range :', nc1, nc2, 'Computed range:', 
C     *                 '1', n_max  
C         endif 
C
C         if ((n_basis .lt. nc1) .or. (n_basis .gt. nc2)) then 
C           write(6,*) ' Mapping of atom dens wrong:' 
C           write(6,*) ' Correct range :', nc1, nc2, 'Computed range:', 
C     *                 '1', n_basis   
C         endif 

         do j = nd1, nd2 
         do i = nc1, nc2 
            ca_perm(i,j) = 0.0 
            cb_perm(i,j) = 0.0 
            fa_perm(i,j) = 0.0 
            fb_perm(i,j) = 0.0 
         enddo 
         enddo 
c     return ! VFL 

c   Set scratch space 

c     nscr  = zmax  
c     niscr = imax 
c     imax = niscr 
c     zmax = nscr 
c     call scratch_array(nscr, cscr)
c     call c_f_pointer(cscr, scr_i, [nscr])
c     call scratch_array(niscr, ciscr)
c     call c_f_pointer(ciscr, iscr_i, [niscr])

         call do_fragscf_lowmem(icurrent_frag, scr,
     *            maxblk, iscr, coords, tcoords,coeffs,alphas, 
     *            ccbeg, ccend,
     *            nc1,nc2, nd1, nd2, 
c    *            H0T, nai, kin, ovl, ncenters, npcoeffs, nshells, 
     *            scr_i, ovl, ncenters, npcoeffs, nshells, 
     *            spherical,  
     *            n_basis, beg_anfps, end_anfps, 
c
     *            ca_perm, cb_perm, fa_perm, fb_perm, 
     *            natoms_frag, 
     *            iatom, 
     *            pcharge, 
     *            atom, 
     *            nfrags, 
     *            baocc_frag,
     *            bbocc_frag,
     *            bavirt_frag,
     *            bbvirt_frag, 
c 
     *            vmax,omax,norb,maxhist,diis_start) 
c  Remove scratch space  
      call delete_scratch_array(cscr)
c     call delete_scratch_array(ciscr)

      return 
      end 

      subroutine do_fragscf_lowmem(current_frag, scr,
     *               maxblk, iscr, coords, tcoords,coeffs,alphas, 
     *               ccbeg, ccend,
     *               nc1,nc2, nd1, nd2, 
c    *               H0T, nai, kin, ovl, ncenters, npcoeffs, nshells, 
     *               H0T, ovl, ncenters, npcoeffs, nshells, 
     *               spherical,
     *               n_basis, beg_anfps, end_anfps, 
c
     *               ca_perm, cb_perm, fa_perm, fb_perm, 
     *               natoms_frag, 
     *               iatom, 
     *               charge, 
     *               atom, 
     *                         nfrags, 
     *                         baocc_frag,
     *                         bbocc_frag,
     *                         bavirt_frag,
     *                         bbvirt_frag, 
c
     *              vmax,omax,norb,maxhist,diis_start) 
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
c     TYPE(C_PTR) :: pcoeffs  
      integer, pointer :: pntot_pcoeff(:), pixpcoeffs(:) 
c     real(C_DOUBLE), pointer :: ppcoeffs(:) 
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
      integer ncenters, npcoeffs, nshells   
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

      double precision frag_nre

      double precision coords(3,*), tcoords(3,*), coeffs(*), alphas(*)
c     double precision nai(nc1:nc2,nd1:nd2)
c     double precision kin(nc1:nc2,nd1:nd2)
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
      double precision Fai(n_basis,n_basis,maxhist+1) 
      double precision Fbj(n_basis,n_basis,maxhist+1) 
      double precision Ea(n_basis,n_basis,maxhist+1) 
      double precision Eb(n_basis,n_basis,maxhist+1) 
      double precision BM(maxhist+1,maxhist+1) 
      double precision TBM(maxhist+1,maxhist+1) 
      double precision CM(maxhist+1) 
c ---------------------------------------------------------------------- 

c     integer max_dim_coeff
c     parameter (max_dim_coeff = 5000)

c     integer max_shells 
c     parameter (max_shells = 5000)
      integer natoms_frag(nfrags) 
c     integer watom_frag(nfrags, *) 
      integer atom(nshells) 

      integer beg_anfps(nshells)  
      integer end_anfps(nshells)  
      double precision scr(*)   
      integer iscr(*)

      integer ccbeg(*), ccend(*)

      integer ccbeg_pack(npcoeffs*4), ccend_pack(npcoeffs*4)
      double precision alpha_pack(npcoeffs*4),
     *                 pcoeff_pack(npcoeffs*4)
      double precision charge(ncenters) 
      double precision etemp 

      write(6,*) ' N_APF = ', natoms_frag(current_frag) 
      write(6,*) ' IATOM = ', (iatom(m), 
     *                         m = 1, natoms_frag(current_frag) )
      n_apf = natoms_frag(current_frag) 
c     do m = 1, n_apf 
c        iatom(m) = watom_frag(current_frag, m) 
c     enddo 
c     return ! VFL  

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

      write(6,*) ' NC = ', nc1, nc2, ':', nd1, nd2, '::', n_basis  

      do n = 1, n_basis  
      do m = 1, n_basis  
         h0(m,n) = H0T(m,n) ! 0.0  
         aovl(m,n) = ovl(m,n) 
C         write(6,*) m,n, ovl(m,n) 
      enddo  
      enddo  

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

       call monomer_nn_repulsion(frag_nre,n_apf,ncenters,
     *                           iatom,coords,charge)

      write(6,*) ' Frag:',current_frag,
     *           ' Nuclear Repulsion Energy = ', frag_nre

       call hfenergy_l(HFD_A,HFD_B,FA,FB,h0,frag_nre,
     *                 n_basis,current_frag) 

c-----------------------------------------------------------------------
c Copy the HF Density into the old Density. 
c-----------------------------------------------------------------------

      call hfdensity_copy(HFD_A,HFD_B,HFDOLD_A,HFDOLD_B,n_basis)  

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

       call monomer_nn_repulsion(frag_nre,n_apf,ncenters,
     *                           iatom,coords,charge)

        call hfenergy_l(HFD_A,HFD_B,FA,FB,h0,frag_nre,
     *                  n_basis,current_frag) 

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

c-----------------------------------------------------------------------
c       Compute the new HF energy  
c-----------------------------------------------------------------------

       call monomer_nn_repulsion(frag_nre,n_apf,ncenters,
     *                           iatom,coords,charge)

        call hfenergy_l(HFD_A,HFD_B,FA,FB,h0,frag_nre,
     *                  n_basis,current_frag) 
       write(6,*) ' Frag: ',current_frag," FINAL SCF = ",frag_nre

c     if (current_frag .eq. -1) then 
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

c      endif 
          write(6,*) ' Done SCF calculation of ATOM :', current_frag  

c-----------------------------------------------------------------------
c     Put the transformation coefficients to the proper spot  
c-----------------------------------------------------------------------

      do n = 1, n_basis 
      do m = 1, n_basis 
          ca_perm(m,n) = ca(m,n) 
          cb_perm(m,n) = cb(m,n) 
          fa_perm(m,n) = FTa(m,n)  
          fb_perm(m,n) = FTb(m,n)  
C JNB debug
C          print *, m,n,ca(m,n), fa_perm(m,n)
      enddo 
      enddo 

c-----------------------------------------------------------------------
c        Alpha 
c-----------------------------------------------------------------------

c     if (current_frag .eq. 1) go to 123 

c     na_offset = baocc_frag(current_frag, 1) 

c     do n = 1, nocc_a 
c     do m = 1, n_basis
c        ca_perm(map(m),na_offset+n-1) = 0.0  
c        if ((map(m) .ne. 0) .and. (map(n) .ne. 0)) then  
c        if ((map(m) .ne. 0)) then  
c           ca_perm(map(m),na_offset+n-1) = Ca(m,n)  
c        endif 
c     enddo 
c     enddo 

c     na_offset = bavirt_frag(current_frag, 1) 

c     do n = 1, n_basis - nocc_a  
c     do m = 1, n_basis
c        ca_perm(map(m),na_offset+n-1) = 0.0  
c        if ((map(m) .ne. 0) .and. (map(n) .ne. 0)) then  
c        if ((map(m) .ne. 0)) then  
c           ca_perm(map(m),na_offset+n-1) = Ca(m,n+nocc_a)  
c        endif 
c     enddo 
c     enddo 
c     write(6,*) ' ETEMP (virt)', etemp 

c-----------------------------------------------------------------------
c        Beta 
c-----------------------------------------------------------------------

c     nb_offset = bbocc_frag(current_frag, 1) 
c     do n = 1, nocc_b 
c     do m = 1, n_basis
c        cb_perm(map(m),nb_offset+n-1) = 0.0  
c        if ((map(m) .ne. 0) .and. (map(n) .ne. 0)) then  
c           cb_perm(map(m),nb_offset+n-1) = Cb(m,n)  
c        endif 
c     enddo 
c     enddo 

c     nb_offset = bbvirt_frag(current_frag, 1) 
c     do n = 1, n_basis - nocc_b  
c     do m = 1, n_basis
c        cb_perm(map(m),nb_offset+n-1) = 0.0  
c        if ((map(m) .ne. 0) .and. (map(n) .ne. 0)) then  
c           cb_perm(map(m),nb_offset+n-1) = Cb(m,n+nocc_b)  
c        endif 
c     enddo 
c     enddo 


c-----------------------------------------------------------------------
c     Done transfering coefficients  
c-----------------------------------------------------------------------

c-----------------------------------------------------------------------
c     Put the Fock matrix to the proper spot  
c-----------------------------------------------------------------------

c-----------------------------------------------------------------------
c        ALPHA 
c-----------------------------------------------------------------------

c     na_offset = baocc_frag(current_frag, 1) 
c     ma_offset = baocc_frag(current_frag, 1) 
c     do n = 1, nocc_a 
c     do m = 1, nocc_a  
c        fa_perm(ma_offset+m-1,na_offset+n-1) = 0.0  
c        if ((map(m) .ne. 0) .and. (map(n) .ne. 0)) then  
c           fa_perm(ma_offset+m-1,na_offset+n-1) = FTa(m,n)  
c        endif 
c     enddo 
c     enddo 

c     na_offset = baocc_frag(current_frag, 1) 
c     ma_offset = bavirt_frag(current_frag, 1)  
c     do n = 1, nocc_a 
c     do m = 1, n_basis - nocc_a   
c        fa_perm(ma_offset+m-1,na_offset+n-1) = 0.0  
c        if ((map(m) .ne. 0) .and. (map(n) .ne. 0)) then  
c           fa_perm(ma_offset+m-1,na_offset+n-1) = 
c    *          FTa(m+nocc_a,n)  
c        endif 
c     enddo 
c     enddo 

c     ma_offset = baocc_frag(current_frag, 1) 
c     na_offset = bavirt_frag(current_frag, 1)  
c     do n = 1, n_basis - nocc_a    
c     do m = 1, nocc_a 
c        fa_perm(ma_offset+m-1,na_offset+n-1) = 0.0  
c        if ((map(m) .ne. 0) .and. (map(n) .ne. 0)) then  
c           fa_perm(ma_offset+m-1,na_offset+n-1) = 
c    *          FTa(m,n+nocc_a)  
c        endif 
c     enddo 
c     enddo 

c     na_offset = bavirt_frag(current_frag, 1)  
c     ma_offset = bavirt_frag(current_frag, 1)  
c     do n = 1, n_basis - nocc_a    
c     do m = 1, n_basis - nocc_a   
c        fa_perm(ma_offset+m-1,na_offset+n-1) = 0.0  
c        if ((map(m) .ne. 0) .and. (map(n) .ne. 0)) then  
c           fa_perm(ma_offset+m-1,na_offset+n-1) =
c    *          FTa(m+nocc_a,n+nocc_a)  
c        endif 
c     enddo 
c     enddo 

c-----------------------------------------------------------------------
c        BETA 
c-----------------------------------------------------------------------
99    continue 

c     nb_offset = bbocc_frag(current_frag, 1) 
c     mb_offset = bbocc_frag(current_frag, 1) 
c     do n = 1, nocc_b 
c     do m = 1, nocc_b  
c        fb_perm(mb_offset+m-1,nb_offset+n-1) = 0.0  
c        if ((map(m) .ne. 0) .and. (map(n) .ne. 0)) then  
c           fb_perm(mb_offset+m-1,nb_offset+n-1) = FTb(m,n)  
c        endif 
c     enddo 
c     enddo 

c     nb_offset = bbocc_frag(current_frag, 1) 
c     mb_offset = bbvirt_frag(current_frag, 1)  
c     do n = 1, nocc_b 
c     do m = 1, n_basis - nocc_b    
c        fb_perm(mb_offset+m-1,nb_offset+n-1) = 0.0  
c        if ((map(m) .ne. 0) .and. (map(n) .ne. 0)) then  
c           fb_perm(mb_offset+m-1,nb_offset+n-1) = 
c    *          FTb(m+nocc_b,n)  
c        endif 
c     enddo 
c     enddo 

c     mb_offset = bbocc_frag(current_frag, 1) 
c     nb_offset = bbvirt_frag(current_frag, 1)  
c     do n = 1, n_basis - nocc_b   
c     do m = 1, nocc_b 
c        fb_perm(mb_offset+m-1,nb_offset+n-1) = 0.0  
c        if ((map(m) .ne. 0) .and. (map(n) .ne. 0)) then  
c           fb_perm(mb_offset+m-1,nb_offset+n-1) = 
c    *          FTb(m,n+nocc_b)  
c        endif 
c     enddo 
c     enddo 

c     nb_offset = bbvirt_frag(current_frag, 1)  
c     mb_offset = bbvirt_frag(current_frag, 1)  
c     do n = 1, n_basis - nocc_b    
c     do m = 1, n_basis - nocc_b   
c        fb_perm(ma_offset+m-1,na_offset+n-1) = 0.0  
c        if ((map(m) .ne. 0) .and. (map(n) .ne. 0)) then  
c           fb_perm(mb_offset+m-1,nb_offset+n-1) =
c    *          FTb(m+nocc_b,n+nocc_b)  
c        endif 
c     enddo 
c     enddo 

      return 

      end
 
      subroutine comp_return_frag_h0_lowmem(z, napf, watom, da1, da2, 
     * db1, db2, 
     * npcoeffs, ncenters, coords, tcoords, charge, atom, zz, iscr)
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
      include 'imemory.h' 

c---------------------------------------------------------------------------

      integer iscr(*) 

      integer da1, da2, db1, db2 
      integer adim, bdim 
      double precision zsum(da1:da2, db1:db2)
      double precision z(*)  
      double precision zz(*)  
      integer i, ix, m, n, nfirst, nints, ncoeff, iatom, ncsum
      integer component
      integer a, b, aa, bb, a1,a2,b1,b2, afirst, bfirst, lastao, igrad
      integer m1, m2, n1, n2
      integer der1x, der1y, der1z
      integer der2x, der2y, der2z
      integer dercx, dercy, dercz
      integer ixderc
      integer ind(2)
      integer ierr
      integer two
      integer npcoeffs

      integer flopcount
      logical spherical
      logical*8 l8true, l8spherical

      integer ccbeg_pack(4*npcoeffs), ccend_pack(4*npcoeffs)
      integer nalpha_pack, ncoeff_pack
      integer jatom, newcenters
      double precision alpha_pack(4*npcoeffs),
     *                 pcoeff_pack(4*npcoeffs)
      double precision xn(ncenters), yn(ncenters), zn(ncenters)
      double precision x1,y1,z1,x2,y2,z2, y
      double precision acharge(ncenters)
      integer do_atom(ncenters)
      integer napf, watom(ncenters), atom(*)
      double precision charge(ncenters)

      integer (C_INT) one
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

c ----------------------------------------------------------------------
c  NCFPS 
c ----------------------------------------------------------------------
      call predefined_int_array("ncfps"//C_NULL_CHAR, one,
     *                                   pishells, pncfps)
      call c_f_pointer(pishells, ishells, [1])
      call c_f_pointer(pncfps, ncfps, ishells)
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

      intSpherical = int_constant("intspherical"//C_NULL_CHAR)
      spherical = .true.
      if (intSpherical .ne. 1) spherical = .false.
c     write(6,*) ' Spherical = ', spherical 

c----------------------------------------------------------------------------
c   Clear the output array.
c----------------------------------------------------------------------------

      i = 0 
      do b = db1,db2
      do a = da1,da2
         i = i + 1 
         Zsum(a,b) = 0.d0
         Z(i) = 0.0 
         ZZ(i) = 0.0 
      enddo
      enddo

      ind(1) = 1
      ind(2) = 2

c-------------------------------------------------------------------------
c   Find the shells we need to loop over for this block.
c--------------------------------------------------------------------------

      call lookup_shell(end_nfps, nshells, da1, m1)
      call lookup_shell(end_nfps, nshells, da2, m2)
      call lookup_shell(end_nfps, nshells, db1, n1)
      call lookup_shell(end_nfps, nshells, db2, n2)

c-------------------------------------------------------------------------
c   Build an array of coordinates per atom.
c-------------------------------------------------------------------

      do m = 1, ncenters
         xn(m) = coords(1,m)
         yn(m) = coords(2,m)
         zn(m) = coords(3,m)
      enddo

      do m = 1, ncenters
         acharge(m) = charge(m)
         do_atom(m) = 0
c        write(6,*) ' CHARGE :', m, charge(m) 
      enddo

      newcenters = 0
      do n = 1, napf
         jatom = watom(n)
      do m = 1, ncenters
         iatom = m
         if (iatom .eq. jatom) then
            newcenters = newcenters + 1
            xn(newcenters) = coords(1,m)
            yn(newcenters) = coords(2,m)
            zn(newcenters) = coords(3,m)
            acharge(newcenters) = charge(iatom)
            do_atom(iatom) = 1
         endif
      enddo
      enddo

c-------------------------------------------------------------------------
c Find starting and ending shell do the cluster. It is assumed
c throughout the cluster code that the the atoms and shells 
c come in order such as 
c frag1 frag2 ....
c-------------------------------------------------------------------------

      m1 = 0
      m2 = 0

      do m = 1, nshells
         if (atom(m) .eq. watom(1)) then
            m1 = m
            n1 = m
            go to 11
         endif
      enddo
11    continue

      do m = nshells, 1, -1
         if (atom(m) .eq. watom(napf)) then
            m2 = m
            n2 = m
            go to 12
         endif
      enddo
12    continue

c     m1 = 1 
c     m2 = nshells 
c     n1 = 1 
c     n2 = nshells 

c-------------------------------------------------------------------------
c   Calculate the integrals over the necessary shell blocks.
c-------------------------------------------------------------------------

       do m = m1, m2
          iatom = atom(m)
          if (do_atom(iatom) .eq. 1) then
            x1 = tcoords(1,m)
            y1 = tcoords(2,m)
            z1 = tcoords(3,m)
       do n = n1, n2
          jatom = atom(n)
          if (do_atom(jatom) .eq. 1) then

            ncsum = ncfps(m) + ncfps(n)

            x2 = tcoords(1,n)
            y2 = tcoords(2,n)
            z2 = tcoords(3,n)

c           write(6,*) ' VFL:', m, n, ':', iatom, jatom 
c           write(6,*) 'XYZ1', x1, y1, z1 
c           write(6,*) 'XYZ2', x2, y2, z2 
c           write(6,*) 'ncsum', ncsum  

            call pack_coeffs_oed(palphas, pixalphas, ppcoeffs,
     *                       pixpcoeffs, ncfps, npfps, m, n,
     *                       alpha_pack, nalpha_pack, pcoeff_pack,
     *                       ncoeff_pack, pccbeg, pccend,
     *                       indx_cc,
     *                       ccbeg_pack, ccend_pack,
     *                       npcoeffs*4)

c ------------------------------------------------------------------------ 
c  KIN (type=2) integrals next.   
c ------------------------------------------------------------------------ 

              call oed__gener_kin_batch(imax, zmax, nalpha_pack,
     *                 ncoeff_pack,
     *                 ncsum, ncfps(m), ncfps(n), npfps(m),npfps(n),
     *                 ivangmom(m), ivangmom(n), x1,y1,z1,x2,y2,z2,
     *                 alpha_pack,
     *                 pcoeff_pack, ccbeg_pack, ccend_pack,
     *                 spherical, .true., iscr, nints, nfirst,
     *                 zz)

            if (nints .gt. 0) then

               if (m1 .gt. 1) afirst = end_nfps(m1-1) 
               if (n1 .gt. 1) bfirst = end_nfps(n1-1) 
               if (m1 .eq. 1) afirst = 0  
               if (n1 .eq. 1) bfirst = 0  

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

                  a1 = a1 - afirst 
                  a2 = a2 - afirst 
                  b1 = b1 - bfirst 
                  b2 = b2 - bfirst 

                call add_integrals2(zsum, da1,da2,db1,db2,
     *                 zz(nfirst),a1,a2,b1,b2,1.0d0)
             endif ! (nints .gt. 0) then

c ------------------------------------------------------------------------ 
c NAI (type=1) integrals first.   
c ------------------------------------------------------------------------ 

              call oed__gener_nai_batch(imax, zmax, nalpha,ncoeff_pack,
     *                 ncsum, ncfps(m), ncfps(n), npfps(m),npfps(n),
     *                 ivangmom(m), ivangmom(n), x1,y1,z1,x2,y2,z2,
     *                 newcenters, xn,yn,zn, acharge, alpha_pack,
     *                 pcoeff_pack, ccbeg_pack, ccend_pack,
     *                 spherical, .true., iscr, nints, nfirst,
     *                 zz)

C               write(6,*) ' NINTS = ', nints 
               if (nints .gt. 0) then

                  if (m1 .gt. 1) afirst = end_nfps(m1-1) 
                  if (n1 .gt. 1) bfirst = end_nfps(n1-1) 
                  if (m1 .eq. 1) afirst = 0  
                  if (n1 .eq. 1) bfirst = 0  

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

                  a1 = a1 - afirst 
                  a2 = a2 - afirst 
                  b1 = b1 - bfirst 
                  b2 = b2 - bfirst 
                
                 call add_integrals2(zsum, da1,da2,db1,db2,
     *                 zz(nfirst),a1,a2,b1,b2,1.0d0)

c Print out the integrals  

c                 i = 0
c                 do b = db1,db2
c                 do a = da1,da2
c                    i = i + 1
c                    do bb = b1, b2 
c                       if ((bb .ge. db1) .and. (bb .le. db2)) then 
c                    do aa = a1, a2 
c                       if ((aa .ge. da1) .and. (aa .le. da2)) then 
c                       z(i) = zsum(a,b)
c                       endif 
c                    enddo 
c                       endif 
c                    enddo 
c                 enddo
c                 enddo

               endif ! nints 

            endif
         enddo   ! n shells
            endif
         enddo   ! m shells

c Print out the integrals  

                  i = 0
                  do b = db1,db2
                  do a = da1,da2
                     i = i + 1
c                    do bb = b1, b2 
c                       if ((bb .ge. db1) .and. (bb .le. db2)) then 
c                    do aa = a1, a2 
c                       if ((aa .ge. da1) .and. (aa .le. da2)) then 
                        z(i) = zsum(a,b)
c                       endif 
c                    enddo 
c                       endif 
c                    enddo 
                  enddo
                  enddo

      return
      end

 
      subroutine comp_return_frag_ovl_lowmem(z, napf, watom, da1, da2, 
     * db1, db2, 
     * npcoeffs, ncenters, coords, tcoords, charge, atom, zz, iscr)
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
      include 'imemory.h' 

c---------------------------------------------------------------------------

      integer iscr(*) 

      integer da1, da2, db1, db2 
      integer adim, bdim 
      double precision zsum(da1:da2, db1:db2)
c     double precision z(da1:da2, db1:db2)
      double precision z(*)  
      double precision zz(*)  
      integer i, ix, m, n, nfirst, nints, ncoeff, iatom, ncsum
      integer component
      integer a, b, aa, bb, a1,a2,b1,b2, afirst, bfirst, lastao, igrad
      integer m1, m2, n1, n2
      integer der1x, der1y, der1z
      integer der2x, der2y, der2z
      integer dercx, dercy, dercz
      integer ixderc
      integer ind(2)
      integer ierr
      integer two
      integer npcoeffs

      integer flopcount
      logical spherical
      logical*8 l8true, l8spherical

      integer ccbeg_pack(4*npcoeffs), ccend_pack(4*npcoeffs)
      integer nalpha_pack, ncoeff_pack
      integer jatom, newcenters
      double precision alpha_pack(4*npcoeffs),
     *                 pcoeff_pack(4*npcoeffs)
      double precision xn(ncenters), yn(ncenters), zn(ncenters)
      double precision x1,y1,z1,x2,y2,z2, y
      double precision acharge(ncenters)
      integer do_atom(ncenters)
      integer napf, watom(ncenters), atom(*)
      double precision charge(ncenters)

      integer (C_INT) one
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

c ----------------------------------------------------------------------
c  NCFPS 
c ----------------------------------------------------------------------
      call predefined_int_array("ncfps"//C_NULL_CHAR, one,
     *                                   pishells, pncfps)
      call c_f_pointer(pishells, ishells, [1])
      call c_f_pointer(pncfps, ncfps, ishells)
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

      intSpherical = int_constant("intspherical"//C_NULL_CHAR)
      spherical = .true.
      if (intSpherical .ne. 1) spherical = .false.
c     write(6,*) ' Spherical = ', spherical 

c----------------------------------------------------------------------------
c   Clear the output array.
c----------------------------------------------------------------------------

      i = 0 
      do b = db1,db2
      do a = da1,da2
         i = i + 1 
         Zsum(a,b) = 0.d0
         Z(i) = 0.0 
         ZZ(i) = 0.0 
      enddo
      enddo

c-------------------------------------------------------------------------
c   Build an array of coordinates per atom.
c-------------------------------------------------------------------

      do m = 1, ncenters
         xn(m) = coords(1,m)
         yn(m) = coords(2,m)
         zn(m) = coords(3,m)
      enddo

      do m = 1, ncenters
         acharge(m) = charge(m)
         do_atom(m) = 0
c        write(6,*) ' CHARGE :', m, charge(m) 
      enddo

      newcenters = 0
      do n = 1, napf
         jatom = watom(n)
      do m = 1, ncenters
         iatom = m
         if (iatom .eq. jatom) then
            newcenters = newcenters + 1
            xn(newcenters) = coords(1,m)
            yn(newcenters) = coords(2,m)
            zn(newcenters) = coords(3,m)
            acharge(newcenters) = charge(iatom)
            do_atom(iatom) = 1
         endif
      enddo
      enddo

c-------------------------------------------------------------------------
c Find starting and ending shell do the cluster. It is assumed
c throughout the cluster code that the the atoms and shells 
c come in order such as 
c frag1 frag2 ....
c-------------------------------------------------------------------------

      m1 = 0
      m2 = 0

      do m = 1, nshells
         if (atom(m) .eq. watom(1)) then
            m1 = m
            n1 = m
            go to 11
         endif
      enddo
11    continue

      do m = nshells, 1, -1
         if (atom(m) .eq. watom(napf)) then
            m2 = m
            n2 = m
            go to 12
         endif
      enddo
12    continue

      if (watom(1) .eq. 1) then 
         write(6,*) ' Fragment 1', napf, 'atoms', (watom(i), i=1,napf) 
         write(6,*) ' M N', m1, m2, n1, n2 
      endif  

c     m1 = 1 
c     m2 = nshells 
c     n1 = 1 
c     n2 = nshells 

c-------------------------------------------------------------------------
c   Calculate the integrals over the necessary shell blocks.
c-------------------------------------------------------------------------

       do m = m1, m2
          iatom = atom(m)
          if (do_atom(iatom) .eq. 1) then
            x1 = tcoords(1,m)
            y1 = tcoords(2,m)
            z1 = tcoords(3,m)
       do n = n1, n2
          jatom = atom(n)
          if (do_atom(jatom) .eq. 1) then

            ncsum = ncfps(m) + ncfps(n)

            x2 = tcoords(1,n)
            y2 = tcoords(2,n)
            z2 = tcoords(3,n)

c           write(6,*) ' VFL:', m, n, ':', iatom, jatom 
c           write(6,*) 'XYZ1', x1, y1, z1 
c           write(6,*) 'XYZ2', x2, y2, z2 
c           write(6,*) 'ncsum', ncsum  

            call pack_coeffs_oed(palphas, pixalphas, ppcoeffs,
     *                       pixpcoeffs, ncfps, npfps, m, n,
     *                       alpha_pack, nalpha_pack, pcoeff_pack,
     *                       ncoeff_pack, pccbeg, pccend,
     *                       indx_cc,
     *                       ccbeg_pack, ccend_pack,
     *                       npcoeffs*4)

c ------------------------------------------------------------------------ 
c  KIN (type=2) integrals next.   
c ------------------------------------------------------------------------ 

              call oed__gener_ovl_batch(imax, zmax, nalpha_pack,
     *                 ncoeff_pack,
     *                 ncsum, ncfps(m), ncfps(n), npfps(m),npfps(n),
     *                 ivangmom(m), ivangmom(n), x1,y1,z1,x2,y2,z2,
     *                 alpha_pack,
     *                 pcoeff_pack, ccbeg_pack, ccend_pack,
     *                 spherical, .true., iscr, nints, nfirst,
     *                 zz)

            if (nints .gt. 0) then

               if (m1 .gt. 1) afirst = end_nfps(m1-1) 
               if (n1 .gt. 1) bfirst = end_nfps(n1-1) 
               if (m1 .eq. 1) afirst = 0  
               if (n1 .eq. 1) bfirst = 0  

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

C      if (watom(1) .eq. 1) then  
C               write(6,*) 'A1 A2 B1 B2:', a1, a2, b1, b2, '::', 
C     *         a1-afirst,a2-afirst,b1-bfirst,b2-bfirst 
C      endif 

               a1 = a1 - afirst 
               a2 = a2 - afirst 
               b1 = b1 - bfirst 
               b2 = b2 - bfirst 

                call add_integrals2(zsum, da1,da2,db1,db2,
     *                 zz(nfirst),a1,a2,b1,b2,0.0d0)
C      if (watom(1) .eq. 1) then 
C          do i = nfirst, nfirst + nints 
C          write(6,*) 'III = ', i, zz(i)
C          enddo
C      endif 

             endif ! (nints .gt. 0) then


            endif
         enddo   ! n shells
            endif
         enddo   ! m shells

c Print out the integrals  

                  i = 0
                  do b = db1,db2
                  do a = da1,da2
                     i = i + 1
                        z(i) = zsum(a,b)
C      if (watom(1) .eq. 1) then 
C          write(6,*) 'JJJ = ', a, b, z(i), zsum(a,b)
C      endif 
                  enddo
                  enddo

      return
      end

      subroutine hfenergy_l(HFD_A,HFD_B,FA,FB,h0,nnr,
     *                      n_basis,current_frag)
      implicit none 
C
C returns the SCF energy in the nnr variable.
C
      integer n_basis 
      integer m, n 
      double precision HFD_A(n_basis,n_basis) 
      double precision HFD_B(n_basis,n_basis) 
      double precision FA(n_basis,n_basis) 
      double precision FB(n_basis,n_basis) 
      double precision h0(n_basis,n_basis) 
      double precision nnr
      double precision ea, eb, etotal 
      integer current_frag

      ea = 0.0 
      eb = 0.0 
      etotal = 0.0 

      do n = 1, n_basis  
      do m = 1, n_basis  
         ea = ea + (h0(m,n)+FA(m,n))*HFD_A(m,n)   
         eb = eb + (h0(m,n)+FB(m,n))*HFD_B(m,n)   
      enddo  
      enddo  

      etotal = 0.5d0*(ea + eb) + nnr
      write(6,*) ' Frag:',current_frag,
     *           ' SCF energy = ', etotal 

      nnr = etotal

      return 
      end 
      subroutine monomer_nn_repulsion(energy,n_apf,ncenters,
     *                                iatom,coords,charge)
      implicit none 
      integer i, j, kcoord, im, jm
      integer ncenters, n_apf
      integer iatom(n_apf)
      double precision energy, coords(3,ncenters), charge(ncenters)
      double precision dist

      energy = 0.d0
      do i = 1, ncenters
      do im = 1,n_apf
      if(i.eq.iatom(im)) then
          do j = i, ncenters
          do jm = 1,n_apf
          if(j.eq.iatom(jm)) then
             if (i .ne. j) then
                dist = 0.0
                do kcoord = 1, 3
                   dist = dist + (coords(kcoord,i)-coords(kcoord,j))**2
                enddo
                dist = dsqrt(dist)
                energy = energy + charge(i)*charge(j) / dist
             endif
          endif
          enddo
          enddo
      endif
      enddo
      enddo

      return
      end
