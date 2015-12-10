      Subroutine Factorial
C 
C Compute factorial of n! (n(n-1)...21), and 0! = 1
c
      Implicit Double Precision(A-H, O-Z)

#include "ecp.par"
#include "ecp.com"
      
      fact(0)  = 1.0D0
      Fact(1)  = 1.0D0
      Fact(2)  = 2.0D0

      FI = 2.0D0

      Do I = 2, 2*Maxang-1
         FI = FI + 1.0D0
         Fact(I+1) =  FI*Fact(I)
      EndDo

      Do I = 1, 2*Maxang
         Do J = 1, I
            Fprod(J, I) = Fact(I+(J-1)-1)/(Fact(J-1)*Fact(I -(J-1)-1))
         Enddo
      Enddo

      Return
      End

