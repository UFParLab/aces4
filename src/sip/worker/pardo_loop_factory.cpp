/*
 * pardo_loop_factory.cpp
 *
 *  Created on: Sep 19, 2015
 *      Author: basbas
 */

#include "pardo_loop_factory.h"
#include <map>
#include <string>
#include "create_map.h"
#include "fragment_loop_manager.h"
#include "interpreter.h"

namespace sip {


#ifdef HAVE_MPI
const PardoLoopFactory::Loop_t PardoLoopFactory::default_loop = PardoLoopFactory::StaticTaskAllocParallelPardoLoop;
const PardoLoopFactory::Loop_t PardoLoopFactory::test_loop = PardoLoopFactory::TestStaticTaskAllocParallelPardoLoop;
#else
//const PardoLoopFactory::Loop_t PardoLoopFactory::default_loop = PardoLoopFactory::SequentialPardoLoop;
#endif


/**
 * Map from pragma text as it should appear in the sial program to enum Loop_t value.
 */
std::map<std::string, enum PardoLoopFactory::Loop_t> PardoLoopFactory::pardo_variant_map= create_map<std::string, enum PardoLoopFactory::Loop_t>::create_map
("default_loop_manager",PardoLoopFactory::default_loop)
("SequentialPardoLoop",PardoLoopFactory::SequentialPardoLoop)
("test_pardo_pragma", PardoLoopFactory::test_loop)
#ifdef HAVE_MPI
("StaticTaskAllocParallelPardoLoop",PardoLoopFactory::StaticTaskAllocParallelPardoLoop)
("BalancedTaskAllocParallelPardoLoop",PardoLoopFactory::BalancedTaskAllocParallelPardoLoop)
("Fragment_i_aa__PardoLoopManager", PardoLoopFactory::Fragment_i_aa__PardoLoopManager)
("Fragment_Nij_aa__PardoLoopManager", PardoLoopFactory::Fragment_Nij_aa__PardoLoopManager)
("Fragment_Nij_a_a_PardoLoopManager", PardoLoopFactory::Fragment_Nij_a_a_PardoLoopManager)
("Fragment_ij_aa_a_PardoLoopManager", PardoLoopFactory::Fragment_ij_aa_a_PardoLoopManager)
("Fragment_ij_aaa__PardoLoopManager", PardoLoopFactory::Fragment_ij_aaa__PardoLoopManager)
("Fragment_ij_ao_ao_PardoLoopManager",PardoLoopFactory::Fragment_ij_ao_ao_PardoLoopManager)
("Fragment_ij_aa_oo_PardoLoopManager",PardoLoopFactory::Fragment_ij_aa_oo_PardoLoopManager)
("Fragment_ij_aa_vo_PardoLoopManager",PardoLoopFactory::Fragment_ij_aa_vo_PardoLoopManager)
("Fragment_ij_aoa_o_PardoLoopManager",PardoLoopFactory::Fragment_ij_aoa_o_PardoLoopManager)
("Fragment_ij_ao_vo_PardoLoopManager",PardoLoopFactory::Fragment_ij_ao_vo_PardoLoopManager)
("Fragment_ij_av_oo_PardoLoopManager",PardoLoopFactory::Fragment_ij_av_oo_PardoLoopManager)
("Fragment_ij_av_vo_PardoLoopManager",PardoLoopFactory::Fragment_ij_av_vo_PardoLoopManager)
("Fragment_ij_ao_oo_PardoLoopManager",PardoLoopFactory::Fragment_ij_ao_oo_PardoLoopManager)
("Fragment_ij_oo_ao_PardoLoopManager",PardoLoopFactory::Fragment_ij_oo_ao_PardoLoopManager)
("Fragment_ij_aoo_o_PardoLoopManager",PardoLoopFactory::Fragment_ij_aoo_o_PardoLoopManager)
("Fragment_ij_vo_vo_PardoLoopManager",PardoLoopFactory::Fragment_ij_vo_vo_PardoLoopManager)
("Fragment_i_vo__PardoLoopManager",PardoLoopFactory::Fragment_i_vo__PardoLoopManager)
("Fragment_i_vovo__PardoLoopManager",PardoLoopFactory::Fragment_i_vovo__PardoLoopManager)
("Fragment_i_aaoo__PardoLoopManager",PardoLoopFactory::Fragment_i_aaoo__PardoLoopManager)
("Fragment_i_aovo__PardoLoopManager",PardoLoopFactory::Fragment_i_aovo__PardoLoopManager)
("Fragment_i_aaaa__PardoLoopManager",PardoLoopFactory::Fragment_i_aaaa__PardoLoopManager)
("Fragment_i_aoo__PardoLoopManager",PardoLoopFactory::Fragment_i_aoo__PardoLoopManager)
("Fragment_Nij_vo_vo_PardoLoopManager",PardoLoopFactory::Fragment_Nij_vo_vo_PardoLoopManager)
("Fragment_NRij_vo_vo_PardoLoopManager",PardoLoopFactory::Fragment_NRij_vo_vo_PardoLoopManager)
("Fragment_NRij_ao_ao_PardoLoopManager",PardoLoopFactory::Fragment_NRij_ao_ao_PardoLoopManager)
("Fragment_NRij_vo_ao_PardoLoopManager",PardoLoopFactory::Fragment_NRij_vo_ao_PardoLoopManager)
("Fragment_NRij_aa_aa_PardoLoopManager",PardoLoopFactory::Fragment_NRij_aa_aa_PardoLoopManager)
("Fragment_NRij_vv_oo_PardoLoopManager",PardoLoopFactory::Fragment_NRij_vv_oo_PardoLoopManager)
("Fragment_NRij_o_ao_PardoLoopManager",PardoLoopFactory::Fragment_NRij_o_ao_PardoLoopManager)
("Fragment_Rij_vo_vo_PardoLoopManager",PardoLoopFactory::Fragment_Rij_vo_vo_PardoLoopManager)
("Fragment_NR1ij_vo_vo_PardoLoopManager",PardoLoopFactory::Fragment_NR1ij_vo_vo_PardoLoopManager)
("Fragment_NR1ij_oo_vo_PardoLoopManager",PardoLoopFactory::Fragment_NR1ij_oo_vo_PardoLoopManager)
("Fragment_NR1ij_vv_vo_PardoLoopManager",PardoLoopFactory::Fragment_NR1ij_vv_vo_PardoLoopManager);
#endif



	LoopManager* PardoLoopFactory::make_loop_manager(std::string pragma, int num_indices,
			const int (&index_ids)[MAX_RANK], DataManager & data_manager,
			const SipTables & sip_tables, SIPMPIAttr& sip_mpi_attr,
			int num_where_clauses, Interpreter* interpreter, long& iteration){
		int which_pardo = -1;
		try{
			which_pardo = pardo_variant_map.at(pragma);
		} catch (const std::out_of_range& oor){
#ifdef HAVE_MPI
			std::stringstream ss;
			ss << " Failed to create specified pardo manager." ;
			ss << " This is probably due to an incorrect pardo loop pragma.";
			ss << " THIS PARDO LOOP IS BEING EXECUTED USING THE DEFAULT LOOP";
			sial_warn(false, ss.str(), interpreter->line_number());
#endif //HAVE_MPI
//if single node, and asking for non-existent loop manager, just give default without complaining
			which_pardo = default_loop;
		}
		switch(which_pardo){
		case SequentialPardoLoop:
			return new sip::SequentialPardoLoop(num_indices, index_ids, data_manager, sip_tables);
//		case TestSequentialPardoLoop:
//			return new sip::TestSequentialPardoLoop(num_indices, index_ids, data_manager, sip_tables);
#ifdef HAVE_MPI
		case StaticTaskAllocParallelPardoLoop:
			return new sip::TestStaticTaskAllocParallelPardoLoop(
			num_indices, index_ids, data_manager,sip_tables, sip_mpi_attr);
		case TestStaticTaskAllocParallelPardoLoop:
			return new sip::StaticTaskAllocParallelPardoLoop(
			num_indices, index_ids, data_manager,sip_tables, sip_mpi_attr);
		case BalancedTaskAllocParallelPardoLoop:
			return  new sip::BalancedTaskAllocParallelPardoLoop(
				num_indices, index_ids, data_manager, sip_tables,
				sip_mpi_attr, num_where_clauses, interpreter, iteration);
		case Fragment_i_aa__PardoLoopManager:
			return new sip::Fragment_i_aa__PardoLoopManager(
					num_indices, index_ids, data_manager, sip_tables,
					sip_mpi_attr, num_where_clauses, interpreter, iteration);
		case Fragment_Nij_aa__PardoLoopManager:
			return new sip::Fragment_Nij_aa__PardoLoopManager(
			num_indices, index_ids, data_manager, sip_tables,
			sip_mpi_attr, num_where_clauses, interpreter, iteration);
		case Fragment_Nij_a_a_PardoLoopManager:
			return new sip::Fragment_Nij_a_a_PardoLoopManager(
			num_indices, index_ids, data_manager, sip_tables,
			sip_mpi_attr, num_where_clauses, interpreter, iteration);
		case Fragment_ij_aa_a_PardoLoopManager:
			return new sip::Fragment_ij_aa_a_PardoLoopManager(
			num_indices, index_ids, data_manager, sip_tables,
			sip_mpi_attr, num_where_clauses, interpreter, iteration);
		case Fragment_ij_aaa__PardoLoopManager:
			return new sip::Fragment_ij_aaa__PardoLoopManager(
			num_indices, index_ids, data_manager, sip_tables,
			sip_mpi_attr, num_where_clauses, interpreter, iteration);
		case Fragment_ij_ao_ao_PardoLoopManager:
			return new sip::Fragment_ij_ao_ao_PardoLoopManager(
					num_indices, index_ids, data_manager, sip_tables,
			sip_mpi_attr, num_where_clauses, interpreter, iteration);
		case Fragment_ij_aa_oo_PardoLoopManager:
			return new sip::Fragment_ij_aa_oo_PardoLoopManager(num_indices, index_ids, data_manager, sip_tables,
			sip_mpi_attr, num_where_clauses, interpreter, iteration);
		case Fragment_ij_aa_vo_PardoLoopManager:
			return new sip::Fragment_ij_aa_vo_PardoLoopManager(num_indices, index_ids, data_manager, sip_tables,
			sip_mpi_attr, num_where_clauses, interpreter, iteration);
		case Fragment_ij_aoa_o_PardoLoopManager:
			return new sip::Fragment_ij_aoa_o_PardoLoopManager(num_indices, index_ids, data_manager, sip_tables,
			sip_mpi_attr, num_where_clauses, interpreter, iteration);
		case Fragment_ij_ao_vo_PardoLoopManager:
			return new sip::Fragment_ij_ao_vo_PardoLoopManager(num_indices, index_ids, data_manager, sip_tables,
			sip_mpi_attr, num_where_clauses, interpreter, iteration);
		case Fragment_ij_av_oo_PardoLoopManager:
			return new sip::Fragment_ij_av_oo_PardoLoopManager(num_indices, index_ids, data_manager, sip_tables,
			sip_mpi_attr, num_where_clauses, interpreter, iteration);
		case Fragment_ij_av_vo_PardoLoopManager:
			return new sip::Fragment_ij_av_vo_PardoLoopManager(num_indices, index_ids, data_manager, sip_tables,
			sip_mpi_attr, num_where_clauses, interpreter, iteration);
		case Fragment_ij_ao_oo_PardoLoopManager:
			return new sip::Fragment_ij_ao_oo_PardoLoopManager(num_indices, index_ids, data_manager, sip_tables,
			sip_mpi_attr, num_where_clauses, interpreter, iteration);
		case Fragment_ij_oo_ao_PardoLoopManager:
			return new sip::Fragment_ij_oo_ao_PardoLoopManager(num_indices, index_ids, data_manager, sip_tables,
			sip_mpi_attr, num_where_clauses, interpreter, iteration);
		case Fragment_ij_aoo_o_PardoLoopManager:
			return new sip::Fragment_ij_aoo_o_PardoLoopManager(num_indices, index_ids, data_manager, sip_tables,
			sip_mpi_attr, num_where_clauses, interpreter, iteration);
		case Fragment_ij_vo_vo_PardoLoopManager:
			return new sip::Fragment_ij_vo_vo_PardoLoopManager(num_indices, index_ids, data_manager, sip_tables,
			sip_mpi_attr, num_where_clauses, interpreter, iteration);
		case Fragment_i_vo__PardoLoopManager:
			return new sip::Fragment_i_vo__PardoLoopManager(num_indices, index_ids, data_manager, sip_tables,
			sip_mpi_attr, num_where_clauses, interpreter, iteration);
		case Fragment_i_vovo__PardoLoopManager:
			return new sip::Fragment_i_vovo__PardoLoopManager(num_indices, index_ids, data_manager, sip_tables,
			sip_mpi_attr, num_where_clauses, interpreter, iteration);
		case Fragment_i_aaoo__PardoLoopManager:
			return new sip::Fragment_i_aaoo__PardoLoopManager(num_indices, index_ids, data_manager, sip_tables,
			sip_mpi_attr, num_where_clauses, interpreter, iteration);
		case Fragment_i_aovo__PardoLoopManager:
			return new sip::Fragment_i_aovo__PardoLoopManager(num_indices, index_ids, data_manager, sip_tables,
			sip_mpi_attr, num_where_clauses, interpreter, iteration);
		case Fragment_i_aaaa__PardoLoopManager:
			return new sip::Fragment_i_aaaa__PardoLoopManager(num_indices, index_ids, data_manager, sip_tables,
			sip_mpi_attr, num_where_clauses, interpreter, iteration);
		case Fragment_i_aoo__PardoLoopManager:
			return new sip::Fragment_i_aoo__PardoLoopManager(num_indices, index_ids, data_manager, sip_tables,
			sip_mpi_attr, num_where_clauses, interpreter, iteration);
		case Fragment_Nij_vo_vo_PardoLoopManager:
			return new sip::Fragment_Nij_vo_vo_PardoLoopManager(num_indices, index_ids, data_manager, sip_tables,
			sip_mpi_attr, num_where_clauses, interpreter, iteration);
		case Fragment_NRij_vo_vo_PardoLoopManager:
			return new sip::Fragment_NRij_vo_vo_PardoLoopManager(num_indices, index_ids, data_manager, sip_tables,
			sip_mpi_attr, num_where_clauses, interpreter, iteration);
		case Fragment_NRij_ao_ao_PardoLoopManager:
			return new sip::Fragment_NRij_ao_ao_PardoLoopManager(num_indices, index_ids, data_manager, sip_tables,
			sip_mpi_attr, num_where_clauses, interpreter, iteration);
		case Fragment_NRij_vo_ao_PardoLoopManager:
			return new sip::Fragment_NRij_vo_ao_PardoLoopManager(num_indices, index_ids, data_manager, sip_tables,
			sip_mpi_attr, num_where_clauses, interpreter, iteration);
		case Fragment_NRij_aa_aa_PardoLoopManager:
			return new sip::Fragment_NRij_aa_aa_PardoLoopManager(num_indices, index_ids, data_manager, sip_tables,
			sip_mpi_attr, num_where_clauses, interpreter, iteration);
		case Fragment_NRij_vv_oo_PardoLoopManager:
			return new sip::Fragment_NRij_vv_oo_PardoLoopManager(num_indices, index_ids, data_manager, sip_tables,
			sip_mpi_attr, num_where_clauses, interpreter, iteration);
		case Fragment_NRij_o_ao_PardoLoopManager:
			return new sip::Fragment_NRij_o_ao_PardoLoopManager(num_indices, index_ids, data_manager, sip_tables,
			sip_mpi_attr, num_where_clauses, interpreter, iteration);
		case Fragment_Rij_vo_vo_PardoLoopManager:
			return new sip::Fragment_Rij_vo_vo_PardoLoopManager(num_indices, index_ids, data_manager, sip_tables,
			sip_mpi_attr, num_where_clauses, interpreter, iteration);
		case Fragment_NR1ij_vo_vo_PardoLoopManager:
			return new sip::Fragment_NR1ij_vo_vo_PardoLoopManager(num_indices, index_ids, data_manager, sip_tables,
			sip_mpi_attr, num_where_clauses, interpreter, iteration);
		case Fragment_NR1ij_oo_vo_PardoLoopManager:
			return new sip::Fragment_NR1ij_vo_vo_PardoLoopManager(num_indices, index_ids, data_manager, sip_tables,
			sip_mpi_attr, num_where_clauses, interpreter, iteration);
		case Fragment_NR1ij_vv_vo_PardoLoopManager:
			return new sip::Fragment_NR1ij_vv_vo_PardoLoopManager(num_indices, index_ids, data_manager, sip_tables,
			sip_mpi_attr, num_where_clauses, interpreter, iteration);
#endif
		default:
			check(false, "cannot create requested pardo loop manager", interpreter->line_number());
			return NULL; //should not get here, this will cause a crash
		}
	}

} /* namespace sip */
