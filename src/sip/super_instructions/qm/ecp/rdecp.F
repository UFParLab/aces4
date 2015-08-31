      subroutine rdecp(iunit,iecp,npecp,ncore,llmax,kfirst,klast,
     &                 nlp,clp,zlp,lmxecp,max27,igotit)

      implicit double precision (a-h,o-z)
c
c read pseudopotential parameters
c design as follows :
c --- <nickname> (as for basis sets)
c --- * (indicates start of data) (this is the actual position)
c --- ncore=<...> llmax=<...>
c     ncore = number of core electrons
c     llmax = lmax+1 where lmax = max. l-quantum number within core
C     note that lsymb(i+1)=type corresponding to l-quantum number i
c     lsymb(1)=s,lsymb(2)=p and so on
c --- <lsymb(llmax)>
c     ... data for lmax+1 
c --- loop : l=0,...,lmax
c --- <lsymb(l)>-<lsymb(llmax)>
c     ... data for l
c --- end of loop
c --- * (indicates end of data)
c
c input : lmxecp = max. allowed value for llmax (now, lmxecp<=7)
c
      dimension npecp(iecp),nlp(max27),clp(max27),zlp(max27),
     1          kfirst(lmxecp),klast(lmxecp)

      character*80 zeile,scrzl
      character*8 lsymb
      character cdummy
c
      data lsymb /'spdfghij'/
c
c --- provide the offsets (sum over ecp's 1,...,iecp-1)
c
      noff=0
      do  i=1, iecp-1
          noff=noff+npecp(i)
      enddo

      noffst=noff
c
      igotit=-1
c
c--- initialize kfirst and klast to 1 and 0, respectively
c
      do i=1,lmxecp
        kfirst(i)=1
        klast(i)=0
      enddo 
      
      Write(6,*)
c
c     --- get ncore,llmax for the new ecp
c
  100 read(iunit,'(a)',end=900) zeile
      call wisch(zeile,80)

      if(zeile(1:1).eq.' '.or.zeile(1:1).eq.'#') goto 100

c --- asterisk terminates input

      if(zeile(1:1).eq.'*') goto 900

      icore=index(zeile,'NCORE')
      ilmax=index(zeile,'LMAX')
      scrzl=zeile(icore+5:80)

      call rdebbs(scrzl,80,1,0,0,isucc,iwert,rdummy,cdummy)

      if(isucc.le.0) then
        write(6,'(/,2x,a,/)')
     1   'CANNOT READ NCORE = NUMBER OF CORE ELECTRONS'
        return
      endif

      ncore=iwert 

      scrzl=zeile(ilmax+4:80)
      call rdebbs(scrzl,80,1,0,0,isucc,iwert,rdummy,cdummy)

      if(isucc.le.0) then
        write(6,'(/,2x,a,/)')
     1   'CANNOT READ LMAX = MAX. L-QUANTUM NUMBER'
        return
      endif

      llmax=iwert
c
c --- loop : look for strings of the type <l> or <k-l> with
c     k,l out of s,p,d,f,g,h,i 

      lsvtyp=0

  200 read(iunit,'(a)',end=900) zeile
      call wisch(zeile,80)

      if(zeile(1:1).eq.' '.or.zeile(1:1).eq.'#') goto 200
C
c  --- asterisk terminates input
c
      if(zeile(1:1).eq.'*') then
        if(lsvtyp.gt.0) klast(lsvtyp)=noff
        goto 250
      endif

c --- data for l=ltyp

      ltyp=0
      do 210 ityp=1,lmxecp
        if(zeile(1:1).eq.lsymb(ityp:ityp)) ltyp=ityp
  210 continue 

      if(ltyp.eq.0.and.lsvtyp.eq.0) stop ' scheisse '


      if(ltyp.gt.llmax+1) then
        write(6,'(/,2x,a,/)')
     1   'ecp data are for an illegal l-quantum number'
        return
      endif

      if(ltyp.eq.0) then

        noff=noff+1

        if(noff.gt.max27) then
          write(6,'(/,2x,a,/)') 'too many primitive ecp terms'
          call errex
        endif

        call rdebbs(zeile,80,0,1,0,isucc,idummy,rwert,cdummy)

        if(isucc.le.0) then
          write(6,'(/,2x,a,/)')
     1          'error in reading coefficient of gaussian'
          return
        endif

        clp(noff)=rwert

        call rdebbs(zeile,80,1,0,0,isucc,iwert,rdummy,cdummy)

        if(isucc.le.0) then
          write(6,'(/,2x,a,/)') 'error in reading exponent of r'
          return
        endif

        nlp(noff)=iwert

        call rdebbs(zeile,80,0,3,0,isucc,idummy,rwert,cdummy)

        if(isucc.le.0) then
          write(6,'(/,2x,a,/)')
     1     'error in reading exponent of gaussian'
          return
        endif

        zlp(noff)=rwert

      elseif(ltyp.gt.0) then

        kfirst(ltyp)=noff+1
        if(lsvtyp.gt.0) klast(lsvtyp)=noff
        lsvtyp=ltyp

      endif

      goto 200

  250 continue

      npecp(iecp)= noff-noffst
      igotit=1
c
      ifscr=kfirst(llmax+1)
      ilscr=klast(llmax+1)

      do i=llmax,1,-1
        kfirst(i+1)=kfirst(i)
        klast(i+1)=klast(i)
      enddo
     
      kfirst(1)=ifscr
      klast(1)=ilscr
c
      return
c
  900 continue
      write(6,'(/,2x,a,/)') 'RDECP : END OF FILE ENCOUNTERED - SORRY'
      return
c
      end
