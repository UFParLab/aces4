      subroutine chrges(CHARGE,mtype,nmax,ierr)
c
c     -----  assign default atomic symbol to nuclear charge  -----
c
      implicit double precision (a-h,o-z)

      parameter (mxchg = 95)

      dimension CHARGE(Nmax)
      character*2 mtype(nmax)
      character*2 atosym
c
      do iat=1, Nmax
         nuchar=CHARGE(iat)

        if ((nuchar.gt.0).AND.(nuchar.lt.mxchg)) then
           call atgrep(atosym,nuchar)
           mtype(iat)=atosym
        else 
          if (nuchar.gt.mxchg) then
             ierr=1
             write(6,"(a,I3,a)") ' WARNING : nuclear charges > ',
     &       mxchg,' not yet catalogued ! '
          endif

        endif
       Enddo
c
      return
      end
