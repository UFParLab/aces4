
C-----------------------------------------------------------------------
!>
!> special a4_return_occupation rw
!>
!> EXECUTE  A4_RETURN_OCCUPATION ORB_OVLP_A A_OCC_NUMS
!>
!> The input array contains the overlap of the current iteration's 
!> MO vectos with the previous iteration's occupied MOs. In this
!> SIP, the the current MOs that shows the maximum overlap with the previous
!> iteration is occupied. Hope is that this will improve convegence
!> by loocking into an occupation. Ajith Perera, 04/2015.
C-----------------------------------------------------------------------

      subroutine A4_return_occupation(
     * array_0, rank_0, index_values_0, size_0, extents_0, data_0,
     * array_1, rank_1, index_values_1, size_1, extents_1, data_1,
     * ierr) BIND(C)

      use, intrinsic :: ISO_C_BINDING
      implicit none

      include 'sip_interface.f'

C The output array will return the occupation numbers.

      integer(C_INT), intent(in)::array_0
      integer(C_INT), intent(in)::rank_0
      integer(C_INT), intent(in)::index_values_0(1:rank_0)
      integer(C_INT), intent(in)::size_0
      integer(C_INT), intent(in)::extents_0(1:rank_0)
      real(C_DOUBLE), intent(in)::data_0(1:size_0)

C The output vector returns the occupation that shows the
C maximum overalp with the previous 

      integer(C_INT), intent(in)::array_1
      integer(C_INT), intent(in)::rank_1
      integer(C_INT), intent(in)::index_values_1(1:rank_1)
      integer(C_INT), intent(in)::size_1
      integer(C_INT), intent(in)::extents_1(1:rank_1)
      real(C_DOUBLE), intent(out)::data_1(1:size_1)

      integer(C_INT), intent(out)::ierr

      TYPE(C_PTR) :: n_ao_segs, aosegs
      integer, pointer :: pn_ao_segs(:), paosegs(:)
      integer(C_INT) offset_1, offset_2
      integer(C_INT) start_1, start_2
      integer(C_INT) i, i1, i2, one

c Get the mo segment ranges
      call predefined_int_array("moa_seg_ranges"//C_NULL_CHAR, one,
     *                           n_ao_segs, aosegs)
      call c_f_pointer(n_ao_segs,pn_ao_segs,[1])
      call c_f_pointer(aosegs,paosegs,pn_ao_segs)

      ierr = 0

      if (rank_0 .eq. 2) then

c Find the offset index ranges

      offset_1 = 0
      offset_2 = 0
      if (index_values_0(1).gt.1) then
          do i = 1, index_values_0(1) - 1
             offset_1 = offset_1 + paosegs(i)
          enddo
      endif
      if (index_values_0(2).gt.1) then
          do i = 1, index_values_0(2) - 1
             offset_2 = offset_2 + paosegs(i)
          enddo
      endif

      call get_occupation(1, extents_0(1), offset_1,
     *                    1, extents_0(2), offset_2,
     *                    data_0,
     *                    1, extents_1(1),
     *                    data_1)

      else

         Write(*,"(1x,a,1x,a,1x,i2)") 
     *   "The rank of the argument to", 
     *   "A4_return_occupation.F must be 2 instead of", 
     *    rank_0
          call abort_job ()

      endif 

      return
      end
C-------------------------------------------------------------------------

      subroutine get_occupation(a1, a2, offset_1, b1, b2, offset_2,
     *                          array1, c1, c2, array2)

      implicit none

      integer a1,a2,b1,b2
      integer c1,c2,offset_1,offset_2
      integer a,b,c
      double precision array1(a1:a2,b1:b2)
      double precision array2(c1:c2)

      double precision z
C
C#ifdef _DEBUG_LVL0
C      if (me .eq. master) then
      Write(*,"(a,(1x,4i4))") "The input ranges", a1, a2, b1, b2
      Write(*,"(a,(1x,2i4))") "The offsets: ", offset_1, offset_2
C      write(6,"(a)") "Checking the overlap map"
C      call output(array1,1,a1,1,a2,a1,a2,1)
C      write(6,"(a)") "End checking the overlap map"
C      endif 
C#endif 
C
C Determine the raw index of the corresponding to the maximum overlap
C (absoulte sense) in a column. The orbital corrsponding to that row is
C occupied. 
   
      Do b = b1, b2

         Z = 0.0D0

         Do a = a1, a2
            Z = MAX(Z, Dabs(array1(a+offset_1,b+offset_2)))
         Enddo 

C#ifdef _DEBUG_LVL0
C      if (me .eq. master) then
      Write(*,"(a,I4,1x,a,F12.8)") "The largest of column", a, "is", Z
      Write(*,*)
C      endif 
C#endif 
         Do c = c1, c2
            If (Z .EQ. Dabs(array1(c+offset_1,b+offset_2))) 
     &                     array2(b) = 1.0D0
         Enddo

C#ifdef _DEBUG_LVL0
C      if (me .eq. master) then
      write(*,"(a)") "The new occupation vector"
      Write(*,"(6(1x,F4.1))") (array2(c),c=c1,c2)
      Write(*,*)
C      endif
C#endif

      Enddo

      return
      end
