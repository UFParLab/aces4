! --------------------------------------------------------------------------
!> @author Jason N Byrd QTP.
!> @brief
!> The diagonal, virtual, elements of a four index array are extracted using storage pattern VOVO
!
!> @details
!> sialx definition: special anti_symm_v u
!
!> sialx usage: execute anti_symm_v Taiai[a,i,a1,i1]
! --------------------------------------------------------------------------      
      subroutine anti_symm_v(
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

c Get the mo segment ranges
      call predefined_int_array("moa_seg_ranges"//C_NULL_CHAR, one,
     *                           n_ao_segs, aosegs)
      call c_f_pointer(n_ao_segs,pn_ao_segs,[1])
      call c_f_pointer(aosegs,paosegs,pn_ao_segs)

      offset_1 = 0
      offset_2 = 0
      offset_3 = 0
      offset_4 = 0
      if ( rank_0 .eq. 4) then

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

      call dosymmforce4v(data_0,
     *                 offset_1+1,offset_1+extents_0(1),
     *                 offset_2+1,offset_2+extents_0(2),
     *                 offset_3+1,offset_3+extents_0(3),
     *                 offset_4+1,offset_4+extents_0(4))
      else
	  print *, "anti_symm_v requires rank 4 temp block"
	  call abort_job()
      endif

      return
      end

      subroutine dosymmforce4v(x,a1,a2,i1,i2,b1,b2,j1,j2)
      implicit none

      integer a1,a2,b1,b2,i1,i2,j1,j2
      double precision x(a1:a2,i1:i2,b1:b2,j1:j2)

      integer a,b,i,j
      double precision val

c
      if (i1 .eq. 1 .and. i2 .eq. 1) then
         do a = a1,a2
         do b = b1,b2
            do i = i1,i2
            do j = j1,j2
               if (a .lt. b)then
                  val = x(a,i,b,j)
                  x(b,i,a,j)= val*(-1.0d0)
               endif
               if (a .eq. b)then
                  val = 0.0
                  x(b,i,a,j)= val*(-1.0d0)
               endif
            enddo
            enddo
         enddo
         enddo
         return
      endif
c
      do b = b1,b2
      do a = a1,a2
         do j = j1,j2
         do i = i1,i2
            if (a .lt. b)then
               val = x(a,i,b,j)
               x(b,i,a,j)= val*(-1.0d0)
            endif
         enddo
         enddo
      enddo
      enddo
c
      val = 0.0d0
      do b = b1,b2
      do a = a1,a2
         do j = j1,j2
         do i = i1,i2
         if(i .eq. j .or. a .eq. b)then
               x(a,i,b,j)= val
         endif
         enddo
         enddo
      enddo
      enddo
c
      return
      end
