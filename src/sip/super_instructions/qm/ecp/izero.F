
      Subroutine izero(Array, Length)
C
C A trivial simple FORTRAN program to initialize a double precision 
C array to zero. 
C
      Implicit None
     
      Integer Array(Length) 
      Integer Length, Index


      Do index = 1, Length

         Array(Index) = 0
   
      Enddo

      Return
      End
   
