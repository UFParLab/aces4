/*
 * remote_array_model.h
 *
 *  Created on: Oct 29, 2014
 *      Author: njindal
 */

#ifndef REMOTE_ARRAY_MODEL_H_
#define REMOTE_ARRAY_MODEL_H_

#include "data_distribution.h"

namespace sip {

class BlockId;
class SipTables;
class SIPMPIAttr;

class RemoteArrayModel {
public:

	struct Parameters {
		double t_s;			//! Startup time in seconds
		double b;			//! Bandwidth in bytes per second
		Parameters(double startup_time, double bandwidth): t_s(startup_time), b(bandwidth) {}
	};

	RemoteArrayModel(const SipTables& sip_tables, const Parameters& parameters);
	~RemoteArrayModel();

	/**
	 * Time to GET/PREPARE a block
	 * @param block_id
	 * @return time in seconds
	 */
	double time_to_get_block(const BlockId& block_id) const ;

	/**
	 * Time to PUT/PREPARE REPLACE a block
	 * @param block_id
	 * @return time in seconds
	 */
	double time_to_put_replace_block(const BlockId& block_id) const ;

	/**
	 * Time to PUT/PREPARE SUM a block
	 * @param block_id
	 * @return time in seconds
	 */
	double time_to_put_accumulate_block(const BlockId& block_id) const;

	/**
	 * Time to create an array
	 * @param array_id
	 * @return
	 */
	double time_to_create_array(int array_id) const;

	/**
	 * Time to delete an array
	 * @param array_id
	 * @return
	 */
	double  time_to_delete_array(int array_id) const;

private:
	const SipTables& sip_tables_;
	const Parameters& parameters_;
};

} /* namespace sip */

#endif /* REMOTE_ARRAY_MODEL_H_ */
