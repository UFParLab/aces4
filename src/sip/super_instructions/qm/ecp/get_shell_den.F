      Subroutine Get_shell_den(Dens, Dens_4shell, Naobfns, La, Lb, 
     &                         Iloc, Jloc, Numcoi, Numcoj)
      
      Implicit Double Precision(A-H, O-Z)
     
      Dimension Dens(Naobfns,*), Dens_4shell(*)

#include "ecp.par"
#include "ecp.com"

#ifdef _DEBUG_LVLM
      Write(6,"(a)")"@-Get-shell:The reordered AO basis density matrix"
      call debugout(Dens, naobfns)
#endif

      Ind = 0
      Do Iang = 1, (La+1)*(La+2)/2
         Do Jang = 1, (Lb+1)*(Lb+2)/2
C
             Do Icon = 1, Numcoi
                Do Jcon = 1, Numcoj

                   Ind  = Ind  + 1
                   Indx = Iloc + (Icon-1)*(La+1)*(La+2)/2 + Iang
                   Jndx = Jloc + (Jcon-1)*(Lb+1)*(Lb+2)/2 + Jang
                   
                   Dens_4shell(Ind) = Dens(Indx,Jndx)

                Enddo
             Enddo
C
         Enddo
      Enddo

#ifdef _DEBUG_LVLM
      length = Numcoi*Numcoj*(La+1)*(La+2)*(Lb+1)*(LB+2)/4
      Write(6,*) 
      Write(6,"(a)") "Shell Densities"
      Write(6, "(6(1x,F10.7))") (dens_4shell(i), i=1, length)
#endif

   
      Return
      End
