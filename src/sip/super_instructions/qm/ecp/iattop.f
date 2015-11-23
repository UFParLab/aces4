      function iattop(igrep,n,ind)
      implicit double precision (a-h,o-z)
c
c --- determine the first of n atoms with ind(iattop)=igrep
c
      dimension ind(n)
c
      iattop=0
      k=0
   10 k=k+1
      if(k.gt.n) return
      if(ind(k).eq.-igrep) return
      if(ind(k).ne.igrep) goto 10
      iattop=k
c
      return
      end
