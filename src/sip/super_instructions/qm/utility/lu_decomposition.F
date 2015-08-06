c--------------------------------------------------------------------------
!> @author Jason N Byrd QTP.
!>   @brief Calculates the LU decomposition of a matrix by calling dgetrf
!
!> @details
!> special lu_decomposition uuu
!>
!>  execute lu_decomposition Array1 Array2 Array3 
!>
!>   where Array1 is the input matrix 
!>
!>         Array2 returns the L
!>
!>         Array3 returns the U
!>
!>   All must be "static" arrays.
c--------------------------------------------------------------------------

      subroutine lu_decomposition(
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

      integer(C_INT), intent(out)::ierr
      integer i, j, n, junk

      n = extents_0(1)

      niscr1 = n

      call integer_scratch_array(niscr1,ciscr1)
      call c_f_pointer(ciscr1, iscr1, [niscr1])
      
      call dgetrf_wrapper(data_0,data_1,data_2,n,iscr1,ierr)

      call delete_integer_scratch_array(ciscr1)

      if (ierr .ne. 0) call abort_job()

      return
      end

      subroutine dgetrf_wrapper(Dmat,DLmat,DUmat,nn,iwork,info)
      use, intrinsic :: ISO_C_BINDING 
      implicit none 

      integer info
      integer nn
      double precision Dmat(nn,nn)
      double precision DLmat(nn,nn)
      double precision DUmat(nn,nn)

      integer iwork(nn)

      integer i,j,k


C returns the lower and upper triangular matricies DLmat and DUmat

      do i = 1,nn
	  iwork(i) = 0.d0
	  do j = 1,nn
	      DLmat(i,j) = 0.d0
	      DUmat(i,j) = 0.d0
	  enddo       
      enddo       

      call dgetrf(nn,nn,Dmat,nn,iwork,info)

      do i = 1,nn
      do j = 1,i
	  DLmat(i,j) = DMat(i,j)
      enddo 
      enddo 

      do i = 1,nn
      do j = i,nn
	  DUmat(i,j) = DMat(i,j)
      enddo 
      enddo 

      do i = 1,nn
	  DLmat(i,i) = 1.d0
      enddo 

 100  format(100F14.6)

      return
      end

