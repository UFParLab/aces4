      Subroutine Ecp_ang_int_typ1(Xhat, Yhat, Zhat, La, Lb, Ltot, 
     &                            Lamu, ACS_xyz, BCS_xyz, Rad, Ang,
     &                            Cint, Int, Grads,
     &
     &                            max_prims_shell)
C
C See the calling routine for meaning of input variables.
C
      Implicit Double Precision (A-H, O-Z)
      Logical Skip, Grads

#include "ecp.par"
#include "ecp.com"

      Dimension ACS_xyz(0:2*Maxang,3),BCS_xyz(0:2*Maxang,3),
     &          Sum1_xyz(0:2*Maxang,3),Ang(0:2*Maxang,0:2*Maxang),
     &          Rad(0:2*Maxang,0:2*Maxang)
      Dimension Cint(max_prims_shell * max_prims_shell)

      If (Grads) Then
          La_lo = Istart_grd(La)
          La_hi = Iend_grd(La)

          Lb_lo = Istart(Lb)
          Lb_hi = Iend(Lb)
      Else
          La_lo = Istart(La)
          La_hi = Iend(La)

          Lb_lo = Istart(Lb)
          Lb_hi = Iend(Lb)
      Endif
C
      Int   = 0
 
C      La_Lo = 3
C      La_hi = 3
C      Lb_lo = 3
C      Lb_hi = 3

      Do Ia = La_lo, La_hi

         If (Grads) Then
            Nax = Lmnval_grd(1, Ia) 
            Nay = Lmnval_grd(2, Ia)
            Naz = Lmnval_grd(3, Ia) 
         Else
            Nax = Lmnval(1, Ia) 
            Nay = Lmnval(2, Ia)
            Naz = Lmnval(3, Ia) 
         Endif

         Do Ib = Lb_lo, Lb_hi
 
            Nbx = Lmnval(1, Ib) 
            Nby = Lmnval(2, Ib)
            Nbz = Lmnval(3, Ib) 

            Int = Int + 1
            Sum = 0.0D0

            Call make_BC_CACB_xyz(Nax, Nbx, ACS_xyz(0,1), 
     &                            BCS_xyz(0,1), Sum1_xyz(0,1))
            Call make_BC_CACB_xyz(Nay, Nby, ACS_xyz(0,2), 
     &                            BCS_xyz(0,2), Sum1_xyz(0,2))
            Call make_BC_CACB_xyz(Naz, Nbz, ACS_xyz(0,3), 
     &                            BCS_xyz(0,3), Sum1_xyz(0,3))
C
#ifdef _DEBUG_LVLM1
      Write(6,*) "The SUm1_xyz array"
      do j=1, 3
         Write(6,*) "The J value :", J
         if (j .eq. 1) k=Nax
         if (j .eq. 2) k=Nay
         if (j .eq. 3) k=Naz
         Write(6,"(6(1x,F10.5))") (Sum1_xyz(i,j), i=0,k)
      enddo
#endif
#ifdef _DEBUG_LVL0
      Write(6,*)
      Write(6,"(a,6(1x,I3))") "Naxyz and Nbxyz", Nax, Nay, Naz,
     &                         Nbx, Nby, Nbz
      Write(6,*)
#endif
C
           Call DZero(Ang(0,0), (2*Maxang+1)**2)
C
C Now we have loops over Nax, Nay, Naz, Nbx, Nby, Nbz
C
            Do iabx = 0, Nax+Nbx
               Do iaby = 0, Nay+Nby
                  Do iabz = 0, Naz+Nbz
   
                     Iabxyz = iabx+iaby+iabz
C
C Loops over lamda (0:la+lb) amd mu (-Lambd:+Lamda)
C
                     Do Lam = 0, Lamu
C
                        Ang_N_lam = 0.0D0

                        Do Mu = -Lam, Lam
C
C The non-zero angular integral occurs for Lam < iabxyz and Lam+iabxyz
C even
                           If (Lam .LE. Iabxyz .and. Mod(Lam+Iabxyz,2) 
     &                         .NE. 1) Then
                               
                               Call Screen_ang_typ1(0, 0, Iabx, Iaby, 
     &                                              Iabz, Lam, Mu, 
     &                                              Skip)
                               IF (.Not. Skip) Then
                                  Call Make_res(Lam, Mu, Xhat, Yhat,
     &                                          Zhat, Res)
                                  Call Ang_int(0, 0, Iabx, Iaby, Iabz,
     &                                         Lam, Mu, Ang_N_lam_mu)

#ifdef _DEBUG_LVLM
      Write(6,"(a,3(1x,F15.8))") "Before Sum:", Ang_N_lam,
     &                            ANg_N_lam_mu, Res
#endif
                                  Ang_N_lam = Ang_N_lam +
     &                                        Ang_N_lam_mu*
     &                                        Res*Sqrt_fpi
C
#ifdef _DEBUG_LVLM
      Write(6,"(a)") "The angular part of the ints in Main"
      Write(6,"(5(1x,i2),(2(1x,F15.8)))"),Lam,mu,Iabx,Iaby,Iabz,
     &                                 Ang_N_lam
      Write(6,*)
#endif
                               Endif
c
                           Endif
C
                        Enddo

                        If (Lam .LE. Iabxyz .and. Mod(Lam+Iabxyz,2)
     &                         .NE. 1) Then

                           Ang(Iabxyz, Lam) = Ang(Iabxyz, Lam) + 
     &                                        Ang_N_lam*
     &                                        Sum1_xyz(Iabx,1)*
     &                                        Sum1_xyz(Iaby,2)*
     &                                        Sum1_xyz(Iabz,3)
#ifdef _DEBUG_LVLM
      Write(6,"(a,2(1x,i2),1x,F15.8)")"The angular int:",Iabxyz,Lam,
     &                                 Ang(Iabxyz, Lam)
      Write(6,*)
#endif
                        Endif

                     Enddo
C
                  Enddo
               Enddo
            Enddo               
C
            Do Lam = 0, Lamu
               Nhi = Ltot - Mod(Ltot-Lam,2)
               Do N = Lam, Nhi, 2

                 Sum = Sum +  Ang(N,Lam)*Rad(N, Lam)

#ifdef _DEBUG_LVL0
      Write(6,"(a,2(1x,I3),3(1x,F15.13))") "Summing Rad and Ang 4N,L: ",
     &         N, Lam, Ang(N,Lam),Rad(N,Lam), Sum
#endif
               Enddo
            Enddo

            Cint(Int) = Cint(Int) +  Sum
#ifdef _DEBUG_LVL0
       Write(6,*)
       Write(6,"(a,1x,(F18.13))") "@-ang_typ1, CINT: ", CINT(int)
       Write(6,*)
#endif 
C
         Enddo
      Enddo

      Return
      End
            
