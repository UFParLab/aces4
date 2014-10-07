      subroutine do_rhf_den4(a1, a2, off_1, b1, b2, off_2, 
     *                       c1, c2, off_3, d1, d2, off_4, 
     *                       array, 
     *                       m1, m2, n1, n2, fock)  
      
c---------------------------------------------------------------------------
c
c
c---------------------------------------------------------------------------

      implicit none
      integer a, a1, a2, b, b1, b2, c, c1, c2, d, d1, d2, m1, m2, n1, n2
      integer off_1, off_2, off_3, off_4 
      double precision array(a1:a2,b1:b2,c1:c2,d1:d2) 
      double precision fock(m1:m2,n1:n2) 
      double precision val, eps, epsa, epsb, epsc, epsd   

      val = 0.0 
      do d = d1, d2 
         epsd = fock(d+off_4,d+off_4) 
      do c = c1, c2 
         epsc = fock(c+off_3,c+off_3) 
      do b = b1, b2 
         epsb = fock(b+off_2,b+off_2) 
      do a = a1, a2 
         epsa = fock(a+off_1,a+off_1) 

         eps = epsb + epsd - epsa - epsc 

c        val = array(a,b,c,d) 
c        write(6,*) ' ABCD :', a, b, c, d, '::',
c    *                         a+off_1,b+off_2,c+off_3,d+off_4, eps   
c        write(6,*) 'EPS', epsa, epsb, epsc, epsd 
c        write(6,*) 'array', array(a,b,c,d)  
         array(a,b,c,d) = array(a,b,c,d)/eps  
c         write(6,*) ' EPS:', a, b, c, d, eps, array(a,b,c,d) 
      enddo 
      enddo 
      enddo 
      enddo 
10    format(4I3,3F10.8) 

      return
      end

      subroutine do_rhf_den4_1d(a1, a2, off_1, b1, b2, off_2, 
     *                       c1, c2, off_3, d1, d2, off_4, 
     *                       array, 
     *                       m1, m2, fock)  
      
c---------------------------------------------------------------------------
c
c
c---------------------------------------------------------------------------

      implicit none
      integer a, a1, a2, b, b1, b2, c, c1, c2, d, d1, d2, m1, m2, n1, n2
      integer off_1, off_2, off_3, off_4 
      double precision array(a1:a2,b1:b2,c1:c2,d1:d2) 
      double precision fock(m1:m2) 
      double precision val, eps, epsa, epsb, epsc, epsd   

      val = 0.0 
      do d = d1, d2 
         epsd = fock(d+off_4) 
      do c = c1, c2 
         epsc = fock(c+off_3) 
      do b = b1, b2 
         epsb = fock(b+off_2) 
      do a = a1, a2 
         epsa = fock(a+off_1) 

         eps = epsb + epsd - epsa - epsc 

c        val = array(a,b,c,d) 
c        write(6,*) ' ABCD :', a, b, c, d, '::',
c    *                         a+off_1,b+off_2,c+off_3,d+off_4, eps   
c        write(6,*) 'EPS', epsa, epsb, epsc, epsd 
c        write(6,*) 'array', array(a,b,c,d), epsa, epsb, epsc, epsd, eps   
         array(a,b,c,d) = array(a,b,c,d)/eps  
c         write(6,*) ' EPS:', a, b, c, d, eps, array(a,b,c,d) 
      enddo 
      enddo 
      enddo 
      enddo 
10    format(4I3,3F10.8) 

      return
      end

