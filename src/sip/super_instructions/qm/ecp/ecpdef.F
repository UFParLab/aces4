      SUBROUTINE ECPDEF(NATOMS,MTYPE,Ecpnam,CNICK)

      IMPLICIT DOUBLE PRECISION (A-H,O-Z)
c
c     NOTE : THE ATOMIC SYMBOL must PRECEED THE ECP NICK NAME
c
c     ---------------------------------------------------------------
c     MEANING OF VARIABLES :
c     ---------------------------------------------------------------
c
c     --------- INPUT -----------------------------------------------
c     MTYPE   : ATOMIC IDENTIFIER (STARTING WITH ELEMENT SYMBOL)
c     NATOMS  : NUMBER OF ATOMS
c     --------- OUTPUT ----------------------------------------------
c     IPSEUX  : PSEUDO POTENTIAL TYPE OF ATOM
c               0  MEANS : NO PSEUDO POTENTIAL OR NOT YET ASSIGNED
c     CNICK   : PSEUDO POTENTIAL NICKNAMES INDEXED BY PSEUDO POTENTIAL
c               TYPES
c
      CHARACTER*16 BLNKBN
      PARAMETER (BLNKBN = '                ')

      CHARACTER ATOSYM*2,MTYPE(NATOMS)*(*)
      CHARACTER*4 DUMPF
      CHARACTER*80 ECPNAM(NATOMS)
      CHARACTER*80 CNICK(NATOMS)
      LOGICAL bDOIT

#include "machsp.com"
#include "ecp.par"
#include "ecp.com"
c
      CALL GETREC(20,'JOBARC','NATOLD',1,NATOLD)
      CALL GETREC(20,'JOBARC','ECPNAM',80*NATOLD,ECPNAM)
c  
c first detect different types of ecp names
C
 
      DO I=1,NATOMS
         IPSEUX(I)=0
      END DO

      DO I=1,NATOMS
         CNICK(I)=BLNKBN
      END DO
C
c ECPNAM is the full list of ECP names per atom including dummy atoms
C
      NOFECP=0

      DO IAT=1, NATOLD

c loop over the atoms and remove all redundant and NONE lines

         IF ((ECPNAM(IAT).NE.BLNKBN).AND.
     &       (INDEX(ECPNAM(IAT),'NONE').EQ.0)) THEN
            bDOIT=.TRUE.
            DO II=1,NOFECP
               IF (ECPNAM(IAT).EQ.CNICK(II)) bDOIT=.FALSE.
            END DO
            IF (bDOIT) THEN
               NOFECP=NOFECP+1
               CNICK(NOFECP)=ECPNAM(IAT)
            END IF
         END IF
      END DO

      ICHECK=0
      DO IPOT=1,NOFECP
         NPOS=INDEX(CNICK(IPOT),':')

         IF (NPOS.EQ.0) THEN
            WRITE(*,*) '@ECPDEF: Error - missing seperator (:) ',
     &                 'between atom symbol and ECP nickname.'
            CALL ERREX
         END IF
c
c extract the full atomic symbol ( C != CL )
c
         DUMPF = '  '
         DUMPF = CNICK(IPOT)(1:NPOS-1)

c loop over the symmetry unique atoms and assign matches to the 
c current CNICK line number

         DO IAT=1,NATOMS

            IF (DUMPF(1:2).EQ.MTYPE(IAT)(1:2)) THEN

c bomb if two orbits have the same atom (CNICK only contains 
C unique ECP definitions)

               if (IPSEUX(IAT).ne.0) then
                  print *, '@ECPDEF: ERROR - Two orbits have the same ',
     &                     'atom and different ECP definitions.'
                  print *, '         The current implementation cannot',
     &                     ' distinguish between the two.'
                  call errex
               end if
               IPSEUX(IAT)=IPOT
               ICHECK=ICHECK+1
            END IF
         END DO
      END DO

      IF (ICHECK.EQ.0) THEN
         IF (NOFECP.EQ.0) THEN
            WRITE(*,*) '@ECPDEF: Error - no ECP nicknames found in ',
     &                 'JOBARC.'
            CALL ERREX
         END IF
         WRITE(*,*) '@ECPDEF: Error - ECP nickname and atom symbols ',
     &              'in ZMAT are incompatible.'
         CALL ERREX
      END IF

      RETURN 
      END

