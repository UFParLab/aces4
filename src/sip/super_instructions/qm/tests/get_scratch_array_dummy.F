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
      subroutine get_scratch_array_dummy(
     c array_slot_0, rank_0, index_values_0, size_0, extents_0, data_0,
     c  ierr) BIND(C)
      use, intrinsic :: ISO_C_BINDING
      implicit none
      integer(C_INT), intent(in)::array_slot_0
      integer(C_INT), intent(in)::rank_0
      integer(C_INT), intent(in)::index_values_0(1:rank_0)
      integer(C_INT), intent(in)::size_0
      integer(C_INT), intent(in)::extents_0(1:rank_0)
      real(C_DOUBLE), intent(out)::data_0(1:size_0)
      integer(C_INT), intent(out)::ierr

      include 'sip_interface.f'

      TYPE(C_PTR) :: cpscratch
      real(C_DOUBLE), pointer :: pscratch(:)
      integer(C_INT) nd, i, numElems
      ierr = 0


      print *,'getting scratch array'

      numElems = 32
      call scratch_array(numElems, cpscratch)
      call c_f_pointer(cpscratch, pscratch, [numElems])


      do i = 1, numElems
          pscratch(i) = i
      enddo

      print *,(pscratch(i), i = 1, numElems)


      do i = 1, numElems
          data_0(i) = pscratch(i)
      enddo

      call delete_scratch_array(cpscratch)


      return

      end
