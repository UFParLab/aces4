

! --------------------------------------------------------------------------
!> @author Jason N Byrd QTP.
!> @brief
!> prints the elements of a block with indices
!>
!> @details
!>  define: special print_block_and_index r
!>
!>  Usage: execute print_block_and_index T1
! --------------------------------------------------------------------------

      subroutine print_block_and_index(
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

c mosegments
      TYPE(C_PTR) :: n_ao_segs, aosegs
      integer, pointer :: pn_ao_segs(:), paosegs(:)
      integer(C_INT) offset_1, offset_2, offset_3, offset_4
      integer(C_INT) offset_5, offset_6
      integer(C_INT) start_1, start_2, start_3, start_4, start_5,start_6
      integer(C_INT) i, i1, i2, i3, i4, one

      ierr = 0

c Get the mo segment ranges
      call predefined_int_array("moa_seg_ranges"//C_NULL_CHAR, one,
     *                           n_ao_segs, aosegs)
      call c_f_pointer(n_ao_segs,pn_ao_segs,[1])
      call c_f_pointer(aosegs,paosegs,pn_ao_segs)

c Find the offset index ranges
      offset_1 = 0
      offset_2 = 0
      offset_3 = 0
      offset_4 = 0
      offset_5 = 0

      if (rank_0 .eq. 4) then

      if (index_values_0(1).gt.1) then
          do i = 1, index_values_0(1) - 1
             offset_1 = offset_1 + paosegs(i)
          enddo
      endif
      if (index_values_0(2).gt.1) then
          do i = 1, index_values_0(2) - 1
             offset_2 = offset_2 + paosegs(i)
          enddo
      endif
      if (index_values_0(3).gt.1) then
          do i = 1, index_values_0(3) - 1
             offset_3 = offset_3 + paosegs(i)
          enddo
      endif
      if (index_values_0(4).gt.1) then
          do i = 1, index_values_0(4) - 1
             offset_4 = offset_4 + paosegs(i)
          enddo
      endif

      call do_print_4block(offset_1+1, extents_0(1)+offset_1,
     *                     offset_2+1, extents_0(2)+offset_2,
     *                     offset_3+1, extents_0(3)+offset_3,
     *                     offset_4+1, extents_0(4)+offset_4,
     *                     data_0)
      else
	  write(*,*) "bad call to invert_diagonal_asym"
	  call abort_job()
      endif

      if (ierr .eq. 1) then
	  call abort_job()
      endif

      return
      end

      subroutine do_print_4block(a1,a2,b1,b2,c1,c2,d1,d2, 
     *                          array1)
      implicit none
      integer ierr
      integer a, a1, a2
      integer b, b1, b2
      integer c, c1, c2 
      integer d, d1, d2 
      double precision array1(a1:a2,b1:b2,c1:c2,d1:d2) 
      double precision val, div


      do d = d1,d2
      do c = c1,c2
      do b = b1,b2
      do a = a1,a2
	  write(6,100) a,b,c,d,array1(a,b,c,d)
      enddo 
      enddo 
      enddo 
      enddo 
  100 format(4I3,1F24.14)

      return
      end
