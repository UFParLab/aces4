
#ifndef SPECIAL_INSTRUCTIONS_H_
#define SPECIAL_INSTRUCTIONS_H_

#include <map>
#include <string>
#include <vector>
#include "sip.h"
#include "io_utils.h"

namespace sip {


/** special_instructions.h
 * Manages special (i.e. user defined) super instructions.
 *
 * Adding additional super instructions requires changing the class implementation in SpecialInstructionManager.cpp
 *
 *  Created on: Aug 3, 2013
 *      Author: Beverly Sanders
 */

class SpecialInstructionManager{
public:
	/** ddD function pointer.  Used for testing, not in SIAL.  Will be deleted at some point*/
	typedef void(*ddD)(double, double, double*);

	/** no arg function pointer.  All pointers are cast to this type to be able to store in a common map and vector. */
	typedef void(*fp0)(int& ierr);

	/** The Fortran header for a super instruction with one argument
	 *
	       subroutine SUB(
	     c array_slot, rank, index_values, size, extents, data,
	     c  ierr) BIND(C)
	      use, intrinsic :: ISO_C_BINDING
	      implicit none
	      integer(C_INT), intent(in)::array_slot_0
	      integer(C_INT), intent(in)::rank_0
	      integer(C_INT), intent(in)::index_values_0(1:rank_0)
	      integer(C_INT), intent(in)::size_0
	      integer(C_INT), intent(in)::extents_0(1:rank_0)
	      real(C_DOUBLE), intent(out)::data_0(1:size_0)
	      integer(C_INT), intent(out)::ierr
	 */
	typedef void(*fp1)(int& array_slot, int& rank, int * index_values, int& size, int * extents, double * block_data, int& ierr);


/** The Fortran header for a super instruction with two arguments
 *
       subroutine SUB(
     c array_slot_0, rank_0, index_values_0, size_0, extents_0, data_0,
     c array_slot_1, rank_1, index_values_1, size_1, extents_1, data_1,
     c  ierr) BIND(C)
      use, intrinsic :: ISO_C_BINDING
      implicit none
      integer(C_INT), intent(in)::array_slot_0
      integer(C_INT), intent(in)::rank_0
      integer(C_INT), intent(in)::index_values_0(1:rank_0)
      integer(C_INT), intent(in)::size_0
      integer(C_INT), intent(in)::extents_0(1:rank_0)
      real(C_DOUBLE), intent(out)::data_0(1:size_0)

      integer(C_INT), intent(in)::array_slot_1
      integer(C_INT), intent(in)::rank_1
      integer(C_INT), intent(in)::index_values_1(1:rank_1)
      integer(C_INT), intent(in)::size_1
      integer(C_INT), intent(in)::extents_1(1:rank_1)
      real(C_DOUBLE), intent(in)::data_1(1:size_0)

      integer(C_INT), intent(out)::ierr
 */
    typedef void(*fp2)(int& array_slot_0, int& rank_0, int * index_values_0, int& size_0, int * extents_0, double * data_0,
    		           int& array_slot_1, int& rank_1, int * index_values_1, int& size_1, int * extents_1, double * data_1, int& ierr);

/** And you can figure the rest out */

    typedef void(*fp3)(int& array_slot_0, int& rank_0, int * index_values_0, int& size_0, int * extents_0, double * data_0,
    		           int& array_slot_1, int& rank_1, int * index_values_1, int& size_1, int * extents_1, double * data_1,
                       int& array_slot_2, int& rank_2, int * index_values_2, int& size_2, int * extents_2, double * data_2, int& ierr);

    typedef void(*fp4)(int& array_slot_0, int& rank_0, int * index_values_0, int& size_0, int * extents_0, double * data_0,
    		           int& array_slot_1, int& rank_1, int * index_values_1, int& size_1, int * extents_1, double * data_1,
                       int& array_slot_2, int& rank_2, int * index_values_2, int& size_2, int * extents_2, double * data_2,
                       int& array_slot_3, int& rank_3, int * index_values_3, int& size_3, int * extents_3, double * data_3, int& ierr);

    typedef void(*fp5)(int& array_slot_0, int& rank_0, int * index_values_0, int& size_0, int * extents_0, double * data_0,
	                   int& array_slot_1, int& rank_1, int * index_values_1, int& size_1, int * extents_1, double * data_1,
                       int& array_slot_2, int& rank_2, int * index_values_2, int& size_2, int * extents_2, double * data_2,
                       int& array_slot_3, int& rank_3, int * index_values_3, int& size_3, int * extents_3, double * data_3,
                       int& array_slot_4, int& rank_4, int * index_values_4, int& size_4, int * extents_4, double * data_4, int& ierr);


    typedef void(*fp6)(int& array_slot_0, int& rank_0, int * index_values_0, int& size_0, int * extents_0, double * data_0,
                       int& array_slot_1, int& rank_1, int * index_values_1, int& size_1, int * extents_1, double * data_1,
                       int& array_slot_2, int& rank_2, int * index_values_2, int& size_2, int * extents_2, double * data_2,
                       int& array_slot_3, int& rank_3, int * index_values_3, int& size_3, int * extents_3, double * data_3,
                       int& array_slot_4, int& rank_4, int * index_values_4, int& size_4, int * extents_4, double * data_4,
                       int& array_slot_5, int& rank_5, int * index_values_5, int& size_5, int * extents_5, double * data_5, int& ierr);
	SpecialInstructionManager();
	~SpecialInstructionManager();


	static void read(SpecialInstructionManager& manager, setup::InputStream &siox_file);



	/** returns the special super instruction at the given index. The given methods return function pointers
	 * cast to correct types for currently supported superinstruction formats.
	 * Note that the number of arguments refers to the number of arguments in the sial program.
	 *
	 * @param  index of the desired special super instruction
	 * @return pointer to function implementing the super instruction
	 */
	 fp0 get_no_arg_special_instruction_ptr(int function_slot);
	 fp1 get_one_arg_special_instruction_ptr(int function_slot);
	 fp2 get_two_arg_special_instruction_ptr(int function_slot);
	 fp3 get_three_arg_special_instruction_ptr(int function_slot);
	 fp4 get_four_arg_special_instruction_ptr(int function_slot);
	 fp5 get_five_arg_special_instruction_ptr(int function_slot);
	 fp6 get_six_arg_special_instruction_ptr(int function_slot);

	 /**
	  * Returns a string containing the signature for the function as declared in the SIAL program.
	  * The string contains a character for each argument in the SIAL program which is one of 'r', 'w', or 'u' for
	  * read, write, and update respectively.  The signatures are used for block management.
	  */
      const std::string get_signature(int function_slot);

  	 /** returns the name of the special superinstruction at the given slot */
  	  std::string name(int procvec_slot);

	  friend std::ostream& operator<<(std::ostream&, const SpecialInstructionManager&);
	  friend class SipTables;

private:
	/**Initializes procmap.  Called by the constructor. */
	void init_procmap();

	/** adds the given special super instruction to proc vector
	 * @param Case sensitive name "c" name of routine (which we hope is the same as the fortran name) with an '@' followed by the signature string
	 * @returns the index of the added function
	 */
	int add_special(const std::string name_with_sig);

	/** optionally, this smethod can be called after all special instructions needed for a sial program have been added to the vector.
	 * It releases resources, such as the procmap that are no longer needed.
	 */
	void add_special_finalize();


	/** returns the address of the special superinstruction at the given slot.
	 * It is stored as type fp0; if the superinstruction has arguments, this pointer must be cast to the appropriate type.
	 */
	fp0 get_instruction_ptr(int function_slot);

	/** The map from name to function pointer.
	 * Contains all known super instructions and is initialized in
	 * the SpecialInstructionManager constructor.
	 */
	std::map<std::string, fp0> procmap_;


	/** the vector holding super instructions needed for the current SIAL program where the slot is assigned by the compiler.
	 * It is initialized using the super instruction names read from the .siox file.  The first component of the pair is the
	 * address of the super instruction. The second is a string with the signature as given in the sial program.  A signature
	 * is a possibly 0 length string containing the characters
	 * r, w, and u, where these mean read, write, and update, respectively of the corresponding argument.
	 */
	typedef std::pair<fp0,std::string> procvec_entry_t;
	typedef std::vector<procvec_entry_t> procvec_t;
	typedef std::vector<procvec_entry_t>::iterator procvec_iter_t;
	procvec_t procvec_;

    typedef std::map <int, std::string> proc_index_name_map_t;
    typedef std::map <int, std::string>::iterator proc_index_Name_map_iter;
    proc_index_name_map_t proc_index_name_map_; //maps index in proc array to name of routine


	DISALLOW_COPY_AND_ASSIGN(SpecialInstructionManager);
};

} /*namespace sip*/

#endif /* SPECIAL_INSTRUCTIONS_H_ */
