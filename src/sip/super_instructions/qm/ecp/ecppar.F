      subroutine ecppar(ishow,cnick,charge,natoms,seward,lusew)

      implicit double precision (a-h,o-z)
C
c ------------------------------------------------------------------
#include "ecp.par"
#include "ecp.com"
c
c -----  routine reads in the parameters specifying the   -----
c -----  local potentials.                                -----
c -----  information used for -ecpint- and -ecdint-       -----

      character*80 cnick(natoms)
      Dimension charge(Natoms),npecp(Max_centers)
      logical skip, seward

      skip=ishow.eq.-1
c
      call ecpget(npecp,cnick,natoms,ishow)
c
      nursex=0

      do  90 iat=1,natoms
          if(ipseux(iat).gt.0.and.nelecp(iat).eq.0) then
             nursex=nursex+1
             if(nursex.eq.1) write(6,*)
             write(6,601) iat,ipseux(iat)
  601        format(' for atom no. ',i4,' and ecp no. ',i4,
     1               ' there are no core electrons ! ')
          endif
   90 continue

      if(nursex.gt.0) write(6,*)
c
      do 100 iat=1,natoms
         CHARGE(iat)=CHARGE(iat)-dble(nelecp(iat))
  100 continue
c
      if (skip) return
c
c-----------------------------------------------------------------------
c ishow=2: to print ecp-parameters to file
c      =1: to print only ecp-nicknames to file
c      =0: to print no parameters to file
c-----------------------------------------------------------------------
      ishow=2
c
      if (ishow.eq.0) goto 200
c
c  -----  print ecp parameters
c
      write(6,1100)
 1100 format(/,10x,18(1h-),/,10x,'-ecp- potential(s)',
     1       /,10x,18(1h-))
c
      necp=0
      do 110 iat=1,natoms
         if(ipseux(iat).gt.necp) necp=ipseux(iat)
  110 continue
c
      do 120 iecp=1,necp
         do 130 iat=1,natoms
            if(ipseux(iat).ne.iecp) goto 130
            lust=lstr(cnick(iecp),80)

          if(ishow.le.1) then
            write(6,1200) iat,cnick(iecp)(1:lust)
 1200       format(i5,a)
          else
          call wrecp(6,nelecp(iat),llmax(iat),kfirst(1,iat),
     &               klast(1,iat),nlp,clp,zlp,lmxecp,Mxecpprim,
     &               cnick(iecp)(1:lust),seward,lusew)
          endif
  130    continue
  120 continue
c
  200 continue
c
      return
      end
