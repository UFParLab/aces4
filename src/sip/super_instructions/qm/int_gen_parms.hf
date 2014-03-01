c--------------------------------------------------------------------------
c   Flag indicating if the SCF has been performed.
c--------------------------------------------------------------------------

      logical if_scf
      logical init_scf

c--------------------------------------------------------------------------
c   Symmetry restrictions.
c--------------------------------------------------------------------------

      logical restrict_mn
      logical restrict_rs
      logical restrict_mn_rs

      parameter (restrict_mn=.false.)
      parameter (restrict_rs=.false.)
      parameter (restrict_mn_rs=.false.)

c--------------------------------------------------------------------------
c   Shell size restrictions
c--------------------------------------------------------------------------

      integer max_centers
      integer max_shells
      
      parameter (max_centers = 300)
      parameter (max_shells = 5000)

c--------------------------------------------------------------------------
c   Integral packages supported.
c--------------------------------------------------------------------------

      integer random_package
      integer flocke_package
      integer gamess_package
      integer gamess_derivative_package
      parameter (random_package = 1)
      parameter (flocke_package = 2)
      parameter (gamess_package = 3)
      parameter (gamess_derivative_package = 4)
 
c--------------------------------------------------------------------------
c   Contraction worker and integral worker types.
c--------------------------------------------------------------------------

      integer sip_contraction_worker
      integer simple_contraction_worker
      integer lotrich_contraction_worker
      integer lotrich_type2_contraction_worker
      integer direct_integral_worker
      integer ram_based_integral_worker
      parameter (simple_contraction_worker = 1)
      parameter (lotrich_contraction_worker = 2)
      parameter (direct_integral_worker = 3)
      parameter (ram_based_integral_worker = 4)
      parameter (lotrich_type2_contraction_worker = 5)
      parameter (sip_contraction_worker = 6)

c--------------------------------------------------------------------------
c   int_gen_com common block
c-------------------------------------------------------------------------- 

c ivAngMom(i) : angular momentum of shell i
c nFpS(i) : number of contracted functions in shell i
c ivShlOff(i) : absolute offset of shell i from the 0 block
c nShells : total number of angular momentum shells
c nContAOs : total number of contracted functions

      integer ivAngMom(max_shells),
     &        nFpS(max_shells), ivShlOff(max_shells), 
     &        end_nfps(max_shells), ixalpha(max_shells),
     &        ixpcoef(max_shells), indx_cc(max_shells),
     &        nShells, nContAOs, nCenters, nSpC(max_centers), 
     &        atom(max_shells),
     &        nCFpS(max_shells), nPFpS(max_shells), 
     &        end_shell_vmol(max_shells),
     &        ixshells(max_shells),
     &        iSpherical, maxbuf, max_mgrbuf, i0mgr, 
     &        nint_blk, fdscr, blksize, nblks_disk, last
      integer nsh_vmol
      integer win1, win2, win3
      integer noccupied, nalpha_occupied, nbeta_occupied
      integer nvirtual, nalpha_virtual, nbeta_virtual
      integer sip_blocksize, sip_mx_ao_segsize,
     *        sip_mx_occ_segsize, sip_mx_virt_segsize
      integer jobarc64
      integer intmax, zmax

      integer nscfa, nscfb, nepsa, nepsb

      integer worker_total, mgr_total, mgr_records
      integer iv
      integer iuhf
      integer ni, nj, na, nb, nc, nd 
      integer maxa, maxb, imax, jmax, cmax, dmax
      integer intpkg, nalloc_pkg, itfct, nbasis, naobasis
      integer basis_request, scf_request 
      integer lfile
      integer iworker_type, cworker_type
      integer workers_per_manager
      integer io_company_id
      integer maxmem
      integer scf_hist, scf_iter, scf_beg
      integer cc_hist, cc_iter, cc_beg
      integer stack_algorithm_type
      integer stacksize, nworkthread
      integer algorithm_flag
      character*100 local_path   ! directory path for local disk storage
      character*20  local_disk   ! file name for local disk storage.
      character*120 fname        ! location for generated scratch file name.
      character*120 molfile      ! MOL file for basis info.
      character*120 sial_filename ! SIAL source code file
      character*120 outfile_name ! SIAL output filename
      character*120 aat_database

      integer*8 memptr, i0wrk, ithread
      integer*8 ipkgscr, dpkgscr, icoord, master_icoord, ialpha, ipcoeff
      integer*8 ierdind, icc_beg, icc_end, iscale_fac
      integer*8 iscfa, iscfb, ifocka, ifockb, ifockrohfa, ifockrohfb
      integer*8 iepsa, iepsb
      integer*8 iccbeg, iccend
      integer*8 i1e_sint, i1e_hint
      double precision scf_energy, totenerg
      double precision damp_init, cc_conv, scf_conv

      double precision excite, eom_tol, eom_roots ! Watson Added
      double precision reg,stabvalue

      integer itrips, itripe
      integer ihess1, ihess2, jhess1, jhess2, subb, sube
      integer sip_sub_segsize, sip_sub_occ_segsize, 
     *        sip_sub_virt_segsize, sip_sub_ao_segsize
      logical jobarc_exists, geom_opt, vib_freq_calc, vib_exact
      logical fast_erd_memcalc
      logical calc_2der_integrals

      common /int_gen_com/ memptr, i0wrk, iscfa, iscfb, iepsa, iepsb,
     &                     ifocka, ifockb,
     &                     ifockrohfa, ifockrohfb,
     &                     iccbeg, iccend, ipkgscr, dpkgscr, icoord, 
     &                     master_icoord, ialpha, ipcoeff, ithread,
     &                     ierdind, icc_beg, icc_end, iscale_fac,
     &                     i1e_sint, i1e_hint,
     &                     scf_energy, totenerg, damp_init, cc_conv, 
     &
     &                     excite, eom_tol, eom_roots, ! Watson Added
     &                     reg, stabvalue,              ! Taube Added ,
     &
     &                     scf_conv, itrips, itripe, 
     &                     charge(max_centers), acenter(max_centers,3),
     &                     ihess1, ihess2, jhess1, jhess2, subb, sube,
     &                     ivAngMom, nFpS, 
     &                     ivShlOff, nShells, nContAOs, nCenters, nSpC,
     &                     nsh_vmol, end_shell_vmol, ixshells,
     &                     atom,ixalpha, ixpcoef,indx_cc,
     &                     end_nfps, nCFpS, nPFpS,iSpherical,maxbuf,
     &                     max_mgrbuf, i0mgr, worker_total,
     &                     mgr_total, mgr_records,
     &                     local_disk, local_path, lfile,
     &                     fname, molfile, sial_filename,outfile_name,
     &                     aat_database,
     &                     maxa, maxb, cmax, dmax, imax, jmax, ni, nj, 
     &                     na, nb, nc, nd, jobarc64,
     &                     nscfa, nscfb, nepsa, nepsb,
     &                     nint_blk, fdscr, blksize, nblks_disk, 
     &                     win1, win2, iworker_type, cworker_type,
     &                     iv, win3,
     &                     managers_are_workers, master_is_worker,
     &                     noccupied, nalpha_occupied, nbeta_occupied,
     &                     nvirtual, nalpha_virtual, nbeta_virtual,
     &                     sip_blocksize, sip_mx_ao_segsize, 
     &                     sip_mx_occ_segsize, sip_mx_virt_segsize,
     &                     intpkg, iuhf,
     &                     nalloc_pkg, scf_request, algorithm_flag,
     &                     itfct, nbasis, naobasis, basis_request, 
     &                     intmax, zmax, workers_per_manager, 
     &                     maxmem, stack_algorithm_type,
     &                     cc_iter, cc_hist, cc_beg,
     &                     scf_iter, scf_hist, scf_beg, 
     &                     io_company_id, stacksize,
     &                     sip_sub_segsize, sip_sub_occ_segsize,
     &                     sip_sub_virt_segsize, sip_sub_ao_segsize,
     &                     nworkthread, jobarc_exists, geom_opt, 
     &                     vib_freq_calc, fast_erd_memcalc, vib_exact,
     &                     if_scf, calc_2der_integrals,
     &                     init_scf,
     &                     last

      logical managers_are_workers, master_is_worker
 
      double precision gradient_data
      logical compute_1e_integrals
      common /gradient_data/gradient_data(3*max_Centers),
     *                     compute_1e_integrals
                       
      integer max_procsx
      parameter (max_procsx = 10000)
      integer scfa_req, scfb_req, epsa_req, epsb_req 
      integer focka_req, fockb_req
      common /scf_requests/scfa_req(max_procsx), 
     *                     scfb_req(max_procsx),
     *                     epsa_req(max_procsx),
     *                     epsb_req(max_procsx),
     *                     focka_req(max_procsx),
     *                     fockb_req(max_procsx)
      save   /int_gen_com/

      double precision nngrad, charge, acenter
      common /NNgrad/nngrad(3,max_centers)
