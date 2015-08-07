c--------------------------------------------------------------------------
!> @author Jason N Byrd QTP.
!>   @brief Calculates the inverse of L and U from an LU decomposition of a matrix
!
!> @details
!> special inverse_lu_decomposition uuu
!>
!>  execute inverse_lu_decomposition Array1 Array2 Array3 
!>
!>   where Array1 is the input matrix 
!>
!>         Array2 returns the inverse of L
!>
!>         Array3 returns the inverse of U
!>
!>   All must be "static" arrays.
c--------------------------------------------------------------------------

      subroutine inverse_lu_decomposition(
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

      TYPE(C_PTR) :: ciscr1
      integer(c_int) :: niscr1
      integer(c_int), pointer :: iscr1(:)
      TYPE(C_PTR) :: ciscr2
      integer(c_int) :: niscr2
      integer(c_int), pointer :: iscr2(:)

      integer(C_INT), intent(out)::ierr
      integer i, j, n, junk

      n = extents_0(1)

      niscr1 = n
      niscr2 = n

      call integer_scratch_array(niscr1,ciscr1)
      call c_f_pointer(ciscr1, iscr1, [niscr1])
      
      call dgetrf_wrapper(data_0,data_1,data_2,n,iscr1,ierr)

      call integer_scratch_array(niscr2,ciscr2)
      call c_f_pointer(ciscr2, iscr2, [niscr2])

      call invert_lu(data_0,data_1,data_2,n,iscr1,iscr2,ierr)

      call delete_integer_scratch_array(ciscr1)
      call delete_integer_scratch_array(ciscr2)

      if (ierr .ne. 0) call abort_job()

      return
      end

      subroutine invert_lu(scratch,DLmat,DUmat,nn,iwork,Pmat,info)
      use, intrinsic :: ISO_C_BINDING 
      implicit none 

      integer info
      integer nn
      double precision scratch(nn,nn)
      double precision DLmat(nn,nn)
      double precision DUmat(nn,nn)

      integer iwork(nn), Pmat(nn)

      integer i,j,k

      do i = 1,nn
	  do j = 1,nn
	      scratch(i,j) = 0.d0
	  enddo 
	  scratch(i,i) = 1.d0
      enddo 

      do i = 1,nn
	  Pmat(i) = i
      enddo 
      do i = 1,nn
	  j = iwork(i)
	  k = Pmat(j)
	  Pmat(j) = i
	  Pmat(i) = k
      enddo 

      print *, (Pmat(i),i=1,nn)

      call dgesv(nn,nn,DLmat,nn,iwork,scratch,nn,info)
      print *, (iwork(i),i=1,nn)

      do i = 1,nn
	  do j = 1,nn
              DLmat(i,j) = scratch(i,Pmat(j))
	  enddo 
	  write(6,100) (DLmat(i,j),j=1,nn)
      enddo 

      do i = 1,nn
	  do j = 1,nn
	      scratch(i,j) = 0.d0
	  enddo 
	  scratch(i,i) = 1.d0
      enddo 

      call dgesv(nn,nn,DUmat,nn,iwork,scratch,nn,info)

      do i = 1,nn
	  do j = 1,nn
	      DUmat(i,j) = scratch(i,j)
	  enddo 
      enddo 

C      print *, ""
C      do i = 1,nn
C	  write(*,100) (DLmat(i,j),j=1,nn)
C      enddo       
C      print *, ""
C      do i = 1,nn
C	  write(*,100) (DUmat(i,j),j=1,nn)
C      enddo       

 100  format(100F14.6)

      return
      end
