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
      subroutine compute_dipole_integrals(
     c array_slot_0, rank_0, index_values_0, size_0, extents_0, data_0,
     c array_slot_1, rank_1, index_values_1, size_1, extents_1, data_1,
     c array_slot_2, rank_2, index_values_2, size_2, extents_2, data_2,
     c  ierr) BIND(C)
      use, intrinsic :: ISO_C_BINDING
      implicit none
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C compute dipole moments.
C execute compute_dipole_integrals Txx II
C INPUT: II is the moment of interest.
C        II = 1 eX
C        II = 2 eY
C        II = 3 eZ
C        II = 10 nX
C        II = 11 nY
C        II = 12 nZ
C
C OUTPUT: Txx is a temp array for placing the ao integrals 
C OUTPUT: II is the nuclear moment
C for nuclear dipole moment incoming scalar is altered to contain the
C corresponding nuclear moment
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC

c Interface added BUT should be include NOT added in the future

      include 'sip_interface.f'
      include 'imemory.h'

!     Block of integrals on fragment defined by second argument  
      integer(C_INT), intent(in)::array_slot_0
      integer(C_INT), intent(in)::rank_0
      integer(C_INT), intent(in)::index_values_0(1:rank_0)
      integer(C_INT), intent(in)::size_0
      integer(C_INT), intent(in)::extents_0(1:rank_0)
      real(C_DOUBLE), intent(out)::data_0(1:size_0)

!     Return scalor with nuclear term
      integer(C_INT), intent(in)::array_slot_1
      integer(C_INT), intent(in)::rank_1
      integer(C_INT), intent(in)::index_values_1(1:rank_1)
      integer(C_INT), intent(in)::size_1
      integer(C_INT), intent(in)::extents_1(1:rank_1)
      real(C_DOUBLE), intent(inout)::data_1(1:size_1)

!     Scalar defining which moment   
      integer(C_INT), intent(in)::array_slot_2
      integer(C_INT), intent(in)::rank_2
      integer(C_INT), intent(in)::index_values_2(1:rank_2)
      integer(C_INT), intent(in)::size_2
      integer(C_INT), intent(in)::extents_2(1:rank_2)
      real(C_DOUBLE), intent(inout)::data_2(1:size_2)

      integer(C_INT)  ierr  

! predefined number of centers
      integer(C_INT) ncenters 
      integer(C_INT) npcoeffs  
      integer(C_INT) nalphas

      real(C_DOUBLE), pointer :: scr(:)
      integer(C_INT), pointer :: iscr(:)
      TYPE(C_PTR) :: cscr
      TYPE(C_PTR) :: ciscr
      TYPE(C_PTR) :: cptr_int
      real(C_DOUBLE), pointer :: fptr_int(:)

!     offsets of atomic indeces  
      integer(C_INT) offset_1, offset_2, offset_3, offset_4
      integer(C_INT) int_length1, int_length2
      integer(C_INT) int_length
      integer(C_INT) i, j, one, two

c aosegments 
c     integer(C_INT) n_ao_segs(1), aosegs(6)    
      TYPE(C_PTR) :: n_ao_segs, aosegs
      integer(C_INT), pointer :: pn_ao_segs(:), paosegs(:)

      integer ix2, diff, crit, ixyz

c Nuclear dipole moment  
c --------------------------------------------------------------------------- 
      double precision dnuc  

      integer nscr, niscr  

c ----------------------------------------------------------------------
c Get the ao segment ranges  
      call predefined_int_array("ao_seg_ranges"//C_NULL_CHAR, one,
     *                           n_ao_segs, aosegs)
      call c_f_pointer(n_ao_segs,pn_ao_segs,[1])
      call c_f_pointer(aosegs,paosegs,pn_ao_segs)

      ncenters = int_constant("ncenters"//C_NULL_CHAR) 
      npcoeffs = int_constant("npcoeffs"//C_NULL_CHAR)
C      nalphas  = int_constant("nalphas"//C_NULL_CHAR) 

c---------------------------------------------------------------------------
c   Compute data block of integrals.
c---------------------------------------------------------------------------

      ix2   = data_2(1)  

      crit = 0.00001 

      diff = dabs((data_2(1)) - 1.0) 
      if (diff .lt. crit) ix2 = 1  

      diff = dabs((data_2(1)) - 2.0) 
      if (diff .lt. crit) ix2 = 2  

      diff = dabs((data_2(1)) - 3.0) 
      if (diff .lt. crit) ix2 = 3  

      diff = dabs((data_2(1)) - 10.0) 
      if (diff .lt. crit) ix2 = 10

      diff = dabs((data_2(1)) - 11.0) 
      if (diff .lt. crit) ix2 = 11

      diff = dabs((data_2(1)) - 12.0) 
      if (diff .lt. crit) ix2 = 12

      offset_1 = 0 
      offset_2 = 0 
      if (index_values_0(1).gt.1) offset_1=paosegs(index_values_0(1)-1) 
      if (index_values_0(2).gt.1) offset_2=paosegs(index_values_0(2)-1) 

      nscr  = zmax  
      niscr = imax 
      call scratch_array(nscr, cscr)
      call c_f_pointer(cscr, scr, [nscr])
      call integer_scratch_array(niscr, ciscr)
      call c_f_pointer(ciscr, iscr, [niscr])

      int_length1 = extents_0(1)+offset_1 
      int_length2 = extents_0(2)+offset_2 
      int_length  = int_length1 * int_length2 
      
      call scratch_array(int_length, cptr_int)
      call c_f_pointer(cptr_int, fptr_int, [int_length])

      call comp_dipole_int(data_0, ix2, ncenters, dnuc, npcoeffs, 
C     *                        nalphas,
     *                        1+offset_1, extents_0(1)+offset_1, 
     *                        1+offset_2, extents_0(2)+offset_2, 
     *                        scr, iscr,
     * 
     *                        fptr_int)
    

C for nuclear dipole moment incoming scalar is altered to contain the
C corresponding nuclear moment
      data_1(1) = dnuc 

c  Remove scratch space  
      call delete_scratch_array(cscr)
      call delete_integer_scratch_array(ciscr)
      call delete_scratch_array(cptr_int)

      return
      end

      subroutine comp_dipole_int(z, ixyz, natoms, dnuc, npcoeffs,
     *        da1, da2, db1, db2, zz, iscr, 
     * 
     *        zsum)
    
c---------------------------------------------------------------------------
c Interface added but should be included instead 
      use, intrinsic :: ISO_C_BINDING
      implicit none

      include 'sip_interface.f' 
      include 'imemory.h' 
c Nuclear dipole moment  
c --------------------------------------------------------------------------- 
      double precision dnuc  
c --------------------------------------------------------------------------- 

      integer (C_INT) nshells
      integer (C_INT) intSpherical
      integer (C_INT) nalpha_occupied 
      integer (C_INT) nbeta_occupied  
      integer (C_INT) nalpha_virtual  
      integer (C_INT) nbeta_virtual   
c ----------------------------------------------------------------------

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

      integer i,j,m,n,nfirst, nints, ncoeff, jatom, fatom, 
     *        natoms, ncsum, npcoeffs, nalpha_pack
      integer component
      integer a, b, a1,a2,b1,b2,lastao, igrad 
      integer m1, m2, n1, n2
      integer ixyz, ix, iy, iz
      integer ixderc
      integer ind(2)
      integer ierr
      integer newcenters, nf, watom(natoms)
      logical dom, don, oldcenter
      double precision acharge(natoms)

      integer flopcount
      logical spherical
      logical*8 l8true, l8spherical

c     integer max_dim_coeff
c     parameter (max_dim_coeff = 5000)
      integer ccbeg_pack(8*npcoeffs), ccend_pack(8*npcoeffs)
      integer ncoeff_pack
      double precision alpha_pack(8*npcoeffs),
     *                 pcoeff_pack(8*npcoeffs)
      double precision xn(natoms), yn(natoms), zn(natoms)
      double precision x1,y1,z1,x2,y2,z2, y
      integer do_atom(natoms)

c Get data needed to compute integrals 
c ---------------------------------------------------------------------- 
c Get predefined data 
c ---------------------------------------------------------------------- 
      nshells = int_constant("nshells"//C_NULL_CHAR)
C      nalpha  = int_constant("nalphas"//C_NULL_CHAR) 
      intSpherical    = int_constant("intspherical"//C_NULL_CHAR)
      nalpha_occupied = int_constant("nalpha_occupied"//C_NULL_CHAR)
      nbeta_occupied  = int_constant("nbeta_occupied"//C_NULL_CHAR)
      nalpha_virtual  = int_constant("nalpha_virtual"//C_NULL_CHAR)
      nbeta_virtual   = int_constant("nbeta_virtual"//C_NULL_CHAR)

c ----------------------------------------------------------------------
c  ATOMIC COORDINATES  
      call predefined_scalar_array("coords"//C_NULL_CHAR,
     *                                  two, sDimCoords,  coords)
      call c_f_pointer(sDimCoords, pdim_coords,[2])
      call c_f_pointer(coords, pcoords, pdim_coords)

c ----------------------------------------------------------------------
c  ATOMIC COORDINATES MAPPED TO SHELLS   
      call predefined_scalar_array("tcoords"//C_NULL_CHAR,
     *                                  two, sDimTCoords, tcoords)
      call c_f_pointer(sDimTCoords, pdim_Tcoords,[2])
      call c_f_pointer(tcoords, ptcoords, pdim_tcoords)

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

      do m = 1, natoms
         xn(m) = pcoords(1,m)
         yn(m) = pcoords(2,m)
         zn(m) = pcoords(3,m)
      enddo

c-------------------------------------------------------------------------
c   Set the xyz flags.  
c-------------------------------------------------------------------------

      if (ixyz .eq. 1 .or. ixyz .eq. 10) then ! x NN moment 
         dnuc = 0.0 
         DO n = 1, natoms  
            dnuc = dnuc + xn(n)*pcharge(n)  
         ENDDO 
         if (ixyz .eq. 10) return 
      endif 

      if (ixyz .eq. 2 .or. ixyz .eq. 11) then ! y NN moment 
         dnuc = 0.0 
         DO n = 1, natoms  
            dnuc = dnuc + yn(n)*pcharge(n)  
         ENDDO 
         if (ixyz .eq. 11) return 
      endif 

      if (ixyz .eq. 3 .or. ixyz .eq. 12) then ! z NN moment 
         dnuc = 0.0 
         DO n = 1, natoms  
            dnuc = dnuc + zn(n)*pcharge(n)  
         ENDDO 
         if (ixyz .eq. 12) return 
      endif 

      if (ixyz .eq. 1) then ! x integrals  
         ix = 1 
         iy = 0 
         iz = 0 
      endif 
      if (ixyz .eq. 2) then ! y integrals  
         ix = 0 
         iy = 1 
         iz = 0 
      endif 
      if (ixyz .eq. 3) then ! z integrals  
         ix = 0 
         iy = 0 
         iz = 1 
      endif 

c-------------------------------------------------------------------------
c   Calculate the integrals over the necessary shell blocks.
c-------------------------------------------------------------------------

       do m = m1, m2
            x1 = ptcoords(1,m)
            y1 = ptcoords(2,m)
            z1 = ptcoords(3,m) 
       do n = n1, n2
            x2 = ptcoords(1,n)  
            y2 = ptcoords(2,n)  
            z2 = ptcoords(3,n)  

            ncsum = ncfps(m) + ncfps(n)

             call pack_coeffs_oed(palphas, pixalphas, ppcoeffs, 
     *                       pixpcoeffs, ncfps, npfps, m, n,
     *                       alpha_pack, nalpha, pcoeff_pack,
     *                       ncoeff, pccbeg, pccend, 
     *                       indx_cc,
     *                       ccbeg_pack, ccend_pack,
     *                       8*npcoeffs)

c ------------------------------------------------------------------------ 
c Compute the xyz integrals. 
c ------------------------------------------------------------------------ 

              call oed__gener_xyz_batch(imax, zmax, 
     *                 nalpha, ncoeff, ncsum, 
     *                 ncfps(m), ncfps(n), npfps(m), npfps(n), 
     *                 ivangmom(m), ivangmom(n), 
     *                 x1,y1,z1,x2,y2,z2,
     *                 alpha_pack, pcoeff_pack, ccbeg_pack, ccend_pack,
     *                 spherical, .false., iscr, ix, iy, iz, nints, 
     *                 nfirst, zz)

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


