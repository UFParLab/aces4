      subroutine qpasy(npi,l,lit,ljt,ltot1,lmalo,lmahi,lmblo,lmbhi,
     1   alp,xka1,xkb1,prd,dum,qsum)
c     ----- partially asymptotic form for q(n,la1,lb1) -----
c     ----- scaled by exp(-xkb*xkb/(4*alpha)) to prevent overflows
      implicit double precision (a-h,o-z)
c*    common/qstore/dum1(49),alpha,xk,t
      common/qstore/dum1(81),alpha,xk,t
      common /dfac/ dfac(29)
c*    common /dfac/ dfac(23)
c*    dimension qsum(7,7,2)
c*    dimension fctr(7), sum(7), term(7)
      dimension qsum(9,9,9)
      dimension fctr(9), sum(9), term(9)
      data a0,accrcy,a1s4,a1s2,a1 /0.0d0,1.0d-13,0.25d0,0.5d0,1.0d0/
c     ----- first set up xkb as larger -----
      save
      if(xka1.gt.xkb1) go to 10
      xka=xka1
      xkb=xkb1
      go to 12
   10 xka=xkb1
      xkb=xka1
c     ----- set up parameters for qcomp using xkb -----
   12 alpha=a1
      sqalp=dsqrt(alp)
      xk=xkb/sqalp
      t=a1s4*xk*xk
      prd=prd*dexp(-(dum-t))
      tk=xka*xka/(alp+alp)
      do 90 lama=lmalo,lmahi
      ldifa1=iabs(l-lama)+1
      if(xka1.gt.xkb1) go to 14
      la=lama-1
      go to 16
   14 lb=lama-1
   16 do 90 lamb=lmblo,lmbhi
      ldifb=iabs(l-lamb)
      nlo=ldifa1+ldifb
      nhi=(ltot1-mod(lit-ldifa1,2))-mod((ljt-1)-ldifb,2)
      if(xka1.gt.xkb1) go to 18
      lb=lamb-1
      go to 20
   18 la=lamb-1
c     ----- run power series using xka, obtaining initial    -----
c     ----- q(n,l) values from qcomp, then recurring upwards -----
c     ----- j=0 term in sum -----
   20 qold2=qcomp(npi+nlo-1+la,lb)/dfac(la+la+3)
      fctr(nlo)=a1
      sum(nlo)=qold2
      if(nlo.eq.nhi.and.tk.eq.a0) go to 60
c     ----- j=1 term in sum -----
      nprime=npi+nlo+la+1
      qold1=qcomp(nprime,lb)/dfac(la+la+3)
      if(nlo.ne.nhi) fctr(nlo+2)=fctr(nlo)
      f1=(la+la+3)
      fctr(nlo)=tk/f1
      term(nlo)=fctr(nlo)*qold1
      sum(nlo)=sum(nlo)+term(nlo)
      if(nlo.ne.nhi) go to 22
      qold2=fctr(nlo)*qold2
      qold1=term(nlo)
      go to 24
   22 nlo2=nlo+2
      sum(nlo2)=qold1
      if(nlo2.eq.nhi.and.tk.eq.a0) go to 60
   24 j=1
c     ----- increment j for next term -----
   30 j=j+1
      nprime=nprime+2
      f1=(nprime+nprime-5)
      f2=((lb-nprime+4)*(lb+nprime-3))
      qnew=(t+a1s2*f1)*qold1+a1s4*f2*qold2
      nlojj=nlo+j+j
      if(nlo.eq.nhi) go to 40
      nhitmp=min0(nlojj,nhi)
      do 38 n=nlo2,nhitmp,2
      nrev=nhitmp+nlo2-n
   38 fctr(nrev)=fctr(nrev-2)
   40 f1=(j*(la+la+j+j+1))
      fctr(nlo)=tk/f1
      if(nlojj.gt.nhi) go to 44
      nhitmp=nlojj-2
      term(nlojj)=qnew
      sum(nlojj)=term(nlojj)
      do 42 n=nlo,nhitmp,2
      nrev=nhitmp+nlo-n
      term(nrev)=fctr(nrev)*term(nrev+2)
   42 sum(nrev)=sum(nrev)+term(nrev)
      if(nlojj.eq.nhi.and.tk.eq.a0) go to 60
      qold2=qold1
      qold1=qnew
      go to 30
   44 qold2=fctr(nhi)*qold1
      qold1=fctr(nhi)*qnew
      term(nhi)=qold1
      sum(nhi)=sum(nhi)+term(nhi)
      if(nlo.eq.nhi) go to 47
      nhitmp=nhi-2
      do 46 n=nlo,nhitmp,2
      nrev=nhitmp+nlo-n
      term(nrev)=fctr(nrev)*term(nrev+2)
   46 sum(nrev)=sum(nrev)+term(nrev)
   47 do 48 n=nlo,nhi,2
   48 if(term(n).gt.accrcy*sum(n)) go to 30
   60 if(la.ne.0) go to 62
      prefac=prd/sqalp**(npi+nlo+la)
      go to 64
   62 prefac=prd*xka**la/sqalp**(npi+nlo+la)
   64 do 66 n=nlo,nhi,2
      qsum(n,lamb,lama)=qsum(n,lamb,lama)+prefac*sum(n)
   66 prefac=prefac/alp
   90 continue
      return
      end


