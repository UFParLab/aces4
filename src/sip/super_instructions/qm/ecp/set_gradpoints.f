c
      subroutine set_gradpoints(max_centers, Cord, Natoms, Displ, eps) 
      Implicit none 
      Integer Natoms 
      Double precision Displ(max_centers,3,2) 
      Integer i, j, k 
      Integer Max_centers 
      Double precision Cord(3,max_centers) 
      Double precision x, y, z, dx_p, dx_m, dy_p, dy_m, dz_p, dz_m 
      Double precision eps 

      eps = 1.0d-5  

10    format(2x,'Initial geometry ', 3F12.8) 

      do i = 1, natoms 

         write(6,10) (Cord(j,i), j=1, 3)  

         x = Cord(1,i) 
         y = Cord(2,i) 
         z = Cord(3,i) 

         dx_p = x + eps 
         dx_m = x - eps 

         dy_p = y + eps 
         dy_m = y - eps 

         dz_p = z + eps 
         dz_m = z - eps 

         Displ(i,1,1) = dx_p 
         Displ(i,1,2) = dx_m 

         Displ(i,2,1) = dy_p 
         Displ(i,2,2) = dy_m 

         Displ(i,3,1) = dz_p 
         Displ(i,3,2) = dz_m 

      enddo 

      write(6,100) 
      do i = 1, natoms 
         write(6,200) i, (Displ(i,1,k), k=1, 2) 
         write(6,300) i, (Displ(i,2,k), k=1, 2) 
         write(6,400) i, (Displ(i,3,k), k=1, 2) 
      enddo 

100   Format(2x,'CARTESIAN DISPLACEMENTS USED TO CONSTRUCT THE ECP 
     *       GRADIENTS')  

200   Format(2x, 'ATOM', I4, 2x, 'X+', F12.8, 2x, 'X-', F12.8) 
300   Format(2x, 'ATOM', I4, 2x, 'Y+', F12.8, 2x, 'Y-', F12.8) 
400   Format(2x, 'ATOM', I4, 2x, 'Z+', F12.8, 2x, 'Z-', F12.8) 
      return 
      end 
