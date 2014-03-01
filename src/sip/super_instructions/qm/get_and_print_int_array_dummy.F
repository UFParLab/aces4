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
      subroutine get_and_print_int_array_dummy(
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

      integer(C_INT) nd, i
      TYPE(C_PTR) :: dimsp, valsp
      integer(C_INT), pointer:: dimsfp(:), valsfp(:)

      include 'sip_interface.f'
!      integer, target :: d(1)
!      integer vals(1)

      ierr = 0

      print *,'executing get_and_print_int_array_dummy'
      call predefined_int_array('int_array_data'//C_NULL_CHAR, nd,
     *   dimsp, valsp);
      call c_f_pointer(dimsp, dimsfp, [1])
      call c_f_pointer(valsp, valsfp, dimsfp)
      print *,'nd=',nd,' dimspf=',dimsfp(nd)
      print *,(valsfp(i), i = 1, dimsfp(1))

      do i = 1, 8
          data_0(i) = valsfp(i)
      enddo
            return

      end
