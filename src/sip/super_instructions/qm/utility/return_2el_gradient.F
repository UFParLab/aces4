C----------------------------------------------------------------
!> special return_2el_gradient rrur
!>
!> Return the two-particle contribution to the gradient from the
!> two electron deriavtive integrals and the two particle density
!> matrix (simple dot product). The two-electron integral and the 
!> two-particle density matrix can be any type of array and the
!> Gradient is a Contiguous local array.
!>
!> PARDO mu, nu, lambda, sigma
!>
!>      execute return_2el_gradient aoint[mu,nu,lambda,sigma] 
!>                                  density[mu,nu,lambda,sgma]
!>                                  Gradient flag
!>
!> ENDPARDO mu, nu, lambda, sigma
!>
!>  Ajith Perera, 09/2015.
C-----------------------------------------------------------------

      subroutine return_2el_gradient(
     c array_slot_0, rank_0, index_values_0, size_0, extents_0, data_0,
     c array_slot_1, rank_1, index_values_1, size_1, extents_1, data_1,
     c array_slot_2, rank_2, index_values_2, size_2, extents_2, data_2,
     c array_slot_3, rank_3, index_values_3, size_3, extents_3, data_3,
     c  ierr) BIND(C)

      use, intrinsic :: ISO_C_BINDING
      implicit none

c Interface added BUT should be include NOT added in the future

      include 'sip_interface.f'
      include 'imemory.h'

!     argument array

      integer(C_INT), intent(in)::array_slot_0
      integer(C_INT), intent(in)::rank_0
      integer(C_INT), intent(in)::index_values_0(1:rank_0)
      integer(C_INT), intent(in)::size_0
      integer(C_INT), intent(in)::extents_0(1:rank_0)
      real(C_DOUBLE), intent(in)::data_0(1:size_0)

      integer(C_INT), intent(in)::array_slot_1
      integer(C_INT), intent(in)::rank_1
      integer(C_INT), intent(in)::index_values_1(1:rank_1)
      integer(C_INT), intent(in)::size_1
      integer(C_INT), intent(in)::extents_1(1:rank_1)
      real(C_DOUBLE), intent(in)::data_1(1:size_1)

      integer(C_INT), intent(in)::array_slot_2
      integer(C_INT), intent(in)::rank_2
      integer(C_INT), intent(in)::index_values_2(1:rank_2)
      integer(C_INT), intent(in)::size_2
      integer(C_INT), intent(in)::extents_2(1:rank_2)
      real(C_DOUBLE), intent(out)::data_2(1:size_2)

      integer(C_INT), intent(in)::array_slot_3
      integer(C_INT), intent(in)::rank_3
      integer(C_INT), intent(in)::index_values_3(1:rank_3)
      integer(C_INT), intent(in)::size_3
      integer(C_INT), intent(in)::extents_3(1:rank_3)
      real(C_DOUBLE), intent(in)::data_3(1:size_3)

      integer(C_INT), intent(out)::ierr
c
c aosegments
c     integer(C_INT) n_ao_segs(1), aosegs(6)
      TYPE(C_PTR) :: n_ao_segs, aosegs
      integer, pointer :: pn_ao_segs(:), paosegs(:)
      integer(C_INT) ncenters

!     offsets of atomic indeces  
      integer(C_INT) offset_01, offset_02, offset_03, offset_04  
      integer(C_INT) offset_11, offset_12, offset_13, offset_14  
      integer(C_INT) i, j, one, two

      integer(C_INT) int_length1, int_length2, int_length3
      integer(C_INT) int_length4, int_length

      integer(C_INT) nscr, niscr
      integer der_flags(12), flag 

      ierr = 0 
      one  = 1
      two  = 2

      flag = INT(data_3(1))

c The number of centers
      ncenters = int_constant("ncenters"//C_NULL_CHAR)
C
c Get the ao segment ranges
c
      call predefined_int_array("ao_seg_ranges"//C_NULL_CHAR, one,
     *                           n_ao_segs, aosegs)
      call c_f_pointer(n_ao_segs,pn_ao_segs,[1])
      call c_f_pointer(aosegs,paosegs,pn_ao_segs)

      offset_01 = 0
      offset_02 = 0
      offset_03 = 0
      offset_04 = 0
      offset_11 = 0
      offset_12 = 0
      offset_13 = 0
      offset_14 = 0

      if (index_values_0(1) .gt. 1) offset_01
     &                              = paosegs(index_values_0(1)-1)
      if (index_values_0(2) .gt. 1) offset_02
     &                              = paosegs(index_values_0(2)-1)
      if (index_values_0(3) .gt. 1) offset_03
     &                              = paosegs(index_values_0(3)-1)
      if (index_values_0(4) .gt. 1) offset_04
     &                              = paosegs(index_values_0(4)-1)

      if (index_values_1(1) .gt. 1) offset_11
     &                              = paosegs(index_values_1(1)-1)
      if (index_values_1(2) .gt. 1) offset_12
     &                              = paosegs(index_values_1(2)-1)
      if (index_values_1(3) .gt. 1) offset_13
     &                              = paosegs(index_values_1(3)-1)
      if (index_values_1(4) .gt. 1) offset_14
     &                              = paosegs(index_values_1(4)-1)

      call compute_2particle_gradient(
     & 
     &                       data_0, data_1, data_2, flag, 
     &
     &                        1+offset_01, extents_0(1)+offset_01,
     &                        1+offset_02, extents_0(2)+offset_02,
     &                        1+offset_03, extents_0(3)+offset_03,
     &                        1+offset_04, extents_0(4)+offset_04,
     & 
     &                        1+offset_11, extents_1(1)+offset_11,
     &                        1+offset_12, extents_1(2)+offset_12,
     &                        1+offset_13, extents_1(3)+offset_13,
     &                        1+offset_14, extents_1(4)+offset_14,
     &                 
     &                        ncenters)
                         

      Return 
      end
C 
      subroutine compute_2particle_gradient(
     &
     &                             two_ints, two_dens, gradient,
     &                             flag, 
     &                             a01,b01,a02,b02,a03,b03,a04,b04,
     &                             a11,b11,a12,b12,a13,b13,a14,b14,
     &                             ncenters)

      Implicit none
     
      integer  ncenters, i
      integer a01,a02,a03,a04,b01,b02,b03,b04
      integer a11,a12,a13,a14,b11,b12,b13,b14
      integer iatom, component, icenter, match, igrad
      integer flag 

      double precision two_ints(a01:b01,a02:b02,a03:b03,a04:b04)
      double precision two_dens(a11:b11,a12:b12,a13:b13,a14:b14)
      double precision gradient(3*ncenters)
      double precision y
C
C Note that the dimension of the two arrays must match. I am not sure
C wether a consistency check here would be beneficial.

      do i = 1, 3*ncenters
         gradient(i) = 0.0D0
      enddo 

      do iatom = 1, ncenters

         if (flag .eq. 1) then
            component= 1 
            icenter  = 1
          elseif (flag .eq. 2) then
            component= 2
            icenter  = 1
          elseif (flag .eq. 3) then
            component= 3 
            icenter  = 1
          elseif (flag .eq. 4) then
            component= 1
            icenter  = 2 
          elseif (flag .eq. 5) then
            component= 2
            icenter  = 2
          elseif (flag .eq. 6) then
            component= 3
            icenter  = 2
          elseif (flag .eq. 7) then
            component= 1
            icenter  = 3
          elseif (flag .eq. 8) then
            component= 2
            icenter  = 3
          elseif (flag .eq. 9) then
            component= 3
            icenter  = 3
          elseif (flag .eq. 10) then
            component= 1
            icenter  = 4
          elseif (flag .eq. 11) then
            component= 2
            icenter  = 4
          elseif (flag .eq. 12) then
            component= 3
            icenter  = 4
          endif 

c-----------------------------------------------------------------------------
c   Process each atom's contribution to the gradient.
c-----------------------------------------------------------------------------

            call sum_atom_data(
     &                    iatom, two_ints, two_dens, 
     & 
     &                    a01,a02,a03,a04,b01,b02,b03,b04,
     &
     &                    icenter, match, y)

c-----------------------------------------------------------------------------
c   Sum the contribution into its proper component for this atom.
c-----------------------------------------------------------------------------

            igrad = (iatom-1)*3 + component
            gradient(igrad) = gradient(igrad) + y

      enddo

C#ifdef _DEBUG_LVL0
       Write(6,*)
       Write(6,"(a)") "The 2-electron gradient"
       write(6,"(3(F15.10))") (Gradient(i), i=1,3*ncenters)
C#endif
      Return
      End

      subroutine sum_atom_data(
     &
     &                    iatom, two_ints, two_dens,
     &
     &                    a1,b1,c1,d1,a2,b2,c2,d2,
     &
     &                    icenter, match, sum)

c---------------------------------------------------------------------------
c   Extracts the derivative integrals  and density data for a particular atom
c   into blocks.
c---------------------------------------------------------------------------
      use, intrinsic :: ISO_C_BINDING
      implicit none

      include 'sip_interface.f'
      include 'imemory.h'

      integer  ncenters
      integer a1,b1,c1,d1,a2,b2,c2,d2
      integer iatom, component, icenter, match

      double precision two_ints(a1:a2,b1:b2,c1:c2,d1:d2)
      double precision two_dens(a1:a2,b1:b2,c1:c2,d1:d2)

      double precision sum
      integer a, b, c, d

      integer (C_INT) one
      integer (C_INT) nshells

      TYPE(C_PTR) :: pnalpha, pend_nfps
      integer, pointer :: nalpha(:)
      integer, pointer :: end_nfps(:)

      TYPE(C_PTR) :: patom
      integer (C_INT), pointer :: atom(:)

      integer i
      logical atom_match
      logical aflag(a1:a2)
      logical bflag(b1:b2)
      logical cflag(c1:c2)
      logical dflag(d1:d2)

      sum   = 0.0D0
      match = 0
      one   = 1

c NSHELLS

       nshells = int_constant("nshells"//C_NULL_CHAR)

c END_NFPS

      call predefined_int_array("end_nfps"//C_NULL_CHAR, one,
     *                                   pnalpha, pend_nfps)
      call c_f_pointer(pnalpha, nalpha, [1])
      call c_f_pointer(pend_nfps, end_nfps, nalpha)

#ifdef _DEBUG_LVL0
      Write(6,*)
      Write(6,"(a,i4)") " The end_nfps length: ", nalpha(1)
      Write(6,"(a)") " The end_nfps array"
      Write(6,"(10i4)") (end_nfps(i), i=1,nalpha(1))
      write(*,*)
#endif 

      call predefined_int_array("atom"//C_NULL_CHAR, one,
     *                                   pnalpha, patom)
      call c_f_pointer(pnalpha, nalpha, [1])
      call c_f_pointer(patom, atom, nalpha)

#ifdef _DEBUG_LVL0
      Write(6,*)
      Write(6,"(a)") " map of atoms to shell"
      Write(6,"(10i4)") (atom(i), i=1,nalpha(1))
#endif 
C

      if (icenter .eq. 1) then
         do a = a1, a2
            aflag(a) = atom_match(a,iatom,nshells,end_nfps,atom,
     &                            nalpha(1))
            if (aflag(a)) match = match + 1
         enddo

         if (match .eq. 0) return

         do d = d1, d2
         do c = c1, c2
         do b = b1, b2
         do a = a1, a2
            if (aflag(a)) sum = sum + two_ints(a,b,c,d) * 
     &                                two_dens(a,b,c,d)
         enddo
         enddo
         enddo
         enddo

      else if (icenter .eq. 2) then
         do b = b1, b2
            bflag(b) = atom_match(b,iatom,nshells,end_nfps,atom,
     &                            nalpha(1))
            if (bflag(b)) match = match + 1
         enddo

         if (match .eq. 0) return

         do b = b1, b2
            if (bflag(b)) then
               do d = d1, d2
               do c = c1, c2
               do a = a1, a2
                  sum = sum + two_ints(a,b,c,d) * 
     &                        two_dens(a,b,c,d)
               enddo
               enddo
               enddo
            endif
         enddo

      else if (icenter .eq. 3) then
         do c = c1, c2
            cflag(c) = atom_match(c,iatom,nshells,end_nfps,atom,
     &                            nalpha(1))
            if (cflag(c)) match = match + 1
         enddo

         if (match .eq. 0) return

         do c = c1, c2
            if (cflag(c)) then
               do d = d1, d2
               do b = b1, b2
               do a = a1, a2
                  sum = sum + two_ints(a,b,c,d) * 
     &                        two_dens(a,b,c,d)
               enddo
               enddo
               enddo
            endif
         enddo

      else if (icenter .eq. 4) then

         do d = d1, d2
            dflag(d) = atom_match(d,iatom,nshells,end_nfps,atom,
     &                            nalpha(1))
            if (dflag(d)) match = match + 1
         enddo

        if (match .eq. 0) return

         do d = d1, d2
            if (dflag(d)) then
               do c = c1, c2
               do b = b1, b2
               do a = a1, a2
                  sum = sum + two_ints(a,b,c,d) * 
     &                        two_dens(a,b,c,d)
               enddo
               enddo
               enddo
            endif
         enddo
      endif

      return
      end

      logical function atom_match(ao_orbital,iatom,nshells,end_nfps,
     &                            atom,end_nfps_dim)
C
c--------------------------------------------------------------------------
c   Returns .true. if the ao_orbital comes from atom "iatom", .false.
c   otherwise.
c--------------------------------------------------------------------------
      implicit none

      integer ao_orbital, iatom
      integer m, end_nfps_dim
      integer nshells

      integer end_nfps(end_nfps_dim)
      integer atom(end_nfps_dim)

c---------------------------------------------------------------------------
c   Convert the AO orbital into a shell block.
c---------------------------------------------------------------------------

      call lookup_shell(end_nfps, nshells, ao_orbital, m)

c---------------------------------------------------------------------------
c   Check the atom corresponding to the shell block index for a match.
c---------------------------------------------------------------------------

      if (atom(m) .eq. iatom) then
         atom_match = .true.
      else
         atom_match = .false.
      endif
C
      return
      end

      subroutine lookup_shell(table, nshells, a, m)
c---------------------------------------------------------------------------
c   Converts basis function coordinates into shell block coordinates,
c   using a lookup table that relates the shell index to the number of
c   functions per shell.
c
c   Arguments:
c       table           Array containing the ending functions per shell.
c       nshells         Number of shells.
c       a               Function coordinate to be converted.
c       m               Return value for the shell block coordinate
c                       corresponding to a.
c---------------------------------------------------------------------------

      implicit none
      integer nshells
      integer table(nshells)
      integer i, a, m, sum

      do i = 1, nshells
         if (table(i) .ge. a) then
            m = i
            return
         endif
      enddo

      m = nshells
      return
      end

