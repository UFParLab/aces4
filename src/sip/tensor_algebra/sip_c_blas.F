
      subroutine sip_blas_daxpy(n, alpha, x, incx, y, incy) BIND(C)
      use, intrinsic :: ISO_C_BINDING
      implicit none

      integer(C_INT),   intent(in)  ::n
      real(C_DOUBLE),   intent(in)  ::alpha
      real(C_DOUBLE),   intent(out) ::x(1:n)
      integer(C_INT),   intent(in)  ::incx
      real(C_DOUBLE),   intent(out) ::y(1:n)
      integer(C_INT),   intent(in)  ::incy

      call daxpy(n, alpha, x, incx, y, incy)

      return
      end


      subroutine sip_blas_dcopy(n, x, incx, y, incy) BIND(C)
      use, intrinsic :: ISO_C_BINDING
      implicit none

      integer(C_INT),   intent(in)  ::n
      real(C_DOUBLE),   intent(out) ::x(1:n)
      integer(C_INT),   intent(in)  ::incx
      real(C_DOUBLE),   intent(out) ::y(1:n)
      integer(C_INT),   intent(in)  ::incy

      call dcopy(n, x, incx, y, incy)

      return
      end



      subroutine sip_blas_dscal(n, alpha, x, incx) BIND(C)
      use, intrinsic :: ISO_C_BINDING
      implicit none

      integer(C_INT),   intent(in)  ::n
      real(C_DOUBLE),   intent(in)  ::alpha
      real(C_DOUBLE),   intent(out) ::x(1:n)
      integer(C_INT),   intent(in)  ::incx

      call dscal(n, alpha, x, incx)

      return
      end
