C  Copyright (c) 2003-2010 University of Florida
C
C  This program is free software; you can redistribute it and/or modify
C  it under the terms of the GNU General Public License as published by
C  the Free Software Foundation; either version 2 of the License, or
C  (at your option) any later version.

C  This program is distributed in the hope that it will be useful,
C  but WITHOUT ANY WARRANTY; without even the implied warranty of
C  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
C  GNU General Public License for more details.

C  The GNU General Public License is included in this distribution
C  in the file COPYRIGHT.
      subroutine comp_return_1el_ecp_ints(
     *
     *                                   z, da1, da2, db1, db2, 
     *
     *                                   clp,zlp,nlp,kfirst_tmp,
     *                                   klast_tmp,llmax,nelecp,
     *                                   ipseux,ipseud,kfirst,klast,
     *                                   igenby,
     *
     *                                   iqmstr,jcostr,nucstr,nrcstr,
     *                                   jstrt,jrs,
     *
     *                                   palphas,palpha_norms,
     *                                   ppcoeffs,end_nfps,
     *                                   npfps,ncfps,indx_cc,
     *                                   ivangmom,pixalphas,
     *                                   pixpcoeffs,
     * 
     *                                   numcon,numpri,katom,
     *                                   ktype,kmini,kmaxi,
     *                                   ndegen,nprims,kprim,
     *
     *                                   expnt,contr,
     * 
     *                                   scr1,scr2,scr3,scr4,
     *                                    
     *                                   cint,ecpint_4shell,
     *                                   ecpint,tmp1,tmp2,tmp3,
     *
     *                                   ncenters,nshells,nalphas,
     *                                   npcoeffs,max_nshells,
     *
     *                                   pccbeg,pccend,cclength,
     *
     *                                   kfls_ecp_length,
     *
     *                                   max_ecp_angmom,max_ecp_prims, 
     *                                   max_nsubshells,
     *                                   max_necp_functns,necp_atoms,
     *                                      
     *                                   nval_prims,nval_ccoeffs,
     *                                   nval_bfns,max_nprims,
     *                                   max_ncontfns,
     *
     *                                   max_prims_shell,
     *                                   max_conts_shell,
     *                                   max_unfns_shell,
     *                                   max_cofns_shell,
     *
     *                                   spherical,ecp,
     * 
     *                                   coords,tcoords,atom,
     *                                   charges, 
     *                                   
     *                                   zz, iscr,zsum)
c---------------------------------------------------------------------------
!>  special return_h1 w
!>
!>  Computes the set of OED integrals of types  
!>
!>   type = 1 --> NAI integrals
!>          2 --> KIN integrals
c---------------------------------------------------------------------------

      use, intrinsic :: ISO_C_BINDING 
      implicit none
    
c---------------------------------------------------------------------------
c Interface added but should be included instead 

      include 'sip_interface.f' 
      include 'imemory.h' 

c---------------------------------------------------------------------------
      integer iscr(*) 

      integer da1, da2, db1, db2
      integer adim, bdim 
      double precision zsum(da1:da2, db1:db2)
      double precision z(da1:da2, db1:db2)  
      double precision zz(*)  

      integer ix, m, n, nfirst, nints, ncoeff, iatom, ncsum
      integer component
      integer a, b, a1,a2,b1,b2,lastao, igrad
      integer m1, m2, n1, n2
      integer der1x, der1y, der1z
      integer der2x, der2y, der2z
      integer dercx, dercy, dercz
      integer ixderc
      integer ind(2)
      integer ierr 
      integer two 
       
      integer i, j, k

      integer flopcount
      logical spherical, ecp
   
      integer max_dim_coeff
      parameter (max_dim_coeff = 5000)
      integer ccbeg_pack(max_dim_coeff), ccend_pack(max_dim_coeff)
      integer nalpha_pack, ncoeff_pack 
      double precision alpha_pack(max_dim_coeff), 
     *                 pcoeff_pack(max_dim_coeff)
      double precision xn(ncenters), yn(ncenters), zn(ncenters)
      double precision x1,y1,z1,x2,y2,z2,y,sum,w

      integer (C_INT) one  
C
c Nuclear coordinates, charges
C 
C ncenters      : The total number of centers (ECP or not)
C nshells       : The total number of shell of the valence basis set.
C atom          : Contains a map of shells to centers
C max_ecp_angmom: The maximum L value of ECP (among all atoms).
C max_ecp_prims : The maximum no. of ECP primitives (among all atoms).
C nalphas       : The total number of primitives
C nocoeffs      : The total number of contraction coefficients, 
C 
c --------------------------------------------------------------------------- 
      integer ncenters,nalphas,npcoeffs,nshells 
      integer max_ecp_prims,max_nshells
      integer max_nsubshells
      integer max_necp_functns
      integer necp_atoms,max_ecp_angmom
      integer nval_prims,nval_ccoeffs,nval_bfns
      integer max_nprims,max_ncontfns
      integer kfls_ecp_length
      integer max_prims_shell
      integer max_conts_shell
      integer max_unfns_shell
      integer max_cofns_shell
      integer cclength 

      integer atom(nshells)
      double precision coords(3,ncenters) 
      double precision tcoords(3,nshells)
      double precision charges(ncenters)
C
c --------------------------------------------------------------------------- 
C The ECP related variables (unfortunately I must keep the legacy varible 
C names in order to avoid cumbersomse benefit less changes in ECP integral
C library). 
C
C llmax            : The maximum ECP ang. momentum of each ECP center.
C nelecp           : The number of electron replaced by ECP potential 
C                    each ECP center.
C ipseux           : 0 or 1 depending on whether there is ECP potential on 
C                    given center.
C kfirst           : Offset of the begining index of the nlp,zlp,clp
C                    arrays (see below).
C klast            : Offset of the endng index of the nlp,zlp,clp
C                    arrays (see below).
C                  :
C                  : kfirst_tmp and klast_tmp has the same info. as
C                  : kfirat and klast but in a format required by
C                  : ECP ints codes(?).
C zlp;clp          : The ECP exponents and contraction coefficients.
C nlp              : ECP radial exponent (usually, 0, 1 or 2).
C necp_atoms       : The number of ECP atoms is the totall number of 
C                    atoms with an ECP.
C max_necp_functns : The maximum possible number of ECP functions given 
C                    all the ECP centers. 
C max_prims_shell  : The size of the largest shell based on the number 
C                    primitives of all centers.
C max_conts_shell  : The size of the largest shell based on the number 
C                     contracted function of all centers.
C Cint,ecpint_4shell,Ecpint holds ECP integrals at various level of 
C their generation. 

      integer llmax(ncenters)
      integer ipseud
      integer nelecp(ncenters)
      integer ipseux(ncenters)
      integer kfirst(max_necp_functns*necp_atoms)
      integer klast(necp_atoms*max_necp_functns) 
      integer kfirst_tmp(kfls_ecp_length)
      integer klast_tmp(kfls_ecp_length) 
      integer nlp(max_ecp_prims)
      double precision clp(max_ecp_prims)
      double precision zlp(max_ecp_prims)

      double precision cint(max_unfns_shell*max_unfns_shell)
      double precision ecpint_4shell(max_cofns_shell*max_cofns_shell)
      double precision ecpint(nval_bfns*(nval_bfns)/2)
      double precision tmp1(max_cofns_shell*max_cofns_shell)
      double precision tmp2(max_cofns_shell*max_cofns_shell)
      double precision tmp3(max_cofns_shell*max_cofns_shell)
C
C The valence basis set related variables (once again varible names are 
C makes no common sense but had to be kept for the compliancy). These are 
C are hold over form the ECP codes for ACES II (and latter ACES III)
C 
C iqmstr          : The number of shells per centers.
C jcostr          : The number of sub-shells (usually 1). This to some
C                 : extent a old concept (ie. split a given ang. momentum
C                 : to mutiple shells to save memoery requiremens).
C nucstr          : The number of primitives on a center, a shell, 
C                 : a sub-shell.
C nrcstr          : The number of contracted functions on a center, a shell, 
C                 : a sub-shell (safe to say that it is the total number 
C                 : of shells.
C jstrt           : Offset for the exponents
C jrs             : Offset for the contracted functions,
C max_nshells     : Different from totall number of shells. This is the 
C                   maximum number of shells on a atom given all the 
C                   atoms.
C max_nsubshells  : This is the maximum number of subshells (among all the
C                   atoms and the shells)

      integer iqmstr(ncenters)
      integer jcostr(nshells) 
      integer nucstr(nshells)
      integer nrcstr(nshells)
      integer Jstrt(nshells) 
      integer Jrs(nshells)
C
C The valence basis set info. for ERD/OED from ACES III. Noth that 
C some of these have overlap with the above block. I have indicated
C where there is an ovelap. 
C 
C end_nfps       : Offset for the contracted funtions (counting 3 for
C                  p, 5(or) 6 for ds etc.)
C indx_cc        : Offset for the contracted functions (without 
C                : counting the degeneracy of the p, d, etc. shells)
C npfps          : The number of primitive function on a center, a shell.
C ncfps          : The number of contractes function on a center, a shell.
C                  (the npfps and ncfps have identical info. as nucstr
C                   nrcstr except the fact the possiblity of having 
C                   subshells is discarded).
C ivangmom       : The angular momentum of valence shells
C pixalpha       : An offset for the exponents.
C pixpcoeffs     : An offset for the contraction coefficents. 
C                  (the ixalpha and  ixpcoeffs have more or less the 
C                  same data; one can generate from the other).
C pccbeg,pccend  : The begining and ending indices of contraction coeffs. 
C palphas        : The valence basis exponents 
C palpha_norms   : The exponent dependent normalization constants.
C ppcoeffs       : The valence basis contraction coeffs. 
C nval_bfns      : The total number of valemnce basis funtions.
c 

      integer end_nfps(nshells)
      integer indx_cc(nshells) 
      integer npfps(nshells)
      integer ncfps(nshells)
      integer ivangmom(nshells)
      integer pixalphas(nshells)
      integer pixpcoeffs(nshells)
      double precision palphas(nval_prims)
      double precision palpha_norms(nval_prims)
      double precision ppcoeffs(nval_ccoeffs)
      double precision pccbeg(cclength)
      double precision pccend(cclength)
C
C The auxilary arrays. These are in some sense unfortuante but unavoidable.
C The ECP ints and ERD/OED are developed independently and so the 
C requirement are sligtly different (ie. Same data, but different order
C ect.).
C  
C numcon       : The number of contration functions of each shell.
C                (constructed from nrcstr),
C numpri       : The number of primives of each shell 
C                (constructed from nucstr),
C ipeoff       : An offset to reorder palphas (reordering may not 
C                change the ordering of the original array).
C iccoff       : An offset to reorder the ppcoeffs array.
C katom        : Mapping of centers to shells (same as atom).
C ktype        : The angular momentum of shells (same as ivangmom)
C kmini        : (l-1)*l*(l+1)/6+1 where l is the ang. momentum of the shell.
C kmaxi        : (l*(l+1)*(l+2)/6  where l is the ang. momentum of the shell.
C ndegen       : l*(l+1)/2 where l is the ang. momentum of the shell.
C igenby       : A hold over from legacy codes that designate sym. equ.
C              : atoms. Since there is no symmetry in ACES IV, this is 
C              : is nothing but atom ordering. 
C nprims       : The number of primitives of a shell.
C kprim        : An offset for the contraction coefficints. Identical 
C                to pixpcoeffs.
C scr1-4      : Scratch arrays.
  
      integer numcon(nshells)
      integer numpri(nshells)
      integer katom(nshells)
      integer ktype(nshells)
      integer kmini(nshells)
      integer kmaxi(nshells)
      integer ndegen(nshells)
      integer nprims(nshells)
      integer kprim(nshells)
      integer imarked(ncenters)
      integer igenby(ncenters)
   
      integer scr1(ncenters)
      integer scr2(ncenters,max_nshells,max_nsubshells)
      integer scr3(ncenters,max_nshells,max_nsubshells)
      integer scr4(ncenters,max_nshells,max_nsubshells)
      double precision expnt(max_nprims)
      double precision contr(max_nprims,max_conts_shell) 
C 
#ifdef _DEBUG_LVL0
      Write(6,"(a)") "----------------------------------------"
      Write(6,"(a)") " Printing from comp_return_1el_ecpints "
      Write(6,"(a,i4)") " The number of atoms: ", ncenters
      Write(6,"(a,i4)") " The number of shells: ", nshells
      Write(6,"(a,i4)") " The number contraction coefs.: ", 
     *                    npcoeffs
      Write(6,"(a,i4)") " The number primitive (exponents).: ",
     *                    nalphas
      Write(6,"(a,i4)") " The number of atoms with ecp: ",
     *                    necp_atoms
      Write(6,"(a,i4)") " The maximum ecp ang. mom.: ",
     *                    max_ecp_angmom
      Write(6,"(a,i4)") " The maximum ecp primitives: ",
     *                    max_ecp_prims
      Write(6,"(a,i4)") " The maximum ecp exponents: ",
     *                    max_necp_functns
      Write(6,"(a,l)") " The spherical on/off: ", spherical
      Write(6,"(a,l)") " The ECP on/off: ", ecp
      write(*,*)

      Write(6,"(a)") " The end_nfps array"
      Write(6,"(10i4)") (end_nfps(i), i=1,nshells)
      write(*,*)

      Write(6,"(a)") " The npfps array"
      Write(6,"(10i4)") (npfps(i), i=1,nshells)
      write(*,*)
      Write(6,"(a)") " The ncfps array"
      Write(6,"(10i4)") (ncfps(i), i=1,nshells)
      write(*,*)

      Write(6,"(a)") " The indx_cc array"
      Write(6,"(10i4)") (indx_cc(i), i=1,nshells)
      write(*,*)

      Write(6,"(a)") " The angular momentum array"
      Write(6,"(10i4)") (ivangmom(i), i=1,nshells)
      write(*,*)

      Write(6,"(a,i4)") " The exponents array"
      Write(6,"(5F15.7)") (palphas(i), i=1,nval_prims)
      write(*,*)

      Write(6,"(a,i4)") " The exponents dependent norms"
      Write(6,"(5F15.7)") (palpha_norms(i), i=1,nval_prims)
      write(*,*)

      Write(6,"(a)") " The contraction coeffs. array"
      Write(6,"(5F15.7)") (ppcoeffs(i), i=1,nval_ccoeffs)
      write(*,*)

      Write(6,"(a)") " The ixalpha array"
      Write(6,"(10I4)") (pixalphas(i), i=1,Nshells)
      write(*,*)

      Write(6,"(a)") " The ixpcoef array"
      Write(6,"(10I4)") (pixpcoeffs(i), i=1,Nshells)
      write(*,*)

      Write(6,"(a)") " The ipsuex array"
      Write(6,"(10I4)") (ipseux(i), i=1,ncenters)
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
      Write(6,"(10I4)") (llmax(i), i=1,ncenters)
      write(*,*)

      Write(6,"(a)") " The kfirst_tmp array"
      Write(6,"(10I4)") (kfirst_tmp(i), i=1,kfls_ecp_length)
      write(*,*)

      Write(6,"(a)") " The klast_tmp array"
      Write(6,"(10I4)") (klast_tmp(i), i=1,kfls_ecp_length)
      write(*,*)

      Write(6,"(a)") " The nelecp array"
      Write(6,"(10I4)") (nelecp(i), i=1,ncenters)
      write(*,*)

      Write(6,"(a)") " The iqmstr array"
      Write(6,"(10I4)") (iqmstr(i), i=1,ncenters)
      write(*,*)

      Write(6,"(a)") " The jstrt array"
      Write(6,"(10I4)") (jstrt(i), i=1,nshells)
      write(*,*)

      Write(6,"(a)") " The jrs array"
      Write(6,"(10I4)") (jrs(i), i=1,nshells)
      write(*,*)

      Write(6,"(a)") " The jcostr array"
      Write(6,"(10I4)") (jcostr(i), i=1,nshells)
      write(*,*)

      Write(6,"(a)") " The nucrst array"
      Write(6,"(10I4)") (nucstr(i), i=1,nshells)
      write(*,*)

      Write(6,"(a)") " The nrcstr array"
      Write(6,"(10I4)") (nrcstr(i), i=1,nshells)
      write(*,*)

      Write(6,"(a)") " The atom coords. array"
      Write(6,"(3F12.6)") ((coords(i,j),i=1,3),j=1,ncenters)
      write(*,*)

      Write(6,"(a)") " The atom charges array"
      Write(6,"(6F12.6)") (charges(i),i=1,ncenters)
      write(*,*)

      Write(6,"(a)") " The atom coords. of shells array"
      Write(6,"(3F12.6)") ((tcoords(i,j),i=1,3),j=1,nshells)
      write(*,*)

      Write(6,"(a)") " The atom to shell map"
      Write(6,"(10I4)") (atom(i), i=1,nshells)
      write(*,*)
      Write(6,"(a)") " Printing from comp_return_1el_ecpints ENDS"
      Write(6,"(a)") "----------------------------------------"
#endif 
      Call a4_ecp_main(nshells,ncenters,max_nshells,
     &                 max_nsubshells,
     &                 max_nprims,nval_bfns,max_ncontfns,
     &                 nval_prims,nval_ccoeffs,
     &
     &                 max_prims_shell,
     &                 max_conts_shell,
     &                 max_unfns_shell,
     &                 max_cofns_shell,
     &
     &                 kfls_ecp_length,necp_atoms,
     &                 max_necp_functns,max_ecp_angmom,
     &                 max_ecp_prims,
     &
     &                 clp,zlp,nlp,llmax,nelecp,ipseux,ipseud,
     &                 igenby,
     &                    
     &                 cint,ecpint_4shell,ecpint,tmp1,tmp2,
     &                 tmp3,
     &                    
     &                 iqmstr,jcostr,nucstr,nrcstr,Jstrt,jrs,
     &                 palphas,palpha_norms,ppcoeffs,
     & 
     &                 kfirst_tmp,klast_tmp, 
     &                           
     &                 numcon,numpri,katom,ktype,kmini,kmaxi,
     &                 ndegen,nprims,kprim,
     &
     &                 charges,coords,tcoords,atom,
     &    
     &                 expnt,contr,
     &
     &                 kfirst,klast, 
     &                       
     &                 scr1,scr2,scr3,scr4,
     &                       
     &                 spherical,ecp)

#ifdef _DEBUG_LVL0
      w=0.0d0
      Write(6,*)
      Write(6,"(a)") "@-comp_return_1el_ecp_ints, The ECP integrals"
      Write(6,"(a,1x,i3)") "The dimension of ints: ", nval_bfns
      do i =1, nval_bfns*(nval_bfns+1)/2
      W = w+ecpint(i)*ecpint(i)
      enddo
      Write(6, "(a,(1x,F20.13))") "The ecpint. check sum = ", w
      Write(6,*)
#endif

      i = 0
      do b = db1,db2
      do a = 1, b
         i = i + 1
         z(a,b) = ecpint(i) 
         z(b,a) = ecpint(i) 
      enddo
      enddo

#ifdef _DEBUG_LVL0
      write(6,*)
      Write(6,"(a)") " Printing from comp_return_1el_ecp_ints ENDS"
      Write(6,"(a)") "--------------------------------------------"
#endif 

#ifdef _NOSKIP
      do b = db1,db2
      do a = da1,da2
         i = i + 1
         zsum(a,b) = 0.d0
      enddo
      enddo
      Write(6,"(a)") " Printing from the begining of T+V"
      Write(6,"(a)") "----------------------------------------"

      ind(1) = 1
      ind(2) = 2

c-------------------------------------------------------------------------
c   Build an array of coordinates per atom.
c-------------------------------------------------------------------------

      do m = 1, ncenters  
         xn(m) = coords(1,m)  
         yn(m) = coords(2,m)  
         zn(m) = coords(3,m)  
      enddo

c-------------------------------------------------------------------------
c   Find the shells we need to loop over for this block.
c--------------------------------------------------------------------------

      call lookup_shell(end_nfps, nshells, da1, m1)
      call lookup_shell(end_nfps, nshells, da2, m2)
      call lookup_shell(end_nfps, nshells, db1, n1)
      call lookup_shell(end_nfps, nshells, db2, n2)

c-------------------------------------------------------------------------
c   Calculate the integrals over the necessary shell blocks.
c-------------------------------------------------------------------------

       do m = m1, m2
            x1 = tcoords(1,m)  
            y1 = tcoords(2,m)  
            z1 = tcoords(3,m)  
       do n = n1, n2

            ncsum = ncfps(m) + ncfps(n)

            x2 = tcoords(1,n)  
            y2 = tcoords(2,n)  
            z2 = tcoords(3,n)  

             call pack_coeffs_oed(palphas, pixalphas, ppcoeffs, 
     *                       pixpcoeffs, ncfps, npfps, m, n,
     *                       alpha_pack, nalpha_pack, pcoeff_pack,
     *                       ncoeff_pack, pccbeg, pccend, 
     *                       indx_cc,
     *                       ccbeg_pack, ccend_pack,
     *                       max_dim_coeff)

#ifdef _DEBUG_LV0
      Write(6,*) "@-entry to ERD_batch"
      Write(6,"((a,2(1x,i4)))") "m, n:", m, n
      Write(6,"(a,2(1x,I4))") "The Nalpha_pack and Ncoeff_pack:",
     &                         nalpha_pack, ncoeff_pack
      Write(6,"((a,2(1x,i4)))") "Ivanagmom:", Ivangmom(m),
     &      Ivangmom(n)
      Write(6,"((a,2(1x,i4)))") "Ncfps:", Ncfps(m),
     &      Ncfps(n)
      Write(6,"((a,2(1x,i4)))") "Npfps:", Npfps(m),
     &      Npfps(n)
      Write(6,*)
      Write(6,"(4(1x,F15.7))") (Alpha_pack(i),i=1,Nalpha_pack)
      Write(6,*)
      Write(6,"(4(1x,F15.7))")(Pcoeff_pack(i),i=1,ncoeff_pack)
      Write(6,*)
      Write(6,"(4(1x,I4))")(CCbeg_pack(i),i=1,Nalpha_pack)
      Write(6,*)
      Write(6,"(4(1x,I4))")(CCend_pack(i),i=1,Nalpha_pack)
      Write(6,*)
      Write(6,"(a,2(1x,I10))") "The imax,Zmax:",imax, zmax
      Write(6,*) "The coordiantes"
      Write(6,*)
      Write(6,"(3(1x,F10.5))") x1, y1, z1
      Write(6,"(3(1x,F10.5))") x2, y2, z2
#endif
c ------------------------------------------------------------------------ 
c  KIN (type=2) integrals next.   
c ------------------------------------------------------------------------ 

              call oed__gener_kin_batch(imax, zmax, nalpha_pack,
     *                 ncoeff_pack,
     *                 ncsum, ncfps(m), ncfps(n), npfps(m),npfps(n),
     *                 ivangmom(m), ivangmom(n), x1,y1,z1,x2,y2,z2,
     *                 alpha_pack,
     *                 pcoeff_pack, ccbeg_pack, ccend_pack,
     *                 spherical, .true., iscr, nints, nfirst,
     *                 zz)
C           write(6,*) ' NINTS = ', nints 
            if (nints .gt. 0) then
               a2 = end_nfps(m)
               if (m .eq. 1) then
                  a1 = 1 
               else
                  a1 = end_nfps(m-1)+1
               endif

               b2 = end_nfps(n)
               if (n .eq. 1) then
                  b1 = 1 
               else
                  b1 = end_nfps(n-1)+1
               endif

                call add_integrals2(zsum, da1,da2,db1,db2,
     *                 zz(nfirst),a1,a2,b1,b2,1.0d0)

             endif ! (nints .gt. 0) then

c ------------------------------------------------------------------------ 
c NAI (type=1) integrals first.   
c ------------------------------------------------------------------------ 
              call oed__gener_nai_batch(imax, zmax, 
     *                 nalpha_pack, ncoeff_pack,
     *                 ncsum, ncfps(m), ncfps(n), npfps(m),npfps(n),
     *                 ivangmom(m), ivangmom(n), x1,y1,z1,x2,y2,z2,
     *                 ncenters, xn,yn,zn, charges, alpha_pack,
     *                 pcoeff_pack, ccbeg_pack, ccend_pack,
     *                 spherical, .true., iscr, nints, nfirst,
     *                 zz)

               if (nints .gt. 0) then
                  a2 = end_nfps(m)
                  if (m .eq. 1) then
                     a1 = 1 
                  else
                     a1 = end_nfps(m-1)+1
                  endif

                  b2 = end_nfps(n)
                  if (n .eq. 1) then
                     b1 = 1
                  else
                     b1 = end_nfps(n-1)+1
                  endif

                 call add_integrals2(zsum, da1,da2,db1,db2,
     *                 zz(nfirst),a1,a2,b1,b2,1.0d0)

               endif ! nints
        enddo   ! n shells
        enddo   ! m shells 
      Write(6,"(a)") " Printing from T+V ENDS"
      Write(6,"(a)") "----------------------------------------"
C#ifdef _DEBUG_LVL0
C      write(6,"(a)") 'One Hamiltonian (T+V) integrals ' 
C      write(6,"(4(1x,F20.9))") ((zsum(a,b),a=da1,da2),b=db1,db2)
      sum = 0.0D0
      do a=da1,da2
      do b=db1,db2
         i = i + 1
         sum = sum + zsum(b,a)
      enddo
      enddo
      write(6,"(a,F20.10)")" The one Hamiltonian integral checkum =",sum
      write(6,*)
C#endif 
#endif 

      return
      end
