/*
 * counter.cpp
 *
 *  Created on: Jan 19, 2015
 *      Author: basbas
 */

#include "counter.h"
#include <iterator>
#include <vector>

namespace sip {

//
//
//std::ostream& CounterList::stream_out(std::ostream& os) const{
//	int i = 0;
//	std::vector<size_t>::const_iterator it = list_.begin();
//	size_t list_size = list_.size();
//	while(i < list_size){
//		os << i << ',' << list_[i] << std::endl;
//		++i;
//		++it;
//	}
//	return os;
//}
//
//
//void PCounterList::gather(){
//	int rank;
//	int comm_size;
//	MPI_Comm_rank(comm_, &rank);
//	MPI_Comm_size(comm_, &comm_size);
//	size_t list_size = list_.size();
//	int vecsize = rank == 0 ? comm_size * list_size : 0;
//	gathered_vals_.resize(vecsize,0);
//	if (comm_size>1){
//		MPI_Gather(list_.data(), list_size, MPI_UNSIGNED_LONG, gathered_vals_.data(),
//				list_size, MPI_UNSIGNED_LONG, 0,
//				comm_);
//	}
//	else {
//		gathered_vals_ = list_;
//	}
//}
//
//void PCounterList::reduce(){
//	int rank;
//	int comm_size;
//	MPI_Comm_rank(comm_, &rank);
//	MPI_Comm_size(comm_, &comm_size);
//	size_t list_size = list_.size();
//	int vecsize = rank == 0 ? list_size : 0;
//	reduced_vals_.resize(vecsize,0.0);
//	if (comm_size>1){
//		MPI_Reduce(list_.data(), reduced_vals_.data(), list_size, MPI_UNSIGNED_LONG, MPI_SUM,
//				0, comm_);
//	}
//	else {
//		reduced_vals_ = list_;
//	}
//}
//
//std::ostream& PCounterList::stream_out(std::ostream& os) const {
//	if (gathered_vals_.size() == 0 && reduced_vals_.size()==0){
//		return CounterList::stream_out(os);
//	}
//	int rank;
//	int comm_size;
//	MPI_Comm_rank(comm_, &rank);
//	MPI_Comm_size(comm_, &comm_size);
//
//	if(gathered_vals_.size() > 0){
//	std::vector<size_t>::const_iterator it;
//	size_t list_size = list_.size();
//	int stride = list_size;
//	int i = 0;
//	while (i < list_size) {
//		os << i;
//		it = gathered_vals_.begin() + i;
//		for (int j = 0; j < comm_size; ++j) {
//			os << ',' << *it;
//			it += stride;
//		}
//		os << std::endl;
//		++i;
//	}
//	}
//	if (reduced_vals_.size()>0){
//		if (gathered_vals_.size() > 0) os << std::endl << std::endl;
//		double dcomm_size = static_cast<double>(comm_size);
//		int i = 0;
//		std::vector<size_t>::const_iterator it = reduced_vals_.begin();
//		size_t list_size = list_.size();
//		while(i < list_size){
//			os << i << ',' << static_cast<double>(reduced_vals_[i])/dcomm_size << std::endl;
//			++i;
//			++it;
//		}
//	}
//	return os;
//}

/*********************************/


//void PMaxCounter::gather(){
//	int rank;
//	int comm_size;
//	MPI_Comm_rank(comm_, &rank);
//	MPI_Comm_size(comm_, &comm_size);
//	int vecsize = rank == 0 ? comm_size*2 : 0;
//	gathered_vals_.resize(vecsize,0);
//	size_t sendbuf[] = {value_,max_};
//	if (comm_size>1){
//		MPI_Gather(sendbuf, 2, MPI_LONG, gathered_vals_.data(),
//				2, MPI_LONG, 0,
//				comm_);
//	}
//	else {
//		gathered_vals_[0] = value_;
//		gathered_vals_[1]=max_;
//	}
//}
//
//void PMaxCounter::reduce(){
//	int rank;
//	int comm_size;
//	MPI_Comm_rank(comm_, &rank);
//	MPI_Comm_size(comm_, &comm_size);
//	int vecsize = rank == 0 ? comm_size*2 : 0;
//	reduced_vals_.resize(vecsize,0);
//	std::vector<long> sendbuf;
//	sendbuf.reserve(2);
//	sendbuf.push_back(value_);
//	sendbuf.push_back(max_);
//	if (comm_size>1){
//		MPI_Reduce(sendbuf.data(), reduced_vals_.data(), 2, MPI_LONG, MPI_SUM,0, comm_);
//	}
//	else {
//		reduced_vals_ = sendbuf;
//	}
//}








//
//
//
//std::ostream& MaxCounterList::stream_out(std::ostream& os) const{
//	int i = 0;
//	std::vector<MaxCounter>::const_iterator it = list_.begin();
//	size_t list_size = list_.size();
//	while(i < list_size){
//		os << i << ',' << list_[i] << std::endl;
//		++i;
//		++it;
//	}
//	return os;
//}
//
//////todo  create an MPI_Datatype instead of copying
//void PMaxCounterList::gather(){
//	int rank;
//	int comm_size;
//	MPI_Comm_rank(comm_, &rank);
//	MPI_Comm_size(comm_, &comm_size);
//
//	//copy values into the send buffer.  This should be replaced by code that uses an MPI_Datatype
//	std::vector<long> sendbuf;
//	sendbuf.reserve(list_.size()*2);
//	std::vector<MaxCounter>::iterator it;
//	for (it = list_.begin(); it != list_.end(); ++it){
//		sendbuf.push_back(it->get_value());
//		sendbuf.push_back(it->get_max());
//	}
//
//	//create destination buffer, its size is 0 except at master
//	int vecsize = rank == 0 ? comm_size*sendbuf.size() : 0;
//	gathered_vals_.resize(vecsize,0);
//
//	if (comm_size>1){
//		MPI_Gather(sendbuf.data(), sendbuf.size(), MPI_LONG, gathered_vals_.data(),
//				sendbuf.size(), MPI_LONG, 0,
//				comm_);
//	}
//	else {
//		gathered_vals_ = sendbuf;
//	}
//}
//
//void PMaxCounterList::reduce(){
//	int rank;
//	int comm_size;
//	MPI_Comm_rank(comm_, &rank);
//	MPI_Comm_size(comm_, &comm_size);
//
//	//copy values into the send buffer.  This should be replaced by code that uses an MPI_Datatype
//	std::vector<long> sendbuf;
//	sendbuf.reserve(list_.size()*2);
//	std::vector<MaxCounter>::iterator it;
//	for (it = list_.begin(); it != list_.end(); ++it){
//		sendbuf.push_back(it->get_value());
//		sendbuf.push_back(it->get_max());
//	}
//
//	//create destination buffer, its size is 0 except at master
//	int vecsize = rank == 0 ? sendbuf.size() : 0;
//	reduced_vals_.resize(vecsize,0);
//
//	if (comm_size>1){
//		MPI_Reduce(sendbuf.data(), reduced_vals_.data(), vecsize, MPI_LONG, MPI_SUM,0,comm_);
//	}
//	else {
//		reduced_vals_ = sendbuf;
//	}
//}
//
//	std::ostream& PMaxCounterList::stream_out(std::ostream& os) const {
//		if (gathered_vals_.size() == 0 && reduced_vals_.size() == 0) {
//			os << "Single process output" << std::endl;
//			return MaxCounterList::stream_out(os);
//		}
//		int comm_size;
//		MPI_Comm_size(comm_, &comm_size);
//		int vals_per_entry = 2;  //value_ and max_
//
//		size_t list_size = list_.size();
//		std::vector<long>::const_iterator it;
//		if (gathered_vals_.size() > 0) {
//			//output the values
//			os << "Values" << std::endl;
//			int i = 0;
//
//
//			size_t stride = list_size * vals_per_entry;
//			size_t offset = 0; //offset of this entry into structure
//			//for each item in the list, print a row of value_ from each process
//			while (i < list_size) {
//				os << i;
//				it = gathered_vals_.begin() + (i * vals_per_entry) + offset;
//				for (int j = 0; j < comm_size; ++j) {
//					os << ',' << *it;
//					it += stride;
//				}
//				os << std::endl;
//				++i;
//			}
//			//output the max
//			os << "Max" << std::endl;
//			i = 0;
//			offset = 1;  //start on the second element
//			while (i < list_size) {
//				os << i;
//				it = gathered_vals_.begin() + (i * vals_per_entry) + offset;
//				for (int j = 0; j < comm_size; ++j) {
//					os << ',' << *it;
//					it += stride;
//				}
//				os << std::endl;
//				++i;
//			}
//		}
//		if (reduced_vals_.size() > 0) {
//			int i = 0;
//			while (i < list_size) {
//				os << i;
//				it = gathered_vals_.begin() + (i * vals_per_entry);
//				os << ',' << *it << std::endl;
//				++i;
//			}
//		}
//		return os;
//	}
//
//
//
//friend std::ostream& operator<<(std::ostream& os, const MaxCounterList& obj){
//	std::vector<MaxCounter>::iterator iter = obj.list_.begin();
//	size_t list_size = obj.list_.size();
//	int i = 0;
//	while (i < list_size){
//	  os << i << ": " << iter->value_ << ", " << iter->max_ << std::endl;
//	  ++iter;
//	  ++i;
//	}
//	return os;
//}
//
//
//void PTimer::gather(){
//	int rank;
//	int comm_size;
//	MPI_Comm_rank(comm_, &rank);
//	MPI_Comm_size(comm_, &comm_size);
//	if (rank == 0){
//		gathered_totals_and_max_.resize(comm_size*2);
//		gathered_num_epochs_.resize(comm_size);
//	}
//	double sendbuf[] = {total_, max_epoch_};
//
//	if (comm_size>1){
//		MPI_Gather(sendbuf.data(),2, MPI_DOUBLE, gathered_totals_and_max_.data(),
//				2, MPI_DOUBLE, 0, comm_);
//		MPI_Gather(&num_epochs_, 1, MPI_LONG_LONG, gathered_num_epochs_.data(),
//				1, MPI_LONG_LONG, 0, comm_);
//	}
//	else {
//		gathered_totals_and_max_[0] = total_;
//		gathered_totals_and_max_[1] = max_epoch_;
//		gathered_num_epochs_[0] = num_epochs_;
//	}
//}
//
//
////FIX ME
//std::ostream& operator<<(std::ostream& os, const PTimerList& obj){
//	if ( obj.gathered_totals_and_max_.size() == 0) {
//		os << "PTimerList: nothing to print" << std::endl;
//		return os;
//	}
//	int comm_size;
//	MPI_Comm_size(obj.comm_, &comm_size);
//
//
//
//    //print items in gathered_totals_and_max_
//	std::vector<double>::iterator it; //iterator for gathered_totals_and_max;
//	size_t list_size = obj.list_.size();
//	size_t stride = obj.list_.size() * 2;
//
//	//output the totals
//	os << "Total times"<< std::endl;
//	int i = 0;
//	while( i < list_size){
//		os << i;
//		it = obj.gathered_totals_and_max_.begin();
//		for (int j = 0; j < comm_size; ++j){
//			os << ',' << *it;
//			it += stride;
//		}
//		os << std::endl;
//		++i;
//	}
//
//	//output the max
//	os << "Max"<< std::endl;
//	i = 0;
//	it = obj.gathered_totals_and_max_.begin() + 1;  //start on the second element
//	while( i < list_size){
//		os << i;
//		it = obj.gathered_totals_and_max_.begin() + i;
//		for (int j = 0; j < comm_size; ++j){
//			os << ',' << *it;
//			it += stride;
//		}
//		os << std::endl;
//		++i;
//	}
//
//	//print the items in gathered_num_epochs_
//
//	std::vector<size_t>::iterator it2;  //iterator for gathered_num_epochs;
//
//	//output the number of epochs
//	os << "Number of Epochs" << std::endl;
//	size_t epoch_stride = list_size;
//	std::vector<size_t>::iterator it2;
//	while( i < list_size){
//		os << i;
//		it2 = obj.gathered_num_epochs_.begin();
//		for (int j = 0; j < comm_size; ++j){
//			os << ',' << *it2;
//			it2 += stride;
//		}
//		os << std::endl;
//		++i;
//	}
//
//	//output ave time per epoch
//	os << "Time per epoch" << std::endl;
//	while (i < list_size){
//		os << i;
//		it = obj.gathered_totals_and_max_.begin()
//	}
//	return os;
//}
//
////std::ostream& MaxCounterList::gather_all(std::ostream& out, const std::string& title) {
////	if (SIPMPIAttr::get_instance().is_company_master()){
////		out << title << std::endl;
////	}
////	int i = 0;
////	std::vector<MaxCounter>::iterator it;
////	for (it = list_.begin(); it != list_.end(); ++it){
////		std::stringstream ss;
////		ss << i;
////		it->gather_from_company(out,ss.str());
////		++i;
////	}
////	return out;
////}
//
//
//
//
//
//	std::ostream& SimpleTimer::gather_max_from_company(std::ostream& out, const std::string& title){
//
//	#ifdef HAVE_MPI
//
//		const SIPMPIAttr& mpi_attr = SIPMPIAttr::get_instance();
//		int my_company_rank = mpi_attr.company_rank();
//		int company_size = mpi_attr.company_size();
//
//
//		std::vector<double> rbuf(company_size * 2, 0);
//		double sendbuf[] = {value_,max_epoch_};
//
//		if (company_size > 1) {
//			MPI_Gather(sendbuf, 2, MPI_DOUBLE, rbuf.data(),
//					2, MPI_DOUBLE, 0,
//					mpi_attr.company_communicator());
//		} else {
//			rbuf[0] = value_;
//			rbuf[1] = max_epoch_;
//		}
//		if (mpi_attr.is_company_master()) {
//			out << title;
////			out <<  " total";
////				for (int j = 0; j < company_size*2; j=j+2) {
////					out << "," << rbuf[j];
////				}
////				out << std::endl;
////				out <<  " max";
//				for (int j = 1; j < company_size*2; j=j+2) {
//					out << "," << rbuf[j];
//				}
//				out << std::endl;
//			}
//
//
//	#else
//	out << title << value_ << ", " << max_epoch_ << std::endl;
//
//	#endif
//return out;
//	}
//
//
//	std::ostream& SimpleTimer::gather_total_from_company(std::ostream& out, const std::string& title){
//
//	#ifdef HAVE_MPI
//
//		const SIPMPIAttr& mpi_attr = SIPMPIAttr::get_instance();
//		int my_company_rank = mpi_attr.company_rank();
//		int company_size = mpi_attr.company_size();
//
//
//		std::vector<double> rbuf(company_size * 2, 0);
//		double sendbuf[] = {value_,max_epoch_};
//
//		if (company_size > 1) {
//			MPI_Gather(sendbuf, 2, MPI_DOUBLE, rbuf.data(),
//					2, MPI_DOUBLE, 0,
//					mpi_attr.company_communicator());
//		} else {
//			rbuf[0] = value_;
//			rbuf[1] = max_epoch_;
//		}
//		if (mpi_attr.is_company_master()) {
//			out << title;
////			out <<  " total";
//				for (int j = 0; j < company_size*2; j=j+2) {
//					out << "," << rbuf[j];
//				}
//				out << std::endl;
////				out <<  " max";
////				for (int j = 1; j < company_size*2; j=j+2) {
////					out << "," << rbuf[j];
////				}
////				out << std::endl;
//			}
//
//
//	#else
//	out << title << value_ << ", " << max_epoch_ << std::endl;
//
//	#endif
//return out;
//	}
//
//
//
//	std::ostream& TimerList::gather_all(std::ostream& out, const std::string& title){
//		if (SIPMPIAttr::get_instance().is_company_master()){
//			out << title;
//		}
//		int i = 0;
//		std::vector<SimpleTimer>::iterator it;
//		for (it = list_.begin(); it != list_.end(); ++it){
//			std::stringstream ss;
//			ss << i;
//			it->gather_total_from_company(out, ss.str());
//			it->gather_max_from_company(out, ss.str());
//			++i;
//		}
//		return out;
//	}

} /* namespace sip */
