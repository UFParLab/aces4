      function qcomp(n,l)
c     ----- computes q(n,l)                                -----
c     ----- scaled by exp(-t) to prevent overflows         -----
c     ----- arguments are alpha, beta, and xval=beta**2/(4*alpha) -----
c     ----- no restriction on the magnitude of xval           -----

      implicit double precision (a-h,o-z)

CSSS      common /pifac/ pi, sqpi2
CSSS      common /dfac/ dfac(29)
CSSS      common/qstore/dum1(81),alpha,xk,t
#include "ecp.par"
#include "ecp.com"

      dimension tmin(9)

      data tmin/31.0d0,28.0d0,25.0d0,23.0d0,22.0d0,20.0d0,19.0d0,
     1   18.0d0,15.0d0/

      data am1,   a0,   accpow, accasy, a1,   a2,   a4
     1    /-1.0d0,0.0d0,1.0d-14,1.0d-10,1.0d0,2.0d0,4.0d0/

      if(mod(n+l,2).ne.0.or.n.le.l) go to 30
c     ----- use alternating series (n+l.le.22.and.l.le.10) -----

      if(l.eq.0) xkp=a1
      if(l.ne.0) xkp=(beta/(alpha+alpha))**l
      prefac=sqpi2*xkp*fac2(n+l+1-2)/
     1   ((alpha+alpha)**((n-l)/2)*dsqrt(alpha+alpha)*fac2(l+l+3-2))
      num=l-n+2
      xden=(l+l+3)
      term=a1
      sum=term
      xc=am1
 10   if(num.eq.0) go to 20
      fnum=num
      term=term*fnum*xval/(xden*xc)
      xc=xc+am1
      sum=sum+term
      num=num+2
      xden=xden+a2
      go to 10
   20 qcomp=prefac*sum
      return

   30 if(xval.lt.tmin(min0(n,8)+1)) go to 60
c     ----- use asymptotic series (arbitrary n,l) -----
      xkp=(beta/(alpha+alpha))**(n-2)
      prefac=xkp*sqpi2/((alpha+alpha)*dsqrt(alpha+alpha))
      sum=a1
      term=a1
      faca=(l-n+2)
      facb=(1-l-n)
      xc=a1
   40 term=term*faca*facb/(a4*xc*xval)
      if(term.eq.a0) go to 50
      sum=sum+term
      if(dabs(term/sum).lt.accasy) go to 50
      faca=faca+a2
      facb=facb+a2
      xc=xc+a1
      go to 40
   50 qcomp=prefac*sum
      return

c     ----- use power series (n+l.le.22.and.l.le.10) -----
   60 if(l.eq.0) xkp=a1
      if(l.ne.0) xkp=(beta/(alpha+alpha))**l
      prefac=dexp(-xval)*xkp/(alpha+alpha)**((n-l+1)/2)
      if(mod(n+l,2).eq.0) prefac=prefac*sqpi2/dsqrt(alpha+alpha)
      xnum=(l+n-1)
      xden=(l+l+1)
      term=fac2(l+n+1-2)/fac2(l+l+3-2)
      sum=term
      xj=a0
   70 xnum=xnum+a2
      xden=xden+a2
      xj=xj+a1
      term=term*xval*xnum/(xj*xden)
      sum=sum+term
      if((term/sum).gt.accpow) go to 70
      qcomp=prefac*sum
      return
      end

