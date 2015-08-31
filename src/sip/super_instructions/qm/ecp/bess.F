      function bess(z,l)
      implicit double precision (a-h,o-z)

#include "ecp.par"
#include "ecp.com"

      data am1,a0,accrcy,a1s2,a1,a5,a16p1
     1   /-1.0d0,0.0d0,5.0d-14,0.5d0,1.0d0,5.0d0,16.1d0/

      if(z.gt.a5) go to 50
      if(z.eq.a0) go to 40
      if(z.lt.a0) go to 35
      zp=a1s2*z*z
      term=(z**l)/fac2(l+l+3-2)
      bess=term
      j=0
 5    j=j+1
      fjlj1=(j*(l+l+j+j+1))
      term=term*zp/fjlj1
      bess=bess+term
      if(dabs(term/bess).gt.accrcy) go to 5
      bess=bess*dexp(-z)
      go to 100
 35   bess=a0
      go to 100
 40   if(l.ne.0)go to 45
      bess=a1
      go to 100
 45   bess=a0
      go to 100
 50   if(z.gt.a16p1) go to 60
      rp=a0
      rm=a0
      tzp=z+z
      tzm=-tzp
      l1=l+1
      do 55 k1=1,l1
      k=k1-1
      rp=rp+fprod(k1,l1)/tzp**k
      rm=rm+fprod(k1,l1)/tzm**k
 55   continue
      bess=(rm-(am1**l)*rp*dexp(tzm))/tzp
      go to 100
 60   rm=a0
      tzm=-z-z
      l1=l+1
      do 65 k1=1,l1
      k=k1-1
      rm=rm+fprod(k1,l1)/tzm**k
 65   continue
      bess=rm/(-tzm)
 100  return
      end


                                   
