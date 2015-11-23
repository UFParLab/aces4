      function lstr(str,l)
c
c     find rightmost non-blank in string str of length l
c     and return its position
c
      implicit double precision (a-h,o-z)
      character*(*) str

      lstr=0
      i=min(len(str),l)+1
   10 i=i-1
      if(i.eq.0) return
      if(str(i:i).eq.' ') go to 10
      lstr=i

      return 
      end
