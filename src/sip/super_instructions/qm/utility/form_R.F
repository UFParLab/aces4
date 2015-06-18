
! --------------------------------------------------------------------------
!> @author Victor Lotrich QTP.
!> @brief
!>   Symmetrize the b_array into the R array
!>
! --------------------------------------------------------------------------
      subroutine form_R(b_array, R, rpack, v, i1, i2, rank, rank2)
      implicit none 
c
      integer i, j, rank, rank2, i1, i2, ndim, nzero, n   
      double precision R(rank+1,rank+1), rpack(rank2), v(rank+1) 
      double precision b_array(i2,i2)
      double precision c1,c2,c3,c4,c5,c6,c7,c8,c9,c10
      double precision temp
#ifdef __crayx1
      integer*8 arg64(5), iP(11)
#else
      integer iP(rank+1)
#endif

      integer maxdim
      maxdim = rank 
c      print *, 'maxdim = ', maxdim
c      print *, 'i2 = ', i2
c      print *, 'i1 = ', i1
c      print *, 'rank = ', rank
c      print *, 'rank2 = ', rank2

      do i = 1, maxdim
      do j = i, maxdim
         R(i,j) = b_array(i,j)
         if (i .ne. j) R(j,i) = b_array(i,j) 
      enddo
      enddo

      ndim = maxdim
      do i = 1, maxdim
         nzero = maxdim
         do j = 1, maxdim
            if (dabs(r(i,j)) .gt. 0.d0) then
               nzero = nzero - 1
            endif
         enddo
         if (nzero .eq. maxdim) ndim = ndim - 1
      enddo

      do i = 1, ndim
         R(i,ndim+1) = -1.0d0 
         R(ndim+1,i) = -1.0d0 
      enddo ! i 
      R(ndim+1,ndim+1) =  0.0d0 

      n = 0
      do j = 1, ndim+1
      do i = 1, ndim+1
         n = n + 1
         rpack(n) = r(i,j)
      enddo
      enddo

c
c Form the v-vector. 
c ------------------ 
c
      do i = 1, ndim
         v(i) = 0.
      enddo
      v(ndim+1) = -1.d0

c     do i = 1, 10     
c     do j = 1, 10   
c        write(6,*) ' BB:', i,j, b_array(i,j)
c     enddo 
c     enddo 

c
c Solve the eigenvalue problem R * C = v for the vector C.
c -------------------------------------------------------- 
c
#ifdef __crayx1
      arg64(1) = ndim+1
      arg64(2) = 1
      arg64(3) = ndim+1
      arg64(4) = ndim+1
      arg64(5) = i
      call dgesv(arg64(1), arg64(2), rpack, arg64(3), iP, v,
     *           arg64(4), arg64(5))
      i = arg64(5)  
#else
      call dgesv(ndim+1,1,rpack,ndim+1,iP,v,ndim+1,i)  
#endif

      if (i .ne. 0) then
         print *,'Error in dgesv: info = ',i
         call abort_job()
      endif

c
c Put solutions into b_array for use in the SIAL program. 
c ------------------------------------------------------------------------ 
c
      do i = ndim+1, maxdim+1
         v(i) = 0.
      enddo

      do i = 1, rank   
         b_array(i,i) = v(i) 
c        write(6,*) ' B:', i, b_array(i,i)
      enddo 
c
      end 
c 

