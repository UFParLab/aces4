      Subroutine Ecp_rad_int_typ2(Lamalo, Lamahi, Lamblo, Lambhi,
     &                            Lamau, Lambu, La, Lb, Ltot, Lprj,  
     &                            NP, Exp1, Exp2, Exp12, Zeta, Dlj, 
     &                            Xahat, Yahat, Zahat, Xbhat, Ybhat,
     &                            Zbhat, Xp, Yp, Zp, Xc,
     &                            Yc, Zc, CA, CB, Pc2, Fact_ab, Rad2,
     &                            Rad2_zero)

      Implicit Double Precision (A-H, O-Z)
      Logical Rad2_zero
       
#include "ecp.par"
#include "ecp.com"
C
      Dimension Rad2(0:2*Maxang,0:2*Maxang, 0:2*Maxang)

      Alpha = Exp12 + Zeta

      Beta1 = 2.0D0*Ca*Exp1
      Beta2 = 2.0D0*Cb*Exp2

      Talpha = 2.0D0*Alpha
      Falpha = 4.0D0*Alpha
      
      Rc  = (Beta1 + Beta2)/Talpha 
      Rc2 = (Beta1 + Beta2)**2/Falpha

C The commented block of code is how I would have computed the
C exponential factor. However, the one below is presented as
C simplifaction in IJQC, 40, 773., 1991. I do not see how. If
C I do not use what is on above paper, I can not match with 
C older codes. 
CSSS      Call Get_center(Xp, Yp, Zp, Xc, Yc, Zc, Exp12, Zeta,
CSSS     &                xq, Yq, Zq, Exp123, Fact_Pc)
CSSS
CSSS      Exp_fac = Fact_ab + Fact_pc

      Fact_ab = ((Exp1*Exp2)/(Exp12))*(Ca-Cb)**2
      Fact_pc = (Zeta*Alpha)*(Rc**2)/(Exp12)
      Exp_fac = Fact_ab + Fact_pc
      Rad2_zero = .True. 

#ifdef _DEBUG_LVL0
      Write(6,*)
      Write(6,"(a,3(1x,F15.10))") "Ca and Cb", Ca, Cb
      Write(6,"(a,3(1x,F15.10))") "Alpha, RC, RC2", Alpha, RC, RC2
      Write(6,"(a,2F15.10)") "The Fact_ab and Pc", Fact_ab, Fact_pc
      Write(6,"(a,F15.10)") "The exponential factor", Exp_fac
#endif

      If (Exp_fac .Le. Tol) Then
      
         Rad2_zero = .False.

         Pre_fac = Dlj*Exp_fac

CSSS         If (Beta1 .EQ. 0 .AND. Beta2 .EQ. 0) 
CSSS            Beta = 0
CSSS            Xval = 0
CSSS            Rad2(Ltot,1,1) = Rad2(Ltot,1,1) + Qcomp(NP+Ltot,0)
CSSS         Elseif (Beta1 .EQ. 0) Then
CSSS            Beta = Beta2
CSSS            Xval = Rc2
CSSS            Do Lamb = Lprj, Lambhi
CSSS               Rad2(Ltot,Lamb,1) = Rad2(Ltot,1,1) + Qcomp(NP+Ltot,0)
CSSS            Enddo
#ifdef _DEBUG_LVL0
CSSS       Temp = 20.0D0
CSSS       Write(6, '(a)') "The Gpwm algorithm for Rad2 is Choosn"
CSSS       If (Temp .LT. 50.0D0) Then
#endif 
         If (Rc2 .LT. 50.0D0) Then
C
C Use the single powers series with recrusions (Eqns. 51 and 52).

            Call Sps(Np, Lprj, La, Lb, Ltot, Lamalo, Lamahi, Lamblo, 
     &               Lambhi, Lamau, Lambu, Alpha, Beta1, Beta2, 
     &               Dlj, Exp_fac+Rc2, Rad2)
C 
#ifdef _DEBUG_LVL0
      Write(6,*)
      Write(6,*) "The Rad2 after Sps"
      Do i=0, 2
         Do j= 0, 2
            Write(6,"(4(1x,F15.13))") (Rad2(k,j,I), k=0,9)
         Enddo
       Enddo
#endif
         Else

C Use the Gaussian quadrature (points and weights)
 
             Prd = Dexp(-Exp_fac)*Dlj
            
             Trc1 = Max(2.0D0*Rc*Beta1,1.0D0)
             Trc2 = Max(2.0D0*Rc*Beta2,1.0D0)
             
             Fexp1 = (4.0D0*Exp1)**(La+1)
             Fexp2 = (4.0D0*Exp2)**(Lb+1)
             Taiaj = (4.0D0*Exp1*Exp2)
            
             Term1 = Dabs(Prd)/(Trc1*Trc2)
             Term2 = Dsqrt(4.0D0*Fexp1*Fexp2*Dsqrt(Taiaj)/Alpha)
         
             Ecp_lim = Term1*Term2
#ifdef _DEBUG_LVL0
             Write(6,*)
             Write(6,"(a,3(1x,F15.10))")"The pre and approx. int: ",
     &                 Ecp_lim,Prd
#endif
             If (Rc .Ge. Ca) Then
                Nplim = Np + La
                If (Rc .Ge. CB) Then
                    Nplim = Np + Lb
                Else
                    Eclim = Ecp_lim*Cb**Lb
                Endif
             Else
                Nplim = Np
                Eclim = Ecp_lim*Ca**la
                If (Rc .Ge. CB) Then
                    Nplim = Np + Lb
                Else
                    Eclim = Ecp_lim*Cb**Lb
                Endif
             Endif
             
             Aprx_Ecp = Eclim*Rc**Nplim

             If (Aprx_Ecp .Gt. Eps1) Call Gpwm(Np, Lprj, La, Lb, Ltot, 
     &                                         Lamalo, Lamahi, Lamblo, 
     &                                         Lambhi, Lamau, Lambu, 
     &                                         Alpha, Rc, Rc2, Prd, 
     &                                         Beta1, Beta2, Rad2)

#ifdef _DEBUG_LVL0
      Write(6,*)
      Write(6,*) "The Rad2 after Gpwm"
      Do i=0, 2
         Do j= 0, 2
            Write(6,*) i,j
            Write(6,"(4(1x,F15.10))") (Rad2(k,j,I), k=0, 9)
         Enddo
       Enddo
#endif

         Endif
     
      Endif 
        
      Return
      End
