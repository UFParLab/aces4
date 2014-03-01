
      integer function test_contraction_small2 (actual_c) bind(C)
      use, intrinsic :: ISO_C_BINDING


      implicit none
      integer, parameter :: MU_ = 9
      integer, parameter :: LA_ = 9
      integer, parameter :: I_ = 5
      integer, parameter :: I1_ = 5
      integer, parameter :: A1_ = 4

      real(C_DOUBLE), intent(out) :: actual_c(MU_, I1_, A1_, I_)
      real(C_DOUBLE) :: a(MU_, I1_, I_, LA_)
      real(C_DOUBLE) :: b(LA_, A1_)
      real(C_DOUBLE) :: c(MU_, I1_, A1_, I_)
      
      integer(C_INT) :: mu, la, i, i1, a1, flag
      character(len=30) :: format
      real(C_DOUBLE) counter 

      interface
      subroutine fill_arr(a, elems, counter)
      use, intrinsic :: ISO_C_BINDING
      real(C_DOUBLE), dimension(*) :: a
      integer(C_INT) :: elems
      real(C_DOUBLE) :: counter
      end subroutine fill_arr
      end interface

      flag = 1 

      counter = 1.0
      call fill_arr(a, mu_*i1_*i_*la_, counter)
      call fill_arr(b, la_*a1_, counter)

        
      do mu = 1, MU_
         do i1 = 1, I1_
             do i = 1, I_
                 do a1 = 1, A1_
                     c(mu, i1, a1, i) = 0.0
                     do la = 1, LA_
                        c(mu, i1, a1, i) = c(mu, i1, a1, i) + 
     *                                   a(mu, i1, i, la) * b(la, a1)
                     enddo
                 enddo
             enddo
         enddo
      enddo

      do mu = 1, MU_
        do i1 = 1, I1_
          do a1 = 1, A1_
            do i = 1, I1_
              if (actual_c(mu, i1, a1, i) .ne. c(mu, i1, a1, i)) then
                  flag = 0
              endif
            enddo
          enddo
        enddo
      enddo

      test_contraction_small2 = flag

      return
      end


      subroutine fill_arr(a, elems, counter)
      use, intrinsic :: ISO_C_BINDING
      implicit none
      real(C_DOUBLE), dimension(*) :: a
      integer(C_INT) :: elems
      real(C_DOUBLE) :: counter
      integer(C_INT) :: i

      real(C_DOUBLE) cnt 
      cnt = counter

      do i= 1, elems
            a(i) = cnt
            cnt = cnt + 1
            if (cnt .eq. 21) cnt = 1
      enddo

      return
      end





