      subroutine A4_ECP_MAIN(nshells,ncenters,max_nshells,
     &                       max_nsubshells,
     &                       max_nprims,nval_bfns,max_ncontfns,
     &                       nval_prims,nval_ccoeffs,
     &
     &                       max_prims_shell,
     &                       max_conts_shell,
     &                       max_unfns_shell,
     &                       max_cofns_shell,
     &  
     &                       kfls_ecp_length,necp_atoms,
     &                       max_necp_functns,
     &                       max_ecp_angmom,max_ecp_prims,
     &
     &                       clp,zlp,nlp,llmax,nelecp,ipseux,
     &                       ipseud,igenby,
     &
     &                       cint,ecpint_4shell,ecpint,tmp1,
     &                       tmp2,tmp3,
     &
     &                       iqmstr,jcostr,nucstr,nrcstr,Jstrt,jrs,
     &                       palphas,expnt_norms,ppcoeffs,
     &
     &                       kfirst_tmp,klast_tmp,
     &
     &                       numcon,numpri,katom,ktype,kmini,kmaxi,
     &                       ndegen,nprims,kprim,
     & 
     &                       charges,coords,tcoords,atom, 
     &
     &                       expnt,contr,
     &
     &                       kfirst,klast,
     &
     &                       scr1,scr2,scr3,scr4,
     &
     &                       spherical)

C      
      Implicit double precision (A-H, O-Z)

#include "ecp.par"
#include "ecp.com"
C
      Dimension iqmstr(ncenters),jcostr(ncenters,nshells)
      Dimension nucstr(nshells)
      Dimension nrcstr(nshells)
      Dimension Jstrt(nshells),Jrs(nshells)
      Dimension palphas(nval_prims),ppcoeffs(nval_ccoeffs)
      Dimension expnt_norms(nval_prims)
      Dimension imarked(ncenters)
      Dimension kfirst_tmp(kfls_ecp_length)
      Dimension kfirst(necp_atoms,max_necp_functns)
      Dimension klast_tmp(kfls_ecp_length)
      Dimension klast(necp_atoms,max_necp_functns)
      Dimension llmax(ncenters)

      integer ipseud
      integer nelecp(ncenters)
      integer ipseux(ncenters)
      integer nlp(max_ecp_prims)
      integer igenby(ncenters)
      integer atom(nshells)

      dimension clp(max_ecp_prims)
      dimension zlp(max_ecp_prims)
      Dimension iccoff(ncenters,max_nshells,max_nsubshells)
      Dimension newoff(ncenters,max_nshells,max_nsubshells)

      Dimension Cint(max_unfns_shell*max_unfns_shell)
      Dimension ecpint_4shell(max_cofns_shell*max_cofns_shell)
      Dimension ecpint(nval_bfns*(nval_bfns+1)/2)
      Dimension tmp1(max_cofns_shell*max_cofns_shell)
      Dimension tmp2(max_cofns_shell*max_cofns_shell)
      Dimension tmp3(max_cofns_shell*max_cofns_shell)

      Dimension charges(ncenters) 
      Dimension coords(3,ncenters)
      Dimension tcoords(3,nshells)
    
      Dimension numcon(nshells),numpri(nshells)
      Dimension katom(nshells),ktype(nshells)
      Dimension kmini(nshells),kmaxi(nshells)
      Dimension ndegen(nshells)
      Dimension nprims(nshells),kprim(nshells)
      Dimension expnt(max_nprims)
      Dimension contr(max_nprims,max_conts_shell)

      Dimension imin(maxang),imax(maxang),nfcts(maxang)

      Logical spherical
C 
      Call A4_ecp_init()

      Call A4_ecp_basis_init(nshells,ncenters,max_nshells,
     &                       max_nsubshells,
     &                       max_nprims,nval_bfns,max_ncontfns,
     &                       nval_prims,nval_ccoeffs,
     &                       kfls_ecp_length,necp_atoms,
     &                       max_necp_functns,
     &                       max_prims_shell,
     &                       max_conts_shell,
     &
     &                       iqmstr,jcostr,nucstr,nrcstr,Jstrt,jrs,
     &                       palphas,ppcoeffs,
     &
     &                       llmax,
     &
     &                       kfirst_tmp,klast_tmp,
     &
     &                       numcon,numpri,katom,ktype,kmini,kmaxi,
     &                       ndegen,nprims,kprim,
     &
     &                       expnt,contr,
     &
     &                       kfirst,klast,
     &
     &                       scr1,scr2,scr3,scr4,
     &
     &                       spherical)
C
C-----------------------------------------------------------------------
C 
C  Since  ACES IV do not use symmetry Igenby is a trivial array.

      Do i = 1, ncenters
          Igenby(i) = i
      Enddo 

      call A4_ecp_int_driver(Ecpint,Ecpint_4shell,Cint,
     &                       tmp1,tmp2,tmp3,
     &
     &                       nshells,max_ecp_prims,
     &                       necp_atoms,max_necp_functns,
     &                       nval_prims,nval_ccoeffs,
     &                       max_prims_shell,max_conts_shell,
     &                       max_unfns_shell,max_cofns_shell,
     &                       nval_bfns,max_nprims,max_ncontfns,
     &                       ncenters,
     &
     &                       clp,zlp,nlp,kfirst,klast,llmax,
     &                       nelecp,ipseux,ipseud,igenby,
     &
     &                       numcon,katom,ktype,kprim,kmini,kmaxi,
     &                       nprims,ndegen,expnt,expnt_norms,
     &                       contr,
     & 
     &                       coords, .FALSE., spherical)
#ifdef _DEBUG_LVL0
      write(6,*)
      write(6,"(a)") " Returned from A4_ecp_int_driver"   
#endif 

      Return
      End
