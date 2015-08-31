C
      SUBROUTINE Get_center(Ax,Ay,Az,Bx,By,Bz,ALPHA,BETA,Px,Py,Pz,
     &                      EExp, Fact_AB)

C THIS COMPUTES THE CENTRE, EXPONENT, AND MULTIPLYING FACTOR OF
C A SINGLE GAUSSIAN WHICH CAN REPLACE THE PRODUCT OF TWO GAUSSIA
C CENTRES A AND B, AND EXPONENTS ALPHA AND BETA.

      IMPLICIT DOUBLE PRECISION (A-H,O-Z)

      IF (ALPHA .EQ. 0.D0 .AND. BETA .EQ. 0.0D0) Then
          Write(6,"(a,a)") " @-Get_center: Both exponents",
     &                      " can not be zero" 
          Call Abort_job()
      Endif

      EEXP=ALPHA+BETA

      T = 1.0D0/EEXP

      Px = (ALPHA*Ax + BETA*Bx)*T
      Py = (ALPHA*Ay + BETA*By)*T
      Pz = (ALPHA*Az + BETA*Bz)*T
      
      ABSQRD  = (AX-BX)**2 + (Ay-By)**2 + (Az-Bz)**2
      Fact_AB = (ALPHA*BETA/EEXp)*ABSQRD
      
      RETURN
      END
