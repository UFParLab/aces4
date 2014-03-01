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
      subroutine set_frag(
     * array_1, rank_1, index_values_1, size_1, extents_1, data_1,
     * ierr) BIND(C)
      use, intrinsic :: ISO_C_BINDING
      implicit none 
c--------------------------------------------------------------------------
      include 'sip_interface.f' 

! scalar array --> array_1
      integer(C_INT), intent(in)::array_1
      integer(C_INT), intent(in)::rank_1
      integer(C_INT), intent(in)::index_values_1(1:rank_1)
      integer(C_INT), intent(in)::size_1
      integer(C_INT), intent(in)::extents_1(1:rank_1)
      real(C_DOUBLE), intent(out)::data_1(1:size_1) 

      integer(C_INT), intent(out)::ierr

      integer(C_INT) nfrags 

      nfrags = int_constant("nfrags"//C_NULL_CHAR) 

      data_1(1) = nfrags
      
      return
      end

