      subroutine ECP_rad_int_typ1(Lamu, Ltot, Np, Dlj, Exp12, Zeta, 
     &                            Xp, Yp, Zp, Xc, Yc, Zc, Pc2, 
     &                            Fact_ab, Rad, Rad1_zero)
 
C The exp12=exp1+exp2, P is the common center of Gaussian on A and
C B, Fact_ab is the external factor that results in migrating to
C the common center P.

      Implicit Double Precision(A-H, O-Z)
      Logical Rad1_zero 

#include "ecp.par"
#include "ecp.com"
      
      Dimension Qnl(0:2*Maxang,0:2*Maxang), Rad(0:2*Maxang,0:2*Maxang)
C
      Alpha = Exp12 + Zeta
C
C Express the gaussians on P and C (ECP) to common center and obtain
C the new external factor.

      Call Get_center(Xp, Yp, Zp, Xc, Yc, Zc, Exp12, Zeta,
     &                xq, Yq, Zq, Exp123, Fact_Pc)

      Exp_fac = Fact_AB + Fact_PC

CSS      Exp_fac = Dexp(-Exp_fac)
CSS      Pre_fac = Dlj*Exp_fac
C
C to match with Dalton.
C
C Notes Rk = 2(exp1+exp2)*PC, t=(Rk)^2/4*(exp1+exp2+zeta)
C See expressions 19 on JCP 65, 1976, page 3856 and McMurchie
C and Davidson, 44, 289, 1981 (t is k^2/4alpha)
C             t  = (Exp12**2)*Pc2/Alpha 
C             Rk = Rp 
#ifdef _DEBUG_LVL0
      Write(6,"(a,1x,F15.10)") "The exponential Factor: ", Exp_fac 
      Write(6,*) 
#endif 

       Rad1_zero = .True.

       If (Exp_fac .Le. Rint_cutoff .And. Dlj .Ne. 0.0D0) Then

          Exp_fac = Dexp(-Exp_fac)
          Pre_fac = Dlj*Exp_fac

          Rad1_zero = .False. 

          Xval = Exp12**2*Pc2/Alpha

          Call Radint_recur(Np, Ltot, Qnl)

#ifdef _DEBUG_LVL0
      Write(6,*) "Radial int before multyplying by pre factor"
      Do i=0,5 
         Write(6,"(6(1x,F10.7))") (Qnl(j,I), j=0, 5)
      Enddo
#endif
c 
          Do Lam = 0, Lamu
             Nhi = Ltot - Mod(Ltot-Lam,2)
             Do N = Lam, Nhi, 2

                Rad(N, Lam) = Rad(N, Lam) +  Pre_fac*Qnl(N, Lam)

             Enddo
          Enddo

#ifdef _DEBUG_LVL0
      Write(6,"(a,1x,F15.10)") 
     &         "The radial ints after  mult. by pre factor", Pre_fac
      Do i=0,5 
         Write(6,"(6(1x,F10.7))") (Rad(j,I), j=0,5)
      Enddo
#endif
      
      Endif

      Return
      End

