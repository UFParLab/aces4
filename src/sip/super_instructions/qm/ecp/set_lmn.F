      Subroutine Set_lmn

      Implicit Integer (A-H, O-Z)

#include "ecp.par"
#include "ecp.com"
C
      II = 0
      DO LVAL = 0,MAXANG-1
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
c     
      Return
      End
