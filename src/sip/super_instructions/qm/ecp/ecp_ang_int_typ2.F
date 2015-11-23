      Subroutine Ecp_ang_int_typ2(Xahat, Yahat, Zahat, Xbhat, Ybhat, 
     &                            Zbhat, La, Lb, Ltot, Lamalo, Lamahi,
     &                            Lamblo, Lambhi, Lamau, Lambu, Lprj,
     &                            ACS_xyz, BCS_xyz, Ang2_A, Ang2_B,
     &                            Rad2, Cint, Int, Grads,
     &
     &                            max_prims_shell)
C
C See the calling routine for meaning of input variables.
C
      Implicit Double Precision (A-H, O-Z)
      Logical Grads

#include "ecp.par"
#include "ecp.com"

      Dimension ACS_xyz(0:2*Maxang,3),BCS_xyz(0:2*Maxang,3),
     &          Ang2_A(0:2*Maxang,-Maxproj:Maxproj,0:2*Maxang),
     &          Ang2_B(0:2*Maxang,-Maxproj:Maxproj,0:2*Maxang),
     &          Rad2(0:2*Maxang,0:2*Maxang, 0:2*Maxang)
      Dimension Cint(max_prims_shell * max_prims_shell) 

      If (Grads) Then
         La_lo = Istart_grd(La)
         La_hi = Iend_grd(La)
           Lap = La + 1

         Lb_lo = Istart(Lb)
         Lb_hi = Iend(Lb)
      Else
         La_lo = Istart(La)
         La_hi = Iend(La)
           Lap = La

         Lb_lo = Istart(Lb)
         Lb_hi = Iend(Lb)
      Endif

      Int   = 0

C      La_Lo = 6
C      La_hi = 6
C      Lb_lo = 13
C      Lb_hi = 13

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

#ifdef _DEBUG_LVLM
      write(6,"(a)") "Entering ECP_ang_int2-1: Nax, Nay, Naz"
      Write(6,"(1x,3I)") Nax, Nay, Naz
#endif
         Call Ecp_ang_int2(Xahat, Yahat, Zahat, Lamau, Lap, 
     &                     Nax, Nay, Naz, Lprj, ACS_xyz, Ang2_A)

         Do Ib = Lb_lo, Lb_hi
 
            Nbx = Lmnval(1, Ib) 
            Nby = Lmnval(2, Ib)
            Nbz = Lmnval(3, Ib) 

            Sum = 0.0D0
            Int = Int + 1

#ifdef _DEBUG_LVLM
      write(6,"(a)") "Entering ECP_ang_int2-2"
      Write(6,*)
#endif
             Call Ecp_ang_int2(Xbhat, Ybhat, Zbhat, Lambu, Lb, 
     &                         Nbx, Nby, Nbz, Lprj, BCS_xyz, Ang2_B)

             Do Lama = Lamalo, Lamahi
         
                Na_min = Iabs(Lprj-Lama)
                Na_max  = Lap - Mod(Lap-Na_min,2) 
            
                Do Lamb = Lamblo, Lambhi
            
                   Nb_min = Iabs(Lprj-Lamb)
                   Nb_max = Lb - MOd(Lb-Nb_min,2)
               
                   Nlo = Na_min + Nb_min
                   Nhi = Na_max + Nb_max

                   Do N = Nlo, Nhi, 2
                      
                      Nlmlo = Max(Na_min, N-Nb_Max)
                      Nlmhi = Min(Na_max, N-Nb_Min)
                      
                      Ang = 0.0D0

                      Do M = -Lprj, Lprj
                         
                         Do Nlm = Nlmlo, Nlmhi

                            Ang = Ang + Ang2_A(Nlm, M, Lama)*
     &                                  Ang2_B(N-Nlm, M, Lamb)
                           
#ifdef _DEBUG_LVLM
      Write(6,"(a,5(1x,I3),1x,3(F18.13))")
     &         "Summing Ang1 and Ang2:N,M,L: ",
     &          Nlm, (N-Nlm), M, Lama,Lamb,
     &          ang2_a(nlm,m,lama), ang2_b(n-nlm,m,lamb), Ang
#endif

                         Enddo
                      Enddo

                      Sum = Sum + Ang*Rad2(N, Lamb, Lama)
#ifdef _DEBUG_LVLM
      Write(6,*)
      Write(6,"(a,3(1x,I3),1x,3(F18.13))")
     &         "Summing Rad and Ang:N,M,L: ",
     &          N,  Lama,Lamb,
     &         Rad2(N,Lamb,Lama), Ang, Sum
#endif

                   Enddo
                Enddo
             Enddo 
C
#ifdef _DEBUG_LVLM
         Write(6,*)
         Write(6,"(a,3(1x,i3))") "Ia,Ib,int; ", ia, ib, int
         Write(6,"(a,1x,i3,1x,(F18.13))") "@-ang_typ2,CINT: ", 
     &         int,CINT(int) 
         Write(6,*)
#endif

         Cint(Int) = Cint(Int) + Sum

#ifdef _DEBUG_LVL0
         Write(6,*)
         Write(6,"(a,3(1x,i3))") "Ia,Ib,int; ", ia, ib, int
         Write(6,"(a,1x,i3,1x,(F18.13))") "@-ang_typ2,CINT: ", 
     &         int,CINT(int) 
         Write(6,*)
#endif
C
         Enddo
      Enddo

      Return
      End
            
