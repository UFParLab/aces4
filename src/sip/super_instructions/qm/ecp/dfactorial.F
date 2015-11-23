      Subroutine Dfactorial 
C 
C Compute Double factorial of n!! (n(n-2)...531) or (n(n-2)..6421) 
C depending on n is odd or even. 0!!,-1!! = 0
c
      Implicit Double Precision(A-H, O-Z)

#include "ecp.par"
#include "ecp.com"
      
      Fac2(-1) = 1.0D0
      Fac2(0)  = 1.0D0
      Fac2(1)  = 1.0D0
      Fac2(2)  = 2.0D0

      FI = 2.0D0

      Do I = 2, 4*Maxang-1
         FI = FI + 1.0D0
         Fac2(I+1) =  FI*Fac2(I-1)
      EndDo

      Return
      End

