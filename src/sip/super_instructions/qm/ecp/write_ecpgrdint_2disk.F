      Subroutine Write_ecpgrdint_2disk(Ntotatoms, Nint, Ecpgrdint)

      Implicit Double Precision (A-H, O-Z)

      Dimension Ecpgrdint(Ntotatoms, 3, *)

      Ldim = Nint*(Nint+1)/2
      Open(Unit=62, File="decpint.dat", Form="unformatted")
      Rewind(62)

      Write(62) Ntotatoms

#ifdef _DEBUG_LVL0
      Write(6,"(a)") "The ECP deriavative integrals"
      Do Icent = 1, Ntotatoms

         Do Ixyz = 1, 3
            w = 0.0D0
            Write(6, "(a,1X,I4,a,i2)") "Atom: ", Icent, "XYZ :", Ixyz
            Write(6, "(7(1x,F15.7))") (Ecpgrdint(Icent,Ixyz,jj),
     &                                  jj =1, Ldim)
            do jj=1, Ldim
            w = w + Ecpgrdint(Icent,Ixyz,jj)*Ecpgrdint(Icent,Ixyz,jj)
            enddo
            Write(6, "(a,(1x,F20.13))") "The int. check sum = ", w
         Enddo
      Enddo
#endif
      Do Icent = 1, Ntotatoms

         Do Ixyz = 1, 3
            Do I = 1, Nint
               Ecpgrdint(Icent,Ixyz,i*(i+1)/2) = 2.0D0*Ecpgrdint(Icent,
     &                                        Ixyz,i*(i+1)/2)
            Enddo
            Write(62) Nint
            Write(62) (Ecpgrdint(Icent,Ixyz,jj), JJ=1, Ldim)
         Enddo
      Enddo
C
      Close(62)

      Return
      End
