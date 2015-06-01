
      subroutine drop_core_in_sip(
     * array_slot_0, rank_0, index_values_0, size_0, extents_0, data_0,
     * array_slot_1, rank_1, index_values_1, size_1, extents_1, data_1,
     *  ierr) BIND(C)
      use, intrinsic :: ISO_C_BINDING
      implicit none

      include 'sip_interface.f'
      include 'imemory.h'

      integer(C_INT), intent(in)::array_slot_0
      integer(C_INT), intent(in)::rank_0
      integer(C_INT), intent(in)::index_values_0(1:rank_0)
      integer(C_INT), intent(in)::size_0
      integer(C_INT), intent(in)::extents_0(1:rank_0)
      real(C_DOUBLE), intent(inout)::data_0(1:size_0)

      integer(C_INT), intent(in)::array_slot_1
      integer(C_INT), intent(in)::rank_1
      integer(C_INT), intent(in)::index_values_1(1:rank_1)
      integer(C_INT), intent(in)::size_1
      integer(C_INT), intent(in)::extents_1(1:rank_1)
      real(C_DOUBLE), intent(out)::data_1(1:size_1)

      TYPE(C_PTR) :: cscr1, cscr2, cscr3
      integer(c_int) :: nscr1, nscr2, nscr3
      real(c_double), pointer :: scr1(:)
      real(c_double), pointer :: scr2(:)
      real(c_double), pointer :: scr3(:)

      integer(C_INT), intent(out)::ierr
      integer i, j, n

      ierr = 0
      if (rank_0 .ne. 2) then
         print *,'Error in drop_core, Number of array 1',
     *           ' indices must be 2, actual number is ',rank_0
         call abort_job()
      endif
      if (rank_1 .ne. 2) then
         print *,'Error in drop_core, Number of array 2',
     *           ' indices must be 2, actual number is ',rank_0
         call abort_job()
      endif

      call drop_core_SCF_coefficients(data_0,data_1,
     *                                extents_0(1),extents_0(2),ierr)

      if (ierr .eq. 1) then
         call abort_job()
      endif

      return
      end

      subroutine drop_core_SCF_coefficients(MO,fock,nao,nmo,ierr)
      use, intrinsic :: ISO_C_BINDING 
      implicit none 

      include 'sip_interface.f'

      integer ierr
      integer nao,nmo
      double precision MO(nao,nmo)
      double precision fock(nmo,nmo)

      integer i, one, mu
      integer drop_core
      integer ncenters
      integer icharge
!     atomic charges
      TYPE(C_PTR) :: charge, atoms
      integer, pointer :: patoms(:)
      real(C_DOUBLE), pointer :: pcharge(:)


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

      drop_core = 0

      do i = 1,ncenters
	  icharge = pcharge(i)
	  if (icharge .gt. 2 .and. icharge .le. 10) then
C drop helium core
	      drop_core = drop_core + 1
	  elseif (icharge .gt. 10 .and. icharge .le. 18) then
C drop neon core
	      drop_core = drop_core + 10
	  else if (icharge .gt. 18 .and. icharge .le. 36) then
C drop argon core
	      drop_core = drop_core + 18
	  else if (icharge .gt. 36 .and. icharge .le. 54) then
C drop krypton core
              write(*,*) 'Warning: dropping krypton core'
	      drop_core = drop_core + 36
	  else if (icharge .gt. 54) then
C what the fuck, drop zenon core
              write(*,*) 'Serious warning: dropping xenon core'
	      drop_core = drop_core + 54
	  endif
      enddo

      if (drop_core .gt. nmo) then
	  write(*,*) "error, drop_core greater than occupied space:",
     *                drop_core
          ierr = 1
          return
      endif

      write(*,*) "Setting first",drop_core,"occupied orbitals to zero"

      do i = 1,drop_core
C	  fock(i,i) = 0.d0
	  do mu = 1,nao
	      MO(mu,i) = 0.d0
	  enddo
      enddo

C debug
C      write(*,*) (i,i=1,nmo)
c      do mu = 1,nao
c	  write(*,101) (MO(mu,i),i=1,nmo)
c      enddo

  101 format(100F14.8)

      return
      end

