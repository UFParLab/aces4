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
C
      subroutine return_1el_ecpints(
     & array_slot, rank, index_values, size, extents, data,
     &  ierr) BIND(C)

      use, intrinsic :: ISO_C_BINDING

      implicit none
    
c---------------------------------------------------------------------------
c Interface added but should be included instead 

      include 'sip_interface.f' 
      include 'imemory.h' 

c--------------------------------------------------------------------------
!>
!>   return_1el_ecpints must be executed with the following syntax:
!>   execute return_h1 h1(mu,nu)
!>
!>   The array h1 is blocked and as of now a temporary array.
!>
c--------------------------------------------------------------------------

      integer(C_INT), intent(in)::array_slot
      integer(C_INT), intent(in)::rank
      integer(C_INT), intent(in)::index_values(1:rank)
      integer(C_INT), intent(in)::size
      integer(C_INT), intent(in)::extents(1:rank)
      integer(C_INT), intent(out)::ierr
      real(C_DOUBLE), intent(out)::data(1:size)

      integer(C_INT) ncenters, nshells 
      integer(C_INT) npcoeffs, nalphas 
      integer(C_INT) intspherical 
      integer(C_INT) iecp
      integer(C_INT) necp_atom
      integer(C_INT) nval_bfns 
      integer(C_INT) degen_shell 
     
      integer(C_INT) ipseud
      integer(C_INT) necp_atoms
      integer(C_INT) max_ecp_angmom
      integer(C_INT) max_ecp_prims 
      integer(C_INT) max_nprims,max_ncontfns
      integer(C_INT) max_prims_shell
      integer(C_INT) max_conts_shell
      integer(C_INT) max_unfns_shell
      integer(C_INT) max_cofns_shell

      integer(C_INT) max_nshells, max_nsubshells, max_necp_functns
      integer(C_INT) kfls_ecp_length
      integer(C_INT) icount_1
      integer(C_INT) icount_2
      integer(C_INT) length_0
      integer(C_INT) length_1
      integer(C_INT) length_2
      integer(C_INT) length_3
      integer(C_INT) length_4
      integer(C_INT) length_5
      integer(C_INT) cclength 
C
C scratch arrays 
C
      TYPE(C_PTR) :: cptr_kfirst 
      TYPE(C_PTR) :: cptr_klast
      TYPE(C_PTR) :: cptr_igenby

      TYPE(C_PTR) :: cscr
      TYPE(C_PTR) :: ciscr
      TYPE(C_PTR) :: cptr_zsum

      TYPE(C_PTR) :: cptr_work1
      TYPE(C_PTR) :: cptr_work2
      TYPE(C_PTR) :: cptr_work3
      TYPE(C_PTR) :: cptr_work4
      TYPE(C_PTR) :: cptr_work5
      TYPE(C_PTR) :: cptr_work6
      TYPE(C_PTR) :: cptr_work7
      TYPE(C_PTR) :: cptr_work8
      TYPE(C_PTR) :: cptr_work9
      TYPE(C_PTR) :: cptr_work10
      TYPE(C_PTR) :: cptr_work11
      TYPE(C_PTR) :: cptr_work12
      TYPE(C_PTR) :: cptr_work13
      TYPE(C_PTR) :: cptr_work14
      TYPE(C_PTR) :: cptr_work15
      TYPE(C_PTR) :: cptr_work16
      TYPE(C_PTR) :: cptr_work17
      TYPE(C_PTR) :: cptr_work18
      TYPE(C_PTR) :: cptr_work19
      TYPE(C_PTR) :: cptr_work20
      TYPE(C_PTR) :: cptr_work21

      INTEGER(C_INT), pointer :: fptr_kfirst(:)
      INTEGER(C_INT), pointer :: fptr_klast(:)
      INTEGER(C_INT), pointer :: fptr_igenby(:)

      real(C_DOUBLE), pointer :: scr(:)
      INTEGER(C_INT), pointer :: iscr(:)
      real(C_DOUBLE), pointer :: fptr_zsum(:)

      INTEGER(C_INT), pointer :: fptr_work1(:)
      INTEGER(C_INT), pointer :: fptr_work2(:)
      INTEGER(C_INT), pointer :: fptr_work3(:)
      INTEGER(C_INT), pointer :: fptr_work4(:)
      INTEGER(C_INT), pointer :: fptr_work5(:)
      INTEGER(C_INT), pointer :: fptr_work6(:)
      INTEGER(C_INT), pointer :: fptr_work7(:)
      INTEGER(C_INT), pointer :: fptr_work8(:)
      INTEGER(C_INT), pointer :: fptr_work9(:)
      INTEGER(C_INT), pointer :: fptr_work10(:)
      INTEGER(C_INT), pointer :: fptr_work11(:)
      INTEGER(C_INT), pointer :: fptr_work12(:)
      INTEGER(C_INT), pointer :: fptr_work13(:)
      INTEGER(C_INT), pointer :: fptr_work14(:)
      INTEGER(C_INT), pointer :: fptr_work15(:)
      real(C_DOUBLE), pointer :: fptr_work16(:)
      real(C_DOUBLE), pointer :: fptr_work17(:)
      real(C_DOUBLE), pointer :: fptr_work18(:)
      real(C_DOUBLE), pointer :: fptr_work19(:)
      real(C_DOUBLE), pointer :: fptr_work20(:)
      real(C_DOUBLE), pointer :: fptr_work21(:)
C
c ---------------------------------------------------------------------------
c aosegments
c     integer(C_INT) n_ao_segs(1), aosegs(6)
      TYPE(C_PTR) :: n_ao_segs, aosegs
      integer, pointer :: pn_ao_segs(:), paosegs(:)

      integer(C_INT) norb, one, two, offset_1, offset_2
      integer(C_INT) i, j, k, nscr, niscr
      integer(C_INT) zsum_length1, zsum_length2, zsum_length

      Logical spherical, ecp
c
c Nuclear coordinates 
c --------------------------------------------------------------------------- 
      TYPE(C_PTR) :: cptr_coords_length
      TYPE(C_PTR) :: cptr_coords
      integer, pointer :: fptr_coords_length(:)
      real(C_DOUBLE), pointer :: fptr_coords(:,:)

c Nuclear coordinates of shells 
c --------------------------------------------------------------------------- 
      TYPE(C_PTR) :: cptr_tcoords_length
      TYPE(C_PTR) :: cptr_tcoords
      integer, pointer :: fptr_tcoords_length(:)
      real(C_DOUBLE), pointer :: fptr_tcoords(:,:)

c ----------------------------------------------------------------------
c  END_NFPS
      TYPE(C_PTR) :: cptr_pnalpha, cptr_pend_nfps

      integer, pointer :: fptr_nalpha(:)
      integer, pointer :: fptr_end_nfps(:)
c ----------------------------------------------------------------------
c  NCFPS
      TYPE(C_PTR) :: cptr_pishells, cptr_pncfps
      integer, pointer :: fptr_ishells(:)
      integer, pointer :: fptr_ncfps(:)
c ----------------------------------------------------------------------
c  NPFPS
      TYPE(C_PTR) :: cptr_pnpfps
      integer, pointer :: fptr_npfps(:)
c ----------------------------------------------------------------------
c  INDX_CC
      TYPE(C_PTR) :: cptr_pindx_cc
      integer, pointer :: fptr_indx_cc(:)
c ----------------------------------------------------------------------
c  IVANGMOM
      TYPE(C_PTR) :: cptr_pivangmom
      integer, pointer :: fptr_ivangmom(:)
c ----------------------------------------------------------------------
c  ALPHAS
      TYPE(C_PTR) :: cptr_ntot_alphas, cptr_alphas 
      integer, pointer :: fptr_ntot_alphas(:)  
      real(C_DOUBLE) , pointer :: fptr_alphas(:)

      TYPE(C_PTR) :: cptr_ixalphas, cptr_ntot_ixalphas
      integer, pointer ::fptr_ixalphas(:),  fptr_ntot_ixalphas(:)  
C ----------------------------------------------------------------------
C ALPHA NORMS 

      TYPE(C_PTR) :: cptr_ntot_alpha_norms, cptr_alpha_norms
      integer, pointer :: fptr_ntot_alpha_norms(:)
      real(C_DOUBLE) , pointer :: fptr_alpha_norms(:)

c ----------------------------------------------------------------------
c  COEFFS
      TYPE(C_PTR) :: cptr_pcoeffs, cptr_ntot_pcoeffs
      integer, pointer :: fptr_ntot_pcoeffs(:) 
      real(C_DOUBLE), pointer :: fptr_pcoeffs(:)

      TYPE(C_PTR) :: cptr_ixpcoeffs, cptr_ntot_ixpcoeffs
      integer, pointer :: fptr_ntot_ixpcoeffs(:), fptr_ixpcoeffs(:)
c ----------------------------------------------------------------------
C  CHARGES
      TYPE(C_PTR) :: cptr_charge, cptr_charge_length
      integer, pointer :: fptr_charge_length(:)
      real(C_DOUBLE), pointer :: fptr_charge(:)
C
C  ATOMS array
      TYPE(C_PTR) :: cptr_atoms, cptr_atoms_length
      integer, pointer :: fptr_atoms(:), fptr_atoms_length(:)
c ----------------------------------------------------------------------

      TYPE(C_PTR) :: cptr_icc, cptr_ccbeg, cptr_ccend
      integer, pointer :: fptr_icc(:), fptr_ccbeg(:), fptr_ccend(:)

C ECP rank nlp integer arrary

      TYPE(C_PTR) :: cptr_nlp, cptr_nlp_length 
      integer, pointer :: fptr_nlp(:), fptr_nlp_length(:)
      
C ECP exponents array

      TYPE(C_PTR) :: cptr_clp, cptr_clp_length 
      integer, pointer :: fptr_clp_length(:)
      real (C_DOUBLE), pointer :: fptr_clp(:)

C ECP coefficients array

      TYPE(C_PTR) :: cptr_zlp, cptr_zlp_length 
      integer,  pointer ::   fptr_zlp_length(:)
      real (C_DOUBLE), pointer :: fptr_zlp(:)

C ipseux array
  
      TYPE(C_PTR) :: cptr_ipseux, cptr_ipseux_length 
      integer, pointer :: fptr_ipseux(:), fptr_ipseux_length(:)

C llmax array  

      TYPE(C_PTR) :: cptr_llmax, cptr_llmax_length 
      integer, pointer :: fptr_llmax(:), fptr_llmax_length(:)

C kfirst 

      TYPE(C_PTR) :: cptr_kfirst_tmp, cptr_kfirst_tmp_length 
      integer, pointer :: fptr_kfirst_tmp(:)
      integer, pointer :: fptr_kfirst_tmp_length(:)

C klast 

      TYPE(C_PTR) :: cptr_klast_tmp, cptr_klast_tmp_length 
      integer, pointer :: fptr_klast_tmp(:)
      integer, pointer :: fptr_klast_tmp_length(:)

C nelecp

      TYPE(C_PTR) :: cptr_nelecp, cptr_nelecp_length 
      integer, pointer :: fptr_nelecp(:), fptr_nelecp_length(:)

C iqmstr 

      TYPE(C_PTR) :: cptr_iqmstr, cptr_iqmstr_length 
      integer, pointer :: fptr_iqmstr(:), fptr_iqmstr_length(:)

C jstrt 

      TYPE(C_PTR) :: cptr_jstrt, cptr_jstrt_length 
      integer, pointer :: fptr_jstrt(:), fptr_jstrt_length(:)

C jrst

      TYPE(C_PTR) :: cptr_jrs, cptr_jrs_length 
      integer, pointer :: fptr_jrs(:), fptr_jrs_length(:)

C jcostr

      TYPE(C_PTR) :: cptr_jcostr, cptr_jcostr_length 
      integer, pointer :: fptr_jcostr(:), fptr_jcostr_length(:)

C nucstr

      TYPE(C_PTR) :: cptr_nucstr, cptr_nucstr_length 
      integer, pointer :: fptr_nucstr(:), fptr_nucstr_length(:)

C nrcstr

      TYPE(C_PTR) :: cptr_nrcstr, cptr_nrcstr_length 
      integer, pointer :: fptr_nrcstr(:), fptr_nrcstr_length(:)
C
C ---------------------------------------------------------------------
C
      call predefined_int_array("end_nfps"//C_NULL_CHAR, one,
     *                                      cptr_pnalpha, 
     *                                      cptr_pend_nfps)
      call c_f_pointer(cptr_pnalpha, fptr_nalpha, [1])
      call c_f_pointer(cptr_pend_nfps, fptr_end_nfps, fptr_nalpha)
C
      call predefined_int_array("npfps"//C_NULL_CHAR, one,
     *                                   cptr_pishells, 
     *                                   cptr_pnpfps)
      call c_f_pointer(cptr_pishells, fptr_ishells, [1])
      call c_f_pointer(cptr_pnpfps, fptr_npfps, fptr_ishells)
C
      call predefined_int_array("ncfps"//C_NULL_CHAR, one,
     *                                   cptr_pishells, 
     *                                   cptr_pncfps)
      call c_f_pointer(cptr_pishells, fptr_ishells, [1])
      call c_f_pointer(cptr_pncfps, fptr_ncfps, fptr_ishells)
C 
      call predefined_int_array("indx_cc"//C_NULL_CHAR, one,
     *                                     cptr_pishells, 
     *                                     cptr_pindx_cc)
      call c_f_pointer(cptr_pishells, fptr_ishells, [1])
      call c_f_pointer(cptr_pindx_cc, fptr_indx_cc, fptr_ishells)
C
      call predefined_int_array("ivangmom"//C_NULL_CHAR, one,
     *                           cptr_pishells,
     *                           cptr_pivangmom)
      call c_f_pointer(cptr_pishells, fptr_ishells, [1])
      call c_f_pointer(cptr_pivangmom, fptr_ivangmom, fptr_ishells)
C 

      call predefined_scalar_array("alphas"//C_NULL_CHAR,
     *                              one, cptr_ntot_alphas, 
     *                                   cptr_alphas)
      call c_f_pointer(cptr_ntot_alphas, fptr_ntot_alphas,[1])
      call c_f_pointer(cptr_alphas, fptr_alphas, fptr_ntot_alphas)

      call predefined_scalar_array("alpha_norms"//C_NULL_CHAR,
     *                              one, cptr_ntot_alpha_norms, 
     *                                   cptr_alpha_norms)
      call c_f_pointer(cptr_ntot_alpha_norms, 
     *                 fptr_ntot_alpha_norms,[1])
      call c_f_pointer(cptr_alpha_norms, fptr_alpha_norms, 
     *                 fptr_ntot_alpha_norms)
C
      call predefined_scalar_array("pcoeffs"//C_NULL_CHAR,
     *                               one, cptr_ntot_pcoeffs,
     *                                    cptr_pcoeffs)
      call c_f_pointer(cptr_ntot_pcoeffs, fptr_ntot_pcoeffs,[1])
      call c_f_pointer(cptr_pcoeffs, fptr_pcoeffs, 
     *                               fptr_ntot_pcoeffs)
C
      call predefined_int_array("ixalphas"//C_NULL_CHAR, one,
     *                                   cptr_ntot_ixalphas, 
     *                                   cptr_ixalphas)
      call c_f_pointer(cptr_ntot_ixalphas, fptr_ntot_ixalphas,[1])
      call c_f_pointer(cptr_ixalphas, fptr_ixalphas, 
     *                                fptr_ntot_ixalphas)
C   
      call predefined_int_array("ixpcoeffs"//C_NULL_CHAR,one,
     *                                   cptr_ntot_ixpcoeffs, 
     *                                   cptr_ixpcoeffs)
      call c_f_pointer(cptr_ntot_ixpcoeffs, fptr_ntot_ixpcoeffs,[1])
      call c_f_pointer(cptr_ixpcoeffs, fptr_ixpcoeffs, 
     *                                 fptr_ntot_ixpcoeffs)

      call predefined_int_array("ccbeg"//C_NULL_CHAR,one,cptr_icc,
     *                           cptr_ccbeg)
      call c_f_pointer(cptr_icc,fptr_icc,[1])
      call c_f_pointer(cptr_ccbeg,fptr_ccbeg,fptr_icc)

      call predefined_int_array("ccend"//C_NULL_CHAR, one, cptr_icc,
     *                           cptr_ccend)
      call c_f_pointer(cptr_icc,fptr_icc,[1])
      call c_f_pointer(cptr_ccend,fptr_ccend,fptr_icc)

c Nuclear coordinates 

      call predefined_scalar_array("coords"//C_NULL_CHAR,two,
     *                                  cptr_coords_length,
     *                                  cptr_coords)
      call c_f_pointer(cptr_coords_length, fptr_coords_length,[2]) 
      call c_f_pointer(cptr_coords,fptr_coords,fptr_coords_length) 

c Nuclear Tcoordinates (coordinates of each shell)

      call predefined_scalar_array("tcoords"//C_NULL_CHAR,two,
     *                                  cptr_tcoords_length,
     *                                  cptr_tcoords)
      call c_f_pointer(cptr_tcoords_length, fptr_tcoords_length,[2]) 
      call c_f_pointer(cptr_tcoords,fptr_tcoords,fptr_tcoords_length) 
C
      call predefined_scalar_array("charge"//C_NULL_CHAR,
     *                               one, cptr_charge_length,
     *                               cptr_charge)
      call c_f_pointer(cptr_charge_length, fptr_charge_length,[1])
      call c_f_pointer(cptr_charge, fptr_charge, fptr_charge_length)
C
      call predefined_int_array("atom"//C_NULL_CHAR, one,
     *                                   cptr_atoms_length, 
     *                                   cptr_atoms)
      call c_f_pointer(cptr_atoms_length, fptr_atoms_length, [1])
      call c_f_pointer(cptr_atoms, fptr_atoms, fptr_atoms_length)
C
      call predefined_int_array("ipseux"//C_NULL_CHAR,one,
     *                                   cptr_ipseux_length,
     *                                   cptr_ipseux)
      call c_f_pointer(cptr_ipseux_length, fptr_ipseux_length,[1])
      call c_f_pointer(cptr_ipseux, fptr_ipseux, fptr_ipseux_length)

      call predefined_int_array("nlp"//C_NULL_CHAR,one,
     *                                   cptr_nlp_length,
     *                                   cptr_nlp)
      call c_f_pointer(cptr_nlp_length, fptr_nlp_length,[1])
      call c_f_pointer(cptr_nlp, fptr_nlp, fptr_nlp_length)
C
      call predefined_scalar_array("clp"//C_NULL_CHAR,
     *                                  one, cptr_clp_length, 
     *                                  cptr_clp)
      call c_f_pointer(cptr_clp_length, fptr_clp_length, [1])
      call c_f_pointer(cptr_clp, fptr_clp, fptr_clp_length)
C
      call predefined_scalar_array("zlp"//C_NULL_CHAR,
     *                                  one, cptr_zlp_length, 
     *                                  cptr_zlp)
      call c_f_pointer(cptr_zlp_length, fptr_zlp_length, [1])
      call c_f_pointer(cptr_zlp, fptr_zlp, fptr_zlp_length)
C

      call predefined_int_array("llmax"//C_NULL_CHAR,one,
     *                                   cptr_llmax_length,
     *                                   cptr_llmax)
      call c_f_pointer(cptr_llmax_length, fptr_llmax_length,[1])
      call c_f_pointer(cptr_llmax, fptr_llmax, fptr_llmax_length)
C
      call predefined_int_array("kfirst"//C_NULL_CHAR,one,
     *                                   cptr_kfirst_tmp_length,
     *                                   cptr_kfirst_tmp)
      call c_f_pointer(cptr_kfirst_tmp_length, 
     *                 fptr_kfirst_tmp_length,[1])
      call c_f_pointer(cptr_kfirst_tmp, fptr_kfirst_tmp, 
     *                 fptr_kfirst_tmp_length)
C
      call predefined_int_array("klast"//C_NULL_CHAR,one,
     *                                   cptr_klast_tmp_length,
     *                                   cptr_klast_tmp)
      call c_f_pointer(cptr_klast_tmp_length, 
     *                 fptr_klast_tmp_length,[1])
      call c_f_pointer(cptr_klast_tmp, fptr_klast_tmp,
     *                 fptr_klast_tmp_length)
C
      call predefined_int_array("nelecp"//C_NULL_CHAR,one,
     *                                   cptr_nelecp_length,
     *                                   cptr_nelecp)
      call c_f_pointer(cptr_nelecp_length, fptr_nelecp_length,[1])
      call c_f_pointer(cptr_nelecp, fptr_nelecp, fptr_nelecp_length)

      call predefined_int_array("iqmstr"//C_NULL_CHAR,one,
     *                                   cptr_iqmstr_length,
     *                                   cptr_iqmstr)
      call c_f_pointer(cptr_iqmstr_length, fptr_iqmstr_length,[1])
      call c_f_pointer(cptr_iqmstr, fptr_iqmstr, fptr_iqmstr_length)

      call predefined_int_array("jstrt"//C_NULL_CHAR,one,
     *                                   cptr_jstrt_length,
     *                                   cptr_jstrt)
      call c_f_pointer(cptr_jstrt_length, fptr_jstrt_length,[1])
      call c_f_pointer(cptr_jstrt, fptr_jstrt, fptr_jstrt_length)

      call predefined_int_array("jrs"//C_NULL_CHAR,one,
     *                                   cptr_jrs_length,
     *                                   cptr_jrs)
      call c_f_pointer(cptr_jrs_length, fptr_jrs_length,[1])
      call c_f_pointer(cptr_jrs, fptr_jrs, fptr_jrs_length)

      call predefined_int_array("jcostr"//C_NULL_CHAR,one,
     *                                   cptr_jcostr_length,
     *                                   cptr_jcostr)
      call c_f_pointer(cptr_jcostr_length, fptr_jcostr_length,[1])
      call c_f_pointer(cptr_jcostr, fptr_jcostr, fptr_jcostr_length)

      call predefined_int_array("nucstr"//C_NULL_CHAR,one,
     *                                   cptr_nucstr_length,
     *                                   cptr_nucstr)
      call c_f_pointer(cptr_nucstr_length, fptr_nucstr_length,[1])
      call c_f_pointer(cptr_nucstr, fptr_nucstr, fptr_nucstr_length)

      call predefined_int_array("nrcstr"//C_NULL_CHAR,one,
     *                                   cptr_nrcstr_length,
     *                                   cptr_nrcstr)
      call c_f_pointer(cptr_nrcstr_length, fptr_nrcstr_length,[1])
      call c_f_pointer(cptr_nrcstr, fptr_nrcstr, fptr_nrcstr_length)

c  Number of nuclear centers 

      ncenters = int_constant("ncenters"//C_NULL_CHAR) 

c Number of contraction coeffs.

      npcoeffs = int_constant("npcoeffs"//C_NULL_CHAR) 

C Number of exponents

      nalphas  = int_constant("nalphas"//C_NULL_CHAR) 

C Number of shells 

      nshells = int_constant("ntotshells"//C_NULL_CHAR) 

      nval_bfns = int_constant("nbasis_funcs"//C_NULL_CHAR)
C
      ipseud  = int_constant("ipseud"//C_NULL_CHAR)
C
      necp_atom  = int_constant("necp_atoms"//C_NULL_CHAR)

      max_ecp_angmom = int_constant("max_ecp_angmom"//C_NULL_CHAR)

      max_ecp_prims = int_constant("max_ecp_prims"//C_NULL_CHAR)

      intSpherical = int_constant("intspherical"//C_NULL_CHAR)
      spherical = .true.
      if (intSpherical .ne. 1) spherical = .false.

      iecp = int_constant("intspherical"//C_NULL_CHAR)
      ecp = .false.
      if (iecp .gt. 0) ecp = .true.

c ge the ao segment ranges

      call predefined_int_array("ao_seg_ranges"//C_NULL_CHAR, one,
     *                           n_ao_segs, aosegs)
      call c_f_pointer(n_ao_segs,pn_ao_segs,[1])
      call c_f_pointer(aosegs,paosegs,pn_ao_segs)

      ierr = 0

c   Set scratch space 

      nscr  = zmax 
      niscr = imax
     
      call scratch_array(nscr, cscr) 
      call c_f_pointer(cscr, scr, [nscr]) 

      call integer_scratch_array(niscr, ciscr) 
      call c_f_pointer(ciscr, iscr, [niscr])

C#ifdef _DEBUG_LVL0
      Write(6,"(a)") "----------------------------------------"
      Write(6,"(a)") " Printing from return_1el_ecpints BGNS"
      Write(6,"(a,i4)") " The number of atoms: ", ncenters
      Write(6,"(a,i4)") " The number of val. bfns: ", nval_bfns 
      Write(6,"(a,i4)") " The number of shells: ", nshells
      Write(6,"(a,i4)") " The number contraction coefs.: ", 
     *                    npcoeffs
      Write(6,"(a,i4)") " The number primitive (exponents).: ", 
     *                    nalphas 
      Write(6,"(a,i4)") " The number of atoms with ecp: ", 
     *                    necp_atom
      Write(6,"(a,i4)") " The maximum ecp ang. mom.: ",
     *                    max_ecp_angmom
      Write(6,"(a,i4)") " The maximum ecp primitives: ",
     *                    max_ecp_prims
      Write(6,"(a,l)") " The spherical on/off: ", spherical
      Write(6,"(a,l)") " The ECP on/off: ", ecp
      write(*,*)
     
      Write(6,"(a)") " The end_nfps array"
      Write(6,"(10i4)") (fptr_end_nfps(i), i=1,fptr_nalpha(1))
      write(*,*)

      Write(6,"(a)") " The npfps array"
      Write(6,"(10i4)") (fptr_npfps(i), i=1,fptr_ishells(1))
      write(*,*)

      Write(6,"(a)") " The ncfps array"
      Write(6,"(10i4)") (fptr_ncfps(i), i=1,fptr_ishells(1))
      write(*,*)

      Write(6,"(a)") " The indx_cc array"
      Write(6,"(10i4)") (fptr_indx_cc(i), i=1,fptr_ishells(1))
      write(*,*)

      Write(6,"(a)") " The angular momentum array"
      Write(6,"(10i4)") (fptr_ivangmom(i), i=1,fptr_ishells(1))
      write(*,*)

      Write(6,"(a,i4)") " The exponents array", fptr_ntot_alphas(1)
      Write(6,"(5F15.7)") (fptr_alphas(i), i=1,fptr_ntot_alphas(1))

      Write(6,"(a,i4)") " The exponents norms array", 
     *                    fptr_ntot_alpha_norms(1)
      Write(6,"(5F15.7)") (fptr_alpha_norms(i), 
     *                     i=1,fptr_ntot_alpha_norms(1))
      write(*,*)

      Write(6,"(a)") " The contraction coeffs. array"
      Write(6,"(5F15.7)") (fptr_pcoeffs(i), i=1,fptr_ntot_pcoeffs(1))
      write(*,*)

      Write(6,"(a)") " The ixalpha array"
      Write(6,"(10I4)") (fptr_ixalphas(i), i=1,fptr_ntot_ixalphas(1))
      write(*,*)

      Write(6,"(a)") " The ixpcoef array"
      Write(6,"(10I4)") (fptr_ixpcoeffs(i), 
     *                   i=1,fptr_ntot_ixpcoeffs(1))
      write(*,*)

      Write(6,"(a)") " The ipsuex array"
      Write(6,"(10I4)") (fptr_ipseux(i), i=1,fptr_ipseux_length(1))
      write(*,*)

      Write(6,"(a)") " The nlp array"
      Write(6,"(10I4)") (fptr_nlp(i), i=1,fptr_nlp_length(1))
      write(*,*)

      Write(6,"(a)") " The clp array"
      Write(6,"(5F15.7)") (fptr_clp(i), i=1,fptr_clp_length(1))
      write(*,*)

      Write(6,"(a)") " The zlp array"
      Write(6,"(5F15.7)") (fptr_zlp(i), i=1,fptr_zlp_length(1))
      write(*,*)

      Write(6,"(a)") " The llmax array"
      Write(6,"(10I4)") (fptr_llmax(i), i=1,fptr_llmax_length(1))
      write(*,*)

      Write(6,"(a)") " The kfirst array"
      Write(6,"(10I4)") (fptr_kfirst_tmp(i), i=1,
     *                   fptr_kfirst_tmp_length(1))
      write(*,*)

      Write(6,"(a)") " The klast array"
      Write(6,"(10I4)") (fptr_klast_tmp(i), i=1,
     *                   fptr_klast_tmp_length(1))
      write(*,*)

      Write(6,"(a)") " The nelecp array"
      Write(6,"(10I4)") (fptr_nelecp(i), i=1,fptr_nelecp_length(1))
      write(*,*)

      Write(6,"(a)") " The iqmstr array"
      Write(6,"(10I4)") (fptr_iqmstr(i), i=1,fptr_iqmstr_length(1))
      write(*,*)

      Write(6,"(a)") " The jstrt array"
      Write(6,"(10I4)") (fptr_jstrt(i), i=1,fptr_jstrt_length(1))
      write(*,*)

      Write(6,"(a)") " The jrs array"
      Write(6,"(10I4)") (fptr_jrs(i), i=1,fptr_jrs_length(1))
      write(*,*)

      Write(6,"(a)") " The jcostr array"
      Write(6,"(10I4)") (fptr_jcostr(i), i=1,fptr_jcostr_length(1))
      write(*,*)

      Write(6,"(a)") " The nucrst array"
      Write(6,"(10I4)") (fptr_nucstr(i), i=1,fptr_nucstr_length(1))
      write(*,*)

      Write(6,"(a)") " The nrcstr array"
      Write(6,"(10I4)") (fptr_nrcstr(i), i=1,fptr_nrcstr_length(1))
      write(*,*)

      Write(6,"(a)") " The atom coords. array"
      Write(6,"(3F12.6)") ((fptr_coords(i,j),i=1,3),
     *                                 j=1,fptr_coords_length(2))
      write(*,*)

      Write(6,"(a)") " The atom charges array"
      Write(6,"(6F12.6)") (fptr_charge(i),i=1,fptr_charge_length(1))
      write(*,*)

      Write(6,"(a)") " The atom coords. of shells array"
      Write(6,"(3F12.6)") ((fptr_tcoords(i,j),i=1,3),
     *                                  j=1,fptr_tcoords_length(2))
      write(*,*)

      Write(6,"(a)") " The atom to shell map"
      Write(6,"(10I4)") (fptr_atoms(i), i=1,fptr_atoms_length(1))
      write(*,*)
      Write(6,"(a)") " Printing from return_1el_ecpints ENDS"
      Write(6,"(a)") "----------------------------------------"
C#endif
C
C Based on the records in .dat file, lets get the maximum dimensions
C of certain arrays. The ncenters, nshells remains the same. 
C 
      max_nshells      = 0
      max_nsubshells   = 0
      max_necp_functns = 0
      kfls_ecp_length  = 0
      icount_1         = 0
      icount_2         = 0

      do i = 1, ncenters 
        
         kfls_ecp_length = kfls_ecp_length + fptr_llmax(i) + 1
         max_nshells     = max(max_nshells,fptr_iqmstr(i))

         do j = 1, fptr_iqmstr(i)
            icount_1 = icount_1 + 1
            max_nsubshells = max(max_nsubshells,
     *                           fptr_jcostr(icount_1))
         enddo 

         if (fptr_ipseux(i) .ne. 0) then
            icount_2 = icount_2 + fptr_llmax(i) + 1
            max_necp_functns = max(max_necp_functns,
     *                             (fptr_klast_tmp(icount_2)))
         endif 
           
      enddo 
 
      do i = 1, nshells 
        
         k = i - 1
         If (Spherical) then
             degen_shell = (2 * k + 1)
         Else
             degen_shell = ((k + 1) * (k + 2))/2  
         Endif 

         max_prims_shell = Max(max_prims_shell,fptr_nucstr(i))
         max_unfns_shell = Max(max_prims_shell,fptr_nucstr(i) *
     &                         degen_shell)

         max_conts_shell = Max(max_conts_shell,fptr_nrcstr(i))
         max_cofns_shell = Max(max_conts_shell,fptr_nrcstr(i) *
     &                     degen_shell)
  
      enddo 

C 
C#ifdef _DEBUG_LVL0
      Write(6,"(a,a)") " The values of max_nshells, max_nsubshells",
     *                 " max_necp_functns"
      write(6,"(a)")" kfls_ecp_length,max_prims_shell,max_conts_shell:"
      Write(6,"(4I4)") max_nshells, max_nsubshells, max_necp_functns,
     *                 kfls_ecp_length
      Write(6,*) 
      Write(6,"(4I4)") max_nshells, max_nsubshells, max_necp_functns,
     *                 kfls_ecp_length,max_prims_shell,max_conts_shell
C#endif 
C Scratch arrays 

      max_nprims   = nalphas
      max_ncontfns = npcoeffs
      length_0     = max_nprims  * max_conts_shell
      length_1     = ncenters * max_nshells * max_nsubshells
      length_2     = ncenters * max_ecp_angmom 
      length_3     = max_unfns_shell * max_unfns_shell
      length_4     = max_cofns_shell * max_cofns_shell
      length_5     = (nval_bfns       * (nval_bfns + 1))/2
      cclength     = fptr_icc(1)

      call integer_scratch_array(nshells,cptr_work1)
      call c_f_pointer(cptr_work1,fptr_work1,[nshells])

      call integer_scratch_array(nshells,cptr_work2)
      call c_f_pointer(cptr_work2,fptr_work2,[nshells])

      call integer_scratch_array(nshells,cptr_work3)
      call c_f_pointer(cptr_work3,fptr_work3,[nshells])

      call integer_scratch_array(nshells,cptr_work4)
      call c_f_pointer(cptr_work4,fptr_work4,[nshells])

      call integer_scratch_array(nshells,cptr_work5)
      call c_f_pointer(cptr_work5,fptr_work5,[nshells])

      call integer_scratch_array(nshells,cptr_work6)
      call c_f_pointer(cptr_work6,fptr_work6,[nshells])

      call integer_scratch_array(nshells,cptr_work7)
      call c_f_pointer(cptr_work7,fptr_work7,[nshells])

      call integer_scratch_array(nshells,cptr_work8)
      call c_f_pointer(cptr_work8,fptr_work8,[nshells])

      call integer_scratch_array(nshells,cptr_work9)
      call c_f_pointer(cptr_work9,fptr_work9,[nshells])
C
C expnt and contr arrays in a4_ecp_basis_init.F and subsequent 
C calls
C
      call scratch_array(max_nprims,cptr_work10)
      call c_f_pointer(cptr_work10,fptr_work10,[max_nprims])
   
      call scratch_array(length_0,cptr_work11)
      call c_f_pointer(cptr_work11,fptr_work11,[length_0])
C
C scr1-4 (used in a4_ecp_basis_init.F)
C
      call integer_scratch_array(ncenters,cptr_work12)
      call c_f_pointer(cptr_work12,fptr_work12,[ncenters])

      call integer_scratch_array(length_1,cptr_work13)
      call c_f_pointer(cptr_work13,fptr_work13,[length_1])

      call integer_scratch_array(length_1,cptr_work14)
      call c_f_pointer(cptr_work14,fptr_work14,[length_1])

      call integer_scratch_array(length_1,cptr_work15)
      call c_f_pointer(cptr_work15,fptr_work15,[length_1])
C
C Cint, ecpint_4shell, ecpint, tmp1, tmp2, tmp3.
C Cint must be of length_3 (fptr_16)
C The ecpint_4shell, tmp1, tmp2, tmp3 must be of length_4
C (fptr_17,fptr_19,fptr_20,fptr_21)
C The ecpint must be length_5 (fptr_18)
C

      call scratch_array(length_3,cptr_work16)
      call c_f_pointer(cptr_work16,fptr_work16,[length_3])

      call scratch_array(length_4,cptr_work17)
      call c_f_pointer(cptr_work17,fptr_work17,[length_4])

      call scratch_array(length_5,cptr_work18)
      call c_f_pointer(cptr_work18,fptr_work18,[length_5])

      call scratch_array(length_4,cptr_work19)
      call c_f_pointer(cptr_work19,fptr_work19,[length_4])

      call scratch_array(length_4,cptr_work20)
      call c_f_pointer(cptr_work20,fptr_work20,[length_4])

      call scratch_array(length_4,cptr_work21)
      call c_f_pointer(cptr_work21,fptr_work21,[length_4])
c
      call integer_scratch_array(length_2,cptr_kfirst)
      call c_f_pointer(cptr_kfirst,fptr_kfirst,[length_2])

      call integer_scratch_array(length_2,cptr_klast)
      call c_f_pointer(cptr_klast,fptr_klast,[length_2])
       
      call integer_scratch_array(ncenters,cptr_igenby)
      call c_f_pointer(cptr_igenby,fptr_igenby,[ncenters])
C
c---------------------------------------------------------------------------
c   Compute data block of integrals.
c---------------------------------------------------------------------------
c
c Find the offset index ranges 
C
      offset_1 = 0 
      offset_2 = 0 
c
      if (index_values(1) .gt. 1) offset_1 = paosegs(index_values(1)-1) 
      if (index_values(2) .gt. 1) offset_2 = paosegs(index_values(2)-1) 

      zsum_length1 = (extents(1)+offset_1) - (1+offset_1)
      zsum_length2 = (extents(2)+offset_2) - (1+offset_2)
      zsum_length  = zsum_length1 * zsum_length2

      call scratch_array(zsum_length, cptr_zsum)
      call c_f_pointer(cptr_zsum, fptr_zsum, [zsum_length])
c
      call comp_return_1el_ecp_ints(     data, 1+offset_1, 
     *                                   extents(1)+offset_1,
     *                                   1+offset_2, 
     *                                   extents(2)+offset_2,
     * 
     *                                   fptr_clp,fptr_zlp,fptr_nlp,
     *                                   fptr_kfirst_tmp,fptr_klast_tmp,
     *                                   fptr_llmax,fptr_nelecp,
     *                                   fptr_ipseux,ipseud,
     *                                   fptr_kfirst,fptr_klast,
     *                                   fptr_igenby,
     *
     *                                   fptr_iqmstr,fptr_jcostr,
     *                                   fptr_nucstr,fptr_nrcstr,
     *                                   fptr_jstrt,fptr_jrs, 
     *
     *                                   fptr_alphas,fptr_alpha_norms,
     *                                   fptr_pcoeffs,
     *                                   fptr_end_nfps,fptr_npfps,
     *                                   fptr_ncfps,fptr_indx_cc,
     *                                   fptr_ivangmom,
     *                                   fptr_ixalphas,
     *                                   fptr_ixpcoeffs,
     *
     *                                   fptr_work1,fptr_work2,
     *                                   fptr_work3,fptr_work4, 
     *                                   fptr_work5,fptr_work6, 
     *                                   fptr_work7,fptr_work8, 
     *                                   fptr_work9,
     *
     *                                   fptr_work10,fptr_work11,
     *                                   
     *                                   fptr_work12,fptr_work13,
     *                                   fptr_work14,fptr_work15,
     *
     *                                   fptr_work16,fptr_work17,
     *                                   fptr_work18,fptr_work19,
     *                                   fptr_work20,fptr_work21,
     *
     *                                   ncenters,nshells,nalphas,
     *                                   npcoeffs,max_nshells,
     *
     *                                   fptr_ccbeg,fptr_ccend, 
     *                                   cclength, 
     *                                   
     *                                   kfls_ecp_length, 
     *
     *                                   max_ecp_angmom,max_ecp_prims,
     *                                   max_nsubshells,
     *                                   max_necp_functns,necp_atom,
     *
     *                                   nalphas,npcoeffs,nval_bfns, 
     *                                   max_nprims,max_ncontfns,
     * 
     *                                   max_prims_shell,
     *                                   max_conts_shell,
     *                                   max_unfns_shell,
     *                                   max_cofns_shell,
     *                                        
     *                                   spherical,ecp,
     *
     *                                   fptr_coords,fptr_tcoords, 
     *                                   fptr_atoms,fptr_charge,
     * 
     *                                   scr,iscr,fptr_zsum)
c
c  Remove scratch space  

      write(6,"(a)")" Cleanning up scratch space"
      write(6,"(a)")" --------------------------"
      call delete_scratch_array(cscr)
      call delete_integer_scratch_array(ciscr)
      call delete_scratch_array(cptr_zsum)

      call delete_integer_scratch_array(cptr_work1)
      call delete_integer_scratch_array(cptr_work2)
      call delete_integer_scratch_array(cptr_work3)
      call delete_integer_scratch_array(cptr_work4)
      call delete_integer_scratch_array(cptr_work5)
      call delete_integer_scratch_array(cptr_work6)
      call delete_integer_scratch_array(cptr_work7)
      call delete_integer_scratch_array(cptr_work8)
      call delete_integer_scratch_array(cptr_work9)
      call delete_integer_scratch_array(cptr_work10)
      call delete_integer_scratch_array(cptr_work11)
      call delete_integer_scratch_array(cptr_work12)
      call delete_integer_scratch_array(cptr_work13)
      call delete_integer_scratch_array(cptr_work14)
      call delete_integer_scratch_array(cptr_work15)

      call delete_scratch_array(cptr_work16)
      call delete_scratch_array(cptr_work17)
      call delete_scratch_array(cptr_work18)
      call delete_scratch_array(cptr_work19)
      call delete_scratch_array(cptr_work20)
      call delete_scratch_array(cptr_work21)

      write(6,"(a)")" Everything is done"
      write(6,"(a)")" ------------------"

      return
      end
