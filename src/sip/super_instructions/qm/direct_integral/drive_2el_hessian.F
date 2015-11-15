C-----------------------------------------------------------------
!> special drive_2el_hessian ru
!>
!> Driver routine to compute the two electron contribution to the 
!> Hessian. Notice that the Hessian integrals are not returned. 
!> Instead, we compute the contribution to the Hessian on the fly. 
!>
!> PARDO mu, nu, lambda, sigma
!>
!>  execute drive_2el_hessian Density{mu,nu,lambda,sigma]  hess[9*natoms*natoms]
!>
!> ENDPARDO mu, nu, lambda, sigma
!>
!>  Ajith Perera, 09/2015.
C-----------------------------------------------------------------

      subroutine drive_2el_hessian(
     c array_slot_0, rank_0, index_values_0, size_0, extents_0, data_0,
     c array_slot_1, rank_1, index_values_1, size_1, extents_1, data_1,
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
      real(C_DOUBLE), intent(in)::data_0(1:size_0)

      integer(C_INT), intent(in)::array_slot_1
      integer(C_INT), intent(in)::rank_1
      integer(C_INT), intent(in)::index_values_1(1:rank_0)
      integer(C_INT), intent(in)::size_1
      integer(C_INT), intent(in)::extents_1(1:rank_1)
      real(C_DOUBLE), intent(in)::data_1(1:size_1)

      integer(C_INT), intent(out)::ierr

c Nuclear coordinates
c ---------------------------------------------------------------------------
      TYPE(C_PTR) :: pDimCoords
      TYPE(C_PTR) :: pcoords
      integer, pointer :: sDimCoords(:)
      real(C_DOUBLE), pointer :: coords(:)

c Nuclear coordinates
c ---------------------------------------------------------------------------
      TYPE(C_PTR) :: pDimTCoords
      TYPE(C_PTR) :: pTcoords
      integer, pointer :: sDimTCoords(:)
      real(C_DOUBLE), pointer :: Tcoords(:)
c ---------------------------------------------------------------------------

      real(C_DOUBLE), pointer :: scr(:) 
      integer, pointer :: iscr(:) 
      TYPE(C_PTR) :: cscr 
      TYPE(C_PTR) :: ciscr 
      TYPE(C_PTR) :: n_ao_segs, aosegs

      integer, pointer :: pn_ao_segs(:), paosegs(:)

!     offsets of atomic indeces  
      integer(C_INT) offset_1, offset_2, offset_3, offset_4  
      integer(C_INT) i, j, one, two
      integer(C_INT) nscr, niscr
      integer(C_INT) ncenters

      ierr = 0 
      one  = 1
      two  = 2
c Number of nuclear centers
c
      ncenters = int_constant("ncenters"//C_NULL_CHAR)

c Nuclear coordinates
      call predefined_scalar_array("coords"//C_NULL_CHAR,
     *                                  two, pDimCoords, pcoords)
      call c_f_pointer(pDimCoords, sDimCoords,[2])
      call c_f_pointer(pcoords, coords, sDimCoords)

c Nuclear Tcoordinates
c
      call predefined_scalar_array("tcoords"//C_NULL_CHAR,
     *                                  two, pDimTCoords, pTcoords)
      call c_f_pointer(pDimTCoords, sDimTCoords,[2])
      call c_f_pointer(pTcoords, Tcoords, sDimTCoords)
c
c Get the ao segment ranges
c
      call predefined_int_array("ao_seg_ranges"//C_NULL_CHAR, one,
     *                           n_ao_segs, aosegs)
      call c_f_pointer(n_ao_segs,pn_ao_segs,[1])
      call c_f_pointer(aosegs,paosegs,pn_ao_segs)


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

c     call TAU_START("start drive_2el_hessian")   

      call return_2el_hessian(
     &
     &                        data_0, data_1, 
     & 
     &                        1+offset_1, extents_0(1)+offset_1,
     &                        1+offset_2, extents_0(2)+offset_2,
     &                        1+offset_3, extents_0(3)+offset_3,
     &                        1+offset_4, extents_0(4)+offset_4,
     &
     &                        ncenters, coords, tcoords,
     &                    
     &                        scr,iscr)

c     call TAU_STOP("start drive_2el_hessian ")   

c ----------------------------------------------------------------------
c
c  Remove scratch space  

      call delete_scratch_array(cscr) 
      call delete_integer_scratch_array(ciscr) 

      return
      end
