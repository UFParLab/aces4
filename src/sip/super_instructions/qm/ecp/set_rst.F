      Subroutine Set_rst(Nx, Ny, Nz)

      Implicit Integer (A-z)

#include "ecp.par"
#include "ecp.com"
C
      Dimension Rstval(3,84)
C
      Indx = 0
      Nxx = Nx + 1
      Nyy = Ny + 1
      Nzz = Nz + 1
      
      Nxyz = Nxx + Nyy + Nzz
     
      Do N = 1, Nxyz
         Do Rx = 1, Nxx
            Do Sy = 1, Nyy
               Do Tz = 1, Nzz

                  If (Rx+Sy+Tz .EQ. (N+2)) Then
                     Ndx = 1 
                     Rstval(1, Indx) = Rx - 1
                     Rstval(2, Indx) = Sy - 1
                     Rstval(3, Indx) = Tz - 1
                  Endif

               Enddo
             Enddo
         Enddo
      Enddo
C     
#ifdef _DEBUG_LVL0     
      Write(6,"(a)") "The rst values"
      Do i=1, 3
         Write(6,"(10I3)") (rstval(i, i1),i1=1,84)
         Write(6,*)
      Enddo
#endif
c     
      Return
      End
