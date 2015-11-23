      Subroutine Make_BC_CACB_xyz(Nkaxyz, Nkbxyz, CASxyz, CBSxyz,
     &                            TCABxyz)
C
c This routine compute sums; Sum_a,b{a,b=0,n_i,n_j}{fac(ni,a)CA_i^^(n_i-a)
C CB_j^^(n_j-b)
C where i=x,y,z
C
      Implicit Double Precision (A-H, O-Z)
C
#include "ecp.par"
#include "ecp.com"
    
      Dimension TCABxyz(0:2*Maxang), CASxyz(0:2*Maxang), 
     &          CBSxyz(0:2*Maxang)

      Call Dzero(TCABxyz, 2*Maxang+1)   

      Do Ka = 0, Nkaxyz

         Pre_Faca = Bcoefs(Nkaxyz, Ka)

         Do Kb = 0, Nkbxyz

            Pre_Facb = Bcoefs(Nkbxyz, Kb) 

            TCABxyz(Ka+Kb) = TCABxyz(Ka+Kb) + Pre_Faca*Pre_Facb*
     &                    CASxyz(Nkaxyz-Ka)*CBSxyz(Nkbxyz-Kb)
#ifdef _DEBUG_LVLM1
          Write(6,"(2(1x,i3),5(1x,F10.4))"), Ka, Kb,
     &            pre_faca, Pre_Facb, CASxyz(Nkaxyz-Ka),
     &            CBSxyz(Nkbxyz-Kb), TCABxyz(Ka+Kb)
#endif

         Enddo

      Enddo
    
      Return
      End
