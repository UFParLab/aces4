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
      subroutine diis_setup(array_table, narray_table, 
     *                      index_table,
     *                      nindex_table, segment_table, nsegment_table,
     *                      block_map_table, nblock_map_table,
     *                      scalar_table, nscalar_table, 
     *                      address_table, op)
c--------------------------------------------------------------------------
c   Establishes the lower triangular array elements for a DIIS calculation.
c--------------------------------------------------------------------------

      implicit none
      include 'interpreter.h'
      include 'trace.h'
      include 'mpif.h'
      include 'saved_data.h'

      integer narray_table, nindex_table, nsegment_table, 
     *        nblock_map_table
      integer op(loptable_entry)
      integer array_table(larray_table_entry, narray_table)
      integer index_table(lindex_table_entry, nindex_table)
      integer segment_table(lsegment_table_entry, nsegment_table)
      integer block_map_table(lblock_map_entry, nblock_map_table)
      integer nscalar_table
      double precision scalar_table(nscalar_table)
      integer*8 address_table(narray_table)

      integer handle, ind
      integer row, col

      integer diis_counter, bhandle
      double precision b_array

      common /diis/b_array(10,10),
     *               diis_counter,bhandle(10)

      save row, col

      if (first_diis_setup) then
         diis_counter = 0
         first_diis_setup = .false.
         row = 1
         col = 1
      endif
      
c-------------------------------------------------------------------------
c   Put the array index in the appropriate slot.
c-------------------------------------------------------------------------

      diis_counter = diis_counter + 1
      if (diis_counter .gt. 55) diis_counter = 1

      handle = op(c_result_array)

c--------------------------------------------------------------------------
c   Make sure the array_type is scalar.
c--------------------------------------------------------------------------

      if (array_table(c_array_type,handle) .ne. scalar_value) then
         print *,'DIIS_SETUP at line ',current_line,': argument should',
     *     ' be declared scalar.'
         call abort_job()
      endif

c---------------------------------------------------------------------------
c   Save the scalar value in the common block.
c---------------------------------------------------------------------------

      ind    = array_table(c_scalar_index, handle)
      b_array(row, col) = scalar_table(ind)

c---------------------------------------------------------------------------
c   Save handle of diagonal elements for storing results.
c---------------------------------------------------------------------------

      if (row .eq. col) bhandle(row) = handle

c---------------------------------------------------------------------------
c   Adjust row and col for next call.
c---------------------------------------------------------------------------

      col = col + 1
      if (col .gt. 10) then
         row = row + 1
         col = row
      endif

      if (row .gt. 10) then
         row = 1
         col = 1
      endif

      return
      end

