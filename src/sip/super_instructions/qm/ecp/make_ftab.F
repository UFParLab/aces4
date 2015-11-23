C
      subroutine Make_Ftab

      Implicit Double Precision (A-H,O-Z)

#include "ecp.par"
#include "ecp.com"

C This routine set up the F-function table. F functions is the
C angula integal Int{domega x^iy^jz^k = 4Pi(i-1)!!(j-1)!!(k-1)!!/
C (i+j+k)!! for even i,j and k. 
C
      Call Dzero(Fijk, (4*Maxang+1)**3) 

      Fijk(0,0,0) = Fpi

      Do I = 0, 4*Maxang, 2
         Do J = 0, 4*Maxang, 2
            Do K = 0, 4*Maxang, 2

                  If (I .NE. 0) Then

                     Fijk(I,J,K) = Fijk(I-2,J,K)*(I-1)/(I+J+K+1.0D0)

                  Elseif (J .NE. 0) Then

                     Fijk(I,J,K) = Fijk(I,J-2,K)*(J-1)/(I+J+K+1.0D0)

                  Elseif (K. NE. 0) Then

                     Fijk(I,J,K) = Fijk(I,J,K-2)*(K-1)/(I+J+K+1.0D0)

                  Endif

            Enddo
         Enddo
      Enddo

#ifdef _DEBUG_LVLM1
      Do i=0, 4*Maxang, 2
        Do j=0, 4*Maxang, 2
           Write(6,"(4F15.7)") (Fijk(i,j,k),k=0, 4*Maxang,2)
        Enddo
      Enddo
      Write(6,*)
#endif

      Return
      End
