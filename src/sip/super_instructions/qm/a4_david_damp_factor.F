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
      subroutine A4_David_damp_factor(
     * array_0, rank_0, index_values_0, size_0, extents_0, data_0,
     * array_1, rank_1, index_values_1, size_1, extents_1, data_1,
     * ierr) BIND(C)

      use, intrinsic :: ISO_C_BINDING
      implicit none

C-----------------------------------------------------------------------
C Compute the Davidson dynamic damping factor. Used in SCF codes.
C Read the comments found in the calling SIAL code for further 
C details.
C-----------------------------------------------------------------------

      include 'sip_interface.h'

C The input array contains the damp parameters computed in the SIAL code is  
C (only 3 elements). 

      integer(C_INT), intent(in)::array_0
      integer(C_INT), intent(in)::rank_0
      integer(C_INT), intent(in)::index_values_0(1:rank_0)
      integer(C_INT), intent(in)::size_0
      integer(C_INT), intent(in)::extents_0(1:rank_0)
      real(C_DOUBLE), intent(in)::data_0(1:size_0)

C The output is the damp factor (a scalar) 

      integer(C_INT), intent(in)::array_1
      integer(C_INT), intent(in)::rank_1
      integer(C_INT), intent(in)::index_values_1(1:rank_1)
      integer(C_INT), intent(in)::size_1
      integer(C_INT), intent(in)::extents_1(1:rank_1)
      real(C_DOUBLE), intent(out)::data_1(1:size_1)

      integer(C_INT), intent(out)::ierr

c---------------------------------------------------------------------------

      call damp_factor(data0, data1, 1, extents_0(1), 1, extents_1(1))

      return
      end
C-------------------------------------------------------------------------

      subroutine damp_factor(xxx, damp, 1, pa1, 1, pa2)

      implicit none
C
      integer pa1,pa2
      double precision xxx(1:pa1)

      double precision DMPMAX, ZERO, PT25, PT5, TWO, FOUR, FAC, PT2
      double precision DE, DEP, DEAVG, DAMP

      DATA DMPMAX /256.0D+00/
      DATA ZERO,PT25,PT5,TWO,FOUR,FAC /0.0D+00,2.5D-01,0.5D+00,
     1 2.0D+00,4.0D+00,1.6D+01/

C#ifdef _DEBUG_LVL0
      if (me .eq. master) then
      write(6,"(a)") "Checking the input arrays"
      Write(*,"(a,1x,i4)") "The input dimensions: ", pa1
      write(6,"(4(1x,F10.5))") (XXX(i),i=1,pa1), DAMP
      endif 
C#endif 

      DE    = XXX(1)
      DEP   = XXX(2)
      DEAVG = XXX(3)

      IF( DE.GT.ZERO) GO TO 400
      IF(DEP.GT.ZERO) GO TO 300
      IF( DE.GT. DEP) GO TO 200
C
C     ----- DE < 0. , DEP < 0. , DE < DEP -----
C
      IF( ABS(DE).LT.TWO*DEAVG) GO TO 110
      DAMP=FAC* DMAX1(DAMP,DEAVG)
      GO TO 600
  110 IF( ABS(DE).GT.PT5*DEAVG) GO TO 600
      DAMP=DAMP/FAC
      GO TO 600
  200 CONTINUE
C
C     ----- DE < 0. , DEP < 0. , DE > DEP -----
C
      IF(DE.GT.PT25*DEP) GO TO 210
      DAMP=(DE/DEP)**2* DMAX1(DAMP,DEAVG)
      GO TO 600
  210 DAMP=DAMP/FAC
      GO TO 600
  300 CONTINUE
C
C     ----- DE < 0. , DEP > 0. -----
C
      DAMP=FOUR* DMAX1(DAMP,DEAVG)
      IF(-DE.GT.DEAVG) DAMP=DAMP*FAC
      IF(-DE+DEP.GE.DEAVG) GO TO 600
      DAMP=DAMP/FAC
      GO TO 600
  400 CONTINUE
      IF(DEP.GT.ZERO) GO TO 500
C
C     ----- DE > 0. , DEP < 0. -----
C
      DAMP=FOUR* DMAX1(DAMP,DEAVG)
      IF(DE.GT.PT5*DEAVG) DAMP=DAMP*FAC
      IF(DE-DEP.GE.PT2*DEAVG) GO TO 600
      DAMP=DAMP/FAC
      GO TO 600
  500 CONTINUE
C
C     ----- DE > 0. , DEP > 0. -----
C
      DAMP=FOUR* DMAX1(DAMP,DEAVG)
      IF(DE.LT.FOUR*DEP) GO TO 510
      DAMP=FAC* DMAX1(DAMP,DEAVG)
      GO TO 600
  510 IF(DE.GT.PT25*DEP) GO TO 520
      DAMP=DAMP/FAC
      GO TO 600
  520 DAMP=(DE/DEP)**2* DMAX1(DAMP,DEAVG)
C
C     ----- DO NOT LET -DAMP- GO OUT OF CONTROL -----
C
  600 CONTINUE
      DAMP=DMIN1(DAMP,DMPMAX)
   
C#ifdef _DEBUG_LVL0
      if (me .eq. master) then
      write(*,"(a,1x, F15.10)") "The damp factor", damp
      endif
C#endif

      return
      end
