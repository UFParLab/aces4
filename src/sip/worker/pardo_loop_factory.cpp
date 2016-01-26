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
//const PardoLoopFactory::Loop_t PardoLoopFactory::default_loop = PardoLoopFactory::StaticTaskAllocParallelPardoLoop;
const PardoLoopFactory::Loop_t PardoLoopFactory::default_loop = PardoLoopFactory::BalancedTaskAllocParallelPardoLoop;
const PardoLoopFactory::Loop_t PardoLoopFactory::test_loop = PardoLoopFactory::TestStaticTaskAllocParallelPardoLoop;
#else
const PardoLoopFactory::Loop_t PardoLoopFactory::default_loop = PardoLoopFactory::SequentialPardoLoop;
const PardoLoopFactory::Loop_t PardoLoopFactory::test_loop = PardoLoopFactory::SequentialPardoLoop;
#endif


/**
 * Map from pragma text as it should appear in the sial program to enum Loop_t value.
 */
std::map<std::string, enum PardoLoopFactory::Loop_t> PardoLoopFactory::pardo_variant_map= create_map<std::string, enum PardoLoopFactory::Loop_t>::create_map
("default_loop_manager",PardoLoopFactory::default_loop)
("SequentialPardoLoop",PardoLoopFactory::SequentialPardoLoop)
#ifdef HAVE_MPI
("StaticTaskAllocParallelPardoLoop",PardoLoopFactory::StaticTaskAllocParallelPardoLoop)
("BalancedTaskAllocParallelPardoLoop",PardoLoopFactory::BalancedTaskAllocParallelPardoLoop)
("Frag{i}{aa}{}", PardoLoopFactory::Fragment_i_aa__PardoLoopManager)
("Frag{Nij}{aa}{}", PardoLoopFactory::Fragment_Nij_aa__PardoLoopManager)
("Frag{Nij}{a}{a}", PardoLoopFactory::Fragment_Nij_a_a_PardoLoopManager)
("Frag{Nij}{o}{o}", PardoLoopFactory::Fragment_Nij_o_o_PardoLoopManager)
("Frag{Nij}{oo}{}", PardoLoopFactory::Fragment_Nij_oo__PardoLoopManager)
("Frag{ij}{aa}{a}", PardoLoopFactory::Fragment_ij_aa_a_PardoLoopManager)
("Frag{ij}{aaa}{}", PardoLoopFactory::Fragment_ij_aaa__PardoLoopManager)
("Frag{ij}{ao}{ao}",PardoLoopFactory::Fragment_ij_ao_ao_PardoLoopManager)
("Frag{ij}{aa}{oo}",PardoLoopFactory::Fragment_ij_aa_oo_PardoLoopManager)
("Frag{ij}{aa}{vo}",PardoLoopFactory::Fragment_ij_aa_vo_PardoLoopManager)
("Frag{ij}{aoa}{o}",PardoLoopFactory::Fragment_ij_aoa_o_PardoLoopManager)
("Frag{ij}{aoo}{o}",PardoLoopFactory::Fragment_ij_aoo_o_PardoLoopManager)
("Frag{i}{aaoo}{}",PardoLoopFactory::Fragment_i_aaoo__PardoLoopManager)
("Frag{i}{aovo}{}",PardoLoopFactory::Fragment_i_aovo__PardoLoopManager)
("Frag{i}{aaaa}{}",PardoLoopFactory::Fragment_i_aaaa__PardoLoopManager)
("Frag{i}{aoo}{}",PardoLoopFactory::Fragment_i_aoo__PardoLoopManager)
("Frag{NRij}{ao}{ao}",PardoLoopFactory::Fragment_NRij_ao_ao_PardoLoopManager)
("Frag{NRij}{vo}{ao}",PardoLoopFactory::Fragment_NRij_vo_ao_PardoLoopManager)
("Frag{NRij}{aa}{aa}",PardoLoopFactory::Fragment_NRij_aa_aa_PardoLoopManager)
("Frag{NRij}{o}{ao}",PardoLoopFactory::Fragment_NRij_o_ao_PardoLoopManager)
("Frag{i}{pppp}{}",PardoLoopFactory::Fragment_i_pppp__PardoLoopManager)
("Frag{i}{pp}{}",PardoLoopFactory::Fragment_i_pp__PardoLoopManager)
("Frag{ij}{ap}{pp}",PardoLoopFactory::Fragment_ij_ap_pp_PardoLoopManager)
("Frag{ij}{pp}{pp}",PardoLoopFactory::Fragment_ij_pp_pp_PardoLoopManager)
("Frag{Nij}{pp}{pp}",PardoLoopFactory::Fragment_Nij_pp_pp_PardoLoopManager)
("Frag{Rij}{pp}{pp}",PardoLoopFactory::Fragment_Rij_pp_pp_PardoLoopManager)
("Frag{NRij}{pp}{pp}",PardoLoopFactory::Fragment_NRij_pp_pp_PardoLoopManager)
#endif
("test_pardo_pragma",PardoLoopFactory::test_loop);



	LoopManager* PardoLoopFactory::make_loop_manager(std::string pragma, int num_indices,
			const int (&index_ids)[MAX_RANK], DataManager & data_manager,
			const SipTables & sip_tables, SIPMPIAttr& sip_mpi_attr,
			int num_where_clauses, Interpreter* interpreter, long& iteration){
		int which_pardo = -1;
//		try{
//			which_pardo = pardo_variant_map.at(pragma);
//		} catch (const std::out_of_range& oor){
//#ifdef HAVE_MPI
//			std::stringstream ss;
//			ss << " Failed to create specified pardo manager." ;
//			ss << " This is probably due to an incorrect pardo loop pragma.";
//			ss << " THIS PARDO LOOP IS BEING EXECUTED USING THE DEFAULT LOOP";
//			sial_warn(false, ss.str(), interpreter->line_number());
//#endif //HAVE_MPI
////if single node, and asking for non-existent loop manager, just give default without complaining
//			which_pardo = default_loop;
//		}

		std::map<std::string, enum Loop_t>::iterator it = pardo_variant_map.find(pragma);
		if (it == pardo_variant_map.end()){
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

		else{
			which_pardo = it->second;
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
		case Fragment_Nij_oo__PardoLoopManager:
			return new sip::Fragment_Nij_oo__PardoLoopManager(
			num_indices, index_ids, data_manager, sip_tables,
			sip_mpi_attr, num_where_clauses, interpreter, iteration);
		case Fragment_Nij_o_o_PardoLoopManager:
			return new sip::Fragment_Nij_o_o_PardoLoopManager(
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
		case Fragment_ij_aoo_o_PardoLoopManager:
			return new sip::Fragment_ij_aoo_o_PardoLoopManager(num_indices, index_ids, data_manager, sip_tables,
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
		case Fragment_NRij_ao_ao_PardoLoopManager:
			return new sip::Fragment_NRij_ao_ao_PardoLoopManager(num_indices, index_ids, data_manager, sip_tables,
			sip_mpi_attr, num_where_clauses, interpreter, iteration);
		case Fragment_NRij_vo_ao_PardoLoopManager:
			return new sip::Fragment_NRij_vo_ao_PardoLoopManager(num_indices, index_ids, data_manager, sip_tables,
			sip_mpi_attr, num_where_clauses, interpreter, iteration);
		case Fragment_NRij_aa_aa_PardoLoopManager:
			return new sip::Fragment_NRij_aa_aa_PardoLoopManager(num_indices, index_ids, data_manager, sip_tables,
			sip_mpi_attr, num_where_clauses, interpreter, iteration);
		case Fragment_NRij_o_ao_PardoLoopManager:
			return new sip::Fragment_NRij_o_ao_PardoLoopManager(num_indices, index_ids, data_manager, sip_tables,
			sip_mpi_attr, num_where_clauses, interpreter, iteration);
        case Fragment_i_pp__PardoLoopManager:
            return new sip::Fragment_i_pp__PardoLoopManager(num_indices, index_ids, data_manager, sip_tables,
                sip_mpi_attr, num_where_clauses, interpreter, iteration);
        case Fragment_i_pppp__PardoLoopManager:
            return new sip::Fragment_i_pppp__PardoLoopManager(num_indices, index_ids, data_manager, sip_tables,
            sip_mpi_attr, num_where_clauses, interpreter, iteration);
        case Fragment_ij_ap_pp_PardoLoopManager:
            return new sip::Fragment_ij_ap_pp_PardoLoopManager(num_indices, index_ids, data_manager, sip_tables,
            sip_mpi_attr, num_where_clauses, interpreter, iteration);
        case Fragment_ij_pp_pp_PardoLoopManager:
            return new sip::Fragment_ij_pp_pp_PardoLoopManager(num_indices, index_ids, data_manager, sip_tables,
            sip_mpi_attr, num_where_clauses, interpreter, iteration);
        case Fragment_Nij_pp_pp_PardoLoopManager:
            return new sip::Fragment_Nij_pp_pp_PardoLoopManager(num_indices, index_ids, data_manager, sip_tables,
                sip_mpi_attr, num_where_clauses, interpreter, iteration);
        case Fragment_Rij_pp_pp_PardoLoopManager:
            return new sip::Fragment_Rij_pp_pp_PardoLoopManager(num_indices, index_ids, data_manager, sip_tables,
                sip_mpi_attr, num_where_clauses, interpreter, iteration);
        case Fragment_NRij_pp_pp_PardoLoopManager:
            return new sip::Fragment_NRij_pp_pp_PardoLoopManager(num_indices, index_ids, data_manager, sip_tables,
            sip_mpi_attr, num_where_clauses, interpreter, iteration);
#endif
		default:
			CHECK_WITH_LINE(false, "cannot create requested pardo loop manager", interpreter->line_number());
			return NULL; //should not get here, this will cause a crash
		}
	}

} /* namespace sip */


