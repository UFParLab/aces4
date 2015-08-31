      Subroutine Qcomp_38D(Qnl, Nlq, Ltot, Talpha)
C
      Implicit Double Precision(A-H, O-Z)
C
#include "ecp.par"
#include "ecp.com"
C
      Dimension Qnl(0:2*Maxang, 0:2*Maxang)
C
C Using 38D of MD paper (Note the error in MD paper)
C
#ifdef _DEBUG_LVLM1
      Write(6,*)
      Write(6,*) "The Qnl at entry to Qcomp_38D"
      Do i=0, 2*Maxang
         Write(6,"(4(1x,F10.7))") (Qnl(j,I), j=0, 2*Maxang)
      Enddo
      Write(6,*)
#endif

      If (Nlq .EQ. 1) Then

         Lmax = Ltot
         Do NN = 0, Ltot-2, 2
            Lmax = Lmax - 2

            Do L = 0, Lmax

                N = L + 2 + NN
                NLM = N + Nlq + L - 1
#ifdef _DEBUG_LVLM1
                Write(6,"(a,3(1x,i3),2(1x,F10.6))") 
     &                "in Qcomp_38D-3", N, L, NLM, Qnl(N-2,L),
     &                 Qnl(N-1,L+1)
#endif
                Qnl(N,L) = ((NLM)*Qnl(N-2,L) + Beta*Qnl(N-1,L+1))/
     &                       Talpha

            Enddo
         Enddo

      Else

         NP_Lo = Mod(Nlq-3,2) + 4
         NP_Hi = Nlq + 1
         Ifac  = 3
#ifdef _DEBUG_LVLM1
         Write(6,"(a,3(1x,i3))") "in Qcomp 38D NP_LO,HI,LTOT", 
     &             NP_LO, NP_hi, ltot
#endif
         Do NP_D = NP_Lo, NP_hi, 2
            Do LL = 2, Ltot+1
C
C N = LL (Note that what we are doing here is (Qnl(0+2+2..) where
C Qnl(0+2) is stored in Qnl(0,0)
C
               N = LL - 2
               L = LL - 2
               NLM = N + L + Ifac
#ifdef _DEBUG_LVLM1
               Write(6,"(a,3(1x,i3),2(1x,F10.6))") 
     &               "In Qcomp_38D-1", N, L, NLM, Qnl(N,L),
     &                Qnl(N+1,L+1)
#endif
               Qnl(N,L) = (NLM*Qnl(N,L) + Beta*Qnl(N+1,L+1))/Talpha

            Enddo

C N = Ltot + 1
               N = Ltot
               L = Ltot
               NLM = N + L + 3
#ifdef _DEBUG_LVLM1
               Write(6,"(a,3(1x,i3),2(1x,F10.6))") 
     &              "In Qcomp_38D-2", N, L, NLM, Qnl(N,L),
     &               Qnl(N-1,L-1)
#endif
               Qnl(N,L) = (2*Qnl(N,L) + Beta*Qnl(N-1,L-1))/Talpha
               Ifac = Ifac + 2
         Enddo

         Lmax = Ltot
         Do NN = 0, Ltot-2, 2
            Lmax = Lmax - 2

            Do L = 0, Lmax

                N = L + 2 + NN
                NLM = N + Nlq + L - 1
#ifdef _DEBUG_LVLM1
                Write(6,"(a,3(1x,i3),2(1x,F10.6))") 
     &               "in Qcomp_38D-3", N, L, NLM, Qnl(N-2,L),
     &                Qnl(N-1,L+1)
#endif
                Qnl(N,L) = ((NLM)*Qnl(N-2,L) + Beta*Qnl(N-1,L+1))/
     &                       Talpha

            Enddo
         Enddo

C
      Endif

      Return
      End
