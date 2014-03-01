      subroutine print_something(ierr) BIND(C)
      use, intrinsic :: ISO_C_BINDING
      implicit none

      integer(C_INT), intent(inout):: ierr
      ierr = 0
      print * , "hello from fortran"
      end subroutine print_something
