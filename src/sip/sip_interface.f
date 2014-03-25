!> Interface to C function to get subroutine
!> Also look at $(top_srcdir)/src/setup/setup_interace.f for other examples

c      interface
c      subroutine predefined_scalar_array(aname,numDims,
c     *                                     dims,vals) bind(C)
c        use, intrinsic :: ISO_C_BINDING
c      character, dimension(*), intent(in):: aname
c      integer (C_INT), intent(out):: numDims
c      integer (C_INT), dimension(*), intent(out)::dims
c      real(C_DOUBLE), dimension(*),  intent(out):: vals
c      end subroutine predefined_scalar_array
c      end interface
c
c      interface
c      subroutine predefined_int_array(aname,numDims,
c     *                                  dims,vals) bind(C)
c        use, intrinsic :: ISO_C_BINDING
c      character, dimension(*), intent(in):: aname
c      integer (C_INT), intent(out):: numDims
c      integer (C_INT), dimension(*), intent(out)::dims
c      integer(C_INT), dimension(*),  intent(out)::vals
c      end subroutine predefined_int_array
c      end interface

      interface
      subroutine predefined_scalar_array(aname,numDims,
     *                                  dims,vals) bind(C)
      use, intrinsic :: ISO_C_BINDING
      character, dimension(*), intent(in):: aname
      integer (C_INT), intent(out)::numDims
!      integer (C_INT), intent(out)::dims(*)
      TYPE(C_PTR), intent(out)::dims
      TYPE(C_PTR), intent(out)::vals
      end subroutine predefined_scalar_array
      end interface


      interface
      subroutine predefined_int_array(aname,numDims,
     *                                  dims,vals) bind(C)
      use, intrinsic :: ISO_C_BINDING
      character, dimension(*), intent(in):: aname
      integer (C_INT), intent(out)::numDims
!      integer (C_INT), intent(out)::dims(*)
      TYPE(C_PTR), intent(out)::dims
      TYPE(C_PTR), intent(out)::vals
      end subroutine predefined_int_array
      end interface


      interface
      integer(C_INT) function int_constant(cname) bind(C)
          use, intrinsic :: ISO_C_BINDING
      character, dimension(*), intent(in):: cname
      end function int_constant
      end interface

      interface
      real(C_DOUBLE) function scalar_constant(cname) bind(C)
          use, intrinsic :: ISO_C_BINDING
      character, dimension(*), intent(in):: cname
      end function scalar_constant
      end interface

      interface
      real(C_DOUBLE) function scalar_value(cname) bind(C)
          use, intrinsic :: ISO_C_BINDING
      character, dimension(*), intent(in):: cname
      end function scalar_value
      end interface

      interface
      subroutine set_scalar_value(cname, val) bind(C)
          use, intrinsic :: ISO_C_BINDING
      character, dimension(*), intent(in):: cname
      real(C_DOUBLE), value,  intent(in):: val
      end subroutine set_scalar_value
      end interface

      interface 
      integer (C_INT) function current_line() bind(C)
        use, intrinsic :: ISO_C_BINDING
      end function current_line
      end interface 


      interface
      subroutine scratch_array(numElements, array) bind (C)
      use, intrinsic :: ISO_C_BINDING
      integer (C_INT), intent(in)::numElements
      TYPE(C_PTR), intent(out)::array
      end subroutine scratch_array
      end interface

      interface
      subroutine delete_scratch_array(array) bind (C)
      use, intrinsic :: ISO_C_BINDING
      TYPE(C_PTR), intent(out)::array
      end subroutine delete_scratch_array
      end interface

      interface
      subroutine integer_scratch_array(numElements, array) bind (C)
      use, intrinsic :: ISO_C_BINDING
      integer (C_INT), intent(in)::numElements
      TYPE(C_PTR), intent(out)::array
      end subroutine scratch_array
      end interface

      interface
      subroutine delete_integer_scratch_array(array) bind (C)
      use, intrinsic :: ISO_C_BINDING
      TYPE(C_PTR), intent(out)::array
      end subroutine delete_scratch_array
      end interface

