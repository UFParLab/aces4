/*
 * check_block_number_calculation.cpp
 *
 *  Created on: Nov 23, 2015
 *      Author: basbas
 */

#include "sip.h"
#include "aces_defs.h"
#include "sip_tables.h"
#include "block_id.h"
#include "block.h"
#include "interpreter.h"
#include "data_manager.h"
#include "block_manager.h"
//declare as special check_block_number_calculation w

//Note that this should only be used in test programs.  Declaring it w omits having
//to create it beforehand.  Also, note that this instruction actually deletes the block
//after it is finished with it.  This is time consuming, but allows the associated
//machinery to be tested as well.


void check_block_number_calculation(int& array_slot, int& rank,
		int* index_values, int& size, int* extents,  double* data, int& ierr){
	ierr = 0;
	int index_values_copy[MAX_RANK];
	for (int k = 0; k < MAX_RANK; ++k){
		index_values_copy[k] = index_values[k];
	}
//	if (index_values_copy[0] == 8 && index_values_copy[1] == 1){
//			    {
//			        int i = 0;
//			        char hostname[256];
//			        gethostname(hostname, sizeof(hostname));
//			        printf("PID %d on %s ready for attach\n", getpid(), hostname);
//			        fflush(stdout);
//			        while (0 == i)
//			            sleep(5);
//			    }
//	}
	sip::BlockId block_id(array_slot, index_values_copy);
//	std::cout << "constructed block_id = " << block_id << std::endl << std::flush;

	    sip::Interpreter * interpreter = sip::Interpreter::global_interpreter;
	    size_t block_number = interpreter->sip_tables().block_number(block_id);
	 	sip::BlockId id2 = interpreter->sip_tables().block_id(block_id.array_id(),block_number);
	 	if (block_id != id2){
	 		std::stringstream ss;
	 		ss << "problem computing block number in block_cyclic_distribution_server_rank";
	 		ss << "block_id=" << block_id.str(interpreter->sip_tables()) << std::endl;
	 		ss << " block_number=" << block_number << std::endl;
	 		ss << " computed_id=" << id2.str(interpreter->sip_tables()) << std::endl;
	        sip::check(false, ss.str());
	 	}

		if (index_values_copy[0] == 8 && index_values_copy[1] == 1){
			std::cout << "PASSED PROBLEM CASE" << std::endl << std::flush;
		}

//        sip::Block::BlockPtr block = interpreter->sial_ops_.get_block_for_reading(block_id,interpreter->pc);
	 	const sip::DataManager& data_manager = interpreter->data_manager();
	 	sip::BlockManager& block_manager = const_cast<sip::BlockManager&>(data_manager.block_manager());
	 	block_manager.delete_block(block_id);
}


