/*
 * rank_distribution.h
 *
 *  Created on: Jan 14, 2014
 *      Author: njindal
 */

#ifndef RANK_DISTRIBUTION_H_
#define RANK_DISTRIBUTION_H_

#include <vector>

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
	 * @return
	 */
	virtual bool is_server(int rank) = 0;

	/**
	 * Which local servers should this worker communicate with.
	 * @param rank
	 * @return
	 */
	virtual std::vector<int> local_servers_to_communicate(int rank) = 0;

	/**
	 * Is this the worker that should communicate with the local server.
	 * @param rank
	 * @return
	 */
	virtual bool is_local_worker_to_communicate(int rank) = 0;


};

/**
 * Distributes worker and server in 2:1 ratio. For 2 ranks, 0 is worker and 1 is server.
 * For other configs, every 3rd rank is a server.
 */
class TwoWorkerOneServerRankDistribution : public RankDistribution {
public:
	TwoWorkerOneServerRankDistribution(int num_processes);
	virtual bool is_server(int rank);
	virtual std::vector<int> local_servers_to_communicate(int rank);
	virtual bool is_local_worker_to_communicate(int rank);
private:
	const int size_;
};

/**
 *  Vector created should look like so :
 * 	W W W S | W W S  | W W S (7 W : 3 S) or
 * 	W S | W S | W S | W S | W S (5 W : 5 S) or
 * 	W S S S | W S S | W S S (3 W : 7 W)
 * 	The servers will always be laid out at the end of a "company"
 * 	Within a "company", one of workers is responsible for
 * 	communicating with all the servers.
 */
class ConfigurableRankDistribution : public RankDistribution {
public:
	ConfigurableRankDistribution(int num_workers, int num_server);
	virtual bool is_server(int rank);
	virtual std::vector<int> local_servers_to_communicate(int rank);
	virtual bool is_local_worker_to_communicate(int rank);
	std::vector<bool>& get_is_server_vector() { return is_server_vector_; }
private:
	int num_workers_;
	int num_servers_;
	std::vector<bool> is_server_vector_;
};


/**
 * All workers in rank distribution.
 * Primarily for tests where distributed arrays are not used
 * and executables to be built with MPI turned on, but run without MPI
 */
class AllWorkerRankDistribution : public RankDistribution{
public:
	virtual bool is_server(int rank){ return false;	}
	virtual std::vector<int> local_servers_to_communicate(int rank){ return std::vector<int>();	}
	virtual bool is_local_worker_to_communicate(int rank){ return false; }
};

} /* namespace sip */

#endif /* RANK_DISTRIBUTION_H_ */
