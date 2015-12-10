      Subroutine Screen_ang_typ2(L, M, Nx, Ny, NZ, Lam, Mu, SKIP)

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

      If (Mod(Nx+1 + Lmx(Imn) + Lmx(Jmn), 2) .NE. 1 .OR.
     &    Mod(Ny+1 + Lmy(Imn) + Lmy(Jmn), 2) .NE. 1 .OR.
     &    Mod(Nz+1 + Lmz(Imn) + Lmz(Jmn), 2) .NE. 1)
     &    Skip = .TRUE.

#ifdef _DEBUG_LVLM1
      Write(6,*) NX+1, Lmx(Imn), Lmx(Jmn)
      Write(6,*) Ny+1, Lmy(Imn), Lmy(Jmn)
      Write(6,*) Nz+1, Lmz(Imn), Lmz(Jmn)
      Write(6, "(a,1x,l)") "@-Screen_ang_typ2, Skip: ", Skip
#endif 
      Return
      End


