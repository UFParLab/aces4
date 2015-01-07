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
      subroutine return_h1frag(
     c array_slot_0, rank_0, index_values_0, size_0, extents_0, data_0,
     c array_slot_1, rank_1, index_values_1, size_1, extents_1, data_1,
     c  ierr) BIND(C)
      use, intrinsic :: ISO_C_BINDING
      implicit none

c Interface added BUT should be include NOT added in the future

      include 'sip_interface.f'
      include 'imemory.h'

!     Block of h1 integrals on fragment defined by second argument  
      integer(C_INT), intent(in)::array_slot_0
      integer(C_INT), intent(in)::rank_0
      integer(C_INT), intent(in)::index_values_0(1:rank_0)
      integer(C_INT), intent(in)::size_0
      integer(C_INT), intent(in)::extents_0(1:rank_0)
      real(C_DOUBLE), intent(out)::data_0(1:size_0)

!     Scalar defining which fragment  
      integer(C_INT), intent(in)::array_slot_1
      integer(C_INT), intent(in)::rank_1
      integer(C_INT), intent(in)::index_values_1(1:rank_1)
      integer(C_INT), intent(in)::size_1
      integer(C_INT), intent(in)::extents_1(1:rank_1)
      real(C_DOUBLE), intent(in)::data_1(1:size_1)

      integer(C_INT)  ierr  

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

c     integer scratch array. HARD coded size should be changed  
c     integer(C_INT) iscr(5000000)   
c     real(C_DOUBLE) scr(5000000)   

      real(C_DOUBLE), pointer :: scr(:)
      integer(C_INT), pointer :: iscr(:)
      TYPE(C_PTR) :: cscr
      TYPE(C_PTR) :: ciscr
!     offsets of atomic indeces  
      integer(C_INT) offset_1, offset_2, offset_3, offset_4
      integer(C_INT) i, j, one, two

c aosegments 
c     integer(C_INT) n_ao_segs(1), aosegs(6)    
      TYPE(C_PTR) :: n_ao_segs, aosegs
      integer, pointer :: pn_ao_segs(:), paosegs(:)

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

c arrays defining fragment to orbital mapping  
      integer(C_INT), pointer :: baocc_frag(:,:)
      integer(C_INT), pointer :: bbocc_frag(:,:)
      integer(C_INT), pointer :: bavirt_frag(:,:)
      integer(C_INT), pointer :: bbvirt_frag(:,:)

      integer (C_INT) intSpherical
      integer (C_INT) nfrags     
      integer (C_INT) nalpha_occupied 
      integer (C_INT) nbeta_occupied  
      integer (C_INT) nalpha_virtual  
      integer (C_INT) nbeta_virtual   
      integer(C_INT) ncenters 
      integer(C_INT) npcoeffs  
      real(C_DOUBLE) r_thresh
      integer include_atom(10000) ! MAX OF 10,000 atoms  
      integer ifrag, jfrag
      double precision pair_flag  

      integer m, ix2, n_apf, iatom(250) ! Assumes < 20 atoms in a fragment TO CHANGE  
      integer nscr, niscr  

c ---------------------------------------------------------------------- 
c Get predefined data 
c ---------------------------------------------------------------------- 
      ncenters = int_constant("ncenters"//C_NULL_CHAR) 
      intSpherical    = int_constant("intspherical"//C_NULL_CHAR)
      nalpha_occupied = int_constant("nalpha_occupied"//C_NULL_CHAR)
      nbeta_occupied  = int_constant("nbeta_occupied"//C_NULL_CHAR)
      nalpha_virtual  = int_constant("nalpha_virtual"//C_NULL_CHAR)
      nbeta_virtual   = int_constant("nbeta_virtual"//C_NULL_CHAR)
      nfrags          = int_constant("nfrags"//C_NULL_CHAR)
      npcoeffs        = int_constant("npcoeffs"//C_NULL_CHAR)
      r_thresh        = scalar_constant("r_thresh"//C_NULL_CHAR) 

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

c---------------------------------------------------------------------------
c   Compute data block of integrals.
c---------------------------------------------------------------------------

      ix2 = data_1(1)  
      n_apf = natoms_frag(ix2)
      do m = 1, n_apf
         iatom(m) = watom_frag(ix2, m)
      enddo
c     iatom = wfrag
      write(6,*) ' Entering H1 on frag:', ix2, ':',
     *            n_apf, (iatom(m), m=1, n_apf), 'with cuttoff',
     *            r_thresh

      offset_1 = 0 
      offset_2 = 0 
      if (index_values_0(1).gt.1) offset_1=paosegs(index_values_0(1)-1) 
      if (index_values_0(2).gt.1) offset_2=paosegs(index_values_0(2)-1) 

c-------------------------------------------------------------------------
c    Determine which atoms in fragments are inside zone 
c-------------------------------------------------------------------------

      ifrag = ix2 
      do jfrag = 1, nfrags 
         call return_dmin(ifrag,jfrag,nfrags,natoms_frag,watom_frag,
     *                    pcoords, r_thresh, pair_flag)
         if (pair_flag .gt. 0.0) then 
            do j = 1, natoms_frag(jfrag) 
               include_atom(watom_frag(jfrag,j)) = 1 
            enddo 
         else 
            do j = 1, natoms_frag(jfrag) 
               include_atom(watom_frag(jfrag,j)) = 0 
            enddo 
         endif 
      enddo 

      nscr  = zmax  
      niscr = imax 
      call scratch_array(nscr, cscr)
      call c_f_pointer(cscr, scr, [nscr])
      call scratch_array(niscr, ciscr)
      call c_f_pointer(ciscr, iscr, [niscr])

      call comp_return_fragh1(data_0, ix2, n_apf, iatom, 
     *                        include_atom, 
     *                        1+offset_1, extents_0(1)+offset_1, 
     *                        1+offset_2, extents_0(2)+offset_2, 
     *                        ncenters, npcoeffs, pcoords, ptcoords,
     *                        scr, iscr) 

c  Remove scratch space  
      call delete_scratch_array(cscr)
      call delete_scratch_array(ciscr)

      return
      end

      subroutine comp_return_fragh1(z, ifrag, n_apf, watom,
     *        include_atom, 
     *        da1, da2, db1, db2, ncenters, npcoeffs, coords, tcoords, 
     *        zz, iscr) 
    
c---------------------------------------------------------------------------
c Interface added but should be included instead 
      use, intrinsic :: ISO_C_BINDING
      implicit none

      include 'sip_interface.f' 
      include 'imemory.h' 
c Nuclear coordinates 
c --------------------------------------------------------------------------- 
      integer ncenters 
      integer npcoeffs  
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
c---------------------------------------------------------------------------
c     mapping of atoms to shells 
      TYPE(C_PTR) :: patom  
      integer, pointer :: atom(:)

      integer iscr(*) 

      integer da1, da2, db1, db2 
      integer adim, bdim 
      integer one, two 
      double precision zsum(da1:da2, db1:db2)
      double precision z(*)
      double precision zz(*)

      integer i,j,ix,m,n,nfirst, nints, ncoeff, iatom, jatom, fatom, 
     *        ncsum
      integer component
      integer a, b, a1,a2,b1,b2,lastao, igrad 
      integer m1, m2, n1, n2
      integer ixderc
      integer ind(2)
      integer ierr
      integer newcenters, nf, n_apf, watom(ncenters)
      logical dom, don, oldcenter
      double precision acharge(ncenters)

      integer flopcount
      logical spherical
      logical*8 l8true, l8spherical

c     integer max_dim_coeff
c     parameter (max_dim_coeff = 5000)
      integer ccbeg_pack(4*npcoeffs), ccend_pack(4*npcoeffs)
      integer nalpha_pack, ncoeff_pack
      double precision alpha_pack(4*npcoeffs),
     *                 pcoeff_pack(4*npcoeffs)
      double precision xn(ncenters), yn(ncenters), zn(ncenters)
      double precision x1,y1,z1,x2,y2,z2, y
      integer do_atom(ncenters)
      integer ifrag, jfrag, nfrags, pair_flag, include_atom(ncenters)  

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
c ----------------------------------------------------------------------


      intSpherical = int_constant("intspherical"//C_NULL_CHAR)
      spherical = .true.
      if (intSpherical .ne. 1) spherical = .false.
c     write(6,*) ' Spherical = ', spherical 

c----------------------------------------------------------------------------
c   Clear the output array.
c----------------------------------------------------------------------------

c     ECP = iflags(71)

      i = 0
      do b = db1,db2
      do a = da1,da2
         i = i + 1 
         Z(i) = 0.0  
         zsum(a,b) = 0.0 
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
c-------------------------------------------------------------------------

      do m = 1, ncenters
         xn(m) = coords(1,m)
         yn(m) = coords(2,m)
         zn(m) = coords(3,m)
      enddo

      do m = 1, ncenters
         acharge(m) = 0 ! charge(m) 
         do_atom(m) = 0
      enddo
       
      newcenters = 0 
      do m = 1, ncenters  
         iatom = m 
         do n = 1, n_apf 
            jatom = watom(n)  
            if (iatom .eq. jatom) go to 33  
            if (include_atom(iatom) .ne. 1) go to 33 
         enddo

            newcenters          = newcenters + 1 
            xn(newcenters)      = coords(1,m)  
            yn(newcenters)      = coords(2,m)  
            zn(newcenters)      = coords(3,m)  
            acharge(newcenters) = pcharge(iatom) 
            do_atom(iatom)      = 1 

33    continue 
      enddo

c      write(6,*) ' newcenters :', newcenters
c      do m = 1, newcenters 
c         write(6,*) '  ', m, xn(m), yn(m), zn(m), acharge(m)  
c      enddo 

C      if (newcenters .ne. ncenters-n_apf) then
C         write(6,*) ' ********************'
C         write(6,*) ' Charge centers wrong in frag_h1 ',
C     *                ncenters, newcenters, n_apf
C         write(6,*) ' centers :', (watom(m), m=1, n_apf)
C         write(6,*) ' ********************'
C      endif

c     write(6,*) ' Through initialization ' 

c     write(6,*) ' M1 M2:', m1, m2, ':', n1, n2 

c     m1 = 1 
c     m2 = nshells 
c     n1 = 1 
c     n2 = nshells 

c-------------------------------------------------------------------------
c   Calculate the integrals over the necessary shell blocks.
c-------------------------------------------------------------------------

       do m = m1, m2
c         iatom = atom(m)
c         if (do_atom(iatom) .eq. 1) then
            x1 = tcoords(1,m)
            y1 = tcoords(2,m)
            z1 = tcoords(3,m) 
       do n = n1, n2
c         iatom = atom(n)
c         if (do_atom(iatom) .eq. 1) then 
            x2 = tcoords(1,n)  
            y2 = tcoords(2,n)  
            z2 = tcoords(3,n)  

            ncsum = ncfps(m) + ncfps(n)

             call pack_coeffs_oed(palphas, pixalphas, ppcoeffs, 
     *                       pixpcoeffs, ncfps, npfps, m, n,
     *                       alpha_pack, nalpha_pack, pcoeff_pack,
     *                       ncoeff_pack, pccbeg, pccend, 
     *                       indx_cc,
     *                       ccbeg_pack, ccend_pack,
     *                       npcoeffs*4)

c ------------------------------------------------------------------------ 
c NAI (type=1) integrals only.   
c ------------------------------------------------------------------------ 

              call oed__gener_nai_batch(imax, zmax, nalpha_pack, 
     *                 ncoeff_pack,
     *                 ncsum, ncfps(m), ncfps(n), npfps(m),npfps(n),
     *                 ivangmom(m), ivangmom(n), x1,y1,z1,x2,y2,z2,
     *                 newcenters, xn,yn,zn, acharge, alpha_pack,
     *                 pcoeff_pack, ccbeg_pack, ccend_pack,
     *                 spherical, .true., iscr , nints, nfirst,
     *                 zz)

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
c     do b = b1,b2
c     do a = a1,a2
c        write(6,*) a, b, zsum(a,b)
c     enddo
c     enddo

               endif ! nints 

c           endif 
         enddo   ! n shells
c           endif 
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


      return
      end


