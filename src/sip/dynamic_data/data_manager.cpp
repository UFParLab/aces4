/*
 * data_manager.cpp
 *
 *  Created on: Aug 14, 2013
 *      Author: Beverly Sanders
 */

#include "data_manager.h"
#include "config.h"
#include <assert.h>
#include <sstream>
#include <climits>

#ifdef HAVE_MPI
#include "sip_mpi_data.h"
#include "mpi.h"
#include "sip_mpi_utils.h"
#endif

namespace sip {

const int DataManager::undefined_index_value = INT_MIN;
int DataManager::scope_count = 0;

#ifdef HAVE_MPI
DataManager::DataManager(SipTables& sipTables,
		SIPMPIAttr& sip_mpi_attr, DataDistribution& data_distribution):
//		pbm_read_(pbm_read), pbm_write_(pbm_write),
		sipTables_(sipTables), scalar_values_(sipTables.scalar_table_), /*initialize scalars from sipTables*/
		index_values_(sipTables.index_table_.num_indices(), undefined_index_value), /*initialize all index values to be undefined */
        block_manager_(sipTables, sip_mpi_attr, data_distribution),
        scalar_blocks_(sipTables.array_table_.entries_.size(),NULL),
        contiguous_array_manager_(sipTables, sipTables.setup_reader()),
        sip_mpi_attr_(sip_mpi_attr), data_distribution_(data_distribution)
        {
		for (int i = 0; i < sipTables_.array_table_.entries_.size(); ++i) {
			if (sipTables_.is_scalar(i)) {
				scalar_blocks_[i] = new Block(scalar_address(i));
			}
		}
}

#else
DataManager::DataManager(SipTables& sipTables,
//		pbm_read_(pbm_read), pbm_write_(pbm_write),
		sipTables_(sipTables), scalar_values_(sipTables.scalar_table_), /*initialize scalars from sipTables*/
		index_values_(sipTables.index_table_.num_indices(), undefined_index_value), /*initialize all index values to be undefined */
        block_manager_(sipTables, pbm_read, pbm_write),
        scalar_blocks_(sipTables.array_table_.entries_.size(),NULL),
        contiguous_array_manager_(sipTables, sipTables.setup_reader())
        {

	    for (int i = 0; i < sipTables_.array_table_.entries_.size(); ++i){
	    	if (sipTables_.is_scalar(i) ){
	    		scalar_blocks_[i] = new Block(scalar_address(i));
	    	}
	    }
}
#endif

DataManager::~DataManager() {
    for (int i = 0; i < sipTables_.array_table_.entries_.size(); ++i){
    	if (sipTables_.is_scalar(i) ){
    		delete scalar_blocks_[i];
    		scalar_blocks_[i] = NULL;
    	}
    }
}

double DataManager::scalar_value(int array_table_slot) {
	assert(sipTables_.is_scalar(array_table_slot));
	int scalar_table_slot = sipTables_.array_table_.scalar_selector(
			array_table_slot);
	return scalar_values_.at(scalar_table_slot);
}

double DataManager::scalar_value(const std::string& name) {
	int array_table_slot = sipTables_.array_table_.array_slot(name);
	return scalar_value(array_table_slot);
}

double* DataManager::scalar_address(int array_table_slot){
	return &(scalar_values_.at(
			sipTables_.array_table_.scalar_selector(array_table_slot)));
}

void DataManager::set_scalar_value(int array_table_slot, double value) {
	assert(sipTables_.is_scalar(array_table_slot));
	int scalar_table_slot = sipTables_.array_table_.scalar_selector(
			array_table_slot);
	scalar_values_[scalar_table_slot] = value;
}

void DataManager::set_scalar_value(const std::string& name, double value) {
	int array_table_slot = sipTables_.array_table_.array_slot(name);
	set_scalar_value(array_table_slot, value);
}


Block::BlockPtr DataManager::get_scalar_block(int array_table_slot){
	Block::BlockPtr b = scalar_blocks_.at(array_table_slot);
	check(b != NULL, "scalar in block wrapper not found");
	return b;
}

int DataManager::int_value(int int_table_slot) {
	return sipTables_.int_table_.value(int_table_slot);
}

int DataManager::index_value(int index_table_slot) {
	return index_values_.at(index_table_slot);
}
std::string DataManager::index_value_to_string(int index_table_slot) {
	int value = index_value(index_table_slot);
	std::stringstream ss; //create a stringstream
	ss << value; //add number to the stream
	return ss.str(); //return a string with the contents of the stream
}

//bool is_valid(int index_table_slot, int value);

void DataManager::set_index_value(int index_table_slot, int value) {
	index_values_[index_table_slot] = value;
}
int DataManager::inc_index(int index_table_slot) {
	int value = index_value(index_table_slot);
	set_index_value(index_table_slot, value + 1);
	return value + 1;
}
void DataManager::set_index_undefined(int index_table_slot) {
	index_values_[index_table_slot] = undefined_index_value;
}

//for arrays and blocks
BlockId DataManager::block_id(const BlockSelector& selector) {
	int array_id = selector.array_id_;
	int rank = sipTables_.array_table_.rank(array_id);
	index_value_array_t index_values;
	for (int i = 0; i < MAX_RANK; ++i) {
		int index_slot = selector.index_ids_[i];
		if (index_slot == wild_card_slot){
			index_values[i] = wild_card_value;
		}
		else if (index_slot == unused_index_slot) {
			index_values[i] = unused_index_value;
		} else {
			index_values[i] = index_values_[selector.index_ids_[i]];
		}
	}
	if (is_subblock(selector)){
		return BlockId(array_id, index_values, super_block_id(selector));
	}
	return BlockId(array_id, index_values);
}


/** Determine if selected block  is a subblock */
bool DataManager::is_subblock(const BlockSelector& selector){
	int array_id = selector.array_id_;
	int rank = sipTables_.array_table_.rank(array_id);
	for (int i = 0; i < rank; ++i){
		int slot = selector.index_ids_[i];
		int index_slot = (slot == wild_card_slot) ?
				sipTables_.array_table_.index_selectors(array_id)[i]:
				slot;
		if (sipTables_.index_table_.index_type(index_slot) == subindex) return true;
	}
	return false;
}

/** Determines if selected block is a block of a static array.  This will be the case if the
 * selector rank is 0 while the declared rank > 0
 */
bool DataManager::is_complete_contiguous_array(const BlockSelector& selector){
	int array_id = selector.array_id_;
	int rank = sipTables_.array_table_.rank(array_id);
	return selector.rank_== 0 && rank > 0;
}

/** The current implementation only allows one level of nesting for subblocks */
BlockId DataManager::super_block_id(const BlockSelector& subblock_selector){
	int array_id = subblock_selector.array_id_;
	int rank = sipTables_.array_table_.rank(array_id);
	int index_values[MAX_RANK];
	for (int i = 0; i < MAX_RANK; ++i) {
		int index_slot = subblock_selector.index_ids_[i];
			if (index_slot == unused_index_slot) {
				index_values[i] = unused_index_value;
			} else if (sipTables_.index_table_.index_type(index_slot) != subindex){ //this index is not a subindex
					index_values[i] = index_values_[index_slot]; //just lookup the  value, should be the same as the subblock id
			}
			else{  //this is a subindex, look up the parent index's current value
				int parent_index_slot = sipTables_.index_table_.parent(index_slot);
				check(sipTables_.index_table_.index_type(parent_index_slot) != subindex, "current implementation only supports one level of subindices");
				index_values[i] = index_values_[parent_index_slot];
		     }
	}
	return BlockId(array_id, index_values);

}
//    void get_subblock_offsets_and_shape(const array::BlockShape& super_block_shape, const array::BlockSelector& subblock_selector,
//array::offset_array_t& offsets, array::BlockShape& subblock_shape);

void DataManager::get_subblock_offsets_and_shape(Block::BlockPtr super_block, const BlockSelector& subblock_selector,
		offset_array_t& offsets, BlockShape& subblock_shape){
	   BlockShape super_block_shape = super_block->shape();
	   int sub_array_id = subblock_selector.array_id_;
	   int rank = sipTables_.array_table_.rank(sub_array_id);
	   for (int i = 0; i < rank; ++i){
		   int index_slot = subblock_selector.index_ids_[i];
		   if (sipTables_.index_table_.index_type(index_slot) != subindex){ //not a subindex
			   offsets[i] = 0;
			   subblock_shape.segment_sizes_[i]  = super_block_shape.segment_sizes_[i];
		   }
		   else {
			   int parent_index_slot = sipTables_.index_table_.parent(index_slot);
			   int parent_segment = index_values_[parent_index_slot];
			   int subsegment = index_values_[index_slot];
//			   offsets[i] = sipTables_.index_table_.entries_.at(index_slot).segment_descriptor_ptr_->offset(parent_segment, subsegment);
			   offsets[i] = sipTables_.index_table_.subsegment_offset(index_slot, parent_segment, subsegment);
//			   std:: cout << "getting ready to compute tmp=" << std::endl;
			   int tmp = sipTables_.index_table_.subsegment_extent(index_slot, parent_segment, subsegment);
//			   std:: cout << "tmp=" << tmp << std::endl;
			   subblock_shape.segment_sizes_[i] = tmp;
//			   subblock_shape.segment_sizes_[i] = sipTables_.index_table_.entries_.at(index_slot).segment_descriptor_ptr_->extent(parent_segment,subsegment);
		   }
	   }
//	   std::cout << "DataManager::get_subblock_offsets_and_shape: offsets = [" ;
//	   for (int i = 0; i < rank; ++i) {
//		   std::cout << ((i == 0)?"":",") << offsets[i];
//	   }
//	   std::cout << "], subblock_shape = " << subblock_shape << std::endl;
}

////returns the id of the superblock and calculates the offsets array in the reference variable
//array::BlockId DataManager::super_block_id(const array::BlockSelector& selector, int (& offset)[MAX_RANK]){
//	int array_id = selector.array_id_;
//	int rank = sipTables_.array_table_.rank(array_id);
//	int index_values[MAX_RANK];
//	for (int i = 0; i < MAX_RANK; ++i) {
//		int index_slot = selector.index_ids_[i];
//		if (index_slot == array::unused_index_slot) {
//			index_values[i] = array::unused_index_value;
//			offset[i] = 0;
//		} else {  //index is used
//			if (sipTables_.index_table_.index_type(index_slot) != array::subindex){ //this index is not a subindex, the offset is 0
//				index_values[i] = index_values_[selector.index_ids_[i]];
//
//
//
//				//this is a subindex
//				int parent_index_slot = sipTables_.index_table_.parent(index_slot);
//				int parent_segment = index_values_[parent_index_slot];
//				int subsegment = index_values_[index_slot];
//				int offset[i] = sipTables_.index_table_.entries_.at(index_slot).segment_descriptor_ptr_->
//
//			}
//			else{ //this is not a subindex
//			index_values[i] = index_values_[selector.index_ids_[i]];
//			offset[i] = 0;
//			}
//		}
//	}

//	std::cout << "returning array_id="<< array_id << " from DataManager::block_id" << std::endl;
//	return array::BlockId(array_id, index_values);
//}


//FOR PARALLEL VERSION REPLACE WIH CALL TO MPI_ALL_REDUCE
void DataManager::collective_sum(int source_array_slot,
		int dest_array_slot) {
	double val_source = scalar_value(source_array_slot);
	double val_dest = scalar_value(dest_array_slot);
	double summed = 0.0;

#ifdef HAVE_MPI
	if (sip_mpi_attr_.num_workers() > 1) {
		double to_send = val_source + val_dest;
		MPI_Comm& worker_comm = sip_mpi_attr_.company_communicator();
		SIPMPIUtils::check_err(MPI_Allreduce(&to_send, &summed, 1, MPI_DOUBLE, MPI_SUM, worker_comm));
	} else {
		summed = val_source + val_dest;
	}
#else
	summed = val_source + val_dest;
#endif
	set_scalar_value(dest_array_slot, summed);
}


//void DataManager::set_persistent_array(int array_id, std::string name, int slot){
////#ifdef HAVE_MPI
////	bool is_remote = sipTables_.is_distributed(array_id) || sipTables_.is_served(array_id);
////	if (is_remote){
////
////		int global_rank = sip_mpi_attr_.global_rank();
////		int global_size = sip_mpi_attr_.global_size();
////		bool am_worker_to_communicate = RankDistribution::is_local_worker_to_communicate(global_rank, global_size);
////
////		if (am_worker_to_communicate){
////
////			int local_server = RankDistribution::local_server_to_communicate(global_rank, global_size);
////			if (local_server > -1){
////				SIP_LOG(std::cout<< "W " << sip_mpi_attr_.global_rank() << " : Sending SET_PERSISTENT to server "<< local_server<< std::endl);
////
////				int save_persistent_tag = SIPMPIUtils::make_mpi_tag(SIPMPIData::SAVE_PERSISTENT, section_number_, message_number_);
////
////				// Send array_id & label
////				int send_buffer[2];
////				send_buffer[0] = array_id;
////				send_buffer[1] = slot;
////				SIPMPIUtils::check_err(MPI_Send(send_buffer, 2, MPI_INT, local_server, save_persistent_tag, MPI_COMM_WORLD));
////
////				int ack_tag = SIPMPIUtils::make_mpi_tag(SIPMPIData::SAVE_PERSISTENT_ACK, section_number_, message_number_);
////				SIPMPIUtils::expect_ack_from_rank(local_server, SIPMPIData::SAVE_PERSISTENT_ACK, ack_tag);
////				message_number_++;
////				SIP_LOG(std::cout<< "W " << sip_mpi_attr_.global_rank() << " : Done with SET_PERSISTENT from server "<< local_server<< std::endl);
////
////			}
////		}
////	} else {
//////		pbm_write_.mark_persistent_array(array_id, name);
////	}
////#else
////	pbm_write_.mark_persistent_array(array_id, name);
////#endif
//}
//
//void DataManager::restore_persistent_array(int array_id, std::string name, int slot){
//
////	bool is_contig = sipTables_.is_contiguous(array_id);
////	if (is_contig){
////		Block::BlockPtr bptr = pbm_read_.get_saved_contiguous_array(name);
////		check_and_warn(bptr != NULL, "Contiguous Array null when restoring !");
////		Block::BlockPtr bptr_clone = bptr->clone();
////		contiguous_array_manager_.create_contiguous_array(array_id, bptr_clone);
////	} else {
////#ifdef HAVE_MPI
////		bool is_remote = sipTables_.is_distributed(array_id) || sipTables_.is_served(array_id);
////		check (is_remote, "Trying to restore an array that is neither contiguous, nor remote!");
////
////		int global_rank = sip_mpi_attr_.global_rank();
////		int global_size = sip_mpi_attr_.global_size();
////		bool am_worker_to_communicate = RankDistribution::is_local_worker_to_communicate(global_rank, global_size);
////
////		if (am_worker_to_communicate){
////			int local_server = RankDistribution::local_server_to_communicate(global_rank, global_size);
////			if (local_server > -1){
////				SIP_LOG(std::cout<< "W " << sip_mpi_attr_.global_rank() << " : Sending RESTORE_PERSISTENT to server "<< local_server<< std::endl);
////
////				// Send array_id  & slot to server master
////				int send_buffer[2];
////				send_buffer[0] = array_id;
////				send_buffer[1] = slot;
////				int restore_persistent_tag = SIPMPIUtils::make_mpi_tag(SIPMPIData::RESTORE_PERSISTENT, section_number_, message_number_);
////				SIPMPIUtils::check_err(MPI_Send(send_buffer, 2, MPI_INT, local_server, restore_persistent_tag, MPI_COMM_WORLD));
////
////				int ack_tag = SIPMPIUtils::make_mpi_tag(SIPMPIData::RESTORE_PERSISTENT_ACK, section_number_, message_number_);
////				SIPMPIUtils::expect_ack_from_rank(local_server, SIPMPIData::RESTORE_PERSISTENT_ACK, ack_tag);
////				message_number_++;
////
////				SIP_LOG(std::cout<< "W " << sip_mpi_attr_.global_rank() << " : Done with SET_PERSISTENT from server "<< local_server<< std::endl);
////			}
////
////		}
////
////#else
////		BlockManager::IdBlockMapPtr bid_map = pbm_read_.get_saved_dist_array(name);
////		block_manager_.restore_distributed(array_id, bid_map);
////#endif
////	}
//}
//
//void DataManager::save_persistent_arrays(){
//	block_manager_.save_persistent_dist_arrays();
//	contiguous_array_manager_.save_persistent_contig_arrays();
//}



void DataManager::enter_scope(){scope_count++;  block_manager_.enter_scope();}
void DataManager::leave_scope(){scope_count--;  block_manager_.leave_scope();}


} /* namespace sip */
