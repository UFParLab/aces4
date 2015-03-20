c--------------------------------------------------------------------------
!> special  eigen_calc_sqr_inv
!>   Calculates the eigenvalues and eigenvectors of a 2-d square matrix.
!>   The matrix is also diagonalized on output and inverted. 
!>
!>   The eigenvalues are stored in a common block.  
!>   The instruction is executed as follows:
!>   
!>   execute eig Array1 Array2 scalar
!>
!>   where Array1 is the input matrix that is to be diagonalized.
!>         Array2 is the matrix whose columns are the eigenvectors of Array1.
!>         scalar is the number of droped elements
!>
!>   Both Array1 and Array2 must be "static" arrays.
c--------------------------------------------------------------------------

      subroutine eigen_calc_sqr_inv(
     c array_slot_0, rank_0, index_values_0, size_0, extents_0, data_0,
     c array_slot_1, rank_1, index_values_1, size_1, extents_1, data_1,
     c array_slot_2, rank_2, index_values_2, size_2, extents_2, data_2,
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

      TYPE(C_PTR) :: cscr1, cscr2, cscr3
      integer(c_int) :: nscr1, nscr2, nscr3
      real(c_double), pointer :: scr1(:)
      real(c_double), pointer :: scr2(:)
      real(c_double), pointer :: scr3(:)

      integer(C_INT), intent(out)::ierr
      integer i, j, n, nlindep

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

      call eig_sqr_inv(data_0,data_1,n,
     *                 scr1,nscr1,scr2,scr3,ierr,nlindep)

      data_2(1) = real(nlindep)
    
      call delete_scratch_array(cscr1)
      call delete_scratch_array(cscr2)
      call delete_scratch_array(cscr3)

      if (ierr .ne. 0) call abort_job()

      return
      end

      subroutine eig_sqr_inv(Amat,Bvec,n,
     *                       work,nw,sw,eigval,ierr,nlindep)
      use, intrinsic :: ISO_C_BINDING
      implicit none

      include 'sip_interface.f'

      integer n,nw,nsw
      double precision Amat(n,n)
      double precision Bvec(n,n)
      double precision work(nw)
      double precision sw(n,n)
      double precision eigval(n)

      integer nlindep

      integer i, j, k, index, ierr, lda
      double precision dtmp, qqq, tresh
      integer lindep_tol

      double precision tolerance
      parameter (tolerance = 1.0d-5)

      call dsyev_wrapper(Amat,Bvec,n,work,nw,sw,eigval,ierr)

c
c take the square root
c
      do i = 1, n
          Amat(i,i) = dsqrt(Amat(i,i))
      enddo 
c
c take the inverse


      nlindep = query_int_constant("lindep_tol"//C_NULL_CHAR)
      if (nlindep .eq. 1) then
	  lindep_tol = int_constant("lindep_tol"//C_NULL_CHAR)
	  tresh = 10.d0**(-1.d0*real(lindep_tol))
      else
	  tresh = 1.d-9
      endif
c------------------------------------------------------------------
      NLINDEP = 0
      do i = 1, n
         qqq = Amat(i,i)*Amat(i,i) 
         IF(dabs(qqq).ge.tresh) then 
         Amat(i,i) = 1.0d0/Amat(i,i) 
         ELSE
         Amat(i,i) = 0.d0
         NLINDEP = NLINDEP + 1
         ENDIF
	 eigval(i) = Amat(i,i)
      enddo 
c------------------------------------------------------------------
      if (nlindep .gt. 0) then
C place 0 values at end of array
      do i = 1,n
          work(i) = real(i)
          if ( abs(eigval(i)) .eq. 0.d0) then
              eigval(i) = 999.d0
          endif
      enddo
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
          if ( abs(eigval(i)) .eq. 999.d0 ) then
              eigval(i) = 0.d0
          endif
      enddo

      do i = 1,n
      do j = 1,n
          Amat(i,j) = 0.d0
      enddo
      enddo
      do i = 1,n
          Amat(i,i) = eigval(i)
      enddo
      endif
c 
      return
      end


