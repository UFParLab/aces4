      subroutine getstr(iunit,irewnd,ifound,zeile,stropt)
      implicit double precision (a-h,o-z)

c     --- scan through file connected to unit=iunit looking for 
c         a line containing string <stropt> 
c         note : comment lines (starting with '#') will be skipped !
c         irewnd>0  : rewind iunit
c         irewnd=0  : no rewind
c         irewnd<0  : search up to next buck (i.e. '$') including
c                     input string zeile !
c         ifound>0  : string has been found (index=ifound)
c         ifound=0  : string has not been found 
c         ifound=-1 : string has not been found since file is empty

      character*80 zeile
      character*(*) stropt
      integer iunit,ifound

      ifound=-1
      iread=0
      if(irewnd.gt.0) rewind iunit
      if(irewnd.lt.0) goto 101
  100 read(iunit,'(a)',end=900) zeile
      iread=iread+1
      if (zeile(1:4).eq.'$end') return
  101 call wisch(zeile,80)
      if(zeile(1:1).eq.' '.or.zeile(1:1).eq.'#') goto 100
      if(irewnd.lt.0.and.iread.gt.0.and.zeile(1:1).eq.'$') return
      ifound=index(zeile,stropt)
      if(ifound.eq.0) goto 100
      return

  900 continue
      return

      end
