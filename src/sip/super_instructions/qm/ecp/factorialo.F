      Subroutine Factorialo
C 
C Compute factorial of (2n-1)! (2n-1..3.1), and 0! = 1
c
      Implicit Double Precision(A-H, O-Z)

#include "ecp.par"
#include "ecp.com"
      
      faco(0)  = 1.0D0
      Faco(1)  = 1.0D0

      Do I = 2, 2*Maxang
         Faco(I) =  (2*I-1)*Faco(I-1)
      EndDo

      Return
      End

