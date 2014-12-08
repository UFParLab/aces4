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
      subroutine abort_job()

      implicit none
!      include 'sip_interface.f' <- This has problems with PGI
      interface
      subroutine sip_abort() bind(C)
        use, intrinsic :: ISO_C_BINDING
      end subroutine sip_abort
      end interface
 
      integer ierr
    
      print *,'aborting job'
!      call print_opcounter()
!      write(66,10)
!      call mpi_abort(mpi_comm_world, 1, ierr)
      call sip_abort()
      return

10    Format(5x,'Aborting job. 
     * Check output and error files to determine cause') 
      end
