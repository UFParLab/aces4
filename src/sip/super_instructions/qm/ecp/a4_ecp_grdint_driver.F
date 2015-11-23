      Subroutine A3_Ecp_grdint_driver(Ecpgrdint,Ecpint_4Shell,Cint,
     &                                Ecpgrd_x,Ecpgrd_y,Ecpgrd_z,
     &                                tmp1,tmp2,tmp3,tmp4,tmp5,
     &                    
     &                                nshells,max_ecp_prims,
     &                                necp_atoms,max_necp_functns,
     &                                nval_prims,nval_ccoeffs,
     &                                max_prims_shell,
     &                                max_conts_shell,
     &                                max_unfns_shell,
     &                                max_cofns_shell,
     &                                nval_bfns,max_nprims,
     &                                max_ncontfns,
     &                                ntotatoms,
     &
     &                                clp,zlp,nlp,kfirst,
     &                                klast,llmax,nelecp,
     &                                ipseux,ipseud,igenby,
     &
     &                                numcon,katom,ktype,
     &                                kprim,kmini,kmaxi,
     &                                nprims,ndegen,expnt,
     &                                expnt_norms,contr,
     &
     &                                coords, grads, spherical)

      implicit double precision (a-h,o-z)
C-----------------------------------------------------------------------
#include "ecp.par"
#include "ecp.com"
C
C Modification to compute ECP integrals in a suitable form
C for ACES III parallel implementation. Since ACES III disregard
C symmetry, all the logic relating to symmtery is removed (the
C reason that symmetry logic there in the first place is that
C this code is written to use both in ACES II and III. 08/2013,
C Ajith Perera.
C
      Logical Grads, Zero_int, Spherical

      integer ipseud,nelecp(ntotatoms),ipseux(ntotatoms)
      integer llmax(ntotatoms),nlp(max_ecp_prims)
      integer kfirst(necp_atoms,max_necp_functns)
      integer klast(necp_atoms,max_necp_functns)
      double precision clp(max_ecp_prims),zlp(max_ecp_prims)

      integer numcon(nshells),numpri(nshells)
      integer katom(nshells),ktype(nshells)
      integer kmini(nshells),kmaxi(nshells)
      integer ndegen(nshells)
      integer nprims(nshells),kprim(nshells)
      double precision expnt(max_nprims)
      double precision expnt_norms(max_nprims)
      double precision contr(max_nprims,max_conts_shell)

      dimension Cint(max_unfns_shell * max_unfns_shell)
      dimension Ecpint_4Shell(max_cofns_shell * max_cofns_shell)
      dimension Ecpgrdint(3 * ntotatoms * nval_bfns * (nval_bfns+1)/2)
      dimension Ecpgrd_x(max_cofns_shell * max_cofns_shell)
      dimension Ecpgrd_y(max_cofns_shell * max_cofns_shell)
      dimension Ecpgrd_z(max_cofns_shell * max_cofns_shell)
C
      dimension tmp1(max_cofns_shell * max_cofns_shell)
      dimension tmp2(max_cofns_shell * max_cofns_shell)
      dimension tmp3(max_cofns_shell * max_cofns_shell)
      dimension tmp4(max_cofns_shell * max_cofns_shell)
      dimension tmp5(max_cofns_shell * max_cofns_shell)
C
      dimension IGenby(Ntotatoms),Coord(3,Ntotatoms)
C
#ifdef _DEBUG_LVL0
CSS      Nshell = 2
      Write(6,"(a,2I3)") "The Debug value for Nshell: ", Nshells,
     &                    Ntotatoms
      do  i=1,nshells
         write(*,*) 'nshells=',i
         write(*,3913) i,katom(i),i,ktype(i)
3913     format('katom<',i2,'>  ',i2,' ktype<',i2,'>  ',i2)
         write(*,3914) i,kprim(i),i,kbfn(i)
3914     format('kprim<',i2,'>  ',i2,' kbfn<',i2,'>   ',i2)
         write(*,3915) i,nprims(i),i,ndegen(i)
3915     format('nprims<',i2,'> ',i2,' ndegen<',i2,'> ',i2)
         write(*,3916) i,kmini(i),i,kmaxi(i)
3916     format('kmini<',i2,'>  ',i2,' kmaxi<',i2,'>  ',i2)
         write(*,*)
      enddo
#endif
C
      Iloc   = 0
      Isoc   = 0
C
CSSS      Do Icent = 1, Ntotatoms 

      do ishell = 1, Nshells

CSSS      do ishell = 1, 1
C
        iatom  = katom(ishell)

CSSS        If (IGenby(Icent) .EQ. Iatom) Then

        xa = Coord(1,Iatom)
        ya = Coord(2,Iatom)
        za = Coord(3,Iatom)
        la = ktype(ishell) - 1

        kprimi=kprim(ishell)-1
        idegen=ndegen(ishell)
        idegen_grd=Ideg_grd(la)
        numcoi=numcon(ishell)

C begin loop jshell 
 
        Jloc = 0
        Jsoc = 0

CSSS        Do Jcent = 1, Ntotatoms

        do jshell=1, Nshells

CSSS        do jshell=1, 1

C
C get symmetry infor regarding shell j
C
          jatom=katom(jshell)
          lb = ktype(jshell) - 1
          Jdegen=ndegen(jshell)
C
CSSS          If (IGenby(Jcent) .EQ. Jatom) Then
 
          xb=Coord(1,Jatom)
          yb=Coord(2,Jatom)
          zb=Coord(3,Jatom)

          kprimj=kprim(jshell)-1
          numcoj=numcon(jshell)

C Get the density for this shell pair.
     
          Do iprim = 1, nprims(ishell)
CSSS          Do iprim =1, 1

             exp1 = expnt(kprim(ishell)+iprim-1)
             Indnpri = Kprimi + Iprim
                     
            Do jprim = 1, nprims(jshell)
CSSS            Do jprim = 1, 1 

                exp2 = expnt(kprim(jshell)+jprim-1)
                Indnprj = Kprimj + Jprim
C
#ifdef _DEBUG_LVL0
      Write(6, "(a, 4(1x,I2))") "The Ishell, Jshell, La, Lb: ", 
     &                           Ishell, Jshell, La, Lb
      Write(6, "(a, 4(1x,I2))") "The prim. pair; Iprim, Jprim",
     &                           Iprim, Jprim
      Write(6, "(a, 3(1x,F10.6))") "The XYZ of A : ", Xa, ya, Za
      Write(6, "(a, 3(1x,F10.6))") "The XYZ of B : ", Xb, yb, Zb
      Write(6, "(a, 2(1x,F10.6))") "The Primtive pair: ", Exp1, Exp2
      Write(6,*)
      Write(6,*) "Initilize debug data"
#endif 
#ifdef _DEBUG_LVLM
      Write(6,"(a)") "Initilize debug xyz of A and B"
      xa = 1.0
      ya = 1.0
      za = 1.0
      xb = -1.0
      yb = -1.0
      zb = -1.0
      exp1 = 1.5
      exp2 = 1.5 
      La = 2
      Lb = 2
      Write(6, "(a, 3(1x,F10.6))") "The XYZ of A : ", Xa, ya, Za
      Write(6, "(a, 3(1x,F10.6))") "The XYZ of B : ", Xb, yb, Zb
      Write(6, "(a, 2(1x,F10.6))") "The Primtive pair: ", Exp1, Exp2
      Write(6,*)
#endif
                Do Icnt = 1,  Ntotatoms

                   Isym_unq_cnt = Icnt

                   If (Ipseux(Icnt) .NE. 0) Then
                      
                      If (.NOT. (Grads .AND. Iatom .EQ. Icnt)) Then
                          Iecp_cnt = Icnt
C
                         Call Dzero(Ecpint_4shell, 
     &                              max_cofns_shell * max_cofns_shell)
                         Call Dzero(Cint, max_unfns_shell * 
     &                                    max_unfns_shell)

                         Call Ecp_grdint_4prim(Xa, Ya, Za, Xb, Yb, 
     &                                         Zb, La, Lb, Coord, Exp1,
     &                                         Exp2, Natoms, Ntotatoms, 
     &                                         Cint, Int, Zero_int, 
     &                                         Icnt, Iecp_cnt, Grads,
     &
     &                                         nshells,max_ecp_prims,
     &                                         necp_atoms,
     &                                         max_necp_functns,
     &                                         nval_prims,
     &                                         nval_ccoeffs,
     &                                         max_unfns_shell,
     &                                         max_cofns_shell,
     &                                         nval_bfns,max_nprims,
     &                                         max_ncontfns,
     &
     &                                         clp,zlp,nlp,kfirst,
     &                                         klast,llmax,nelecp,
     &                                         ipseux,ipseud,
     &
     &                                         numcon,katom,ktype,
     &                                         kprim,kmini,
     &                                         kmaxi,nprims,ndegen)




                         If (.NOT. Zero_int)
     &                   Call ecp_int_4shell(Cint, Ecpint_4shell, 
     &                                       La, Lb, Numcoi, Numcoj, 
     &                                       Indnpri, Indnprj, Iprim,
     &                                       Jprim, Jnt, Grads,
     &
     &                                       expnt_norms,Contr,
     &
     &                                       max_nprims,
     &                                       max_conts_shell,
     &                                       max_unfns_shell,
     &                                       max_cofns_shell)

         

#ifdef _DEBUG_LVL0
      Write(6,*)
      Write(6,"(a,6(1x,I2))")
     &"The Contracted integral for shell pair", Ishell, JShell, Iprim,
     & Jprim, Idegen_grd, Jdegen
       Do I = 1, Idegen_grd
          Write(6,"(4(1x,F20.13))")(Ecpint_4shell((I-1)*Jdegen+J),
     &                                 J=1,Jdegen)
      Enddo
#endif

                         Call A4_Ecpgrd_int(Ecpint_4shell, La, Lb, 
     &                                      Idegen_grd, Idegen,
     &                                      Jdegen, Numcoi, Numcoj,
     &                                      Iprim, Jprim, Exp1, Exp2,
     &                                      Ecpgrd_x, Ecpgrd_y, 
     &                                      Ecpgrd_z, Ntotatoms, 
     &                                      Iatom, ICnto,
     &
     &                                      max_cofns_shell)
                        
                         Call Ecp_grdint(Ecpgrdint, Ecpgrd_X, Ecpgrd_Y,
     &                                   Ecpgrd_Z, La, Lb, Iloc, Jloc, 
     &                                   Isoc, Jsoc, Numcoi, Numcoj, 
     &                                   Iatom, Icnt, Ntotatoms,
     &                                   Spherical,
     & 
     &                                   Ecpgrd_SX,Ecpgrd_SY,
     &                                   Ecpgrd_SZ,tmp1,tmp2,
     &
     &                                   max_cofns_shell,nval_bfns)

                      Endif
                   Endif 
C
                Enddo
C
             Enddo
          Enddo        

          Jloc = Jloc + (Lb+1)*(Lb+2)*Numcoj/2
          Jsoc = Jsoc + (2*Lb+1)*NUmcoj

CSSS        Endif 
        Enddo
CSSS        Enddo
        Iloc = Iloc + (La+1)*(La+2)*Numcoi/2
        Isoc = Isoc + (2*La+1)*Numcoi

CSSS      Endif 

      Enddo 

#ifdef _DEBUG_LVL0
       Nint = Iloc
       If (SPherical) Nint = Isoc 
       Write(6, "(a)") "The ECP derivative integral atom:xyz:int"
       Do icent = 1, Ntotatoms 
          w=0.0d0
          DO Ixyz = 1, 3 
                Write(6, "(6(1x,F12.6))") (Ecpgrdint(Icent,Ixyz,I),
     &                                    i=1, Nint)
             DO I = 1, Nint 
                w = w+Ecpgrdint(Icent,Ixyz,I)*Ecpgrdint(Icent,Ixyz,I)
             Enddo
          Enddo
       Write(6, "(a,(1x,F20.13))") "The ecpgrdint. check sum = ", w 
       Enddo 
#endif 
  
CSSS      Enddo

      return
      end
