      Subroutine a4_ecp_Init()

      Implicit Double Precision (A-H, O-Z)

#include "ecp.par"
#include "ecp.com"

C -Note that (PRMTESTING block is used only in the early stages of
C development and no longer supported). Ajith Perera.

#ifdef _PRMTESTING
      Dimension Ilmf(122), Ilmx(581), Ilmy(581), Ilmz(581)

      DATA ILMF/1, 2,3,4, 5,7,8,10,11, 12,14,16,18,20,22,23,
     &25,28,30,34,36,39,41,43,45, 47,50,53,57,61,64,67,70,72,76,78,
     &81,85,88,94,98,104,107,111,114,117,121,125,128,
     &131,135,139,145,151,157,163,167,171,175,178,184,188,194,197,
     &201,206,210,218,224,233,239,247,251,256,260,264,270,276,282,
     &288,292, 296,301,306,314,322,331,340,348,356,361,366,371,375,383,
     &389,398,404,412,416, 421,427,432,442,450,462,471,483,491,501,506,
     &512,517,522,530,538,547,556,564,572,577, 582/


      DATA ILMX/0, 1,0,0, 2,0,1,0,0,0,1, 3,1,2,0,1,1,0,0,0,0,1,2,0,
     &4,2,0,3,1,2,0,0,2,1,1,0,0,0,0,0,1,1,2,0,3,1, 5,3,1,0,2,4,3,1,1,3,
     &2,0,2,0,1,1,1,0,0,0,0,0,0,1,1,2,2,0,0,3,1,4,2,0, 6,4,2,0,5,3,1,4,
     &2,0,4,2,0,3,1,1,3,2,0,2,0,2,0,1,1,1,0,0,0,0,0,0,0,1,1,1,2,0,2,0,3,
     &1,3,1,4,2,0,5,3,1, 7,3,5,1,6,4,0,2,5,3,1,5,3,1,4,2,0,4,2,0,3,1,3,
     &1,3,1,2,0,2,0,2,0,1,1,1,1,0,0,0,0,0,0,0,0,1,1,1,2,0,2,0,2,0,3,1,3,
     &1,4,2,0,4,2,0,5,3,1,0,4,2,6, 8,6,4,2,0,7,5,3,1,6,4,2,0,6,4,2,0,5,
     &3,1,5,3,1,4,0,2,4,0,2,4,0,2,3,1,3,1,3,1,2,0,2,0,2,0,2,0,1,1,1,1,0,
     &0,0,0,0,0,0,0,0,1,1,1,1,2,0,2,0,2,0,3,1,3,1,3,1,4,2,0,4,2,0,5,3,1,
     &5,3,1,6,4,2,0,7,5,3,1, 9,7,5,3,1,8,6,4,2,0,7,5,3,1,7,5,3,1,6,4,2,
     &0,6,4,2,0,5,3,1,5,3,1,5,3,1,4,0,2,4,0,2,4,0,2,3,1,3,1,3,1,3,1,2,0,
     &2,0,2,0,2,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,1,1,1,1,2,0,2,0,2,0,2,0,
     &3,1,3,1,3,1,4,2,0,4,2,0,4,2,0,5,3,1,5,3,1,6,4,2,0,6,4,2,0,7,5,3,1,
     &8,6,4,2,0, 10,8,6,4,2,0,9,7,5,3,1,8,6,4,2,0,8,6,4,2,0,7,5,3,1,7,5,
     &3,1,6,4,2,0,6,4,2,0,6,4,2,0,5,3,1,5,3,1,5,3,1,4,0,2,4,0,2,4,0,2,4,
     &0,2,3,1,3,1,3,1,3,1,2,0,2,0,2,0,2,0,2,0,1,1,1,1,1,0,0,0,0,0,0,0,
     &0,0,0,0,1,1,1,1,1,2,0,2,0,2,0,2,0,3,1,3,1,3,1,3,1,0,2,4,0,2,4,0,2,
     &4,5,3,1,5,3,1,5,3,1,6,4,2,0,6,4,2,0,7,5,3,1,7,5,3,1,8,6,4,2,0,9,7,
     &5,3,1/


      DATA ILMY/0, 0,0,1, 0,2,0,0,0,1,1, 0,2,0,2,0,0,0,0,1,1,1,1,3,
     &0,2,4,0,2,0,2,2,0,0,0,0,0,0,1,1,1,1,1,3,1,3, 0,2,4,4,2,0,0,2,2,0,
     &0,2,0,2,0,0,0,0,0,0,1,1,1,1,1,1,1,3,3,1,3,1,3,5, 0,2,4,6,0,2,4,0,
     &2,4,0,2,4,0,2,2,0,0,2,0,2,0,2,0,0,0,0,0,0,0,1,1,1,1,1,1,1,3,1,3,1,
     &3,1,3,1,3,5,1,3,5, 0,4,2,6,0,2,6,4,0,2,4,0,2,4,0,2,4,0,2,4,0,2,0,
     &2,0,2,0,2,0,2,0,2,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,3,1,3,1,3,1,3,1,
     &3,1,3,5,1,3,5,1,3,5,7,3,5,1, 0,2,4,6,8,0,2,4,6,0,2,4,6,0,2,4,6,0,
     &2,4,0,2,4,0,4,2,0,4,2,0,4,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,0,0,0,0,
     &0,0,0,0,1,1,1,1,1,1,1,1,1,3,1,3,1,3,1,3,1,3,1,3,1,3,5,1,3,5,1,3,5,
     &1,3,5,1,3,5,7,1,3,5,7, 0,2,4,6,8,0,2,4,6,8,0,2,4,6,0,2,4,6,0,2,4,
     &6,0,2,4,6,0,2,4,0,2,4,0,2,4,0,4,2,0,4,2,0,4,2,0,2,0,2,0,2,0,2,0,2,
     &0,2,0,2,0,2,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,3,1,3,1,3,1,3,
     &1,3,1,3,1,3,1,3,5,1,3,5,1,3,5,1,3,5,1,3,5,1,3,5,7,1,3,5,7,1,3,5,7,
     &1,3,5,7,9, 0,2,4,6,8,10,0,2,4,6,8,0,2,4,6,8,0,2,4,6,8,0,2,4,6,0,2,
     &4,6,0,2,4,6,0,2,4,6,0,2,4,6,0,2,4,0,2,4,0,2,4,0,4,2,0,4,2,0,4,2,0,
     &4,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,0,0,0,0,0,0,0,0,0,0,1,
     &1,1,1,1,1,1,1,1,1,1,3,1,3,1,3,1,3,1,3,1,3,1,3,1,3,5,3,1,5,3,1,5,3,
     &1,1,3,5,1,3,5,1,3,5,1,3,5,7,1,3,5,7,1,3,5,7,1,3,5,7,1,3,5,7,9,1,3,
     &5,7,9/

      DATA ILMZ/0, 0,1,0, 0,0,1,2,0,1,0, 0,0,1,1,2,0,3,1,2,0,1,0,0,
     &0,0,0,1,1,2,2,0,0,3,1,4,2,0,3,1,2,0,1,1,0,0, 0,0,0,1,1,1,2,2,0,0,
     &3,3,1,1,4,2,0,5,3,1,4,2,0,3,1,2,0,2,0,1,1,0,0,0, 0,0,0,0,1,1,1,2,
     &2,2,0,0,0,3,3,1,1,4,4,2,2,0,0,5,3,1,6,4,2,0,5,3,1,4,2,0,3,3,1,1,2,
     &2,0,0,1,1,1,0,0,0, 0,0,0,0,1,1,1,1,2,2,2,0,0,0,3,3,3,1,1,1,4,4,2,
     &2,0,0,5,5,3,3,1,1,6,4,2,0,7,5,3,1,6,4,2,0,5,3,1,4,4,2,2,0,0,3,3,1,
     &1,2,2,2,0,0,0,1,1,1,0,0,0,0, 0,0,0,0,0,1,1,1,1,2,2,2,2,0,0,0,0,3,
     &3,3,1,1,1,4,4,4,2,2,2,0,0,0,5,5,3,3,1,1,6,6,4,4,2,2,0,0,7,5,3,1,8,
     &6,4,2,0,7,5,3,1,6,4,2,0,5,5,3,3,1,1,4,4,2,2,0,0,3,3,3,1,1,1,2,2,2,
     &0,0,0,1,1,1,1,0,0,0,0, 0,0,0,0,0,1,1,1,1,1,2,2,2,2,0,0,0,0,3,3,3,
     &3,1,1,1,1,4,4,4,2,2,2,0,0,0,5,5,5,3,3,3,1,1,1,6,6,4,4,2,2,0,0,7,7,
     &5,5,3,3,1,1,8,6,4,2,0,9,7,5,3,1,8,6,4,2,0,7,5,3,1,6,6,4,4,2,2,0,0,
     &5,5,3,3,1,1,4,4,4,2,2,2,0,0,0,3,3,3,1,1,1,2,2,2,2,0,0,0,0,1,1,1,1,
     &0,0,0,0,0, 0,0,0,0,0,0,1,1,1,1,1,2,2,2,2,2,0,0,0,0,0,3,3,3,3,1,1,
     &1,1,4,4,4,4,2,2,2,2,0,0,0,0,5,5,5,3,3,3,1,1,1,6,6,6,4,4,4,2,2,2,0,
     &0,0,7,7,5,5,3,3,1,1,8,8,6,6,4,4,2,2,0,0,9,7,5,3,1,10,8,6,4,2,0,9,
     &7,5,3,1,8,6,4,2,0,7,7,5,5,3,3,1,1,6,6,4,4,2,2,0,0,5,5,5,3,3,3,1,1,
     &1,4,4,4,2,2,2,0,0,0,3,3,3,3,1,1,1,1,2,2,2,2,0,0,0,0,1,1,1,1,1,0,0,
     &0,0,0/
#endif
      Pi  = Dacos(-1.0D0)
      Fpi = 4.0D0*Pi
      Sqrt_Fpi = Dsqrt(4.0D0*PI)
      Sqpi2 = Dsqrt(Pi/2.0D0)

C Set lmnval array:

      II = 0
      DO LVAL = 0,MAXANG
         DO L = LVAL,0,-1
            LEFT = LVAL - L
            DO M = LEFT,0,-1
               N = LEFT - M
               II = II + 1
               LMNVAL(1,II) = L
               LMNVAL(2,II) = M
               LMNVAL(3,II) = N
             ENDDO
          ENDDO
      ENDDO
C
#ifdef _DEBUG_LVL0
      Write(6,"(a)") "The lmn values"
      Do i=1, 3
         Write(6,"(10I3)") (lmnval(i, i1),i1=1,84)
         Write(6,*)
      Enddo
#endif
C Prepare I*(I+1)/2 values

      Ideg(0)= 1
      II = 1
      Do I = 1, Maxang
         II = II + 1
         Ideg(I) = II*(II+1)/2
      Enddo

#ifdef _DEBUG_LVL0
      Write(6,*) "I*(I+1)/2 values"
      Write(6,"(8I3)") (Ideg(I), I= 0, Maxang)
      Write(6,*)
#endif
C Prepare Istart and Iend arrays 

      Istart(0) = 1
      Istart(1) = 2
      Do I = 2, Maxang
         Istart(I) = Istart(I-1)+ Ideg(I-1)
      Enddo
C
      Iend(0) = 1
      Do I = 1, Maxang
         Iend(I) = Iend(I-1) + Ideg(I)
      Enddo
C
#ifdef _DEBUG_LVL0
      Write(6,*) "Istart values"
      Write(6,"(8I4)") (Istart(I), I= 0, Maxang)
      Write(6,*)
      Write(6,*) "Iend values"
      Write(6,"(8I4)") (IEnd(I), I= 0, Maxang)
      Write(6,*)
#endif
C Prepare I(I+1)/2 + (I-1)I/2 array for gradient

      Ideg_grd(0) = 3
      II = 1
      Do I = 1, Maxang
         II = II + 1
         Ideg_grd(I) =  (II+1)*(II+2)/2  + (II-1)*(II)/2
      Enddo

#ifdef _DEBUG_LVL0
      Write(6,*) "I*(I+1)/2 + (I-1)*I/2 (Ideg_grd) values"
      Write(6,"(8I3)") (Ideg_grd(I), I= 0, Maxang)
      Write(6,*)
#endif
C Prepare Istart and Iend arrays for gradients

      Istart_grd(0) = 1
      Istart_grd(1) = 4
      Do I = 2, Maxang
         Istart_grd(I) = Istart_grd(I-1)+ Ideg_grd(I-1)
      Enddo

      Iend_grd(0) = 3
      Do I = 1, Maxang
         Iend_grd(I) = Iend_grd(I-1) + Ideg_grd(I)
      Enddo
#ifdef _DEBUG_LVL0
      Write(6,*) "Istart_grd values"
      Write(6,"(8I4)") (Istart_grd(I), I= 0, Maxang)
      Write(6,*)
      Write(6,*) "Iend_grd values"
      Write(6,"(8I4)") (IEnd_grd(I), I= 0, Maxang)
      Write(6,*)
#endif
      do I = 1, Lmnmaxg
         do J = 1, 7
            Lmnval_grd(J,I) = 0
         enddo
      enddo 
        
      Ioff = 1
      Do Iang = 0, Maxang
         
         Do Nlm = Istart(Iang), Iend(Iang)

            If (Lmnval(1, Nlm) .Gt. 0) Then 
               Nn = Lmnval(1, Nlm) - 1
               Ll = Lmnval(2, Nlm)
               Mm = Lmnval(3, Nlm)
               Ifound = 0
               Do J = Istart_grd(Iang), Ioff -1
                  If (Lmnval_grd(1,J) .EQ. Nn .AND.
     &                Lmnval_grd(2,J) .EQ. Ll .AND.
     &                Lmnval_grd(3,J) .EQ. Mm) Ifound = J
               Enddo

               If (Ifound .NE. 0) Then
                   Lmnval_grd(4, Ifound) =  Nlm - Istart(Iang) + 1
               Else
                   Lmnval_grd(7, Ioff) = -1
                   Lmnval_grd(1, Ioff) = Nn
                   Lmnval_grd(2, Ioff) = LL
                   Lmnval_grd(3, Ioff) = Mm
                   Lmnval_grd(4, Ioff) = Nlm - Istart(Iang) + 1
                   Ioff = Ioff + 1
               Endif
            Endif

            If (Lmnval(2, Nlm) .Gt. 0) Then 
               Nn = Lmnval(1, Nlm) 
               Ll = Lmnval(2, Nlm) - 1
               Mm = Lmnval(3, Nlm)
               Ifound = 0
               Do J = Istart_grd(Iang), Ioff -1
                  If (Lmnval_grd(1,J) .EQ. Nn .AND.
     &                Lmnval_grd(2,J) .EQ. Ll .AND.
     &                Lmnval_grd(3,J) .EQ. Mm) Ifound = J
               Enddo

               If (Ifound .NE. 0) Then
                   Lmnval_grd(5, Ifound) =  Nlm - Istart(Iang) + 1
               Else
                   Lmnval_grd(7, Ioff) = -1
                   Lmnval_grd(1, Ioff) = Nn
                   Lmnval_grd(2, Ioff) = LL
                   Lmnval_grd(3, Ioff) = Mm
                   Lmnval_grd(5, Ioff) =  Nlm - Istart(Iang) + 1
                   Ioff = Ioff + 1
               Endif
           Endif

           If (Lmnval(3, Nlm) .Gt. 0) Then 
               Nn = Lmnval(1, Nlm) 
               Ll = Lmnval(2, Nlm) 
               Mm = Lmnval(3, Nlm) - 1
               Ifound = 0
               Do J = Istart_grd(Iang), Ioff -1
                  If (Lmnval_grd(1,J) .EQ. Nn .AND.
     &                Lmnval_grd(2,J) .EQ. Ll .AND.
     &                Lmnval_grd(3,J) .EQ. Mm) Ifound = J
               Enddo

               If (Ifound .NE. 0) Then
                   Lmnval_grd(6, Ifound) = Nlm - Istart(Iang) + 1
               Else
                   Lmnval_grd(7, Ioff) = -1
                   Lmnval_grd(1, Ioff) = Nn
                   Lmnval_grd(2, Ioff) = LL
                   Lmnval_grd(3, Ioff) = Mm
                   Lmnval_grd(6, Ioff) = Nlm - Istart(Iang) + 1
                   Ioff = Ioff + 1
               Endif
            Endif
C
            Nn = Lmnval(1, Nlm) + 1
            Ll = Lmnval(2, Nlm)
            Mm = Lmnval(3, Nlm)
            Ifound = 0
            Do J = Istart_grd(Iang), Ioff -1
               If (Lmnval_grd(1,J) .EQ. Nn .AND.
     &             Lmnval_grd(2,J) .EQ. Ll .AND.
     &             Lmnval_grd(3,J) .EQ. Mm) Ifound = J
            Enddo
            
            If (Ifound .NE. 0) Then
               Lmnval_grd(4, Ifound) = Nlm - Istart(Iang) + 1
            Else
               Lmnval_grd(7, Ioff) = 1
               Lmnval_grd(1, Ioff) = Nn
               Lmnval_grd(2, Ioff) = LL
               Lmnval_grd(3, Ioff) = Mm
               Lmnval_grd(4, Ioff) = Nlm - Istart(Iang) + 1
               Ioff = Ioff + 1
            Endif

            Nn = Lmnval(1, Nlm) 
            Ll = Lmnval(2, Nlm) + 1
            Mm = Lmnval(3, Nlm)
            Ifound = 0
            
            Do J = Istart_grd(Iang), Ioff -1
               If (Lmnval_grd(1,J) .EQ. Nn .AND.
     &             Lmnval_grd(2,J) .EQ. Ll .AND.
     &             Lmnval_grd(3,J) .EQ. Mm) Ifound = J
            Enddo
            If (Ifound .NE. 0) Then
               Lmnval_grd(5, Ifound) = Nlm - Istart(Iang) + 1
            Else
               Lmnval_grd(7, Ioff) = 1
               Lmnval_grd(1, Ioff) = Nn
               Lmnval_grd(2, Ioff) = LL
               Lmnval_grd(3, Ioff) = Mm
               Lmnval_grd(5, Ioff) = Nlm - Istart(Iang) + 1
               Ioff = Ioff + 1
            Endif

            Nn = Lmnval(1, Nlm) 
            Ll = Lmnval(2, Nlm)
            Mm = Lmnval(3, Nlm) + 1
            Ifound = 0
            Do J = Istart_grd(Iang), Ioff -1
               If (Lmnval_grd(1,J) .EQ. Nn .AND.
     &             Lmnval_grd(2,J) .EQ. Ll .AND.
     &             Lmnval_grd(3,J) .EQ. Mm) Ifound = J
            Enddo
   
            If (Ifound .NE. 0) Then
               Lmnval_grd(6, Ifound) = Nlm - Istart(Iang) + 1
            Else
               Lmnval_grd(7, Ioff) = 1
               Lmnval_grd(1, Ioff) = Nn
               Lmnval_grd(2, Ioff) = LL
               Lmnval_grd(3, Ioff) = Mm    
               Lmnval_grd(6, Ioff) = Nlm - Istart(Iang) + 1
               Ioff = Ioff + 1
             Endif
C
         Enddo
      Enddo
C
#ifdef _DEBUG_LVL0
      Write(6,"(a)") "Lmnval_grd array"
      do j=1,Lmnmaxg
      Write(6,"(7(1x,I4))") (Lmnval_grd(i,j), i=1, 7)
      Write(6,*)
      Enddo
#endif
      call Factorial
      call Dfactorial
      call Factorialo
      call Binomial_coefs
C
#ifdef _DEBUG_LVLM1
      Write(6,"(a)") "Double factorial array"
      Write(6,"(6(1x,F10.1))") (fac2(j1), j1=-1, 2*maxang)
      Write(6,*)

      Write(6,"(a)") "Factorial array"
      Write(6,"(6(1x,F10.1))") (fact(j1), j1=0, 11)
      Write(6,*)

      Write(6,"(a)") "Factorial_O array"
      Write(6,"(6(1x,F10.1))") (faco(j1), j1=0, 11)
      Write(6,*)

      Write(6,"(a)") "Binomial expansion Coefs. array"
      do i1 = 0, 2*Maxang
          Write(6,"(6(1x,F10.2))") (Bcoefs(i1, j1), j1=0,i1)
          Write(6,*)
      enddo
      Write(6,*)
#endif
C Prepare Zlm, real spherical harmonic table and table 
C required to do the angular integral (Ftab). See the comments
C in the header of the individual subroutines for further details.
      
      Call Make_ztab
      Call Make_ftab

C -Note that (PRMTESTING block is used only in the early stages of
C development and no longer supported). Ajith Perera.

#ifdef _PRMTESTING
C
C Initilize the Lmx, Lmy, Lmz and Lmf arrays. 
C 
      Do Lmxyz = 1, 581
         Lmx(Lmxyz) = Ilmx(Lmxyz)
         Lmy(Lmxyz) = Ilmy(Lmxyz)
         Lmz(Lmxyz) = Ilmz(Lmxyz)
      Enddo
C
      DO Imf = 1, 122
         Lmf(Imf) = Ilmf(Imf)
      Enddo
#endif

      call Factorial
      call Dfactorial
      call Factorialo
      call Binomial_coefs
C
#ifdef _DEBUG_LVLM1
      Write(6,"(a)") "Double factorial array"
      Write(6,"(6(1x,F10.1))") (fac2(j1), j1=-1, 2*maxang)
      Write(6,*)

      Write(6,"(a)") "Factorial array"
      Write(6,"(6(1x,F10.1))") (fact(j1), j1=0, 11)
      Write(6,*)

      Write(6,"(a)") "Factorial_O array"
      Write(6,"(6(1x,F10.1))") (faco(j1), j1=0, 11)
      Write(6,*)

      Write(6,"(a)") "Binomial expansion Coefs. array"
      do i1 = 0, 2*Maxang
          Write(6,"(6(1x,F10.2))") (Bcoefs(i1, j1), j1=0,i1)
          Write(6,*)
      enddo
      Write(6,*)
#endif
C Prepare Zlm, real spherical harmonic table and table 
C required to do the angular integral (Ftab). See the comments
C in the header of the individual subroutines for further details.
      
      Call Make_ztab
      Call Make_ftab

C -Note that (PRMTESTING block is used only in the early stages of
C development and no longer supported). Ajith Perera.

#ifdef _PRMTESTING
C
C Initilize the Lmx, Lmy, Lmz and Lmf arrays. 
C 
      Do Lmxyz = 1, 581
         Lmx(Lmxyz) = Ilmx(Lmxyz)
         Lmy(Lmxyz) = Ilmy(Lmxyz)
         Lmz(Lmxyz) = Ilmz(Lmxyz)
      Enddo
C
      DO Imf = 1, 122
         Lmf(Imf) = Ilmf(Imf)
      Enddo
#endif
#ifdef _DEBUG_LVLM1
      Write(6,"(a)") "The ZLM array"
      Write(6,"(6(1x,F15.13))") (zlm(j1), j1=1, Lmnpwr)
      Write(6,*)
#endif

      Return
      End












