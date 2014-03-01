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
      subroutine lookup_shell(table, nshells, a, m)
c---------------------------------------------------------------------------
c   Converts basis function coordinates into shell block coordinates,
c   using a lookup table that relates the shell index to the number of
c   functions per shell.
c
c   Arguments:
c   	table		Array containing the ending functions per shell.
c       nshells         Number of shells.
c	a		Function coordinate to be converted.
c	m		Return value for the shell block coordinate 
c			corresponding to a.
c---------------------------------------------------------------------------

      implicit none
      integer nshells
      integer table(nshells)
      integer i, a, m, sum

      do i = 1, nshells
         if (table(i) .ge. a) then
            m = i
            return
         endif
      enddo

      m = nshells
      return
      end 
     
