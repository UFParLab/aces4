      Subroutine Ang_int(L, M, Ii, Jj, Kk, Lam, Mu, Angi)
      
      Implicit Double Precision (A-H, O-Z)
C
#include "ecp.par"
#include "ecp.com"
     
      Id = L*(L+1) - M + 1

      IMn = Lmf(Id)
      Imx = Lmf(Id+1) - 1
   
      JD = Lam*(Lam+1) - Mu + 1
     
      JMn = Lmf(Jd)
      Jmx = Lmf(Jd+1) - 1
C
      Angi = 0.0D0

#ifdef _DEBUG_LVLM1
      Write(6,"(a,9(1x,I3))") "At entry to Ang_int ", II,JJ,KK,
     &                          lam,mu,Imn,Imx,Jmn,Jmx
#endif

      Do I = Imn, Imx
         Angj = 0.D0
         Do J = Jmn, Jmx
            Jx = Lmx(I) + Ii + Lmx(J) 
            Jy = Lmy(I) + Jj + Lmy(J) 
            Jz = Lmz(I) + Kk + Lmz(J) 
            Angj = Angj + Zlm(j)*Fijk(Jx, Jy, Jz)
C
#ifdef _DEBUG_LVLM1
      Write(6,*) "The angular part of Ints"
      Write(6,"(3(1x,i2),3(1x,F15.8))"), jx,jy,jz,
     &          Zlm(J),Fijk(Jx, Jy, Jz),angj
#endif
         Enddo
         Angi = Angi + Angj*Zlm(i)
C
      Enddo
     
      Return
      End
            
           
