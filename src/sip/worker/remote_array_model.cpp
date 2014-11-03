/*
 * remote_array_model.cpp
 *
 *  Created on: Oct 29, 2014
 *      Author: njindal
 */

#include "remote_array_model.h"
#include "block_id.h"
#include "sip_tables.h"

namespace sip {

RemoteArrayModel::RemoteArrayModel(const SipTables& sip_tables,
		const Parameters& parameters) :
		sip_tables_(sip_tables), parameters_(parameters) {
}

RemoteArrayModel::~RemoteArrayModel() {}

double RemoteArrayModel::time_to_get_block(const BlockId& block_id) const {
	// TODO add hierarchical parameters.
	// TODO add time for disk reads/caching
	// TODO account for caching at worker.
	std::size_t block_size = sip_tables_.block_size(block_id) * sizeof(double); // in bytes
	double time = 2 * parameters_.t_s + block_size / (double) parameters_.b;
	return time;
}

double RemoteArrayModel::time_to_put_replace_block(const BlockId& block_id) const {
	// TODO add hierarchical parameters.
	// TODO add time for disk reads/caching
	// TODO account for caching at worker.
	std::size_t block_size = sip_tables_.block_size(block_id) * sizeof(double); // in bytes
	double time = 2 * parameters_.t_s + block_size / (double) parameters_.b;
	return time;
}

double RemoteArrayModel::time_to_put_accumulate_block(const BlockId& block_id) const {
	// TODO add hierarchical parameters.
	// TODO add time for disk reads/caching
	// TODO account for caching at worker.
	// TODO add time to do summing up.
	std::size_t block_size = sip_tables_.block_size(block_id) * sizeof(double); // in bytes
	double time = 2 * parameters_.t_s + block_size / (double) parameters_.b;
	return time;
}

double RemoteArrayModel::time_to_create_array(int array_id) const{
	return 0.0; 			// Currently CREATE array is a no-op
}

double RemoteArrayModel::time_to_delete_array(int array_id) const {
	return parameters_.t_s;	// Time to send a small message.
}

} /* namespace sip */
