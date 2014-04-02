/*
 * sip_mpi_attr.h
 *
 *  Created on: Jan 14, 2014
 *      Author: njindal
 */

#ifndef SIP_MPI_ATTR_H_
#define SIP_MPI_ATTR_H_

#include "mpi.h"
#include "sip.h"
#include "rank_distribution.h"
#include "barrier_support.h"


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

	int company_rank_; 
~SIPMPIAttr();

	std::vector<int>& server_ranks() { return servers_; }
	std::vector<int>& worker_ranks() { return workers_; }

	int num_servers() const { return num_servers_; }
	int num_workers() const { return num_workers_; }

	int global_rank() const { return global_rank_; }
	int global_size() const { return global_size_; }

	// Each worker is in a worker company. Each server is in a server company.
	int company_rank() const { return company_rank_; }
	int company_size() const { return company_size_; }
	bool is_company_master() const;

	bool is_worker() const;
	bool is_server() const;

	int worker_master() const { return worker_master_; }
	int server_master() const { return server_master_; }

	MPI_Comm& company_communicator() { return company_comm_; }

	int my_server(){return my_server_;}

	friend std::ostream& operator<<(std::ostream&, const SIPMPIAttr&);

	//TODO refactor this
	BarrierSupport barrier_support_;

private:
	SIPMPIAttr();

	std::vector<int> servers_; // Server MPI ranks;
	std::vector<int> workers_; // Worker MPI ranks
	int num_servers_; // Number of servers
	int num_workers_; // Number of workers
	int worker_master_; // Worker master
	int server_master_; // Server master
	bool is_server_; // Is this rank a server
	bool is_company_master_; // Is this rank a company master (master worker or master server)
	int global_rank_; // Rank w.r.t. the global communicator
	int global_size_; // Number of ranks in MPI_COMM_WORLD
	// Rank w.r.t. company
	int company_size_; // Size of company
	int my_server_; //server to communicate with, or none if not responsible for a server.

	MPI_Comm company_comm_; // This company's communicator
	MPI_Comm server_comm_; // Server company communicator
	MPI_Comm worker_comm_; // Worker company communicator

	// Temp variables to free up.
	MPI_Group server_group;
	MPI_Group worker_group;
	MPI_Group univ_group;



	DISALLOW_COPY_AND_ASSIGN(SIPMPIAttr);

};

} /* namespace sip */

#endif /* SIP_MPI_ATTR_H_ */
