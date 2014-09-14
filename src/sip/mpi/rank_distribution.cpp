/*
 * rank_distribution.cpp
 *
 *  Created on: Jan 14, 2014
 *      Author: njindal
 */

#include <rank_distribution.h>

#include <assert.h>

namespace sip {

bool TwoWorkerOneServerRankDistribution::is_server(int rank, int size){
	assert (size >= 2);

	if (size >= 3)
		return 2 == rank % 3;	// Every third rank is a server
	else
		return 1 == rank;
}

int TwoWorkerOneServerRankDistribution::local_server_to_communicate(int rank, int size){
	if (size < 3)
		return 1;

	int server_rank = ((rank / 3) + 1) * 3 - 1;
	return (server_rank < size) ? (server_rank) : (-1);
}

bool TwoWorkerOneServerRankDistribution::is_local_worker_to_communicate(int rank, int size){
	if (size >= 3)
		return 0 == rank % 3;	// Every 1 out of 3 is the worker that shall communicate with the server
	else
		return 0 == rank;
}



} /* namespace sip */
