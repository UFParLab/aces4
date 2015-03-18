c--------------------------------------------------------------------------
!> special eigen_calc uu
!>   Calculates the eigenvalues and eigenvectors of a 2-d square matrix.
!>   The matrix is also diagonalized on output and inverted. 
!>
!>   The eigenvalues are stored in a common block.  
!>   The instruction is executed as follows:
!>   
!>   execute eig Array1 Array2
!>
!>   where Array1 is the input matrix that is to be diagonalized.
!>         Array2 is the matrix whose columns are the eigenvectors of Array1.
!>
!>   Both Array1 and Array2 must be "static" arrays.
c--------------------------------------------------------------------------

      subroutine eigen_calc(
     c array_slot_0, rank_0, index_values_0, size_0, extents_0, data_0,
     c array_slot_1, rank_1, index_values_1, size_1, extents_1, data_1,
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

      TYPE(C_PTR) :: cscr1, cscr2, cscr3
      integer(c_int) :: nscr1, nscr2, nscr3
      real(c_double), pointer :: scr1(:)
      real(c_double), pointer :: scr2(:)
      real(c_double), pointer :: scr3(:)

      integer(C_INT), intent(out)::ierr
      integer i, j, n

      ierr = 0
      if (rank_0 .ne. 2) then
         ierr = ierr + 1
         print *,'Error in eigen_calc, Number of array',
     *           ' indices must be 2, actual number is ',rank_0
      endif

      if (rank_1 .ne. 2) then
         ierr = ierr + 1
         print *,'Error in eigen_calc, Number of array',
     *           ' indices must be 2, actual number is ',rank_0
      endif

      if (ierr .ne. 0) call abort_job()

      n = extents_0(1)
      nscr1 = 10*n
      nscr2 = n*n
      nscr3 = n

      call scratch_array(nscr1,cscr1)
      call scratch_array(nscr2,cscr2)
      call scratch_array(nscr3,cscr3)

      call c_f_pointer(cscr1, scr1, [nscr1])
      call c_f_pointer(cscr2, scr2, [nscr2])
      call c_f_pointer(cscr3, scr3, [nscr3])

      call dsyev_wrapper(data_0,data_1,n,scr1,nscr1,scr2,scr3,ierr)
    
      call delete_scratch_array(cscr1)
      call delete_scratch_array(cscr2)
      call delete_scratch_array(cscr3)

      if (ierr .ne. 0) call abort_job()

      return
      end

      subroutine dsyev_wrapper(Amat,Bvec,n,work,nw,sw,eigval,ierr)
      use, intrinsic :: ISO_C_BINDING 
      implicit none 

      integer n,nw,nsw
      double precision Amat(n,n)
      double precision Bvec(n,n)
      double precision work(nw)
      double precision sw(n,n)
      double precision eigval(n)

      integer i, j, k, index, ierr, lda
      double precision dtmp

      double precision tolerance
      parameter (tolerance = 1.0d-5)

C checking
      if (n.le.1) then
	  ierr = 1
	  return
      endif

      dtmp = 0.d0
      do j = 2, n
         do i = 1, j-1
            dtmp = dtmp + dabs(Amat(j,i)-Amat(i,j))
         end do
      end do
      if (dtmp.gt.1.d-8) then
         print *, '@EIG: WARNING - the matrix is not symmetric'
         print *, '      sum of differences is ',dtmp
      endif

*  DSYEV computes all eigenvalues and, optionally, eigenvectors of a
*  real symmetric matrix A.
*
*  Arguments
*  =========
*
*  JOBZ    (input) CHARACTER*1
*          = 'N':  Compute eigenvalues only;
*          = 'V':  Compute eigenvalues and eigenvectors.
*
*  UPLO    (input) CHARACTER*1
*          = 'U':  Upper triangle of A is stored;
*          = 'L':  Lower triangle of A is stored.
*
*  N       (input) INTEGER
*          The order of the matrix A.  N >= 0.
*
*  A       (input/output) DOUBLE PRECISION array, dimension (LDA, N)
*          On entry, the symmetric matrix A.  If UPLO = 'U', the
*          leading N-by-N upper triangular part of A contains the
*          upper triangular part of the matrix A.  If UPLO = 'L',
*          the leading N-by-N lower triangular part of A contains
*          the lower triangular part of the matrix A.
*          On exit, if JOBZ = 'V', then if INFO = 0, A contains the
*          orthonormal eigenvectors of the matrix A.
*          If JOBZ = 'N', then on exit the lower triangle (if UPLO='L')
*          or the upper triangle (if UPLO='U') of A, including the
*          diagonal, is destroyed.
*
*  LDA     (input) INTEGER
*          The leading dimension of the array A.  LDA >= max(1,N).
*
*  W       (output) DOUBLE PRECISION array, dimension (N)
*          If INFO = 0, the eigenvalues in ascending order.
*
*  WORK    (workspace/output) DOUBLE PRECISION array, dimension (MAX(1,LWORK))
*          On exit, if INFO = 0, WORK(1) returns the optimal LWORK.
*
*  LWORK   (input) INTEGER
*          The length of the array WORK.  LWORK >= max(1,3*N-1).
*          For optimal efficiency, LWORK >= (NB+2)*N,
*          where NB is the blocksize for DSYTRD returned by ILAENV.
*
*          If LWORK = -1, then a workspace query is assumed; the routine
*          only calculates the optimal size of the WORK array, returns
*          this value as the first entry of the WORK array, and no error
*          message related to LWORK is issued by XERBLA.
*
*  INFO    (output) INTEGER
*          = 0:  successful exit
*          < 0:  if INFO = -i, the i-th argument had an illegal value
*          > 0:  if INFO = i, the algorithm failed to converge; i
*                off-diagonal elements of an intermediate tridiagonal
*                form did not converge to zero.

C zero eigval
      do i = 1,n
	  eigval(i) = 0.d0
      enddo
C copy Amat to Bvec
      do i = 1,n
      do j = 1,n
	  Bvec(i,j) = Amat(i,j)
      enddo
      enddo

      call DSYEV('V','L', n,Bvec,n, eigval, work,nw, ierr)

      if (ierr.ne.0) then
         write(*,*) 'dsyev error. Error code: ',ierr
         return
      end if

c   make first significant element of each eigenvector positive
      do j = 1, n
         index = 0
         do i = 1, n
            if (index.eq.0) then
               if (abs(Bvec(i,j)).gt.tolerance) index = i
            end if
         end do
         if (index.ne.0) then
            if (Bvec(index,j).lt.0.0d0) then
               do i = 1, n
                  Bvec(i,j) = -Bvec(i,j)
               end do
            end if
         end if
      end do

C sort eigenvalues
      do i = 1,n      
	  work(i) = real(i)
      enddo
      i = 1
      j = n
      call quicksort(eigval,work,i,j,n)

      do i = 1,n
	  do j = 1,n
	      sw(i,j) = Bvec(i,j)
	  enddo
      enddo
      do i = 1,n
	  do j = 1,n
	      Bvec(i,j) = sw(i,int(work(j)))
	  enddo
      enddo

      do i = 1,n
      do j = 1,n
	  Amat(i,j) = 0.d0
      enddo
      enddo
      do i = 1,n
	  Amat(i,i) = eigval(i)
      enddo

C
C return diagonal Amat with eigenvalue elements
C return Bvec with eigenvectors
C
      return
      end

