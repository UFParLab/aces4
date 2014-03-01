/*
 * rank_distribution.h
 *
 *  Created on: Jan 14, 2014
 *      Author: njindal
 */

#ifndef RANK_DISTRIBUTION_H_
#define RANK_DISTRIBUTION_H_

#include "sip_mpi_attr.h"

namespace sip {

/**
 * Decides the distribution of servers in a SIP run
 */
class RankDistribution {
public:

	static bool is_server(int rank, int size);

private:
	/**
	 * Distributes worker and server in 3:1 ratio. For 2 ranks, 0 is worker and 1 is server.
	 * For other configs, every 3rd rank is a server.
	 * @param rank
	 * @param size
	 * @return
	 */
	static bool three_to_one_server(int rank, int size);

};

} /* namespace sip */

#endif /* RANK_DISTRIBUTION_H_ */
