
C-----------------------------------------------------------------
!> special a4_dscale urr
!>
!> This SIP does something closer to the serial blas routine dscale does.
!> That C is A = C(i) A(*,i)  where A is a vector and C is scalar array.
!> Primarily used in scf_rhf and scf_uhf.
!> This must be invoked from the SIALX as
!> execute  A4_dscale A(a,b) B(a) C where A is a anytype of blocked 
!> array, B is static and array and C is 0.0 or 1.0 indicating 
!> RHF or UHF respectively. Ajith Perera, 05/2015.
C-----------------------------------------------------------------

      subroutine A4_dscale(
     & array_0, rank_0, index_values_0, size_0, extents_0, data_0,
     & array_1, rank_1, index_values_1, size_1, extents_1, data_1,
     & array_2, rank_2, index_values_2, size_2, extents_2, data_2,
     & ierr) BIND(C)
      use, intrinsic :: ISO_C_BINDING

      implicit none

      Integer na1, na2, nb1, i, one
      Character*5 type


      include 'sip_interface.f'

C input/output array 

      integer(C_INT), intent(in)::array_0
      integer(C_INT), intent(in)::rank_0
      integer(C_INT), intent(in)::index_values_0(1:rank_0)
      integer(C_INT), intent(in)::size_0
      integer(C_INT), intent(in)::extents_0(1:rank_0)
      real(C_DOUBLE), intent(out)::data_0(1:size_0)

C The input scalar array.

      integer(C_INT), intent(in)::array_1
      integer(C_INT), intent(in)::rank_1
      integer(C_INT), intent(in)::index_values_1(1:rank_1)
      integer(C_INT), intent(in)::size_1
      integer(C_INT), intent(in)::extents_1(1:rank_1)
      real(C_DOUBLE), intent(in)::data_1(1:size_1)

C A constant to indacate the spin propeeties of the
C incomming aray.

      integer(C_INT), intent(in)::array_2
      integer(C_INT), intent(in)::rank_2
      integer(C_INT), intent(in)::index_values_2(1:rank_2)
      integer(C_INT), intent(in)::size_2
      integer(C_INT), intent(in)::extents_2(1:rank_2)
      real(C_DOUBLE), intent(in)::data_2(1:size_2)

      integer(C_INT), intent(out)::ierr

      TYPE(C_PTR) C_nao_segs,  C_aosegs
      TYPE(C_PTR) C_nmoa_segs, C_moasegs
      TYPE(C_PTR) C_nmob_segs, C_mobsegs
      integer, pointer :: F_nao_segs(:),  F_naosegs(:)
      integer, pointer :: F_nmoa_segs(:), F_nmoasegs(:)
      integer, pointer :: F_nmob_segs(:), F_nmobsegs(:)
      integer(C_INT) Iflag
      integer(C_INT) offset_1, offset_2 

c Get the ao segment ranges

      call predefined_int_array("ao_seg_ranges"//C_NULL_CHAR, one,
     &                           C_nao_segs, C_aosegs)
      call c_f_pointer(C_nao_segs,F_nao_segs,[1])
      call c_f_pointer(C_aosegs,F_naosegs,F_nao_segs)

C Get the MO alpha/beta segment ranges 

      Iflag = INT(data_2(1))

      if (Iflag .eq. 0) then
          type = "Alpha"
      else if (Iflag .eq. 1) then
          type = "Beta "
      endif 

      if (type .eq. "Alpha") then

          call predefined_int_array("moa_seg_ranges"//C_NULL_CHAR, one,
     &                               C_nmoa_segs, C_moasegs)
          call c_f_pointer(C_nmoa_segs,F_nmoa_segs,[1])
          call c_f_pointer(C_moasegs,F_nmoasegs,F_nmoa_segs)

      elseif (type .eq. "Beta ") then

          call predefined_int_array("mob_seg_ranges"//C_NULL_CHAR, one,
     &                               C_nmob_segs, C_mobsegs)
          call c_f_pointer(C_nmob_segs,F_nmob_segs,[1])
          call c_f_pointer(C_mobsegs,F_nmobsegs,F_nmob_segs)

      endif 

      na1 = 1
      na2 = 1
      nb1 = 1

      if (rank_0 .eq. 2) then

         offset_1 = 0
         offset_2 = 0

         if (index_values_0(1).gt.1) then
             do i = 1, index_values_0(1) - 1
                offset_1 = offset_1 + F_naosegs(i)
             enddo
         endif

         if (type .eq. "Alpha") then
            if (index_values_0(2).gt.1) then
               do i = 1, index_values_0(2) - 1
                  offset_2 = offset_2 + F_nmoasegs(i)
                  Write(*,*) "A segs", F_nmoasegs(i)
               enddo
             endif
         elseif (type .eq. "Beta ") then
            if (index_values_0(2).gt.1) then
               do i = 1, index_values_0(2) - 1
                  offset_2 = offset_2 + F_nmobsegs(i)
                  Write(*,*) "B segs", F_nmobsegs(i)
               enddo
             endif
         endif 

      else 

         Write(6,"(a,a)") " The rank of the array must be 2 in", 
     &                    " supper instruction A4_dscale.F" 
         call abort_job()

      endif 

      call mult_212(data_0, data_1, na1, extents_0(1), offset_1,
     *              na2, extents_0(2), offset_2, nb1, extents_1(1))

      ierr = 0

      return
      end
C-------------------------------------------------------------------------

      subroutine mult_212(xxx, yyy, ba1, ea1, off_1, ba2, ea2,
     &                    off_2, bb1, eb1)

      implicit none

      integer ba1,ea1,ba2,ea2,bb1,eb1
      integer off_1, off_2
      integer i, j

      double precision xxx(ba1:ea1,ba2:ea2)
      double precision yyy(bb1:eb1)
      double precision z
C
#ifdef _DEBUG_LVL0
C      if (me .eq. master) then
      write(6,"(a)") "Checking the input arrays"
      Write(6,*) "The input dimensions"
      Write(*,"(7I4)") ba1,ea1,ba2,ea2,bb1,eb1,off_1, off_2
      Write(*,"(6(1x,F10.7))") ((xxx(i,j),i=ba1,ea1),j=ba2,ea2)
      write(*,*)
      write(*,"(a)") "The occupation vector"
      Write(*,"(6(1x,F3.1))") (yyy(i),i=bb1,eb1)
      write(6,"(a)") "End checking input array"
C      endif 
#endif 
   
      Do j = ba2, ea2
         Z = yyy(j+off_2) 
         Do i = ba1, ea1
            xxx(i,j) = xxx(i,j) * Z
         Enddo 

      Enddo

#ifdef _DEBUG_LVL0
C      if (me .eq. master) then
      write(*,*)
      write(*,"(a)") "The scalled array"
      Write(*,"(6(1x,F10.7))") ((xxx(i,j),i=ba1,ea1),j=ba2,ea2)
      Write(*,*)
C      endif
#endif

      return
      end
