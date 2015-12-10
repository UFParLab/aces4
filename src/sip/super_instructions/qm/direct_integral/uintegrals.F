 
c ------------------------------------------------------------------- 
c  Used if 
c                IF m  < n
c                IF l  < s
c                IF m  < l
c                IF n  != s
c                IF n  != l
c                IF m  != s  
c ------------------------------------------------------------------- 
c 
      subroutine uscf_tp1a(nalpha_occupied, nbeta_occupied,
     &                nc1,nc2,nd1,nd2,xa,xb,va1,va2,vb1,vb2,
     &                vc1,vc2,
     &                vd1,vd2,integrals,intblk, a1, a2, b1, b2, 
     &                c1, c2, d1, d2,
     &                Fa,Fb)
      use, intrinsic :: ISO_C_BINDING
      implicit none
      integer va1, va2, vb1,vb2, vc1, vc2, vd1, vd2
      integer a1, a2, b1, b2, c1, c2, d1, d2
      integer a,b,c,d,sym 
      integer nc1,nc2,nd1,nd2,p  
      integer drange1, drange2
      integer crange1, crange2
      integer brange1, brange2
      integer arange1, arange2
      integer dstart, dend  
      integer delta_a, ainc, amax, jx,itype  
      integer nalpha_occupied, nbeta_occupied

      double precision Xa(nc1:nc2,nd1:nd2)
      double precision Xb(nc1:nc2,nd1:nd2)

      double precision Fa(nc1:nc2,nc1:nc2)
      double precision Fb(nc1:nc2,nc1:nc2)
      double precision cab(nc1:nc2,nd1:nd2)
      double precision ca_d(a1:a2)
      double precision cb_d(a1:a2)
      double precision ca_c(a1:a2)
      double precision cb_c(a1:a2)
      double precision intblk(a1:a2,b1:b2,c1:c2,d1:d2)
      double precision integrals(va1:va2,vb1:vb2,vc1:vc2,vd1:vd2)
      double precision dtemp, etemp, itemp(5), dcrit, factor  
      double precision dtemp2, etemp2   
      double precision dtemp3, etemp3   
      double precision dtemp6, etemp6   
      double precision dtemp7, etemp7   
      double precision etemp_cd, etempa_bc, etempa_bd, etempb_bc, 
     *                  etempb_bd 

      dcrit = 0.0 
      drange1 = max(vd1, d1)
      drange2 = min(vd2, d2)
      crange1 = max(vc1, c1)
      crange2 = min(vc2, c2)
      brange1 = max(vb1, b1)
      brange2 = min(vb2, b2)
      arange1 = max(va1, a1)
      arange2 = min(va2, a2)

c ----------------------------------------------------------------- 
      delta_a = arange2 - arange1
c ----------------------------------------------------------------- 

      do b = brange1, brange2  
      do a = arange1, arange2  
         cab(a,b) = 2.0*(xa(a,b) + xb(a,b)) 
      enddo 
      enddo 

      do d = drange1, drange2
         do a = arange1, arange2 
            ca_d(a) = xa(a,d) 
            cb_d(a) = xb(a,d) 
         enddo 
      do c = crange1, crange2

         dtemp = 2.0*(xa(c,d) + xb(c,d))  
         do a = arange1, arange2 
            ca_c(a) = xa(a,c) 
            cb_c(a) = xb(a,c) 
         enddo 
         etemp_cd = 0.0 

            do b = brange1, brange2

               dtemp2 = -1.0*xa(b,d)   
               dtemp3 = -1.0*xa(b,c)   
               dtemp6 = -1.0*xb(b,d)   
               dtemp7 = -1.0*xb(b,c)   

               etempa_bc = 0.0 
               etempa_bd = 0.0 
               etempb_bc = 0.0 
               etempb_bd = 0.0 

            do a = arange1, arange2, 5  
                  amax = min(arange2-a,4) 
                  ainc = 0 

c                 do p = 1, amax+1 
c                    itemp(p) = intblk(a+p-1,b,c,d) 
c                 enddo  
                                   itemp(1)  = intblk(a  ,b,c,d) 
                  if (amax .ge. 1) itemp(2)  = intblk(a+1,b,c,d) 
                  if (amax .ge. 2) itemp(3)  = intblk(a+2,b,c,d) 
                  if (amax .ge. 3) itemp(4)  = intblk(a+3,b,c,d) 
                  if (amax .ge. 4) itemp(5)  = intblk(a+4,b,c,d) 
33       continue 

c                 itemp  = intblk(a+ainc,b,c,d) 
c---------------------------------------------------- 

c---------------------------------------------------- 
                  etemp  = dtemp *itemp(ainc+1)  
                  Fa(a+ainc,b) = Fa(a+ainc,b) + etemp 
                  Fa(b,a+ainc) = Fa(b,a+ainc) + etemp 
                  Fb(a+ainc,b) = Fb(a+ainc,b) + etemp 
                  Fb(b,a+ainc) = Fb(b,a+ainc) + etemp  
c---------------------------------------------------- 

c---------------------------------------------------- 
                  etemp2 = dtemp2*itemp(ainc+1) 
                  Fa(a+ainc,c) = Fa(a+ainc,c) + etemp2 
                  Fa(c,a+ainc) = Fa(c,a+ainc) + etemp2 
c---------------------------------------------------- 

c---------------------------------------------------- 
                  etemp3 = dtemp3*itemp(ainc+1) 
                  Fa(a+ainc,d) = Fa(a+ainc,d) + etemp3 
                  Fa(d,a+ainc) = Fa(d,a+ainc) + etemp3 
c---------------------------------------------------- 

c---------------------------------------------------- 
                  etemp6 = dtemp6*itemp(ainc+1)  
                  Fb(a+ainc,c) = Fb(a+ainc,c) + etemp6  
                  Fb(c,a+ainc) = Fb(c,a+ainc) + etemp6  
c---------------------------------------------------- 

c---------------------------------------------------- 
                  etemp7 = dtemp7*itemp(ainc+1) 
                  Fb(a+ainc,d) = Fb(a+ainc,d) + etemp7  
                  Fb(d,a+ainc) = Fb(d,a+ainc) + etemp7  
c---------------------------------------------------- 

c------------------------------------------------------------- 
                  etemp_cd  = etemp_cd  + cab(a+ainc,b)*itemp(ainc+1)

                  etempa_bc = etempa_bc + ca_d(a+ainc)*itemp(ainc+1) 
                  etempa_bd = etempa_bd + ca_c(a+ainc)*itemp(ainc+1) 
                  etempb_bc = etempb_bc + cb_d(a+ainc)*itemp(ainc+1) 
                  etempb_bd = etempb_bd + cb_c(a+ainc)*itemp(ainc+1) 
c------------------------------------------------------------- 

                  ainc = ainc + 1 
                  if (ainc .le. amax) go to 33 

            enddo

               Fa(b,c) = Fa(b,c) - etempa_bc 
               Fa(c,b) = Fa(c,b) - etempa_bc 
               Fa(b,d) = Fa(b,d) - etempa_bd 
               Fa(d,b) = Fa(d,b) - etempa_bd  

               Fb(b,c) = Fb(b,c) - etempb_bc 
               Fb(c,b) = Fb(c,b) - etempb_bc  
               Fb(b,d) = Fb(b,d) - etempb_bd  
               Fb(d,b) = Fb(d,b) - etempb_bd 

            enddo

            Fa(c,d) = Fa(c,d) + etemp_cd  
            Fa(d,c) = Fa(d,c) + etemp_cd  

            Fb(c,d) = Fb(c,d) + etemp_cd 
            Fb(d,c) = Fb(d,c) + etemp_cd 
      enddo
      enddo

      return
      end
c
      subroutine uscf_tp1(nalpha_occupied, nbeta_occupied,
     &                nc1,nc2,nd1,nd2,ca,cb,va1,va2,vb1,vb2,
     &                vc1,vc2,
     &                vd1,vd2,intblk, a1, a2, b1, b2, c1, c2, d1, d2,
     &                Fa,Fb)
      use, intrinsic :: ISO_C_BINDING
      implicit none
      integer va1, va2, vb1,vb2, vc1, vc2, vd1, vd2
      integer a1, a2, b1, b2, c1, c2, d1, d2
      integer a,b,c,d,sym 
      integer nc1,nc2,nd1,nd2,p  
      integer drange1, drange2
      integer crange1, crange2
      integer brange1, brange2
      integer arange1, arange2
      integer dstart, dend  
      integer delta_a, ainc, amax, jx,itype  
      integer nalpha_occupied, nbeta_occupied

      double precision ca(nc1:nc2,nd1:nd2)
      double precision cb(nc1:nc2,nd1:nd2)

      double precision Xa(nc1:nc2,nd1:nd2)
      double precision Xb(nc1:nc2,nd1:nd2)

      double precision Fa(nc1:nc2,nc1:nc2)
      double precision Fb(nc1:nc2,nc1:nc2)
      double precision cab(nc1:nc2,nd1:nd2)
      double precision ca_d(a1:a2)
      double precision cb_d(a1:a2)
      double precision ca_c(a1:a2)
      double precision cb_c(a1:a2)
      double precision intblk(a1:a2,b1:b2,c1:c2,d1:d2)
      double precision dtemp, etemp, itemp, dcrit, factor  
      double precision dtemp2, etemp2   
      double precision dtemp3, etemp3   
      double precision dtemp6, etemp6   
      double precision dtemp7, etemp7   
      double precision etemp_cd, etempa_bc, etempa_bd, etempb_bc, 
     *                  etempb_bd 

      dcrit = 0.0 
      drange1 = max(vd1, d1)
      drange2 = min(vd2, d2)
      crange1 = max(vc1, c1)
      crange2 = min(vc2, c2)
      brange1 = max(vb1, b1)
      brange2 = min(vb2, b2)
      arange1 = max(va1, a1)
      arange2 = min(va2, a2)

      dstart = min(arange1,brange1,crange1,drange1) 
      dend = max(arange2,brange2,crange2,drange2) 

      do d = dstart, dend ! nd1, nd2 
      do c = dstart, dend ! nc1, nc2 

             dtemp = 0.0 
             do p = 1, nalpha_occupied ! nd1,nd2
                dtemp = dtemp + ca(c,p)*ca(d,p)
             enddo 
             xa(c,d) = dtemp 

             dtemp = 0.0 
             do p = 1, nbeta_occupied ! nd1,nd2
                dtemp = dtemp + cb(c,p)*cb(d,p)
             enddo 
             xb(c,d) = dtemp 

      enddo 
      enddo

c ----------------------------------------------------------------- 
      delta_a = arange2 - arange1
c ----------------------------------------------------------------- 

      do b = brange1, brange2  
      do a = arange1, arange2  
         cab(a,b) = 2.0*(xa(a,b) + xb(a,b)) 
      enddo 
      enddo 

      do d = drange1, drange2
         do a = arange1, arange2 
            ca_d(a) = xa(a,d) 
            cb_d(a) = xb(a,d) 
         enddo 
      do c = crange1, crange2

         dtemp = 2.0*(xa(c,d) + xb(c,d))  
         do a = arange1, arange2 
            ca_c(a) = xa(a,c) 
            cb_c(a) = xb(a,c) 
         enddo 
         etemp_cd = 0.0 

            do b = brange1, brange2

               dtemp2 = -1.0*xa(b,d)   
               dtemp3 = -1.0*xa(b,c)   
               dtemp6 = -1.0*xb(b,d)   
               dtemp7 = -1.0*xb(b,c)   

               etempa_bc = 0.0 
               etempa_bd = 0.0 
               etempb_bc = 0.0 
               etempb_bd = 0.0 

            do a = arange1, arange2, 5  
                  amax = min(arange2-a,4) 

                  ainc = 0 
33       continue 

                  itemp  = intblk(a+ainc,b,c,d) 
c---------------------------------------------------- 

c---------------------------------------------------- 
                  etemp  = dtemp *itemp 
                  Fa(a+ainc,b) = Fa(a+ainc,b) + etemp 
                  Fa(b,a+ainc) = Fa(b,a+ainc) + etemp 
                  Fb(a+ainc,b) = Fb(a+ainc,b) + etemp 
                  Fb(b,a+ainc) = Fb(b,a+ainc) + etemp  
c---------------------------------------------------- 

c---------------------------------------------------- 
                  etemp2 = dtemp2*itemp 
                  Fa(a+ainc,c) = Fa(a+ainc,c) + etemp2 
                  Fa(c,a+ainc) = Fa(c,a+ainc) + etemp2 
c---------------------------------------------------- 

c---------------------------------------------------- 
                  etemp3 = dtemp3*itemp 
                  Fa(a+ainc,d) = Fa(a+ainc,d) + etemp3 
                  Fa(d,a+ainc) = Fa(d,a+ainc) + etemp3 
c---------------------------------------------------- 

c---------------------------------------------------- 
                  etemp6 = dtemp6*itemp  
                  Fb(a+ainc,c) = Fb(a+ainc,c) + etemp6  
                  Fb(c,a+ainc) = Fb(c,a+ainc) + etemp6  
c---------------------------------------------------- 

c---------------------------------------------------- 
                  etemp7 = dtemp7*itemp 
                  Fb(a+ainc,d) = Fb(a+ainc,d) + etemp7  
                  Fb(d,a+ainc) = Fb(d,a+ainc) + etemp7  
c---------------------------------------------------- 

c------------------------------------------------------------- 
                  etemp_cd  = etemp_cd  + cab(a+ainc,b)*itemp

                  etempa_bc = etempa_bc + ca_d(a+ainc)*itemp 
                  etempa_bd = etempa_bd + ca_c(a+ainc)*itemp 
                  etempb_bc = etempb_bc + cb_d(a+ainc)*itemp 
                  etempb_bd = etempb_bd + cb_c(a+ainc)*itemp 
c------------------------------------------------------------- 

                  ainc = ainc + 1 
                  if (ainc .le. amax) go to 33 

            enddo

               Fa(b,c) = Fa(b,c) - etempa_bc 
               Fa(c,b) = Fa(c,b) - etempa_bc 
               Fa(b,d) = Fa(b,d) - etempa_bd 
               Fa(d,b) = Fa(d,b) - etempa_bd  

               Fb(b,c) = Fb(b,c) - etempb_bc 
               Fb(c,b) = Fb(c,b) - etempb_bc  
               Fb(b,d) = Fb(b,d) - etempb_bd  
               Fb(d,b) = Fb(d,b) - etempb_bd 

            enddo

            Fa(c,d) = Fa(c,d) + etemp_cd  
            Fa(d,c) = Fa(d,c) + etemp_cd  

            Fb(c,d) = Fb(c,d) + etemp_cd 
            Fb(d,c) = Fb(d,c) + etemp_cd 
      enddo
      enddo

      return
      end
c
c ------------------------------------------------------------------- 
c  Used if 
c ------------------------------------------------------------------- 
c 
      subroutine uscf_tp3a(nalpha_occupied, nbeta_occupied,
     &                nc1,nc2,nd1,nd2,ca,cb,va1,va2,vb1,vb2,
     &                vc1,vc2,
     &                vd1,vd2,integrals,intblk, 
     &                a1, a2, b1, b2, c1, c2, d1, d2,
     &                Fa,Fb)
      use, intrinsic :: ISO_C_BINDING
      implicit none
      integer va1, va2, vb1,vb2, vc1, vc2, vd1, vd2
      integer a1, a2, b1, b2, c1, c2, d1, d2
      integer a,b,c,d,sym 
      integer nc1,nc2,nd1,nd2,p  
      integer drange1, drange2
      integer crange1, crange2
      integer brange1, brange2
      integer arange1, arange2
      integer delta_a,itype  
      integer nalpha_occupied, nbeta_occupied

      double precision ca(nc1:nc2,nd1:nd2)
      double precision cb(nc1:nc2,nd1:nd2)
      double precision Fa(nc1:nc2,nc1:nc2)
      double precision Fb(nc1:nc2,nc1:nc2)
      double precision intblk(a1:a2,b1:b2,c1:c2,d1:d2)
      double precision integrals(va1:va2,vb1:vb2,vc1:vc2,vd1:vd2)
      double precision dtemp, etemp, dcrit, factor  

      dcrit = 0.0 
      drange1 = max(vd1, d1)
      drange2 = min(vd2, d2)
      crange1 = max(vc1, c1)
      crange2 = min(vc2, c2)
      brange1 = max(vb1, b1)
      brange2 = min(vb2, b2)
      arange1 = max(va1, a1)
      arange2 = min(va2, a2)

c ----------------------------------------------------------------- 

      do d = drange1, drange2
      do c = crange1, crange2
         dtemp = ca(c,d) + cb(c,d)  
         dtemp = 2.0d0*dtemp  
         if (dabs(dtemp) .gt. dcrit) then 
            do b = brange1, brange2
            do a = arange1, arange2
               etemp = dtemp*intblk(a,b,c,d) 
               Fa(a,b) = Fa(a,b) + etemp ! intblk(a,b,c,d)*dtemp 
               Fa(b,a) = Fa(b,a) + etemp ! intblk(a,b,c,d)*dtemp 

               Fb(a,b) = Fb(a,b) + etemp ! intblk(a,b,c,d)*dtemp 
               Fb(b,a) = Fb(b,a) + etemp ! intblk(a,b,c,d)*dtemp 
            enddo
            enddo
         endif 
      enddo
      enddo

      do a = arange1, arange2
      do b = brange1, brange2
         dtemp = ca(a,b) + cb(a,b)  
         dtemp = 2.0d0*dtemp  
         if (dabs(dtemp) .gt. dcrit) then 
            do c = crange1, crange2
            do d = drange1, drange2
               etemp = dtemp*intblk(a,b,c,d) 
               Fa(c,d) = Fa(c,d) + etemp ! intblk(a,b,c,d)*dtemp 
               Fa(d,c) = Fa(d,c) + etemp ! intblk(a,b,c,d)*dtemp 

               Fb(c,d) = Fb(c,d) + etemp ! intblk(a,b,c,d)*dtemp 
               Fb(d,c) = Fb(d,c) + etemp ! intblk(a,b,c,d)*dtemp 
            enddo
            enddo
         endif 
      enddo
      enddo

      do b = brange1, brange2
      do d = drange1, drange2
         dtemp = ca(b,d) 
         dtemp = -1.0d0*dtemp  
         if (dabs(dtemp) .gt. dcrit) then 
            do a = arange1, arange2
            do c = crange1, crange2
               etemp = dtemp*intblk(a,b,c,d) 
               Fa(a,c) = Fa(a,c) + etemp ! intblk(a,b,c,d)*dtemp 
               Fa(c,a) = Fa(c,a) + etemp ! intblk(a,b,c,d)*dtemp 
            enddo
            enddo
         endif 
      enddo
      enddo

      do b = brange1, brange2
      do c = crange1, crange2
         dtemp = ca(b,c) 
         dtemp = -1.0d0*dtemp  
         if (dabs(dtemp) .gt. dcrit) then 
            do a = arange1, arange2
            do d = drange1, drange2
               etemp = dtemp*intblk(a,b,c,d) 
               Fa(a,d) = Fa(a,d) + etemp ! intblk(a,b,c,d)*dtemp 
               Fa(d,a) = Fa(d,a) + etemp ! intblk(a,b,c,d)*dtemp 
            enddo
            enddo
         endif 
      enddo
      enddo

      do a = arange1, arange2
      do d = drange1, drange2
         dtemp = ca(a,d) 
         dtemp = -1.0d0*dtemp  
         if (dabs(dtemp) .gt. dcrit) then 
            do b = brange1, brange2
            do c = crange1, crange2
               etemp = dtemp*intblk(a,b,c,d) 
               Fa(b,c) = Fa(b,c) + etemp ! intblk(a,b,c,d)*dtemp 
               Fa(c,b) = Fa(c,b) + etemp ! intblk(a,b,c,d)*dtemp 
            enddo
            enddo
         endif 
      enddo
      enddo

      do a = arange1, arange2
      do c = crange1, crange2
         dtemp = ca(a,c) 
         dtemp = -1.0d0*dtemp  
         if (dabs(dtemp) .gt. dcrit) then 
            do b = brange1, brange2
            do d = drange1, drange2
               etemp = dtemp*intblk(a,b,c,d) 
               Fa(b,d) = Fa(b,d) + etemp ! intblk(a,b,c,d)*dtemp 
               Fa(d,b) = Fa(d,b) + etemp ! intblk(a,b,c,d)*dtemp 
            enddo
            enddo
         endif 
      enddo
      enddo

      do b = brange1, brange2
      do d = drange1, drange2
         dtemp = cb(b,d) 
         dtemp = -1.0d0*dtemp  
         if (dabs(dtemp) .gt. dcrit) then 
            do a = arange1, arange2
            do c = crange1, crange2
               etemp = dtemp*intblk(a,b,c,d) 
               Fb(a,c) = Fb(a,c) + etemp ! intblk(a,b,c,d)*dtemp 
               Fb(c,a) = Fb(c,a) + etemp ! intblk(a,b,c,d)*dtemp 
            enddo
            enddo
         endif 
      enddo
      enddo

      do b = brange1, brange2
      do c = crange1, crange2
         dtemp = cb(b,c) 
         dtemp = -1.0d0*dtemp  
         if (dabs(dtemp) .gt. dcrit) then 
            do a = arange1, arange2
            do d = drange1, drange2
               etemp = dtemp*intblk(a,b,c,d) 
               Fb(a,d) = Fb(a,d) + etemp ! intblk(a,b,c,d)*dtemp 
               Fb(d,a) = Fb(d,a) + etemp ! intblk(a,b,c,d)*dtemp 
            enddo
            enddo
         endif 
      enddo
      enddo

      do a = arange1, arange2
      do d = drange1, drange2
         dtemp = cb(a,d) 
         dtemp = -1.0d0*dtemp  
         if (dabs(dtemp) .gt. dcrit) then 
            do b = brange1, brange2
            do c = crange1, crange2
               etemp = dtemp*intblk(a,b,c,d) 
               Fb(b,c) = Fb(b,c) + etemp ! intblk(a,b,c,d)*dtemp 
               Fb(c,b) = Fb(c,b) + etemp ! intblk(a,b,c,d)*dtemp 
            enddo
            enddo
         endif 
      enddo
      enddo

      do a = arange1, arange2
      do c = crange1, crange2
         dtemp = cb(a,c) 
         dtemp = -1.0d0*dtemp  
         if (dabs(dtemp) .gt. dcrit) then 
            do b = brange1, brange2
            do d = drange1, drange2
               etemp = dtemp*intblk(a,b,c,d) 
               Fb(b,d) = Fb(b,d) + etemp ! intblk(a,b,c,d)*dtemp 
               Fb(d,b) = Fb(d,b) + etemp ! intblk(a,b,c,d)*dtemp 
            enddo
            enddo
         endif 
      enddo
      enddo

      return
      end
c
c ------------------------------------------------------------------- 
c  Used if 
c ------------------------------------------------------------------- 
c 
      subroutine uscf_tp4a(nalpha_occupied, nbeta_occupied,
     &                nc1,nc2,nd1,nd2,xa,xb,va1,va2,vb1,vb2,
     &                vc1,vc2,
     &                vd1,vd2,integrals,intblk, 
     &                a1, a2, b1, b2, c1, c2, d1, d2,
     &                Fa,Fb)
      use, intrinsic :: ISO_C_BINDING
      implicit none
      integer va1, va2, vb1,vb2, vc1, vc2, vd1, vd2
      integer a1, a2, b1, b2, c1, c2, d1, d2
      integer a,b,c,d,sym 
      integer nc1,nc2,nd1,nd2,p  
      integer drange1, drange2
      integer crange1, crange2
      integer brange1, brange2
      integer arange1, arange2
      integer dstart, dend  
      integer delta_a, ainc, amax, jx,itype  
      integer nalpha_occupied, nbeta_occupied

      double precision Xa(nc1:nc2,nd1:nd2)
      double precision Xb(nc1:nc2,nd1:nd2)

      double precision Fa(nc1:nc2,nc1:nc2)
      double precision Fb(nc1:nc2,nc1:nc2)
      double precision cab(nc1:nc2,nd1:nd2)
      double precision ca_d(a1:a2)
      double precision cb_d(a1:a2)
      double precision ca_c(a1:a2)
      double precision cb_c(a1:a2) 
      double precision intblk(a1:a2,b1:b2,c1:c2,d1:d2)
      double precision integrals(va1:va2,vb1:vb2,vc1:vc2,vd1:vd2)
      double precision dtemp, etemp, itemp(5), dcrit, factor  
      double precision dtemp1, etemp1 
      double precision dtemp2, etemp2 
      double precision dtemp3, etemp3 
      double precision dtemp4, etemp4 
      double precision dtemp5, etemp5 
      double precision dtemp6, etemp6 
      double precision dtemp7, etemp7 
      double precision dtemp8, etemp8 
      double precision dtemp9, etemp9 
      double precision etemp_cd,etempa_bc,etempa_bd,etempb_bc,etempb_bd 

      dcrit = 0.0 
      drange1 = max(vd1, d1)
      drange2 = min(vd2, d2)
      crange1 = max(vc1, c1)
      crange2 = min(vc2, c2)
      brange1 = max(vb1, b1)
      brange2 = min(vb2, b2)
      arange1 = max(va1, a1)
      arange2 = min(va2, a2)

c ----------------------------------------------------------------- 

      do b = brange1, brange2
      do a = arange1, arange2
           cab(a,b) = 2.0*(xa(a,b) + xb(a,b))  
      enddo 
      enddo 

      delta_a = arange2 - arange1 

      do d = drange1, drange2
         do a = arange1, arange2
            ca_d(a) = xa(a,d)
            cb_d(a) = xb(a,d)
         enddo 
      do c = crange1, crange2

         dtemp = 2.0*(xa(c,d) + xb(c,d))  
         do a = arange1, arange2
            ca_c(a) = xa(a,c)
            cb_c(a) = xb(a,c)
         enddo 
         etemp_cd = 0.0 

            do b = brange1, brange2

               dtemp2 = -1.0*xa(b,d) 
               dtemp3 = -1.0*xa(b,c) 
               dtemp6 = -1.0*xb(b,d) 
               dtemp7 = -1.0*xb(b,c) 

               etempa_bc = 0.0  
               etempa_bd = 0.0  
               etempb_bc = 0.0  
               etempb_bd = 0.0  

            do a = arange1, arange2, 5 
                 amax = min(arange2-a,4) 
                 ainc = 0 

                                   itemp(1)  = intblk(a  ,b,c,d)
                  if (amax .ge. 1) itemp(2)  = intblk(a+1,b,c,d)
                  if (amax .ge. 2) itemp(3)  = intblk(a+2,b,c,d)
                  if (amax .ge. 3) itemp(4)  = intblk(a+3,b,c,d)
                  if (amax .ge. 4) itemp(5)  = intblk(a+4,b,c,d) 
33           continue 

c              itemp = intblk(a+ainc,b,c,d) 

               etemp  = dtemp *itemp(ainc+1)  
               etemp2 = dtemp2*itemp(ainc+1) 
               etemp3 = dtemp3*itemp(ainc+1) 
               etemp6 = dtemp6*itemp(ainc+1) 
               etemp7 = dtemp7*itemp(ainc+1) 

               Fa(a+ainc,b) = Fa(a+ainc,b) + etemp 
               Fa(b,a+ainc) = Fa(b,a+ainc) + etemp 

               Fb(a+ainc,b) = Fb(a+ainc,b) + etemp 
               Fb(b,a+ainc) = Fb(b,a+ainc) + etemp 

               Fa(a+ainc,c) = Fa(a+ainc,c) + etemp2 
               Fa(c,a+ainc) = Fa(c,a+ainc) + etemp2 

               Fa(a+ainc,d) = Fa(a+ainc,d) + etemp3 
               Fa(d,a+ainc) = Fa(d,a+ainc) + etemp3 

               Fb(a+ainc,c) = Fb(a+ainc,c) + etemp6 
               Fb(c,a+ainc) = Fb(c,a+ainc) + etemp6 

               Fb(a+ainc,d) = Fb(a+ainc,d) + etemp7 
               Fb(d,a+ainc) = Fb(d,a+ainc) + etemp7 

               etempa_bc = etempa_bc + ca_d(a+ainc)*itemp(ainc+1)  
               etempa_bd = etempa_bd + ca_c(a+ainc)*itemp(ainc+1)  
               etempb_bc = etempb_bc + cb_d(a+ainc)*itemp(ainc+1)  
               etempb_bd = etempb_bd + cb_c(a+ainc)*itemp(ainc+1)  

               etemp_cd = etemp_cd + cab(a+ainc,b)*itemp(ainc+1)  

                  ainc = ainc + 1
                  if (ainc .le. amax) go to 33

            enddo

               Fa(b,c) = Fa(b,c) - etempa_bc  
               Fa(c,b) = Fa(c,b) - etempa_bc 

               Fa(b,d) = Fa(b,d) - etempa_bd 
               Fa(d,b) = Fa(d,b) - etempa_bd 

               Fb(b,c) = Fb(b,c) - etempb_bc  
               Fb(c,b) = Fb(c,b) - etempb_bc 

               Fb(b,d) = Fb(b,d) - etempb_bd 
               Fb(d,b) = Fb(d,b) - etempb_bd 

            enddo

            Fa(c,d) = Fa(c,d) + etemp_cd  
            Fa(d,c) = Fa(d,c) + etemp_cd  

            Fb(c,d) = Fb(c,d) + etemp_cd 
            Fb(d,c) = Fb(d,c) + etemp_cd 

      enddo
      enddo

      return
      end
c
 
c ------------------------------------------------------------------- 
c  Used if 
c ------------------------------------------------------------------- 
c 
      subroutine uscf_tp4(nalpha_occupied, nbeta_occupied,
     &                nc1,nc2,nd1,nd2,ca,cb,va1,va2,vb1,vb2,
     &                vc1,vc2,
     &                vd1,vd2,intblk, a1, a2, b1, b2, c1, c2, d1, d2,
     &                Fa,Fb)
      use, intrinsic :: ISO_C_BINDING
      implicit none
      integer va1, va2, vb1,vb2, vc1, vc2, vd1, vd2
      integer a1, a2, b1, b2, c1, c2, d1, d2
      integer a,b,c,d,sym 
      integer nc1,nc2,nd1,nd2,p  
      integer drange1, drange2
      integer crange1, crange2
      integer brange1, brange2
      integer arange1, arange2
      integer dstart, dend  
      integer delta_a, ainc, amax, jx,itype  
      integer nalpha_occupied, nbeta_occupied

      double precision ca(nc1:nc2,nd1:nd2)
      double precision cb(nc1:nc2,nd1:nd2)

      double precision Xa(nc1:nc2,nd1:nd2)
      double precision Xb(nc1:nc2,nd1:nd2)

      double precision Fa(nc1:nc2,nc1:nc2)
      double precision Fb(nc1:nc2,nc1:nc2)
      double precision cab(nc1:nc2,nd1:nd2)
      double precision ca_d(a1:a2)
      double precision cb_d(a1:a2)
      double precision ca_c(a1:a2)
      double precision cb_c(a1:a2) 
      double precision intblk(a1:a2,b1:b2,c1:c2,d1:d2)
      double precision dtemp, etemp, itemp, dcrit, factor  
      double precision dtemp1, etemp1 
      double precision dtemp2, etemp2 
      double precision dtemp3, etemp3 
      double precision dtemp4, etemp4 
      double precision dtemp5, etemp5 
      double precision dtemp6, etemp6 
      double precision dtemp7, etemp7 
      double precision dtemp8, etemp8 
      double precision dtemp9, etemp9 
      double precision etemp_cd,etempa_bc,etempa_bd,etempb_bc,etempb_bd 

      dcrit = 0.0 
      drange1 = max(vd1, d1)
      drange2 = min(vd2, d2)
      crange1 = max(vc1, c1)
      crange2 = min(vc2, c2)
      brange1 = max(vb1, b1)
      brange2 = min(vb2, b2)
      arange1 = max(va1, a1)
      arange2 = min(va2, a2)

c ----------------------------------------------------------------- 


      dstart = min(arange1,brange1,crange1,drange1)
      dend = max(arange2,brange2,crange2,drange2)

      do d = dstart, dend ! nd1, nd2 
      do c = dstart, dend ! nc1, nc2 

             dtemp = 0.0
             do p = 1, nalpha_occupied ! nd1,nd2
                dtemp = dtemp + ca(c,p)*ca(d,p)
             enddo
             xa(c,d) = dtemp

             dtemp = 0.0
             do p = 1, nbeta_occupied ! nd1,nd2
                dtemp = dtemp + cb(c,p)*cb(d,p)
             enddo
             xb(c,d) = dtemp

      enddo
      enddo


      do b = brange1, brange2
      do a = arange1, arange2
           cab(a,b) = 2.0*(xa(a,b) + xb(a,b))  
      enddo 
      enddo 

      delta_a = arange2 - arange1 

      do d = drange1, drange2
         do a = arange1, arange2
            ca_d(a) = xa(a,d)
            cb_d(a) = xb(a,d)
         enddo 
      do c = crange1, crange2

         dtemp = 2.0*(xa(c,d) + xb(c,d))  
         do a = arange1, arange2
            ca_c(a) = xa(a,c)
            cb_c(a) = xb(a,c)
         enddo 
         etemp_cd = 0.0 

            do b = brange1, brange2

               dtemp2 = -1.0*xa(b,d) 
               dtemp3 = -1.0*xa(b,c) 
               dtemp6 = -1.0*xb(b,d) 
               dtemp7 = -1.0*xb(b,c) 

               etempa_bc = 0.0  
               etempa_bd = 0.0  
               etempb_bc = 0.0  
               etempb_bd = 0.0  

            do a = arange1, arange2, 5 
                 amax = min(arange2-a,4) 
                 ainc = 0 
33           continue 

               itemp = intblk(a+ainc,b,c,d) 

               etemp  = dtemp *itemp 
               etemp2 = dtemp2*itemp 
               etemp3 = dtemp3*itemp 
               etemp6 = dtemp6*itemp 
               etemp7 = dtemp7*itemp 

               Fa(a+ainc,b) = Fa(a+ainc,b) + etemp 
               Fa(b,a+ainc) = Fa(b,a+ainc) + etemp 

               Fb(a+ainc,b) = Fb(a+ainc,b) + etemp 
               Fb(b,a+ainc) = Fb(b,a+ainc) + etemp 

               Fa(a+ainc,c) = Fa(a+ainc,c) + etemp2 
               Fa(c,a+ainc) = Fa(c,a+ainc) + etemp2 

               Fa(a+ainc,d) = Fa(a+ainc,d) + etemp3 
               Fa(d,a+ainc) = Fa(d,a+ainc) + etemp3 

               Fb(a+ainc,c) = Fb(a+ainc,c) + etemp6 
               Fb(c,a+ainc) = Fb(c,a+ainc) + etemp6 

               Fb(a+ainc,d) = Fb(a+ainc,d) + etemp7 
               Fb(d,a+ainc) = Fb(d,a+ainc) + etemp7 

               etempa_bc = etempa_bc + ca_d(a+ainc)*itemp  
               etempa_bd = etempa_bd + ca_c(a+ainc)*itemp  
               etempb_bc = etempb_bc + cb_d(a+ainc)*itemp  
               etempb_bd = etempb_bd + cb_c(a+ainc)*itemp  

               etemp_cd = etemp_cd + cab(a+ainc,b)*itemp  

                  ainc = ainc + 1
                  if (ainc .le. amax) go to 33

            enddo

               Fa(b,c) = Fa(b,c) - etempa_bc  
               Fa(c,b) = Fa(c,b) - etempa_bc 

               Fa(b,d) = Fa(b,d) - etempa_bd 
               Fa(d,b) = Fa(d,b) - etempa_bd 

               Fb(b,c) = Fb(b,c) - etempb_bc  
               Fb(c,b) = Fb(c,b) - etempb_bc 

               Fb(b,d) = Fb(b,d) - etempb_bd 
               Fb(d,b) = Fb(d,b) - etempb_bd 

            enddo

            Fa(c,d) = Fa(c,d) + etemp_cd  
            Fa(d,c) = Fa(d,c) + etemp_cd  

            Fb(c,d) = Fb(c,d) + etemp_cd 
            Fb(d,c) = Fb(d,c) + etemp_cd 

      enddo
      enddo

      return
      end
c
c ------------------------------------------------------------------- 
c  Used if 
c ------------------------------------------------------------------- 
c 
      subroutine uscf_tp6a(nalpha_occupied, nbeta_occupied,
     &                jx,nc1,nc2,nd1,nd2,ca,cb,va1,va2,vb1,vb2,
     &                vc1,vc2, vd1,vd2, 
     &                integrals, intblk, a1, a2, b1, b2, 
     &                c1, c2, d1, d2,
     &                Fa,Fb)
      use, intrinsic :: ISO_C_BINDING
      implicit none
      integer va1, va2, vb1,vb2, vc1, vc2, vd1, vd2
      integer a1, a2, b1, b2, c1, c2, d1, d2
      integer a,b,c,d,sym 
      integer nc1,nc2,nd1,nd2,p  
      integer drange1, drange2
      integer crange1, crange2
      integer brange1, brange2
      integer arange1, arange2
      integer dstart, dend  
      integer delta_a, amax, ainc, jx, itype  
      integer nalpha_occupied, nbeta_occupied

      double precision ca(nc1:nc2,nd1:nd2)
      double precision cb(nc1:nc2,nd1:nd2)

      double precision Xa(nc1:nc2,nd1:nd2)
      double precision Xb(nc1:nc2,nd1:nd2)

      double precision Fa(nc1:nc2,nc1:nc2)
      double precision Fb(nc1:nc2,nc1:nc2)
      double precision intblk(a1:a2,b1:b2,c1:c2,d1:d2)
      double precision integrals(va1:va2,vb1:vb2,vc1:vc2,vd1:vd2)
      double precision dtemp, etemp, itemp(5), dcrit, factor  
      double precision dtemp1, etemp1 
      double precision dtemp2, etemp2 
      double precision sa, sb
     

      dcrit = 0.0 
      drange1 = max(vd1, d1)
      drange2 = min(vd2, d2)
      crange1 = max(vc1, c1)
      crange2 = min(vc2, c2)
      brange1 = max(vb1, b1)
      brange2 = min(vb2, b2)
      arange1 = max(va1, a1)
      arange2 = min(va2, a2)

c Form the output  integral array 

      do d = d1, d2  
      do c = c1, c2  
            do b = b1, b2  
            do a = a1, a2  
                integrals(a,b,c,d) = intblk(a,b,c,d) 
c               write(6,*) ' INT6:',a,b,c,d,intblk(a,b,c,d)
c    * , integrals(a,b,c,d) 
            enddo 
            enddo 
       enddo 
       enddo 
#ifdef _DEBUG_LVL0
       sa=0.0d0
       sb=0.0d0
       do a = nc1,nc2
       do b = nd1,nd2
          sa = ca(a,b) *ca(a,b) + sa
          sb = cb(a,b) *cb(a,b) + sb
       enddo
       enddo
       Write(6,"(a,F15.8)") "Checksum of DA =",sa
       Write(6,"(a,F15.8)") "Checksum of DB =",sb
#endif 
c

c ----------------------------------------------------------------- 

c     do d = drange1, drange2
c     do c = crange1, crange2

c           do b = brange1, brange2
c           do a = arange1, arange2  
c                 write(6,*) ' 2EL INT :', a, b, c, d, intblk(a  ,b,c,d)
c           enddo 
c           enddo 
c     enddo 
c     enddo 

      dstart = min(arange1,brange1,crange1,drange1)
      dend = max(arange2,brange2,crange2,drange2)

      do d = dstart, dend ! nd1, nd2 
      do c = dstart, dend ! nc1, nc2 

      jx = 2 
      if (jx .eq. 1) then

             dtemp = 0.0
             do p = 1, nalpha_occupied ! nd1,nd2
                dtemp = dtemp + ca(c,p)*ca(d,p)
             enddo
             xa(c,d) = dtemp

             dtemp = 0.0
             do p = 1, nbeta_occupied ! nd1,nd2
                dtemp = dtemp + cb(c,p)*cb(d,p)
             enddo
             xb(c,d) = dtemp

      else

             xa(c,d) = ca(c,d)
             xb(c,d) = cb(c,d)

      endif

      enddo
      enddo

      do d = drange1, drange2
      do c = crange1, crange2

         dtemp = xa(c,d) + xb(c,d)  

            do b = brange1, brange2

               dtemp1 = -1.0*xa(b,d)   
               dtemp2 = -1.0*xb(b,d)   

            do a = arange1, arange2, 5 
                  amax = min(arange2-a,4)
                  ainc = 0 

                                   itemp(1)  = intblk(a  ,b,c,d)
                  if (amax .ge. 1) itemp(2)  = intblk(a+1,b,c,d)
                  if (amax .ge. 2) itemp(3)  = intblk(a+2,b,c,d)
                  if (amax .ge. 3) itemp(4)  = intblk(a+3,b,c,d)
                  if (amax .ge. 4) itemp(5)  = intblk(a+4,b,c,d) 
33       continue 

c              itemp = intblk(a+ainc,b,c,d) 
               etemp  = dtemp *itemp(ainc+1)  
               etemp1 = dtemp1*itemp(ainc+1)  
               etemp2 = dtemp2*itemp(ainc+1) 

               Fa(a+ainc,b) = Fa(a+ainc,b) + etemp 
               Fb(a+ainc,b) = Fb(a+ainc,b) + etemp 

               Fa(a+ainc,c) = Fa(a+ainc,c) + etemp1 

               Fb(a+ainc,c) = Fb(a+ainc,c) + etemp2 

               ainc = ainc + 1 
               if (ainc .le. amax) go to 33 
            enddo
            enddo
      enddo
      enddo

#ifdef _DEBUG_LVL0
       sa=0.0d0
       sb=0.0d0
       do a = nc1,nc2
       do b = nd1,nd2
          sa = Fa(a,b) *Fa(a,b) + sa
          sb = Fb(a,b) *Fb(a,b) + sb
       enddo
       enddo
       Write(6,"(a,F15.8)") "Checksum of FA =",sa
       Write(6,"(a,F15.8)") "Checksum of FB =",sb
#endif

      return
      end
c
c ------------------------------------------------------------------- 
c  Used if 
c ------------------------------------------------------------------- 
c 
      subroutine uscf_tp7a(nalpha_occupied, nbeta_occupied,
     &                jx,nc1,nc2,nd1,nd2,ca,cb,va1,va2,vb1,vb2,
     &                vc1,vc2,
     &                vd1,vd2,integrals,intblk, 
     &                a1, a2, b1, b2, c1, c2, d1, d2,
     &                Fa,Fb)
      use, intrinsic :: ISO_C_BINDING
      implicit none
      integer va1, va2, vb1,vb2, vc1, vc2, vd1, vd2
      integer a1, a2, b1, b2, c1, c2, d1, d2
      integer a,b,c,d,sym 
      integer nc1,nc2,nd1,nd2,p  
      integer drange1, drange2
      integer crange1, crange2
      integer brange1, brange2
      integer arange1, arange2
      integer dstart, dend  
      integer delta_a, ainc, amax, jx,itype  
      integer nalpha_occupied, nbeta_occupied

      double precision ca(nc1:nc2,nd1:nd2)
      double precision cb(nc1:nc2,nd1:nd2)

      double precision Xa(nc1:nc2,nd1:nd2)
      double precision Xb(nc1:nc2,nd1:nd2)

      double precision cab(nc1:nc2,nd1:nd2)
      double precision Fa(nc1:nc2,nc1:nc2)
      double precision Fb(nc1:nc2,nc1:nc2)
      double precision intblk(a1:a2,b1:b2,c1:c2,d1:d2)
      double precision integrals(va1:va2,vb1:vb2,vc1:vc2,vd1:vd2)
      double precision dtemp, etemp, itemp(5), dcrit, factor  
      double precision dtemp1, etemp1 
      double precision dtemp2, etemp2 
      double precision dtemp3, etemp3 
      double precision etemp_cd, etempa_ac, etempb_ac  
      double precision ca_bd, cb_bd 

      dcrit = 0.0 
      drange1 = max(vd1, d1)
      drange2 = min(vd2, d2)
      crange1 = max(vc1, c1)
      crange2 = min(vc2, c2)
      brange1 = max(vb1, b1)
      brange2 = min(vb2, b2)
      arange1 = max(va1, a1)
      arange2 = min(va2, a2)

c ----------------------------------------------------------------- 


      dstart = min(arange1,brange1,crange1,drange1)
      dend = max(arange2,brange2,crange2,drange2)

      do d = dstart, dend ! nd1, nd2 
      do c = dstart, dend ! nc1, nc2 

      jx = 2 
      if (jx .eq. 1) then

             dtemp = 0.0
             do p = 1, nalpha_occupied ! nd1,nd2
                dtemp = dtemp + ca(c,p)*ca(d,p)
             enddo
             xa(c,d) = dtemp

             dtemp = 0.0
             do p = 1, nbeta_occupied ! nd1,nd2
                dtemp = dtemp + cb(c,p)*cb(d,p)
             enddo
             xb(c,d) = dtemp

      else

             xa(c,d) = ca(c,d)
             xb(c,d) = cb(c,d)

      endif

      enddo
      enddo

      do b = brange1, brange2
      do a = arange1, arange2
         cab(a,b) = xa(a,b) + xb(a,b)  
      enddo 
      enddo 

      do d = drange1, drange2
      do c = crange1, crange2

         dtemp = xa(c,d) + xb(c,d)  
         etemp_cd = 0.0 

            do b = brange1, brange2

               ca_bd = xa(b,d) 
               cb_bd = xb(b,d) 

            do a = arange1, arange2, 5 
                  amax = min(arange2-a,4)
                  ainc = 0

                                   itemp(1)  = intblk(a  ,b,c,d)
                  if (amax .ge. 1) itemp(2)  = intblk(a+1,b,c,d)
                  if (amax .ge. 2) itemp(3)  = intblk(a+2,b,c,d)
                  if (amax .ge. 3) itemp(4)  = intblk(a+3,b,c,d)
                  if (amax .ge. 4) itemp(5)  = intblk(a+4,b,c,d) 
33       continue 

c              itemp  = intblk(a+ainc,b,c,d) 

               etemp  = dtemp *itemp(ainc+1)  

               Fa(a+ainc,b) = Fa(a+ainc,b) + etemp 
               Fb(a+ainc,b) = Fb(a+ainc,b) + etemp 

               etempa_ac = ca_bd*itemp(ainc+1)  
               etempb_ac = cb_bd*itemp(ainc+1)  

               etemp_cd = etemp_cd + cab(a+ainc,b)*itemp(ainc+1)  

               Fa(a+ainc,c) = Fa(a+ainc,c) - etempa_ac  
               Fa(c,a+ainc) = Fa(c,a+ainc) - etempa_ac  

               Fb(a+ainc,c) = Fb(a+ainc,c) - etempb_ac  
               Fb(c,a+ainc) = Fb(c,a+ainc) - etempb_ac 

                  ainc = ainc + 1
                  if (ainc .le. amax) go to 33

            enddo

            enddo

            Fa(c,d) = Fa(c,d) + etemp_cd 
            Fb(c,d) = Fb(c,d) + etemp_cd  
      enddo
      enddo

      return
      end
c
c ------------------------------------------------------------------- 
c  Used if 
c ------------------------------------------------------------------- 
c 
      subroutine uscf_tp8a(nalpha_occupied, nbeta_occupied,
     &                jx,nc1,nc2,nd1,nd2,ca,cb,va1,va2,vb1,vb2,
     &                vc1,vc2,
     &                vd1,vd2,integrals,intblk, 
     &                a1, a2, b1, b2, c1, c2, d1, d2,
     &                Fa,Fb)
      use, intrinsic :: ISO_C_BINDING
      implicit none
      integer va1, va2, vb1,vb2, vc1, vc2, vd1, vd2
      integer a1, a2, b1, b2, c1, c2, d1, d2
      integer a,b,c,d,sym 
      integer nc1,nc2,nd1,nd2,p  
      integer drange1, drange2
      integer crange1, crange2
      integer brange1, brange2
      integer arange1, arange2
      integer dstart, dend  
      integer delta_a, ainc, amax, jx, itype  
      integer nalpha_occupied, nbeta_occupied

      double precision ca(nc1:nc2,nd1:nd2)
      double precision cb(nc1:nc2,nd1:nd2)

      double precision Xa(nc1:nc2,nd1:nd2)
      double precision Xb(nc1:nc2,nd1:nd2)

      double precision Fa(nc1:nc2,nc1:nc2)
      double precision Fb(nc1:nc2,nc1:nc2)
      double precision ca_d(a1:a2)
      double precision cb_d(a1:a2)
      double precision ca_c(a1:a2)
      double precision cb_c(a1:a2) 
      double precision intblk(a1:a2,b1:b2,c1:c2,d1:d2)
      double precision integrals(va1:va2,vb1:vb2,vc1:vc2,vd1:vd2)
      double precision dtemp, etemp, itemp(5), dcrit, factor  
      double precision dtemp1, etemp1 
      double precision dtemp2, etemp2 
      double precision dtemp3, etemp3 
      double precision dtemp4, etemp4 
      double precision dtemp5, etemp5 
      double precision dtemp6, etemp6 
      double precision dtemp7, etemp7 
      double precision dtemp8, etemp8 
      double precision etempa_bd, etempa_bc, etempb_bd, etempb_bc 

      dcrit = 0.0 
      drange1 = max(vd1, d1)
      drange2 = min(vd2, d2)
      crange1 = max(vc1, c1)
      crange2 = min(vc2, c2)
      brange1 = max(vb1, b1)
      brange2 = min(vb2, b2)
      arange1 = max(va1, a1)
      arange2 = min(va2, a2)

c ----------------------------------------------------------------- 


      dstart = min(arange1,brange1,crange1,drange1)
      dend = max(arange2,brange2,crange2,drange2)

      do d = dstart, dend ! nd1, nd2 
      do c = dstart, dend ! nc1, nc2 

      jx = 2 
      if (jx .eq. 1) then

             dtemp = 0.0
             do p = 1, nalpha_occupied ! nd1,nd2
                dtemp = dtemp + ca(c,p)*ca(d,p)
             enddo
             xa(c,d) = dtemp

             dtemp = 0.0
             do p = 1, nbeta_occupied ! nd1,nd2
                dtemp = dtemp + cb(c,p)*cb(d,p)
             enddo
             xb(c,d) = dtemp

      else

             xa(c,d) = ca(c,d)
             xb(c,d) = cb(c,d)

      endif

      enddo
      enddo

      do d = drange1, drange2
         do a = arange1, arange2
            ca_d(a) = xa(a,d)
            cb_d(a) = xb(a,d)
         enddo 
      do c = crange1, crange2
         dtemp = 2.0*(xa(c,d) + xb(c,d))  
         do a = arange1, arange2
            ca_c(a) = xa(a,c)
            cb_c(a) = xb(a,c)
         enddo
            do b = brange1, brange2
               dtemp1 = -1.0*xa(b,d)   
               dtemp2 = -1.0*xa(b,c)   
               dtemp5 = -1.0*xb(b,d)   
               dtemp6 = -1.0*xb(b,c)   
               etempa_bd = 0.0 
               etempa_bc = 0.0 
               etempb_bd = 0.0 
               etempb_bc = 0.0 
            do a = arange1, arange2, 5 
                  amax = min(arange2-a,4)
                  ainc = 0

                                   itemp(1)  = intblk(a  ,b,c,d)
                  if (amax .ge. 1) itemp(2)  = intblk(a+1,b,c,d)
                  if (amax .ge. 2) itemp(3)  = intblk(a+2,b,c,d)
                  if (amax .ge. 3) itemp(4)  = intblk(a+3,b,c,d)
                  if (amax .ge. 4) itemp(5)  = intblk(a+4,b,c,d) 
33       continue

c              itemp = intblk(a+ainc,b,c,d) 

               etemp  = dtemp*itemp(ainc+1)  
               etemp1 = dtemp1*itemp(ainc+1) 
               etemp2 = dtemp2*itemp(ainc+1) 
               etemp5 = dtemp5*itemp(ainc+1) 
               etemp6 = dtemp6*itemp(ainc+1) 

               Fa(a+ainc,b) = Fa(a+ainc,b) + etemp 
               Fa(b,a+ainc) = Fa(b,a+ainc) + etemp 

               Fb(a+ainc,b) = Fb(a+ainc,b) + etemp 
               Fb(b,a+ainc) = Fb(b,a+ainc) + etemp 

               Fa(a+ainc,c) = Fa(a+ainc,c) + etemp1 

               Fa(a+ainc,d) = Fa(a+ainc,d) + etemp2 

               Fb(a+ainc,c) = Fb(a+ainc,c) + etemp5 

               Fb(a+ainc,d) = Fb(a+ainc,d) + etemp6 

               etempa_bc = etempa_bc + ca_d(a+ainc)*itemp(ainc+1)  
               etempb_bc = etempb_bc + cb_d(a+ainc)*itemp(ainc+1)  

               etempa_bd = etempa_bd + ca_c(a+ainc)*itemp(ainc+1)  
               etempb_bd = etempb_bd + cb_c(a+ainc)*itemp(ainc+1)  

                  ainc = ainc + 1
                  if (ainc .le. amax) go to 33

            enddo

               Fa(b,c) = Fa(b,c) - etempa_bc  
               Fb(b,c) = Fb(b,c) - etempb_bc 

               Fa(b,d) = Fa(b,d) - etempa_bd 
               Fb(b,d) = Fb(b,d) - etempb_bd 

            enddo

      enddo    
      enddo    


      return
      end
 
c ------------------------------------------------------------------- 
c  Used if 
c ------------------------------------------------------------------- 
c 
      subroutine uscf_tp2a(nalpha_occupied, nbeta_occupied, 
     &                nc1,nc2,nd1,nd2,xa,xb,va1,va2,vb1,vb2,
     &                vc1,vc2,
     &                vd1,vd2,integrals,intblk, 
     &                a1, a2, b1, b2, c1, c2, d1, d2, 
     &                Fa,Fb)
      use, intrinsic :: ISO_C_BINDING
      implicit none
      integer va1, va2, vb1,vb2, vc1, vc2, vd1, vd2 
      integer a1, a2, b1, b2, c1, c2, d1, d2
      integer a,b,c,d,sym 
      integer nc1,nc2,nd1,nd2,p  
      integer drange1, drange2
      integer crange1, crange2
      integer brange1, brange2
      integer arange1, arange2
      integer dstart, dend  
      integer delta_a, ainc, amax, jx,itype  
      integer nalpha_occupied, nbeta_occupied 

      double precision Xa(nc1:nc2,nd1:nd2)
      double precision Xb(nc1:nc2,nd1:nd2)

      double precision Fa(nc1:nc2,nc1:nc2)
      double precision Fb(nc1:nc2,nc1:nc2)
      double precision cab(nc1:nc2,nd1:nd2)
      double precision intblk(a1:a2,b1:b2,c1:c2,d1:d2)
      double precision integrals(va1:va2,vb1:vb2,vc1:vc2,vd1:vd2)
      double precision dtemp, etemp, itemp(5), dcrit, factor  
      double precision dtemp1, etemp1   
      double precision dtemp2, etemp2   
      double precision dtemp3, etemp3   
      double precision dtemp4, etemp4   
      double precision dtemp5, etemp5   
      double precision ftemp_cd,ftempa_ad,ftempb_ad,ftempa_ac,ftempb_ac 
      double precision ca_bc, ca_bd, cb_bc, cb_bd 


      dcrit = 0.0 
      drange1 = max(vd1, d1) 
      drange2 = min(vd2, d2) 
      crange1 = max(vc1, c1) 
      crange2 = min(vc2, c2) 
      brange1 = max(vb1, b1) 
      brange2 = min(vb2, b2) 
      arange1 = max(va1, a1) 
      arange2 = min(va2, a2) 

c ----------------------------------------------------------------- 

      do a = arange1, arange2
      do b = brange1, brange2
           cab(a,b) = xa(a,b) + xb(a,b)
      enddo
      enddo


      do d = drange1, drange2
      do c = crange1, crange2

         dtemp = xa(c,d) + xb(c,d)
         dtemp = 2.0d0*dtemp
         ftemp_cd = 0.0

            do b = brange1, brange2
               ca_bc = xa(b,c)
               ca_bd = xa(b,d)
               cb_bc = xb(b,c)
               cb_bd = xb(b,d)
            do a = arange1, arange2, 5
                  amax = min(arange2-a,4)
                  ainc = 0

                                   itemp(1)  = intblk(a  ,b,c,d)
                  if (amax .ge. 1) itemp(2)  = intblk(a+1,b,c,d)
                  if (amax .ge. 2) itemp(3)  = intblk(a+2,b,c,d)
                  if (amax .ge. 3) itemp(4)  = intblk(a+3,b,c,d)
                  if (amax .ge. 4) itemp(5)  = intblk(a+4,b,c,d)
33       continue

c              itemp = intblk(a+ainc,b,c,d) 

               etemp  = dtemp *itemp(ainc+1)

               ftemp_cd  = ftemp_cd + cab(a+ainc,b)*itemp(ainc+1) ! etemp1 
               ftempa_ad = ca_bc*itemp(ainc+1) ! etemp3 
               ftempb_ad = cb_bc*itemp(ainc+1) ! etemp5 
               ftempa_ac = ca_bd*itemp(ainc+1) ! etemp2 
               ftempb_ac = cb_bd*itemp(ainc+1) ! etemp4 

               Fa(a+ainc,b) = Fa(a+ainc,b) + etemp
               Fb(a+ainc,b) = Fb(a+ainc,b) + etemp

               Fa(a+ainc,c) = Fa(a+ainc,c) - ftempa_ac
               Fa(c,a+ainc) = Fa(c,a+ainc) - ftempa_ac

               Fa(a+ainc,d) = Fa(a+ainc,d) - ftempa_ad
               Fa(d,a+ainc) = Fa(d,a+ainc) - ftempa_ad

               Fb(a+ainc,c) = Fb(a+ainc,c) - ftempb_ac
               Fb(c,a+ainc) = Fb(c,a+ainc) - ftempb_ac

               Fb(a+ainc,d) = Fb(a+ainc,d) - ftempb_ad
               Fb(d,a+ainc) = Fb(d,a+ainc) - ftempb_ad

                  ainc = ainc + 1
                  if (ainc .le. amax) go to 33

            enddo

            enddo

            Fa(c,d) = Fa(c,d) + ftemp_cd
            Fa(d,c) = Fa(d,c) + ftemp_cd
            Fb(c,d) = Fb(c,d) + ftemp_cd
            Fb(d,c) = Fb(d,c) + ftemp_cd
      enddo
      enddo

      return
      end
c
c 
      subroutine return_int(nalpha_occupied, nbeta_occupied,
     &                jx,va1,va2,vb1,vb2,
     &                vc1,vc2, vd1,vd2, 
     &                integrals, intblk, a1, a2, b1, b2, 
     &                c1, c2, d1, d2)
      use, intrinsic :: ISO_C_BINDING
      implicit none
      integer va1, va2, vb1,vb2, vc1, vc2, vd1, vd2
      integer a1, a2, b1, b2, c1, c2, d1, d2
      integer a,b,c,d,sym 
      integer nc1,nc2,nd1,nd2,p  
      integer drange1, drange2
      integer crange1, crange2
      integer brange1, brange2
      integer arange1, arange2
      integer dstart, dend  
      integer delta_a, amax, ainc, jx, itype  
      integer nalpha_occupied, nbeta_occupied

      double precision intblk(a1:a2,b1:b2,c1:c2,d1:d2)
      double precision integrals(va1:va2,vb1:vb2,vc1:vc2,vd1:vd2)
      double precision dtemp, etemp, itemp(5), dcrit, factor  
      double precision dtemp1, etemp1 
      double precision dtemp2, etemp2 

      dcrit = 0.0 
      drange1 = max(vd1, d1)
      drange2 = min(vd2, d2)
      crange1 = max(vc1, c1)
      crange2 = min(vc2, c2)
      brange1 = max(vb1, b1)
      brange2 = min(vb2, b2)
      arange1 = max(va1, a1)
      arange2 = min(va2, a2)

c Form the output  integral array 

      do d = d1, d2  
      do c = c1, c2  
            do b = b1, b2  
            do a = a1, a2  
                integrals(a,b,c,d) = intblk(a,b,c,d) 
c               write(6,*) ' INT6:',a,b,c,d,intblk(a,b,c,d)
c    * , integrals(a,b,c,d) 
            enddo 
            enddo 
       enddo 
       enddo 

      return
      end
c
