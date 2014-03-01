/*
 * data_distribution.h
 *
 *  Created on: Jan 16, 2014
 *      Author: njindal
 */

#ifndef DATA_DISTRIBUTION_H_
#define DATA_DISTRIBUTION_H_

#include "blocks.h"
#include "sip_tables.h"
#include "sip_mpi_attr.h"

namespace sip {

/**
 * Decides distribution of block (statically)
 */
class DataDistribution {
public:
	DataDistribution(SipTables&, SIPMPIAttr&);
	virtual ~DataDistribution();

	/**
	 * Calculates and returns MPI rank of server that "owns" a given block.
	 * @param
	 * @return
	 */
	int get_server_rank(const sip::BlockId&) const;

private:

	SipTables& sip_tables_;
	SIPMPIAttr& sip_mpi_attr_;

};

} /* namespace sip */

#endif /* DATA_DISTRIBUTION_H_ */
