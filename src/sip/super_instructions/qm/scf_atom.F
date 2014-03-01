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
      subroutine scf_atom(array_table,
     *                      narray_table, index_table,
     *                      nindex_table, segment_table, nsegment_table,
     *                      block_map_table, nblock_map_table,
     *                      scalar_table, nscalar_table,
     *                      address_table, op)
c-------------------------------------------------------------------------- 
      implicit none
      include 'mpif.h'
      include 'interpreter.h'
      include 'int_gen_parms.h'
      include 'trace.h'
      include 'parallel_info.h'
#ifdef ALTIX
      include 'sheap.h'
#endif

      integer narray_table, nindex_table, nsegment_table,
     *        nblock_map_table, array 
      integer op(loptable_entry)
      integer array_table(larray_table_entry, narray_table)
      integer index_table(lindex_table_entry, nindex_table)
      integer segment_table(lsegment_table_entry, nsegment_table)
      integer block_map_table(lblock_map_entry, nblock_map_table)
      integer nscalar_table
      double precision scalar_table(nscalar_table)
      integer*8 address_table(narray_table), get_index_from_base
      integer*8 get_block_data_index
      integer find_current_block, block 

      integer array_type, nindex, inda(2), indb(2)    

      logical direct_flag

      integer i, j, k
      integer nind, nsend
      integer ierr, handle
      integer blk, blkndx, maxblk
      integer*8 iarray, farray, indblk, get_block_index
      integer*8 ibarray, fbarray 
      integer*8 nai_array, kin_array, ovl_array 
      integer*8 integral_scr
      integer ind, seg, val1(mx_array_index), val2(mx_array_index)
      integer msg(len_sip_server_message)
      integer flag
      integer stack
      integer sind  

      double precision x(1)
      double precision dbuf(1), temp 
      integer ibuf(1)
#ifdef ALTIX
      pointer (dptr, dbuf)
      pointer (xptr, x)
      pointer (iptr, ibuf)
#else
      common dbuf, x
      equivalence (ibuf(1), dbuf(1))
#endif

      common /d2int_com/jatom, jx, jcenter
      integer jatom, jx, jcenter

#ifdef ALTIX
      iptr = ishptr
      dptr = dshptr
      xptr = dshptr
#endif

      array       = op(c_result_array)
      direct_flag = .false. 
      array_type  = array_table(c_array_type, array)
      nindex      = array_table(c_nindex, array)

      if (array_type .ne. scalar_value) then
         print *,'Error: The only argument in scf_atom  
     *            must be a scalar.'
         print *,(op(i),i=1,loptable_entry)
         call abort_job()
      endif

      sind =  array_table(c_scalar_index, array)
      if (sind .lt. 1 .or. sind .gt. nscalar_table) then
         print *,'Scalar table index out of range in scf_atom, ',
     *           'line ',current_line
         print *,'Index for array ',array,' is ',sind,' should be ',
     *           'between 1 and ',nscalar_table
         call abort_job()
      endif

c----------------------------------------------------------------------------
c   Set up the integral pointer 
c----------------------------------------------------------------------------

      integral_scr = dpkgscr
      maxblk  = zmax

c----------------------------------------------------------------------------
c   Get the Nuclear attraction integrals.
c----------------------------------------------------------------------------

      array = 11 
      array_type = array_table(c_array_type, array)

      if (array_type .ne. static_array) then
         print *,'Error: Nuclear attraction integrals calc
     *            must be static.'
         call abort_job()
      endif

      nindex      = array_table(c_nindex, array)

      if (nindex .ne. 2) then
         print *,'Error: NAI array must have ',
     *           'two indices.'
         call abort_job()
      endif

      do i = 1, nindex
         na = array_table(c_index_array1+i-1,array)
         inda(i) = index_table(c_index_size, na)  ! pick up length of index
      enddo

      nai_array = get_index_from_base(address_table(array), x, 2)

c----------------------------------------------------------------------------
c   Get the Kinetic energy integrals.
c----------------------------------------------------------------------------

      array = 12 
      array_type = array_table(c_array_type, array)

      if (array_type .ne. static_array) then
         print *,'Error: Kinetic energy integrals calc
     *            must be static.'
         call abort_job()
      endif

      nindex      = array_table(c_nindex, array)

      if (nindex .ne. 2) then
         print *,'Error: KIN array must have ',
     *           'two indices.'
         call abort_job()
      endif

      do i = 1, nindex
         na = array_table(c_index_array1+i-1,array)
         inda(i) = index_table(c_index_size, na)  ! pick up length of index
      enddo

      kin_array = get_index_from_base(address_table(array), x, 2)

c----------------------------------------------------------------------------
c   Get the Overlap integrals.
c----------------------------------------------------------------------------

      array = 13 
      array_type = array_table(c_array_type, array)

      if (array_type .ne. static_array) then
         print *,'Error: overlap integrals calc
     *            must be static.'
         call abort_job()
      endif

      nindex      = array_table(c_nindex, array)

      if (nindex .ne. 2) then
         print *,'Error: ovl array must have ',
     *           'two indices.'
         call abort_job()
      endif

      do i = 1, nindex
         na = array_table(c_index_array1+i-1,array)
         inda(i) = index_table(c_index_size, na)  ! pick up length of index
      enddo

      ovl_array = get_index_from_base(address_table(array), x, 2)

c----------------------------------------------------------------------------
c   Get the alpha Fock array.
c----------------------------------------------------------------------------

      array = 17 ! Fockrohf_a for a placeholder   
      array_type = array_table(c_array_type, array)

      if (array_type .ne. static_array) then
         print *,'Error: Fock arrays in integral calc
     *            must be static.'
         call abort_job()
      endif

      nindex      = array_table(c_nindex, array)

      if (nindex .ne. 2) then
         print *,'Error: Fock array must have ',
     *           'two indices.'
         call abort_job()
      endif

      do i = 1, nindex
         na = array_table(c_index_array1+i-1,array)
         inda(i) = index_table(c_index_size, na)  ! pick up length of index
      enddo

      farray = get_index_from_base(address_table(array), x, 2)

c----------------------------------------------------------------------------
c   Get the beta Fock array.
c----------------------------------------------------------------------------

      array = 18 ! Fockrohf_b for a placeholder   
      array_type = array_table(c_array_type, array)

      if (array_type .ne. static_array) then
         print *,'Error: Fock arrays in integral calc
     *            must be static.'
         call abort_job()
      endif

      nindex      = array_table(c_nindex, array)

      if (nindex .ne. 2) then
         print *,'Error: Fock array must have ',
     *           'two indices.'
         call abort_job()
      endif

      do i = 1, nindex
         na = array_table(c_index_array1+i-1,array)
         indb(i) = index_table(c_index_size, na)  ! pick up length of index
      enddo

      fbarray = get_index_from_base(address_table(array), x, 2)

c---------------------------------------------------------------------------
c---------------------------------------------------------------------------

c     write(6,*) '  x(nai_array), x(kin_array), x(ovl_array),
c    *                 x(farray), x(fbarray)) ', 
c    *                 nai_array, kin_array, ovl_array,
c    *                 farray, fbarray 

      if (.not. dryrun) 
     *   call compute_atomscf(scalar_table(sind), 
     *                 dbuf(integral_scr),maxblk, ibuf(ipkgscr),
     *                 dbuf(icoord),dbuf(ipcoeff),dbuf(ialpha), 
     *                 ibuf(iccbeg), ibuf(iccend), 
     *                 1, inda(1), 1,inda(2), 
     *                 x(nai_array), x(kin_array), x(ovl_array),
     *                 x(farray), x(fbarray))

c    *   call compute_atommp2(scalar_table(sind), 
c    *                 dbuf(integral_scr),maxblk, ibuf(ipkgscr),
c    *                 dbuf(icoord),dbuf(ipcoeff),dbuf(ialpha), 
c    *                 ibuf(iccbeg), ibuf(iccend), 
c    *                 1, inda(1), 1,inda(2), 
c    *                 x(nai_array), x(kin_array), x(ovl_array),
c    *                 x(farray), x(fbarray))


      return
      end
