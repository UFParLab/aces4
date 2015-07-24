

      subroutine debug_permute_1432(
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
      real(C_DOUBLE), intent(inout)::data_0(1:size_0)

      integer(C_INT), intent(in)::array_slot_1
      integer(C_INT), intent(in)::rank_1
      integer(C_INT), intent(in)::index_values_1(1:rank_1)
      integer(C_INT), intent(in)::size_1
      integer(C_INT), intent(in)::extents_1(1:rank_1)
      real(C_DOUBLE), intent(inout)::data_1(1:size_1)

      integer(C_INT), intent(out)::ierr
      integer i, j, n, current_line, junk


      call do_perm_1432(data_0,data_1,
     *                 1,extents_0(1),
     *                 1,extents_0(2),
     *                 1,extents_0(3),
     *                 1,extents_0(4))


      return
      end

      subroutine do_perm_1432(x,y,a1,a2,i1,i2,b1,b2,j1,j2)
      implicit none

      integer a1,a2,b1,b2,i1,i2,j1,j2
      double precision x(a1:a2,i1:i2,b1:b2,j1:j2)
      double precision y(a1:a2,j1:j2,b1:b2,i1:i2)

      integer a,b,i,j
      double precision val
c
C x lhs
c y rhs
      do b = b1,b2
      do a = a1,a2
         do j = j1,j2
         do i = i1,i2
            x(a,i,b,j) = y(a,j,b,i)
         enddo
         enddo
      enddo
      enddo
c
      return
      end
