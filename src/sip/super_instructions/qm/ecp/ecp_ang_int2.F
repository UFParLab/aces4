      Subroutine Ecp_Ang_int2(Xph, Yph, Zph, Lamu, lab, Nx, Ny, Nz, 
     &                        Lprj, PQS_xyz, Ang)

      Implicit Double Precision (A-H, O-Z)
      Logical Skip

#include "ecp.par"
#include "ecp.com"

      Dimension PQS_Xyz(0:2*Maxang,3),TPQ_Xyz(0:2*Maxang,3),
     &          Ang(0:2*Maxang,-Maxproj:Maxproj,0:2*Maxang)

#ifdef _DEBUG_LVLM
      Write(6,*) "@-Ecp_Ang_int2: The PQS_xyz array"
      Write(6,*)
      Write(6,"(3F10.6)") ((PQS_xyz(i,j), j=1, 3),i=0,lab)
      Write(6,*) 
       Write(6,"(a,F10.6)") "Ftab:", Fijk(3, 3, 0)
#endif
       Call make_BC_PQ_xyz(Nx, PQS_xyz(0,1), TPQ_xyz(0,1))
       Call make_BC_PQ_xyz(Ny, PQS_xyz(0,2), TPQ_xyz(0,2))
       Call make_BC_PQ_xyz(Nz, PQS_xyz(0,3), TPQ_xyz(0,3))
C
#ifdef _DEBUG_LVLM
      Write(6,*) "The SUm1_xyz array"
      do j=1, 3
         Write(6,*) "The J value :", J
         if (j .eq. 1) k=Nx
         if (j .eq. 2) k=Ny
         if (j .eq. 3) k=Nz
         Write(6,"(6(1x,F10.5))") (TPQ_xyz(i,j), i=0,k)
      enddo
#endif
C loop over Mprj (Mprj= -Lprj:Lprj)
       Do Iang = 0, 2*Maxang
          Do iprj = -Maxproj, Maxproj
              Do Jang = 0, 2*Maxang
                 Ang(Jang, Iprj, Iang) = 0.0D0
              Enddo
          Enddo
      Enddo
C
      Do Mprj = -Lprj, Lprj
C
c Now we have loops over Nax, Nay, NaZ
c
         Do ix = 0, Nx
            Do iy = 0, Ny
               Do iz = 0, Nz
#ifdef _DEBUG_LVLM
      Write(6,*)
      Write(6,'(a,4(1x,I2))') "@-Ecp_Ang_int2: Lprj,Nxyz: ", Lprj,Ix,
     &                         Iy, Iz
#endif
C
                  Ixyz = ix+iy+iz

                  l_high = Min(Lprj+Ixyz, Lamu)

                  Do Lam = 0, Lamu

c Loops over mu (-la to +la and -lb to +lb)

                     Ang_N_lam = 0.0D0

                     Do mu = -Lam, Lam
C
                        If (Lam .LE. Ixyz+Lprj .and. 
     &                      Mod(Lam+Lprj+Ixyz,2) .NE. 1) Then

CSSS                           Write(6,*) Lam, Ixyz+Lprj, Lam+Lprj+Ixyz
                            Call Screen_ang_typ2(Lprj, MPrj, Ix, Iy,
     &                                           Iz, Lam, Mu, Skip)
CSSS                            If (.NOT. Skip) Then

                               Call Make_res(Lam, Mu, Xph, Yph, Zph,
     &                                       Res)
C
                               Call Ang_int(Lprj, Mprj, Ix, Iy, Iz, 
     &                                      Lam, Mu, Ang_N_lam_mu)
#ifdef _DEBUG_LVLM
      Write(6,"(a,1x,a,2I2,3(1x,F15.8))") "Before Sum:", "L and M",
     &                                    LPrj, MPRj,  Ang_N_lam,
     &                                    ANg_N_lam_mu, Res
#endif
                               Ang_N_lam = Ang_N_lam + Ang_N_lam_mu*
     &                                     Res*Sqrt_fpi
C
#ifdef _DEBUG_LVLM
      Write(6,"(a)") "The angular part of the ints in Main"
      Write(6,"(5(1x,i2),(2(1x,F15.8)))"),Lam,mu,Ix,Iy,Iz,
     &                                 Ang_N_lam
      Write(6,*) 
#endif
CSSS                           Endif
                        Endif
C
                     Enddo

                     If (Lam .LE. Ixyz+Lprj .and. Mod(Lam+Lprj+Ixyz,2)
     &                   .NE. 1) Then
CSSS                        Write(6,*) Lam, Ixyz+Lprj, Lam+Lprj+Ixyz

#ifdef _DEBUG_LVLM
       Write(6,"(a, 3(1x,i2),1x,F15.8)")"The angular int:",Ixyz,
     &                                   Mprj,Lam,
     &                                   Ang(Ixyz, Mprj, Lam)
       Write(6,*)
#endif
                         Ang(Ixyz, Mprj, Lam) = Ang(Ixyz, Mprj, Lam) +
     &                                          Ang_N_lam*
     &                                          TPQ_xyz(Ix,1)*
     &                                          TPQ_xyz(Iy,2)*
     &                                          TPQ_xyz(Iz,3)
C
#ifdef _DEBUG_LVLM
       Write(6,"(a, 3(1x,i2),1x,F15.8)")"The angular int:",Ixyz,
     &                                   Mprj,Lam,
     &                                   Ang(Ixyz, Mprj, Lam)
       Write(6,*)
#endif
                     Endif

                  Enddo

                Enddo
            Enddo
         Enddo
C
      Enddo

      Return
      End

