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
      subroutine return_h1(
     c array_slot, rank, index_values, size, extents, data,
     c  ierr) BIND(C)
      use, intrinsic :: ISO_C_BINDING
      implicit none
    
c---------------------------------------------------------------------------
c Interface added but should be included instead 

      include 'sip_interface.f' 
      include 'imemory.h' 

c---------------------------------------------------------------------------

      integer(C_INT), intent(in)::array_slot
      integer(C_INT), intent(in)::rank
      integer(C_INT), intent(in)::index_values(1:rank)
      integer(C_INT), intent(in)::size
      integer(C_INT), intent(in)::extents(1:rank)
      real(C_DOUBLE), intent(out)::data(1:size)
      integer(C_INT), intent(out)::ierr
      integer(C_INT) ncenters  
c     integer(C_INT) iscr(10000000) !!! NEED TO MALLOC VFL  
c     real(C_DOUBLE) scr(10000000)  !!! NEED TO MALLOC VFL  

      real(C_DOUBLE), pointer :: scr(:)
      real(C_DOUBLE), pointer :: iscr(:)
      TYPE(C_PTR) :: cscr
      TYPE(C_PTR) :: ciscr

c Nuclear coordinates 
c --------------------------------------------------------------------------- 
      TYPE(C_PTR) :: pDimCoords
      TYPE(C_PTR) :: pcoords
      integer, pointer :: sDimCoords(:)
      real(C_DOUBLE), pointer :: coords(:)

c Nuclear coordinates 
c --------------------------------------------------------------------------- 
      TYPE(C_PTR) :: pDimTCoords
      TYPE(C_PTR) :: pTcoords
      integer, pointer :: sDimTCoords(:)
      real(C_DOUBLE), pointer :: Tcoords(:)
c --------------------------------------------------------------------------- 
c aosegments 
c     integer(C_INT) n_ao_segs(1), aosegs(6)    
      TYPE(C_PTR) :: n_ao_segs, aosegs
      integer, pointer :: pn_ao_segs(:), paosegs(:)

      integer(C_INT) norb, one, two, offset_1, offset_2 
      integer(C_INT) i, nscr, niscr   

c--------------------------------------------------------------------------
c
c   return_h1 must be executed with the following syntax:
c   execute return_h1 h1(mu,nu)
c
c   The array h1 is blocked and as of now a temporary array.  
c
c--------------------------------------------------------------------------

c-------------------------------------------------------------------------
c   Get the data needed to do the computation.  
c--------------------------------------------------------------------------

c                     Nuclear coordinates 
      call predefined_scalar_array("coords"//C_NULL_CHAR,
     *                                  two, pDimCoords, pcoords)
      call c_f_pointer(pDimCoords, sDimCoords,[2]) 
      call c_f_pointer(pcoords, coords, sDimCoords) 

c                     Nuclear Tcoordinates 
      call predefined_scalar_array("tcoords"//C_NULL_CHAR,
     *                                  two, pDimTCoords, pTcoords)
      call c_f_pointer(pDimTCoords, sDimTCoords,[2]) 
      call c_f_pointer(pTcoords, Tcoords, sDimTCoords) 


c                   Number of nuclear centers 
      ncenters = int_constant("ncenters"//C_NULL_CHAR) 

c
c Get the ao segment ranges  
      call predefined_int_array("ao_seg_ranges"//C_NULL_CHAR, one, 
     *                           n_ao_segs, aosegs)  
      call c_f_pointer(n_ao_segs,pn_ao_segs,[1]) 
      call c_f_pointer(aosegs,paosegs,pn_ao_segs) 
c     write(6,*) ' NAO_SEGS', pn_ao_segs(1), (paosegs(i), i=1, 
c    *                                             pn_ao_segs(1))  

c   Set scratch space 

      nscr  = zmax ! 5000000 
      niscr = imax ! 5000000
      call scratch_array(nscr, cscr) 
      call c_f_pointer(cscr, scr, [nscr]) 

      call scratch_array(niscr, ciscr) 
      call c_f_pointer(ciscr, iscr, [niscr])

c---------------------------------------------------------------------------
c   Compute data block of integrals.
c---------------------------------------------------------------------------
c
c Find the offset index ranges 
      offset_1 = 0 
      offset_2 = 0 
      if (index_values(1) .gt. 1) offset_1 = paosegs(index_values(1)-1) 
      if (index_values(2) .gt. 1) offset_2 = paosegs(index_values(2)-1) 

c     write(6,*) ' OFFSETS = ', index_values(1), offset_1 
c     write(6,*) ' OFFSETS = ', index_values(2), offset_2 
      call comp_return_h1(data, 1+offset_1, extents(1)+offset_1,
     *           1+offset_2, extents(2)+offset_2, ncenters, coords,
     *           tcoords, scr, iscr)

c  Remove scratch space  
      call delete_scratch_array(cscr)
      call delete_scratch_array(ciscr)

      return
      end
