/*
 * sip_mpi_attr.h
 *
 *  Created on: Jan 14, 2014
 *      Author: njindal
 */

#ifndef SIP_MPI_ATTR_H_
#define SIP_MPI_ATTR_H_

#include "sip.h"
#include "rank_distribution.h"
#include "mpi.h"

namespace sip {

class SIPMPIAttr {
public:

	static const int COMPANY_MASTER_RANK = 0;
	static SIPMPIAttr *instance_;
	static bool destroyed_;

	// Singleton factory
	static SIPMPIAttr& get_instance() ;

	/**
	 * Delete singleton instance
	 */
	static void cleanup() ;

	~SIPMPIAttr();

	const std::vector<int>& server_ranks() { return servers_; }
	const std::vector<int>& worker_ranks() { return workers_; }

	const int num_servers() const { return num_servers_; }
	const int num_workers() const { return num_workers_; }

	const int global_rank() const { return global_rank_; }
	const int global_size() const { return global_size_; }

	// Each worker is in a worker company. Each server is in a server company.
	const int company_rank() const { return company_rank_; }
	const int company_size() const { return company_size_; }
	const bool is_company_master() const;

	const bool is_worker() const;
	const bool is_server() const;

	const int worker_master() const { return worker_master_; }
	const int server_master() const { return server_master_; }

	MPI_Comm& company_communicator() { return company_comm_; }
//	MPI_Comm& server_communicator() { return server_comm_; }
//	MPI_Comm& worker_communicator() { return worker_comm_; }

	friend std::ostream& operator<<(std::ostream&, const SIPMPIAttr&);


private:
	SIPMPIAttr();

	/**
	 * Server MPI ranks;
	 */
	std::vector<int> servers_;

	/**
	 * Worker MPI ranks
	 */
	std::vector<int> workers_;

	/**
	 * Number of servers
	 */
	int num_servers_;

	/**
	 * Number of workers
	 */
	int num_workers_;

	/**
	 * Server master
	 */
	int worker_master_;

	/**
	 * Worker master
	 */
	int server_master_;

	/**
	 * Is this rank a server
	 */
	bool is_server_;

	/**
	 * Is this rank a company master (master worker or master server)
	 */
	bool is_company_master_;

	/**
	 * Rank w.r.t. the global communicator
	 */
	int global_rank_;
	/**
	 * Number of ranks in MPI_COMM_WORLD
	 */
	int global_size_;

	/**
	 * Rank w.r.t. company
	 */
	int company_rank_;

	/**
	 * Size of company
	 */
	int company_size_;

	/**
	 * This company's communicator
	 */
	MPI_Comm company_comm_;

	/**
	 * Server company communicator
	 */
	MPI_Comm server_comm_;
	/**
	 * Worker company communicator
	 */
	MPI_Comm worker_comm_;

	// Temp variables to free up.
	MPI_Group server_group;
	MPI_Group worker_group;
	MPI_Group univ_group;

	DISALLOW_COPY_AND_ASSIGN(SIPMPIAttr);

};

} /* namespace sip */

#endif /* SIP_MPI_ATTR_H_ */
