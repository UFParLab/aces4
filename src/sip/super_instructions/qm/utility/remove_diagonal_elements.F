
! --------------------------------------------------------------------------
!> @author Jason N Byrd QTP.
!> @brief
!> The diagonal, elements of a two index array will be zeroed. 
!
!> @details
!> sialx definition: special remove_diagonal u
!>
!> sialx usage: execute remove_diagonal Tai[a,i]
! --------------------------------------------------------------------------

      subroutine remove_diagonal(
     c array_slot_0, rank_0, index_values_0, size_0, extents_0, data_0,
     c  ierr) BIND(C)
      use, intrinsic :: ISO_C_BINDING
      implicit none
      include 'sip_interface.f'
      integer(C_INT), intent(in)::array_slot_0
      integer(C_INT), intent(in)::rank_0
      integer(C_INT), intent(in)::index_values_0(1:rank_0)
      integer(C_INT), intent(in)::size_0
      integer(C_INT), intent(in)::extents_0(1:rank_0)
      real(C_DOUBLE), intent(inout)::data_0(1:size_0)

      integer(C_INT), intent(out)::ierr
c mosegments
      TYPE(C_PTR) :: n_ao_segs, aosegs
      integer, pointer :: pn_ao_segs(:), paosegs(:)
      integer(C_INT) offset_1, offset_2, offset_3, offset_4
      integer(C_INT) offset_5, offset_6
      integer(C_INT) start_1, start_2, start_3, start_4, start_5,start_6
      integer(C_INT) i, i1, i2, i3, i4, one

      ierr = 0

      if (rank_0 .ne. 2) then
	  write(*,*) "ERROR in remove_diagonal_elements, wrong rank", 
     *               rank_0
	  call abort_job()
      else if (index_values_0(1) .ne. index_values_0(2)) then
	  write(*,*) "ERROR in remove_diagonal_elements, i != j", 
     *               index_values_0(1),index_values_0(2)
	  call abort_job()
      endif

      call do_removediag(data_0,1,extents_0(1),1,extents_0(2))

      return
      end

      subroutine do_removediag(Z,a1,a2,b1,b2)
      implicit none
      integer a1, a2, b1, b2, i
      double precision Z(a1:a2,b1:b2)

      do i = a1,a2
          Z(i,i) = 0.d0
      enddo

      return
      end
