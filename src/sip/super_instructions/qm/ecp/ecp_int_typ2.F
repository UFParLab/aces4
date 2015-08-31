      Subroutine Ecp_int_typ2(Iecp_cnt, Lamau, Lambu, La, Lb, Exp1,
     &                        Exp2, Exp12, Xahat, Yahat, Zahat, Xbhat,
     &                        Ybhat, Zbhat, Xp, Yp, Zp, Xc, Yc, Zc, 
     &                        Acs_xyz, Bcs_xyz, Ca, Cb, Pc2, Fact_ab,
     &                        CA_Zero, CB_zero, Cint, Rad2_zero, 
     &                        Grads,
     &
     &                        nshells,max_ecp_prims,
     &                        necp_atoms,max_necp_functns,
     &                        nval_prims,nval_ccoeffs,
     &                        max_unfns_shell,max_cofns_shell,
     &                        nval_bfns,max_nprims,max_ncontfns,
     &                        ntotatoms,
     &
     &                        clp,zlp,nlp,kfirst,klast,llmax,
     &                        nelecp,ipseux,ipseud,igenby,
     & 
     &                        numcon,katom,ktype,kprim,kmini,
     &                        kmaxi,nprims,ndegen,expnt,contr)

      Implicit Double Precision (A-H, O-Z)
      Logical Rad2_zero, CA_zero, CB_zero, Grads

#include "ecp.par"
#include "ecp.com"
   
      Dimension ACS_xyz(0:2*Maxang,3),BCS_xyz(0:2*Maxang,3)
      Dimension Rad2(0:2*Maxang,0:2*Maxang, 0:2*Maxang),
     &          Ang2_A(0:2*Maxang,-Maxproj:Maxproj,0:2*Maxang),
     &          Ang2_B(0:2*Maxang,-Maxproj:Maxproj,0:2*Maxang)

      integer ipseud,nelecp(ntotatoms),ipseux(ntotatoms)
      integer llmax(ntotatoms),nlp(max_ecp_prims)
      integer kfirst(necp_atoms,max_necp_functns)
      integer klast(necp_atoms,max_necp_functns)
      double precision clp(max_ecp_prims),zlp(max_ecp_prims)

      integer numcon(nshells),numpri(nshells)
      integer katom(nshells),ktype(nshells)
      integer kmini(nshells),kmaxi(nshells)
      integer ndegen(nshells)
      integer nprims(nshells),kprim(nshells)

      double precision Cint(max_unfns_shell * max_unfns_shell)

      Lmax  = LLMax(Iecp_cnt)
      If (Grads) Then
          Lap = La + 1
         Ltot = La + Lb + 1
      Else
          Lap = La
         Ltot = La + Lb 
      Endif
C     
#ifdef _DEBUG_LVL0
CSS      Lmax = 2
      Write(6,"(a,1x,i2)") "The Lmax value = ",Lmax
      Write(6,*) "CA and CB zero?: ", CA_zero, CB_zero
#endif
CC
C This handles U(l)-U(lmax), l = 0,...,lmax-1 
C
      Test = ((exp1*exp2)/exp12)*(Ca-Cb)**2
      If (Test .Gt. Tol) Then
         Rad2_zero = .True.
         Return
      Endif
           
      If (.Not. CA_zero) Then
          Lamau = Lamau + Lmax - 1
      Else
CSSS          Lamau = 1
          Lamau = 0
      Endif
      If (.Not. CB_zero) Then
          Lambu = Lambu + Lmax - 1
      Else
CSSS          Lambu = 1
          Lambu = 0
      Endif

      If (CA_zero .And. CB_zero) then
          Lprjhi = Min(Lmax-1, Lap+1, Lb+1)
          Lprjlo = Mod(Lap, 2)
          Inc    = 2
C
          If (Lprjlo .Ne. Mod(Lb,2) .Or. Lprjlo .Gt. Lprjhi) Then
             Rad2_zero = .True.
             Return
          Endif

      Elseif (CA_zero) Then
          Lprjhi = Min(Lmax-1, Lap+1)
          Lprjlo = Mod(Lap, 2)
          Inc    = 2

          If (Lprjlo .Gt. Lprjhi) Then
             Rad2_zero = .True.
             Return
          Endif
C
      Elseif (CB_zero) Then
          Lprjhi = Min(Lmax-1, Lb+1)
          Lprjlo = Mod(Lb, 2)
          Inc    = 2

          If (Lprjlo .Gt. Lprjhi) Then
             Rad2_zero = .True.
             Return
          Endif
C
      Else     
          Lprjhi = Lmax-1 
          Lprjlo = 0
          Inc    = 1
      Endif
C          
#ifdef _DEBUG_LVL0
      Write(6,*) "Initialize debug values for Lprjlo, Lprjhi"
CS      Lprjlo = 0
CS      Lprjhi = 0
      Write(6, "(a,2(1x,i3))") "Lprjlo, Lprjhi:", Lprjlo, Lprjhi
#endif
C

      Do Lprj = Lprjlo, Lprjhi, Inc

         Lamalo = Max(Lprj-Lap, 0)
         Lamahi = Min(Lprj+Lap, Lamau)
         Lamblo = Max(Lprj-Lb, 0)
         Lambhi = Min(Lprj+Lb, Lambu)

#ifdef _DEBUG_LVL0
      Write(6,"(a,1x,i2)") "Loop over lprj-lprj", lprj
      Write(6,"(a,4(1x,i2))") "Lamalo,Lamahi,Lamblo,Lambhi", 
     &                         Lamalo,Lamahi,Lamblo,Lambhi
#endif
        Call Dzero(Rad2, (2*Maxang+1)**3)
	Kcrl = Kfirst(Lprj+2, Iecp_cnt)
        Kcru = Klast(Lprj+2, Iecp_cnt)

#ifdef _DEBUG_LVL0
CS      Write(6,"(a)") "Set Debug values for Kcr and Kcru"
CS      Kcrl = 1
CS      Kcru = 1
      Write(6, "(a,2(1x,i3))") "Kcrl and Kcru:", Kcrl, Kcru
#endif
         Do Kcr = Kcrl, kcru
        
            Zeta = Zlp(Kcr)
            Np   = Nlp(Kcr)
            Dlj  = Clp(Kcr)

#ifdef _DEBUG_LVL0
CSS      Write(6,*) "Initialize debug values for Zeta, Np and Dlj"
CSS      Zeta = 3.0D0
CSS      NP   = 0
CSS      Dlj  = 1.0D0
      Write(6, "(a,(1x,i2,F10.6,1x,F10.6))") "Np,Zeta,Dlj: ",
     &      Np, Zeta, Dlj
#endif
C
            Call Ecp_rad_int_typ2(Lamalo, Lamahi, Lamblo, Lambhi,
     &                            Lamau, Lambu, Lap, Lb, Ltot, Lprj, 
     &                            Np, Exp1, Exp2, Exp12, Zeta, Dlj, 
     &                            Xahat, Yahat, Zahat, Xbhat, Ybhat, 
     &                            Zbhat, Xp, Yp, Zp, Xc, Yc, Zc, CA,
     &                            CB, Pc2, Fact_ab, Rad2, Rad2_zero)
         Enddo
C
#ifdef _DEBUG_LVLM
      write(6,"(a)") "Entering ECP_ang_int_typ2"
      Write(6,*)
#endif
           If (.Not. Rad2_zero) 
     &     call Ecp_ang_int_typ2(Xahat, Yahat, Zahat, Xbhat, Ybhat, 
     &                           Zbhat, La, Lb, Ltot, Lamalo, Lamahi, 
     &                           Lamblo, Lambhi, Lamau, Lambu, Lprj, 
     &                           Acs_xyz, Bcs_xyz, Ang2_A, Ang2_B,
     &                           Rad2, Cint, Int, Grads,
     &                                                 
     &                           max_prims_shell)

      Enddo
      
CSSS      Call Dscal(Int, Fpi, Cint, 1) 

#ifdef _DEBUG_LVL0
      If (.Not. Rad2_zero) Then
      Write(6,*)
      Write(6,"(a)") "The Type1+2 integral"
      Write(6,"(4(1x,F20.13))")(Cint(i), i=1,Int)
      Endif
#endif
C 
      Return
      End
