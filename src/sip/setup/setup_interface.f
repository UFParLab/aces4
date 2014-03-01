!>  Interfaces for routines to support writing ACES initialization header and data files

!>  Usage:  include this file in the program
!>  Begin with a call to init_setup and end with a call to finalize_setup.  The
!>  other routines may be called in any order.  
!>
!>  @see setup_writer.h and setup_writer.cpp for C++ implementation

!>  @todo add time stamp consistency checking

!>  @warning  ISO Fortran 2003 standard is used For compatibility with C/C++.
!>      All strings  must be null terminated.  
!>      Example: call init_setup('jobf'//C_NULL_CHAR)
!>   
!>      Variables that will be passed as parameters should be declared with type
!>         integer(C_INT) or real(C_DOUBLE)
!>
!> These definitions are made available by including the following statement
!>  	use, intrinsic :: ISO_C_BINDING
    
!>  Initializes the setup routines. For job "jname", files name.h and name.dat will be generated
!>
!>  @param jname The null terminated name of the job.
      interface
      subroutine init_setup (jname) bind(C)
      character, dimension(*), intent(in) :: jname
      end subroutine init_setup
      end interface


!>  Sets the value of a predefined integer symbolic constant (norb, cciter, etc.)
!>  @param cname  The null terminated name of the constant
!>  @param value  The integer value
!>  Example:  call set_constant('norb'//C_NULL_CHAR, 32)
      interface
      subroutine set_constant(cname, value) bind(C)
          use, intrinsic :: ISO_C_BINDING
      character, dimension(*), intent(in):: cname
      integer (C_INT), value, intent(in)::value
      end subroutine set_constant
      end interface

!>  Sets the value of a predefined scalar symbolic constant
!>  @param cname  The null terminated name of the constant
!>  @param value  The double value
      interface
      subroutine set_scalar(cname, value) bind(C)
                use, intrinsic :: ISO_C_BINDING
      character, dimension(*), intent(in):: cname
      real(C_DOUBLE), value, intent(in)::value
      end subroutine set_scalar
      end interface

!>! Adds a sial program to the list of programs
      interface
      subroutine add_sial_program(sname) bind(C)
                use, intrinsic :: ISO_C_BINDING
      character, dimension(*), intent(in):: sname
      end subroutine add_sial_program
      end interface

!>  Sets the shape and contents of a predefined static array
!>  @param aname  The null terminated name of the array
!>  @param  numDims  The rank of the array
!>  @param  dims an array containing the sizes of each dimension.  This size of this array may either
!>    be the actual number of dimensions (i.e. 2 for a 2-d static array) or MAXDIM
!>  @param  vals  Array containing the elements of the static array, which must be laid out contiguously.      
      interface
      subroutine set_predefined_scalar_array(aname, numDims, dims, vals)
     * bind(C) use, intrinsic :: ISO_C_BINDING
      character, dimension(*), intent(in):: aname
      integer (C_INT), value, intent(in):: numDims
      integer (C_INT), dimension(*), intent(in)::dims
      real(C_DOUBLE), dimension(*),  intent(in):: vals
      end subroutine set_predefined_scalar_array
      end interface

!>  Sets the shape and contents of a predefined static array
!>  @param aname  The null terminated name of the array
!>  @param  numDims  The rank of the array
!>  @param  dims an array containing the sizes of each dimension.  This size of this array may either
!>    be the actual number of dimensions (i.e. 2 for a 2-d static array) or MAXDIM
!>  @param  vals  Array containing the elements of the INTEGER static array, which must be laid out contiguously.      
      interface
      subroutine set_predefined_integer_array(aname, numDims, dims, 
     *                                        vals)bind(C)
          use, intrinsic :: ISO_C_BINDING
      character, dimension(*), intent(in):: aname
      integer (C_INT), value, intent(in):: numDims
      integer (C_INT), dimension(*), intent(in)::dims
      integer (C_INT), dimension(*),  intent(in):: vals
      end subroutine set_predefined_integer_array
      end interface

!> Sets segment sizes for aoindex
!> @param num_segments The number of segments
!> @param segment_sizes an array containing the size of each segment
      interface
      subroutine set_aoindex_info(num_segments, segment_sizes) bind(C)
          use, intrinsic :: ISO_C_BINDING
      integer (C_INT), value, intent(in)::num_segments
      integer (C_INT), dimension(*), intent(in)::segment_sizes
      end subroutine set_aoindex_info
      end interface

!> Sets segment sizes for moaindex
!> @param num_segments The number of segments
!> @param segment_sizes an array containing the size of each segment
      interface
      subroutine set_moaindex_info(num_segments, segment_sizes) bind(C)
          use, intrinsic :: ISO_C_BINDING
      integer (C_INT), value, intent(in)::num_segments
      integer (C_INT), dimension(*), intent(in)::segment_sizes
      end subroutine set_moaindex_info
      end interface

!> Sets segment sizes for mobindex
!> @param num_segments The number of segments
!> @param segment_sizes an array containing the size of each segment
      interface
      subroutine set_mobindex_info(num_segments, segment_sizes) bind(C)
          use, intrinsic :: ISO_C_BINDING
      integer (C_INT), value, intent(in)::num_segments
      integer (C_INT), dimension(*), intent(in)::segment_sizes
      end subroutine set_mobindex_info
      end interface

!> Sets segment sizes for moindex
!> @param num_segments The number of segments
!> @param segment_sizes an array containing the size of each segment
      interface
      subroutine set_moindex_info(num_segments, segment_sizes) bind(C)
          use, intrinsic :: ISO_C_BINDING
      integer (C_INT), value, intent(in)::num_segments
      integer (C_INT), dimension(*), intent(in)::segment_sizes
      end subroutine set_moindex_info
      end interface
          

!> Sets sial config paramaters for sial codes in jobflow
!> @param sialfile the sial file for which config option is begin set
!> @param key config key
!> @param value config value
      interface
      subroutine set_config_info(sialfile, key, value) bind(C)
          use, intrinsic :: ISO_C_BINDING
      character, dimension(*), intent(in):: sialfile
      character, dimension(*), intent(in):: key
      character, dimension(*), intent(in):: value
      end subroutine set_config_info
      end interface


!> Finalizes the setup information process and writes the output files
      interface
      subroutine finalize_setup() bind(C)
      end subroutine finalize_setup
      end interface

      interface
      subroutine dump_file (fname) bind(C)
      character, dimension(*), intent(in) :: fname
      end subroutine dump_file
      end interface
