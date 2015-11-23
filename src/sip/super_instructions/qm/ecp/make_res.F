      Subroutine Make_Res(Lam, Mu, xph, Yph, Zph, Res)
      
      Implicit Double Precision (A-H, O-Z)
C
#include "ecp.par"
#include "ecp.com"
     
      JD = Lam*(Lam+1) - Mu + 1
     
      Jmn = Lmf(Jd)
      Jmx = Lmf(Jd+1) - 1
C
      Res = 0.0D0
      
#ifdef _DEBUG_LVLM1
      Write(6,"(a,10(1x,I3))") "At entry to make_res, Lam and Mu",
     &                          Lam, Mu
      Write(6,"(a,10(1x,I3))") "At entry to make_res, Jmn and Jmx ",
     &                          Jmn, Jmx
#endif
      Do J = Jmn, Jmx
     
         Jx = Lmx(J)
         Jy = Lmy(J)
         Jz = Lmz(J)  
         Res = Res + Zlm(J)*(Xph**Jx)*(Yph**Jy)*(Zph**Jz)
C
#ifdef _DEBUG_LVLM1
      Write(6,"(a,3(1x,i2),2(1x,F15.8))"), "The real Sph ",jx,jy,jz,
     &          Zlm(J),Res
#endif
      Enddo
     
      Return
      End
            
           
