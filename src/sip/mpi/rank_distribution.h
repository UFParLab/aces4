/*
 * rank_distribution.h
 *
 *  Created on: Jan 14, 2014
 *      Author: njindal
 */

#ifndef RANK_DISTRIBUTION_H_
#define RANK_DISTRIBUTION_H_

namespace sip {

/**
 * Decides the distribution of servers in a SIP run
 */
class RankDistribution {
public:

	virtual ~RankDistribution(){}

	/**
	 * Is this rank a server?
	 * @param rank
	 * @param size
	 * @return
	 */
	virtual bool is_server(int rank, int size) = 0;

	/**
	 * Which local server should this worker communicate with.
	 * If server rank to be communicated with is more than size, -1 is returned.
	 * @param rank
	 * @param size
	 * @return
	 */
	virtual int local_server_to_communicate(int rank, int size) = 0;

	/**
	 * Is this the worker that should communicate with the local server.
	 * @param rank
	 * @param size
	 * @return
	 */
	virtual bool is_local_worker_to_communicate(int rank, int size) = 0;


};

/**
 * Distributes worker and server in 2:1 ratio. For 2 ranks, 0 is worker and 1 is server.
 * For other configs, every 3rd rank is a server.
 */
class TwoWorkerOneServerRankDistribution : public RankDistribution {
public:
	virtual bool is_server(int rank, int size);
	virtual int local_server_to_communicate(int rank, int size);
	virtual bool is_local_worker_to_communicate(int rank, int size);
};

} /* namespace sip */

#endif /* RANK_DISTRIBUTION_H_ */
