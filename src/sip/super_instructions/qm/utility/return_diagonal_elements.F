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

      TYPE(C_PTR) :: n_ao_segs, aosegs
      integer, pointer :: pn_ao_segs(:), paosegs(:)
      integer(C_INT) offset_1, offset_2, offset_3, offset_4
      integer(C_INT) offset_5, offset_6
      integer(C_INT) start_1, start_2, start_3, start_4, start_5,start_6
      integer(C_INT) i, i1, i2, i3, i4, one
c Get the mo segment ranges
      call predefined_int_array("moa_seg_ranges"//C_NULL_CHAR, one,
     *                           n_ao_segs, aosegs)
      call c_f_pointer(n_ao_segs,pn_ao_segs,[1])
      call c_f_pointer(aosegs,paosegs,pn_ao_segs)

      offset_1 = 0
      offset_2 = 0
      offset_3 = 0
      offset_4 = 0

      if (rank_0 .eq. 2) then
          if (extents_0(1) .ne. extents_0(2)) then
              write(*,*) "T_pq doublet is not square"
	      call abort_job()
	  endif

	  if (index_values_0(1).gt.1) then
	      do i = 1, index_values_0(1) - 1
		 offset_1 = offset_1 + paosegs(i)
	      enddo
	  endif
	  n1 = extents_0(1)
	  call ret_diag_tensor2(data_0,offset_1+1,offset_1+n1)
      elseif (rank_0 .eq. 4) then
          if (extents_0(1) .ne. extents_0(2) .or.
     *        extents_0(3) .ne. extents_0(4)) then
              write(*,*) "T_pqrs doublets are not square",
     *        extents_0(1),extents_0(2),extents_0(3),extents_0(4)
	      call abort_job()
	  endif
	  if (index_values_0(1).gt.1) then
	      do i = 1, index_values_0(1) - 1
		 offset_1 = offset_1 + paosegs(i)
	      enddo
	  endif
	  if (index_values_0(3).gt.1) then
	      do i = 1, index_values_0(3) - 1
		 offset_3 = offset_3 + paosegs(i)
	      enddo
	  endif
	  n1 = extents_0(1)
	  n2 = extents_0(3)
	  call ret_diag_tensor4(data_0,
     *	                    offset_1+1,offset_1+n1,
     *	                    offset_3+1,offset_3+n2)
      else
              write(*,*) "bad call to return_diagonal_elements"
	      call abort_job()
      endif

      return
      end

      subroutine ret_diag_tensor2(tensor,n1,n2)
      implicit none
      integer n1, n2
      double precision val
      double precision tensor(n1:n2,n1:n2)

      integer p,q,r,s

      do p = n1,n2
      do q = n1,n2
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

      subroutine ret_diag_tensor4(tensor,a1,a2,b1,b2)
      implicit none
      integer a1,a2,b1,b2
      double precision val
      double precision tensor(a1:a2,a1:a2,b1:b2,b1:b2)

      integer p,q,r,s

      do p = a1,a2
      do q = a1,a2
      do r = b1,b2
      do s = b1,b2
	  val = 0.d0
	  if (p.eq.q .and. r.eq.s) then
	      val = tensor(p,q,r,s)
	  else
	      val = 0.0
	  endif
	  tensor(p,q,r,s) = val
      enddo
      enddo
      enddo
      enddo

      return
      end

