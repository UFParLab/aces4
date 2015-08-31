
C THIS ROUTINE SHOULD BE CALLED WHEN THERE IS INSUFFICIENT CORE TO
C CONTINUE THE CALCULATION.  PRINTS OUT A FATAL ERROR MESSAGE AND
C CALLS THE CRAPS EXIT HANDLER.
C
C INPUT:
C    ROUTINE - ROUTINE FROM WHICH INSMEM IS CALLED [CHARACTER*(*)]
C    INEED   - AMOUNT OF MEMORY REQUIRED.
C    IAVAIL  - AMOUNT OF MEMORY AVAILABLE.

      SUBROUTINE INSMEM(ROUTINE,INEED,IAVAIL)
      INTEGER INEED,IAVAIL
      CHARACTER*(*) ROUTINE

      double precision dneed, davail
      character*2 units(6)
      data units /' B','kB','MB','GB','TB','PB'/
      COMMON /MACHSP/ IINTLN,IFLTLN,IINTFP,IALONE,IBITWD

      dneed = 1.d0*ineed*iintln
      i = 1
      do while (dneed.gt.1000.d0)
         i = i+1
         dneed = dneed/1024.d0
      end do
      davail = 1.d0*iavail*iintln
      j = 1
      do while (davail.gt.1000.d0)
         j = j+1
         davail = davail/1024.d0
      end do

      WRITE(*,100) ROUTINE(1:linblnk(ROUTINE)),
     &             dneed,units(i),davail,units(j)
100   FORMAT(T3,'@',A,': Insufficient memory available.',/,
     &       T3,'  needed: ',F5.1,1x,A,', available: ',F5.1,1x,A,'.')
      CALL ERREX
      RETURN
      END
