
      Subroutine dzero(Array, length)
C
C A trivial simple FORTRAN program to initialize a double precision 
C array to zero. 
C
      Implicit None
     
      Double precision Array(Length) 
      Integer Length, Index


      Do index = 1, Length

         Array(Index) = 0.0D0
   
      Enddo

      Return
      End
   
