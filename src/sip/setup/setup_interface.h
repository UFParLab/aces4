/**Provides definitions of fortran callable routines used in the setup phase
 * 
 *  Created on: Jul 3, 2013
 *      Author: Beverly Sanders
 */

#ifndef SETUP_INTERFACE_H_
#define SETUP_INTERFACE_H_


/*! The following routines are callable from Fortran */

#ifdef __cplusplus
extern "C" {
#endif

    /*!Call to initialize with name of job.
     *
     * The header file will be job_name.h
     * The data file will be job_name.dat
     *
     * @param job_name The name of the job.
     * @warning job_name must be a null terminated character array
    */
   void init_setup(const char * job_name);

   /*Call when finished.  Writes provided data to appropriate files. */
   void finalize_setup();

   /*! Sets value of a symbolic constant */
   void set_constant(const char * name, int value);

   /*! Set the value of a predefined scalar */
   void set_scalar(const char * name, double value);

   /*! Adds a sial program to the list of programs */
   void add_sial_program(const char *sial_name);

   /*!  Specify segment sizes for the aoindex type */
   void set_aoindex_info(int num_segments, int *segment_sizes);

   /*!  Specify  segment sizes for the moaindex type */
   void set_moaindex_info(int num_segments, int *segment_sizes);

   /*!  Specify  segment sizes for the mobindex type */
   void set_mobindex_info(int num_segments, int *segment_sizes);

    /*!  Specify segment sizes for the moindex type */
   void set_moindex_info(int num_segments, int *segment_sizes);

   /*!  Specify static predefined real arrays */
  void set_predefined_scalar_array(const char*aname, int num_dims, int *dims, double *values);

  /*! Specify static predefined integer arrays */
  void set_predefined_integer_array(const char *static_array_name, int numDims, int *dims, int *vals);

  /*!  Specify sial config parameters */
  void set_config_info(const char* sialfile, const char* key, const char* value);

   /*!  For debugging, dump file after calling finalize*/
   void dump_file(const char * file_name);

#ifdef __cplusplus
   }
#endif


#endif /* SETUP_INTERFACE_H_ */
