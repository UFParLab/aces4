/*
 * counter.cpp
 *
 *  Created on: Jan 19, 2015
 *      Author: basbas
 */

#include "counter.h"


namespace sip {

Counter::CounterList  Counter::list_;
Counter::Counter(std::string name, bool register_counter):
	value_(0), name_(name){
	if (register_counter) list_.push_back(this);
}

Counter::~Counter() {

}


void Counter::gather_from_company(std::ostream& out){
#ifdef HAVE_MPI

	const SIPMPIAttr& mpi_attr = SIPMPIAttr::get_instance();
	int my_worker_rank = mpi_attr.company_rank();
	int num_workers = mpi_attr.company_size();


	std::vector<size_t> rbuf(num_workers, 0);

	if (num_workers > 1) {
		MPI_Gather(&value_, 1, MPI_LONG_LONG, rbuf.data(),
				1, MPI_LONG_LONG, mpi_attr.worker_master(),
				mpi_attr.company_communicator());
	} else {
		rbuf[0] = value_;
	}
	if (mpi_attr.is_company_master()) {
		out << name_ ;
			for (int j = 0; j < num_workers; j++) {
				out << "," << rbuf[j];
			}
			out << std::endl;
		}


#else
out << name_ << "," << value_ << std::endl;

#endif

}

void Counter::gather_all(std::ostream& out){
	std::vector<Counter*>::iterator it;
	for (it = Counter::list_.begin(); it != Counter::list_.end(); ++it){
		(*it)->gather_from_company(out);
	}
}


std::vector<MaxCounter*> MaxCounter::list_;

MaxCounter::MaxCounter(std::string name, bool register_counter):
	value_(0), max_(0), name_(name){
	if (register_counter) list_.push_back(this);
}

MaxCounter::~MaxCounter() {

}


void MaxCounter::gather_from_company(std::ostream& out){
#ifdef HAVE_MPI

	const SIPMPIAttr& mpi_attr = SIPMPIAttr::get_instance();
	int my_worker_rank = mpi_attr.company_rank();
	int num_workers = mpi_attr.company_size();


	std::vector<size_t> rbuf(num_workers * 2, 0);
	size_t sendbuf[] = {value_,max_};

	if (num_workers > 1) {
		MPI_Gather(sendbuf, 2, MPI_LONG_LONG, rbuf.data(),
				2, MPI_LONG_LONG, mpi_attr.worker_master(),
				mpi_attr.company_communicator());
	} else {
		rbuf[0] = value_;
	}
	if (mpi_attr.is_company_master()) {
		out << name_ << " current";
			for (int j = 0; j < num_workers*2; j=j+2) {
				out << "," << rbuf[j];
			}
			out << std::endl;
			out << name_ << " max";
			for (int j = 1; j < num_workers*2; j=j+2) {
				out << "," << rbuf[j];
			}
			out << std::endl;
		}


#else
out << name_ << "," << value_ << ", " << max_ << std::endl;

#endif

}

void MaxCounter::gather_all(std::ostream& out){
	std::vector<MaxCounter*>::iterator it;
	for (it = MaxCounter::list_.begin(); it != MaxCounter::list_.end(); ++it){
		(*it)->gather_from_company(out);
	}
}

std::vector<SimpleTimer*> SimpleTimer::list_;
	SimpleTimer::SimpleTimer(std::string name, bool register_timer):name_(name), start_time_(0.0), num_epochs_(0), max_epoch_(0.0), value_(0.0), on_(false) {
		if (register_timer) list_.push_back(this);
	}
	SimpleTimer::~SimpleTimer(){
	}


//TODO rename *workers*
	void SimpleTimer::gather_from_company(std::ostream& out){

	#ifdef HAVE_MPI

		const SIPMPIAttr& mpi_attr = SIPMPIAttr::get_instance();
		int my_worker_rank = mpi_attr.company_rank();
		int num_workers = mpi_attr.company_size();


		std::vector<double> rbuf(num_workers * 2, 0);
		double sendbuf[] = {value_,max_epoch_};

		if (num_workers > 1) {
			MPI_Gather(sendbuf, 2, MPI_DOUBLE, rbuf.data(),
					2, MPI_DOUBLE, mpi_attr.worker_master(),
					mpi_attr.company_communicator());
		} else {
			rbuf[0] = value_;
			rbuf[1] = max_epoch_;
		}
		if (mpi_attr.is_company_master()) {
			out << name_ << " total";
				for (int j = 0; j < num_workers*2; j=j+2) {
					out << "," << rbuf[j];
				}
				out << std::endl;
				out << name_ << " max";
				for (int j = 1; j < num_workers*2; j=j+2) {
					out << "," << rbuf[j];
				}
				out << std::endl;
			}


	#else
	out << name_ << "," << value_ << ", " << max_epoch_ << std::endl;

	#endif

	}

	void SimpleTimer::start(){check(!on_,"starting on timer" + name_);start_time_ = MPI_Wtime(); on_=true;}


	double SimpleTimer::pause(){check(on_,"pausing off timer");
	     double curr = MPI_Wtime();
	     double elapsed = curr-start_time_;
	     value_ = value_+elapsed;
	     num_epochs_++;
	     if (elapsed > max_epoch_) {max_epoch_ = elapsed;}
	     on_= false;
	     return elapsed;
	}


	void SimpleTimer::gather_all(std::ostream& out){
		std::vector<SimpleTimer*>::iterator it;
		for (it = SimpleTimer::list_.begin(); it != SimpleTimer::list_.end(); ++it){
			(*it)->gather_from_company(out);
		}
	}

} /* namespace sip */
