      subroutine Ecp_Grdint_4prim(Xa, Ya, Za, Xb, Yb, Zb, La, Lb, 
     &                            Coord, Exp1, Exp2, Natoms, Ntotatoms, 
     &                            Cint, Int, Zero_int, Icnt, Iecp_cnt, 
     &                            Grads,
     &                  
     &                            nshells,max_ecp_prims,
     &                            necp_atoms,max_necp_functns,
     &                            nval_prims,nval_ccoeffs,
     &                            max_unfns_shell,max_cofns_shell,
     &                            nval_bfns,max_nprims,max_ncontfns,
     &
     &                            clp,zlp,nlp,kfirst,klast,llmax,
     &                            nelecp,ipseux,ipseud,
     &
     &                            numcon,katom,ktype,kprim,kmini,
     &                            kmaxi,nprims,ndegen)
C
C (XA,YA,ZA) and (XB,YB.ZB) are the coordintes of center A and B.
C LA and LB are the angular momentums of gaussians on A and B (0,...n)
C Cont1 and Cont1 are the contractions coefficients for gaussian A and B.
C Exp1 and Exp1 are the exponents of gaussian A and B
C
      Implicit Double Precision (A-H, O-Z)

#include "ecp.par"
#include "ecp.com"

      Dimension Coord(3,Ntotatoms), ACS_xyz(0:2*Maxang,3),
     &          BCS_xyz(0:2*Maxang,3), 
     &          Cint(max_unfns_shell * max_unfns_shell)

      Logical CA_zero, CB_zero, PC_zero, Zero_int, Rad1_zero,
     &        Rad2_zero, Grads, Zero_int_cnt(Ntotatoms)
C
      integer ipseud,nelecp(ntotatoms),ipseux(ntotatoms)
      integer llmax(ntotatoms),nlp(max_ecp_prims)
      integer kfirst(necp_atoms,max_necp_functns)
      integer klast(necp_atoms,max_necp_functns)
      double precision clp(max_ecp_prims),zlp(max_ecp_prims)
C
      integer numcon(nshells),numpri(nshells)
      integer katom(nshells),ktype(nshells)
      integer kmini(nshells),kmaxi(nshells)
      integer ndegen(nshells)
      integer nprims(nshells),kprim(nshells)
C
#ifdef _DEBUG_LVL0
      Write(6,*) "ECP Paramters: Kfirst and Klast"
      Write(6,"(6(1x,I2))") ((Kfirst(i,j),i=1,Maxang),
     &                                    J=1,Natoms)
      Write(6,"(6(1x,I2))") ((Klast(i,j),i=1,Maxang),
     &                                    J=1,Natoms)
#endif
C 
C Loop over all the ECP centers and compute the integral 
C for each pair of  primitives

      Ltot = La + Lb + 1
      Lap = La + 1

#ifdef _DEBUG_LVL0
C      Natoms = 1
      Write(6,"(a,I3)") "The Debug value for No. atoms: ", Natoms
      Write(6,"(a,2I3)") "The Debug value Icnt,Iecp_cnt: ", Icnt,
     &                    Iecp_cnt
#endif
      Xc = Coord(1, Icnt)
      Yc = Coord(2, Icnt)
      Zc = Coord(3, Icnt)

#ifdef _DEBUG_LVLM
      Write(6,*) "Initialize debug xyz of C"
      xc=2.0
      yc=2.0
      zc=2.0
      Write(6, "(a, 3(1x,F10.6))") "The XYZ of C : ", Xc, yc, Zc
#endif
C
C Build CA anb CB,(= C-A, C-B) vector components
C
      Xca = Xc - Xa
      Yca = Yc - Ya
      Zca = Zc - Za
      Ca2 = Xca**2 + Yca**2 + Zca**2

      if (Ca2 .Gt. 0.0D0) Then
         Ca = Dsqrt(Ca2)
         Lamau = Lap
         Xahat = -Xca/Ca
         Yahat = -Yca/Ca
         Zahat = -Zca/Ca
         CA_zero = .False.
      Else 
         Ca = 0.0D0
         Lamau = 0
         Xahat = 0
         Yahat = 0
         Zahat = 0
         CA_zero = .True.
      Endif
C
      Xcb = Xc - Xb
      Ycb = Yc - Yb
      Zcb = Zc - Zb
      Cb2 = Xcb**2 + Ycb**2 + Zcb**2

      if (Cb2 .Gt. 0.0D0) Then
          Cb = Dsqrt(Cb2)
          Lambu = Lb
          Xbhat = -Xcb/Cb
          Ybhat = -Ycb/Cb
          Zbhat = -Zcb/Cb
          CB_zero = .False.
      Else
          Cb = 0.0D0
          Lambu = 0
          Xbhat = 0
          Ybhat = 0
          Zbhat = 0
          CB_zero = .True.
      Endif
C
C Build PIn_i=n,,k_i=0...n_i}CA_i^^(n_i-k_i) i=x,y,z
C

      Call Make_xyz(Lap, Xca, ACS_xyz(0,1))
      Call Make_xyz(Lap, Yca, ACS_xyz(0,2))
      Call Make_xyz(Lap, Zca, ACS_xyz(0,3))

      Call Make_xyz(Lb, Xcb, BCS_xyz(0,1)) 
      Call Make_xyz(Lb, Ycb, BCS_xyz(0,2))
      Call Make_xyz(Lb, Zcb, BCS_xyz(0,3))

#ifdef _DEBUG_LVL0
      Write(6,*)
      Write(6,"(a)") "ACS_xyz array from make_xyz"
      Write(6,"(3F10.6)") ((ACS_xyz(i,j), j=1, 3),i=0,la)

      Write(6,"(a)") "ACS_xyz array from make_xyz"
      Write(6,"(3F10.6)") ((BCS_xyz(i,j), j=1, 3),i=0,la)
#endif

C
C Get the common center of gaussian on A and B and the new
C multification factor (Fact_AB).
C
      Call Get_center(Xa, Ya, Za, Xb, Yb, Zb, Exp1, Exp2,
     &                Xp, Yp, Zp, Exp12, Fact_Ab)

      Xpc = Xp - Xc
      Ypc = Yp - Yc
      Zpc = Zp - Zc
       
      Pc2 = Xpc**2 + Ypc**2 + Zpc**2

      If (Pc2 .Gt. 0.0D0) Then
         Pc    = Dsqrt(Pc2)
         Lamu  = Ltot
         Xhat  = Xpc/pc
         Yhat  = Ypc/pc
         Zhat  = Zpc/pc
         Beta  = 2.0D0*Exp12*PC
         PC_Zero = .False.
      Else 
          Pc    = 0.0D0
          Lamu  = 0
          Beta  = 0.0D0 
          Xhat  = 0.0D0
          Yhat  = 0.0D0
          ZhAt  = 0.0D0
          PC_Zero = .True.
      Endif

#ifdef _DEBUG_LVL0
      Write(6,*) 
      Write(6,"(a)") "At entry to Ecp_int_typ12"
      Write(6,"(a,4(1x,I2))" ) "La, Lb, Lamu and Ltot: ", La, Lb, 
     &                          Lamu, Ltot

      Write(6,"(a,3(1x,F10.6))" ) "Xc,Yc,Zc, Xhat,Yhat,Zhat: ", 
     &                             Xc,Yc,Zc
      Write(6,"(a,3(1x,F10.6))" ) "                          ",
     &                             Xhat,Yhat,Zhat
      Write(6,"(a,4(1x,F10.6))" ) "Exp12,Beta,Pc2,Fact_ab:   ", 
     &                             Exp12,Beta,Pc2,Fact_ab
      Write(6,"(a,6(1x,F10.6))" ) "Xah,Yah,Zah,Xbh,Ybh,Zbh:  ",
     &                             Xahat,Yahat,Zahat
      Write(6,"(a,3(1x,F10.6))" ) "                          ",
     &                             Xbhat,Ybhat,Zbhat
      Write(6,*) 
      Write(6,"(a)") "Entering Ecp_int_typ1" 
#endif
      Call Ecp_int_typ1(Iecp_cnt, Lamu, Ltot, La, Lb, Exp12, 
     &                  Xhat, Yhat, Zhat, Xp, Yp, Zp, Xc, Yc,
     &                  Zc, Acs_xyz, Bcs_xyz, Pc2, Fact_Ab,
     &                  PC_zero, Cint, Int, Rad1_zero, Grads,
     &
     &                  nshells,max_ecp_prims,
     &                  necp_atoms,max_necp_functns,
     &                  nval_prims,nval_ccoeffs,
     &                  max_unfns_shell,max_cofns_shell,
     &                  nval_bfns,max_nprims,max_ncontfns,
     &                  ntotatoms,
     &
     &                  clp,zlp,nlp,kfirst,klast,llmax,
     &                  nelecp,ipseux,ipseud,
     &
     &                  numcon,katom,ktype,kprim,kmini,
     &                  kmaxi,nprims,ndegen)

#ifdef _DEBUG_LVL0
           Write(6,*) 
           Write(6,"(a)") "Entering Ecp_int_typ2"
#endif
      Call Ecp_int_typ2(Iecp_cnt, Lamau, Lambu, La, Lb, 
     &                  Exp1, Exp2, EXp12, Xahat, Yahat, Zahat, 
     &                  Xbhat, Ybhat, Zbhat, Xp, Yp, Zp, Xc, Yc,
     &                  Zc, Acs_xyz, Bcs_xyz, Ca, Cb, Pc2, 
     &                  Fact_Ab, CA_Zero, CB_Zero, Cint, 
     &                  Rad2_zero, Grads,
     &
     &                  nshells,max_ecp_prims,
     &                  necp_atoms,max_necp_functns,
     &                  nval_prims,nval_ccoeffs,
     &                  max_unfns_shell,max_cofns_shell,
     &                  nval_bfns,max_nprims,max_ncontfns,
     &                  ntotatoms,
     &
     &                  clp,zlp,nlp,kfirst,klast,llmax,
     &                  nelecp,ipseux,ipseud,
     &
     &                  numcon,katom,ktype,kprim,kmini,
     &                  kmaxi,nprims,ndegen)


      Zero_int_cnt(Icnt) = Rad1_zero .AND. Rad2_zero 

#ifdef _DEBUG_LVL0
           Write(6,*) "Test for Zero integrals"
           Write(6,*) Zero_int_cnt(Icnt), Rad1_zero, Rad2_zero 
#endif

      Zero_int = .True. 
      Do Kcnt = 1, Natoms
         If (.NOT. Zero_int_cnt(Kcnt)) Then
             Zero_int = .False.
             Return
         Endif
      Enddo 
C
C Do the contractions. 
C
      Return
      End
