      Subroutine Sps(Nlq, Lprj, La, Lb, Ltot, Lamalo, Lamahi,
     &               Lamblo, Lambhi, Lamau, Lambu, Alpha2,
     &               Beta1, Beta2, prd, Dum, Rad2)

      Implicit Double Precision (A-H, O-Z)

#include "ecp.par"
#include "ecp.com"

      Dimension Rad2(0:2*Maxang,0:2*Maxang,0:2*Maxang), 
     &          Fctr(2*maxang+3), Sum(2*Maxang+3), Term(2*Maxang+3)

CSSS      common /dfac/ dfac(29)
CSSS      common/qstore/dum1(81),alpha,xk,t
CSSS
      Data a0,accrcy,a1s4,a1s2,a1 /0.0d0,1.0d-13,0.25d0,0.5d0,1.0d0/
C
#ifdef _DEBUG_LVL0
      Write(6,"(a,1x,4(1x,F20.13))") "Prd,,beta1,beta2,Alp.",Prd,beta1,
     &                                beta2, Alpha
      Write(6,*) "Dum: ", Dum
      Write(6,*)
#endif

      Xka1 = Beta1
      Xkb1 = Beta2
      Alp  = Alpha

#ifdef _DEBUG_LVL0
      Write(6,"(a,7(1x,I3))") "Ltot,La,Lb,Lamalo,Lamahi,Lamblo,Lambhi:",
     &    Ltot,La,Lb,Lamalo,Lamahi,Lamblo,Lambhi
#endif
      L     = Lprj + 1
      Lit   = La + 1
      Ljt   = Lb + 1
      Ljtm1 = Ljt - 1

      Ltot1 = Ltot + 1
      Lmalo = Lamalo + 1
      Lmahi = Lamahi + 1
      Lmblo = Lamblo + 1
      Lmbhi = Lambhi + 1

      Npi   = Nlq

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
C
C Set Qstore common bolck, so that Qcomp can work.
C
      Beta = xk
      Xval = t

      prd=prd*dexp(-(dum-t))
#ifdef _DEBUG_LVLM
      Prd = 1.0
      Write(6,"(a,F10.5)") "Setting the scaling factor for debuging",
     &                       prd
#endif

      tk=xka*xka/(alp+alp)

      do 90 lama=lmalo,lmahi
      ldifa1=iabs(l-lama)+1
      if(xka1.gt.xkb1) go to 14
      la1=lama-1
      go to 16
   14 lb1=lama-1

   16 do 90 lamb=lmblo,lmbhi
      ldifb=iabs(l-lamb)
      nlo=ldifa1+ldifb
      nhi=(ltot1-mod(lit-ldifa1,2))-mod((ljt-1)-ldifb,2)
      if(xka1.gt.xkb1) go to 18
      lb1=lamb-1
      go to 20
   18 la1=lamb-1

c     ----- run power series using xka, obtaining initial    -----
c     ----- q(n,l) values from qcomp, then recurring up wards -----
c     ----- j=0 term in sum -----

   20 continue
#ifdef _DEBUG_LVL0
      Write(6,"(a,3(1x,F10.7))") "Qstore varialbles :", Alpha, Xk, t
      Write(6, "(a,4(1x,I2))") "Nstart,Lama,Lamb: ", nlo,npi+nlo-1+la1,
     &           la1,lb1
#endif

      qold2=qcomp(npi+nlo-1+la1,lb1)/fac2(la1+la1+3-2)
      fctr(nlo)=a1
      sum(nlo)=qold2
      if(nlo.eq.nhi.and.tk.eq.a0) go to 60

c     ----- j=1 term in sum -----
      nprime=npi+nlo+la1+1
      qold1=qcomp(nprime,lb1)/fac2(la1+la1+3-2)
      if(nlo.ne.nhi) fctr(nlo+2)=fctr(nlo)
      f1=(la1+la1+3)
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
      f2=((lb1-nprime+4)*(lb1+nprime-3))
      qnew=(t+a1s2*f1)*qold1+a1s4*f2*qold2

      nlojj=nlo+j+j
      if(nlo.eq.nhi) go to 40
      nhitmp=min0(nlojj,nhi)

      do 38 n=nlo2,nhitmp,2
      nrev=nhitmp+nlo2-n
      fctr(nrev)=fctr(nrev-2)
   38 continue

   40 f1=(j*(la1+la1+j+j+1))
      fctr(nlo)=tk/f1
      if(nlojj.gt.nhi) go to 44
      nhitmp=nlojj-2
      term(nlojj)=qnew
      sum(nlojj)=term(nlojj)

      do 42 n=nlo,nhitmp,2
      nrev=nhitmp+nlo-n
      term(nrev)=fctr(nrev)*term(nrev+2)
       sum(nrev)=sum(nrev)+term(nrev)
   42 continue

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
      sum(nrev)=sum(nrev)+term(nrev)
  46  continue

   47 do 48 n=nlo,nhi,2
   48 if(term(n).gt.accrcy*sum(n)) go to 30
   60 if(la1.ne.0) go to 62
      prefac=prd/sqalp**(npi+nlo+la1)
      go to 64
   62 prefac=prd*xka**la1/sqalp**(npi+nlo+la1)
   64 do 66 n=nlo,nhi,2
      Rad2(n-1,lamb-1,lama-1)=Rad2(n-1,lamb-1,lama-1)+prefac*sum(n) 
#ifdef _DEBUG_LVLM
      write(6,*) sum(n), prefac, Rad2(n,lamb,lama), n, lama, lamb
#endif

   66 prefac=prefac/alp
   90 continue
      return
      end

