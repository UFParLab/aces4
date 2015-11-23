
c This takes an NROWxNCOL matrix and reorders the rows and/or columns.
c   op=0    reorder the columns
c   op=1    reorder the rows
c   op=2    reorder both the rows and columns
c If sorting both rows and columns, the matrix must be square.
c
c scr      is a vector of the appropriate length to hold 1 column or 1 row
c          (whichever is being sorted)
c order    is an integer array telling what order to put the columns or rows in
c scrorder is a similar length integer vector used for scratch space
c
c If we are sorting the columns of a Nx4 matrix [ C1 C2 C3 C4 ], and order
c is [ 2 3 4 1 ] and reverse is 0, then the matrix is reordered to be
c [ C4 C1 C2 C3 ].  In other words, column 1 is put in the 2nd columns
c position, column 2 is moved to column 3, etc.  If reverse is 1, the
c reverse reorder is done, so that if order is [ 2 3 4 1 ] and the matrix
c [ C4 C1 C2 C3 ] is passed in, it unreorders it and returns [ C1 C2 C3 C4].

      subroutine mat_reorder(op,reverse,nrow,ncol,matrix,scr,scrorder,
     &                       order)
      implicit none

      integer op,reverse,nrow,ncol,order(*),scrorder(*)
      double precision matrix(nrow,ncol),scr(*)

      integer ito,ifrom,irow,icol

      if (op.eq.2 .and. nrow.ne.ncol) then
         write(*,*) '@MAT_REORDER: fatal error, nrows ne ncols.'
         stop
      end if

c Sort columns
      if (op.eq.0 .or. op.eq.2) then
         do icol=1,ncol
            scrorder(icol)=order(icol)
         end do
         do 30 icol=1,ncol
         if (scrorder(icol).eq.0) goto 30

c           Normal (slow) reorder
            if (reverse.eq.0) then
               do while (scrorder(icol).ne.0)
                  ito=scrorder(icol)
                  call dcopy(nrow,matrix(1,icol),1,scr,1)
                  call dcopy(nrow,matrix(1,ito),1,matrix(1,icol),1)
                  call dcopy(nrow,scr,1,matrix(1,ito),1)
                  scrorder(icol)=scrorder(ito)
                  scrorder(ito)=0
               end do

c           Reverse (quick) reorder
            else
               call dcopy(nrow,matrix(1,icol),1,scr,1)
               ito=icol
   20          continue
               ifrom=scrorder(ito)
               if (ifrom.eq.icol) then
                  call dcopy(nrow,scr,1,matrix(1,ito),1)
                  scrorder(ito)=0
               else
                  call dcopy(nrow,matrix(1,ifrom),1,matrix(1,ito),1)
                  scrorder(ito)=0
                  ito=ifrom
                  goto 20
               end if
            end if

   30    continue
      end if

c Sort rows (very slow)
      if (op.eq.1 .or. op.eq.2) then
         do irow=1,nrow
            scrorder(irow)=order(irow)
         end do
         do 60 irow=1,nrow
         if (scrorder(irow).eq.0) goto 60

c           Normal (slow) reorder
            if (reverse.eq.0) then
               do while (scrorder(irow).ne.0)
                  ito=scrorder(irow)
                  do icol=1,ncol
                     scr(icol)=matrix(irow,icol)
                     matrix(irow,icol)=matrix(ito,icol)
                     matrix(ito,icol)=scr(icol)
                  end do
                  scrorder(irow)=scrorder(ito)
                  scrorder(ito)=0
               end do

c           Reverse (quick) reorder
            else
               do icol=1,ncol
                  scr(icol)=matrix(irow,icol)
               end do
               ito=irow
   50          continue
               ifrom=scrorder(ito)
               if (ifrom.eq.irow) then
                  do icol=1,ncol
                     matrix(ito,icol)=scr(icol)
                  end do
                  scrorder(ito)=0
               else
                  do icol=1,ncol
                     matrix(ito,icol)=matrix(ifrom,icol)
                  end do
                  scrorder(ito)=0
                  ito=ifrom
                  goto 50
               end if
            end if

   60    continue
      end if

      return
      end

