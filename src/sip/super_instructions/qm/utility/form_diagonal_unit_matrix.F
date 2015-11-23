! --------------------------------------------------------------------------
!> @author Jason Byrd QTP
!> @brief
!> forms a diagonal unit matrix from a static array
!>
!> @details
!>
! --------------------------------------------------------------------------

      subroutine form_diagonal_unit_matrix(
     * array_0, rank_0, index_values_0, size_0, extents_0, data_0,
     * ierr) BIND(C)
      use, intrinsic :: ISO_C_BINDING
      implicit none

      include 'sip_interface.f'

! output array --> array_0
      integer(C_INT), intent(in)::array_0
      integer(C_INT), intent(in)::rank_0
      integer(C_INT), intent(in)::index_values_0(1:rank_0)
      integer(C_INT), intent(in)::size_0
      integer(C_INT), intent(in)::extents_0(1:rank_0)
      real(C_DOUBLE), intent(out)::data_0(1:size_0)

      integer(C_INT), intent(out)::ierr


      if (rank_0 .eq. 2 .and. extents_0(1) .eq. extents_0(2)) then 

      call form_diag2(1, extents_0(1),
     *                1, extents_0(2),
     *                data_0) 

      else
	  write(*,*) "ERROR: form_diagonal_unit_matrix 2 index only"
	  call abort_job()
      end if

      ierr = 0

      return 
      end 

      subroutine form_diag2(a1, a2, b1, b2, array)
      
      implicit none
      integer a, a1, a2, b, b1, b2
      integer m1, m2, n1, n2 
      integer offset_1, offset_2 
      double precision array(a1:a2,b1:b2), diag_sum  

      do b = b1, b2 
      do a = a1, a2 
	  array(a,b) = 0.d0
      enddo 
          array(b,b) = 1.d0
      enddo 


      return
      end

