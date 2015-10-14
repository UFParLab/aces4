      subroutine A4_ecp_int_driver(Ecpint,Ecpint_4shell,Cint,
     &                             tmp1,tmp2,tmp3,
     &
     &                             nshells,max_ecp_prims,
     &                             necp_atoms,max_necp_functns,
     &                             nval_prims,nval_ccoeffs,
     &                             max_prims_shell,max_conts_shell,
     &                             max_unfns_shell,max_cofns_shell,
     &                             nval_bfns,max_nprims,max_ncontfns,
     &                             ntotatoms,
     &
     &                             clp,zlp,nlp,kfirst,klast,llmax,
     &                             nelecp,ipseux,ipseud,igenby,
     & 
     &                             numcon,katom,ktype,kprim,kmini,
     &                             kmaxi,nprims,ndegen,expnt,
     &                             expnt_norms,contr,
     &  
     &                             Coord,Grads,Spherical)
c----------------------------------------------------------------------
c     Ecp_int_driver drives the calculation of 1e-matrices in the basis of
c     cartesian atomic orbitals using subroutine ecp_int.
c----------------------------------------------------------------------
c     tol    = integral cutoff (ints to be neglected, if value expected
c              to be < exp(-tol)(i)
c     Cint   = 1e-integral array ('batch') of dimension nfij
c       
c----------------------------------------------------------------------
      implicit double precision (a-h,o-z)
C-----------------------------------------------------------------------
#include "ecp.par"
#include "ecp.com"

C Ecpints need to be (2*Maxang+1)**2 times the square of the 
C maximum number of contracted functions per shell (currently
C set at 20 for ECP integrals). The current Max_cbf is 1000. 
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
      integer kfirst(max_necp_functns,necp_atoms)
      integer klast(max_necp_functns,necp_atoms)
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
      dimension Ecpint(nval_bfns * (nval_bfns+1)/2) 
      dimension tmp1(max_cofns_shell * max_cofns_shell)
      dimension tmp2(max_cofns_shell * max_cofns_shell)
      dimension tmp3(max_cofns_shell * max_cofns_shell)
C
      dimension IGenby(Ntotatoms),Coord(3,Ntotatoms)
c
c-----------------------------------------------------------------------
#ifdef _DEBUG_LVL0
      Write(6,"(a)") " Printing from A4_ecp_int_driver BGNS"
      Write(6,"(a)") "----------------------------------------"
      Write(6,"(a,I4)") " The ipseud --", ipseud
      Write(6,"(a)") " The ipsuex array"
      Write(6,"(10I4)") (ipseux(i), i=1,ntotatoms)
      write(*,*)

      Write(6,"(a)") " The nlp array"
      Write(6,"(10I4)") (nlp(i), i=1,max_ecp_prims)
      write(*,*)

      Write(6,"(a)") " The clp array"
      Write(6,"(5F15.7)") (clp(i), i=1,max_ecp_prims)
      write(*,*)

      Write(6,"(a)") " The zlp array"
      Write(6,"(5F15.7)") (zlp(i), i=1,max_ecp_prims)
      write(*,*)

      Write(6,"(a)") " The llmax array"
      Write(6,"(10I4)") (llmax(i), i=1,ntotatoms)
      write(*,*)

      Write(6,"(a)") " The nelecp array"
      Write(6,"(10I4)") (nelecp(i), i=1,ntotatoms)
      write(6,*)

      Write(6,"(a)") " The kfirst and klast arrays"
      do i=1,necp_atoms
           write(6,*) (kfirst(i,k),k=1,max_necp_functns)
      enddo 
      do i=1,necp_atoms
         write(6,*) (klast(i,k), k=1,max_necp_functns)
      enddo 

      write(6,*)
      Write(6,"(a)") " The numcon array"
      Write(6,"(10I4)") (numcon(i), i=1,nshells)
      write(6,*)
      Write(6,"(a)") " The katom array"
      Write(6,"(6I)") (katom(i), i=1, Nshells)
      write(6,*)
      Write(6,"(a)") " The ktype array"
      Write(6,"(6I)") (ktype(i), i=1, Nshells)
      write(6,*)
      Write(6,"(a)") " The kmini array"
      Write(6,"(6I)") (kmini(i), i=1, Nshells)
      write(6,*)
      Write(6,"(a)") " The kmaxi array"
      Write(6,"(6I)") (kmaxi(i), i=1, Nshells)
      write(6,*)
      Write(6,"(6I)") (ndegen(i), i=1, Nshells)
      Write(6,"(a)") " The nprims array"
      Write(6,"(6I)") (nprims(i), i=1, Nshells)
      write(6,*)
      Write(6,"(a)") " The kprims array"
      Write(6,"(6I)") (kprim(i), i=1, Nshells)
      write(6,*)
      Write(6,"(a)") " The expnt array"
      Write(6,"(5F15.7)") (expnt(i), i=1,max_nprims)
      write(*,*)
      Write(6,"(a)") "----------------------------------------"
      Write(6,"(a)") " Printing from A4_ecp_int_driver BGNS"
#endif 

#ifdef _DEBUG_LVL0
CSS      Nshell = 2
      Write(6,"(a,I3)") "The Debug value for Nshell: ", Nshell
CSS      nprims(2) = 1
#endif

      Iloc   = 0
      Isoc   = 0

CSSS      Do Icent = 1, Ntotatoms

      Do ishell = 1, Nshells

CSSS      do ishell = 1, 1

        iatom  = katom(ishell)

CSSS        If (IGenby(Icent) .EQ. Iatom) Then

        xa = Coord(1,iatom)
        ya = Coord(2,iatom)
        za = Coord(3,iatom)
        la = ktype(ishell) - 1

        kprimi=kprim(ishell)-1
        numcoi=numcon(ishell)

C begin loop jshell 

        Jloc = 0
        Jsoc = 0

CSSS        Do Jcent = 1, Icent

        do jshell=1, Nshells

CSSS        do jshell=1, 1

          jatom=katom(jshell)
          lb = ktype(jshell) - 1
C
#ifdef _DEBUG_LVL0       
      Write(6,"(a)") " The katom array"
      Write(6,"(6I)") (katom(i), i=1, Nshells)
      write(6,*)
      Write(6,"(a)") "Iatom, Jatom, Ishell, Jshell, Jloc: " 
      write(6,"(6(1x,i4))") Iatom, Jatom, Ishell, Jshell, Jloc
      Write(6,*)
#endif
CSSS          If (IGenby(Jcent) .EQ. Jatom) Then
CSSS          If (.NOT. (ICent .EQ. Jcent .AND. JShell .GT. IShell)) Then

          If (.NOT. (Iatom .EQ. Jatom .AND. JShell .GT. IShell)) Then
C
          xb=Coord(1,jatom)
          yb=Coord(2,jatom)
          zb=Coord(3,jatom)

          kprimj=kprim(jshell)-1
          numcoj=numcon(jshell)

          Call Dzero(Ecpint_4shell, max_cofns_shell * max_cofns_shell)
C
          Do iprim = 1, nprims(ishell)
CSSS                  Do iprim = 3, 3

              exp1 = expnt(kprim(ishell)+iprim-1)
              Indnpri = Kprimi + Iprim
                     
              Do jprim = 1, nprims(jshell)
CSSS                     Do jprim = 3, 3
                 exp2 = expnt(kprim(jshell)+jprim-1)
                 Indnprj = Kprimj + Jprim
                 Call Dzero(Cint, max_unfns_shell * max_unfns_shell)
C
#ifdef _DEBUG_LVL0
      Write(6, "(a, 4(1x,I2))") "The Ishell, Jshell, La, Lb: ", 
     &                           Ishell, Jshell, La, Lb
      Write(6, "(a, 4(1x,I2))") "The prim. pair; Iprim, Jprim",
     &                           Iprim, Jprim
#endif
#ifdef _DEBUG_LVL0
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
                 Call Ecp_int_4prim(Xa, Ya, Za, Xb, Yb, Zb, 
     &                              La, Lb, Coord, Exp1, Exp2,
     &                              Ntotatoms, Ntotatoms, IGenby,
     &                              Cint, Int, Zero_int, 
     &                              Grads,
     & 
     &                              nshells,max_ecp_prims,
     &                              necp_atoms,max_necp_functns,
     &                              nval_prims,nval_ccoeffs,
     &                              max_unfns_shell,max_cofns_shell,
     &                              nval_bfns,max_nprims,max_ncontfns,
     &
     &                              clp,zlp,nlp,kfirst,klast,llmax,
     &                              nelecp,ipseux,ipseud,
     &
     &                              numcon,katom,ktype,kprim,kmini,
     &                              kmaxi,nprims,ndegen)
#ifdef _DEBUG_LVL0
      write(6,*) 
      write(6,"(a,l)") " Finished the ecp_int_4prim.F", zero_int
#endif 

                 If (.NOT. Zero_int)
     &              Call ecp_int_4shell(Cint, Ecpint_4shell, 
     &                                  La, Lb, Numcoi, Numcoj, 
     &                                  Indnpri, Indnprj, Iprim,
     &                                  Jprim, Jnt, Grads,
     &                                       
     &                                  expnt_norms,Contr,
     &                                  
     &                                  max_nprims,max_conts_shell,
     &                                  max_unfns_shell,
     &                                  max_cofns_shell)

             Enddo
          Enddo        
C 
#ifdef _DEBUG_LVL0
      Write(6,*)
      Idegen = (la+1)*(la+2)/2
      Jdegen = (lb+1)*(lb+2)/2
      Write(6,"(a,4(1x,I2))")
     &"The Contracted integral for shell pair", Ishell, JShell, Idegen,
     & Jdegen

      If (Ishell .NE. Jshell) Then
          Do I = 1, Idegen
             Write(6,"(4(1x,F20.13))")(Ecpint_4shell((I-1)*Jdegen+J),
     &                                 J=1,Jdegen)
          Enddo
      Else
          Do I = 1, Idegen
             Write(6,"(4(1x,F20.13))")(Ecpint_4shell((I-1)*Jdegen+J),
     &                                 J=1,I)
          Enddo
      Endif
#endif  
                  Call Ecp_int(Ecpint, Ecpint_4shell, 
     &                         La, Lb, Iloc, Jloc, Isoc, Jsoc, 
     &                         Numcoi, Numcoj, Ishell, Jshell,
     &                         Spherical,
     &
     &                         tmp1,tmp2,tmp3,
     & 
     &                         max_confns_shell,nval_bfns) 

#ifdef _DEBUG_LVL0
      Write(6,*) " Returned from Ecp_int"
#endif
C
          Jloc = Jloc + (Lb+1)*(Lb+2)*Numcoj/2
          Jsoc = Jsoc + (2*Lb+1)*NUmcoj
C
        Endif
cSSS        Endif
C
        Enddo
CSSS        Enddo

        Iloc = Iloc + (La+1)*(La+2)*Numcoi/2
        Isoc = Isoc + (2*La+1)*Numcoi
C
CSSS      Endif 
      Enddo 
CSSS      Enddo
C     
      Ndim = Iloc
      If (Spherical) Ndim = Isoc 
#ifdef _DEBUG_LVL0
      w=0.0d0
      Write(6,"(a)") "@-Ecpint_Driver, The ECP integrals"
      Write(6,"(a,1x,i3)") "The dimension of ints: ", Ndim
CSSS      Write(6, "(6(1x,F10.7))") (Ecpint(I), I=1, Ndim*(Ndim+1)/2)
      do i =1, Ndim*(Ndim+1)/2
      W = w+ecpint(i)*ecpint(i)
      enddo
      Write(6, "(a,(1x,F20.13))") "The ecpint. check sum = ", w
#endif
C
      return
      end
