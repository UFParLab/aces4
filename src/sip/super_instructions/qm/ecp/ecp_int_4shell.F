      Subroutine Ecp_int_4shell(Cint, Ecpint_4shell, La, Lb, Numcoi,
     &                          Numcoj, Indnpri, Indnprj, Iprim, Jprim,
     &                          Jnt, Grads,
     &
     &                          expnt_norms,Contr,
     &
     &                          max_nprims,max_conts_shell,
     &                          
     &                          max_unfns_shell,
     &                          max_cofns_shell)

      Implicit Double Precision (A-H, O-Z)
      Logical Grads

#include "ecp.par"
#include "ecp.com"

      dimension Cint(max_unfns_shell * max_unfns_shell)
      dimension Ecpint_4Shell(max_cofns_shell * max_cofns_shell)
      dimension contr(max_nprims,max_conts_shell)
      dimension expnt_norms(max_nprims)

      If (Grads) Then
         La_lo = Istart_grd(La)
         La_hi = Iend_grd(La)
         Lb_lo = Istart(Lb)
         Lb_hi = Iend(Lb)
      Else
         La_lo = Istart(La)
         La_hi = Iend(La)
         Lb_lo = Istart(Lb)
         Lb_hi = Iend(Lb)
      Endif
      Int   = 0
      Jnt   = 0
C
       Do Ia = La_lo, La_hi
          Do Ib = Lb_lo, Lb_hi
             Int = Int + 1

             Do Icon = 1, Numcoi
                Do Jcon = 1, Numcoj
                   Jnt = Jnt + 1
                   Ecpint_4shell(Jnt) = Ecpint_4shell(Jnt) +
     &                                  Cint(Int)*
     &                                  Contr(Indnpri, Icon) *
     &                                  Contr(Indnprj, Jcon) * Fpi *
     &                                  expnt_norms(Indnpri) *
     &                                  expnt_norms(Indnprj)
C
#ifdef _DEBUG_LVL0
      Write(6, "(a,2(1x,I4))") "Jnt: ", Jnt
      Write(6,"(a,2(1x,I2))")
     &"Contracting ECP integral for primitive pair", Iprim, Jprim
      Write(6,"(2(1x,F20.13))")Contr(Indnpri,Icon),Contr(Indnprj,JCon)
      Write(6,"(2(1x,F20.13))")Ecpint_4shell(Jnt),Cint(Int)
#endif
                Enddo
             Enddo

          Enddo
       Enddo
       
       Return
       End

