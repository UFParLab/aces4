      Subroutine Make_ztab
 
      Implicit Double Precision (a-h,o-z)

#include "ecp.par"
#include "ecp.com"
C
C form of liner cartesian products. The table is generated
C using the expression provided by Richard J. Mathar. 
C
C -Note that (PRMTESTING block is used only in the early stages of
C development and no longer supported). Ajith Perera.
C
#ifdef _PRMTESTING
C l=0,m=0

      zlm(1) = sqrt(1.0d0/fpi)
c
C l=1,m=+1,0,-1
c
      zlm(2) = sqrt(3.0d0/fpi)
      zlm(3) = zlm(2)
      zlm(4) = zlm(2)
C
C l=2,m=-2,-1....1,2
C
      zlm(5) = sqrt(15.0d0/fpi)/2.0d0
      zlm(6) = -zlm(5)
      zlm(7) = 2.0d0*zlm(5)
      zlm(8) = 3.0d0*sqrt(5.0d0/fpi)/2.0d0
      zlm(9) = -zlm(8)/3.0d0
      zlm(10) = zlm(7)
      zlm(11) = zlm(7)
c l=3
      zlm(12) = sqrt(35.0d0/(8.0d0*fpi))
      zlm(13) = -3.0d0*zlm(12)
      zlm(14) = sqrt(105.0d0/(4.0d0*fpi))
      zlm(15) = -zlm(14)
      zlm(16) = 5.0d0*sqrt(21.0d0/(8.0d0*fpi))
      zlm(17) = -zlm(16)/5.0d0
      zlm(18) = 5.0d0*sqrt(7.0d0/fpi)/2.0d0
      zlm(19) = -3.0d0*zlm(18)/5.0d0
      zlm(20) = zlm(16)
      zlm(21) = zlm(17)
      zlm(22) = 2.0d0*zlm(14)
      zlm(23) = -zlm(13)
      zlm(24) = -zlm(12)
c l=4
      zlm(25) = sqrt(315.0d0/(64.0d0*fpi))
      zlm(26) = -6.0d0*zlm(25)
      Zlm(27) = zlm(25)
      zlm(28) = sqrt(315.0d0/(8.0d0*fpi))
      zlm(29) = -3.0d0*zlm(28)
      temp    = sqrt(45.0d0/fpi)/4.0d0
      zlm(30) = 7.0d0*temp
      zlm(31) = -zlm(30)
      zlm(32) = temp
      zlm(33) = -temp
      temp    = sqrt(45.0d0/(8.0d0*fpi))
      zlm(34) = 7.0d0*temp
      zlm(35) = -3.0d0*temp
      temp    = sqrt(9.0d0/fpi)/8.0d0
      zlm(36) = 35.0d0*temp
      zlm(37) = -30.0d0*temp
      zlm(38) = 3.0d0*temp
      zlm(39) = zlm(34)
      zlm(40) = zlm(35)
      temp    = sqrt(45.0d0/(4.0d0*fpi))
      zlm(41) = 7.0d0*temp
      zlm(42) = -temp
      zlm(43) = -zlm(29)
      zlm(44) = -zlm(28)
      zlm(45) = sqrt(315.0d0/(4.0d0*fpi))
      zlm(46) = -zlm(45)
c l=5
      zlm(47) = sqrt(693.0d0/(128.0d0*fpi))
      zlm(48) = -10.0d0*zlm(47)
      zlm(49) = 5.0d0*zlm(47)
      zlm(50) = sqrt(3465.0d0/(64.0d0*fpi))
      zlm(51) = -6.0d0*zlm(50)
      zlm(52) = zlm(50)
      temp    = sqrt(385.0d0/(128.0d0*fpi))
      zlm(53) = 9.0d0*temp
      zlm(54) = -27.0d0*temp
      zlm(55) = 3.0d0*temp
      zlm(56) = -temp
      temp    = sqrt(1155.0d0/fpi)/4.0d0
      zlm(57) = 3.0d0*temp
      zlm(58) = -zlm(57)
      zlm(59) = -temp
      zlm(60) = +temp
      temp    = sqrt(165.0d0/fpi)/8.0d0
      zlm(61) = 21.0d0*temp
      zlm(62) = -14.0d0*temp
      zlm(63) = temp
      temp    = sqrt(11.0d0/fpi)/8.0d0
      zlm(64) = 63.0d0*temp
      zlm(65) = -70.0d0*temp
      zlm(66) = 15.0d0*temp
      zlm(67) = zlm(61)
      zlm(68) = zlm(62)
      zlm(69) = zlm(63)
      temp    = sqrt(1155.0d0/fpi)/2.0d0
      zlm(70) = 3.0d0*temp
      zlm(71) = -temp
      zlm(72) = -zlm(54)
      zlm(73) = -zlm(55)
      zlm(74) = -zlm(53)
      zlm(75) = -zlm(56)
      zlm(76) = sqrt(3465.0d0/fpi)/2.0d0
      zlm(77) = -zlm(76)
      zlm(78) = zlm(49)
      zlm(79) = zlm(48)
      zlm(80) = zlm(47)
c l=6
      temp    = sqrt(3003.0d0/(512.0d0*fpi))
      zlm(81) = 6.0d0*temp
      zlm(82) = -20.0d0*temp
      zlm(83) = zlm(81)
      zlm(84) = sqrt(9009.0d0/(128.0d0*fpi))
      zlm(85) = -10.0d0*zlm(84)
      zlm(86) = 5.0d0*zlm(84)
      temp    = sqrt(819.0d0/(256.0d0*fpi))
      zlm(87) = 11.0d0*temp
      zlm(88) = -66.0d0*temp
      zlm(89) = zlm(87)
      zlm(90) = -temp
      zlm(91) = 6.0d0*temp
      zlm(92) = -temp
      temp    = sqrt(1365.0d0/(128.0d0*fpi))
      zlm(93) = 11.0d0*temp
      zlm(94) = -33.0d0*temp
      zlm(95) = 9.0d0*temp
      zlm(96) = -3.0d0*temp
      temp    = sqrt(1365.0d0/(512.0d0*fpi))
      zlm(97) = 33.0d0*temp
      zlm(98) = -zlm(97)
      zlm(99) = -18.0d0*temp
      zlm(100) = +18.0d0*temp
      zlm(101) = temp
      zlm(102) = -temp
      temp     = sqrt(273.0d0/fpi)/8.0d0
      zlm(103) = 33.0d0*temp
      zlm(104) = -30.0d0*temp
      zlm(105) = 5.0d0*temp
      temp     = sqrt(13.0d0/fpi)/16.0d0
      zlm(106) = 231.0d0*temp
      zlm(107) = -315.0d0*temp
      zlm(108) = 105.0d0*temp
      zlm(109) = -5.0d0*temp
      zlm(110) = zlm(103)
      zlm(111) = zlm(104)
      zlm(112) = zlm(105)
      temp     = sqrt(1365.0d0/(128.0d0*fpi))
      zlm(113) = 33.0d0*temp
      zlm(114) = -18.0d0*temp
      zlm(115) = temp
      zlm(116) = -zlm(94)
      zlm(117) = -zlm(93)
      zlm(118) = -zlm(95)
      zlm(119) = -zlm(96)
      temp     = sqrt(819.0d0/fpi)/4.0d0
      zlm(120) = 11.0d0*temp
      zlm(121) = -zlm(120)
      zlm(122) = -temp
      zlm(123) = temp
      zlm(124) = zlm(86)
      zlm(125) = zlm(85)
      zlm(126) = zlm(84)
      zlm(127) = sqrt(3003.0d0/(512.0d0*fpi))
      zlm(128) = -15.0d0*zlm(127)
      zlm(129) = -zlm(128)
      zlm(130) = -zlm(127)
      Return
      End
#else
c compute tables by recursion for real spherical harmonics.  they
c are indexed by l, m and sigma.  the sequence number of the
c harmonic with quantum numbers l, m and sigma is given by
c l**2+2*m+1-sigma lmf(index) and lml(index) hold the positions of 
c the first and last terms of the harmonic in the lmx, lmy, lmz, 
c and zlm arrays. The harmonics with angular momentum l are generated 
c from those with angular momenta l-1 and l-2.
c for m = 0,1,2,...,l-1, the recursion relation
c z*Z(l-1,m,s) = sqrt(((l-m)*(l+m))/((2*l-1)*(2*l+1)))*Z(l,m,s)+
c                sqrt(((l+m-1)*(l-m-1))/((2*l-3)*(2*l-1)))*Z(l-2,m,s)
c is used.
c for m = l, the recursion relation
c x*Z(l-1,l-1,s)+(-1)**(1-s)*y*Z(l-1,l-1,1-s) =
c                sqrt((2*l))/((2*l+1)))*Z(l,l,s)
c is used.
c  l=0
      lmf(1) = 1
      lml(1) = 1
      lmx(1) = 0
      lmy(1) = 0
      lmz(1) = 0
      zlm(1) = 1.0D0/Sqrt_fpi
c l=1
      lmf(2) = 2
      lml(2) = 2
      lmx(2) = 0
      lmy(2) = 0
      lmz(2) = 1
      zlm(2) = sqrt(3.0D0)/Sqrt_fpi
      lmf(3) = 3
      lml(3) = 3
      lmx(3) = 0
      lmy(3) = 1
      lmz(3) = 0
      zlm(3) = zlm(2)
      lmf(4) = 4
      lml(4) = 4
      lmx(4) = 1
      lmy(4) = 0
      lmz(4) = 0
      zlm(4) = zlm(2)
      nterm=4

      do 270 lang=2,Maxang
        do 240 mang=0,lang-1
          anum = ((2*lang-1)*(2*lang+1))
          aden = ((lang-mang)*(lang+mang))
          coef1 = sqrt(anum/aden)
          anum = ((lang+mang-1)*(lang-mang-1)*(2*lang+1))
          aden = (2*lang-3)*aden
          coef2 = sqrt(anum/aden)
          nsigma=min(1,mang)
          do 230 isigma=nsigma,0,-1
            indexh=lang**2+2*mang+1-isigma
            lone=lang-1
            ltwo=lang-2
            ione=lone**2+2*mang+1-isigma
            itwo=ltwo**2+2*mang+1-isigma
            lmf(indexh)=lml(indexh-1)+1
            lml(indexh)=lml(indexh-1)
            nxy=(mang-isigma+2)/2
            iu=lmf(ione)+nxy-1
            do i=lmf(ione),iu
              lml(indexh)=lml(indexh)+1
              j=lml(indexh)
              lmx(j)=lmx(i)
              lmy(j)=lmy(i)
              lmz(j)=lmz(i)+1
              zlm(j)=zlm(i)*coef1
              nterm=nterm+1
            enddo
            if(ltwo.ge.mang) then
              il=iu+1
              do i=il,lml(ione)
                lml(indexh)=lml(indexh)+1
                j=lml(indexh)
                k=lmf(itwo)+i-il
                lmx(j)=lmx(k)
                lmy(j)=lmy(k)
                lmz(j)=lmz(k)
                zlm(j)=zlm(i)*coef1-zlm(k)*coef2
                nterm=nterm+1
              enddo
              il=lml(itwo)-nxy+1
              if(mod(lang-mang,2).eq.0) then
                do i=il,lml(itwo)
                  lml(indexh)=lml(indexh)+1
                  j=lml(indexh)
                  lmx(j)=lmx(i)
                  lmy(j)=lmy(i)
                  lmz(j)=lmz(i)
                  zlm(j)=-zlm(i)*coef2
                  nterm=nterm+1
                enddo
              endif
            endif
  230     enddo
  240   enddo

        anum = (2*lang+1)
        aden = (2*lang)
        coef = sqrt(anum/aden)
        mang=lang
        isigma=1
        indexh=lang**2+2*mang+1-isigma
        lmf(indexh)=lml(indexh-1)+1
        lml(indexh)=lml(indexh-1)
c isig:  index of the harmonic (l-1),(m-1),sigma
c isigm: index of the harmonic (l-1),(m-1),(1-sigma)
        isig=(lang-1)**2+2*(mang-1)+1-isigma
        isigm=(lang-1)**2+2*(mang-1)+isigma
        k=lmf(isigm)
        do i=lmf(isig),lml(isig)
          lml(indexh)=lml(indexh)+1
          j=lml(indexh)
          lmx(j)=lmx(i)+1
          lmy(j)=lmy(i)
          lmz(j)=lmz(i)
          zlm(j)=(zlm(i)+zlm(k))*coef
          k=k+1
          nterm=nterm+1
        enddo
        if(mod(mang,2).eq.1) then
          lml(indexh)=lml(indexh)+1
          j=lml(indexh)
          lmx(j)=lmx(k)
          lmy(j)=lmy(k)+1
          lmz(j)=lmz(k)
          zlm(j)=zlm(k)*coef
          nterm=nterm+1
        endif
        isigma=0
        indexh=lang**2+2*mang+1-isigma
c isig:  index of the harmonic (l-1),(m-1),sigma
c isigm: index of the harmonuc (l-1),(m-1),(1-sigma)
        isig=(lang-1)**2+2*(mang-1)+1-isigma
        isigm=(lang-1)**2+2*(mang-1)+isigma
        lmf(indexh)=lml(indexh-1)+1
        lml(indexh)=lmf(indexh)
        j=lml(indexh)
        i=lmf(isig)
        lmx(j)=lmx(i)+1
        lmy(j)=lmy(i)
        lmz(j)=lmz(i)
        zlm(j)=zlm(i)*coef
        nterm=nterm+1
        k=lmf(isigm)
        do  i=lmf(isig)+1,lml(isig)
          lml(indexh)=lml(indexh)+1
          j=lml(indexh)
          lmx(j)=lmx(i)+1
          lmy(j)=lmy(i)
          lmz(j)=lmz(i)
          zlm(j)=(zlm(i)-zlm(k))*coef
          k=k+1
          nterm=nterm+1
        enddo
        if(mod(mang,2).eq.0) then
          lml(indexh)=lml(indexh)+1
          j=lml(indexh)
          k=lml(isigm)
          lmx(j)=lmx(k)
          lmy(j)=lmy(k)+1
          lmz(j)=lmz(k)
          zlm(j)=-zlm(k)*coef
          nterm=nterm+1
        endif
  270 enddo
C---
CSS      ixy = 0
CSS      iz = 0
CSS      do 300 lang=1,lproju
CSS        do 290 mang=0,lang-1
CSS          nsigma=min(1,mang)
CSS          ndelta=max(0,1-mang)
CSS          anum = ((lang-mang)*(lang+mang+1))
CSS          aden = (2*(2-ndelta))
CSS          coef=sqrt(anum/aden)
CSS          do 280 isigma=nsigma,0,-1
CSS            isign=2*isigma-1
CSS            ixy = ixy+1
CSS            flmtx(1,ixy) = (isign)*coef
CSS            flmtx(2,ixy) = coef
CSS            if(mang.ne.0) then
CSS              iz=iz+1
CSS              flmtx(3,iz) = -(mang*isigma)
CSS            endif
CSS  280     enddo
CSS  290   enddo
CSS        iz=iz+1
CSS        flmtx(3,iz) = -(lang)
CSS  300 enddo
c  column and row indices for angular momentum matrix elements.
CSS      iadd = 1
CSS      do i=1,2*lproju-1
CSS        mc(1,i) = i
CSS        mc(2,i) = i
CSS        mc(3,i) = i+1
CSS        mr(1,i) = i+iadd
CSS        mr(2,i) = i+2
CSS        mr(3,i) = i+2
CSS        iadd = 4-iadd
CSS      enddo
C--
#endif
      Return
      End
