      Subroutine Ecp_grdint(Ecpgrdint, Ecpgrd_X, Ecpgrd_Y, Ecpgrd_Z, 
     &                      La, Lb, Iloc, Jloc, Isoc, Jsoc, Numcoi,  
     &                      Numcoj, Iatom, Jatom, Natoms, Spherical,
     &
     &                      Ecpgrd_SX,Ecpgrd_SY,Ecpgrd_SZ,
     &                      tmp1,tmp2,
     &
     &                      max_cofns_shell,nval_bfns)
    
      
      Implicit Double Precision(A-H, O-Z)
      Logical Spherical 
      Integer Zused, Zbegin, Z00a, Z00b
    
#include "ecp.par"
#include "ecp.com"

      Dimension Ecpgrdint(Natoms,3,nval_bfns * (nval_bfns+1)/2)
      Dimension Ecpgrd_X(max_cofns_shell * max_cofns_shell)
      Dimension Ecpgrd_Y(max_cofns_shell * max_cofns_shell)
      Dimension Ecpgrd_Z(max_cofns_shell * max_cofns_shell)

      Dimension Cart2sph(Dmap_L), Ic2smap(Imap_L)

      Dimension Ecpgrd_SX(max_cofns_shell * max_cofns_shell)
      Dimension Ecpgrd_SY(max_cofns_shell * max_cofns_shell) 
      Dimension Ecpgrd_SZ(max_cofns_shell * max_cofns_shell) 

      Dimension Tmp1(max_cofns_shell * max_cofns_shell)
      Dimension Tmp2(max_cofns_shell * max_cofns_shell)
     
      If (.Not. Spherical) Then
      Ind = 0
      Do Iang = 1, (La+1)*(La+2)/2
         Do Jang = 1, (Lb+1)*(Lb+2)/2
C
             Do Icon = 1, Numcoi
                Do Jcon = 1, Numcoj

                   Ind  = Ind  + 1
                   Indx = Iloc + (Icon-1)*(La+1)*(La+2)/2 + Iang
                   Jndx = Jloc + (Jcon-1)*(Lb+1)*(Lb+2)/2 + Jang

                   If (Indx .GE. Jndx) Indecp = Indx*(Indx-1)/2 + Jndx
                   If (Jndx .GT. Indx) Indecp = Jndx*(Jndx-1)/2 + Indx
#ifdef _DEBUGLVL0
      Write(6,"(2(1x,I3),3(1x,F10.5))") indecp, ind,
     &                               ecpgrd_x(ind), ecpgrd_y(ind),
     &                               ecpgrd_z(ind)
#endif

                   Ecpgrdint(Iatom,1,Indecp) = Ecpgrd_X(ind) +
     &                                         Ecpgrdint(Iatom,1,Indecp)
                   Ecpgrdint(Iatom,2,Indecp) = Ecpgrd_Y(ind) +
     &                                         Ecpgrdint(Iatom,2,Indecp)
                   Ecpgrdint(Iatom,3,Indecp) = Ecpgrd_Z(ind) +
     &                                         Ecpgrdint(Iatom,3,Indecp)

                   Ecpgrdint(Jatom,1,Indecp) = -Ecpgrd_X(ind) +
     &                                         Ecpgrdint(Jatom,1,Indecp)
                   Ecpgrdint(Jatom,2,Indecp) = -Ecpgrd_Y(ind) + 
     &                                         Ecpgrdint(Jatom,2,Indecp)
                   Ecpgrdint(Jatom,3,Indecp) = -Ecpgrd_Z(ind) +
     &                                         Ecpgrdint(Jatom,3,Indecp)

                Enddo
             Enddo
C
         Enddo
      Enddo
C
      Else     
         Nxyza  = (La+1)*(La+2)/2
         Nxyzb  = (Lb+1)*(Lb+2)/2
         Nrya   = (2*La+1)
         Nryb   = (2*Lb+1)
         Ibegin = 1
         Zbegin = 1
         Nctr   = Numcoi*Numcoj

         Call Dcopy(Nctr*Nxyza*Nxyzb, Ecpgrd_X, 1, Ecpgrd_SX, 1)
         Call Dcopy(Nctr*Nxyza*Nxyzb, Ecpgrd_Y, 1, Ecpgrd_SY, 1)
         Call Dcopy(Nctr*Nxyza*Nxyzb, Ecpgrd_Z, 1, Ecpgrd_SZ, 1)


         If (La .Gt. 1 .OR. Lb .Gt. 1) then
           Call oedecp__xyz_to_ry_ab(Nxyza, Nxyzb, Nrya, Nryb,
     &                              La, Lb, Ibegin, Zbegin, Nrowa,
     &                              Nrowb, Nrota, Nrotb, Z00a, Z00b,
     &                              I0a1, I0b1, I0a2, I0b2, Iused,
     &                              Zused, Ic2smap, Cart2Sph)
         endif
C
#ifdef _DEBUG_LVL0
      If (Lb .gt. 1 .or. La .Gt. 1) then
      Write(6,"(a,6(1x,i2))") "La,Lb,nrowa,nrowb,numcoi and numcoj",
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
C Transform X derivative

       If (La .GT. 1) Then
         Call oed__spherical_transform(Nctr*Nxyzb, Nrowa, Nxyza,
     &                                 Nrya,
     &                                 Ic2smap(I0a1), Ic2smap(I0a2),
     &                                 Cart2Sph(Z00a),
     &                                 Ecpgrd_X, Tmp2)
C
#ifdef _DEBUG_LVL0
      if (la .gt. 1) then
      Write(6,*) "The A CtoS transformed ints A|B"
      Write(6, "(6(1x,F10.5))") (Tmp2(i), i=1, Nctr*Nrya*Nxyzb)
      endif
#endif
      If (Lb .Le. 1) Call Dcopy(Nctr*Nrya*Nxyzb, Tmp2, 1, Ecpgrd_SX, 1)

      Endif

      If (Lb .GT. 1) Then

         If (La .LE. 1) Call Dcopy(Nctr*Nrya*Nxyzb, Ecpgrd_X,
     &                                                 1, Tmp2, 1)
            Call transp(Tmp2, Tmp1, Nrya, Nctr*Nxyzb)

#ifdef _DEBUG_LVL0
      if (la .gt. 1 .or. Lb .gt. 1) then
      Write(6,*) "The trnasposed ints A|B"
      Write(6, "(6(1x,F10.5))") (Tmp1(i), i=1, Nctr*Nrya*Nxyzb)
      endif
#endif
            Call oed__spherical_transform(Nctr*Nrya, Nrowb, Nxyzb,
     &                                    Nryb,
     &                                    Ic2smap(I0b1), Ic2smap(I0b2),
     &                                    Cart2Sph(Z00b),
     &                                    Tmp1, Ecpgrd_SX)

#ifdef _DEBUG_LVL0
      Write(6,*) "The CtoS transformed ints A|B"
      Write(6, "(6(1x,F10.5))") (Ecpgrd_SX(i), i=1, Nctr*Nrya*Nryb)
#endif
            Call transp(Ecpgrd_SX, Tmp1, Nctr*Nryb, Nrya)
            Call Dcopy(Nctr*Nryb*Nrya, Tmp1, 1, Ecpgrd_SX, 1)
         Endif

C Transform Y deriavtive

      If (La .GT. 1) Then
         Call oed__spherical_transform(Nctr*Nxyzb, Nrowa, Nxyza,
     &                                 Nrya,
     &                                 Ic2smap(I0a1), Ic2smap(I0a2),
     &                                 Cart2Sph(Z00a),
     &                                 Ecpgrd_Y, Tmp2)
C
#ifdef _DEBUG_LVL0
      if (la .gt. 1) then
      Write(6,*) "The A CtoS transformed ints A|B"
      Write(6, "(6(1x,F10.5))") (Tmp2(i), i=1, Nctr*Nrya*Nxyzb)
      endif
#endif
      If (Lb .Le. 1) Call Dcopy(Nctr*Nrya*Nxyzb, Tmp2, 1, Ecpgrd_SY, 1)

      Endif

      If (Lb .GT. 1) Then
         If (La .LE. 1) Call Dcopy(Nctr*Nrya*Nxyzb, Ecpgrd_Y,
     &                                                 1, Tmp2, 1)
         Call transp(Tmp2, Tmp1, Nrya, Nctr*Nxyzb)

#ifdef _DEBUG_LVL0
      if (la .gt. 1 .or. Lb .gt. 1) then
      Write(6,*) "The trnasposed ints A|B"
      Write(6, "(6(1x,F10.5))") (Tmp1(i), i=1, Nctr*Nrya*Nxyzb)
      endif
#endif
         Call oed__spherical_transform(Nctr*Nrya, Nrowb, Nxyzb,
     &                                 Nryb,
     &                                 Ic2smap(I0b1), Ic2smap(I0b2),
     &                                 Cart2Sph(Z00b),
     &                                 Tmp1, Ecpgrd_SY)

#ifdef _DEBUG_LVL0
      Write(6,*) "The CtoS transformed ints A|B"
      Write(6, "(6(1x,F10.5))") (Ecpgrd_SX(i), i=1, Nctr*Nrya*Nryb)
#endif
            Call transp(Ecpgrd_SY, Tmp1, Nctr*Nryb, Nrya)
            Call Dcopy(Nctr*Nryb*Nrya, Tmp1, 1, Ecpgrd_SY, 1)
         Endif
C
C Transform the Z derivative
C
      If (La .GT. 1) Then
         Call oed__spherical_transform(Nctr*Nxyzb, Nrowa, Nxyza,
     &                                 Nrya,
     &                                 Ic2smap(I0a1), Ic2smap(I0a2),
     &                                 Cart2Sph(Z00a),
     &                                 Ecpgrd_Z, Tmp2)
C
#ifdef _DEBUG_LVL0
      if (la .gt. 1) then
      Write(6,*) "The A CtoS transformed ints A|B"
      Write(6, "(6(1x,F10.5))") (Tmp2(i), i=1, Nctr*Nrya*Nxyzb)
      endif
#endif
      If (Lb .Le. 1) Call Dcopy(Nctr*Nrya*Nxyzb, Tmp2, 1, Ecpgrd_SZ, 1)

      Endif

      If (Lb .GT. 1) Then

         If (La .LE. 1) Call Dcopy(Nctr*Nrya*Nxyzb, Ecpgrd_Z,
     &                                                 1, Tmp2, 1)
         Call transp(Tmp2, Tmp1, Nrya, Nctr*Nxyzb)

#ifdef _DEBUG_LVL0
      if (la .gt. 1 .or. Lb .gt. 1) then
      Write(6,*) "The trnasposed ints A|B"
      Write(6, "(6(1x,F10.5))") (Tmp1(i), i=1, Nctr*Nrya*Nxyzb)
      endif
#endif
            Call oed__spherical_transform(Nctr*Nrya, Nrowb, Nxyzb,
     &                                    Nryb,
     &                                    Ic2smap(I0b1), Ic2smap(I0b2),
     &                                    Cart2Sph(Z00b),
     &                                    Tmp1, Ecpgrd_SZ)

#ifdef _DEBUG_LVL0
      Write(6,*) "The CtoS transformed ints A|B"
      Write(6, "(6(1x,F10.5))") (Ecpgrd_SZ(i), i=1, Nctr*Nrya*Nryb)
#endif
            Call transp(Ecpgrd_SZ, Tmp1, Nctr*Nryb, Nrya)
            Call Dcopy(Nctr*Nryb*Nrya, Tmp1, 1, Ecpgrd_SZ, 1)
         Endif

#ifdef _DEBUG_LVL0
      if (la .gt. 1 .or. Lb .gt. 1) then
      Write(6,*) "The CtoS transformed ints A|B afer transpose"
      Write(6,*) "X-Derivative"
      Write(6, "(6(1x,F10.5))") (Ecpgrd_SX(i), i=1, Nctr*Nrya*Nryb)
      Write(6,*) "Y-Derivative"
      Write(6, "(6(1x,F10.5))") (Ecpgrd_SY(i), i=1, Nctr*Nrya*Nryb)
      Write(6,*) "Z-Derivative"
      Write(6, "(6(1x,F10.5))") (Ecpgrd_SZ(i), i=1, Nctr*Nrya*Nryb)
      endif
#endif
      Ind = 0
      Do Iang = 1, (2*La+1)
         Do Jang = 1, (2*Lb+1)
C
             Do Icon = 1, Numcoi
                Do Jcon = 1, Numcoj

                   Ind  = Ind  + 1
                   Indx = ISoc + (Icon-1)*(2*La+1) + Iang
                   Jndx = JSoc + (Jcon-1)*(2*Lb+1) + Jang

                   If (Indx .GE. Jndx) Indecp = Indx*(Indx-1)/2 + Jndx
                   If (Jndx .GT. Indx) Indecp = Jndx*(Jndx-1)/2 + Indx
#ifdef _DEBUGLVL0
      Write(6,"(2(1x,I3),3(1x,F10.5))") indecp, ind,
     &                               ecpgrd_Sx(ind), ecpgrd_Sy(ind),
     &                               ecpgrd_Sz(ind)
#endif

                   Ecpgrdint(Iatom,1,Indecp) = Ecpgrd_SX(ind) +
     &                                         Ecpgrdint(Iatom,1,Indecp)
                   Ecpgrdint(Iatom,2,Indecp) = Ecpgrd_SY(ind) +
     &                                         Ecpgrdint(Iatom,2,Indecp)
                   Ecpgrdint(Iatom,3,Indecp) = Ecpgrd_SZ(ind) +
     &                                         Ecpgrdint(Iatom,3,Indecp)

                   Ecpgrdint(Jatom,1,Indecp) = -Ecpgrd_SX(ind) +
     &                                         Ecpgrdint(Jatom,1,Indecp)
                   Ecpgrdint(Jatom,2,Indecp) = -Ecpgrd_SY(ind) +
     &                                         Ecpgrdint(Jatom,2,Indecp)
                   Ecpgrdint(Jatom,3,Indecp) = -Ecpgrd_SZ(ind) +
     &                                         Ecpgrdint(Jatom,3,Indecp)

                Enddo
             Enddo
C
         Enddo
      Enddo

      Endif
C
      Return
      End
