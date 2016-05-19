      subroutine ecpget(npecp,cnick,natoms,ishow)

      implicit double precision (a-h,o-z)
c
c --- this subroutine reads ecp data for unique ecp definitions
c     and assigns arrays llmax,nelecp,kfirst and klast indexed
c     by atoms (not by unique ecp definitions) ---
c
c     ipseux   : index of ecp belonging to an atom
c     npecp    : number of primitive terms per ecp
c     llmax    : maximum l-quantum number of ecp
c     nelecp   : number of core electrons per atom
c     clp,nlp,zlp   : specifications of primitive terms
c     cnick    : nicknames of ecp's
c
c     kfirst, klast: gives the starting and ending points
c                    for each angular momentum blocks (first
c                    U(llmax) U(0-llmax)
c  --- note : atoms with no ecp are marked by nelecp=0
c
#include "ecp.par"
#include "ecp.com"
C
      dimension npecp(natoms),lfirst(lmxecp),llast(lmxecp)
      character*80 filnam,zeile
      character*80 cnick(natoms)

c --- look for the flag $ecp in control file fcntri and get 
c     alternate file name if specified ---
c
      iunit=44
      filnam='ECPDATA'
c
c --- now we are ready for reading
c
      lenfil=index(filnam,' ')-1
      Write(6,*)
      write(6,'(3a)') ' Reading data block $ecp from file <',
     1                  filnam(1:lenfil),'>'
      open(unit=iunit,file=filnam(1:lenfil),form='formatted')

      necp=0
      do 10 i=1,natoms
         if(ipseux(i).gt.necp) necp=ipseux(i)
   10 continue

      if(necp.eq.0) return
c
c loop over ecp sets in $ecp data block :
c
      do 400 iecp=1,necp
C
      call getstr(iunit,1,igotit,zeile,cnick(iecp))
c
      if(igotit.le.0) then
        write(6,4091) cnick(iecp),filnam(1:lenfil)
 4091   format(/,1x,' ECPGET ABEND : ECP SET WITH NICKNAME ',A,/,
     1           1x,' IS NOT CONTAINED WITHIN FILE ',A,/)
        goto 999
      endif
c
  11  call getstr(iunit,-1,ifound,zeile,'*')
c
c '*' has to be the first character in the line :

      if(ifound.gt.1) goto 11

      if(ifound.gt.0) call rdecp(iunit,iecp,npecp,nelecp(iecp),
     1   llmax(iecp),kfirst(1,iecp),klast(1,iecp),nlp,clp,zlp,
     2   lmxecp,Mxecpprim,igotit)
c
      if(igotit.le.0 .or. ifound.le.0) then
        write(6,4092) cnick(iecp)
 4092   format(/,1x,' ecpget abend : ecp set with nickname ',a,/,
     1           1x,' cannot be read in properly ',/)
        goto 999
      endif
c
  400 continue
c
C --- icopy is the number of ecp's already assigned
C
      icopy=0

   15 iecp=0
   20 iecp=iecp+1
      if(iecp.gt.necp) goto 15
      iwhere=iecp

   25 iat=iattop(iecp,natoms,ipseux)

      if(iat.eq.0) goto 20

      if(iat.eq.iecp) then
        ipseux(iat)=-ipseux(iat)
        icopy=icopy+1
        if(icopy.eq.necp) goto 90
        goto 20
      endif

      if(iat.gt.necp) then

        nelecp(iat)=nelecp(iwhere)
        llmax(iat)=llmax(iwhere)

        do k=1,llmax(iwhere)+1
          kfirst(k,iat)=kfirst(k,iwhere)
          klast(k,iat)=klast(k,iwhere)
        enddo
      
        ipseux(iat)=-ipseux(iat)
        icopy=icopy+1
        if(icopy.eq.necp) goto 90
        goto 20

      endif

      nelecs=nelecp(iat) 
      llmaxs=llmax(iat)

      do k=1,llmax(iat)+1
        lfirst(k)=kfirst(k,iat)
        llast(k)=klast(k,iat)
      Enddo
c
c --- iwhere is the location of ecp data to be assigned 
c
      nelecp(iat)=nelecp(iwhere)
      llmax(iat)=llmax(iwhere)

      do  k=1,llmax(iwhere)+1
        kfirst(k,iat)=kfirst(k,iwhere)
        klast(k,iat)=klast(k,iwhere)
      Enddo

      ipseux(iat)=-ipseux(iat)
      icopy=icopy+1
      if(icopy.eq.necp) goto 90

      nelecp(iwhere)=nelecs
      llmax(iwhere)=llmaxs

      do k=1,llmax(iwhere)+1
        kfirst(k,iwhere)=lfirst(k)
        klast(k,iwhere)=llast(k)
      Enddo

c  --- number of the ecp which is to be assigned next
      iecp=iat
      goto 25

   90 continue

      do 100 iat=1,natoms
        if(ipseux(iat).lt.0) ipseux(iat)=-ipseux(iat)
  100 continue

c  --- assign remaining data

      if(necp.eq.natoms) return

      do 200 iat=1,natoms
        iecp=ipseux(iat)

        if(iecp.eq.0) then
          nelecp(iat)=0
          goto 200
        endif

        iwhere=iattop(iecp,natoms,ipseux) 
        if(iwhere.eq.iat) goto 200
        nelecp(iat)=nelecp(iwhere)
        llmax(iat)=llmax(iwhere)

        do k=1,llmax(iwhere)+1
          kfirst(k,iat)=kfirst(k,iwhere)
          klast(k,iat)=klast(k,iwhere)
        Enddo

  200 continue
c
      close(iunit)
      return
c
  999 continue
      write(6,901)
  901 format(/,1x,' <ECPGET> : CANNOT ASSIGN ECP DATA ',/)
      close(iunit)
      CALL ERREX
      end
