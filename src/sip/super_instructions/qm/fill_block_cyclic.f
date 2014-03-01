      !two arg super instruction.  The first is a block, the second
      ! a scalar.  Fills the block linearly with x, x+1.0, x_2.0 till
      ! 20. etc. starting with the scalalr.
      ! Based off fill_block_sequential.


!          (arg_1, nindex_1, type_1, bval_1, eval_1, bdim_1, edim_1,
!           arg_2, nindex_2, type_2, bval_2, eval_2, bdim_2, edim_2,
!           arg_last, nindex_last, type_last, bval_last, eval_last, bdim_last, edim_last)
!    REAL(8) arg_x                                   !pointer to the data representing argument#x
!    INTEGER nindex_x                                !dimensionality of argument#x (0 for scalars)
!    INTEGER type_x(1:nindex_x)                      !array of index types
!    INTEGER bval_x(1:nindex_x),eval_x(1:nindex_x)   !lower and upper bounds for each dimension of the argument#x
!    INTEGER bdim_x(1:nindex_x),edim_x(1:nindex_x)   !beginning and ending dimensions of each index of the array  (see below)
!    ...
!    Code
!    ...
!    return

!This is a super instruction for testing.  It simply fills the given
! block with double values starting at the given one and incrementing
! each time

      subroutine fill_block_cyclic(
     c array_slot_0, rank_0, index_values_0, size_0, extents_0, data_0,
     c array_slot_1, rank_1, index_values_1, size_1, extents_1, data_1,
     c  ierr) BIND(C)
      use, intrinsic :: ISO_C_BINDING
      implicit none
      integer(C_INT), intent(in)::array_slot_0
      integer(C_INT), intent(in)::rank_0
      integer(C_INT), intent(in)::index_values_0(1:rank_0)
      integer(C_INT), intent(in)::size_0
      integer(C_INT), intent(in)::extents_0(1:rank_0)
      real(C_DOUBLE), intent(out)::data_0(1:size_0)

      integer(C_INT), intent(in)::array_slot_1
      integer(C_INT), intent(in)::rank_1
      integer(C_INT), intent(in)::index_values_1(1:rank_1)
      integer(C_INT), intent(in)::size_1
      integer(C_INT), intent(in)::extents_1(1:rank_1)
      real(C_DOUBLE), intent(in)::data_1(1:size_0)

      integer(C_INT), intent(out)::ierr

      real val
      integer i

!second argument should be a scalar
      if (rank_1.ne.0) then
      ierr = rank_1
      return
      endif


      val = data_1(1)

      do i = 1,size_0
        data_0(i) = val
        if (val .eq. 20) then
            val = 0.0
        endif
        val = val + 1.0
      enddo

      end subroutine fill_block_cyclic
