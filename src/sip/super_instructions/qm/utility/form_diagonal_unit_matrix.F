! --------------------------------------------------------------------------
!> @author Jason Byrd QTP
!> @brief
!> forms a diagonal unit block with given exponent.
!>
!> @details
!>
! --------------------------------------------------------------------------

      subroutine form_diagonal_unit_matrix(
     * array_0, rank_0, index_values_0, size_0, extents_0, data_0,
     * array_1, rank_1, index_values_1, size_1, extents_1, data_1,
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
! input scalar 
      integer(C_INT), intent(in)::array_1
      integer(C_INT), intent(in)::rank_1
      integer(C_INT), intent(in)::index_values_1(1:rank_1)
      integer(C_INT), intent(in)::size_1
      integer(C_INT), intent(in)::extents_1(1:rank_1)
      real(C_DOUBLE), intent(in)::data_1(1:size_1)

      integer(C_INT), intent(out)::ierr


      if (rank_0 .eq. 2 .and. extents_0(1) .eq. extents_0(2)) then 

      call form_diag2(extents_0(1),data_0,data_1) 

      else
	  write(*,*) "ERROR: form_diagonal_unit_matrix 2 index only"
	  call abort_job()
      end if

      ierr = 0

      return 
      end 

      subroutine form_diag2(NA, array,block_exponent)
      
      implicit none
      integer NA,s,t
      double precision array(NA,Na), diag_sum  
      double precision block_exponent
      double precision etemp

      do s = 1,NA
	  do t = 1,s-1
	      array(s,t) = 0.d0
	      array(t,s) = 0.d0
	  enddo 

          etemp = array(s,s)

          if (block_exponent .eq. 0.5
     * 	      .and. etemp .gt. 0.d0  ) then
	      array(s,s) = dsqrt(etemp)
          else if (block_exponent .eq. -0.5
     * 	      .and. etemp .gt. 0.d0  ) then
	      array(s,s) = 1.d0/dsqrt(etemp)
          else
	      array(s,s) = 1.d0
          endif
      enddo 


      return
      end

