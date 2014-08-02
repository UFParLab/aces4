/*
 * sip_mpi_attr.cpp
 *
 *  Created on: Jan 14, 2014
 *      Author: njindal
 */

#include "sip_mpi_attr.h"
#include <iostream>


#include "sip_mpi_utils.h"


namespace sip {

SIPMPIAttr* SIPMPIAttr::instance_ = NULL;
bool SIPMPIAttr::destroyed_ = false;

SIPMPIAttr& SIPMPIAttr::get_instance() {
	if (destroyed_)
		sip::fail("SIPMPIAttr instance has been destroyed !");
	if (instance_ == NULL)
		instance_ = new SIPMPIAttr();
	return *instance_;
}

void SIPMPIAttr::cleanup() {
	delete instance_;
	destroyed_ = true;
}

SIPMPIAttr::SIPMPIAttr() {
	SIPMPIUtils::check_err(MPI_Comm_rank(MPI_COMM_WORLD, &global_rank_));
	SIPMPIUtils::check_err(MPI_Comm_size(MPI_COMM_WORLD, &global_size_));

	// Splitting up communicator for servers and worker
	// From http://static.msi.umn.edu/tutorial/scicomp/general/MPI/communicator.html

	/* Extract the original group handle */
	SIPMPIUtils::check_err(MPI_Comm_group(MPI_COMM_WORLD, &univ_group));

	int *worker_ranks = new int[global_size_];
	int *server_ranks = new int[global_size_];

	int w=0, s=0;

	for (int i=0; i<global_size_; i++){
		if (sip::RankDistribution::is_server(i, global_size_)){
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

	SIPMPIUtils::check_err(MPI_Group_incl(univ_group, s, server_ranks, &server_group));
	SIPMPIUtils::check_err(MPI_Group_incl(univ_group, w, worker_ranks, &worker_group));

	SIPMPIUtils::check_err(MPI_Comm_create(MPI_COMM_WORLD, server_group, &server_comm_));
	SIPMPIUtils::check_err(MPI_Comm_create(MPI_COMM_WORLD, worker_group, &worker_comm_));

	is_server_ = sip::RankDistribution::is_server(global_rank_, global_size_);

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
	SIPMPIUtils::check_err(MPI_Group_translate_ranks(server_group, 1, &company_master, univ_group, &server_master_));
	SIPMPIUtils::check_err(MPI_Group_translate_ranks(worker_group, 1, &company_master, univ_group, &worker_master_));

	delete [] worker_ranks;
	delete [] server_ranks;

	if (RankDistribution::is_local_worker_to_communicate(global_rank_, global_size_))
		my_server_ = RankDistribution::local_server_to_communicate(global_rank_, global_size_);
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
	os << "rank : " << obj.global_rank_;
	os << ", size : " << obj.global_size_;
	os << ", is server? : " << obj.is_server_;
	os << ", company rank : " << obj.company_rank_ ;
	os << ", company size : " << obj.company_size_;
	os << ", is master? : " << obj.is_company_master_ ;
    os << ", server_master : " << obj.server_master_;
    os << ", worker master : " << obj.worker_master_ << "]";
    os << std::endl;
	return os;
}


SIPMPIAttr::~SIPMPIAttr() {
	SIPMPIUtils::check_err(MPI_Group_free(&server_group));
	SIPMPIUtils::check_err(MPI_Group_free(&worker_group));
	SIPMPIUtils::check_err(MPI_Group_free(&univ_group));
	if (server_comm_ != MPI_COMM_NULL)
		SIPMPIUtils::check_err(MPI_Comm_free(&server_comm_));
	if (worker_comm_ != MPI_COMM_NULL)
		SIPMPIUtils::check_err(MPI_Comm_free(&worker_comm_));
}

} /* namespace sip */


//#else  //not HAVE_MPI
//
//
//namespace sip{
//
//
//
//SIPMPIAttr* SIPMPIAttr::instance_ = NULL;
//bool SIPMPIAttr::destroyed_ = false;
//
//SIPMPIAttr& SIPMPIAttr::get_instance() {
//	if (destroyed_)
//		sip::fail("SIPMPIAttr instance has been destroyed !");
//	if (instance_ == NULL)
//		instance_ = new SIPMPIAttr();
//	return *instance_;
//}
//
//void SIPMPIAttr::cleanup() {
//	delete instance_;
//	destroyed_ = true;
//}
//
//
//SIPMPIAttr::SIPMPIAttr() {
//	std::cout << "creating sip mpi attr " << std::endl;
//}
//SIPMPIAttr::~SIPMPIAttr() {
//}
//}//namespace sip
//
//#endif  //HAVE_MPI


