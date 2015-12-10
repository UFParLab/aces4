      subroutine wisch(line,n)
      implicit double precision (a-h,o-z)

c ----------------------------------------------------------------------
c       delete leading blanks in string <line>
c       19.09.90  C.M.K.
c ----------------------------------------------------------------------

      character*(*) line

      m=len(line)

      if(line(1:1).ne.' ') return

      i=m+1
c     --- last non-blank
    1 i=i-1
      if(i.gt.0.and.line(i:i).eq.' ') goto 1
      if(i.eq.0) return
c     --- first non-blank
      j=0
    2 j=j+1
      if(j.lt.i.and.line(j:j).eq.' ') goto 2
      l=j
c     --- shift
      k=1
    3 line(k:k)=line(j:j)
      k=k+1
      j=j+1
      if(j.le.i) goto 3
c     --- fill up with blanks
      if(k.gt.l) l=k
    4 line(l:l)=' '
      l=l+1
      if(l.le.i) goto 4

      return
      end
