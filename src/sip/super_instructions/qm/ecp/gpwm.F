      Subroutine Gpwm(Nlq, Lprj, La, Lb, Ltot, Lamalo, Lamahi, Lamblo, 
     &                Lambhi, Lamau, Lambu, Alpha, Rc, Rc2, Prd, Beta1, 
     &                Beta2, Rad2)
      Implicit Double Precision (A-H, O-Z) 

#include "ecp.par"
      
      Dimension Rad2(0:2*Maxang,0:2*Maxang,0:2*maxang), Z(35), W(35),
     &          Bessa(20,0:Maxang), Bessb(20,0:Maxang), 
     &          Pntw(20,0:Maxang)

C
      data              z /-.20201828704561d+01,-.95857246461382d+00,
     & .00000000000000d+00, .95857246461382d+00, .20201828704561d+01,
     &                     -.34361591188377d+01,-.25327316742328d+01,
     &-.17566836492999d+01,-.10366108297895d+01,-.34290132722370d+00,
     & .34290132722370d+00, .10366108297895d+01, .17566836492999d+01,
     & .25327316742328d+01, .34361591188377d+01,
     &                     -.53874808900112d+01,-.46036824495507d+01,
     &-.39447640401156d+01,-.33478545673832d+01,-.27888060584281d+01,
     &-.22549740020893d+01,-.17385377121166d+01,-.12340762153953d+01,
     &-.73747372854539d+00,-.24534070830090d+00, .24534070830090d+00,
     & .73747372854539d+00, .12340762153953d+01, .17385377121166d+01,
     & .22549740020893d+01, .27888060584281d+01, .33478545673832d+01,
     & .39447640401156d+01, .46036824495507d+01, .53874808900112d+01/
      data              w / .19953242059046d-01, .39361932315224d+00,
     & .94530872048294d+00, .39361932315224d+00, .19953242059046d-01,
     &                      .76404328552326d-05, .13436457467812d-02,
     & .33874394455481d-01, .24013861108231d+00, .61086263373533d+00,
     & .61086263373533d+00, .24013861108231d+00, .33874394455481d-01,
     & .13436457467812d-02, .76404328552326d-05, 
     &                      .22293936455342d-12, .43993409922732d-09,
     & .10860693707693d-06, .78025564785321d-05, .22833863601635d-03,
     & .32437733422379d-02, .24810520887464d-01, .10901720602002d+00,
     & .28667550536283d+00, .46224366960061d+00, .46224366960061d+00,
     & .28667550536283d+00, .10901720602002d+00, .24810520887464d-01,
     & .32437733422379d-02, .22833863601635d-03, .78025564785321d-05,
     & .10860693707693d-06, .43993409922732d-09, .22293936455342d-12/

#ifdef _DEBUG_LVL0
      Write(6,"(a,9(1x,I3))") "Nlp,lprj,La,Lb,ltot,lamalo,lamahi,
     &lamblo,lambhi: ",Nlq,lprj,La,Lb,ltot,lamalo,lamahi,lamblo,
     &                lambhi
      Write(6,*)
#endif 
      TRc2  = Rc2

#ifdef _DEBUG_LVLM
      Trc2 = 5000
      Write(6,"(a)") "Setting Trc2 in Gpmw for Debugging its options"
#endif

      If (Trc2 .Gt. 5.0D4) Then
         Npnts = 5
         Idifs = 0 
      Else  if (Trc2 .Gt. 5.0D2) Then
         Npnts = 10
         Idifs = 5
      Else
         Npnts = 20
         Idifs = 15
      Endif

      Sqalpha = Dsqrt(Alpha)
      Rcalpha = Rc
      Prd     = Prd/Sqalpha
#ifdef _DEBUG_LVLM
      Prd = 1.0D0
      Write(6,*)
      Write(6,"(a,I3,1x,2F15.13)") "Npints, Prd in ptwt", Npnts, prd,
     &                              Rcalpha
#endif

      Do Ipnts = 1,  Npnts
        
         Pnt = Z(Ipnts + Idifs)/Sqalpha + Rcalpha 
         Do Lama = Lamalo, Lamahi
            Bessa(Ipnts, Lama) = Bess(Beta1*Pnt, Lama)

#ifdef _DEBUG_LVLM1
            Write(6,"((1x,F15.13,i3))")Bessa(Ipnts, Lama),lama
#endif
         Enddo
        
         
         Do Lamb = Lamblo, Lambhi
            Bessb(Ipnts, Lamb) = Bess(Beta2*Pnt, Lamb)
#ifdef _DEBUG_LVLM1
            Write(6,"((1x,F15.13,i3))")Bessb(Ipnts, Lamb),lamb
#endif
         Enddo

         If (Nlq .Gt. 0) Then
   
            Pntw(Ipnts, 0) = Prd*pnt**Nlq

            Do L = 1, Ltot
        
               Pntw(Ipnts, L) = pnt*Pntw(Ipnts, L-1)

            Enddo

         Else

            Pntw(Ipnts, 0) = Prd

            Do L = 1, Ltot
        
               Pntw(Ipnts, L) = pnt*Pntw(Ipnts, L-1)

            Enddo

         Endif
       
         Do Lama = Lamalo, Lamahi

            Na_min = Iabs(Lprj-Lama) 

            Do Lamb = Lamblo, Lambhi

               Nb_min = Iabs(Lprj-Lamb)

               Nlo = Na_min + Nb_min
               Nhi = (Ltot - Mod(La-Na_min,2)) - Mod(Lb-Nb_min,2)

               Do N = Nlo, Nhi, 2
#ifdef _DEBUG_LVLM1
                   Write(6,"(4(1x,F15.13))") W(Ipnts+Idifs),
     &                        Bessa(Ipnts, Lama),
     &                        Bessb(Ipnts, Lamb),  Pntw(Ipnts, N)
#endif


                   Rad2(N, Lamb, Lama) = Rad2(N, Lamb, Lama) + 
     &                                   ((W(Ipnts+Idifs)*
     &                                    Bessa(Ipnts, Lama))*
     &                                    Bessb(Ipnts, Lamb))*
     &                                    Pntw(Ipnts, N)
               Enddo
            Enddo
         Enddo

      Enddo

      Return
      End

