/** special_instructions.cpp   This class manages special (i.e. user defined) super instructions.
 *
 *To extend ACES4 with a super instruction the following changes to this class are required:
 *
 * 	4. add a C prototype (inside the "extern "C"" block for instruction written in fortran and c, outside for instructions written in c++)
 * 	2. add a statement to the init_procmap method to add it to the procmap_. This is done as follows
 * 	    (*procmap)["sialname"] = (fp)&cname;
 * 	    where "sialname" is a string that matches the name declared in sial programs
 * 	    and cname is the name of the routine implementing the program as declared in the "extern "C"" block.
 * 	    It is strongly encouraged that these names be the same!!!!
 *
 *
 * 	Comment:  This design allow the sial compiler to set up an arbitrary index mapping for special super instructions and
 * 	thus adding a super instruction only requires changes to this file.
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

void scf_atom(
int & array_slot_1, int& rank_1, int * index_values_1, int& size_1, int * extents_1, double * block_data_1, 
int & array_slot_2, int& rank_2, int * index_values_2, int& size_2, int * extents_2, double * block_data_2, 
int & array_slot_3, int& rank_3, int * index_values_3, int& size_3, int * extents_3, double * block_data_3, 
int & array_slot_4, int& rank_4, int * index_values_4, int& size_4, int * extents_4, double * block_data_4, 
int & array_slot_5, int& rank_5, int * index_values_5, int& size_5, int * extents_5, double * block_data_5, 
int & array_slot_6, int& rank_6, int * index_values_6, int& size_6, int * extents_6, double * block_data_6, 
int& ierr);

void return_pairs(
int & array_slot_1, int& rank_1, int * index_values_1, int& size_1, int * extents_1, double * block_data_1, 
int & array_slot_2, int& rank_2, int * index_values_2, int& size_2, int * extents_2, double * block_data_2, 
int & array_slot_3, int& rank_3, int * index_values_3, int& size_3, int * extents_3, double * block_data_3, 
int & array_slot_4, int& rank_4, int * index_values_4, int& size_4, int * extents_4, double * block_data_4, 
int& ierr);

void return_h1frag(
int& array_slot_0, int& rank_0, int * index_values_0, int& size_0, int * extents_0, double * block_data_0, 
int& array_slot_1, int& rank_1, int * index_values_1, int& size_1, int * extents_1, double * block_data_1, 
int& ierr);

void compute_int_scratchmem(int& array_slot_0, int& rank_0, int * index_values_0, int& size_0, int * extents_0, double * data_0,
        int& array_slot_1, int& rank_1, int * index_values_1, int& size_1, int * extents_1, double * data_1, int& array_slot_2, int& rank_2, int * index_values_2, int& size_2, int * extents_2, double * data_2, int& ierr);

void energy_denominator_rhf(int& array_slot_0, int& rank_0, int * index_values_0, int& size_0, int * extents_0, double * data_0,
        int& array_slot_1, int& rank_1, int * index_values_1, int& size_1, int * extents_1, double * data_1, int& ierr);

void return_vpq(int& array_slot_0, int& rank_0, int * index_values_0, int& size_0, int * extents_0, double * data_0,
        int& array_slot_1, int& rank_1, int * index_values_1, int& size_1, int * extents_1, double * data_1, int& ierr);

void return_diagonal(int& array_slot_0, int& rank_0, int * index_values_0, int& size_0, int * extents_0, double * data_0, 
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

void aoladder_contraction(
        int& array_slot_1, int& rank_1, int * index_values_1, int& size_1, int * extents_1, double * data_1, 
        int& array_slot_2, int& rank_2, int * index_values_2, int& size_2, int * extents_2, double * data_2, 
        int& array_slot_3, int& rank_3, int * index_values_3, int& size_3, int * extents_3, double * data_3, int& ierr);

void compute_nn_repulsion(
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

void disable_debug_print();
void enable_debug_print();


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


void SpecialInstructionManager::init_procmap(){
    // TEST  The next few instructions are used for testing
	procmap_["test_increment_counter"]=(fp0)&test_increment_counter;
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
	procmap_["scf_atom"]=(fp0)&scf_atom;
	procmap_["return_pairs"]=(fp0)&return_pairs;
	procmap_["return_h1frag"]=(fp0)&return_h1frag;
	procmap_["compute_int_scratchmem"]=(fp0)&compute_int_scratchmem;
	procmap_["energy_denominator_rhf"]=(fp0)&energy_denominator_rhf;
	procmap_["return_vpq"]=(fp0)&return_vpq;
	procmap_["return_diagonal"]=(fp0)&return_diagonal;
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
    procmap_["aoladder_contraction"]=(fp0)&aoladder_contraction;
    procmap_["compute_nn_repulsion"]=(fp0)&compute_nn_repulsion;
    procmap_["scf_frag"]=(fp0)&scf_frag;
    procmap_["set_frag"]=(fp0)&set_frag;
    procmap_["frag_index_range"]=(fp0)&frag_index_range;
    procmap_["stripi"]=(fp0)&stripi;
    procmap_["set_ijk_aaa"]=(fp0)&set_ijk_aaa;
    procmap_["set_ijk_aab"]=(fp0)&set_ijk_aab;
    procmap_["swap_blocks"]=(fp0)&swap_blocks;

    procmap_["enable_debug_print"]=(fp0)&enable_debug_print;
    procmap_["disable_debug_print"]=(fp0)&disable_debug_print;


	//ADD STATEMENT TO ADD SPECIAL SUPERINSTRUCTION TO MAP HERE.  COPY ONE OF THE ABOVE LINES AND REPLACE THE
	//CHARACTERS IN QUOTES WITH THE (CASE SENSITIVE NAME USED IN SIAL PROGRAMS.  REPLACE THE CHARACTERS FOLLOWING
	//THE &WITH THE NAME IN THE C PROTOTYPE.
}

} /*namespace sip*/
