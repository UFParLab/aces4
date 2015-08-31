      subroutine A4_Ecp_basis_init(nshells,ncenters,max_nshells,
     &                             max_nsubshells,max_nprims,nval_bfns,
     &                             max_ncontfns,nval_prims,
     &                             nval_ccoeffs,
     &                             kfls_ecp_length,necp_atoms,
     &                             max_necp_functns,
     &                             max_prims_shell,
     &                             max_conts_shell,
     &
     &                             iqmstr,jcostr,nucstr,nrcstr,
     &                             Jstrt,jrs,palphas,ppcoeffs,
     &
     &                             llmax,
     &
     &                             kfirst_tmp,klast_tmp,
     &
     &                             numcon,numpri,katom,ktype,kmini,
     &                             kmaxi,ndegen,nprims,kprim,
     & 
     &                             expnt,contr,
     &
     &                             kfirst,klast,
     & 
     &                             imarked,ipeoff,iccoff,newoff,
     &                          
     &                             spherical)

      Implicit double precision (A-H, O-Z)
     
#include "ecp.par"
#include "ecp.com"

      Dimension iqmstr(ncenters),jcostr(ncenters,nshells)
      Dimension nucstr(nshells)
      Dimension nrcstr(nshells)
      Dimension Jstrt(nshells),Jrs(nshells)
      Dimension palphas(nval_prims),ppcoeffs(nval_ccoeffs)
      Dimension imarked(ncenters)
      Dimension kfirst_tmp(kfls_ecp_length)
      Dimension kfirst(necp_atoms,max_necp_functns)
      Dimension klast_tmp(kfls_ecp_length)
      Dimension klast(necp_atoms,max_necp_functns)
      Dimension llmax(ncenters)

      Dimension ipeoff(ncenters,max_nshells,max_nsubshells)
      Dimension iccoff(ncenters,max_nshells,max_nsubshells)
      Dimension newoff(ncenters,max_nshells,max_nsubshells)

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
c Create offsets of primitive exponents and contraction coefficients
C The NMax refers 
#ifdef _DEBUG_LVL0
      Write(6,*) 
      write(6,"(a)") " Printing from A4_Ecp_basis_init BEGIN"
      write(6,"(a)") "--------------------------------------"
#endif 

      icount=0
      do iat=1, ncenters
         if (llmax(iat) .ne. 0) then
            do ilq=1, llmax(iat)+1
               icount = icount + 1
               kfirst(ilq,iat) = kfirst_tmp(icount)
               klast(ilq,iat)  = klast_tmp(icount)
            enddo 
          endif 
      enddo

#ifdef _DEBUG_LVL0
      write(6,*)
      write(6,"(a)") " The kifist and klast arrays"
      write(6,"(6I14)") ((kfirst(iat,ilq),iat=1,necp_atoms),
     *                                    ilq=1,max_necp_functns)
      write(6,*)
      write(6,"(6I14)") ((klast(iat,ilq),iat=1,necp_atoms),
     *                                   ilq=1,max_necp_functns)
#endif

      icount=0
      do  iat=1,Ncenters 
        do  ilq=1,iqmstr(iat)
          do  ijco=1,jcostr(iat,ilq)

              icount=icount+1
C
              if (imarked(iat).ne.1) then

                 ipeoff(iat,ilq,ijco)=jstrt(icount)
                 iccoff(iat,ilq,ijco)=jrs(icount)

              endif

          enddo
        enddo 
C
        imarked(iat)=1 
C
      enddo
#ifdef _DEBUG_LVL0
      write(*,*)
      Write(*,"(a)") " The ipeoff and iccoff arrays"
      do iat = 1, ncenters 
      do ilq = 1, iqmstr(iat)
         write(6,"(10I4)") (ipeoff(iat,ilq,ijco),
     &                       ijco=1,jcostr(iat,ilq))
         write(6,"(10I4)") (iccoff(iat,ilq,ijco),
     &                      ijco=1,jcostr(iat,ilq))
      enddo
      enddo
#endif 

C create shell-structure and arrays expnt and contr

      icount=0
      kcount=0
      ncount=0
      lcount=0

      do iat=1,Ncenters 
        do ilq=1,iqmstr(iat)
          do  ijco=1,jcostr(iat,ilq)
C
            ncount=ncount+1
            icount=icount+1

            do inuco=1,nucstr(ncount)

               lcount=lcount+1

              do inrco=1,nrcstr(icount)

                 contr(lcount,inrco)=Ppcoeffs(iccoff(iat,ilq,ijco)
     &                               + inuco +
     &                               nucstr(ncount)*(inrco-1))

#ifdef _DEBUG_LVL0
      write(*,3) lcount,inrco,contr(lcount,inrco)
   3  format('lcount: ',i3,' inrco: ',i3,' contr: ',f12.8)
#endif
              enddo

              expnt(lcount)=Palphas(ipeoff(iat,ilq,ijco)+inuco)

#ifdef _DEBUG_LVL0
      write(*,4) lcount,expnt(lcount)
   4  format('lcount: ',i3,' expnt: ',f12.8)
#endif
              kcount = kcount+1

            enddo
            numpri(icount) = kcount

            kcount=0
            numcon(icount) = nrcstr(icount)

#ifdef _DEBUG_LVL0
      write(*,5) iat,ilq,ijco,numpri(icount)
   5  format('iat: ',i3,' ilq: ',i3,' ijco: ',i3,' numpri: ',i3)
      write(*,6) ncount,numcon(ncount)
   6  format('ncount: ',i3,' numcon: ',i3)
#endif
          enddo

        enddo

      enddo

#ifdef _DEBUG_LVL0
      Write(*,*)
      write(*,"(a)") "The reordered/organized exponents"
      Write(6,"(4F15.8)") (expnt(i), i=1,max_nprims) 
      write(*,"(a)") "The reordered/organized contraction coefs."
      Write(6,"(4F15.8)") ((contr(i,j),i=1,max_nprims), 
     &                                j=1,max_conts_shell)
     
#endif 

      khelp  = 1
      icount = 0

      do iat=1, ncenters
        do  ilq=1,iqmstr(iat)
          do ijco=1,jcostr(iat,ilq)

            newoff(iat,ilq,ijco)=khelp
 
#ifdef _DEBUG_LVL0
      write(*,99) iat,ilq,ijco,newoff(iat,ilq,ijco)
  99  format('iat: ',i3,' ilq: ',i3,' ijco: ',i3,' newoff: ',i3)
#endif
            icount = icount + 1
            khelp=khelp+numpri(icount)

          enddo
        enddo
      enddo

      ltmax=1
      do  iat=1, ncenters 
        if (ltmax.le.iqmstr(iat)) then 
            ltmax=iqmstr(iat)
        endif  
      enddo 

      nftdrv=ltmax*(ltmax+1)*(ltmax+2)/6

      do l=1,Maxang
         imin(l)  = (l-1)*l*(l+1)/6+1
         imax(l)  = l*(l+1)*(l+2)/6 

         k = l - 1 
         if (spherical) then
             nfcts(l) = (2*k + 1)
         else
             nfcts(l) = ((k+1)*(k+2))/2
         endif 
      enddo

      nbf=0
      nshll=0
      indsh=0
C
c loop over atoms

      do iat=1,ncenters 

c loop over l-quantum-numbers

        do ilq=1,iqmstr(iat)

c loop over contracted basisfunctions

          do ijco=1,jcostr(iat,ilq)

             indsh=indsh+1
             nshll=nshll+1
             katom(nshll)=iat
             ktype(nshll)=ilq
             kmini(nshll)=imin(ilq)
             kmaxi(nshll)=imax(ilq)
             ndegen(nshll)=nfcts(ilq)             
             nprims(nshll)=numpri(indsh)
             kprim(nshll)=newoff(iat,ilq,ijco)
             nbf=nbf+ndegen(nshll)*numcon(nshll)
 
          Enddo
        Enddo
      Enddo


      If (nshells .NE. Nshll) Then
         Write(6,"(a,a,a)") "The basis functions with the same angular"
     &                      " entered in GENBAS file as more than one"
     &                      " shell. It us not allowed in ACES IV"
         call abort_job()
      Endif 

      If (nbf .NE. nval_bfns) Then
         
         Write(6,"(a,a,a)") "Inconsistency is detected in the number"
     &                      " of basis functions: This can not happen!"
         call abort_job()
      Endif  

#ifdef _DEBUG_LVL0
      Write(6,"(a)") " The katom array"
      Write(6,"(6I)") (katom(i), i=1, Nshells)
      Write(6,"(a)") " The ktype array"
      Write(6,"(6I)") (ktype(i), i=1, Nshells)
      Write(6,"(a)") " The kmini array"
      Write(6,"(6I)") (kmini(i), i=1, Nshells)
      Write(6,"(a)") " The kmaxi array"
      Write(6,"(6I)") (kmaxi(i), i=1, Nshells)
      Write(6,"(a)") " The ndegen array"
      Write(6,"(6I)") (ndegen(i), i=1, Nshells)
      Write(6,"(a)") " The nprims array"
      Write(6,"(6I)") (nprims(i), i=1, Nshells)
      Write(6,"(a)") " The kprims array"
      Write(6,"(6I)") (kprim(i), i=1, Nshells)
#endif

      return
      end 
