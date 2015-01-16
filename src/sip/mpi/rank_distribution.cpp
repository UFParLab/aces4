/*
 * rank_distribution.cpp
 *
 *  Created on: Jan 14, 2014
 *      Author: njindal
 */

#include <iostream>
#include <rank_distribution.h>
#include <assert.h>
#include "sip.h"

namespace sip {

TwoWorkerOneServerRankDistribution::TwoWorkerOneServerRankDistribution(int num_processes) : size_(num_processes) {}

bool TwoWorkerOneServerRankDistribution::is_server(int rank){
	assert (size_ >= 2);

	if (size_ >= 3)
		return 2 == rank % 3;	// Every third rank is a server
	else
		return 1 == rank;
}

std::vector<int> TwoWorkerOneServerRankDistribution::local_servers_to_communicate(int rank){
	std::vector<int> servers;
	if (size_ < 3){
		servers.push_back(1);
		return servers;
	}

	int server_rank = ((rank / 3) + 1) * 3 - 1;
	servers.push_back((server_rank < size_) ? (server_rank) : (-1));
	return servers;
}

bool TwoWorkerOneServerRankDistribution::is_local_worker_to_communicate(int rank){
	if (size_ >= 3)
		return 0 == rank % 3;	// Every 1 out of 3 is the worker that shall communicate with the server
	else
		return 0 == rank;
}





ConfigurableRankDistribution::ConfigurableRankDistribution(int num_workers, int num_servers)
	:num_workers_(num_workers), num_servers_(num_servers), is_server_vector_(num_workers+num_servers, false) {
	check (num_workers_ >= 1, "Number of workers must be more than 1");
	check (num_workers_ >= 1, "Number of servers must be more than 1");

	int num_processes = num_workers + num_servers;
	if (num_workers >= num_servers){	// More workers than server
		// At least have 1 server in each group of workers & servers.
		int num_workers_to_servers = num_workers / num_servers;
		int num_groups = num_processes / (num_workers_to_servers + 1);
		int remaining_workers = num_workers % num_servers;
		int count = 0;
		for (int i=0; i<num_groups; ++i){
			for (int j=0; j<num_workers_to_servers; ++j){
				is_server_vector_[count++] = false;	// Worker
			}
			if (i < remaining_workers){
				is_server_vector_[count++] = false;	// Worker
			}
			is_server_vector_[count++] = true;		// Server
		}
	} else {	// More servers than workers
		// At least 1 worker in each group of workers & servers
		int num_servers_to_workers = num_servers / num_workers;
		int num_groups = num_processes / (num_servers_to_workers + 1);
		int remaining_servers = num_servers % num_workers;
		int count = 0;
		for (int i=0; i<num_groups; ++i){
			is_server_vector_[count++] = false;
			for (int j=0; j<num_servers_to_workers; ++j){
				is_server_vector_[count++] = true;
			}
			if (i < remaining_servers){
				is_server_vector_[count++] = true;
			}
		}
	}
}

bool ConfigurableRankDistribution::is_server(int rank){
	return is_server_vector_[rank];
}

std::vector<int> ConfigurableRankDistribution::local_servers_to_communicate(int rank){
	int size = num_workers_ + num_servers_;
	std::vector<int> servers;
	if (is_local_worker_to_communicate(rank)){
		int i=1;
		// Skip till server is encountered or size is reached.
		while ((rank+i) < size && is_server_vector_[rank+i] == false){
			i++;
		}
		// Skip till next worker is encountered or size is reached.
		while ((rank+i) < size && is_server_vector_[rank+i] == true){
			servers.push_back(rank + i);
			i++;
		}
	}
	return servers;
}

bool ConfigurableRankDistribution::is_local_worker_to_communicate(int rank){
	if (is_server(rank))
		return false;
	if (rank == 0)
		return true;
	if (is_server_vector_[rank-1] == true)	// If previous rank is a server
		return true;
	return false;
}

} /* namespace sip */
