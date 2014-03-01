      subroutine do_rhf_den2(a1, a2, off_1, b1, b2, off_2, 
     *                       array, 
     *                       m1, m2, n1, n2, fock)  
      
c---------------------------------------------------------------------------
c
c
c---------------------------------------------------------------------------

      implicit none
      integer a, a1, a2, b, b1, b2
      integer m1, m2, n1, n2 
      integer off_1, off_2
      double precision array(a1:a2,b1:b2) 
      double precision fock(m1:m2,n1:n2) 
      double precision val, eps, epsa, epsb   

      val = 0.0 
      do b = b1, b2 
         epsb = fock(b+off_2,b+off_2) 
      do a = a1, a2 
         epsa = fock(a+off_1,a+off_1) 

         eps = epsb - epsa  

c        val = array(a,b,c,d) 
         array(a,b) = array(a,b)/eps  
c         write(6,*) ' EPS:', a, b, c, d, eps, array(a,b,c,d) 
      enddo 
      enddo 
10    format(4I3,3F10.8) 

      return
      end
