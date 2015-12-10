      Subroutine Binomial_coefs

      Implicit Double Precision (A-H, O-Z)

#include "ecp.par"
#include "ecp.com"

      Bcoefs(0, 0) = 1.0D0

      Do I1 = 1, 2*Maxang

           J2 = 0
           Bcoefs(I1, J2) = 1.D0

           Do J1 = 2,  I1
              J2 = J2 + 1
              Bcoefs(I1, J2) = Bcoefs(I1-1,J2-1) + Bcoefs(I1-1,J2)
           Enddo

              J2 = J2 + 1
              Bcoefs(I1, J2) = 1.D0

      Enddo

      Return
      End

