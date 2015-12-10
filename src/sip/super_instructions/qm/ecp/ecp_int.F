      Subroutine Ecp_int(Ecpint, Ecpint_4shell, La, Lb, 
     &                   Iloc, Jloc, Isoc, Jsoc, Numcoi, Numcoj, 
     &                   Ishell, Jshell, Spherical,
     & 
     &                   Ecpint_s,tmp1,tmp2,
     &       
     &                   max_cofns_shell,nval_bfns)
      
      Implicit Double Precision(A-H, O-Z)

      Logical Spherical
      Integer Zused, Zbegin, Z00a, Z00b

#include "ecp.par"
#include "ecp.com"
     
      Dimension Ecpint(nval_bfns * (nval_bfns+1)/2 )
      Dimension Ecpint_4shell(max_cofns_shell * max_cofns_shell) 
      Dimension Ecpint_s(max_cofns_shell * max_cofns_shell) 
      Dimension Tmp1(max_cofns_shell * max_cofns_shell) 
      Dimension Tmp2(max_cofns_shell * max_cofns_shell) 
      Dimension Ic2smap(Imap_L),Cart2sph(Dmap_L)

      If (.Not. Spherical) Then

#ifdef _DEBUG_LVL0
      Nxyza  = (La+1)*(La+2)/2
      Nxyzb  = (Lb+1)*(Lb+2)/2
      Nctr   = Numcoi*Numcoj
C
      Write(6,*) "The A|B integrls befor C to S"
      Write(6, "(6(1x,F10.5))") (Ecpint_4shell(i), i=1, 
     &                           Nctr*Nxyza*Nxyzb)
#endif
         Ind = 0
         Do Iang = 1, (La+1)*(La+2)/2
            Do Jang = 1, (Lb+1)*(Lb+2)/2
C
               Do Icon = 1, Numcoi
                  Do Jcon = 1, Numcoj

                     Ind  = Ind  + 1
                     Indx = Iloc + (Icon-1)*(La+1)*(La+2)/2 + Iang
                     Jndx = Jloc + (Jcon-1)*(Lb+1)*(Lb+2)/2 + Jang

                     If (Indx .GE. Jndx) Then
                        Indecp = Indx*(Indx-1)/2 + Jndx
                        Ecpint(indecp) = Ecpint_4shell(ind)
                     Endif

                  Enddo
               Enddo
C
            Enddo
         Enddo
      
      Else
         Nxyza  = (La+1)*(La+2)/2
         Nxyzb  = (Lb+1)*(Lb+2)/2
         Nrya   = (2*La+1)
         Nryb   = (2*Lb+1)
         Ibegin = 1
         Zbegin = 1
         Nctr   = Numcoi*Numcoj 

C   
#ifdef _DEBUG_LVL0
      Write(6,*) "The A|B integrls before C to S"
      Write(6, "(6(1x,F10.5))") (Ecpint_4shell(i), i=1,
     &                           Nctr*Nxyza*Nxyzb)
#endif
         Call Dcopy(Nctr*Nxyza*Nxyzb, Ecpint_4shell, 1, Ecpint_S, 1)
         
         If (La .Gt. 1 .OR. Lb .Gt. 1) then
           Call oedecp__xyz_to_ry_ab(Nxyza, Nxyzb, Nrya, Nryb,
     &                              La, Lb, Ibegin, Zbegin, Nrowa,
     &                              Nrowb, Nrota, Nrotb, Z00a, Z00b,
     &                              I0a1, I0b1, I0a2, I0b2, Iused, 
     &                              Zused, Ic2smap, Cart2Sph)
         endif

#ifdef _DEBUG_LVL0
      If (Lb .gt. 1 .or. La .Gt. 1) then
      Write(6,"(a,6(1x,i4))") "La,Lb,nrowa,nrowb,numcoi and numcoj",
     &                         La, Lb,nrowa,nrowb,numcoi,numcoj
      if (la .gt. 1) then
      Write(6, "(a)") "Mapping for A of A|B"
      Write(6,"(12(1x,I3))") (Ic2smap(I0a1-1+i), i=1, Nrya)
      Write(6,"(12(1x,I3))") (Ic2smap(I0a2-1+i), i=1, Nrota)
      Write(6, "(a)") "The transformation matrix for A of A|B"
      Write(6,"(6(1x,F10.5))") (Cart2Sph(z00a-1+i), i=1, Nrota)
      endif
      if (lb .gt. 1) then
      Write(6, "(a)") "Mapping for b of A|B"
      Write(6,"(12(1x,I3))") (Ic2smap(I0b1-1+i), i=1, Nryb)
      Write(6,"(12(1x,I3))") (Ic2smap(I0b2-1+i), i=1, Nrotb)
      Write(6, "(a)") "The transformation matrix for B of A|B"
      Write(6,"(6(1x,F10.5))") (Cart2Sph(z00b-1+i), i=1, Nrotb)
      endif
      Endif
#endif

      If (La .GT. 1) Then
         Call oed__spherical_transform(Nctr*Nxyzb, Nrowa, Nxyza, 
     &                                 Nrya,
     &                                 Ic2smap(I0a1), Ic2smap(I0a2), 
     &                                 Cart2Sph(Z00a),
     &                                 Ecpint_4Shell, Tmp1)
C
#ifdef _DEBUG_LVL0
      if (la .gt. 1) then
      Write(6,*) "The A CtoS transformed ints A|B"
      Write(6, "(6(1x,F10.5))") (Tmp1(i), i=1, Nctr*Nrya*Nxyzb)
      endif
#endif
      If (Lb .Le. 1) Call Dcopy(Nctr*Nrya*Nxyzb, Tmp1, 1, Ecpint_S, 1)

      Endif

         If (Lb .GT. 1) Then
            If (La .LE. 1) Call Dcopy(Nctr*Nrya*Nxyzb, Ecpint_4Shell, 
     &                                                    1, Tmp1, 1)
            Call transp(Tmp1, Tmp2, Nrya, Nctr*Nxyzb)

#ifdef _DEBUG_LVL0
      if (la .gt. 1 .or. Lb .gt. 1) then
      Write(6,*) "The transposed ints A|B"
      Write(6, "(6(1x,F10.5))") (Tmp2(i), i=1, Nctr*Nrya*Nxyzb)
      endif
#endif
            Call oed__spherical_transform(Nctr*Nrya, Nrowb, Nxyzb, 
     &                                    Nryb, 
     &                                    Ic2smap(I0b1), Ic2smap(I0b2), 
     &                                    Cart2Sph(Z00b),
     &                                    Tmp2, Ecpint_S)
#ifdef _DEBUG_LVL0
      Write(6,*) "The CtoS transformed ints A|B"
      Write(6, "(6(1x,F10.5))") (Ecpint_S(i), i=1, Nctr*Nrya*Nryb)
#endif
            Call transp(Ecpint_s, Tmp2, Nctr*Nryb, Nrya)
            Call Dcopy(Nctr*Nryb*Nrya, Tmp2, 1, Ecpint_S, 1)
         Endif

#ifdef _DEBUG_LVL0
      if (la .gt. 1 .or. Lb .gt. 1) then
      Write(6,*) "The CtoS transformed ints A|B afer transpose"
      Write(6, "(6(1x,F10.5))") (Ecpint_S(i), i=1, Nctr*Nrya*Nryb)
      endif
#endif
          Ind = 0
          Do Iang = 1, (2*La+1)
            Do Jang = 1, (2*Lb+1)
C
               Do Icon = 1, Numcoi
                  Do Jcon = 1, Numcoj

                     Ind  = Ind  + 1
                     Indx = Isoc + (Icon-1)*(2*La+1) + Iang
                     Jndx = Jsoc + (Jcon-1)*(2*Lb+1) + Jang

                     If (Indx .GE. Jndx) Then
                        Indecp = Indx*(Indx-1)/2 + Jndx
#ifdef _DEBUG_LVL0
                     Write(6,"(a,1x,i3,1x,i3,1x,i3,1x,i3)")
     &              "Indx and Jndx",Indx, Jndx, Ind,Indecp
#endif
                     Ecpint(indecp) = Ecpint_S(ind)
                     Endif

                  Enddo
               Enddo
C
            Enddo
          Enddo
      Endif
  
#ifdef _DEBUG_LVL0
      Write(6,*)  " Returning from ecp_int.F"
#endif
      Return
      End
