
      subroutine return_h1_moi(
     c array_slot_0, rank_0, index_values_0, size_0, extents_0, data_0,
     c array_slot_1, rank_1, index_values_1, size_1, extents_1, data_1,
     c  ierr) BIND(C)
      use, intrinsic :: ISO_C_BINDING
      implicit none

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

c     mapping of atoms to shells
      TYPE(C_PTR) :: patom
      TYPE(C_PTR) :: pnalpha
      integer, pointer :: atom(:)
      integer, pointer :: nalpha(:)

      integer (C_INT) nfrags     
      integer(C_INT) ncenters 
      integer(C_INT) npcoeffs  

      integer m, ix2, n_apf ! Assumes < 20 atoms in a fragment TO CHANGE  
      integer nscr, niscr  

C temp array
      TYPE(C_PTR) :: ciatom
      integer(C_INT), pointer :: iatom(:)

c ---------------------------------------------------------------------- 
c Get predefined data 
c ---------------------------------------------------------------------- 
      ncenters = int_constant("ncenters"//C_NULL_CHAR) 
      nfrags          = int_constant("nfrags"//C_NULL_CHAR)
      npcoeffs        = int_constant("npcoeffs"//C_NULL_CHAR)

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

c ----------------------------------------------------------------------
c  ATOMIC COORDINATES  
c ----------------------------------------------------------------------
      call predefined_scalar_array("coords"//C_NULL_CHAR,
     *                                  two, sDimCoords,  coords)
      call c_f_pointer(sDimCoords, pdim_coords,[2])
      call c_f_pointer(coords, pcoords, pdim_coords)

c ----------------------------------------------------------------------
c  ATOMIC COORDINATES MAPPED TO SHELLS   
c ----------------------------------------------------------------------
      call predefined_scalar_array("tcoords"//C_NULL_CHAR,
     *                                  two, sDimTCoords, tcoords)
      call c_f_pointer(sDimTCoords, pdim_Tcoords,[2])
      call c_f_pointer(tcoords, ptcoords, pdim_tcoords)

c ----------------------------------------------------------------------
c Get the ao segment ranges  
c ----------------------------------------------------------------------
      call predefined_int_array("ao_seg_ranges"//C_NULL_CHAR, one,
     *                           n_ao_segs, aosegs)
      call c_f_pointer(n_ao_segs,pn_ao_segs,[1])
      call c_f_pointer(aosegs,paosegs,pn_ao_segs)

c---------------------------------------------------------------------------
c   Compute data block of integrals.
c---------------------------------------------------------------------------

      ix2 = data_1(1)  
      n_apf = natoms_frag(ix2)

      call integer_scratch_array(n_apf,ciatom)
      call c_f_pointer(ciatom,iatom,[n_apf])

      do m = 1, n_apf
         iatom(m) = watom_frag(ix2, m)
      enddo


      offset_1 = 0 
      offset_2 = 0 
      if (index_values_0(1).gt.1) offset_1=paosegs(index_values_0(1)-1) 
      if (index_values_0(2).gt.1) offset_2=paosegs(index_values_0(2)-1) 

      nscr  = zmax  
      niscr = imax 
      call scratch_array(nscr, cscr)
      call c_f_pointer(cscr, scr, [nscr])
      call scratch_array(niscr, ciscr)
      call c_f_pointer(ciscr, iscr, [niscr])

      call comp_return_frag_h1(data_0, n_apf, iatom, 
     *                        1+offset_1, extents_0(1)+offset_1, 
     *                        1+offset_2, extents_0(2)+offset_2, 
     *                        npcoeffs, ncenters, pcoords, ptcoords,
     *                        pcharge, scr, iscr) 

c  Remove scratch space  
      call delete_scratch_array(cscr)
      call delete_scratch_array(ciscr)
      call delete_scratch_array(ciatom)

      return
      end

      subroutine comp_return_frag_h1(z, napf, watom, da1, da2, 
     * db1, db2, 
     * npcoeffs, ncenters, coords, tcoords, charge, zz, iscr)
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
      integer napf, watom(napf)
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

c ----------------------------------------------------------------------
c  INDX_CX  
c ----------------------------------------------------------------------
      call predefined_int_array("indx_cc"//C_NULL_CHAR, one,
     *                                   pishells, pindx_cc)
      call c_f_pointer(pishells, ishells, [1])
      call c_f_pointer(pindx_cc, indx_cc, ishells)

c ----------------------------------------------------------------------
c  IVANGMOM  
c ----------------------------------------------------------------------
      call predefined_int_array("ivangmom"//C_NULL_CHAR, one,pishells,
     *                                   pivangmom)
      call c_f_pointer(pishells, ishells, [1])
      call c_f_pointer(pivangmom, ivangmom, ishells)

c ----------------------------------------------------------------------
c  IXALPHAS  
c ----------------------------------------------------------------------
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
c  ALPHAS  
c ----------------------------------------------------------------------
      call predefined_scalar_array("alphas"//C_NULL_CHAR,
     *                              one, ntot_alpha, alphas)
      call c_f_pointer(ntot_alpha, pntot_alpha,[1])
      call c_f_pointer(alphas, palphas, pntot_alpha)

c ----------------------------------------------------------------------
c  PCOEFFS  
c ----------------------------------------------------------------------
      call predefined_scalar_array("pcoeffs"//C_NULL_CHAR,
     *                               one, ntot_pcoeff,pcoeffs)
      call c_f_pointer(ntot_pcoeff, pntot_pcoeff,[1])
      call c_f_pointer(pcoeffs, ppcoeffs, pntot_pcoeff)

c ----------------------------------------------------------------------
c  CCBEG   
c ----------------------------------------------------------------------
      call predefined_int_array("ccbeg"//C_NULL_CHAR, one, icc,
     *                                   ccbeg)
      call c_f_pointer(icc, picc,[1])
      call c_f_pointer(ccbeg,pccbeg,picc)

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
             endif ! (nints .gt. 0) then

c ------------------------------------------------------------------------ 
c NAI (type=1) integrals first.   
c ------------------------------------------------------------------------ 

              call oed__gener_nai_batch(imax, zmax, 
     *    	       nalpha_pack,ncoeff_pack,
     *                 ncsum, ncfps(m), ncfps(n), npfps(m),npfps(n),
     *                 ivangmom(m), ivangmom(n), x1,y1,z1,x2,y2,z2,
     *                 newcenters, xn,yn,zn, acharge, alpha_pack,
     *                 pcoeff_pack, ccbeg_pack, ccend_pack,
     *                 spherical, .true., iscr, nints, nfirst,
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

               endif ! nints 

         enddo   ! n shells
         enddo   ! m shells

                  i = 0
                  do b = db1,db2
                  do a = da1,da2
                     i = i + 1
		     z(i) = zsum(a,b)
                  enddo
                  enddo

      return
      end

