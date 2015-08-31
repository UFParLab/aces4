!> special compute_ubatch1 wrruu
!>
!>############# CLASS 6  ############
!>  Do four-center part (mu nu |la si)->(mu nu |si la ),(nu mu|la si),(nu mu|si la)
!>                      (la si |mu nu),(si la |mu nu),(la si |nu mu),(si la |nu mu)
!>
!>        PARDO mu, nu, lambda, sigma
!>
!>                 WHERE mu      < nu
!>                 WHERE lambda  < sigma
!>                 WHERE mu     < lambda
!>                 WHERE nu     != sigma
!>                 WHERE nu     != lambda
!>                 WHERE mu     != sigma
!>
!>                    execute compute_ubatch1 aoint[mu,nu,lambda,sigma] oed_kin oed_ovl fockrohf_a fockrohf_b
!>
!>        ENDPARDO mu, nu, lambda, sigma

      subroutine compute_ubatch1(
     c array_slot_0, rank_0, index_values_0, size_0, extents_0, data_0,
     c array_slot_1, rank_1, index_values_1, size_1, extents_1, data_1,
     c array_slot_2, rank_2, index_values_2, size_2, extents_2, data_2,
     c array_slot_3, rank_3, index_values_3, size_3, extents_3, data_3,
     c array_slot_4, rank_4, index_values_4, size_4, extents_4, data_4,
     c  ierr) BIND(C)
      use, intrinsic :: ISO_C_BINDING
      implicit none

c Interface added BUT should be include NOT added in the future

      include 'sip_interface.f'
      include 'imemory.h'

!     argument array
      integer(C_INT), intent(in)::array_slot_0
      integer(C_INT), intent(in)::rank_0
      integer(C_INT), intent(in)::index_values_0(1:rank_0)
      integer(C_INT), intent(in)::size_0
      integer(C_INT), intent(in)::extents_0(1:rank_0)
      real(C_DOUBLE), intent(out)::data_0(1:size_0)

!     should be static array  oed_kin
      integer(C_INT), intent(in)::array_slot_1
      integer(C_INT), intent(in)::rank_1
      integer(C_INT), intent(in)::index_values_1(1:rank_1)
      integer(C_INT), intent(in)::size_1
      integer(C_INT), intent(in)::extents_1(1:rank_1)
      real(C_DOUBLE), intent(in)::data_1(1:size_1)

!     should be static array oed_ovl
      integer(C_INT), intent(in)::array_slot_2
      integer(C_INT), intent(in)::rank_2
      integer(C_INT), intent(in)::index_values_2(1:rank_2)
      integer(C_INT), intent(in)::size_2
      integer(C_INT), intent(in)::extents_2(1:rank_2)
      real(C_DOUBLE), intent(in)::data_2(1:size_2)

!     should be static array  fockrohf_a
      integer(C_INT), intent(in)::array_slot_3
      integer(C_INT), intent(in)::rank_3
      integer(C_INT), intent(in)::index_values_3(1:rank_3)
      integer(C_INT), intent(in)::size_3
      integer(C_INT), intent(in)::extents_3(1:rank_3)
      real(C_DOUBLE), intent(in)::data_3(1:size_3)

!     should be static array fockrohf_b
      integer(C_INT), intent(in)::array_slot_4
      integer(C_INT), intent(in)::rank_4
      integer(C_INT), intent(in)::index_values_4(1:rank_4)
      integer(C_INT), intent(in)::size_4
      integer(C_INT), intent(in)::extents_4(1:rank_4)
      real(C_DOUBLE), intent(in)::data_4(1:size_4)

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
      TYPE(C_PTR) :: cptr_int

      real(C_DOUBLE), pointer :: fptr_int(:)

!     offsets of atomic indeces  
      integer(C_INT) offset_1, offset_2, offset_3, offset_4  
      integer(C_INT) i, one, two   
      integer(C_INT) int_length1, int_length2, int_length3
      integer(C_INT) int_length4, int_length
c

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
      nscr  = zmax 
      niscr = imax 

c     call TAU_START("start scratch_array")   

      call scratch_array(nscr, cscr) 
      call c_f_pointer(cscr, scr, [nscr]) 

      call integer_scratch_array(niscr, ciscr) 
      call c_f_pointer(ciscr, iscr, [niscr]) 

c     call TAU_STOP("start scratch_array")   

      offset_1 = 0 
      offset_2 = 0 
      offset_3 = 0 
      offset_4 = 0 
      if (index_values_0(1).gt.1) offset_1=paosegs(index_values_0(1)-1) 
      if (index_values_0(2).gt.1) offset_2=paosegs(index_values_0(2)-1) 
      if (index_values_0(3).gt.1) offset_3=paosegs(index_values_0(3)-1) 
      if (index_values_0(4).gt.1) offset_4=paosegs(index_values_0(4)-1)

c     call TAU_START("start compute_uintegrals1")   

      int_length1 = (extents_1(1)+offset_1 - 1+offset_1)
      int_length2 = (extents_1(2)+offset_2 - 1+offset_2)
      int_length3 = (extents_1(3)+offset_3 - 1+offset_3)
      int_length4 = (extents_1(4)+offset_4 - 1+offset_4)
      int_length  = int_length1 * int_length2 * int_length3 *
     +              int_length4

      call scratch_array(int_length, cptr_int)
      call c_f_pointer(cptr_int, fptr_int, [int_length])
c
      call compute_uintegrals1(1+offset_1, extents_0(1)+offset_1,
     *                         1+offset_2, extents_0(2)+offset_2,
     *                         1+offset_3, extents_0(3)+offset_3,
     *                         1+offset_4, extents_0(4)+offset_4,
     *                         data_0, !integrals data_0,  
     *                         scr, ! scratch spaces for integrals data_0,  
  
     *                         iscr, ! integer scratch spaces for integrals 
     *                         pcoords, ptcoords, 
     *                         ppcoeffs, pixpcoeffs, palphas, pixalphas,
     *                         pccbeg, pccend,
  
     *                         data_1, data_2, 
     *                         1, extents_1(1), 1,extents_1(2),
     *                         data_3, data_4, fptr_int)

c     call TAU_STOP("start compute_uintegrals1")   

c ----------------------------------------------------------------------
c
c  Remove scratch space  
      call delete_scratch_array(cscr) 
      call delete_scratch_array(cptr_int) 
      call delete_integer_scratch_array(ciscr) 

c     do i = 1, (extents_0(1)-1)*(extents_0(2)-1)*  
c    *          (extents_0(3)-1)*(extents_0(4)-1)  
c        write(6,*) 'INT6', i, data_0(i), scr(i)  
c     enddo 
c     stop 
      return
      end
