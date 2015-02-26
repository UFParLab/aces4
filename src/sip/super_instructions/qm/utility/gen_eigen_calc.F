c--------------------------------------------------------------------------
!> special gen_eigen_calc uuuu
!>   Calculates the generalized eigenvalues and eigenvectors of a 2-d 
!>   non-symmetric square matrix.
!>
!>   execute eig Array1 Array2 Array3 Array4
!>
!>   where Array1 is the input matrix that is to be diagonalized.
!>         Array2 returns the matrix whose columns are the left eigenvectors of Array1.
!>         Array3 returns the matrix whose columns are the right eigenvectors of Array1.
!>         Array4 returns the eigenvalues.
!>
!>   All must be "static" arrays.
c--------------------------------------------------------------------------

      subroutine gen_eigen_calc(
     c array_slot_0, rank_0, index_values_0, size_0, extents_0, data_0,
     c array_slot_1, rank_1, index_values_1, size_1, extents_1, data_1,
     c array_slot_2, rank_2, index_values_2, size_2, extents_2, data_2,
     c array_slot_3, rank_3, index_values_3, size_3, extents_3, data_3,
     c  ierr) BIND(C)
      use, intrinsic :: ISO_C_BINDING
      implicit none

      include 'sip_interface.f'
      include 'imemory.h'
            
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

      integer(C_INT), intent(in)::array_slot_2
      integer(C_INT), intent(in)::rank_2
      integer(C_INT), intent(in)::index_values_2(1:rank_2)
      integer(C_INT), intent(in)::size_2
      integer(C_INT), intent(in)::extents_2(1:rank_2)
      real(C_DOUBLE), intent(inout)::data_2(1:size_2)

      integer(C_INT), intent(in)::array_slot_3
      integer(C_INT), intent(in)::rank_3
      integer(C_INT), intent(in)::index_values_3(1:rank_3)
      integer(C_INT), intent(in)::size_3
      integer(C_INT), intent(in)::extents_3(1:rank_3)
      real(C_DOUBLE), intent(inout)::data_3(1:size_3)

      TYPE(C_PTR) :: cscr1, cscr2, cscr3, cscr4, cscr5
      integer(c_int) :: nscr1, nscr2, nscr3, nscr4, nscr5
      real(c_double), pointer :: scr1(:)
      real(c_double), pointer :: scr2(:)
      real(c_double), pointer :: scr3(:)
      real(c_double), pointer :: scr4(:)
      real(c_double), pointer :: scr5(:)


      integer(C_INT), intent(out)::ierr
      integer i, j, n, junk

      n = extents_0(1)
C      print *, "n=",n

!      B(n,n)
      nscr1 = n*n
!      alphar(n)
      nscr2 = n
!      alphai(n)
      nscr3 = n
!      beta(n)
      nscr4 = n
!      lwork(8n) (int)
      nscr5 = 10*n
C      print *, "nw=",nscr5

C      print *, "entering gen_eig_calc"

      call scratch_array(nscr1,cscr1)
      call scratch_array(nscr2,cscr2)
      call scratch_array(nscr3,cscr3)
      call scratch_array(nscr4,cscr4)
      call scratch_array(nscr5,cscr5)

      call c_f_pointer(cscr1, scr1, [nscr1])
      call c_f_pointer(cscr2, scr2, [nscr2])
      call c_f_pointer(cscr3, scr3, [nscr3])
      call c_f_pointer(cscr4, scr4, [nscr4])
      call c_f_pointer(cscr5, scr5, [nscr5])
      
      call dggev_wrapper(data_0,data_1,data_2,data_3,n,
     *                   scr1,scr2,scr3,scr4,scr5,nscr5,ierr)

      call delete_scratch_array(cscr1)
      call delete_scratch_array(cscr2)
      call delete_scratch_array(cscr3)
      call delete_scratch_array(cscr4)
      call delete_scratch_array(cscr5)

      if (ierr .ne. 0) call abort_job()

      return
      end


      subroutine dggev_wrapper(Amat,Lvec,Rvec,eigval,nn,
     *           Bmat,alphar,alphai,beta,work,nw,info)
      use, intrinsic :: ISO_C_BINDING 
      implicit none 

      integer info
      integer nn, nw
      double precision Amat(nn,nn)
      double precision Lvec(nn,nn)
      double precision Rvec(nn,nn)
      double precision eigval(nn)

      double precision Bmat(nn,nn)
      double precision alphar(nn), alphai(nn), beta(nn)
      double precision work(nw)

      double precision small, zero, Lval, Rval
      double precision norm, tmp, val1, val2
      integer i,j,k

*  =====================================================================
*  DGGEV computes for a pair of N-by-N real nonsymmetric matrices (A,B)
*  the generalized eigenvalues, and optionally, the left and/or right
*  generalized eigenvectors.
*
*  A generalized eigenvalue for a pair of matrices (A,B) is a scalar
*  lambda or a ratio alpha/beta = lambda, such that A - lambda*B is
*  singular. It is usually represented as the pair (alpha,beta), as
*  there is a reasonable interpretation for beta=0, and even for both
*  being zero.
*
*  The right eigenvector v(j) corresponding to the eigenvalue lambda(j)
*  of (A,B) satisfies
*
*                   A * v(j) = lambda(j) * B * v(j).
*
*  The left eigenvector u(j) corresponding to the eigenvalue lambda(j)
*  of (A,B) satisfies
*
*                   u(j)**H * A  = lambda(j) * u(j)**H * B .
*
*  where u(j)**H is the conjugate-transpose of u(j).
*  =====================================================================
*  ALPHAR  (output) DOUBLE PRECISION array, dimension (N)
*  ALPHAI  (output) DOUBLE PRECISION array, dimension (N)
*  BETA    (output) DOUBLE PRECISION array, dimension (N)
*          On exit, (ALPHAR(j) + ALPHAI(j)*i)/BETA(j), j=1,...,N, will
*          be the generalized eigenvalues.  If ALPHAI(j) is zero, then
*          the j-th eigenvalue is real; if positive, then the j-th and
*          (j+1)-st eigenvalues are a complex conjugate pair, with
*          ALPHAI(j+1) negative.
*
*          Note: the quotients ALPHAR(j)/BETA(j) and ALPHAI(j)/BETA(j)
*          may easily over- or underflow, and BETA(j) may even be zero.
*          Thus, the user should avoid naively computing the ratio
*          alpha/beta.  However, ALPHAR and ALPHAI will be always less
*          than and usually comparable with norm(A) in magnitude, and
*          BETA always less than and usually comparable with norm(B).
*  =====================================================================

C hard clear everything but amat
      do i = 1,nn
          alphar(i) = 0.d0
          alphai(i) = 0.d0
          beta(i) = 0.d0
          do j = 1,nn
              Lvec(i,j) = 0.d0
              Rvec(i,j) = 0.d0
              Bmat(i,j) = 0.d0
          enddo
      enddo
      do i = 1,nn
          Bmat(i,i) = 1.d0
          eigval(i) = 0.d0
      enddo 

C      print *, 'V'
C      print *, 'V'
C      print *, "nn=",nn
C      print *, "Amat"
C      do i = 1,nn
C          print *, (Amat(i,j),j=1,nn)
C      enddo
C      print *, "Bmat"
C      do i = 1,nn
C          print *, (Amat(i,j),j=1,nn)
C      enddo
C      print *, nw

C standard call to dggev
      call DGGEV('V','V', nn,Amat,nn, Bmat, nn, 
     *            alphar, alphai, beta,
     *            Lvec,nn,Rvec,nn, work, nw, info )

C place eigenvalues
      small = 1.d-8
      do i = 1,nn
          if (abs(beta(i)) .gt. small) then
              if (alphai(i) .gt. small) then
		  write(*,*) "large complex component from DGGEV", alphai(i)
		  info = 1
		  return
              endif

              eigval(i) = alphar(i)/beta(i)
          else
              info = 1
              return
          endif
      enddo
C
C sort eigenvalues
      do i = 1,nn
          work(i) = real(i)
	  if ( abs(eigval(i)) .lt. 1.d-12 ) then
	      eigval(i) = 999.d0
	  endif
      enddo
      i = 1
      j = nn 
      call quicksort(eigval,work,i,j,nn)
c      print *, "eig"
c      print *, (eigval(i),i=1,nn)

C use Bmat scratch storage as swap space
      do i = 1,nn
          do j = 1,nn
              Bmat(i,j) = Lvec(i,j)
          enddo
      enddo
      do i = 1,nn
          do j = 1,nn
              Lvec(i,j) = Bmat(i,int(work(j)))
          enddo
      enddo
      do i = 1,nn
          do j = 1,nn
              Bmat(i,j) = rvec(i,j)
          enddo
      enddo
      do i = 1,nn
          do j = 1,nn
              Rvec(i,j) = Bmat(i,int(work(j)))
          enddo
      enddo

      do j = 1,nn
	  val1 = 0.0
	  do i = 1,nn
	      val1 = val1 + rvec(i,j)*rvec(i,j)
	  enddo
	  val2 = dsqrt(val1)
	  do i = 1,nn
	      rvec(i,j) = rvec(i,j)/val2
	  enddo
      enddo

      do i = 1,nn
	  if ( abs(eigval(i)) .eq. 999.d0 ) then
	      eigval(i) = 0.d0
	  endif
      enddo

      return
      end

      RECURSIVE SUBROUTINE quicksort(x,y,left,right,n)
      implicit none
      
      INTEGER, intent(IN) :: n
      INTEGER, intent(INOUT) :: left, right
      REAL(kind=8), intent(INOUT) :: x(n), y(n)
      INTEGER :: l_hold, r_hold, newleft, newright
      REAL(kind=8) :: pivot, pivoty
      
      l_hold = left
      r_hold = right
      pivot = x(left)
      pivoty = y(left)
      
      DO WHILE (left < right)
      DO WHILE ((x(right) >= pivot).AND.(left < right))
      right = right - 1
      END DO
      IF (left /= right) THEN
      x(left) = x(right)
      y(left) = y(right)
      left = left + 1
      ENDIF
      DO WHILE ((x(left) <= pivot).AND.(left < right))
      left = left + 1
      END DO
      IF (left /= right) THEN
      x(right) = x(left)
      y(right) = y(left)
      right = right - 1
      ENDIF
      END DO
      
      x(left) = pivot
      y(left) = pivoty
      pivot = left
      newright = left - 1
      left = l_hold
      right = r_hold
      if (left < pivot) CALL quicksort(x, y, left, newright,n)
      newleft = pivot + 1
      if (right > pivot) CALL quicksort(x, y, newleft, right,n)
      
      RETURN
      END SUBROUTINE quicksort
