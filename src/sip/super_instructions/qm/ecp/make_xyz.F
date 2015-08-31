      Subroutine Make_xyz(Nxyz, Cxyz, CS_xyz)
C
c This routine compute arrays PI{n_i=n,,k_i=0...n_i}CA_i^^(n_i-k_i) i=x,y,z
C where i=x,y,z
C

      Implicit Double Precision (A-H, O-Z)

#include "ecp.par"
    
      Dimension CS_xyz(0:2*Maxang)

      CS_xyz(0) = 1.0D0
      CS_xyz(1) = Cxyz
      IF (NXyz .EQ. 0 .OR. Nxyz .EQ. 1) Return

      Do I = 2, Nxyz

            CS_xyz(I) = CS_xyz(I-1)*CS_xyz(1)

      Enddo
    
      Return
      End
