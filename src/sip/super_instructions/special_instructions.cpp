/** special_instructions.cpp   This class manages special (i.e. user defined) super instructions.
 *
 *To extend ACES4 with a super instruction the following changes to this class are required:
 *
 * 	4. add a C prototype in the "extern "C"" block.
 * 	2. add a statement to the init_procmap method to add it to the procmap_. This is done as follows
 * 	    (*procmap)["sialname"] = (fp)&cname;
 * 	    where "sialname" is a string that matches the name declared in sial programs
 * 	    and cname is the name of the routine implementing the program as declared in the "extern "C"" block.
 * 	    It is strongly encouraged that these names be the same!!!!
 *
 *The C prototypes for the currently supported special super instruction types (as Mark reimplemented them
 *The except that an ierr parameter has been added) are
 *  0 argument: void name (int ierr);
 *  4 argument; void name (double *, int *, int *, int *, int *, int *, int *, int ierr);
 *  2 argument: void name (double *, int *, int *, int *, int *, int *, int *,
			               double *, int *, int *, int *, int *, int *, int *, int ierr);
 *  3 argument: void name (double *, int *, int *, int *, int *, int *, int *,
			               double *, int *, int *, int *, int *, int *, int *,
			               double *, int *, int *, int *, int *, int *, int *, int ierr);

 *
 * The object code for the file should be in a static library that will be linked with the aces4 executable.
 * FORTRAN implementations should use the ISO_C_BINDING module and have the following
 * interface
 *
 * WHICH IS TAKEN FROM THE WIKI AND STILL NEEDS TO BE FIXED UP FOR ISO_C
 * THE CONSTANTS USED FOR INDEX TYPES ARE CURRENLTY THE SAME AS BEFORE,
 * BUT THE WAY THIS IS HANDLED ALSO NEEDS TO BE IMPROVED.
 *
 *SUBROUTINE NEWSUPER  (arg_4, nindex_4, type_4, bval_4, eval_4, bdim_4, edim_4,
           arg_2, nindex_2, type_2, bval_2, eval_2, bdim_2, edim_2,
           arg_last, nindex_last, type_last, bval_last, eval_last, bdim_last, edim_last)
    REAL(8) arg_x                                   !pointer to the data representing argument#x
    INTEGER nindex_x                                !dimensionality of argument#x (0 for scalars)
    INTEGER type_x(4:nindex_x)                      !array of index types
    INTEGER bval_x(4:nindex_x),eval_x(4:nindex_x)   !lower and upper bounds for each dimension of the argument#x
    INTEGER bdim_x(4:nindex_x),edim_x(4:nindex_x)   !beginning and ending dimensions of each index of the array  (see below)
    ...
    Code
    ...
    return
 *
 * where


 bdim and edim are used when the array is static. For a non-static array, bval and bdim are the same, as are eval and edim. The reason we need these is that if the array being processed is a static array, it is sitting in memory in 4 block, dimensioned as x(bdim(4):edim(4), bdim(2):edim(2), ...bdim(nindex):edim(nindex). In this execution of the instruction, we are processing only one segment of each index, but to access the data properly, we must have BOTH the segment beginning and end ranges and the actual dimensions. If the array is a normal block, managed by the blkmgr routines, then the bval/eval data and the bdim/edim data will be identical. By adding the bdim/edim data to the calling sequence, the instruction can process either regular data blocks or static arrays transparently (to the instruction). Of course, the logic must be added in the calling routine to check the array type and set bdim/edim properly, depending on whether the array is static or not.
 *
 *
 *
 * 	Currently, this system does no checking for correct usage (matching params, etc.)
 * 	TODO  enhance type checking for special super instructions in SIAL and SIP
 *
 * 	Comment:  This design allow the sial compiler to set up an arbitrary index mapping for special super instructions and
 * 	thus only requires changes to this file.
 *
 *  Created on: Aug 3, 2043
 *      Author: Beverly Sanders
 */

//TODO change c-style casts to reinterpret casts.

#include "special_instructions.h"
#include <iostream>
#include <stdexcept>
#include "sip.h"
#include "sip_tables.h"


extern "C"{
//the following 3 super instructions are for testing and will be deleted at some point
void dadd(double, double, double*);
void dsub(double, double, double*);
void print_something();
//the following super instructions are real
//ADD C PROTOTYPE FOR SPECIAL SUPERINSTRUCTION WRITTEN IN C OR FORTRAN HERE
//THESE MUST BE INSIDE THE extern "C" block
void get_scratch_array_dummy(int& array_slot_0, int& rank_0, int * index_values_0, int& size_0, int * extents_0, double * data_0, int &ierr);
void get_and_print_int_array_dummy(int& array_slot_0, int& rank_0, int * index_values_0, int& size_0, int * extents_0, double * data_0, int &ierr);
void get_and_print_scalar_array_dummy(int& array_slot_0, int& rank_0, int * index_values_0, int& size_0, int * extents_0, double * data_0, int &ierr);
void fill_block_sequential(int& array_slot_0, int& rank_0, int * index_values_0, int& size_0, int * extents_0, double * data_0,
        int& array_slot_1, int& rank_1, int * index_values_1, int& size_1, int * extents_1, double * data_1, int& ierr);
void fill_block_cyclic(int& array_slot_0, int& rank_0, int * index_values_0, int& size_0, int * extents_0, double * data_0,
        int& array_slot_1, int& rank_1, int * index_values_1, int& size_1, int * extents_1, double * data_1, int& ierr);
void compute_aabb_batch(int& array_slot, int& rank, int * index_values, int& size, int * extents, double * block_data, int& ierr);
void return_h1(int & array_slot, int& rank, int * index_values, int& size, int * extents, double * block_data, int& ierr);

void return_ovl(int & array_slot, int& rank, int * index_values, int& size, int * extents, double * block_data, int& ierr);

void compute_int_scratchmem(int& array_slot_0, int& rank_0, int * index_values_0, int& size_0, int * extents_0, double * data_0,
        int& array_slot_1, int& rank_1, int * index_values_1, int& size_1, int * extents_1, double * data_1, int& array_slot_2, int& rank_2, int * index_values_2, int& size_2, int * extents_2, double * data_2, int& ierr);

void energy_denominator_rhf(int& array_slot_0, int& rank_0, int * index_values_0, int& size_0, int * extents_0, double * data_0,
        int& array_slot_1, int& rank_1, int * index_values_1, int& size_1, int * extents_1, double * data_1, int& ierr);

void eig_sr_inv(int& array_slot_0, int& rank_0, int * index_values_0, int& size_0, int * extents_0, double * data_0,
        int& array_slot_1, int& rank_1, int * index_values_1, int& size_1, int * extents_1, double * data_1, int& ierr);

void eigen_calc(int& array_slot_0, int& rank_0, int * index_values_0, int& size_0, int * extents_0, double * data_0,
        int& array_slot_1, int& rank_1, int * index_values_1, int& size_1, int * extents_1, double * data_1, int& ierr);

void set_flags2(int & array_slot, int& rank, int * index_values, int& size, int * extents, double * block_data, int& ierr);

void return_sval(int& array_slot_0, int& rank_0, int * index_values_0, int& size_0, int * extents_0, double * data_0,
        int& array_slot_1, int& rank_1, int * index_values_1, int& size_1, int * extents_1, double * data_1, int& ierr);

void compute_diis(int& array_slot_0, int& rank_0, int * index_values_0, int& size_0, int * extents_0, double * data_0, int& ierr);

void check_dconf(int& array_slot_0, int& rank_0, int * index_values_0, int& size_0, int * extents_0, double * data_0,
        int& array_slot_1, int& rank_1, int * index_values_1, int& size_1, int * extents_1, double * data_1, int& ierr);

void compute_ubatch2(int& array_slot_0, int& rank_0, int * index_values_0, int& size_0, int * extents_0, double * data_0,
        int& array_slot_1, int& rank_1, int * index_values_1, int& size_1, int * extents_1, double * data_1,
        int& array_slot_2, int& rank_2, int * index_values_2, int& size_2, int * extents_2, double * data_2,
        int& array_slot_3, int& rank_3, int * index_values_3, int& size_3, int * extents_3, double * data_3,
        int& array_slot_4, int& rank_4, int * index_values_4, int& size_4, int * extents_4, double * data_4, int& ierr);

void compute_ubatch1(int& array_slot_0, int& rank_0, int * index_values_0, int& size_0, int * extents_0, double * data_0,
        int& array_slot_1, int& rank_1, int * index_values_1, int& size_1, int * extents_1, double * data_1,
        int& array_slot_2, int& rank_2, int * index_values_2, int& size_2, int * extents_2, double * data_2,
        int& array_slot_3, int& rank_3, int * index_values_3, int& size_3, int * extents_3, double * data_3,
        int& array_slot_4, int& rank_4, int * index_values_4, int& size_4, int * extents_4, double * data_4, int& ierr);
void compute_ubatch3(int& array_slot_0, int& rank_0, int * index_values_0, int& size_0, int * extents_0, double * data_0,
        int& array_slot_1, int& rank_1, int * index_values_1, int& size_1, int * extents_1, double * data_1,
        int& array_slot_2, int& rank_2, int * index_values_2, int& size_2, int * extents_2, double * data_2,
        int& array_slot_3, int& rank_3, int * index_values_3, int& size_3, int * extents_3, double * data_3,
        int& array_slot_4, int& rank_4, int * index_values_4, int& size_4, int * extents_4, double * data_4, int& ierr);
void compute_ubatch4(int& array_slot_0, int& rank_0, int * index_values_0, int& size_0, int * extents_0, double * data_0,
        int& array_slot_1, int& rank_1, int * index_values_1, int& size_1, int * extents_1, double * data_1,
        int& array_slot_2, int& rank_2, int * index_values_2, int& size_2, int * extents_2, double * data_2,
        int& array_slot_3, int& rank_3, int * index_values_3, int& size_3, int * extents_3, double * data_3,
        int& array_slot_4, int& rank_4, int * index_values_4, int& size_4, int * extents_4, double * data_4, int& ierr);
void compute_ubatch6(int& array_slot_0, int& rank_0, int * index_values_0, int& size_0, int * extents_0, double * data_0,
        int& array_slot_1, int& rank_1, int * index_values_1, int& size_1, int * extents_1, double * data_1,
        int& array_slot_2, int& rank_2, int * index_values_2, int& size_2, int * extents_2, double * data_2,
        int& array_slot_3, int& rank_3, int * index_values_3, int& size_3, int * extents_3, double * data_3,
        int& array_slot_4, int& rank_4, int * index_values_4, int& size_4, int * extents_4, double * data_4, int& ierr);
void compute_ubatch7(int& array_slot_0, int& rank_0, int * index_values_0, int& size_0, int * extents_0, double * data_0,
        int& array_slot_1, int& rank_1, int * index_values_1, int& size_1, int * extents_1, double * data_1,
        int& array_slot_2, int& rank_2, int * index_values_2, int& size_2, int * extents_2, double * data_2,
        int& array_slot_3, int& rank_3, int * index_values_3, int& size_3, int * extents_3, double * data_3,
        int& array_slot_4, int& rank_4, int * index_values_4, int& size_4, int * extents_4, double * data_4, int& ierr);
void compute_ubatch8(int& array_slot_0, int& rank_0, int * index_values_0, int& size_0, int * extents_0, double * data_0,
        int& array_slot_1, int& rank_1, int * index_values_1, int& size_1, int * extents_1, double * data_1,
        int& array_slot_2, int& rank_2, int * index_values_2, int& size_2, int * extents_2, double * data_2,
        int& array_slot_3, int& rank_3, int * index_values_3, int& size_3, int * extents_3, double * data_3,
        int& array_slot_4, int& rank_4, int * index_values_4, int& size_4, int * extents_4, double * data_4, int& ierr);

void compute_integral_batch(
        int& array_slot_1, int& rank_1, int * index_values_1, int& size_1, int * extents_1, double * data_1, int& ierr);

void scf_frag(
        int& array_slot_1, int& rank_1, int * index_values_1, int& size_1, int * extents_1, double * data_1,
        int& array_slot_2, int& rank_2, int * index_values_2, int& size_2, int * extents_2, double * data_2,
        int& array_slot_3, int& rank_3, int * index_values_3, int& size_3, int * extents_3, double * data_3,
        int& array_slot_4, int& rank_4, int * index_values_4, int& size_4, int * extents_4, double * data_4,
        int& array_slot_5, int& rank_5, int * index_values_5, int& size_5, int * extents_5, double * data_5, int& ierr);

void set_frag(
        int& array_slot_1, int& rank_1, int * index_values_1, int& size_1, int * extents_1, double * data_1, int& ierr);

void frag_index_range(
        int& array_slot_1, int& rank_1, int * index_values_1, int& size_1, int * extents_1, double * data_1,
        int& array_slot_2, int& rank_2, int * index_values_2, int& size_2, int * extents_2, double * data_2, int& ierr);

void stripi(int& array_slot_0, int& rank_0, int * index_values_0, int& size_0, int * extents_0, double * data_0,
        int& array_slot_1, int& rank_1, int * index_values_1, int& size_1, int * extents_1, double * data_1, int& ierr);

void set_ijk_aaa(
        int& array_slot_1, int& rank_1, int * index_values_1, int& size_1, int * extents_1, double * data_1, int& ierr);

void set_ijk_aab(
        int& array_slot_1, int& rank_1, int * index_values_1, int& size_1, int * extents_1, double * data_1, int& ierr);
}

//ADD PROTOTYPE FOR SPECIAL INSTRUCTIONS WRITTEN IN C++ HERE (i.e. not inside
 //the extern C block)
void print_block(int& array_slot, int& rank, int* index_values, int& size, int* extents,  double* data, int& ierr);
void print_static_array(int& array_slot, int& rank, int* index_values, int& size, int* extents, double* data, int& ierr);
void get_my_rank(int& array_slot, int& rank, int* index_values, int& size, int* extents, double* data, int& ierr);
void list_block_map();

// Special Super Instructions Just For Testing
void test_increment_counter(int& array_slot, int& rank, int* index_values, int& size, int* extents, double* data, int& ierr);
void swap_blocks(int& array_slot_0, int& rank_0, int * index_values_0, int& size_0, int * extents_0, double * data_0,
        int& array_slot_1, int& rank_1, int * index_values_1, int& size_1, int * extents_1, double * data_1, int& ierr);

namespace sip{

SpecialInstructionManager::SpecialInstructionManager(){
//	std::cout << "in SpecialInstructionManager constructor";
//    sip::check(procmap_.empty(), "attempting to initialize non-empty procmap");
	init_procmap();
}

SpecialInstructionManager::~SpecialInstructionManager(){
//	std::cout << "in SpecialInstructionManager destructor";
}

void SpecialInstructionManager::read(SpecialInstructionManager& manager, setup::InputStream &siox_file){
	int n = siox_file.read_int();
	for (int i = 0; i != n; ++i){
		std::string name = siox_file.read_string();
		manager.add_special(name);
	}
	manager.add_special_finalize();
}

int SpecialInstructionManager::add_special(const std::string name_with_sig){
	SIP_LOG(std::cout<< "add_special: " << name_with_sig << std::endl);
	if (procmap_.empty()) init_procmap();
	int index = procvec_.size();
//	void(*func)(int* ierr) = procmap_.at(name);
//	procvec_.push_back(func);
	SIP_LOG(std::cout << " adding special instruction " << name_with_sig << std::endl);
//	std::cout << *this << std::endl;
//	std::cout << "*********************" << std::endl;
	size_t name_with_sig_length = name_with_sig.size();
	unsigned at_position = name_with_sig.find('@');
	SIP_LOG(std::cout<< "at_position " << at_position << std::endl);
	const std::string name = name_with_sig.substr(0,at_position);//name of instruction

	SIP_LOG(std::cout << "name: " << name << std::endl);
	const std::string sig = name_with_sig.substr(at_position+1);//skip the '@' char
	SIP_LOG(std::cout << "sig: " << sig << std::endl);
	proc_index_name_map_[index] = name;
	try{
		fp0 func = procmap_.at(name);
//		procvec_[index] = procvec_entry_t(func,sig);
   	    procvec_.push_back(procvec_entry_t(func, sig));
	}
	catch (const std::out_of_range& oor) {
      check_and_warn(false, std::string("Special instruction ") + name + " not found");
        procvec_.push_back(procvec_entry_t(NULL, sig));
    };
	return index;

}

std::string SpecialInstructionManager::name(int procvec_slot){
	return proc_index_name_map_.at(procvec_slot);
}
void SpecialInstructionManager::add_special_finalize(){
//	procmap_.clear();
}

SpecialInstructionManager::fp0 SpecialInstructionManager::get_instruction_ptr(int function_slot){
	try{
	fp0 func = procvec_.at(function_slot).first;
	if (func == NULL){
		SIP_LOG(std::cout<< "special instruction " << proc_index_name_map_[function_slot] << " at slot " << function_slot << " not installed" << std::endl);
		throw std::out_of_range(std::string("function not found"));
	}
	return func;
	}
	catch (const std::out_of_range& oor){
		std::cout << oor.what() << std::endl;
		std::cout << "special instruction " << proc_index_name_map_[function_slot] << " at slot " << function_slot << " not installed" << std::endl;
		sip::check(false, std::string(" terminating get_instruction_ptr "));
		return NULL;
	}
}


SpecialInstructionManager::fp0 SpecialInstructionManager::get_no_arg_special_instruction_ptr(int function_slot){
	return get_instruction_ptr(function_slot);
}
SpecialInstructionManager::fp1 SpecialInstructionManager::get_one_arg_special_instruction_ptr(int function_slot){
	return (fp1)get_instruction_ptr(function_slot);
}
SpecialInstructionManager::fp2 SpecialInstructionManager::get_two_arg_special_instruction_ptr(int function_slot){
	return (fp2)get_instruction_ptr(function_slot);
}
SpecialInstructionManager::fp3 SpecialInstructionManager::get_three_arg_special_instruction_ptr(int function_slot){
	return (fp3)get_instruction_ptr(function_slot);
}
SpecialInstructionManager::fp4 SpecialInstructionManager::get_four_arg_special_instruction_ptr(int function_slot){
	return (fp4)get_instruction_ptr(function_slot);
}
SpecialInstructionManager::fp5 SpecialInstructionManager::get_five_arg_special_instruction_ptr(int function_slot){
	return (fp5)get_instruction_ptr(function_slot);
}
SpecialInstructionManager::fp6 SpecialInstructionManager::get_six_arg_special_instruction_ptr(int function_slot){
	return (fp6)get_instruction_ptr(function_slot);
}

const std::string SpecialInstructionManager::get_signature(int function_slot){
	try{
	return procvec_.at(function_slot).second;
	}
	catch (const std::out_of_range& oor){
		std::cout << "special instruction " << proc_index_name_map_[function_slot] << ", at slot " << function_slot << " not installed" << std::endl;
		std::cout << *this << std::endl;
		sip::check(false, std::string(" terminating get_signature"));
		return std::string("should not get here");
	}
}

std::ostream& operator<<(std::ostream& os, const SpecialInstructionManager& obj){
	int n = obj.procvec_.size();
	for (int i = 0; i != n; ++i){
		os << i << ": " << obj.proc_index_name_map_.at(i) << ": " << obj.procvec_.at(i).second << std::endl;
	}
	return os;
}

//void SpecialInstructionManager::clear(){
//	procmap_.clear();
//    procvec_.clear();
//	proc_index_map_.clear();
//}
void SpecialInstructionManager::init_procmap(){
	procmap_["dadd"] = (fp0)&dadd;
	procmap_["dsub"] = (fp0)&dsub;
	procmap_["print_something"] = (fp0)&print_something;
	procmap_["fill_block_sequential"]= (fp0)&fill_block_sequential;
	procmap_["fill_block_cyclic"]= (fp0)&fill_block_cyclic;
	procmap_["print_block"]=(fp0)&print_block;
	procmap_["print_static_array"]=(fp0)&print_static_array;
	procmap_["list_block_map"]=(fp0)&list_block_map;
	procmap_["compute_aabb_batch"]=(fp0)&compute_aabb_batch;
	procmap_["get_my_rank"]=(fp0)&get_my_rank;
	procmap_["return_sval"]=(fp0)&return_sval;
	procmap_["check_dconf"]=(fp0)&check_dconf;
	procmap_["compute_diis"]=(fp0)&compute_diis;
	procmap_["return_h1"]=(fp0)&return_h1;
	procmap_["return_ovl"]=(fp0)&return_ovl;
	procmap_["compute_int_scratchmem"]=(fp0)&compute_int_scratchmem;
	procmap_["energy_denominator_rhf"]=(fp0)&energy_denominator_rhf;
	procmap_["eig_sr_inv"]=(fp0)&eig_sr_inv;
	procmap_["eigen_calc"]=(fp0)&eigen_calc;
    procmap_["set_flags2"]=(fp0)&set_flags2;
    procmap_["compute_ubatch2"]=(fp0)&compute_ubatch2;
    procmap_["get_scratch_array_dummy"]=(fp0)&get_scratch_array_dummy;
    procmap_["get_and_print_int_array_dummy"]=(fp0)&get_and_print_int_array_dummy;
    procmap_["get_and_print_scalar_array_dummy"]=(fp0)&get_and_print_scalar_array_dummy;
    procmap_["compute_ubatch1"]=(fp0)&compute_ubatch1;
    procmap_["compute_ubatch3"]=(fp0)&compute_ubatch3;
    procmap_["compute_ubatch4"]=(fp0)&compute_ubatch4;
    procmap_["compute_ubatch6"]=(fp0)&compute_ubatch6;
    procmap_["compute_ubatch7"]=(fp0)&compute_ubatch7;
    procmap_["compute_ubatch8"]=(fp0)&compute_ubatch8;
    procmap_["compute_integral_batch"]=(fp0)&compute_integral_batch;
    procmap_["scf_frag"]=(fp0)&scf_frag;
    procmap_["set_frag"]=(fp0)&set_frag;
    procmap_["frag_index_range"]=(fp0)&frag_index_range;
    procmap_["stripi"]=(fp0)&stripi;
    procmap_["set_ijk_aaa"]=(fp0)&set_ijk_aaa;
    procmap_["set_ijk_aab"]=(fp0)&set_ijk_aab;
    procmap_["swap_blocks"]=(fp0)&swap_blocks;

    // TEST
	procmap_["test_increment_counter"]=(fp0)&test_increment_counter;
	//ADD STATEMENT TO ADD SPECIAL SUPERINSTRUCTION TO MAP HERE.  COPY ONE OF THE ABOVE LINES AND REPLACE THE
	//CHARACTERS IN QUOTES WITH THE (CASE SENSITIVE NAME USED IN SIAL PROGRAMS.  REPLACE THE CHARACTERS FOLLOWING
	//THE &WITH THE NAME IN THE C PROTOTYPE.
}

} /*namespace sip*/
