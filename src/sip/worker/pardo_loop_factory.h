/*
 * pardo_loop_factory.h
 *
 *  Created on: Sep 19, 2015
 *      Author: basbas
 */

#ifndef PARDO_LOOP_FACTORY_H_
#define PARDO_LOOP_FACTORY_H_

#include "sip.h"

namespace sip {

class DataManager;
class SipTables;
class Interpreter;
class SIPMPIAttr;
class LoopManager;


/**
 * This class provides static methods to create specialized pardo loop managers.
 * Currently, a loop manager is requested by appending a string to the pardo command in the sial program.
 *
 *  pardo i,j,k,l  "my_loop_manager"
 *
 *
 *  More formally:
 *
 *  Statement ::= pardo Indices PardoPragma EOLs$
	 WhereClauseList
	 StatementList
	 endpardo Indices

	 PardoPragma ::= %empty | StringLiteral



 *  The case sensitive text of the pragma is used as a key in the pardo_variant_map, which returns
 *  a value from the Loop_t enum.  This value is used in a switch statement to select the action
 *  which currently is returning a LoopManager object of the requested type.
 *
 *  In parallel versions, failure to find a key matching the pragma text results in a warning, and return of the default LoopManager
 *  In single node versions, this simply returns the default without warning.
 *
 *  To add a new PardoLoop Manager,
 *  1.  create an entry (usually just the class name) in the Loop_t enum.
 *  2.  add a (pragma text, Loop_t value) pair to the pardo_variant_map initializer (in pardo_loop_factory.cpp)
 *  3.  add a case for the Loop_t value to the switch statement in the make_loop_manager method (in pardo_loop_factory.cpp).
 *
 *
 */
class PardoLoopFactory {
public:
	enum Loop_t {
		SequentialPardoLoop,
		TestSequentialPardoLoop,
		TestStaticTaskAllocParallelPardoLoop,
		StaticTaskAllocParallelPardoLoop,
		BalancedTaskAllocParallelPardoLoop,
		Fragment_Nij_aa__PardoLoopManager,
		Fragment_Nij_a_a_PardoLoopManager,
		Fragment_Nij_o_o_PardoLoopManager,
		Fragment_Nij_oo__PardoLoopManager,
		Fragment_ij_aa_a_PardoLoopManager,
		Fragment_ij_aaa__PardoLoopManager,
		Fragment_ij_ao_ao_PardoLoopManager,
		Fragment_ij_aa_oo_PardoLoopManager,
		Fragment_ij_aa_vo_PardoLoopManager,
		Fragment_ij_aoa_o_PardoLoopManager,
		Fragment_ij_aoo_o_PardoLoopManager,
		Fragment_i_aaoo__PardoLoopManager,
		Fragment_i_aovo__PardoLoopManager,
		Fragment_i_aaaa__PardoLoopManager,
		Fragment_i_aoo__PardoLoopManager,
		Fragment_NRij_ao_ao_PardoLoopManager,
		Fragment_NRij_vo_ao_PardoLoopManager,
		Fragment_NRij_aa_aa_PardoLoopManager,
		Fragment_NRij_o_ao_PardoLoopManager,
		WhereFragment_i_aaa__PardoLoopManager,
		WhereFragment_i_aaaa__PardoLoopManager,
		WhereFragment_ij_aa_aa_PardoLoopManager,
		Fragment_Nij_aa_aa_PardoLoopManager,
		Fragment_i_aa__PardoLoopManager,
		Fragment_i_ap__PardoLoopManager,
		Fragment_i_pp__PardoLoopManager,
		Fragment_i_pppp__PardoLoopManager,
		Fragment_ij_ap_pp_PardoLoopManager,
		Fragment_ij_pp_pp_PardoLoopManager,
		Fragment_Nij_pp_pp_PardoLoopManager,
		Fragment_Rij_pp_pp_PardoLoopManager,
		Fragment_NRij_pp_pp_PardoLoopManager,
    };

	/**
	 * Returns a loop manager as specified by the given pragma.  If pragma = "" the default manager is returned.
	 * @param pragma
	 * @param num_indices
	 * @param index_ids
	 * @param data_manager
	 * @param sip_tables
	 * @param sip_mpi_attr
	 * @param num_where_clauses
	 * @param interpreter
	 * @param iteration
	 * @return
	 */
	static LoopManager* make_loop_manager(std::string pragma, int num_indices,
			const int (&index_ids)[MAX_RANK], DataManager & data_manager,
			const SipTables & sip_tables, SIPMPIAttr& sip_mpi_attr,
			int num_where_clauses, Interpreter* interpreter, long& iteration);

	static const Loop_t default_loop;
	static std::map<std::string, enum Loop_t> pardo_variant_map;

	static const Loop_t test_loop;

private:

	DISALLOW_COPY_AND_ASSIGN (PardoLoopFactory);

};



} /* namespace sip */

#endif /* PARDO_LOOP_FACTORY_H_ */
