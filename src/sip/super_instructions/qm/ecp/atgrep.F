      subroutine atgrep(atosym,nuchar)

c
c     --- detects atomsymbol from nuclear charge ---
c
c     --- note that iodine is abbreviated by "i" and not by "j" 
c
      implicit double precision (a-h,o-z)
      character*2 atosym,nuc(95)
      dimension chrg(95)

c
      data nuc /'H ','HE','LI','BE','B ','C ','N ','O ','F ','NE',
     1          'NA','MG','AL','SI','P ','S ','CL','AR',
     2          'K ','CA','SC','TI','V ','CR','MN','FE','CO','NI',
     3          'CU','ZN','GA','GE','AS','SE','BR','KR',
     4          'RB','SR','Y ','ZR','NB','MO','TC','RU','RH','PD',
     5          'AG','CD','IN','SN','SB','TE','I ','XE',
     6          'CS','BA','LA','CE','PR','ND','PM','SM','EU','GD',
     7          'TB','DY','HO','ER','TM','YB','LU','HF','TA','W ',
     8          'RE','OS','IR','PT','AU','HG','TL','PB','BI','PO',
     9          'AT','RN','FR','RA','AC','TH','PA','U ','NP','PU','Q '/
      data chrg /1.D0, 2.D0, 3.D0, 4.D0, 5.D0, 6.D0, 7.D0, 8.D0, 9.D0,
     1           10.D0, 11.D0, 12.D0, 13.D0, 14.D0, 15.D0, 16.D0, 
     2           17.D0, 18.D0, 19.D0, 20.D0, 21.D0, 22.D0, 23.D0,
     3           24.D0, 25.D0, 26.D0, 27.D0, 28.D0, 29.D0, 30.D0,
     4           31.D0, 32.D0, 33.D0, 34.D0, 35.D0, 36.D0, 37.D0, 
     5           38.D0, 39.D0, 40.D0, 41.D0, 42.D0, 43.D0, 44.D0,
     6           45.D0, 46.D0, 47.D0, 48.D0, 49.D0, 50.D0, 51.D0,
     7           52.D0, 53.D0, 54.D0, 55.D0, 56.D0, 57.D0, 58.D0,
     8           59.D0, 60.D0, 61.D0, 62.D0, 63.D0, 64.D0, 65.D0,
     9           66.D0, 67.D0, 68.D0, 69.D0, 70.D0, 71.D0, 72.D0,
     &           73.D0, 74.D0, 75.D0, 76.D0, 77.D0, 78.D0, 79.D0,
     &           80.D0, 81.D0, 82.D0, 83.D0, 84.D0, 85.D0, 86.D0,
     &           87.D0, 88.D0, 89.D0, 90.D0, 91.D0, 92.D0, 93.D0,
     &           94.D0, 95.D0/

c
c
      do 100 i=1,95
        if (nuchar.eq.chrg(i)) then
          atosym=nuc(i)
          goto 200
        endif
  100 continue
  200 continue
      return
      end
