      subroutine wrecp(iunit,ncore,llmax,kfirst,klast,
     &                 nlp,clp,zlp,lmxecp,max27,inick,
     &                 seward,lusew)
C
      implicit double precision (a-h,o-z)
c
c     write pseudopotential parameters
c     design as follows :
c     --- <nickname> (as for basis sets)
c     --- * (indicates start of data) (this is the actual position)
c     --- ncore=<...> llmax=<...>
c         ncore = number of core electrons
c         llmax = lmax+1 where lmax = max. l-quantum number within core
c         note that lsymb(i+1)=type corresponding to l-quantum number i
c         lsymb(1)=s,lsymb(2)=p and so on
c     --- <lsymb(llmax)>
c         ... data for lmax+1 
c     --- loop : l=0,...,lmax
c     --- <lsymb(l)>-<lsymb(llmax)>
c         ... data for l
c     --- end of loop
c     --- * (indicates end of data)
c
      dimension nlp(max27),clp(max27),zlp(max27),
     &          kfirst(lmxecp),klast(lmxecp)
      character*80 inick
      character*8 lsymb
      logical seward
c
      data lsymb /'spdfghij'/
c
c     --- nickname
c
      write(iunit,'(a)') inick
      write(iunit,'(a)') '*'
      write(iunit,901) ncore,llmax
  901 format(1x,' NCORE = ',i5,10x,' LMAX = ',i5)
      write(iunit,'(a)') '#        COEFFICIENT   R^N          EXPONENT'
c
c     --- U(lmax)
c
      lmaxp1=llmax+1
      write(iunit,'(a1)') lsymb(lmaxp1:lmaxp1)
      do 100 k=kfirst(1),klast(1)
  100   write(iunit,911) clp(k),nlp(k),zlp(k)
  911   format(f20.7,i5,f20.7)
c
c     --- U(l)-U(lmax), l = 0,...,lmax-1
c
      do 200 l=1,llmax
        write(iunit,'(3a1)') lsymb(l:l),'-',lsymb(lmaxp1:lmaxp1)
        do 300 k=kfirst(l+1),klast(l+1)
  300   write(iunit,911) clp(k),nlp(k),zlp(k)
  200 continue
c
      write(iunit,'(a)') '*'
      if (seward) then
          write(lusew, *) inick(1:2),'..... / inline'
          lmaxp1 = llmax + 1 
          do k = kfirst(1), klast(1)
             write(lusew,'(f18.10)') zlp(k)
          enddo 
          do l=1, llmax
             do k = kfirst(l+1), klast(l+1)
                write(lusew,'(f18.10)') zlp(k)
             enddo
          enddo
          do k = kfirst(1), klast(1)
             write(lusew,'(f18.10)') clp(k)
          enddo
          do l=1, llmax
             do k = kfirst(l+1), klast(l+1)
                write(lusew,'(f18.10)') clp(k)
             enddo 
          enddo
      endif 
c
      return
      end

