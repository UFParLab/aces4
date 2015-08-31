      Subroutine Screen_ang_typ1(L, M, Nx, Ny, NZ, Lam, Mu, SKIP)

      Implicit Double Precision (A-H, O-Z)
      Logical Skip

#include "ecp.par"
#include "ecp.com"

      Id = L*(L+1) - M + 1

      IMn = Lmf(Id)
      Imx = Lmf(Id+1) - 1

      JD = Lam*(Lam+1) - Mu + 1

      JMn = Lmf(Jd)
      Jmx = Lmf(Jd+1) - 1
  
      Skip = .False.

      If (Mod(Nx+1,2) .EQ. Mod(Lmx(Jmn),2) .OR.
     &    Mod(Ny+1,2) .EQ. Mod(Lmy(Jmn),2) .OR.
     &    Mod(Nz+1,2) .EQ. Mod(Lmz(Jmn),2)) Skip = .TRUE.

#ifdef _DEBUG_LVLM1
      Write(6, "(a,1x,l)") "@-Screen_ang_typ1, Skip: ", Skip
      Write(6,*)
#endif 
      Return
      End


