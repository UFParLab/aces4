/*
 * sip_mpi_attr.cpp
 *
 *  Created on: Jan 14, 2014
 *      Author: njindal
 */


#include "config.h"
#include "sip_mpi_attr.h"
#include <iostream>

#ifdef HAVE_MPI

#include "mpi.h"
#include "sip_mpi_utils.h"

namespace sip {

SIPMPIAttr* SIPMPIAttr::instance_ = NULL;
RankDistribution* SIPMPIAttr::rank_distribution_ = NULL;
bool SIPMPIAttr::destroyed_ = false;

/**
 * Gets instance of Sip attributes.
 * The default rank distribution is 2 workers : 1 server.
 * To change this, use the overloaded get_instance method.
 */
SIPMPIAttr& SIPMPIAttr::get_instance() {
	if (destroyed_)
		sip::fail("SIPMPIAttr instance has been destroyed !");
	if (instance_ == NULL){
		if (rank_distribution_ == NULL)
			rank_distribution_ = new TwoWorkerOneServerRankDistribution();
		instance_ = new SIPMPIAttr(*rank_distribution_);
	}
	return *instance_;
}

/**
 * Sets and gets the instance of Sip attributes.
 * Must only be called once and before all other methods in SIPMPIAttr
 */
void SIPMPIAttr::set_rank_distribution(RankDistribution *rank_dist) {
	if (destroyed_)
		sip::fail("SIPMPIAttr instance has been destroyed !");
	if (rank_distribution_ != NULL){
		fail("RankDistribution instance was already set previously !");
	}
	rank_distribution_ = rank_dist;
}


void SIPMPIAttr::cleanup() {
	delete instance_;
	//delete rank_distribution_; // TODO FIXME - Minor leak
	destroyed_ = true;
}

SIPMPIAttr::SIPMPIAttr(RankDistribution& rank_distribution):
		// Initialize all values to throw an error if not properly re-initialized in the constructor.
		num_servers_(-1), num_workers_ (-1),
		worker_master_(-1), server_master_(-1),
		is_server_(false), is_company_master_(false),
		global_rank_(-1), global_size_(-1),
		company_size_(-1), my_server_(-1),
		server_group_(MPI_GROUP_NULL), worker_group_(MPI_GROUP_NULL),
		server_comm_(MPI_COMM_NULL), worker_comm_(MPI_COMM_NULL), company_comm_(MPI_COMM_NULL){


	SIPMPIUtils::check_err(MPI_Comm_rank(MPI_COMM_WORLD, &global_rank_));
	SIPMPIUtils::check_err(MPI_Comm_size(MPI_COMM_WORLD, &global_size_));


	// Splitting up communicator for servers and worker
	// From http://static.msi.umn.edu/tutorial/scicomp/general/MPI/communicator.html

	/* Extract the original group handle */
	SIPMPIUtils::check_err(MPI_Comm_group(MPI_COMM_WORLD, &univ_group_));

	int *worker_ranks = new int[global_size_];
	int *server_ranks = new int[global_size_];

	int w=0, s=0;

	for (int i=0; i<global_size_; i++){
		if (rank_distribution.is_server(i, global_size_)){
			server_ranks[s++] = i;
			servers_.push_back(i);
		} else {
			worker_ranks[w++] = i;
			workers_.push_back(i);
		}
	}

	num_workers_ = w;
	num_servers_ = s;

	if (global_rank_ == 0){
		std::cout<<"There will be " << s << " servers :" << std::endl << std::flush;
		for(int i=0; i<s; i++)
			std::cout<<server_ranks[i]<<" ";
		std::cout<<std::endl;
		std::cout<<"There will be " << w << " workers : " << std::endl << std::flush;
		for(int i=0; i<w; i++)
			std::cout<<worker_ranks[i]<<" ";
		std::cout<<std::endl;
	}

	if (s >= 1){
		SIPMPIUtils::check_err(MPI_Group_incl(univ_group_, s, server_ranks, &server_group_));
		SIPMPIUtils::check_err(MPI_Comm_create(MPI_COMM_WORLD, server_group_, &server_comm_));
	}
	if (w >= 1){
		SIPMPIUtils::check_err(MPI_Group_incl(univ_group_, w, worker_ranks, &worker_group_));
		SIPMPIUtils::check_err(MPI_Comm_create(MPI_COMM_WORLD, worker_group_, &worker_comm_));
	}

	is_server_ = rank_distribution.is_server(global_rank_, global_size_);

	if (is_server_){
		company_comm_ = server_comm_;
	} else {
		company_comm_ = worker_comm_;
	}
	sip::check(company_comm_ != MPI_COMM_NULL, "Company Communicator is MPI_COMM_NULL !");

	SIPMPIUtils::check_err(MPI_Comm_rank(company_comm_, &company_rank_));
	SIPMPIUtils::check_err(MPI_Comm_size(company_comm_, &company_size_));
	is_company_master_ = COMPANY_MASTER_RANK == company_rank_;

	// Determine company masters
	int company_master = COMPANY_MASTER_RANK;
	if (s >= 1)
		SIPMPIUtils::check_err(MPI_Group_translate_ranks(server_group_, 1, &company_master, univ_group_, &server_master_));
	if (w >= 1)
		SIPMPIUtils::check_err(MPI_Group_translate_ranks(worker_group_, 1, &company_master, univ_group_, &worker_master_));

	delete [] worker_ranks;
	delete [] server_ranks;

	if (rank_distribution.is_local_worker_to_communicate(global_rank_, global_size_))
		my_server_ = rank_distribution.local_server_to_communicate(global_rank_, global_size_);
	else my_server_ =  -1;

}

bool SIPMPIAttr::is_company_master() const {
	return is_company_master_;
}

bool SIPMPIAttr::is_server() const {
	return is_server_;
}

bool SIPMPIAttr::is_worker() const {
	return ! is_server_;
}

std::ostream& operator<<(std::ostream& os, const SIPMPIAttr& obj){
	os << "SIP MPI Attributes [";
	os << "rank : " << obj.global_rank();
	os << ", size : " << obj.global_size();
	os << ", is server? : " << obj.is_server();
	os << ", company rank : " << obj.company_rank() ;
	os << ", company size : " << obj.company_size();
	os << ", is master? : " << obj.is_company_master() ;
    os << ", server_master : " << obj.server_master();
    os << ", worker master : " << obj.worker_master() << "]";
    os << std::endl;
	return os;
}


SIPMPIAttr::~SIPMPIAttr() {
	if (server_group_ != MPI_GROUP_NULL)
		SIPMPIUtils::check_err(MPI_Group_free(&server_group_));
	if (worker_group_ != MPI_GROUP_NULL)
		SIPMPIUtils::check_err(MPI_Group_free(&worker_group_));
	if (univ_group_ != MPI_GROUP_NULL)
		SIPMPIUtils::check_err(MPI_Group_free(&univ_group_));
	if (server_comm_ != MPI_COMM_NULL)
		SIPMPIUtils::check_err(MPI_Comm_free(&server_comm_));
	if (worker_comm_ != MPI_COMM_NULL)
		SIPMPIUtils::check_err(MPI_Comm_free(&worker_comm_));
}

} /* namespace sip */



#else  //not HAVE_MPI


namespace sip{



SIPMPIAttr* SIPMPIAttr::instance_ = NULL;
bool SIPMPIAttr::destroyed_ = false;

/**
 * Gets instance of Sip attributes.
 * The default rank distribution is 2 workers : 1 server.
 * To change this, use the overloaded get_instance method.
 */
SIPMPIAttr& SIPMPIAttr::get_instance() {
	if (destroyed_)
		sip::fail("SIPMPIAttr instance has been destroyed !");
	if (instance_ == NULL){
		instance_ = new SIPMPIAttr();
	}
	return *instance_;
}

void SIPMPIAttr::cleanup() {
	delete instance_;
	destroyed_ = true;
}


SIPMPIAttr::SIPMPIAttr() {
	std::cout << "creating sip mpi attr " << std::endl;
}
SIPMPIAttr::~SIPMPIAttr() {
}

std::ostream& operator<<(std::ostream& os, const SIPMPIAttr& obj){
	os << "SIP MPI Attributes [";
	os << "rank : " << obj.global_rank();
	os << ", size : " << obj.global_size();
	os << ", is server? : " << obj.is_server();
	os << ", company rank : " << obj.company_rank();
	os << ", company size : " << obj.company_size();
	os << ", is master? : " << obj.is_company_master();
    os << ", server_master : " << obj.server_master();
    os << ", worker master : " << obj.worker_master() << "]";
    os << std::endl;
	return os;
}

}//namespace sip

#endif  //HAVE_MPI



