! --------------------------------------------------------------------------
!> @author Jason N Byrd QTP
!> @brief
!> force zeroes off diagonal element of a block.
!> 4 index returns Tpqrs = Tpqrs * delta_pq*delta_rs
!>
!> @details
!>   special return_diagonal u
!>
!>   Usage: execute return_diagonal temp
!>
! --------------------------------------------------------------------------

      subroutine return_diagonal_elements(
     *  array_0, rank_0, index_values_0, size_0, extents_0, data_0,  
     *  ierr) BIND(C) 
      use, intrinsic :: ISO_C_BINDING 
      implicit none 

      include 'sip_interface.f' 

!     output array --> array_0 
      integer(C_INT), intent(in)::array_0
      integer(C_INT), intent(in)::rank_0
      integer(C_INT), intent(in)::index_values_0(1:rank_0)
      integer(C_INT), intent(in)::size_0
      integer(C_INT), intent(in)::extents_0(1:rank_0)
      real(C_DOUBLE), intent(out)::data_0(1:size_0) 
      integer(C_INT), intent(out)::ierr  
      integer(C_INT) :: n1,n2
      integer :: i

      if (rank_0 .eq. 2 .and.
     *    index_values_0(1).eq.index_values_0(2)) then
          if (extents_0(1) .ne. extents_0(2)) then
              write(*,*) "T_pq doublet is not square"
	      call abort_job()
	  endif
	  n1 = extents_0(1)
	  call ret_diag_tensor2(data_0,n1)
      elseif (rank_0 .eq. 4 .and.
     *    index_values_0(1).eq.index_values_0(2) .and.
     *    index_values_0(3).eq.index_values_0(4)) then
          if (extents_0(1) .ne. extents_0(2) .or.
     *        extents_0(3) .ne. extents_0(4)) then
              write(*,*) "T_pqrs doublets are not square",
     *        extents_0(1),extents_0(2),extents_0(3),extents_0(4)
	      call abort_job()
	  endif
	  n1 = extents_0(1)
	  n2 = extents_0(3)
C	  call ret_diag_tensor4(data_0,n1,n2)
	  call ret_diag_tensor4(data_0,
     *                          1,extents_0(1),
     *                          1,extents_0(2),
     *                          1,extents_0(3),
     *                          1,extents_0(4))
      else
C              write(*,*) "bad call to return_diagonal_elements"
C	      call abort_job()
          do i = 1,size_0
	      data_0(i) = 0.d0
          enddo
      endif

      return
      end

      subroutine ret_diag_tensor2(tensor,n1)
      implicit none
      integer n1
      double precision val
      double precision tensor(n1,n1)

      integer p,q,r,s

      do p = 1,n1
      do q = 1,n1
	  val = tensor(p,q)
	  if (p .eq. q) then
	      val = tensor(p,q)
	  else
	      val = 0.d0
	  endif
	  tensor(p,q) = val
      enddo
      enddo

      return
      end

      subroutine ret_diag_tensor4(x, a1,a2,
     *                      i1,i2,b1,b2,
     *                      j1,j2)
      implicit none

      integer a1,a2,b1,b2,i1,i2,j1,j2
      integer atype, itype, btype, jtype
      double precision x(a1:a2,i1:i2,b1:b2,j1:j2)

      integer a,b,i,j
      double precision val

      do j = j1,j2
      do b = b1,b2
         do i = i1, i2
         do a = a1,a2
            val = 0.0d0
            if ((i .eq. a) .and. (j .eq. b)) then
               val =  x(a,i,b,j)
            else
               val = 0.0d0
            endif
            x(a,i,b,j) = val
         enddo
         enddo
      enddo
      enddo


      return
      end

