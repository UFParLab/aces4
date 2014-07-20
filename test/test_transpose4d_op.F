

      integer function test_transpose4d_op (actual_b) bind(C)
      use, intrinsic :: ISO_C_BINDING

      implicit none
      INTEGER, PARAMETER ::  v = 5
      INTEGER, PARAMETER ::  o = 1
      real(C_DOUBLE), intent(out)::actual_b(v,o,v,o)
      real(C_DOUBLE) :: a(v,o,v,o), b(v,o,v,o), counter
      integer(C_INT) :: i, j, k, l, flag
      CHARACTER(LEN=30)  :: Format

      interface
      subroutine fill_a4d(a)
      use, intrinsic :: ISO_C_BINDING
      real(C_DOUBLE), dimension(*) :: a
      end subroutine fill_a4d
      end interface

      flag = 1      ! Flag to denote equality between ref & actual

c     Set a
      call fill_a4d(a)

c     Transpose a to b
      do i= 1, v
        do j = 1, o
            do k = 1, v
                do l = 1, o
                    b(k, j, i, l) = a(i, j, k, l)
c                    b(i, j, k, l) = a(i, j, k, l) + 0
                enddo
            enddo
        enddo
      enddo

      Format='( 8F6.1 )'

c     Print a
      print *, 'actual a'
      do i= 1, v
        do j = 1, o
          do k = 1, v
            write (6, Format) (a(i, j, k, l), l = 1, o)
          enddo
        enddo
      enddo

c     Print actual & ref
      print *, 'ref b'
      do i= 1, v
        do j = 1, o
          do k = 1, v
            write (6, Format) (b(i, j, k, l), l = 1, o)
          enddo
        enddo
      enddo

      print *, 'actual b'
      do i= 1, v
        do j = 1, o
          do k = 1, v
            write (6, Format) (actual_b(i, j, k, l), l = 1, o)
          enddo
        enddo
      enddo

c     Do comparison
      do i= 1, v
        do j = 1, o
            do k = 1, v
                do l = 1, o
                    if (actual_b(i,j,k,l) .ne. b(i,j,k,l)) flag = 0
                enddo
            enddo
        enddo
      enddo

      test_transpose4d_op = flag

      return
      end



      subroutine fill_a4d(a)
      use, intrinsic :: ISO_C_BINDING
      implicit none
      real(C_DOUBLE), dimension(*) :: a
      real(C_DOUBLE) :: counter
      integer(C_INT) :: i
      INTEGER, PARAMETER ::  v = 1
      INTEGER, PARAMETER ::  o = 5
      counter = 53

      do i= 1, v*o*v*o
            a(i) = counter
            counter = counter + 1
      enddo

      return
      end
