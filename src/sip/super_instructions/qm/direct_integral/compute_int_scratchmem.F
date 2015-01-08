      subroutine compute_int_scratchmem(
     c array_slot_0, rank_0, index_values_0, size_0, extents_0, data_0,
     c array_slot_1, rank_1, index_values_1, size_1, extents_1, data_1,
     c array_slot_2, rank_2, index_values_2, size_2, extents_2, data_2,
     c  ierr) BIND(C)
      use, intrinsic :: ISO_C_BINDING
      implicit none

C------------------------------------------------------
!> special compute_int_scratchmem uuu
!>     Get memory requirements from integral calculation
!>     -------------------------------------------------
!>      imax = 0.0
!>      zmax = 0.0
!>      execute compute_int_scratchmem oed_ovl zmax imax
C------------------------------------------------------

c Interface added BUT should be include NOT added in the future

      include 'sip_interface.f'
      include 'imemory.h'

!     argument array for mock integrals 
      integer(C_INT), intent(in)::array_slot_0
      integer(C_INT), intent(in)::rank_0
      integer(C_INT), intent(in)::index_values_0(1:rank_0)
      integer(C_INT), intent(in)::size_0
      integer(C_INT), intent(in)::extents_0(1:rank_0)
      real(C_DOUBLE), intent(out)::data_0(1:size_0)

!     scalar --> real scratch memory needed for integral calc  
      integer(C_INT), intent(in)::array_slot_1
      integer(C_INT), intent(in)::rank_1
      integer(C_INT), intent(in)::index_values_1(1:rank_1)
      integer(C_INT), intent(in)::size_1
      integer(C_INT), intent(in)::extents_1(1:rank_1)
      real(C_DOUBLE), intent(out)::data_1(1:size_1)

!     scalar --> integer scratch memory needed for integral calc  
      integer(C_INT), intent(in)::array_slot_2
      integer(C_INT), intent(in)::rank_2
      integer(C_INT), intent(in)::index_values_2(1:rank_2)
      integer(C_INT), intent(in)::size_2
      integer(C_INT), intent(in)::extents_2(1:rank_2)
      real(C_DOUBLE), intent(out)::data_2(1:size_2)

      integer(C_INT), intent(out)::ierr

!     one dimensional array of atomic coordinates  
      TYPE(C_PTR) :: sDimCoords 
      TYPE(C_PTR) :: coords 
      integer(C_INT), pointer :: pdim_coords(:)  
      real(C_DOUBLE), pointer :: pcoords(:,:)

!     tw0 dimensional array mapping atomic coordinates to shells  
      TYPE(C_PTR) :: sDimTCoords 
      TYPE(C_PTR) :: Tcoords 
      integer(C_INT), pointer :: pdim_Tcoords(:)  
      real(C_DOUBLE), pointer :: pTcoords(:,:)

!     one dimensional arrays containg ccbeg and ccend  
      TYPE(C_PTR) :: icc, ccbeg, ccend  
      integer(C_INT), pointer :: picc(:), pccbeg(:), pccend(:)  

!     one dimensional array containg alphas(exponents)   
      TYPE(C_PTR) :: ntot_alpha, ixalphas 
      integer(C_INT), pointer :: pntot_alpha(:), pixalphas(:) 
      TYPE(C_PTR) :: alphas
      real(C_DOUBLE) , pointer :: palphas(:)

!     one dimensional array containing coefficients  
      TYPE(C_PTR) :: ntot_pcoeff, ixpcoeffs 
      TYPE(C_PTR) :: pcoeffs  
      integer(C_INT), pointer :: pntot_pcoeff(:), pixpcoeffs(:) 
      real(C_DOUBLE), pointer :: ppcoeffs(:)

c aosegments 
c     integer(C_INT) n_ao_segs(1), aosegs(6)    
      TYPE(C_PTR) :: n_ao_segs, aosegs
      integer(C_INT), pointer :: pn_ao_segs(:), paosegs(:)

      integer(C_INT) offset_1, offset_2, offset_3, offset_4
c ----------------------------------------------------------------------
c  NCFPS 
      TYPE(C_PTR) :: pishells, pncfps 
      integer(C_INT), pointer :: ishells(:) 
      integer(C_INT), pointer :: ncfps(:) 
c ----------------------------------------------------------------------

c ----------------------------------------------------------------------
c  NPFPS 
      TYPE(C_PTR) :: pnpfps 
      integer(C_INT), pointer :: npfps(:) 
c ----------------------------------------------------------------------

c ----------------------------------------------------------------------
c  END_NFPS 
      TYPE(C_PTR) :: pnalpha, pend_nfps
      integer(C_INT), pointer :: nalpha(:)
      integer(C_INT), pointer :: end_nfps(:)
c ----------------------------------------------------------------------

c ----------------------------------------------------------------------
c  INDX_CC   
      TYPE(C_PTR) :: pindx_cc    
      integer(C_INT), pointer :: indx_cc(:) 
c ----------------------------------------------------------------------

c ----------------------------------------------------------------------
c  IVANGMOM  
      TYPE(C_PTR) :: pivangmom   
      integer(C_INT), pointer :: ivangmom(:) 
c ----------------------------------------------------------------------

      integer(C_INT):: nshells, ncenters, intSpherical  
      integer(C_INT):: one, two, m1, m2
      integer(C_INT):: my_imax, my_zmax 
      logical(C_INT):: calc_2der, spherical 

      integer i, nalphas, npcoeffs  

      nullify(pdim_coords)
      nullify(pcoords)
      nullify(pdim_tcoords)
      nullify(ptcoords)
      nullify(picc)
      nullify(pccbeg)
      nullify(pccend)
      nullify(pntot_alpha)
      nullify(pixalphas)
      nullify(palphas)
      nullify(pntot_pcoeff)
      nullify(pixpcoeffs)
      nullify(pn_ao_segs)
      nullify(paosegs)
      nullify(ishells)
      nullify(ncfps)
      nullify(nalpha)
      nullify(end_nfps)
      nullify(indx_cc)
      nullify(ivangmom)


c
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
c     write(6,*) ' NPFPS', ishells(1), (npfps(i), i=1, 
c    *                                           ishells(1))  
c ----------------------------------------------------------------------

c ----------------------------------------------------------------------
c  IVANGMOMO   
      call predefined_int_array("ivangmom"//C_NULL_CHAR, one, pishells, 
     *                                   pivangmom) 
      call c_f_pointer(pishells, ishells, [1]) 
      call c_f_pointer(pivangmom, ivangmom, [ishells]) 
c     write(6,*) ' IVANGMOMOM', ishells(1), (ivangmom(i), i=1, 
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
c Get the ao segment ranges  
      call predefined_int_array("ao_seg_ranges"//C_NULL_CHAR, one,
     *                           n_ao_segs, aosegs)
      call c_f_pointer(n_ao_segs,pn_ao_segs,[1])
      call c_f_pointer(aosegs,paosegs,pn_ao_segs)
c     write(6,*) ' NAO_SEGS', pn_ao_segs(1), (paosegs(i), i=1, 
c    *                                             pn_ao_segs(1)) 
c ----------------------------------------------------------------------

      intSpherical = int_constant("intspherical"//C_NULL_CHAR)
      spherical = .true.
      if (intSpherical .ne. 1) spherical = .false.

      nshells  = int_constant("nshells"//C_NULL_CHAR) 
      ncenters = int_constant("ncenters"//C_NULL_CHAR) 
      nalphas  = int_constant("nalphas"//C_NULL_CHAR) 
      npcoeffs = int_constant("npcoeffs"//C_NULL_CHAR) 

      if(.not. associated(pdim_coords) .or.
     * .not. associated(pcoords) .or.
     * .not. associated(pdim_tcoords) .or.
     * .not.  associated(ptcoords) .or.
     * .not.  associated(picc) .or.
     * .not.  associated(pccbeg) .or.
     * .not.  associated(pccend) .or. 
     * .not.  associated(pntot_alpha) .or. 
     * .not.  associated(pixalphas) .or.
     * .not.  associated(palphas) .or.
     * .not.  associated(pntot_pcoeff) .or.
     * .not.  associated(pixpcoeffs) .or.
     * .not.  associated(pn_ao_segs) .or.
     * .not.  associated(paosegs) .or.
     * .not.  associated(ishells) .or.
     * .not.  associated(ncfps) .or.
     * .not.  associated(npfps) .or.
     * .not.  associated(nalpha) .or.
     * .not.  associated(end_nfps) .or. 
     * .not.  associated(indx_cc) .or. 
     * .not.  associated(ivangmom))
     *    print *, "ERROR fortran pointer is not associated!!!"

      calc_2der = .false. ! No second-derivative capability yet  

      offset_1 = 0 
      offset_2 = 0 
      if (index_values_0(1).gt.1) offset_1=paosegs(index_values_0(1)-1) 
      if (index_values_0(2).gt.1) offset_2=paosegs(index_values_0(2)-1) 
      offset_1 = 1 + offset_1 
      offset_2 = extents_0(1) + offset_2 

c-----------------------------------------------------------------------
c   Find the shell blocks for which we shall loop through.
c-----------------------------------------------------------------------

      call lookup_shell(end_nfps, nshells, offset_1, m1) 
      call lookup_shell(end_nfps, nshells, offset_2, m2) 
c
c     call TAU_START("erd_scratch_mem_cal") 
c     write(6,*) ' M1 M2', m1, m2, 'nshells', nshells 

      call erd_scratch_mem_calc(nshells, ncenters, npcoeffs, nalphas, 
     *                         ivangmom, ncfps, npfps, 
     *                         pcoords, ptcoords, 
     *                         palphas,  ppcoeffs, 
     *                         pixalphas, pixpcoeffs, 
     *                         pccbeg, pccend, indx_cc,
     *                         spherical,ncenters,calc_2der,
     *                         my_imax, my_zmax, 
     * 
     *                         m1, m2)  
c     call TAU_STOP("erd_scratch_mem_cal") 

c     imax = max0(imax,my_imax) 
c     zmax = max0(zmax,my_zmax) 
      imax = my_imax 
      zmax = my_zmax 

C       write(6,*) 'IMAX = ', imax
C       write(6,*) 'ZMAX = ', Zmax
c ----------------------------------------------------------------------
c
      return
      end
