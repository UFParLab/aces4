      subroutine dadd(a, b, c) BIND(C)
      use, intrinsic :: ISO_C_BINDING
      implicit none
      real (C_DOUBLE), value, intent(in) :: a
      real (C_DOUBLE), value, intent(in) :: b
      real (C_DOUBLE), intent(out) :: c

      c = a + b
      end subroutine dadd

      subroutine dsub(a, b, c) BIND(C)
      use, intrinsic :: ISO_C_BINDING
      implicit none
      real (C_DOUBLE), value, intent(in) :: a
      real (C_DOUBLE), value, intent(in) :: b
      real (C_DOUBLE), intent(out) :: c
      double precision y, z
      y = a
      z = b
      c = y-z
      end subroutine dsub

!      subroutine print_something() BIND(C)
!      use, intrinsic :: ISO_C_BINDING
!      implicit none
!      print * , "hello from fortran"
!      end subroutine print_something
