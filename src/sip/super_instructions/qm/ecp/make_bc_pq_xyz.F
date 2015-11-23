      Subroutine Make_BC_PQ_xyz(Nkxyz, PQSxyz, TPQxyz)
C
c This routine compute sums; Sum_a,b{a,b=0,n_i,n_j}{fac(ni,a)CA_i^^(n_i-a)
C where i=x,y,z
C
      Implicit Double Precision (A-H, O-Z)
C
#include "ecp.par"
#include "ecp.com"
    
      Dimension TPQxyz(0:2*Maxang), PQSxyz(0:2*Maxang)

      Call Dzero(TPQxyz, 2*Maxang+1)   
      
      Do Ka = 0, Nkxyz

         Pre_Faca = Bcoefs(Nkxyz, Ka)

         TPQxyz(Ka) = TPQxyz(Ka) + Pre_Faca*PQSxyz(Nkxyz-Ka)

#ifdef _DEBUG_LVLM1
          Write(6,"((1x,i3),3(1x,F10.4))"), Ka,
     &            pre_faca, PQSxyz(Nkxyz-Ka),TPQxyz(kA+KB)
#endif

      Enddo
    
      Return
      End
