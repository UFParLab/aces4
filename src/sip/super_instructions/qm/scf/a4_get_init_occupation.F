

C-----------------------------------------------------------------------
!>
!> special a4_get_init_occupation wr
!>
!> EXECUTE A4_GET_INIT_OCCUPATION A_OCC_NUMS ZERO
!>
!> Obitain an inital orbital occupation vector (used in SCF codes).
!> Currently support two options: 1. read from a external formatted 
!> file. 2. based simple energy ordering. Other possible option 
!> that need to considered is orbital smear implemented in MOLCAS, ADF 
!> programs. Ajith Perera, 04/2015. 
C-----------------------------------------------------------------------

      subroutine A4_get_init_occupation(
     * array_0, rank_0, index_values_0, size_0, extents_0, data_0,
     * array_1, rank_1, index_values_1, size_1, extents_1, data_1,
     * ierr) BIND(C)

      use, intrinsic :: ISO_C_BINDING
      implicit none

      integer (C_INT) nalpha_occupied, nbeta_occupied

      integer pa1

      include 'sip_interface.f'
      include 'imemory.h'

C The output array will return the occupation numbers. 

      integer(C_INT), intent(in)::array_0
      integer(C_INT), intent(in)::rank_0
      integer(C_INT), intent(in)::index_values_0(1:rank_0)
      integer(C_INT), intent(in)::size_0
      integer(C_INT), intent(in)::extents_0(1:rank_0)
      real(C_DOUBLE), intent(out)::data_0(1:size_0)

C The input scalar track the reference (RHF=0, UHF=1, ROHF =3)

      integer(C_INT), intent(in)::array_1
      integer(C_INT), intent(in)::rank_1
      integer(C_INT), intent(in)::index_values_1(1:rank_1)
      integer(C_INT), intent(in)::size_1
      integer(C_INT), intent(in)::extents_1(1:rank_1)
      real(C_DOUBLE), intent(in)::data_1(1:size_1)

      integer(C_INT), intent(out)::ierr
      real(C_DOUBLE) :: uhf

      nalpha_occupied = int_constant("nalpha_occupied"//C_NULL_CHAR)
      nbeta_occupied  = int_constant("nbeta_occupied"//C_NULL_CHAR)

      pa1 = 1
      uhf = data_1(1)
      call built_occupation(data_0, uhf, pa1, extents_0(1),
     &                      nalpha_occupied, nbeta_occupied)
      ierr = 0

      return
      end
C-------------------------------------------------------------------------

      subroutine built_occupation(xxx,uhf,pa1,pa2,nalpha_occupied,
     &                            nbeta_occupied)

      implicit none
C
      integer pa1,pa2
      integer itmp1(2),Itmp2(2),iuhf,iunit,i
      integer nalpha_occupied, nbeta_occupied
      double precision uhf

      logical OCCNUMS_FILE
      character*80 BLANK
      double precision xxx(pa1:pa2)
      double precision DOCC_NUM
C
C Check whether OCCNUM file exist. If it is read the file as is done 
C in the serial code (but no symmetry)
C
      INQUIRE(FILE="OCCNUMS", EXIST=OCCNUMS_FILE)
      IUNIT = 5
      IUHF  = INT(UHF)

      IF (OCCNUMS_FILE) THEN

#ifdef _DEBUG_LVL0
C      if (me .eq. master) then
      write(*,*)
      write(*,"(a)") "The occupation number file is present"
      write(*,"(a)") "The occupations are read from OCCNUMS file"
      write(*,*)
C      endif
#endif 
         OPEN(UNIT=IUNIT, FILE="OCCNUMS", FORM="FORMATTED")
         READ(IUNIT,10,END=19) Itmp1(1)
         READ(IUNIT,10,END=19) Itmp1(2)

         READ(IUNIT, "(80a)") Blank
        
         READ(IUNIT,10,END=19) Itmp2(1)
         READ(IUNIT,10,END=19) Itmp2(2)

         READ(IUNIT, "(80a)") Blank

         IF (IUHF .EQ. 0) THEN
            DO I = 1, Itmp2(1)
               READ(IUNIT, "(F4.2)",END=19) DOCC_NUM
               XXX(i) = DOCC_NUM 
            ENDDO 

         ELSE

            DO I = 1, Itmp2(1)
               READ(IUNIT, "(F4.2)",END=19) DOCC_NUM
            ENDDO 

            READ(IUNIT, "(80a)") Blank

            DO I =1, Itmp2(2)
               READ(IUNIT, "(F4.2)",END=19) DOCC_NUM
               XXX(i) = DOCC_NUM
            ENDDO 

         ENDIF 

      ELSE
           
         IF (IUHF .EQ. 0) THEN

            DO I = 1, nalpha_occupied
               XXX(i) = 1.0D0
            ENDDO 

         ELSE 
           
            DO I = 1, nbeta_occupied
               XXX(i) = 1.0D0
            ENDDO 

         ENDIF

      ENDIF 

 10   FORMAT(16I5)
 19   CLOSE(IUNIT)

#ifdef _DEBUG_LVL0
C      if (me .eq. master) then
      write(*,*)
      If (IUHF .EQ. 0) THEN
      write(*,"(a)") "The Alpha occupation vector from init_occupy.F"
      write(*,*)
      Write(*,"(6(1x,F4.1))") (xxx(i),i=1,nalpha_occupied)
      Write(*,*)
      Else
      write(*,"(a)") "The Beta occupation vector from init_occupy.F"
      write(*,*)
      Write(*,"(6(1x,F4.1))") (xxx(i),i=1,nbeta_occupied)
      Write(*,*)
      Endif 
C      endif
#endif

      return
      end
