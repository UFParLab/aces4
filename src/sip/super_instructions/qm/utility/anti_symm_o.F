! --------------------------------------------------------------------------
!> @author Jason N Byrd QTP.
!> @brief
!> The diagonal, occupied, elements of a four index array are extracted using storage pattern VOVO
!
!> @details
!> sialx definition: special anti_symm_o u
!>
!> sialx usage: execute anti_symm_o Taiai[a,i,a1,i1]
! --------------------------------------------------------------------------

      subroutine anti_symm_o(
     c array_slot_0, rank_0, index_values_0, size_0, extents_0, data_0,
     c  ierr) BIND(C)
      use, intrinsic :: ISO_C_BINDING
      implicit none
      integer(C_INT), intent(in)::array_slot_0
      integer(C_INT), intent(in)::rank_0
      integer(C_INT), intent(in)::index_values_0(1:rank_0)
      integer(C_INT), intent(in)::size_0
      integer(C_INT), intent(in)::extents_0(1:rank_0)
      real(C_DOUBLE), intent(inout)::data_0(1:size_0)

      integer(C_INT), intent(out)::ierr
      integer i, j, n, current_line, junk
      
      if ( rank_0 .eq. 4) then
      call dosymmforce4o(data_0,
     *                 1,extents_0(1),
     *                 1,extents_0(2),
     *                 1,extents_0(3),
     *                 1,extents_0(4))
      else
	  print *, "anti_symm_o requires rank 4 temp block"
	  call abort_job()
      endif

      return
      end

      subroutine dosymmforce4o(x,a1,a2,i1,i2,b1,b2,j1,j2)
      implicit none

      integer a1,a2,b1,b2,i1,i2,j1,j2
      double precision x(a1:a2,i1:i2,b1:b2,j1:j2)

      integer a,b,i,j
      double precision val
c
      do b = b1,b2
      do a = a1,a2
         do j = j1,j2
         do i = i1,i2
            if (i .lt. j)then
               val = x(a,i,b,j)
               x(a,j,b,i)= val*(-1.0d0)
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
