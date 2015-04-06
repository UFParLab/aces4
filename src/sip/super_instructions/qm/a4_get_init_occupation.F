C  Copyright (c) 2003-2010 University of Florida
C
C  This program is free software; you can redistribute it and/or modify
C  it under the terms of the GNU General Public License as published by
C  the Free Software Foundation; either version 2 of the License, or
C  (at your option) any later version.

C  This program is distributed in the hope that it will be useful,
C  but WITHOUT ANY WARRANTY; without even the implied warranty of
C  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
C  GNU General Public License for more details.

C  The GNU General Public License is included in this distribution
C  in the file COPYRIGHT.

      subroutine A4_get_init_occupation(
     * array_0, rank_0, index_values_0, size_0, extents_0, data_0,
     * array_1, rank_1, index_values_1, size_1, extents_1, data_1,
     * ierr) BIND(C)

      use, intrinsic :: ISO_C_BINDING
      implicit none

C-----------------------------------------------------------------------
C Obitain an inital orbital occupation vector (used in SCF codes).
C Currently support two options: 1. read from a external formatted 
C file. 2. based simple energy ordering. Other possible option 
C that need to considered is orbital smear implemented in MOLCAS, ADF 
C programs. 
C-----------------------------------------------------------------------

      include 'sip_interface.h'

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
      real(C_DOUBLE), intent(out)::data_1(1:size_1)

      integer(C_INT), intent(in)::ierr

      call built_occupation(data0, data1, 1, extents_0(1), 1, 
     +                      extents_1(1))

      return
      end
C-------------------------------------------------------------------------

      subroutine built_occupation(xxx,uhf,1,pa1,1,pa2)

      implicit none

      include 'int_gen_parms.hf"
C
      integer pa1,pa2
      integer itmp1(2),Itmp2(2),uhf

      logical OCCNUMS_FILE
      character*80 BLANK
      double precision xxx(1:pa1)
      double precision DOCC_NUM
C
C Check whether OCCNUM file exist. If it is read the file as is done 
C in the serial code (but no symmetry)
C
      INQUIRE(FILE="OCCNUMS", EXIST=OCCNUMS_FILE)
      IUNIT = 5

      IF (OCCNUMS_FILE) THEN

C#ifdef _DEBUG_LVL0
      if (me .eq. master) then
      write(*,*)
      write(*,"(a)") "The occupation number file is present"
      write(*,"(a)") "The occupations are read from OCCNUMS file"
      write(*,*)
      endif
C#endif 
         OPEN(UNIT=IUNIT, FILE="OCCNUMS", FORM="FORMATTED")
         READ(IUNIT,10,END=19) Itmp1(1)
         READ(IUNIT,10,END=19) Itmp1(2)

         READ(IUNIT, "(80a)") Blank
        
         READ(IUNIT,10,END=19) Itmp2(1)
         READ(IUNIT,10,END=19) Itmp2(2)

         READ(IUNIT, "(80a)") Blank

         IF (UHF .EQ. 0) THEN
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
           
         IF (UHF .EQ. 0) THEN

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

C#ifdef _DEBUG_LVL0
      if (me .eq. master) then
      write(*,*)
      If (UHF .EQ. 0) THEN
      write(*,"(a)") "The Alpha occupation vector from init_occupy.F"
      Else
      write(*,"(a)") "The Beta occupation vector from init_occupy.F"
      Endif 
      write(*,*)
      Write(*,"(6(1x,F3.1))") (xxx(i),i=1,pa1)
      Write(*,*)
      endif
C#endif

      return
      end
