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
      subroutine compute_fragscf(current_frag, scr,
     *                 maxblk, iscr, coords,coeffs,alphas, ccbeg, ccend,
     *                 nc1,nc2, nd1, nd2,
     *                 nai, kin, ovl, fa, fb, 
     *                 ca_perm, cb_perm, fa_perm, fb_perm, 
c
     *                 vmax,omax,norb,maxhist,diis_start)  
c---------------------------------------------------------------------------

      implicit none

      include 'mpif.h'
      include 'int_gen_parms.h'
      include 'machine_types.h'
      include 'frag_def.h'

      integer aa1,aa2,bb1,bb2,cc1,cc2,dd1,dd2
      integer adim, bdim, cdim, ddim  
      integer m1, m2, n1, n2, r1, r2, s1, s2
      integer i, j, n, m, r, s
      integer a,b,c,d
      integer iatom(ncenters), n_basis, n_apf, nf   
      double precision current_frag 
      integer icurrent_frag 

      integer num_to_do, nsend
      integer nints, maxblk
      integer nalpha_pack, npcoeff_pack
      integer ncsum, next, nfirst
      integer me, ierr
      integer nc1, nc2, nd1, nd2 

      integer imin, zmin, iblk, zblk

      logical skip
      logical mn_symmetry
      logical rs_symmetry
      logical mn_rs_symmetry
      logical*8 l8true, l8spherical
      logical spherical

      double precision x1,y1,z1
      double precision x2,y2,z2
      double precision x3,y3,z3
      double precision x4,y4,z4

      double precision coords(3,*), coeffs(*), alphas(*)
      double precision nai(nc1:nc2,nd1:nd2)
      double precision kin(nc1:nc2,nd1:nd2)
      double precision ovl(nc1:nc2,nd1:nd2)
      double precision H0T(nc1:nc2,nd1:nd2)

      double precision ca(nc1:nc2,nd1:nd2)
      double precision cb(nc1:nc2,nd1:nd2)

      double precision fa(nc1:nc2,nd1:nd2)
      double precision fb(nc1:nc2,nd1:nd2)

      double precision ca_perm(nc1:nc2,nd1:nd2)
      double precision cb_perm(nc1:nc2,nd1:nd2)
      double precision fa_perm(nc1:nc2,nd1:nd2)
      double precision fb_perm(nc1:nc2,nd1:nd2)

      integer map(nc1:nc2) 
      integer umap(nc1:nc2) 
      integer beg_anfps(max_shells)  
      integer end_anfps(max_shells)  
      double precision scr(*)   
      integer iscr(*)

      integer ccbeg(*), ccend(*)

      integer max_dim_coeff
      parameter (max_dim_coeff = 25000)
      integer ccbeg_pack(max_dim_coeff), ccend_pack(max_dim_coeff)
      integer*8 ccbeg_pack64(max_dim_coeff), ccend_pack64(max_dim_coeff)
      double precision alpha_pack(max_dim_coeff), 
     *                 pcoeff_pack(max_dim_coeff)
      integer*8 arg64(25)
      integer n_max 
c
c ---------------------------------------------------------------------- 
c Arrays used in the DIIS procedure 
c ---------------------------------------------------------------------- 

      integer vmax,omax,norb,maxhist,diis_start  
      double precision Fai(vmax,omax,maxhist+1) 
      double precision Fbj(vmax,omax,maxhist+1) 
      double precision Ea(norb,norb,maxhist+1) 
      double precision Eb(norb,norb,maxhist+1) 
      double precision BM(maxhist+1,maxhist+1) 
      double precision TBM(maxhist+1,maxhist+1) 
      double precision CM(maxhist+1) 
c
c ---------------------------------------------------------------------- 

      open(66,file='summary.out') 

c     call mpi_comm_rank(mpi_comm_world, me, ierr)

      l8true = .true.
      spherical = (ispherical .eq. 1)
      l8spherical = spherical

      icurrent_frag = current_frag 
      n_apf = natoms_frag(icurrent_frag) 
      do m = 1, n_apf 
         iatom(m) = watom_frag(icurrent_frag, m) 
      enddo 
c     iatom = wfrag 
      write(6,*) ' Entering H0 on frag:', icurrent_frag, ':', 
     *            n_apf, (iatom(m), m=1, n_apf)  

      call comp_return_frag_h0(H0T, n_apf, iatom, nc1, nc2, nd1, nd2) 

c-----------------------------------------------------------------------
c   Find the shell blocks for which we shall loop through.
c-----------------------------------------------------------------------

         m1 = 1 
         n1 = 1 
         r1 = 1 
         s1 = 1 

         m2 = (nshells)   
         n2 = (nshells)  
         r2 = (nshells) 
         s2 = (nshells)  

c-----------------------------------------------------------------------
c   Find the number of basis functions and shells in the atom.  
c-----------------------------------------------------------------------

         n_basis = 0 
         do m = m1, m2 
         do n = 1, n_apf 
            if(atom(m) .eq. iatom(n)) n_basis = n_basis + 
     *                end_nfps(m) - end_nfps(m-1)   
         enddo 
         enddo 
c     do m = 1, n_basis 
c     do n = 1, n_basis 
c        write(6,*) ' VFL:', m,n, h0t(m,n) 
c     enddo 
c     enddo 
c         write(6,*) ' The number of basis functions on atom', 
c     *                iatom, '=', n_basis  

c-----------------------------------------------------------------------
c   Find the mapping from atom <--> molecule.  
c-----------------------------------------------------------------------

         do n = nc1, nc2 
            map(n) = 0 
            umap(n) = 0 
         enddo 

         n_basis = 0 
         n_max = 0 
         do m = m1, m2 
            beg_anfps(m) = 0   
            end_anfps(m) = 0   

          do nf = 1, n_apf 
            if(atom(m) .eq. iatom(nf)) then 
               beg_anfps(m) = n_basis + 1  

               if (m .eq. 1) then
                  DO n = 1, end_nfps(m) 
                     n_basis = n_basis + 1 
                     map(n_basis) = n 
                     umap(n) = n_basis  
                     if (n .gt. n_max) n_max = n 
                  enddo 
               else 
                  DO n = end_nfps(m-1) + 1, end_nfps(m) 
                     n_basis = n_basis + 1 
                     map(n_basis) = n 
                     umap(n) = n_basis  
                     if (n .gt. n_max) n_max = n 
                  enddo 
               endif 

               end_anfps(m) = n_basis 

            endif 
          enddo 

         enddo 

         if ((n_max .lt. nc1) .or. (n_max .gt. nc2)) then 
           write(66,*) ' UMapping of atom dens wrong:' 
           write(66,*) ' Correct range :', nc1, nc2, 'Computed range:', 
     *                 '1', n_max  
         endif 

         if ((n_basis .lt. nc1) .or. (n_basis .gt. nc2)) then 
           write(66,*) ' Mapping of atom dens wrong:' 
           write(66,*) ' Correct range :', nc1, nc2, 'Computed range:', 
     *                 '1', nbasis   
         endif 

c        write(66,*) ' Mapping Atom', watom  
c        do n = nc1, nc2 
c           write(66,*) 'n umap(n)', n, umap(n)  
c        enddo 
c        do m = m1, m2 
c           if(atom(m) .eq. iatom) then 
c           write(66,*) ' Mth shell:', m, beg_anfps(m), end_anfps(m) 
c           endif 
c        enddo 
c         write(6,*) ' The number of basis functions on atom', 
c     *                iatom, '=', n_basis  

         do j = nd1, nd2 
         do i = nc1, nc2 
            ca_perm(i,j) = 0.0 
            cb_perm(i,j) = 0.0 
            fa_perm(i,j) = 0.0 
            fb_perm(i,j) = 0.0 
         enddo 
         enddo 

         call do_fragscf(icurrent_frag, scr,
     *                 maxblk, iscr, coords,coeffs,alphas, ccbeg, ccend,
     *                 nc1,nc2, nd1, nd2,
     *                 H0T, nai, kin, ovl,  
     *                 n_basis, beg_anfps, end_anfps, map, umap, 
c
     *                 ca_perm, cb_perm, fa_perm, fb_perm, 
c 
     *                 vmax,omax,norb,maxhist,diis_start) 

      return 
      end 

      subroutine do_fragscf(current_frag, scr,
     *                 maxblk, iscr, coords,coeffs,alphas, ccbeg, ccend,
     *                 nc1,nc2, nd1, nd2,
     *                 H0T, nai, kin, ovl,  
     *                 n_basis, beg_anfps, end_anfps, map, umap, 
c
     *                 ca_perm, cb_perm, fa_perm, fb_perm, 
c
     *                 vmax,omax,norb,maxhist,diis_start) 
c---------------------------------------------------------------------------

      implicit none

      include 'mpif.h'
      include 'int_gen_parms.h'
      include 'machine_types.h'
      include 'frag_def.h'

      integer a1, a2, b1, b2, c1, c2, d1, d2 
      integer aa1,aa2,bb1,bb2,cc1,cc2,dd1,dd2
      integer adim, bdim, cdim, ddim  
      integer m1, m2, n1, n2, r1, r2, s1, s2
      integer i, j, n, m, r, s, l, mn, rs  
      integer a,b,c,d
      integer iatom(ncenters), n_basis, n_basis_org, n_apf, nf   
      integer current_frag 

      integer num_to_do, nsend
      integer nints, maxblk
      integer nalpha_pack, npcoeff_pack
      integer ncsum, next, nfirst
      integer me, ierr
      integer nc1, nc2, nd1, nd2 

      integer imin, zmin, iblk, zblk

      logical dom, don, dor, dos 
      logical skip
      logical mn_symmetry
      logical rs_symmetry
      logical mn_rs_symmetry
      logical*8 l8true, l8spherical
      logical spherical

      double precision x1,y1,z1
      double precision x2,y2,z2
      double precision x3,y3,z3
      double precision x4,y4,z4

      double precision coords(3,*), coeffs(*), alphas(*)
      double precision nai(nc1:nc2,nd1:nd2)
      double precision kin(nc1:nc2,nd1:nd2)
      double precision ovl(nc1:nc2,nd1:nd2)
      double precision H0T(nc1:nc2,nd1:nd2)

      double precision ca_perm(nc1:nc2,nd1:nd2)
      double precision cb_perm(nc1:nc2,nd1:nd2)
      double precision fa_perm(nc1:nc2,nd1:nd2)
      double precision fb_perm(nc1:nc2,nd1:nd2)

      double precision h0(n_basis,n_basis) 
      double precision aovl(n_basis,n_basis) 
      double precision sos(n_basis,n_basis) 
      double precision Qxx(n_basis,n_basis) 

      double precision HFD_A(n_basis,n_basis) 
      double precision HFD_B(n_basis,n_basis) 
      double precision HFDOLD_A(n_basis,n_basis) 
      double precision HFDOLD_B(n_basis,n_basis) 

      double precision ca(n_basis,n_basis) 
      double precision cb(n_basis,n_basis) 
      double precision cba(n_basis,n_basis) 
      double precision cbb(n_basis,n_basis) 
      double precision FTa(n_basis,n_basis) 
      double precision FTb(n_basis,n_basis) 
      double precision Fa(n_basis,n_basis) 
      double precision Fb(n_basis,n_basis) 
      double precision temp, tempa, tempb   
      integer doit, itemp, jtemp, p, p1  

      integer tcharge, ma_offset, na_offset 
      integer mb_offset, nb_offset, nstart  
      integer nocc_a, nocc_b, nvirt_a, nvirt_b  
      integer nocc_a_org, nocc_b_org 
      integer iter, max_iter 
c
c ---------------------------------------------------------------------- 
c Arrays used in the DIIS procedure 
c ---------------------------------------------------------------------- 

      integer vmax,omax,norb,nhist,maxhist,diis_start   
      double precision Fai(vmax,omax,maxhist+1) 
      double precision Fbj(vmax,omax,maxhist+1) 
      double precision Ea(norb,norb,maxhist+1) 
      double precision Eb(norb,norb,maxhist+1) 
      double precision BM(maxhist+1,maxhist+1) 
      double precision TBM(maxhist+1,maxhist+1) 
      double precision CM(maxhist+1) 
c ---------------------------------------------------------------------- 

      integer map(nc1:nc2) 
      integer umap(nc1:nc2) 
      integer beg_anfps(max_shells)  
      integer end_anfps(max_shells)  
      double precision scr(*)   
      integer iscr(*)

      integer ccbeg(*), ccend(*)

      integer max_dim_coeff
      parameter (max_dim_coeff = 25000)
      integer ccbeg_pack(max_dim_coeff), ccend_pack(max_dim_coeff)
      integer*8 ccbeg_pack64(max_dim_coeff), ccend_pack64(max_dim_coeff)
      double precision alpha_pack(max_dim_coeff), 
     *                 pcoeff_pack(max_dim_coeff)
      integer*8 arg64(25)

c     call mpi_comm_rank(mpi_comm_world, me, ierr)

      l8true = .true.
      spherical = (ispherical .eq. 1)
      l8spherical = spherical

      n_apf = natoms_frag(current_frag) 
      do m = 1, n_apf 
         iatom(m) = watom_frag(current_frag, m) 
      enddo 

      nocc_a = 0 
      nocc_b = 0 
      tcharge = 0 
      do m = 1, max_centers 
       do n = 1, n_apf 
         if (m .eq. iatom(n)) then 
c           nocc_b = nocc_b + charge(m)/2 
c           nocc_a = nocc_a + charge(m) - nocc_b 
            tcharge = tcharge + charge(m)  
         endif 
       enddo 
      enddo 
      nocc_a = tcharge/2 
      nocc_b = tcharge - nocc_a 

      nvirt_a = n_basis - nocc_a  
      nvirt_b = n_basis - nocc_b  

c TEST !!!! 

      write(6,*) ' Performing an SCF calculation on frag:', 
     *  current_frag, 
     * 'in a basis of', n_basis, 'functions with', nocc_a, nocc_b, 
     * 'alpha and beta occupied electrons and ', nvirt_a, nvirt_b, 
     * ' alpha and beta virtual electrons'    
      if (nocc_a .eq. 0) return  

c-----------------------------------------------------------------------
c   Find the shell blocks for which we shall loop through.
c-----------------------------------------------------------------------

      m1 = 1 
      n1 = 1 
      r1 = 1 
      s1 = 1 

      m2 = (nshells)   
      n2 = (nshells)  
      r2 = (nshells) 
      s2 = (nshells)  

c-----------------------------------------------------------------------
c Sum nai and kin into small array and copy ovl there too. 
c --> initial guess   
c-----------------------------------------------------------------------

      do n = 1, n_basis  
      do m = 1, n_basis  
         h0(m,n) = 0.0  
      enddo  
      enddo  

      itemp = 0 
      do n = nc1, nc2 
         if (umap(n).ne.0) then 
            itemp = itemp + 1 
            jtemp = 0 
      do m = nc1, nc2 
         if (umap(m).ne.0) then 
            aovl(umap(m),umap(n)) = ovl(m,n) 
            h0(umap(m),umap(n))   = H0T(m,n)   
            jtemp = jtemp + 1 
         endif 
      enddo  
         if (jtemp .ne. n_basis) then 
            write(6,*) ' Jtemp .ne. n_basis ', jtemp, n_basis 
            call abort_job() 
         endif 
         endif 
      enddo  
      if (itemp .ne. n_basis) then 
         write(6,*) ' itemp .ne. n_basis ', itemp, n_basis 
         call abort_job() 
      endif 

      if (itemp*jtemp .ne. n_basis**2) then 
         write(6,*) ' Something possibly wrong with umap ', 
     *                itemp, n_basis 
         call abort_job()
      endif 

c-----------------------------------------------------------------------
c Construct the hcore initial guess  
c-----------------------------------------------------------------------

      do n = 1, n_basis  
      do m = 1, n_basis  
         FA(m,n)    = 0.0  
         FB(m,n)    = 0.0  
         FTA(m,n)   = 0.0  
         FTB(m,n)   = 0.0  
         HFD_a(m,n) = 0.0 
         HFD_b(m,n) = 0.0 
         ca(m,n)    = 0.0 
         cb(m,n)    = 0.0 
      enddo  
      enddo  

      do n = 1, n_basis  
      do m = 1, n_basis  
         FA(m,n) = h0(m,n) 
         FB(m,n) = h0(m,n) 
      enddo  
      enddo  

c-----------------------------------------------------------------------
c Construct U*S**(-1/2)  
c-----------------------------------------------------------------------

      call diag(aovl,sos,m,n_basis,0,1,1) 
      do m = 1, n_basis  
      do n = 1, n_basis  
         temp = 0.0 
         do l = 1, n_basis  
            temp = temp + sos(m,l)*aovl(l,n) 
         enddo 
         Qxx(m,n) = temp 
       enddo  
       enddo  

c-----------------------------------------------------------------------
c Transpose the Fock matrix -> Construct S^(-1/2) F S^(-1/2)  
c-----------------------------------------------------------------------

       call fock_transpose(FA,FB,Qxx,FTa,FTb,n_basis) 

c-----------------------------------------------------------------------
c Diagonalize the transposed Fock matrix  
c-----------------------------------------------------------------------

       call diag(FTa,ca,m,n_basis,0,0,0) 
       call diag(FTb,cb,m,n_basis,0,0,0) 

       if (nocc_a .eq. nocc_b) then 
          do n = 1, n_basis
          do m = 1, n_basis
                 cb(m,n) = ca(m,n)  
          enddo 
          enddo 
       endif 

c-----------------------------------------------------------------------
c Back transform the coefficient array  
c-----------------------------------------------------------------------

       call c_backtran(Qxx,ca,cb,cba,cbb,n_basis) 

c-----------------------------------------------------------------------
c Compute the HF density  
c-----------------------------------------------------------------------

       call hfdensity(ca,cb,HFD_A,HFD_B,n_basis,nocc_a,nocc_b) 

c-----------------------------------------------------------------------
c Compute the HF energy   
c-----------------------------------------------------------------------

       call hfenergy(HFD_A,HFD_B,FA,FB,h0,n_basis) 

c-----------------------------------------------------------------------
c Copy the HF Density into the old Density. 
c-----------------------------------------------------------------------

      call hfdensity_copy(HFD_A,HFD_B,HFDOLD_A,HFDOLD_B,n_basis)  

c     if (nocc_a .ne. nocc_b) go to 100 

c     if (nocc_a .eq. 1) go to 100 

c-----------------------------------------------------------------------
c Start the SCF iterations  
c-----------------------------------------------------------------------

      max_iter = 100  
      DO iter = 1, max_iter 

c-----------------------------------------------------------------------
c       Construct the new Fock matrix  
c-----------------------------------------------------------------------

c-----------------------------------------------------------------------
c       One-electron piece  
c-----------------------------------------------------------------------

        do m = 1, n_basis 
        do n = 1, n_basis 
           FA(m,n) = h0(m,n)  
           FB(m,n) = h0(m,n)  
        enddo 
        enddo 

c-----------------------------------------------------------------------
c       Two-electron piece  
c-----------------------------------------------------------------------

         do m = m1, m2
            dom = .false. 
            do nf = 1, n_apf 
               if(atom(m) .eq. iatom(nf)) dom = .true.   
            enddo    
            if(dom) then  
            aa1 = beg_anfps(m)
            aa2 = end_anfps(m)

            x1 = coords(1,m)
            y1 = coords(2,m)
            z1 = coords(3,m)
         do n = n1, n2
            if (m .le. n) then 
            don = .false. 
            do nf = 1, n_apf 
               if(atom(n) .eq. iatom(nf)) don = .true.   
            enddo    
            if(don) then  
            bb1 = beg_anfps(n)
            bb2 = end_anfps(n)

            x2 = coords(1,n)
            y2 = coords(2,n)
            z2 = coords(3,n)
         do r = r1, r2
            dor = .false. 
            do nf = 1, n_apf 
               if(atom(r) .eq. iatom(nf)) dor = .true.   
            enddo    
            if(dor) then  
            cc1 = beg_anfps(r)
            cc2 = end_anfps(r)

            x3 = coords(1,r)
            y3 = coords(2,r)
            z3 = coords(3,r)
         do s = s1, s2
            if (r .le. s) then 
            dos = .false. 
            do nf = 1, n_apf 
               if(atom(s) .eq. iatom(nf)) dos = .true.   
            enddo    
            if(dos) then  
            dd1 = beg_anfps(s)
            dd2 = end_anfps(s)
c
c-----------------------------------------------------------------------
c   Determine the largest density element.
c-----------------------------------------------------------------------

               x4 = coords(1,s)
               y4 = coords(2,s)
               z4 = coords(3,s)
               call pack_coeffs(alphas, ixalpha, coeffs, ixpcoef, 
     *                          ncfps, npfps, m, n, 
     *                          r, s, alpha_pack, nalpha_pack, 
     *                          pcoeff_pack, npcoeff_pack, 
     *                          ccbeg, ccend, indx_cc,
     *                          ccbeg_pack, ccend_pack) 

c---------------------------------------------------------------------------
c   Calling sequence for ERD version 2.
c---------------------------------------------------------------------------

               ncsum = ncfps(m) + ncfps(n) + ncfps(r) + ncfps(s)

c              if (doit .eq. 1) then 

               call ERD__GENER_ERI_BATCH(intmax, zmax,
     *                nalpha_pack, npcoeff_pack, ncsum, 
     *                ncfps(m),ncfps(n), ncfps(r), ncfps(s),
     *                npfps(m),npfps(n), npfps(r), npfps(s),
     *                ivangmom(m), ivangmom(n), 
     *                ivangmom(r), ivangmom(s), x1,y1,z1,
     *                x2,y2,z2,x3,y3,z3,x4,y4,z4, alpha_pack,
     *                pcoeff_pack, ccbeg_pack, ccend_pack,
     *                spherical, .true., iscr, nints, 
     *                nfirst, scr)    

c               endif 

c---------------------------------------------------------------------------
c   Move the integrals into the output block.  
c---------------------------------------------------------------------------

           if (nints .gt. 0) then

               call form_ss1fock(scr(nfirst), n_basis,
     *                        aa1,aa2,bb1,bb2,cc1,cc2,dd1,dd2,
     *                        HFD_A,HFD_B,FA,FB)
               call form_ss2fock(scr(nfirst), n_basis,
     *                        aa1,aa2,bb1,bb2,cc1,cc2,dd1,dd2,
     *                        HFD_A,HFD_B,FA,FB)

               call form_ss3fock(scr(nfirst), n_basis,
     *                        aa1,aa2,bb1,bb2,cc1,cc2,dd1,dd2,
     *                        HFD_A,HFD_B,FA,FB)

               call form_ss4fock(scr(nfirst), n_basis,
     *                        aa1,aa2,bb1,bb2,cc1,cc2,dd1,dd2,
     *                        HFD_A,HFD_B,FA,FB)

               call form_ss5fock(scr(nfirst), n_basis,
     *                        aa1,aa2,bb1,bb2,cc1,cc2,dd1,dd2,
     *                        HFD_A,HFD_B,FA,FB)

               call form_ss6fock(scr(nfirst), n_basis,
     *                        aa1,aa2,bb1,bb2,cc1,cc2,dd1,dd2,
     *                        HFD_A,HFD_B,FA,FB)

               call form_ss7fock(scr(nfirst), n_basis,
     *                        aa1,aa2,bb1,bb2,cc1,cc2,dd1,dd2,
     *                        HFD_A,HFD_B,FA,FB)

               call form_ss8fock(scr(nfirst), n_basis,
     *                        aa1,aa2,bb1,bb2,cc1,cc2,dd1,dd2,
     *                        HFD_A,HFD_B,FA,FB)

           endif
c 
            endif 
            endif 
         enddo   ! s
            endif 
         enddo   ! r
            endif 
            endif 
         enddo   ! n
            endif 
         enddo   ! m

c-----------------------------------------------------------------------
c       Done computing the Fock matrix   
c-----------------------------------------------------------------------

c-----------------------------------------------------------------------
c       Compute the new HF energy  
c-----------------------------------------------------------------------

        call hfenergy(HFD_A,HFD_B,FA,FB,h0,n_basis) 

c-----------------------------------------------------------------------
c       Compute the DIIS coefficients  
c-----------------------------------------------------------------------

        call gen_hist(iter,maxhist,diis_start,n_basis,nvirt_a,
     *                nocc_a,nvirt_b,nocc_b,ca,cb,FA,FB,Fai,Fbj,Ea,Eb)

        if ((iter .lt. maxhist+diis_start-1) .and. 
     *      (iter .gt. diis_start)) then 
           nhist = iter - (diis_start-1)  
           call do_diis(nhist,nvirt_a,nocc_a,nvirt_b,nocc_b,n_basis,
     *                  Fai,Ea, Fbj,Eb, FA, FB) 
        endif 

        if (iter .ge. maxhist+diis_start-1) then 
           nhist = maxhist   
           call do_diis(nhist,nvirt_a,nocc_a,nvirt_b,nocc_b,n_basis,
     *                  Fai,Ea, Fbj,Eb, FA, FB) 
           call move_hist(nhist,diis_start,n_basis,Fa,Fb,Ea,Eb) 
        endif 

c-----------------------------------------------------------------------
c       Transpose the new Fock Matrix   
c-----------------------------------------------------------------------

        call fock_transpose(FA,FB,Qxx,FTa,FTb,n_basis) 

c-----------------------------------------------------------------------
c       Diagonalize the new Transposed Fock Matrix   
c-----------------------------------------------------------------------

        call diag(FTa,ca,m,n_basis,0,0,0) 
        call diag(FTb,cb,m,n_basis,0,0,0) 

        if (nocc_a .eq. nocc_b) then 
           do n = 1, n_basis
           do m = 1, n_basis
                  cb(m,n) = ca(m,n)  
           enddo 
           enddo 
        endif 

c-----------------------------------------------------------------------
c       Back Transform the coefficient array  
c-----------------------------------------------------------------------

        call c_backtran(Qxx,ca,cb,cba,cbb,n_basis) 

c-----------------------------------------------------------------------
c       Check on convergence and replace the old density with the new   
c-----------------------------------------------------------------------

c-----------------------------------------------------------------------
c       Compute the new HF density  
c-----------------------------------------------------------------------

        call hfdensity(ca,cb,HFD_A,HFD_B,n_basis,nocc_a,nocc_b) 

c-----------------------------------------------------------------------
c       Check for convergence    
c-----------------------------------------------------------------------

        call check_conv(HFD_A,HFD_B,HFDOLD_A,HFDOLD_B,n_basis,doit)  
        if (doit .eq. 1) go to 100 

c-----------------------------------------------------------------------
c Copy the HF Density into the old Density. 
c-----------------------------------------------------------------------

        call hfdensity_copy(HFD_A,HFD_B,HFDOLD_A,HFDOLD_B,n_basis)  


      ENDDO ! iter = 1, max_iter 
c If no convergence set contribution to density to zero???  
c     HFD_a(m,n) = 0.0  
c     HFD_b(m,n) = 0.0   
100   continue 

      if (current_frag .eq. 12) then 
       write(6,*) ' Alpha Orbital energies ' 
       write(6,*) ' -----------------------------------------------' 
       do m = 1, nocc_a 
          write(6,*) '  ', m, FTA(m,m), FTA(m,m)*27.21138386 
       enddo  
       write(6,*) ' -----------------------------------------------' 
       do m = nocc_a + 1, n_basis  
          write(6,*) '  ', m, FTA(m,m), FTA(m,m)*27.21138386  
       enddo  
       write(6,*) ' -----------------------------------------------' 
       write(6,*) ' ' 

       write(6,*) ' Beta Orbital energies ' 
       write(6,*) ' -----------------------------------------------' 
       do m = 1, nocc_b 
          write(6,*) '  ', m, FTB(m,m), FTB(m,m)*27.21138386 
       enddo  
       write(6,*) ' -----------------------------------------------' 
       do m = nocc_b + 1, n_basis  
          write(6,*) '  ', m, FTB(m,m), FTB(m,m)*27.21138386  
       enddo  
       write(6,*) ' -----------------------------------------------' 
       write(6,*) ' ' 

       endif 
c         write(6,*) ' Done SCF calculation of ATOM :', iatom 

c-----------------------------------------------------------------------
c     Put the transformation coefficients to the proper spot  
c-----------------------------------------------------------------------

c-----------------------------------------------------------------------
c        Alpha 
c-----------------------------------------------------------------------

      na_offset = baocc_frag(current_frag, 1) 
      do n = 1, nocc_a 
      do m = 1, n_basis
         if ((map(m) .ne. 0) .and. (map(n) .ne. 0)) then  
            ca_perm(map(m),na_offset+n-1) = Ca(m,n)  
         endif 
      enddo 
      enddo 

      na_offset = bavirt_frag(current_frag, 1) 
      do n = 1, n_basis - nocc_a  
      do m = 1, n_basis
         if ((map(m) .ne. 0) .and. (map(n) .ne. 0)) then  
            ca_perm(map(m),na_offset+n-1) = Ca(m,n+nocc_a)  
         endif 
      enddo 
      enddo 

c-----------------------------------------------------------------------
c        Beta 
c-----------------------------------------------------------------------

      nb_offset = bbocc_frag(current_frag, 1) 
      do n = 1, nocc_b 
      do m = 1, n_basis
         if ((map(m) .ne. 0) .and. (map(n) .ne. 0)) then  
            cb_perm(map(m),nb_offset+n-1) = Cb(m,n)  
         endif 
      enddo 
      enddo 

      nb_offset = bbvirt_frag(current_frag, 1) 
      do n = 1, n_basis - nocc_b  
      do m = 1, n_basis
         if ((map(m) .ne. 0) .and. (map(n) .ne. 0)) then  
            cb_perm(map(m),nb_offset+n-1) = Cb(m,n+nocc_b)  
         endif 
      enddo 
      enddo 


c-----------------------------------------------------------------------
c     Done transfering coefficients  
c-----------------------------------------------------------------------

c-----------------------------------------------------------------------
c     Put the Fock matrix to the proper spot  
c-----------------------------------------------------------------------

c-----------------------------------------------------------------------
c        ALPHA 
c-----------------------------------------------------------------------

      na_offset = baocc_frag(current_frag, 1) 
      ma_offset = baocc_frag(current_frag, 1) 
      do n = 1, nocc_a 
      do m = 1, nocc_a  
         fa_perm(ma_offset+m-1,na_offset+n-1) = 0.0  
         if ((map(m) .ne. 0) .and. (map(n) .ne. 0)) then  
            fa_perm(ma_offset+m-1,na_offset+n-1) = FTa(m,n)  
         endif 
      enddo 
      enddo 

      na_offset = baocc_frag(current_frag, 1) 
      ma_offset = bavirt_frag(current_frag, 1)  
      do n = 1, nocc_a 
      do m = 1, n_basis - nocc_a   
         fa_perm(ma_offset+m-1,na_offset+n-1) = 0.0  
         if ((map(m) .ne. 0) .and. (map(n) .ne. 0)) then  
            fa_perm(ma_offset+m-1,na_offset+n-1) = 
     *          FTa(m+nocc_a,n)  
         endif 
      enddo 
      enddo 

      ma_offset = baocc_frag(current_frag, 1) 
      na_offset = bavirt_frag(current_frag, 1)  
      do n = 1, n_basis - nocc_a    
      do m = 1, nocc_a 
         fa_perm(ma_offset+m-1,na_offset+n-1) = 0.0  
         if ((map(m) .ne. 0) .and. (map(n) .ne. 0)) then  
            fa_perm(ma_offset+m-1,na_offset+n-1) = 
     *          FTa(m,n+nocc_a)  
         endif 
      enddo 
      enddo 

      na_offset = bavirt_frag(current_frag, 1)  
      ma_offset = bavirt_frag(current_frag, 1)  
      do n = 1, n_basis - nocc_a    
      do m = 1, n_basis - nocc_a   
         fa_perm(ma_offset+m-1,na_offset+n-1) = 0.0  
         if ((map(m) .ne. 0) .and. (map(n) .ne. 0)) then  
            fa_perm(ma_offset+m-1,na_offset+n-1) =
     *          FTa(m+nocc_a,n+nocc_a)  
         endif 
      enddo 
      enddo 

c-----------------------------------------------------------------------
c        BETA 
c-----------------------------------------------------------------------
99    continue 

      nb_offset = bbocc_frag(current_frag, 1) 
      mb_offset = bbocc_frag(current_frag, 1) 
      do n = 1, nocc_b 
      do m = 1, nocc_b  
         fb_perm(mb_offset+m-1,nb_offset+n-1) = 0.0  
         if ((map(m) .ne. 0) .and. (map(n) .ne. 0)) then  
            fb_perm(mb_offset+m-1,nb_offset+n-1) = FTb(m,n)  
         endif 
      enddo 
      enddo 

      nb_offset = bbocc_frag(current_frag, 1) 
      mb_offset = bbvirt_frag(current_frag, 1)  
      do n = 1, nocc_b 
      do m = 1, n_basis - nocc_b    
         fb_perm(mb_offset+m-1,nb_offset+n-1) = 0.0  
         if ((map(m) .ne. 0) .and. (map(n) .ne. 0)) then  
            fb_perm(mb_offset+m-1,nb_offset+n-1) = 
     *          FTb(m+nocc_b,n)  
         endif 
      enddo 
      enddo 

      mb_offset = bbocc_frag(current_frag, 1) 
      nb_offset = bbvirt_frag(current_frag, 1)  
      do n = 1, n_basis - nocc_b   
      do m = 1, nocc_b 
         fb_perm(mb_offset+m-1,nb_offset+n-1) = 0.0  
         if ((map(m) .ne. 0) .and. (map(n) .ne. 0)) then  
            fb_perm(mb_offset+m-1,nb_offset+n-1) = 
     *          FTb(m,n+nocc_b)  
         endif 
      enddo 
      enddo 

      nb_offset = bbvirt_frag(current_frag, 1)  
      mb_offset = bbvirt_frag(current_frag, 1)  
      do n = 1, n_basis - nocc_b    
      do m = 1, n_basis - nocc_b   
         fb_perm(ma_offset+m-1,na_offset+n-1) = 0.0  
         if ((map(m) .ne. 0) .and. (map(n) .ne. 0)) then  
            fb_perm(mb_offset+m-1,nb_offset+n-1) =
     *          FTb(m+nocc_b,n+nocc_b)  
         endif 
      enddo 
      enddo 

      end
 

 

