! --------------------------------------------------------------------------
!> @author Jason N Byrd QTP.
!> @brief Establishes the lower triangular array elements for a DIIS calculation.
!
!> @details
!>   special compute_diis u
!>
!>   execute compute_diis array
! --------------------------------------------------------------------------

      subroutine compute_diis(
     *  array_0, rank_0, index_values_0, size_0, extents_0, data_0,  
     *  ierr) BIND(C) 
      use, intrinsic :: ISO_C_BINDING 
      implicit none 

c--------------------------------------------------------------------------
!>   special compute_diis u
!>
!>   execute compute_diis array 
!>
!>   Establishes the lower triangular array elements for a DIIS calculation.
c--------------------------------------------------------------------------

      include 'sip_interface.f' 

!     output array --> array_0 
      integer(C_INT), intent(in)::array_0
      integer(C_INT), intent(in)::rank_0
      integer(C_INT), intent(in)::index_values_0(1:rank_0)
      integer(C_INT), intent(in)::size_0
      integer(C_INT), intent(in)::extents_0(1:rank_0)
      real(C_DOUBLE), intent(out)::data_0(1:size_0) 
      integer(C_INT), intent(out)::ierr  

      TYPE(C_PTR) :: cscr1, cscr2, cscr3
      integer(c_int) :: nscr1, nscr2, nscr3
      real(c_double), pointer :: scr1(:)
      real(c_double), pointer :: scr2(:)
      real(c_double), pointer :: scr3(:)

      integer(C_INT)  size, size2  

      size = extents_0(2)   
      size2= (extents_0(2) + 1)**2   

      nscr1 = size2
      nscr2 = size2
      nscr3 = size+1

      call scratch_array(nscr1,cscr1)
      call scratch_array(nscr2,cscr2)
      call scratch_array(nscr3,cscr3)
      call c_f_pointer(cscr1, scr1, [nscr1])
      call c_f_pointer(cscr2, scr2, [nscr2])
      call c_f_pointer(cscr3, scr3, [nscr3])

      call form_R(data_0,scr1,scr2,scr3,1,extents_0(1),size,size2)

      call delete_scratch_array(cscr1)
      call delete_scratch_array(cscr2)
      call delete_scratch_array(cscr3)

      return
      end

