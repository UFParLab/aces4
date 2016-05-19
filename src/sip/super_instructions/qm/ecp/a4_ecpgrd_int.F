      Subroutine A4_Ecpgrd_int(Ecpint_4shell, La, Lb, 
     &                         Idegen_grd, Idegen, Jdegen, Numcoi,
     &                         Numcoj, Iprim, Jprim, Exp1, EXp2, 
     &                         Ecpgrd_x, Ecpgrd_y, Ecpgrd_z, Natoms,
     &                         Iatom, Jatom,
     &
     &                         max_cofns_shell)

      Implicit Double Precision (A-H, O-Z)

#include "ecp.par"
#include "ecp.com"
     
      Dimension Ecpint_4shell(max_cofns_shell * max_cofns_shell)
      Dimension Ecpgrd_X(max_cofns_shell * max_cofns_shell)
      Dimension Ecpgrd_Y(max_cofns_shell * max_cofns_shell)
      Dimension Ecpgrd_Z(max_cofns_shell * max_cofns_shell)
      
      Length = Idegen*Jdegen*Numcoi*Numcoj
      Call Dzero(Ecpgrd_x, Length)
      Call Dzero(Ecpgrd_y, Length)
      Call Dzero(Ecpgrd_z, Length)

      Texp1 = 2.0D0*Exp1
      Texp2 = 2.0D0*Exp2
      
      La_lo = Istart_grd(La)
      La_hi = Iend_grd(La)
      
      Lb_lo = Istart_grd(Lb)
      Lb_hi = Iend_grd(Lb) 

      Do Ndegen_grd = 1, Idegen_grd

         Ix = Lmnval_grd(4, Ndegen_grd + La_lo - 1)
         Iy = Lmnval_grd(5, Ndegen_grd + La_lo - 1)
         Iz = Lmnval_grd(6, Ndegen_grd + La_lo - 1)
         
         Tax = Texp1
         Tay = Texp1
         Taz = Texp1

         If (Lmnval_grd(7, Ndegen_grd + La_lo - 1) .Lt. 0) Then

            Tax = -(Lmnval_grd(1, Ndegen_grd + La_lo - 1) +1)
            Tay = -(Lmnval_grd(2, Ndegen_grd + La_lo - 1) +1)
            Taz = -(Lmnval_grd(3, Ndegen_grd + La_lo - 1) +1)

         Endif

#ifdef _DEBUG_LVLM
         Write(6,"(3I4,3(1x,F10.5))") ix, Iy, Iz, Tax, Tay, Taz
#endif

         Ndim = Jdegen*Numcoi*Numcoj

         If (Ix .NE. 0) Then
            Ioff = (Ndegen_grd-1)*Ndim+1
            Joff = (Ix-1)*Ndim+1
            Call daxpy(Ndim, Tax, Ecpint_4shell(Ioff), 1, 
     &                 Ecpgrd_x(Joff), 1)
         Endif


         If (Iy .NE. 0) Then
            Ioff = (Ndegen_grd-1)*Ndim+1
            Joff = (Iy-1)*Ndim+1
            Call daxpy(Ndim, Tay, Ecpint_4shell(Ioff), 1, 
     &                 Ecpgrd_y(Joff), 1)
         Endif
             
         If (Iz .NE. 0) Then
            Ioff = (Ndegen_grd-1)*Ndim+1
            Joff = (Iz-1)*Ndim+1
            Call daxpy(Ndim, Taz, Ecpint_4shell(Ioff), 1, 
     &                 Ecpgrd_z(Joff), 1)
         Endif

      Enddo

#ifdef _DEBUG_LVL0
      Write(6,*)
      Write(6,"(a)") "The differentiated integrals"
      Write(6,"(a,4(1x,I2))")
      Write(6,"(a)") "d/dx integrals"
      Write(6,"(4(1x,F20.13))")(Ecpgrd_x(j), J=1,Length)

      Write(6,"(a)") "d/dy integrals"
      Write(6,"(4(1x,F20.13))")(Ecpgrd_y(j), J=1,Length)

      Write(6,"(a)") "d/dz integrals"
      Write(6,"(4(1x,F20.13))")(Ecpgrd_z(j), J=1,Length)
#endif
#ifdef _OBSOLETED
      Grad_x = Ddot(Length, Ecpgrd_x, 1, Dens_4shell, 1)
      Grad_y = Ddot(Length, Ecpgrd_y, 1, Dens_4shell, 1)
      Grad_z = Ddot(Length, Ecpgrd_z, 1, Dens_4shell, 1)

#ifdef _DEBUG_LVL0
      Write(6,*) 
      Write(6,"(a)") "Grad_xyz"
      Write(6,"(3(1x,F20.13))") Grad_x, Grad_y, Grad_z
      Write(99, '(2I4)') Iatom, Jatom
      Write(99,"(3(1x,F20.13))") Grad_x, Grad_y, Grad_z
#endif
      
      Grad_xyz(1, Iatom) = Grad_x + Grad_xyz(1, Iatom)
      Grad_xyz(2, Iatom) = Grad_y + Grad_xyz(2, Iatom)
      Grad_xyz(3, Iatom) = Grad_z + Grad_xyz(3, Iatom) 

      Grad_xyz(1, Jatom) = -Grad_x + Grad_xyz(1, Jatom)
      Grad_xyz(2, Jatom) = -Grad_y + Grad_xyz(2, Jatom)
      Grad_xyz(3, Jatom) = -Grad_z + Grad_xyz(3, Jatom) 
#endif

      Return
      End
