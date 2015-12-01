      subroutine moi_nn_repulsion(
     * array_slot_0, rank_0, index_values_0, size_0, extents_0, data_0, 
     * array_slot_1, rank_1, index_values_1, size_1, extents_1, data_1, 
     * ierr) BIND(C) 
       use, intrinsic :: ISO_C_BINDING
      implicit none
      include 'sip_interface.f'  
c---------------------------------------------------------------------------
!> special moi_nn_repulsion u
!>  Compute the nuclear-nuclear repulsion energy for a monomer of interest 
!>  Returns a scalar
c----------------------------------------------------------------------------

      integer(C_INT), intent(in)::array_slot_0
      integer(C_INT), intent(in)::rank_0
      integer(C_INT), intent(in)::index_values_0(1:rank_0)
      integer(C_INT), intent(in)::size_0
      integer(C_INT), intent(in)::extents_0(1:rank_0)
      real(C_DOUBLE), intent(out)::data_0(1:size_0)

      integer(C_INT), intent(in)::array_slot_1
      integer(C_INT), intent(in)::rank_1
      integer(C_INT), intent(in)::index_values_1(1:rank_1)
      integer(C_INT), intent(in)::size_1
      integer(C_INT), intent(in)::extents_1(1:rank_1)
      real(C_DOUBLE), intent(out)::data_1(1:size_1)

      integer(C_INT), intent(out)::ierr 

!     one dimensional array of atomic coordinates  
      TYPE(C_PTR) :: sDimCoords
      TYPE(C_PTR) :: coords
      integer, pointer :: pdim_coords(:)
      real(C_DOUBLE), pointer :: pcoords(:,:)

!     atomic charges     
      TYPE(C_PTR) :: charge, atoms
      integer, pointer :: patoms(:)
      real(C_DOUBLE), pointer :: pcharge(:)

c array defining the number of atoms per fragment
      TYPE(C_PTR) :: pnatoms_frag
      integer(C_INT), pointer :: natoms_frag(:)

c array defining which atoms are in which fragment
      TYPE(C_PTR) :: sDimfrag
      TYPE(C_PTR) :: pwatom_frag
      integer, pointer :: pdim_frag(:)
      integer(C_INT), pointer :: watom_frag(:,:)

      integer m
      integer one, two
      integer ncenters 
      integer nfrags
      integer n_apf
      integer current_frag
      double precision energy

C temp array
      TYPE(C_PTR) :: ciatom
      integer(C_INT), pointer :: iatom(:)

c-----------------------------------------------------------------------
c Get the number of atoms 
c------------------------------------------------------------------------
      ncenters = int_constant("ncenters"//C_NULL_CHAR) 
      nfrags   = int_constant("nfrags"//C_NULL_CHAR)

c-----------------------------------------------------------------------
c Get the nuclear coordinates and charges  
c------------------------------------------------------------------------
      call predefined_scalar_array("charge"//C_NULL_CHAR,
     *                               one, atoms ,charge)
      call c_f_pointer(atoms, patoms,[1])
      call c_f_pointer(charge, pcharge, patoms)

c ----------------------------------------------------------------------
c  ATOMIC COORDINATES  
c ----------------------------------------------------------------------
      call predefined_scalar_array("coords"//C_NULL_CHAR,
     *                                  two, sDimCoords,  coords)
      call c_f_pointer(sDimCoords, pdim_coords,[2]) 
      call c_f_pointer(coords, pcoords, pdim_coords) 

c ----------------------------------------------------------------------
c array defining the number of atoms in each fragment
c ----------------------------------------------------------------------
      call predefined_int_array("natoms_frag"//C_NULL_CHAR,
     *                                  one, sDimfrag,  pnatoms_frag)
      call c_f_pointer(sDimfrag, pdim_frag,[1])
      call c_f_pointer(pnatoms_frag, natoms_frag, pdim_frag)

c ----------------------------------------------------------------------
c array defining which atoms are in which fragments
c ----------------------------------------------------------------------
      call predefined_int_array("watom_frag"//C_NULL_CHAR,
     *                                  two, sDimfrag,  pwatom_frag)
      call c_f_pointer(sDimfrag, pdim_frag,[2])
      call c_f_pointer(pwatom_frag, watom_frag, pdim_frag)

C cast argument two into int
      current_frag = int(data_1(1))
      n_apf = natoms_frag(current_frag)

      
      call integer_scratch_array(n_apf,ciatom)
      call c_f_pointer(ciatom,iatom,[n_apf])

      do m = 1,n_apf
          iatom(m) = watom_frag(current_frag,m)
      enddo

      energy = 0.0
      call monomer_nn_repulsion(energy, n_apf, ncenters,
     *                          iatom, pcoords, pcharge) 
      data_0(1) = energy

      call delete_scratch_array(ciatom)

      return
      end

