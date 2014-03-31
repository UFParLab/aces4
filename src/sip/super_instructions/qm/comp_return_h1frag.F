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
      subroutine comp_return_h1frag(z, watom, da1, da2, db1, db2)
c---------------------------------------------------------------------------
c  Computes the set of OED integrals of types  
c
c   type = 1 --> NAI integrals
c          2 --> KIN integrals
c---------------------------------------------------------------------------
      implicit none

      include 'mpif.h'
      include 'int_gen_parms.h'
      include 'machine_types.h'
      include 'parallel_info.h'
      include 'frag_def.h'
#ifdef ALTIX
      include 'sheap.h'
#endif

      integer da1, da2, db1, db2
      integer adim, bdim 
      double precision z(da1:da2, db1:db2)
      double precision dbuf(1)
      integer ibuf(1)
#ifdef ALTIX
      pointer (iptr, ibuf)
      pointer (dptr, dbuf)
#else
      common dbuf
      equivalence (ibuf, dbuf)
#endif

      integer i, ix, m, n, nfirst, nints, nalpha, ncoeff, iatom, jatom,
     *        ncsum
      integer component
      integer a, b, a1,a2,b1,b2,lastao, igrad
      integer m1, m2, n1, n2
      integer der1x, der1y, der1z
      integer der2x, der2y, der2z
      integer dercx, dercy, dercz
      integer ixderc
      integer ind(2)
      integer ierr 
      integer newcenters, ncount, watom, last_atom   
      double precision acharge(ncenters)

      integer flopcount
      logical spherical
      logical*8 l8true, l8spherical
   
      integer max_dim_coeff
      parameter (max_dim_coeff = 25000)
      integer ccbeg_pack(max_dim_coeff), ccend_pack(max_dim_coeff)
      double precision alpha_pack(max_dim_coeff), 
     *                 pcoeff_pack(max_dim_coeff)
      double precision xn(max_centers), yn(max_centers), 
     *                 zn(max_centers)
      double precision x1,y1,z1,x2,y2,z2, y

      integer ECP, ntot
      double precision ecp_erd
      common /FECP/ntot, ecp_erd(2500,2500)

      common /flags/ iflags
      integer iflags(100)

#ifdef ALTIX
      iptr = ishptr
      dptr = dshptr
#endif
      spherical = (ispherical .eq. 1)
      l8spherical = spherical
      l8true = .false. ! .true.
      lastao = end_nfps(nshells)

c----------------------------------------------------------------------------
c   Clear the output array.
c----------------------------------------------------------------------------

      ECP = iflags(71) 

      do b = db1,db2
      do a = da1,da2
         Z(a,b) = 0.d0
      enddo
      enddo

      ind(1) = 1
      ind(2) = 2

      if (intpkg .ne. flocke_package) then
         print *,'Error: comp_fock_der requires ERD ',
     *           'integral package.'
         call abort_job()
      endif

c-------------------------------------------------------------------------
c   Find the shells we need to loop over for this block.
c--------------------------------------------------------------------------

      call lookup_shell(end_nfps, nshells, da1, m1)
      call lookup_shell(end_nfps, nshells, da2, m2)
      call lookup_shell(end_nfps, nshells, db1, n1)
      call lookup_shell(end_nfps, nshells, db2, n2)
      
c-------------------------------------------------------------------------
c   Build an array of coordinates per atom.
c-------------------------------------------------------------------------

      do m = 1, nshells
         iatom = atom(m)
         xn(iatom) = dbuf(icoord+3*(m-1))
         yn(iatom) = dbuf(icoord+3*(m-1)+1)
         zn(iatom) = dbuf(icoord+3*(m-1)+2)
      enddo

      do m = 1, ncenters 
         acharge(m) = charge(m) 
      enddo 

c     write(6,*) 'VFL H1FRAG :', watom 

      newcenters = 0  
      last_atom  = 0 
      do m = 1, nshells
         iatom = atom(m)
         do n = 1, natoms_frag(watom)   
            jatom = watom_frag(watom, n)  
            if ((iatom .ne. jatom) .and. (last_atom .ne. iatom)) then 
               newcenters = newcenters + 1 
               xn(newcenters) = dbuf(icoord+3*(m-1))
               yn(newcenters) = dbuf(icoord+3*(m-1)+1)
               zn(newcenters) = dbuf(icoord+3*(m-1)+2)
               acharge(newcenters) = charge(iatom) 
               last_atom = iatom 
c              write(6,*) '  ', newcenters, iatom, xn(newcenters), 
c    *                          yn(newcenters), zn(newcenters) 
            endif 
         enddo  
      enddo
      
      if (newcenters .ne. ncenters-natoms_frag(watom)) then 
         write(6,*) ' NCENTERS incorrect in h1frag '
      endif 

c-------------------------------------------------------------------------
c   Calculate the integrals over the necessary shell blocks.
c-------------------------------------------------------------------------

       do m = m1, m2
       do n = n1, n2

            ncsum = ncfps(m) + ncfps(n) 
            x1 = dbuf(icoord+3*(m-1))
            y1 = dbuf(icoord+3*(m-1)+1)
            z1 = dbuf(icoord+3*(m-1)+2)
 
            x2 = dbuf(icoord+3*(n-1))
            y2 = dbuf(icoord+3*(n-1)+1)
            z2 = dbuf(icoord+3*(n-1)+2)

            call pack_coeffs_oed(dbuf(ialpha), ixalpha, dbuf(ipcoeff), 
     *                       ixpcoef, ncfps, npfps, m, n,
     *                       alpha_pack, nalpha, pcoeff_pack,
     *                       ncoeff, ibuf(iccbeg), ibuf(iccend), 
     *                       indx_cc,
     *                       ccbeg_pack, ccend_pack,
     *                       max_dim_coeff)

c ------------------------------------------------------------------------ 
c NAI (type=1) integrals first.   
c ------------------------------------------------------------------------ 

              call oed__gener_nai_batch(intmax, zmax, nalpha, ncoeff,
     *                 ncsum, ncfps(m), ncfps(n), npfps(m),npfps(n),
     *                 ivangmom(m), ivangmom(n), x1,y1,z1,x2,y2,z2,
     *                 newcenters, xn,yn,zn, acharge, alpha_pack,
     *                 pcoeff_pack, ccbeg_pack, ccend_pack,
     *                 spherical, .true., ibuf(ipkgscr), nints, nfirst,
     *                 dbuf(dpkgscr))

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

                 call add_integrals2(z, da1,da2,db1,db2,
     *                 dbuf(dpkgscr+nfirst-1),a1,a2,b1,b2,1.0d0)

               endif ! nints 

c              IF (ECP .eq. 1) then 
c                 do a = a1, a2 
c                 do b = b1, b2 
c                    z(a,b) = z(a,b)+ecp_erd(a,b)
c                 enddo 
c                 enddo 
c              endif 

         enddo   ! n shells
         enddo   ! m shells

      return
      end
