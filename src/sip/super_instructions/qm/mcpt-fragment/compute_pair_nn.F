      subroutine compute_pair_nn(
     * array_1, rank_1, index_values_1, size_1, extents_1, data_1,
     * array_2, rank_2, index_values_2, size_2, extents_2, data_2,
     * array_3, rank_3, index_values_3, size_3, extents_3, data_3,
     * ierr) BIND(C)
      use, intrinsic :: ISO_C_BINDING
      implicit none 
c--------------------------------------------------------------------------
      include 'sip_interface.f' 

! scalar array_1 --> fragment 1  
      integer(C_INT), intent(in)::array_1
      integer(C_INT), intent(in)::rank_1
      integer(C_INT), intent(in)::index_values_1(1:rank_1)
      integer(C_INT), intent(in)::size_1
      integer(C_INT), intent(in)::extents_1(1:rank_1)
      real(C_DOUBLE), intent(out)::data_1(1:size_1) 

! scalar array_2 --> fragment 2  
      integer(C_INT), intent(in)::array_2
      integer(C_INT), intent(in)::rank_2
      integer(C_INT), intent(in)::index_values_2(1:rank_2)
      integer(C_INT), intent(in)::size_2
      integer(C_INT), intent(in)::extents_2(1:rank_2)
      real(C_DOUBLE), intent(out)::data_2(1:size_2) 

! scalar array_3 --> pair distance threshold  
      integer(C_INT), intent(in)::array_3
      integer(C_INT), intent(in)::rank_3
      integer(C_INT), intent(in)::index_values_3(1:rank_3)
      integer(C_INT), intent(in)::size_3
      integer(C_INT), intent(in)::extents_3(1:rank_3)
      real(C_DOUBLE), intent(out)::data_3(1:size_3) 

      integer(C_INT), intent(out)::ierr
      integer one, two 
      integer ifrag, jfrag 
      double precision dmin, dcrit   
c
c ---------------------------------------------------------------------- 
c predefined data declarations  
c ---------------------------------------------------------------------- 
      integer(C_INT) nfrags 
      real(C_DOUBLE) r_thresh  
!     two dimensional array of atomic coordinates  
      TYPE(C_PTR) :: sDimCoords
      TYPE(C_PTR) :: coords
      integer, pointer :: pdim_coords(:)
      real(C_DOUBLE), pointer :: pcoords(:,:)
c array defining which atoms are in which fragment 
      TYPE(C_PTR) :: sDimfrag
      TYPE(C_PTR) :: pwatom_frag
      integer, pointer :: pdim_frag(:)
      integer(C_INT), pointer :: watom_frag(:,:)
c array defining the number of atoms per fragment 
      TYPE(C_PTR) :: pnatoms_frag
      integer(C_INT), pointer :: natoms_frag(:)
!     atomic charges     
      TYPE(C_PTR) :: charge, atoms
      integer, pointer :: patoms(:)
      real(C_DOUBLE), pointer :: pcharge(:)

      integer ncenters 
      double precision energy
c
c-----------------------------------------------------------------------
c Get the number of atoms 
c------------------------------------------------------------------------
      ncenters = int_constant("ncenters"//C_NULL_CHAR) 
c-----------------------------------------------------------------------
c Get the nuclear coordinates and charges  
c------------------------------------------------------------------------
      call predefined_scalar_array("charge"//C_NULL_CHAR,
     *                               one, atoms ,charge)
      call c_f_pointer(atoms, patoms,[1])
      call c_f_pointer(charge, pcharge, patoms)
c ----------------------------------------------------------------------
c  ATOMIC COORDINATES  
      call predefined_scalar_array("coords"//C_NULL_CHAR,
     *                                  two, sDimCoords,  coords)
      call c_f_pointer(sDimCoords, pdim_coords,[2]) 
      call c_f_pointer(coords, pcoords, pdim_coords) 
c ----------------------------------------------------------------------

      ifrag = data_1(1)  
      jfrag = data_2(1)  

      energy = 0.0
      call return_pair_nn_repulsion(ifrag,jfrag,energy, 
     *                              ncenters, pcoords, pcharge) 
      data_3(1) = energy
      ierr = 0

      return
      end

      subroutine return_pair_nn_repulsion(ifrag,jfrag,energy, 
     *                                    ncenters, coords, charge) 

c---------------------------------------------------------------------------

      use, intrinsic :: ISO_C_BINDING 
      implicit none 
      
c---------------------------------------------------------------------------
c Interface added but should be included instead 

      include 'sip_interface.f'

c array defining which atoms are in which fragment 
      TYPE(C_PTR) :: sDimfrag  
      TYPE(C_PTR) :: pwatom_frag 
      integer, pointer :: pdim_frag(:)  
      integer(C_INT), pointer :: watom_frag(:,:)
c array defining the number of atoms per fragment 
      TYPE(C_PTR) :: pnatoms_frag 
      integer(C_INT), pointer :: natoms_frag(:)

      integer (C_INT) nfrags

c---------------------------------------------------------------------------
      
      integer ncenters, i, j, kcoord, one, two     
      integer ifrag, jfrag, iatom, jatom
      double precision coords(3, ncenters), charge(ncenters), energy, 
     *                 dist   
      
c array defining which atoms are in which fragments 
c ----------------------------------------------------------------------
      call predefined_int_array("watom_frag"//C_NULL_CHAR,
     *                                  two, sDimfrag,  pwatom_frag)
      call c_f_pointer(sDimfrag, pdim_frag,[2])
      call c_f_pointer(pwatom_frag, watom_frag, pdim_frag)
c array defining the number of atoms in each fragment 
c ----------------------------------------------------------------------
      call predefined_int_array("natoms_frag"//C_NULL_CHAR,
     *                                  one, sDimfrag,  pnatoms_frag)
      call c_f_pointer(sDimfrag, pdim_frag,[1])
      call c_f_pointer(pnatoms_frag, natoms_frag, pdim_frag)

      energy = 0.0 
      if (ifrag .ne. jfrag) then 
         do i = 1, natoms_frag(ifrag)  
         do j = 1, natoms_frag(jfrag)  
               dist = 0.0 
               iatom = watom_frag(ifrag,i) 
               jatom = watom_frag(jfrag,j) 
               do kcoord = 1, 3 
                  dist = dist  
     *                 + (coords(kcoord,iatom)-coords(kcoord,jatom))**2  
               enddo
               dist = dsqrt(dist) 
               energy = energy + charge(iatom)*charge(jatom) / dist    
         enddo  
         enddo  
C         energy = energy/2.0 
      else
      endif 

      return 
      end 

