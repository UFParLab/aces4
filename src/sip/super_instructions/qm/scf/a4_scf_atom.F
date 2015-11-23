

C-------------------------------------------------------------------------
!>
!> special a4_scf_atom wwrrrr
!>
!> execute a4_scf_atom  fockrohf_a fockrohf_b oed_kin oed_nai oed_ovl etemp
!>
!> Obtain an atomic guess as a starting densities of SCF codes. The
!> original ACES III version was coded by Victor Lotrich and adapted to 
!> ACES IV. 
C-------------------------------------------------------------------------

      subroutine a4_scf_atom(
     * array_slot_0, rank_0, index_values_0, size_0, extents_0, data_0,
     * array_slot_1, rank_1, index_values_1, size_1, extents_1, data_1,
     * array_slot_2, rank_2, index_values_2, size_2, extents_2, data_2,
     * array_slot_3, rank_3, index_values_3, size_3, extents_3, data_3,
     * array_slot_4, rank_4, index_values_4, size_4, extents_4, data_4,
     * array_slot_5, rank_5, index_values_5, size_5, extents_5, data_5,
     *
     *  ierr) BIND(C)

      use, intrinsic :: ISO_C_BINDING
      implicit none
c--------------------------------------------------------------------------
      include 'sip_interface.f'
      include 'imemory.h'
C 
C The Alpha atomic density is returned (Fockrohf_a)

      integer(C_INT), intent(in)::array_slot_0
      integer(C_INT), intent(in)::rank_0
      integer(C_INT), intent(in)::index_values_0(1:rank_0)
      integer(C_INT), intent(in)::size_0
      integer(C_INT), intent(in)::extents_0(1:rank_0)
      real(C_DOUBLE), intent(out)::data_0(1:size_0)

C The Beta atomic density is returned(Fockrohf_b)

      integer(C_INT), intent(in)::array_slot_1
      integer(C_INT), intent(in)::rank_1
      integer(C_INT), intent(in)::index_values_1(1:rank_1)
      integer(C_INT), intent(in)::size_1
      integer(C_INT), intent(in)::extents_1(1:rank_1)
      real(C_DOUBLE), intent(out)::data_1(1:size_1)

C Should be static array  of kinetic energy integrals 

      integer(C_INT), intent(in)::array_slot_2
      integer(C_INT), intent(in)::rank_2
      integer(C_INT), intent(in)::index_values_2(1:rank_2)
      integer(C_INT), intent(in)::size_2
      integer(C_INT), intent(in)::extents_2(1:rank_2)
      real(C_DOUBLE), intent(in)::data_2(1:size_2)

C Should be static array  of kinetic and nuclear attraction integrals 

      integer(C_INT), intent(in)::array_slot_3
      integer(C_INT), intent(in)::rank_3
      integer(C_INT), intent(in)::index_values_3(1:rank_3)
      integer(C_INT), intent(in)::size_3
      integer(C_INT), intent(in)::extents_3(1:rank_3)
      real(C_DOUBLE), intent(in)::data_3(1:size_3)

C Should be static array  of overlap integrals

      integer(C_INT), intent(in)::array_slot_4
      integer(C_INT), intent(in)::rank_4
      integer(C_INT), intent(in)::index_values_4(1:rank_4)
      integer(C_INT), intent(in)::size_4
      integer(C_INT), intent(in)::extents_4(1:rank_4)
      real(C_DOUBLE), intent(in)::data_4(1:size_4)

C The target atom index to do the atomic SCF calc.

      integer(C_INT), intent(in)::array_slot_5
      integer(C_INT), intent(in)::rank_5
      integer(C_INT), intent(in)::index_values_5(1:rank_5)
      integer(C_INT), intent(in)::size_5
      integer(C_INT), intent(in)::extents_5(1:rank_5)
      real(C_DOUBLE), intent(in)::data_5(1:size_5)
      real(C_DOUBLE) :: dtarget_atom

      integer(C_INT), intent(out)::ierr
      integer(C_INT)  nscr

C one dimensional array of atomic coordinates
      TYPE(C_PTR) :: sDimCoords
      TYPE(C_PTR) :: coords
      integer, pointer :: pdim_coords(:)
      real(C_DOUBLE), pointer :: pcoords(:,:)

C two dimensional array mapping atomic coordinates to shells
      TYPE(C_PTR) :: sDimTCoords
      TYPE(C_PTR) :: Tcoords
      integer, pointer :: pdim_Tcoords(:)
      real(C_DOUBLE), pointer :: pTcoords(:,:)
c  CHARGES
      TYPE(C_PTR) :: charge, atoms
      integer, pointer :: patoms(:)
      real(C_DOUBLE), pointer :: pcharge(:)

C one dimensional arrays containg ccbeg and ccend
      TYPE(C_PTR) :: icc, ccbeg, ccend
      integer, pointer :: picc(:), pccbeg(:), pccend(:)

C one dimensional array containg alphas(exponents)
      TYPE(C_PTR) :: ntot_alpha, ixalphas
      integer, pointer :: pntot_alpha(:), pixalphas(:)
      TYPE(C_PTR) :: alphas
      real(C_DOUBLE) , pointer :: palphas(:)

C one dimensional array containing coefficients
      TYPE(C_PTR) :: ntot_pcoeff, ixpcoeffs
      TYPE(C_PTR) :: pcoeffs
      integer, pointer :: pntot_pcoeff(:), pixpcoeffs(:)
      real(C_DOUBLE), pointer :: ppcoeffs(:)
C END_NFPS
      TYPE(C_PTR) :: pnalpha, pend_nfps
      integer, pointer :: nalpha(:)
      integer, pointer :: end_nfps(:)
c NCFPS
      TYPE(C_PTR) :: pishells, pncfps
      integer, pointer :: ishells(:)
      integer, pointer :: ncfps(:)
c NPFPS
      TYPE(C_PTR) :: pnpfps
      integer, pointer :: npfps(:)
c INDX_CC
      TYPE(C_PTR) :: pindx_cc
      integer, pointer :: indx_cc(:)
c IVANGMOM
      TYPE(C_PTR) :: pivangmom
      integer, pointer :: ivangmom(:)
c
c ----------------------------------------------------------------------
c     mapping of atoms to shells

      TYPE(C_PTR) :: patom
      integer (C_INT), pointer :: atom(:)
C -----------------------------------------------------------------------

      TYPE (C_PTR)  :: cptr_iscr
      TYPE (C_PTR)  :: cptr_scr
      TYPE (C_PTR)  :: cptr_nalphas_pack
      TYPE (C_PTR)  :: cptr_npcoeffs_pack
      TYPE (C_PTR)  :: cptr_ccbeg_pack
      TYPE (C_PTR)  :: cptr_ccend_pack
      TYPE (C_PTR)  :: cptr_acharge
      TYPE (C_PTR)  :: cptr_alphas_pack
      TYPE (C_PTR)  :: cptr_pcoeffs_pack

      INTEGER (C_INT), pointer :: fptr_iscr(:)
      INTEGER (C_INT), pointer :: fptr_ccbeg_pack(:)
      INTEGER (C_INT), pointer :: fptr_ccend_pack(:)
      REAL (C_DOUBLE), pointer :: fptr_scr(:)
      REAL (C_DOUBLE), pointer :: fptr_acharge(:)
      REAL (C_DOUBLE), pointer :: fptr_alphas_pack(:)
      REAL (C_DOUBLE), pointer :: fptr_pcoeffs_pack(:)

      TYPE (C_PTR) :: cptr_work1
      TYPE (C_PTR) :: cptr_work2
      TYPE (C_PTR) :: cptr_work3
      TYPE (C_PTR) :: cptr_work4
      TYPE (C_PTR) :: cptr_work5
      TYPE (C_PTR) :: cptr_work6
      TYPE (C_PTR) :: cptr_work7
      TYPE (C_PTR) :: cptr_work8
      TYPE (C_PTR) :: cptr_work9
      TYPE (C_PTR) :: cptr_work10
      TYPE (C_PTR) :: cptr_work11
      TYPE (C_PTR) :: cptr_work12
      TYPE (C_PTR) :: cptr_work13
      TYPE (C_PTR) :: cptr_work14
      TYPE (C_PTR) :: cptr_work15
      TYPE (C_PTR) :: cptr_work16
      TYPE (C_PTR) :: cptr_work17
      TYPE (C_PTR) :: cptr_work18
      TYPE (C_PTR) :: cptr_work19
      TYPE (C_PTR) :: cptr_work20

      TYPE (C_PTR) :: cptr_diis1
      TYPE (C_PTR) :: cptr_diis2
      TYPE (C_PTR) :: cptr_diis3
      TYPE (C_PTR) :: cptr_diis4
      TYPE (C_PTR) :: cptr_diis5
      TYPE (C_PTR) :: cptr_diis6
      TYPE (C_PTR) :: cptr_diis7

      TYPE (C_PTR) :: cptr_scr1
      TYPE (C_PTR) :: cptr_scr2
      TYPE (C_PTR) :: cptr_scr3

      REAL (C_DOUBLE), pointer :: fptr_work1(:)
      REAL (C_DOUBLE), pointer :: fptr_work2(:)
      REAL (C_DOUBLE), pointer :: fptr_work3(:)
      REAL (C_DOUBLE), pointer :: fptr_work4(:)
      REAL (C_DOUBLE), pointer :: fptr_work5(:)
      REAL (C_DOUBLE), pointer :: fptr_work6(:)
      REAL (C_DOUBLE), pointer :: fptr_work7(:)
      REAL (C_DOUBLE), pointer :: fptr_work8(:)
      REAL (C_DOUBLE), pointer :: fptr_work9(:)
      REAL (C_DOUBLE), pointer :: fptr_work10(:)
      REAL (C_DOUBLE), pointer :: fptr_work11(:)
      REAL (C_DOUBLE), pointer :: fptr_work12(:)
      REAL (C_DOUBLE), pointer :: fptr_work13(:)
      REAL (C_DOUBLE), pointer :: fptr_work14(:)
      REAL (C_DOUBLE), pointer :: fptr_work15(:)
      REAL (C_DOUBLE), pointer :: fptr_work16(:)
      REAL (C_DOUBLE), pointer :: fptr_work17(:)
      REAL (C_DOUBLE), pointer :: fptr_work18(:)
      REAL (C_DOUBLE), pointer :: fptr_work19(:)
      REAL (C_DOUBLE), pointer :: fptr_work20(:)


      REAL (C_DOUBLE), pointer :: fptr_diis1(:)
      REAL (C_DOUBLE), pointer :: fptr_diis2(:)
      REAL (C_DOUBLE), pointer :: fptr_diis3(:)
      REAL (C_DOUBLE), pointer :: fptr_diis4(:)
      REAL (C_DOUBLE), pointer :: fptr_diis5(:)
      REAL (C_DOUBLE), pointer :: fptr_diis6(:)
      REAL (C_DOUBLE), pointer :: fptr_diis7(:)

      REAL (C_DOUBLE), pointer :: fptr_scr1(:)
      REAL (C_DOUBLE), pointer :: fptr_scr2(:)
      REAL (C_DOUBLE), pointer :: fptr_scr3(:)

      integer (C_INT) iscratch_dim, dscratch_dim
      integer (C_INT) ncenters
      integer (C_INT) nbasis_funcs,n_c_funcs
      integer (C_INT) npcoeffs
      integer (C_INT) nshells
      integer (C_INT) nalpha_occupied, nbeta_occupied
      integer (C_INT) nalpha_virtual,  nbeta_virtual 
      integer (C_INT) scrsize1
      integer (C_INT) maxhist,diis_start,intSpherical,nlindep_tol
      integer (C_INT) diis_dim1, diis_dim2,diis_dim3,diis_dim4
      integer (C_INT) diis_dim5,diis_dim6,diis_dim7
      integer (C_INT) nscr1,nscr2,nscr3
      integer (C_INT) max_iter
   
      integer Nbasis_total2,Nbasis_patom2,n_basis
      integer n_mo_alpha_total, n_mo_beta_total,N_bfns_total
      integer norbmax,vmax,omax
      integer one,two,i
      integer alphas_pack_dim,pcoeffs_pack_dim
 
      logical spherical,aExist,bExist

C ----------------------------------------------------------------------
C  CC_BEG
      call predefined_int_array("ccbeg"//C_NULL_CHAR, one, icc,
     *                                   ccbeg)
      call c_f_pointer(icc, picc,[1])
      call c_f_pointer(ccbeg,pccbeg,picc)
c     write(6,*) ' CC_BEG', picc(1), (pccbeg(i), i=1,
c    *                                           picc(1))
c ----------------------------------------------------------------------
c  CC_END
      call predefined_int_array("ccend"//C_NULL_CHAR, one, icc,
     *                                   ccend)
      call c_f_pointer(icc, picc,[1])
      call c_f_pointer(ccend,pccend,picc)
c     write(6,*) ' CC_END', picc(1), (pccend(i), i=1,
c    *                                           picc(1))
c ----------------------------------------------------------------------
c  ALPHAS(exponents)
      call predefined_scalar_array("alphas"//C_NULL_CHAR,
     *                              one, ntot_alpha, alphas)
      call c_f_pointer(ntot_alpha, pntot_alpha,[1])
      call c_f_pointer(alphas, palphas, pntot_alpha)
c     write(6,*) ' ALPHAS', pntot_alpha(1) , (palphas(i), i=1,
c    *                pntot_alpha(1))
c ----------------------------------------------------------------------
c  IXALPHAS
c ----------------------------------------------------------------------
      call predefined_int_array("ixalphas"//C_NULL_CHAR, one,
     *                                   ntot_alpha, ixalphas)
      call c_f_pointer(ntot_alpha, pntot_alpha,[1])
      call c_f_pointer(ixalphas, pixalphas, pntot_alpha)
C      write(6,*) ' IXALPHAS', pntot_alpha(1), (pixalphas(i), i=1,
C     *                                         pntot_alpha(1))
c ----------------------------------------------------------------------
c  COEFFICIENTS
      call predefined_scalar_array("pcoeffs"//C_NULL_CHAR,
     *                               one, ntot_pcoeff,pcoeffs)
      call c_f_pointer(ntot_pcoeff, pntot_pcoeff,[1])
      call c_f_pointer(pcoeffs, ppcoeffs, pntot_pcoeff)
c     write(6,*) ' PCOEFFS', pntot_pcoeff(1) , (ppcoeffs(i), i=1,
c    *                pntot_pcoeff(1))
c ----------------------------------------------------------------------
c  IXPCOEFFS
c ----------------------------------------------------------------------
      call predefined_int_array("ixpcoeffs"//C_NULL_CHAR,one,
     *                                   ntot_pcoeff, ixpcoeffs)
      call c_f_pointer(ntot_pcoeff, pntot_pcoeff,[1])
      call c_f_pointer(ixpcoeffs, pixpcoeffs, pntot_pcoeff)
C      write(6,*) ' IXPCOEFFS', pntot_pcoeff(1), (pixpcoeffs(i), i=1,
C     *                                         pntot_pcoeff(1))
c ----------------------------------------------------------------------
c  ATOMIC COORDINATES

      call predefined_scalar_array("coords"//C_NULL_CHAR,
     *                                  two, sDimCoords,  coords)
      call c_f_pointer(sDimCoords, pdim_coords,[2])
      call c_f_pointer(coords, pcoords, pdim_coords)
C
c  NUCLEAR CHARGES
c ----------------------------------------------------------------------
      call predefined_scalar_array("charge"//C_NULL_CHAR,
     *                               one, atoms ,charge)
      call c_f_pointer(atoms, patoms,[1])
      call c_f_pointer(charge, pcharge, patoms)

c ----------------------------------------------------------------------
c  mapping of atoms to shells
      call predefined_int_array("atom"//C_NULL_CHAR, one,
     *                                   pnalpha, patom)
      call c_f_pointer(pnalpha, nalpha, [1])
      call c_f_pointer(patom, atom, nalpha)

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
c -----------------------------------------------------------------------
      call predefined_int_array("indx_cc"//C_NULL_CHAR, one,
     *                                     pishells, pindx_cc)
      call c_f_pointer(pishells, ishells, [1])
      call c_f_pointer(pindx_cc, indx_cc, ishells)
c     write(6,*) ' INDX_CC', ishells(1), (indx_cc(i), i=1,
c    *                                           ishells(1))
c ----------------------------------------------------------------------
c  ATOMIC COORDINATES of each shell.

      call predefined_scalar_array("tcoords"//C_NULL_CHAR,
     *                                  two, sDimCoords,  tcoords)
      call c_f_pointer(sDimCoords, pdim_coords,[2])
      call c_f_pointer(tcoords, ptcoords, pdim_coords)
C
C-----------------------------------------------------------------------
C  END_NFPS

      call predefined_int_array("end_nfps"//C_NULL_CHAR, one,
     *                                   pnalpha, pend_nfps)
      call c_f_pointer(pnalpha, nalpha, [1])
      call c_f_pointer(pend_nfps, end_nfps, nalpha)
C
C-----------------------------------------------------------------------
c  IVANGMOM
c ----------------------------------------------------------------------
      call predefined_int_array("ivangmom"//C_NULL_CHAR, one,pishells,
     *                                   pivangmom)
      call c_f_pointer(pishells, ishells, [1])
      call c_f_pointer(pivangmom, ivangmom, ishells)
c ----------------------------------------------------------------------

C Noticed that the imax and zmax is set from ?? and in imemory.h
C
      ierr = 0
      iscratch_dim   = imax
      dscratch_dim   = zmax

      call integer_scratch_array(iscratch_dim,cptr_iscr)
      call c_f_pointer(cptr_iscr,fptr_iscr,[iscratch_dim])
      
      call scratch_array(dscratch_dim,cptr_scr)
      call c_f_pointer(cptr_scr,fptr_scr,[dscratch_dim])

      ncenters        = int_constant("ncenters"//C_NULL_CHAR)
      nbasis_funcs    = int_constant("nbasis_funcs"//C_NULL_CHAR)
      npcoeffs        = int_constant("npcoeffs"//C_NULL_CHAR)
      nshells         = int_constant("nshells"//C_NULL_CHAR)
      nalpha_occupied = int_constant("nalpha_occupied"//C_NULL_CHAR)
      nbeta_occupied  = int_constant("nbeta_occupied"//C_NULL_CHAR)
      nalpha_virtual  = int_constant("nalpha_virtual"//C_NULL_CHAR)
      nbeta_virtual   = int_constant("nbeta_virtual"//C_NULL_CHAR)
      maxhist         = int_constant("scf_hist"//C_NULL_CHAR)
      diis_start      = int_constant("scf_beg"//C_NULL_CHAR)
      intSpherical    = int_constant("intspherical"//C_NULL_CHAR)
      nlindep_tol     = int_constant("lindep_tol"//C_NULL_CHAR)
      n_c_funcs       = int_constant("n_c_funcs"//C_NULL_CHAR)
      max_iter        = int_constant("scf_iter"//C_NULL_CHAR)

      spherical = .false.
      spherical = (intSpherical .Gt. 0)

      N_MO_alpha_total = nalpha_occupied + nalpha_virtual
      N_MO_beta_total  = nbeta_occupied  + nbeta_virtual
      N_bfns_total     = nbasis_funcs
      norbmax          = extents_1(1)
      vmax             = extents_1(1)
      omax             = extents_1(1)

      If ((N_MO_alpha_total .NE. N_bfns_total) .or.
     *    (N_MO_beta_total  .NE. N_bfns_total)) then

           Write(*,"(5a)") "The total number of alpha or beta ",
     *                     "orbitals are not equal to the total ",
     *                     "number of atomic basis functions." ,
     *                     "This is not allowed during HF guess ",
     *                     "generation."

            call abort_job ()
      Endif
C
C In principle we need to write the length of these arrays from
C the set up code. Since it is not there the following length that I 
C allocate for C these arrays are very generous (fail safe). They are in fact
C nshell * (number of contracted function per shell or number of 
C primitive functions per shell). The Nshells * npcoeffs (total 
C primitives) is a overkill. 
C
      alphas_pack_dim   = Nshells*npcoeffs
      pcoeffs_pack_dim  = Nshells*npcoeffs

      call scratch_array(alphas_pack_dim,cptr_alphas_pack)
      call c_f_pointer(cptr_alphas_pack,fptr_alphas_pack,
     *                 [alphas_pack_dim])

      call scratch_array(pcoeffs_pack_dim,cptr_pcoeffs_pack)
      call c_f_pointer(cptr_pcoeffs_pack,fptr_pcoeffs_pack,
     *                 [pcoeffs_pack_dim])

      call integer_scratch_array(pcoeffs_pack_dim,cptr_ccbeg_pack)
      call c_f_pointer(cptr_ccbeg_pack,fptr_ccbeg_pack,
     *                 [pcoeffs_pack_dim])

      call integer_scratch_array(pcoeffs_pack_dim,cptr_ccend_pack)
      call c_f_pointer(cptr_ccend_pack,fptr_ccend_pack,
     *                 [pcoeffs_pack_dim])

      call scratch_array(ncenters,cptr_acharge)
      call c_f_pointer(cptr_acharge,fptr_acharge,[ncenters])
C
C Load of scratch arrays are being use in compute atomscf. Only
C H0T, OVL,KIN, and DA, DB are the input arrays are
C provided as arguments for the SIAL code. Lets get the pointers
C for the rest of the arrays. First the ones that require the
C length of square of the number of basis functions. For convenince
C all these scratch arrays are designated as work1,work2,work3,..
C The CPP pointer is cptr_work1,.... and the FORTRAN pointer is 
C fptr_work1,....
C 
      dtarget_atom = data_5(1)
      call get_num_bfns_patom(atom,end_nfps,n_basis,nshells,
     *                        dtarget_atom)

      Nbasis_total2 = N_bfns_total * N_bfns_total
      Nbasis_patom2 = n_basis      * N_basis 
C       
C FINA,FINB,AOVL,SOS,QXX,HFDOLD_A,HFDOLD_B,FTA,FTB,CBA,CBB,
C NAI,KIN,H0

      call scratch_array(Nbasis_patom2,cptr_work1)
      call c_f_pointer(cptr_work1,fptr_work1,[Nbasis_patom2])

      call scratch_array(Nbasis_patom2,cptr_work2)
      call c_f_pointer(cptr_work2,fptr_work2,[Nbasis_patom2])
       
      call scratch_array(Nbasis_patom2,cptr_work3)
      call c_f_pointer(cptr_work3,fptr_work3,[Nbasis_patom2])
      
      call scratch_array(Nbasis_patom2,cptr_work4)
      call c_f_pointer(cptr_work4,fptr_work4,[Nbasis_patom2])

      call scratch_array(Nbasis_patom2,cptr_work5)
      call c_f_pointer(cptr_work5,fptr_work5,[Nbasis_patom2])

      call scratch_array(Nbasis_patom2,cptr_work6)
      call c_f_pointer(cptr_work6,fptr_work6,[Nbasis_patom2])

      call scratch_array(Nbasis_patom2,cptr_work7)
      call c_f_pointer(cptr_work7,fptr_work7,[Nbasis_patom2])

      call scratch_array(Nbasis_patom2,cptr_work8)
      call c_f_pointer(cptr_work8,fptr_work8,[Nbasis_patom2])

      call scratch_array(Nbasis_total2,cptr_work9)
      call c_f_pointer(cptr_work9,fptr_work9,[Nbasis_total2])

      call scratch_array(Nbasis_total2,cptr_work10)
      call c_f_pointer(cptr_work10,fptr_work10,[Nbasis_total2])

      call scratch_array(Nbasis_patom2,cptr_work11)
      call c_f_pointer(cptr_work11,fptr_work11,[Nbasis_patom2])

      call scratch_array(Nbasis_patom2,cptr_work12)
      call c_f_pointer(cptr_work12,fptr_work12,[Nbasis_patom2])

      call scratch_array(Nbasis_patom2,cptr_work13)
      call c_f_pointer(cptr_work13,fptr_work13,[Nbasis_patom2])

      call scratch_array(Nbasis_patom2,cptr_work14)
      call c_f_pointer(cptr_work14,fptr_work14,[Nbasis_patom2])

      call scratch_array(Nbasis_patom2,cptr_work15)
      call c_f_pointer(cptr_work15,fptr_work15,[Nbasis_patom2])

      call scratch_array(Nbasis_patom2,cptr_work16)
      call c_f_pointer(cptr_work16,fptr_work16,[Nbasis_patom2])

      call scratch_array(Nbasis_total2,cptr_work17)
      call c_f_pointer(cptr_work17,fptr_work17,[Nbasis_total2])

      call scratch_array(Ncenters,cptr_work18)
      call c_f_pointer(cptr_work18,fptr_work18,[Ncenters])

      call scratch_array(Ncenters,cptr_work19)
      call c_f_pointer(cptr_work19,fptr_work19,[Ncenters])

      call scratch_array(Ncenters,cptr_work20)
      call c_f_pointer(cptr_work20,fptr_work20,[Ncenters])
C
C Continue reserving space for DIIS extrapolations. They are
C FAI,FBJ,EA,EB,BM,TBM,CM. Thesr pointer are labelles as cptr_diis1,,
C fptr_diis1,...
C   
      diis_dim1 = vmax * omax   * (maxhist+1)
      diis_dim2 = Nbasis_total2 *  (maxhist+1)
      diis_dim3 = (maxhist+1)   * (maxhist+1)
      diis_dim4 = (maxhist+1)

      call scratch_array(diis_dim1,cptr_diis1)
      call c_f_pointer(cptr_diis1,fptr_diis1,[diis_dim1])

      call scratch_array(diis_dim1,cptr_diis2)
      call c_f_pointer(cptr_diis2,fptr_diis2,[diis_dim1])

      call scratch_array(diis_dim2,cptr_diis3)
      call c_f_pointer(cptr_diis3,fptr_diis3,[diis_dim2])

      call scratch_array(diis_dim2,cptr_diis4)
      call c_f_pointer(cptr_diis4,fptr_diis4,[diis_dim2])

      call scratch_array(diis_dim3,cptr_diis5)
      call c_f_pointer(cptr_diis5,fptr_diis5,[diis_dim3])

      call scratch_array(diis_dim3,cptr_diis6)
      call c_f_pointer(cptr_diis6,fptr_diis6,[diis_dim3])

      call scratch_array(diis_dim4,cptr_diis7)
      call c_f_pointer(cptr_diis7,fptr_diis7,[diis_dim4])
      
      nscr1 = 10 * N_basis
      nscr2 = N_basis * N_basis
      nscr3 = N_basis 

      call scratch_array(nscr1,cptr_scr1)
      call c_f_pointer(cptr_scr1,fptr_scr1,[nscr1])

      call scratch_array(nscr2,cptr_scr2)
      call c_f_pointer(cptr_scr2,fptr_scr2,[nscr2])

      call scratch_array(nscr3,cptr_scr3)
      call c_f_pointer(cptr_scr3,fptr_scr3,[nscr3])

      INQUIRE(file='ca.data',exist=aExist)
      INQUIRE(file='cb.data',exist=bExist)

      if ((aExist .or. bExist) .or. (aExist .and. bExist)) then

         call read_density(data_0,data_1,data_2,data_3,
     *                     1,extents_0(1),1,extents_0(2),
     *                     1,extents_1(1),1,extents_1(2),
     *                     1,extents_2(1),1,extents_2(2),
     *                     1,extents_3(1),1,extents_3(2),
     *
     *                     nalpha_occupied,nbeta_occupied,
     *                     nalpha_virtual,nbeta_virtual,
     *                     N_MO_alpha_total,N_MO_beta_total,
     *                     N_bfns_total)
                                         
      else 
#ifdef _DEBUG_LVL0
      write(6,"(a)") " At the entry to compute atom scf"
      Write(6,"(a)") " N_bfns_total,ncenters,spherical"
      Write(6,"(a)") " nshells,npcoeffs,diis_start,maxhist,nlindep_tol"
      Write(6,"(a)") " vmax,omax,norbmax,n_c_funcs,target_atom,max_iter"
      Write(6,"(12I3,F6.1)")
     *                        N_bfns_total,
     *                        ncenters,
     *                        spherical,nshells,npcoeffs,
     *                        diis_start,maxhist,nlindep_tol,
     *                        vmax,omax,norbmax,n_c_funcs,dtarget_atom,
     *                        max_iter
      Write(*,*) 
      Write(*,"(a)") " The extents of arrays passed in from SIAL"
      Write(*,"(6I4)")
     *                        extents_0(1),extents_0(2),
     *                        extents_1(1),extents_1(2),
     *                        extents_2(1),extents_2(2),
     *                        extents_3(1),extents_3(2),
     *                        extents_4(1),extents_4(2)
     *
CSSS      write(6,"(6(1x,F12.6))")(data_4(i),i=1,size_4)
#endif
         call compute_atomscf(data_0,data_1,data_2,data_3,data_4,
     *                        dtarget_atom,
     *
     *                        1,extents_0(1),1,extents_0(2),
     *                        1,extents_1(1),1,extents_1(2),
     *                        1,extents_2(1),1,extents_2(2),
     *                        1,extents_3(1),1,extents_3(2),
     *                        1,extents_4(1),1,extents_4(2),
     *
     *                        fptr_work1,fptr_work2,fptr_work3,
     *                        fptr_work4,fptr_work5,fptr_work6,
     *                        fptr_work7,fptr_work8,fptr_work9,
     *                        fptr_work10,fptr_work11,fptr_work12,
     *                        fptr_work13,fptr_work14,fptr_work15,
     *                        fptr_work16,fptr_work17,
     *
     *
     *                        fptr_work18,fptr_work19,fptr_work20,
     *
     *                        fptr_diis1,fptr_diis2,fptr_diis3,
     *                        fptr_diis4,fptr_diis5,fptr_diis6,
     *                        fptr_diis7,
     * 
     *                        fptr_iscr,fptr_scr, 
     *
     *                        fptr_scr1,fptr_scr2,fptr_scr3,
     * 
     *                        N_bfns_total,n_c_funcs,n_basis,
     *                        ncenters,pcoords,ptcoords,pcharge,
     *                        fptr_acharge,atom,max_iter,
     *
     *                        ncfps,npfps,
     *                        ppcoeffs,palphas,
     *                        pixalphas,pixpcoeffs,
     *
     *                        spherical,nshells,npcoeffs,
     *
     *                        fptr_alphas_pack,fptr_pcoeffs_pack,
     *
     *                        pccbeg,pccend,fptr_ccbeg_pack,
     *                        fptr_ccend_pack,
     *                        
     *                        ivangmom,indx_cc,end_nfps,
     * 
     *                        diis_start,maxhist,nlindep_tol,
     *                        vmax,omax,norbmax)

      endif 

      call delete_integer_scratch_array(cptr_iscr)
      call delete_scratch_array(cptr_scr)
      call delete_scratch_array(cptr_alphas_pack)
      call delete_scratch_array(cptr_pcoeffs_pack)
      call delete_integer_scratch_array(cptr_ccbeg_pack)
      call delete_integer_scratch_array(cptr_ccend_pack)
      call delete_scratch_array(cptr_acharge)
      call delete_scratch_array(cptr_work1)
      call delete_scratch_array(cptr_work2)
      call delete_scratch_array(cptr_work3)
      call delete_scratch_array(cptr_work4)
      call delete_scratch_array(cptr_work5)
      call delete_scratch_array(cptr_work6)
      call delete_scratch_array(cptr_work7)
      call delete_scratch_array(cptr_work8)
      call delete_scratch_array(cptr_work9)
      call delete_scratch_array(cptr_work10)
      call delete_scratch_array(cptr_work11)
      call delete_scratch_array(cptr_work12)
      call delete_scratch_array(cptr_work13)
      call delete_scratch_array(cptr_work14)
      call delete_scratch_array(cptr_work15)
      call delete_scratch_array(cptr_work16)
      call delete_scratch_array(cptr_work17)
      call delete_scratch_array(cptr_work18)
      call delete_scratch_array(cptr_work19)
      call delete_scratch_array(cptr_work20)
      call delete_scratch_array(cptr_diis1)
      call delete_scratch_array(cptr_diis2)
      call delete_scratch_array(cptr_diis3)
      call delete_scratch_array(cptr_diis4)
      call delete_scratch_array(cptr_diis5)
      call delete_scratch_array(cptr_diis6)
      call delete_scratch_array(cptr_diis7)
      call delete_scratch_array(cptr_scr1)
      call delete_scratch_array(cptr_scr2)
      call delete_scratch_array(cptr_scr3)


      return
      end

C
      subroutine compute_atomscf(da,db,kin,nai,ovl,dtarget_atom,
     *
     *                           na1,na2,nb1,nb2,nc1,nc2,nd1,nd2,
     *                           ne1,ne2,nf1,nf2,ng1,ng2,nh1,nh2,
     *                           ni1,ni2,nj1,nj2,
     *
     *                           hfdold_a,hfdold_b,cba,cbb,fta,
     *                           ftb,ca,cb,fa,fb,h0,aovl,
     *                           sos,qxx,hfd_a,hfd_b,h0T,
     *                            
     *                           xn,yn,zn,
     * 
     *                           fai,fbj,ea,eb,bm,tbm,cm,
     *
     *                           iscr,scr,
     *
     *                           scr1, scr2, scr3,
     * 
     *                           N_bfns_total,n_c_funcs,N_basis, 
     *                           ncenters,coords,tcoords,charge,
     *                           acharge,atom,max_iter,
     *
     *                           ncfps,npfps,coeffs,alphas,
     *                           ixalphas,ixpcoeffs,
     *
     *                           spherical,nshells,npcoeffs,
     * 
     *                           alphas_pack,pcoeffs_pack,

     *                           ccbeg,ccend,ccbeg_pack,ccend_pack,
     *
     *                           ivangmom,indx_cc,end_nfps,
     *                           
     *                           diis_start,maxhist,nlindep_tol,
     *                           vmax,omax,norbmax)

      implicit none 

      integer vmax,omax,norb,maxhist,diis_start,npcoeffs
      integer n_max,ncenters,n_c_funcs
      integer na1,na2,nb1,nb2,nc1,nc2,nd1,nd2,ne1,ne2,nf1,nf2
      integer ng1,ng2,nh1,nh2,ni1,ni2,nj1,nj2
      integer n_basis,norbmax,nshells,N_bfns_total,nlindep_tol
      integer max_iter
C
C The actual declared dimension of ccbeg, ccend is now known.
C They should been written by the program that wrote these 
C arrays. 

      integer ccbeg(n_c_funcs),ccend(n_c_funcs),iscr(*)
C
      integer indx_cc(nshells)
      integer ncfps(nshells), npfps(nshells),end_nfps(nshells)
      integer ivangmom(nshells)
      integer ixpcoeffs(nshells),ixalphas(nshells)
      integer ccend_pack(n_c_funcs)
      integer ccbeg_pack(n_c_funcs)
      integer atom(Nshells)

      integer map(nc1:nc2)
      integer umap(nc1:nc2)
      integer beg_anfps(nshells)
      integer end_anfps(nshells)
      integer m1,n1,r1,s1,m2,n2,s2,r2,m,n,l,k
      integer target_atom
c
      logical spherical 

      double precision scr(*),coeffs(*),alphas(*)
      double precision alphas_pack(npcoeffs*Nshells)
      double precision pcoeffs_pack(npcoeffs*Nshells)
      double precision dtarget_atom
      double precision scr1(10*n_basis),scr2(n_basis*N_basis)
      double precision scr3(n_basis)

      double precision coords(3*ncenters),tcoords(3*Nshells*ncenters)
      double precision charge(ncenters),acharge(ncenters)
      double precision xn(ncenters),yn(ncenters),zn(ncenters)

      double precision da(na1:na2,nb1:nb2)
      double precision db(nc1:nc2,nd1:nd2)
      double precision kin(ne1:ne2,nf1:nf2)
      double precision nai(ng1:ng2,nh1:nh2)
      double precision ovl(ni1:ni2,nj1:nj2)
C 
      double precision cba(N_basis,N_basis)
      double precision cbb(N_basis,N_basis)
      double precision fta(N_basis,N_basis)
      double precision ftb(N_basis,N_basis)
      double precision hfd_a(N_basis,N_basis)
      double precision hfd_b(N_basis,N_basis)
      double precision h0(N_basis,N_basis)
      double precision aovl(N_basis,N_basis)
      double precision sos(N_basis,N_basis)
      double precision qxx(N_basis,N_basis)
      double precision hfdold_a(N_basis,N_basis)
      double precision hfdold_b(N_basis,N_basis)
      double precision ca(N_basis,N_basis)
      double precision cb(N_basis,N_basis)
      double precision fa(N_basis,N_basis)
      double precision fb(N_basis,N_basis)
      double precision H0T(N_bfns_total,N_bfns_total)

      double precision Fai(N_basis,N_basis,maxhist+1)
      double precision Fbj(N_basis,N_basis,maxhist+1)
      double precision Ea(N_basis,N_basis,maxhist+1)
      double precision Eb(N_basis,N_basis,maxhist+1)
      double precision BM(maxhist+1,maxhist+1)
      double precision TBM(maxhist+1,maxhist+1)
      double precision CM(maxhist+1)
C   
      target_atom = INT(dtarget_atom)

#ifdef _DEBUG_LVL0
      Write(*,*) "Entry to comp_return_h0" 
      write(6,*)
      Write(*,*) "Entry to comp_return_h0" 
      write(6,"(a,8I4)") "The mapping array: ",(atom(m),m=1,Nshells)
      write(6,"(a,8I4)") "The end_nfps array:",(end_nfps(m),m=1,Nshells)
      write(6,"(a,8I4)") "The ixalphas:",(ixalphas(m),m=1,Nshells)
      write(6,"(a,8I4)") "The ixpcoefss:",(ixpcoeffs(m),m=1,Nshells)
      Write(*,"(a)") "The atom coordinates at entry to compute_atomscf"
      n=0
      do m=1,ncenters
      Write(*,"(a,4F10.6)") "Atom charge and coords: ", charge(m),
     *                       coords(n+m),coords(n+m+1),coords(n+m+2)
      n = 2*m
      enddo

#endif

      call comp_return_h0(H0T,ng1,ng2,nh1,nh2,scr,iscr,
     *
     *                    ncenters,npcoeffs,nshells,coords,tcoords,
     *                    charge,acharge,
     *
     *                    xn,yn,zn,
     *
     *                    N_bfns_total,n_c_funcs,
     *
     *                    ncfps,npfps,coeffs,alphas,ixalphas,
     *                    ixpcoeffs,
     *
     *                    alphas_pack,pcoeffs_pack,ccbeg,ccend,
     *                    ccbeg_pack,ccend_pack,indx_cc,end_nfps,
     *
     *                    ivangmom,
     *
     *                    atom,target_atom,
     * 
     *                    spherical)

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

            if(atom(m) .eq. target_atom) then 
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
#ifdef _DEBUG_LVL0
      write(6,*)
      write(6,"(a)") "The umap array"
      write(6,"(8I4)") (umap(m),m=nc1,nc2)
      write(6,"(a,I4)") ' The number of basis functions on atom :', 
     *                   n_basis
      write(*,*)
      Write(*,"(a)") "The begining and end _anfps arrays"
      write(6,"(8(1x,I4))") (beg_anfps(m),m=1, Nshells)
      write(6,"(8(1x,I4))") (end_anfps(m),m=1, Nshells)
#endif


      call do_atomscf(da,db,kin,nai,ovl,target_atom,
     *
     *                           na1,na2,nb1,nb2,nc1,nc2,nd1,nd2,
     *                           ne1,ne2,nf1,nf2,ng1,ng2,nh1,nh2,
     *                           ni1,ni2,nj1,nj2,
     *
     *                           hfdold_a,hfdold_b,cba,cbb,fta,
     *                           ftb,ca,cb,fa,fb,h0,aovl,
     *                           sos,qxx,hfd_a,hfd_b,H0T,
     *
     *                           xn,yn,zn,
     *
     *                           fai,fbj,ea,eb,bm,tbm,cm,
     *
     *                           iscr,scr,
     *
     *                           scr1,scr2,scr3,
     *                          
     *                           N_bfns_total,n_basis,n_c_funcs,
     *                           ncenters,coords,tcoords,charge,
     *                           acharge,atom,max_iter,
     *
     *                           spherical,nshells,npcoeffs,
     *
     *                           ncfps,npfps,coeffs,alphas,
     *                           ixpcoeffs,ixalphas,
     *
     *                           alphas_pack,pcoeffs_pack,
     *
     *                           ccbeg,ccend,ccbeg_pack,ccend_pack,
     *                           ivangmom,indx_cc,end_nfps,
     *                            
     *                           map,umap, 
     *
     *                           beg_anfps,end_anfps,
     * 
     *                           diis_start,maxhist,nlindep_tol,
     *                           vmax,omax,norbmax)



      return 
      end 

      subroutine do_atomscf(da,db,kin,nai,ovl,target_atom,
     *
     *                           na1,na2,nb1,nb2,nc1,nc2,nd1,nd2,
     *                           ne1,ne2,nf1,nf2,ng1,ng2,nh1,nh2,
     *                           ni1,ni2,nj1,nj2,
     *
     *                           hfdold_a,hfdold_b,cba,cbb,fta,
     *                           ftb,ca,cb,fa,fb,h0,aovl,
     *                           sos,qxx,hfd_a,hfd_b,H0T,
     *
     *                           xn,yn,zn,
     *
     *                           fai,fbj,ea,eb,bm,tbm,cm,
     *
     *                           iscr,scr,
     *
     *                           scr1,scr2,scr3,
     *
     *                           N_bfns_total,n_basis,n_c_funcs,
     *                           ncenters,coords,tcoords,charge,
     *                           acharge,atom,max_iter,
     *
     *                           spherical,nshells,npcoeffs,
     *
     *                           ncfps,npfps,coeffs,alphas,
     *                           ixpcoeffs,ixalphas,
     *
     *                           alphas_pack,pcoeffs_pack,
     *
     *                           ccbeg,ccend,ccbeg_pack,ccend_pack,
     *                           ivangmom,indx_cc,end_nfps,
     *
     *                           map,umap,
     *
     *                           beg_anfps,end_anfps,
     *
     *                           diis_start,maxhist,nlindep_tol,
     *                           vmax,omax,norbmax)
c
      use, intrinsic :: ISO_C_BINDING

      implicit none

      include 'sip_interface.f'
      include 'imemory.h'

C      TYPE(C_PTR) :: cscr1, cscr2, cscr3
C      integer(c_int) :: nscr1, nscr2, nscr3
C      real(c_double), pointer :: scr1(:)
C      real(c_double), pointer :: scr2(:)
C      real(c_double), pointer :: scr3(:)
C
      logical spherical

      integer vmax,omax,norb,maxhist,diis_start,npcoeffs
      integer na1,na2,nb1,nb2,nc1,nc2,nd1,nd2,ne1,ne2,nf1,nf2
      integer ng1,ng2,nh1,nh2,ni1,ni2,nj1,nj2
      integer n_basis, n_max,ncenters,n_c_funcs
      integer norbmax,nshells,N_bfns_total,nlindep_tol,ierr
      integer nlindep,max_iter
C
      integer ccbeg(n_c_funcs),ccend(n_c_funcs)
      integer atom(nshells),iscr(*),indx_cc(*)
      integer ixalphas(*),ixpcoeffs(*)
      integer ncfps(nshells), npfps(nshells),end_nfps(Nshells)
      integer ivangmom(nshells)
      integer ccend_pack(n_c_funcs),ccbeg_pack(n_c_funcs)
      integer nalpha_pack, npcoeff_pack
      integer map(nc1:nc2)
      integer umap(nc1:nc2)
      integer beg_anfps(nshells)
      integer end_anfps(nshells)
      integer m1,n1,r1,s1,m2,n2,s2,r2,m,n,l,r,s
      integer nocc_b,nocc_a,nvirt_a,nvirt_b
      integer aa1,aa2,bb1,bb2,cc1,cc2,dd1,dd2
      integer itemp,jtemp,iter,ncsum
      integer nints,nfirst,nhist,doit,target_atom
      integer matom,natom,ratom,satom,i
c
      double precision coords(3,ncenters),tcoords(3,Nshells*ncenters)
      double precision scr(*),coeffs(*),alphas(*)
      double precision alphas_pack(*), pcoeffs_pack(*)
      double precision charge(ncenters),acharge(ncenters)
      double precision xn(ncenters),yn(ncenters),zn(ncenters)
      double precision x1,y1,z1,x2,y2,z2,x3,y3,z3,x4,y4,z4

      double precision da(na1:na2,nb1:nb2)
      double precision db(nc1:nc2,nd1:nd2)
      double precision kin(ne1:ne2,nf1:nf2)
      double precision nai(ng1:ng2,nh1:nh2)
      double precision ovl(ni1:ni2,nj1:nj2)

      double precision cba(N_basis,N_basis)
      double precision cbb(N_basis,N_basis)
      double precision fta(N_basis,N_basis)
      double precision ftb(N_basis,N_basis)
      double precision hfd_a(N_basis,N_basis)
      double precision hfd_b(N_basis,N_basis)
      double precision h0(N_basis,N_basis)
      double precision aovl(N_basis,N_basis)
      double precision sos(N_basis,N_basis)
      double precision qxx(N_basis,N_basis)
      double precision hfdold_a(N_basis,N_basis)
      double precision hfdold_b(N_basis,N_basis)
      double precision ca(N_basis,N_basis)
      double precision cb(N_basis,N_basis)
      double precision fa(N_basis,N_basis)
      double precision fb(N_basis,N_basis)
      double precision H0T(N_bfns_total,N_bfns_total)

      double precision scr1(10*N_basis),scr2(N_basis*N_basis)
      double precision scr3(N_basis)

      double precision Fai(N_basis,N_basis,maxhist+1)
      double precision Fbj(N_basis,N_basis,maxhist+1)
      double precision Ea(N_basis,N_basis,maxhist+1)
      double precision Eb(N_basis,N_basis,maxhist+1)
      double precision BM(maxhist+1,maxhist+1)
      double precision TBM(maxhist+1,maxhist+1)
      double precision CM(maxhist+1)
      double precision temp    
C
c ---------------------------------------------------------------------- 

      nocc_a = 0
      nocc_b = 0
C 
C Notice that H atom is handelled as H2. 
C

      do m = 1, ncenters 
         if (m .eq. target_atom) then 
            If (charge(m) .EQ. 1.0D0) then
               nocc_a = 1
               nocc_b = 0
            Else
               nocc_b = int(charge(m))/2 
               nocc_a = int(charge(m)) - nocc_b 
            Endif 
         endif 
      enddo 
      nvirt_a = n_basis - nocc_a  
      nvirt_b = n_basis - nocc_b  

c TEST !!!! 

c      write(6,*) ' Performing an SCF calculation on atom:', target_atom, 
c     * 'in a basis of', n_basis, 'functions with', nocc_a, nocc_b, 
c     * 'alpha and beta occupied electrons and ', nvirt_a, nvirt_b, 
c     * ' alpha and beta virtual electrons'    

C      if ((nocc_a .eq. 0 .OR. nocc_b .eq. 0) return  
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
         FA(m,n) = 0.0  
         FB(m,n) = 0.0  
         FTA(m,n) = 0.0  
         FTB(m,n) = 0.0  
         HFD_a(m,n) = 0.0 
         HFD_b(m,n) = 0.0 
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
 
C      nscr1 = 10*n_basis
C      nscr2 = n_basis*n_basis
C      nscr3 = n_basis
C
C      call scratch_array(nscr1,cscr1)
C      call scratch_array(nscr2,cscr2)
C      call scratch_array(nscr3,cscr3)
C
C      call c_f_pointer(cscr1, scr1, [nscr1])
C      call c_f_pointer(cscr2, scr2, [nscr2])
C      call c_f_pointer(cscr3, scr3, [nscr3])
C
#ifdef _DEBUG_LVL0
      Write(6,"(a)") "The overlap and one-electron hamiltonian"
      Write(6,"(a,2I3)") "The nocca and noccb", nocc_a,nocc_b
      write(6,"(5F15.9)") ((aovl(m,n), m=1,n_basis),n=1,n_basis)
      write(6,*)
      write(6,"(5F15.9)") ((h0(m,n), m=1,n_basis),n=1,n_basis)
#endif

      call eig_sqr_inv(aovl,sos,n_basis,
     *                 scr1,10*N_basis,scr2,scr3,ierr,nlindep)

      if (ierr .ne. 0) call abort_job()

C      write(*,"(a,i3,a)") " Dropping ",nlindep," linear dependencies"

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
C Notice that fock_transpose is defined in mutiple places. This need
C to be a some sort of a library. At the moment, I give a different 
C label (dup:duplicate). The same applies to several other routines.

       call fock_transpose(FA,FB,Qxx,FTa,FTb,n_basis) 

c-----------------------------------------------------------------------
c Diagonalize the transposed Fock matrix  
c-----------------------------------------------------------------------
        call dsyev_wrapper(FTa,ca,n_basis,scr1,10*N_basis,scr2,scr3,
     *                     ierr)
        if (ierr .ne. 0) call abort_job()
        call dsyev_wrapper(FTb,cb,n_basis,scr1,10*N_basis,scr2,scr3,
     *                     ierr)
        if (ierr .ne. 0) call abort_job()
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
            matom = atom(m)
            if(matom .eq. target_atom) then  
            aa1 = beg_anfps(m)
            aa2 = end_anfps(m)

            x1 = tcoords(1,m)
            y1 = tcoords(2,m)
            z1 = tcoords(3,m)
         do n = n1, n2
            
            if (m .le. n) then 
            natom = atom(n)
            if(natom .eq. target_atom) then  
            bb1 = beg_anfps(n)
            bb2 = end_anfps(n)

            x2 = tcoords(1,n)
            y2 = tcoords(2,n)
            z2 = tcoords(3,n)
         do r = r1, r2
            ratom = atom(r)
            if(ratom .eq. target_atom) then  
            cc1 = beg_anfps(r)
            cc2 = end_anfps(r)

            x3 = tcoords(1,r)
            y3 = tcoords(2,r)
            z3 = tcoords(3,r)
         do s = s1, s2
            if (r .le. s) then 
            satom = atom(s)
            if(satom .eq. target_atom) then  
            dd1 = beg_anfps(s)
            dd2 = end_anfps(s)
c
c-----------------------------------------------------------------------
c   Determine the largest density element.
c-----------------------------------------------------------------------

               x4 = tcoords(1,s)
               y4 = tcoords(2,s)
               z4 = tcoords(3,s)

               call pack_coeffs(alphas, ixalphas, coeffs, ixpcoeffs, 
     *                          ncfps, npfps, m, n, 
     *                          r, s, alphas_pack, nalpha_pack, 
     *                          pcoeffs_pack, npcoeff_pack, 
     *                          ccbeg, ccend, indx_cc,
     *                          ccbeg_pack, ccend_pack) 

c---------------------------------------------------------------------------
c   Calling sequence for ERD version 2.
c---------------------------------------------------------------------------

#ifdef _DEBUG_LVL0
      Write(*,*)
      write(*,"(a)")   "aa1,aa2,bb1,bb2,cc1,cc2,dd1,dd2"
      write(*,"(8I4)") aa1,aa2,bb1,bb2,cc1,cc2,dd1,dd2
      write(*,"(a)") "m,n,r,s"
      Write(*,"(4I4)") m,n,r,s

      Write(6,"((a,4(1x,i4)))") "Ivanagmom:", Ivangmom(m),
     &      Ivangmom(n), Ivangmom(r), Ivangmom(s)
      Write(6,"((a,4(1x,i4)))") "Ncfps:", Ncfps(m),
     &      Ncfps(n), Ncfps(r), Ncfps(s)
      Write(6,"((a,4(1x,i4)))") "Npfps:", Npfps(m),
     &      Npfps(n), Npfps(r), Npfps(s)
      Write(6,*)
      Write(6,"(8(1x,F15.7))") (Alphas_pack(i),i=1,Nalpha_pack)
      Write(6,*)
      Write(6,"(8(1x,F15.7))")(Pcoeffs_pack(i),i=1,npcoeff_pack)

      Write(6,"(8(1x,I4))")(CCbeg_pack(i),i=1,Nshells)
      Write(6,*)
      Write(6,"(8(1x,I4))")(CCend_pack(i),i=1,Nshells)
      Write(6,*)
      Write(6,"(a,3(1x,I10))") "The imax and zmax",imax, zmax
      Write(6,*) "The bra/ket coordinates"
      Write(6,*)
      Write(6,"(3(1x,F10.5))") x1, y1, z1
      Write(6,"(3(1x,F10.5))") x2, y2, z2
      Write(6,"(3(1x,F10.5))") x3, y3, z3
      Write(6,"(3(1x,F10.5))") x4, y4, z4

#endif 
               ncsum = ncfps(m) + ncfps(n) + ncfps(r) + ncfps(s)

c              if (doit .eq. 1) then 

               call ERD__GENER_ERI_BATCH(imax, zmax,
     *                nalpha_pack, npcoeff_pack, ncsum, 
     *                ncfps(m),ncfps(n), ncfps(r), ncfps(s),
     *                npfps(m),npfps(n), npfps(r), npfps(s),
     *                ivangmom(m), ivangmom(n), 
     *                ivangmom(r), ivangmom(s), x1,y1,z1,
     *                x2,y2,z2,x3,y3,z3,x4,y4,z4, alphas_pack,
     *                pcoeffs_pack, ccbeg_pack, ccend_pack,
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
C               Write(6,*) "After SS1"
C               write(6,"(6(F10.5))") (scr(nfirst-1+i),i=1,nints)
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

C        Write(*,*) "Entering HF energy"
        call hfenergy(HFD_A,HFD_B,FA,FB,h0,n_basis) 

c-----------------------------------------------------------------------
c       Compute the DIIS coefficients  
c-----------------------------------------------------------------------

C        Write(*,*) "Entering gen_hist"
        call gen_hist(iter,maxhist,diis_start,n_basis,nvirt_a,
     *                nocc_a,nvirt_b,nocc_b,ca,cb,FA,FB,Fai,Fbj,Ea,Eb)

        if ((iter .lt. maxhist+diis_start-1) .and. 
     *      (iter .gt. diis_start)) then 
           nhist = iter - (diis_start-1)  
C         Write(*,*) "do_diis1"
           call do_diis(nhist,nvirt_a,nocc_a,nvirt_b,nocc_b,n_basis,
     *                  Fai,Ea, Fbj,Eb, FA, FB) 
        endif 

        if (iter .ge. maxhist+diis_start-1) then 
           nhist = maxhist   
C           Write(*,*) "do_diis2"
           call do_diis(nhist,nvirt_a,nocc_a,nvirt_b,nocc_b,n_basis,
     *                  Fai,Ea, Fbj,Eb, FA, FB) 
           call move_hist(nhist,diis_start,n_basis,Fa,Fb,Ea,Eb) 
        endif 

c-----------------------------------------------------------------------
c       Transpose the new Fock Matrix   
c-----------------------------------------------------------------------

C        Write(*,*) "fock_transpose"
        call fock_transpose(FA,FB,Qxx,FTa,FTb,n_basis) 

c-----------------------------------------------------------------------
c       Diagonalize the new Transposed Fock Matrix   
c-----------------------------------------------------------------------

        call dsyev_wrapper(FTa,ca,n_basis,scr1,10*N_basis,scr2,scr3,
     *                      ierr)
        if (ierr .ne. 0) call abort_job()
        call dsyev_wrapper(FTb,cb,n_basis,scr1,10*N_basis,scr2,scr3,
     *                     ierr)
        if (ierr .ne. 0) call abort_job()

c-----------------------------------------------------------------------
c       Back Transform the coefficient array  
c-----------------------------------------------------------------------

C        Write(*,*) "back_transpose"
        call c_backtran(Qxx,ca,cb,cba,cbb,n_basis) 

c-----------------------------------------------------------------------
c       Check on convergence and replace the old density with the new   
c-----------------------------------------------------------------------

c-----------------------------------------------------------------------
c       Compute the new HF density  
c-----------------------------------------------------------------------

C        Write(*,*) "hf_density"
        call hfdensity(ca,cb,HFD_A,HFD_B,n_basis,nocc_a,nocc_b) 

c-----------------------------------------------------------------------
c       Check for convergence    
c-----------------------------------------------------------------------

c        Write(*,*) "check_convergence"
        call check_conv(HFD_A,HFD_B,HFDOLD_A,HFDOLD_B,n_basis,doit)  
        if (doit .eq. 1) go to 100 

c-----------------------------------------------------------------------
c Copy the HF Density into the old Density. 
c-----------------------------------------------------------------------

C        Write(*,*) "density copy"
        call hfdensity_copy(HFD_A,HFD_B,HFDOLD_A,HFDOLD_B,n_basis)  


      ENDDO ! iter = 1, max_iter 
100   continue 

c      write(6,*) ' Alpha Orbital energies ' 
c      write(6,*) ' -----------------------------------------------' 
c      do m = 1, nocc_a 
c         write(6,*) '  ', m, FTA(m,m), FTA(m,m)*27.21138386 
c      enddo  
c      write(6,*) ' -----------------------------------------------' 
c      do m = nocc_a + 1, n_basis  
c         write(6,*) '  ', m, FTA(m,m), FTA(m,m)*27.21138386  
c      enddo  
c      write(6,*) ' -----------------------------------------------' 
c      write(6,*) ' ' 

c      write(6,*) ' Beta Orbital energies ' 
c      write(6,*) ' -----------------------------------------------' 
c      do m = 1, nocc_b 
c         write(6,*) '  ', m, FTB(m,m), FTB(m,m)*27.21138386 
c      enddo  
c      write(6,*) ' -----------------------------------------------' 
c      do m = nocc_b + 1, n_basis  
c         write(6,*) '  ', m, FTB(m,m), FTB(m,m)*27.21138386  
c      enddo  
c      write(6,*) ' -----------------------------------------------' 
c      write(6,*) ' ' 

c         write(6,*) ' Done SCF calculation of ATOM :', target_atom

c-----------------------------------------------------------------------
c     Put the density matrix into the full matrix   
c-----------------------------------------------------------------------

#ifdef _DEBUG_LVL0
      Write(6,"(a)") "The intermediate density matrices"
      Write(6,"(6F12.7)") ((HFD_a(m,n), m=1,N_basis),
     *                                  n=1,N_basis)
      Write(*,*) 
      Write(6,"(6F12.7)") ((HFD_b(m,n), m=1,N_basis),
     *                                  n=1,N_basis)
#endif 

      do n = 1, n_basis
      do m = 1, n_basis
         if ((map(m) .ne. 0) .and. (map(n) .ne. 0)) then  
         Da(map(m),map(n)) = HFD_a(m,n)  
         Db(map(m),map(n)) = HFD_b(m,n)   
c        Fina(map(m),map(n)) = FTa(m,n)  
c        Finb(map(m),map(n)) = FTb(m,n)   
         endif 
      enddo 
      enddo 

#ifdef _DEBUG_LVL0
      Write(6,"(a)") "The final density matrices"
      Write(6,"(6F12.7)") ((Da(m,n), m=1,N_bfns_total),
     *                               n=1,N_bfns_total)
      Write(*,*) 
      Write(6,"(6F12.7)") ((Db(m,n), m=1,N_bfns_total),
     *                               n=1,N_bfns_total)
#endif 

C      call delete_scratch_array(cscr1)
C      call delete_scratch_array(cscr2)
C      call delete_scratch_array(cscr3)

      return
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
c      write(6,*) ' Total SCF energy(-NN) = ', etotal 

      return 
      end 
   
c
c ---------------------------------------------------------------------------- 

      subroutine form_ss1fock(int_block,n_basis,aa1,aa2,bb1,
     *                      bb2,cc1,cc2,
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

      subroutine form_ss2fock(int_block,n_basis,aa1,aa2,bb1,
     *                      bb2,cc1,cc2,
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

      subroutine form_ss3fock(int_block,n_basis,aa1,aa2,bb1,
     *                      bb2,cc1,cc2,
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

      subroutine form_ss4fock(int_block,n_basis,aa1,aa2,
     *                      bb1,bb2,cc1,cc2,
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

      subroutine form_ss5fock(int_block,n_basis,aa1,aa2,
     *                      bb1,bb2,cc1,cc2,
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

      subroutine form_ss6fock(int_block,n_basis,aa1,aa2,
     *                      bb1,bb2,cc1,cc2,
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
C            Write(*,*) "S and L", dd1, dd2,cc1, cc2
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

      subroutine form_ss7fock(int_block,n_basis,aa1,aa2,
     *                      bb1,bb2,cc1,cc2,
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

      subroutine form_ss8fock(int_block,n_basis,aa1,aa2,
     *                      bb1,bb2,cc1,cc2,
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

      subroutine form_ss(int_block,m,n,r,s,n_basis,aa1,
     *                   aa2,bb1,bb2,
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

      subroutine hfdensity_copy(HFD_A,HFD_B,HFDOLD_A,HFDOLD_B,
     &                              n_basis)  
      implicit none 

      integer n_basis, a, b 
      double precision HFD_A(n_basis,n_basis) 
      double precision HFD_B(n_basis,n_basis) 
      double precision HFDOLD_A(n_basis,n_basis) 
      double precision HFDOLD_B(n_basis,n_basis) 

#ifdef _DEBUG_LVL0
      write(*,*)
      Write(*,"(a)") "At entry hfdensity_copy the density matrices"
      write(*,"(5F15.8)") ((HFD_A(b,a),a=1,n_basis),b=1,n_basis)
      Write(*,*)
      write(*,"(5F15.8)") ((HFD_B(b,a),a=1,n_basis),b=1,n_basis)
#endif 

      do a = 1, n_basis 
      do b = 1, n_basis 
         HFDOLD_A(b,a) = HFD_A(b,a) 
         HFDOLD_B(b,a) = HFD_B(b,a) 
      enddo 
      enddo 

      return 
      end 


      subroutine check_conv(HFD_A,HFD_B,HFDOLD_A,HFDOLD_B,
     *                          n_basis,flag)  
      use, intrinsic :: ISO_C_BINDING
      implicit none 
      include 'sip_interface.f'

      integer n_basis, a, b, flag  
      double precision HFD_A(n_basis,n_basis) 
      double precision HFD_B(n_basis,n_basis) 
      double precision HFDOLD_A(n_basis,n_basis) 
      double precision HFDOLD_B(n_basis,n_basis) 
      real(C_DOUBLE) max, diffa, diffb, thresh    

      max = 0.d0
      do a = 1, n_basis 
      do b = 1, n_basis 
         diffa = dabs(HFDOLD_A(b,a) - HFD_A(b,a))  
         diffb = dabs(HFDOLD_B(b,a) - HFD_B(b,a))  
         if (diffa .gt. max) max = diffa 
         if (diffb .gt. max) max = diffb 
      enddo 
      enddo 

      thresh = scalar_constant("scf_conv"//C_NULL_CHAR)
      flag = 0 
      if (max .lt. thresh) flag = 1 
C      write(6,*) '      Max density difference :', max 

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

      subroutine gen_hist(kiter,maxhist,diis_start,norb,navirt,
     *                    naocc,nbvirt,nbocc,ca,cb,Fpq_a,Fpq_b,Fai,
     *                    Fbj,Ea,Eb)

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

      subroutine comp_return_h0(zsum,da1,da2,db1,db2,
     *                          scr,iscr,
     *
     *                          ncenters,npcoeffs,nshells,coords,
     *                          tcoords,charge,acharge,
     *
     *                          xn,yn,zn,
     *
     *                          N_bfns_total,n_c_funcs,
     *
     *                          ncfps,npfps,coeffs,alphas,ixalphas,
     *                          ixpcoeffs,
     *
     *                          alphas_pack,pcoeffs_pack,ccbeg,ccend,
     *                          ccbeg_pack,ccend_pack,indx_cc,end_nfps,
     *
     *                          ivangmom,
     *                          
     *                          atom,target_atom,
     *
     *                          spherical)
c---------------------------------------------------------------------------
c  Computes the set of OED integrals of types  
c
c   type = 1 --> NAI integrals
c          2 --> KIN integrals
C  Changes to make GNU compiler happy, C. Taylor, 08/2013.
C 
c---------------------------------------------------------------------------
      use, intrinsic :: ISO_C_BINDING

      implicit none

c---------------------------------------------------------------------------
c Interface added but should be included instead

      include 'sip_interface.f'
      include 'imemory.h'

      integer (C_INT) one

      integer da1,da2,db1,db2
      integer m1,m2,n1,n2, ncenters,npcoeffs,nshells
      integer N_bfns_total,n_c_funcs
      integer m,n,r,s,a1,a2,b1,b2,i
      integer target_atom,ncsum,iatom
      integer ncfps(nshells),npfps(nshells)
      integer ivangmom(nshells),atom(nshells)
      integer nalpha_pack,npcoeff_pack
      integer ixalphas(*),ixpcoeffs(*)
      integer ccbeg(n_c_funcs),ccend(n_c_funcs)
      integer ccbeg_pack(n_c_funcs),ccend_pack(n_c_funcs)
      integer a,b,newcenters,nfirst,nints
      integer iscr(*), end_nfps(nshells),indx_cc(*)
C
      double precision scr(*)
      double precision zsum(da1:da2,db1:db2)
      double precision alphas(*),coeffs(*)
      double precision x1,y1,z1,x2,y2,z2
      double precision coords(3,ncenters),tcoords(3,nshells)
      double precision xn(ncenters),yn(ncenters),zn(ncenters)
      double precision charge(ncenters),acharge(ncenters)
      double precision alphas_pack(Nshells*npcoeffs)
      double precision pcoeffs_pack(Nshells*npcoeffs)

      logical spherical 
C 
      do b = db1,db2
      do a = da1,da2
         zsum(a,b) = 0.d0
      enddo
      enddo
c-----------------------------------------------------------------------

      call lookup_shell(end_nfps, nshells, da1, m1)
      call lookup_shell(end_nfps, nshells, da2, m2)
      call lookup_shell(end_nfps, nshells, db1, n1)
      call lookup_shell(end_nfps, nshells, db2, n2)

#ifdef _DEBUG_LVL0
      write(6,*)
      Write(6,"(a)") "The end_nfps and shell indices"
      Write(6,"(4(1x,I4))")(end_nfps(i),i=1,Nshells)
      Write(6,("(a,4(1x,I4))")) "m1,m2,n1,n2 :", m1,m2,n1,n2
      Write(6,"(4(1x,F15.10))")(alphas(i),i=1,db2-db1)
      Write(6,"(4(1x,I4))")(ixalphas(i),i=1,Nshells)
      Write(6,"(4(1x,I4))")(ixpcoeffs(i),i=1,Nshells)
#endif 
      
c-------------------------------------------------------------------------
c   Build an array of coordinates per atom.
c-------------------------------------------------------------------------

      do m = 1, Nshells 
         iatom = atom(m)
         if (iatom .eq. target_atom) then 
            xn(iatom) = coords(1,iatom)
            yn(iatom) = coords(2,iatom)
            zn(iatom) = coords(3,iatom)
         endif 
      enddo

      do m = 1, ncenters 
         acharge(m) = charge(m) 
      enddo 

      newcenters = 1

      do m = 1, Nshells
         iatom = atom(m)
         if (iatom .eq. target_atom) then 
            xn(1) = coords(1,iatom)
            yn(1) = coords(2,iatom)
            zn(1) = coords(3,iatom)
            acharge(1) = charge(iatom) 
         endif 
      enddo
#ifdef _DEBUG_LVL0
      do m=1,ncenters
      Write(*,"(a,4F10.6)") "Atom charge and coords: ", acharge(m),
     *                       xn(m),yn(m),zn(m)
      enddo
      write(6,"(a,8I4)") "The mapping array: ",(atom(m),m=1,Nshells)
      write(6,"(a,i3)") "The target atom: ", target_atom
      write(6,"(a,i3)") "The number of contracted functions: ", 
     *                   n_c_funcs
#endif
c-------------------------------------------------------------------------
c   Calculate the integrals over the necessary shell blocks.
c-------------------------------------------------------------------------
      m1 = 1
      m2 = nshells
      n1 = 1
      n2 = nshells
      do m = m1, m2

         iatom = atom(m)
         if (iatom .eq. target_atom) then 

       do n = n1, n2

         iatom = atom(n)
         if (iatom .eq. target_atom) then 

            ncsum = ncfps(m) + ncfps(n) 

            x1 = tcoords(1,m)
            y1 = tcoords(2,m)
            z1 = tcoords(3,m)

            x2 = tcoords(1,n)
            y2 = tcoords(2,n)
            z2 = tcoords(3,n)

            call pack_coeffs_oed(alphas, ixalphas, coeffs, ixpcoeffs,
     *                       ncfps, npfps, m, n,
     *                       alphas_pack, nalpha_pack,
     *                       pcoeffs_pack, npcoeff_pack,
     *                       ccbeg, ccend, indx_cc,
     *                       ccbeg_pack, ccend_pack,2*npcoeffs)

c ------------------------------------------------------------------------ 
c  KIN (type=2) integrals next.   
c ------------------------------------------------------------------------ 

C
#ifdef _DEBUG_LVL0
      Write(6,*) "@-entry to KIN BATCH"
      Write(6,"((a,4(1x,i4)))") "The bra/ket shell index m, n", m,n
      Write(6,"(a,2(1x,I4))") "The Nalpha_pack and Npcoeff_pack:",
     &                         nalpha_pack, npcoeff_pack
      Write(6,"((a,4(1x,i4)))") "Ivanagmom:", Ivangmom(m),
     &      Ivangmom(n)
      Write(6,"((a,4(1x,i4)))") "Ncfps:", Ncfps(m),
     &      Ncfps(n)
      Write(6,"((a,4(1x,i4)))") "Npfps:", Npfps(m),
     &      Npfps(n)
      Write(6,*)
      Write(6,"(8(1x,F15.7))") (Alphas_pack(i),i=1,Nalpha_pack)
      Write(6,*)
      Write(6,"(8(1x,F15.7))")(Pcoeffs_pack(i),i=1,npcoeff_pack)
  
      Write(6,"(8(1x,I4))")(CCbeg_pack(i),i=1,Nshells)
      Write(6,*)
      Write(6,"(8(1x,I4))")(CCend_pack(i),i=1,Nshells)
      Write(6,*)
      Write(6,"(a,3(1x,I10))") "The imax and zmax",imax, zmax,
     &                       ncsum
      Write(6,*) "The bra/ket coordinates"
      Write(6,*)
      Write(6,"(3(1x,F10.5))") x1, y1, z1
      Write(6,"(3(1x,F10.5))") x2, y2, z2
#endif

              call oed__gener_kin_batch(imax, zmax, nalpha_pack, 
     *                 npcoeff_pack,
     *                 ncsum, ncfps(m), ncfps(n), npfps(m),npfps(n),
     *                 ivangmom(m), ivangmom(n), x1,y1,z1,x2,y2,z2,
     *                 alphas_pack,
     *                 pcoeffs_pack, ccbeg_pack, ccend_pack,
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

                call add_integrals2(zsum, da1,da2,db1,db2,
     *                 scr(nfirst),a1,a2,b1,b2,1.0d0)

             endif ! (nints .gt. 0) then

c ------------------------------------------------------------------------ 
c NAI (type=1) integrals first.   
c ------------------------------------------------------------------------ 

              call oed__gener_nai_batch(imax, zmax, nalpha_pack, 
     *                 npcoeff_pack,
     *                 ncsum, ncfps(m), ncfps(n), npfps(m),npfps(n),
     *                 ivangmom(m), ivangmom(n), x1,y1,z1,x2,y2,z2,
     *                 newcenters, xn,yn,zn, acharge, alphas_pack,
     *                 pcoeffs_pack, ccbeg_pack, ccend_pack,
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

                 call add_integrals2(zsum, da1,da2,db1,db2,
     *                 scr(nfirst),a1,a2,b1,b2,1.0d0)

               endif ! nints 

            endif 
         enddo   ! n shells
            endif 
         enddo   ! m shells

#ifdef _DEBUG_LVL0
      write(6,"(a)") "The one-electron Hamiltonian ints (per atom)"
      write(6,"(5(1x,F15.9))") ((zsum(a,b),a=da1,da2),b=db1,db2)
#endif 
      return
      end

      subroutine read_density(da,db,ca,cb,
     *                        na1,na2,nb1,nb2,nc1,nc2,nd1,nd2,
     *                        ne1,ne2,nf1,nf2,ng1,ng2,nh1,nh2,
     *
     *                        nalpha_occupied,nbeta_occupied,
     *                        nalpha_virtual,nbeta_virtual,
     *                        N_MO_alpha_total,N_MO_beta_total,
     *                        N_bfns_total)
c---------------------------------------------------------------------------

      implicit none

      integer a,b
      integer na1,na2,nb1,nb2,nc1,nc2,nd1,nd2
      integer ne1,ne2,nf1,nf2,ng1,ng2,nh1,nh2
      integer nalpha_occupied,nbeta_occupied
      integer nalpha_virtual,nbeta_virtual
      integer N_MO_alpha_total,N_MO_beta_total
      integer N_bfns_total
      integer nbasis_from_file,ierror

      double precision ca(na1:na2,nb1:nb2)
      double precision cb(nc1:nc2,nd1:nd2)
      double precision da(ne1:ne2,nf1:nf2)
      double precision db(ng1:ng2,nh1:nh2)

      logical aExist,bExist

c-----------------------------------------------------------------------
c   Check if a guess file exists. If yes READ the orbitals, 
c   construct the density and finish.  
c-----------------------------------------------------------------------

101   Format(5x,'Something wring with read orbitals. N read N correct',
     *       2I8)

      INQUIRE(file='ca.data',exist=aExist)
      INQUIRE(file='cb.data',exist=bExist)

      if (aexist .or. aexist) then

CSSS         if (me .eq. 0 .and. .not. breturn) then

            OPEN (135, File = 'ca.data',
     *            Status = 'Old', Iostat = Ierror)

            rewind 135
            read(135,*) nbasis_from_file
            write(66,*) ' Reading alpha orbitals'
                        if (nbasis_from_file .ne. N_bfns_total)
     *                  write(66,101) nbasis_from_file, N_bfns_total
            do a = 1, N_bfns_total
            do b = 1, N_bfns_total
               read(135,*) ca(a,b)
               cb(a,b) = ca(a,b)
            enddo
            enddo

            close (135)

            write(66,*) ' Computing the RHF HF density'
            call hfdensity(ca,cb,dA,dB,N_bfns_total,nalpha_occupied,
     *                     nbeta_occupied)
CSS            breturn = .true.

            return

CSSS         endif

      endif 

      if (aexist .and. bexist) then

CSSS         if (me .eq. 0 .and. .not. breturn) then

            OPEN (135, File = 'ca.data',
     *            Status = 'Old', Iostat = Ierror)

            rewind 135
            read(135,*) nbasis_from_file
            write(66,*) ' Reading alpha orbitals'
                        if (nbasis_from_file .ne. N_bfns_total)
     *                  write(66,101) nbasis_from_file,N_bfns_total
            do a = 1, N_bfns_total
            do b = 1, N_bfns_total
               read(135,*) ca(a,b)
            enddo
            enddo

            close (135)
            OPEN (357, File = 'cb.data',
     *            Status = 'Old', Iostat = Ierror)

            rewind 357
            read(357,*)  nbasis_from_file 
            write(66,*) ' Reading beta orbitals'
                        if (nbasis_from_file .ne. N_bfns_total)
     *                  write(66,101) nbasis_from_file,N_bfns_total
            do a = 1, N_bfns_total 
            do b = 1, N_bfns_total
               read(357,*) cb(a,b)
            enddo
            enddo
            close(357)

            write(66,*) ' Computing the UHF density'
            call hfdensity(ca,cb,da,db,N_bfns_total,nalpha_occupied,
     *                     nbeta_occupied)
            return

CSSS         endif

      endif ! aexist and bexist 

CSSS      if (breturn) return

c-----------------------------------------------------------------------
c   Done initial guess from file  
c-----------------------------------------------------------------------

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

#ifdef _DEBUG_LVL0
      Write(6,"(a)") "The CA and CB arrays in hfdensity"
      Write(6,"(a,2I3)") "The nocca and noccb", nocc_a,nocc_b
      write(6,"(5F15.9)") ((Ca(i,j), i=1,n_basis),j=1,n_basis)
      write(6,*) 
      write(6,"(5F15.9)") ((CB(i,j), i=1,n_basis),j=1,n_basis)
#endif
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

      subroutine get_num_bfns_patom(atom,end_nfps,n_basis,Nshells,
     *                              dtarget_atom)
      implicit none

      integer n_basis, target_atom, Nshells 
      integer n,m,m1,m2
      integer atom(Nshells),end_nfps(Nshells)
      double precision dtarget_atom

      m1 = 1
      m2 = Nshells
      n_basis = 0
      target_atom = INT(dtarget_atom)

      do m = m1, m2

         if(atom(m) .eq. target_atom) then

            if (m .eq. 1) then
                DO n = 1, end_nfps(m)
                   n_basis = n_basis + 1
                enddo
             else

                 DO n = end_nfps(m-1) + 1, end_nfps(m)
                        n_basis = n_basis + 1
                 enddo
             endif

         endif

      enddo

      return
      end



