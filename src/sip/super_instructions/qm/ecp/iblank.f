      function iblank(zeile)
      implicit double precision (a-h,o-z)
c
      integer iblank
      character*(*) zeile
c
c     --- delete leading blanks and equal signs ("=")
c
      n=len(zeile) 
c
      iblank=index(zeile,' ')
      if(iblank.eq.0) then
        iblank=n+1
        return
      endif
c
      i=0
  100 i=i+1
      if(i.gt.n) goto 200
      if(zeile(i:i).eq.' '.or.zeile(i:i).eq.'=') goto 100
  200 i=i-1
      if(i.eq.0) return
c
      do 400 j=1,n-i
  400   zeile(j:j)=zeile(i+j:i+j)
c
      if(i.lt.n) zeile(n-i+1:)=' '
c
      iblank=index(zeile,' ')
c
      return
      end
