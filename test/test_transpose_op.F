

      integer function test_transpose_op (actual_b) bind(C)
      use, intrinsic :: ISO_C_BINDING

      implicit none
      real(C_DOUBLE), intent(out)::actual_b(8,8,8)
      real(C_DOUBLE) :: a(8,8,8), b(8,8,8), counter
      integer(C_INT) :: i, j,k, flag
      CHARACTER(LEN=30)  :: Format

      interface
      subroutine fill_a(a)
      use, intrinsic :: ISO_C_BINDING
      real(C_DOUBLE), dimension(*) :: a
      end subroutine fill_a
      end interface

      flag = 1      ! Flag to denote equality between ref & actual

c     Set a
      call fill_a(a)

c     Transpose a to b
      do i= 1, 8
        do j = 1, 8
            do k = 1, 8
                b(j, k, i) = a(i, j, k)
            enddo
        enddo
      enddo

      Format='( 8F6.1 )'

c     Print actual & ref
      print *, 'ref b'
      do i= 1, 8
        do j = 1, 8
           write (6, Format) (b(i,j,k), k = 1, 8)
        enddo
      enddo

      print *, 'actual b'
      do i= 1, 8
        do j = 1, 8
           write (6, Format) (actual_b(i,j,k), k = 1, 8)
        enddo
      enddo

c     Do comparison
      do i= 1, 8
        do j = 1, 8
            do k = 1, 8
                if (actual_b(i,j,k) .ne. b(i,j,k)) flag = 0
            enddo
        enddo
      enddo

      test_transpose_op = flag

      return
      end



      subroutine fill_a(a)
      use, intrinsic :: ISO_C_BINDING
      implicit none
      real(C_DOUBLE), dimension(*) :: a
      real(C_DOUBLE) :: counter
      integer(C_INT) :: i
      counter = 53

      do i= 1, 8*8*8
            a(i) = counter
            counter = counter + 1
      enddo

      return
      end
