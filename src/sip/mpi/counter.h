/*
 * counter.h
 *
 *  Created on: Jan 19, 2015
 *      Author: basbas
 */

#ifndef COUNTER_H_
#define COUNTER_H_

#include <ctime>
#include <ostream>
#include <sstream>
#include <vector>
#include <iomanip>
#include <limits>
#include "sip.h"
#include "sip_tables.h"

#ifdef HAVE_MPI
#include "sip_mpi_attr.h"
#endif

namespace sip {
template<typename T, typename R> class Timer;
template<typename T, typename R> std::ostream& operator<<(std::ostream&,
                const Timer<T, R>&);

/**Basic monotonic counter */
template<typename T>
class Counter {
public:
        Counter() :
                        value_(0) {
        }
        ~Counter() {
        }
        ;
        void inc(size_t delta = 1) {
                value_ += delta;
        }
        size_t get_value() {
                return value_;
        }
        void reset() {
                value_ = 0;
        }
        void gather() {
                static_cast<T*>(this)->gather_impl();
        }
        void reduce() {
                static_cast<T*>(this)->reduce_impl();
        }
        friend std::ostream& operator<<(std::ostream& os, const Counter<T>& obj){
        return static_cast<const T&>(obj).stream_out(os);
    }
protected:
        size_t value_;
        std::ostream& stream_out(std::ostream& os) const {
                os << value_;
                return os;
        }
        DISALLOW_COPY_AND_ASSIGN(Counter);
};


template<typename T>
class NoopCounter{
public:
        NoopCounter(){}
        void inc(T delta = 0){}
        void dec(T delta = 0){}
        T get_value(){}
        void reset(){}
        void gather(){}
        void reduce(){}
        void set(T val){}
        std::ostream& stream_out(std::ostream& os) const{ return os;}
        friend std::ostream& operator<<(std::ostream& os, const NoopCounter<T>& obj){
        return os;
    }
};



class SingleNodeCounter: public Counter<SingleNodeCounter> {
public:
        //template<typename T> friend class Counter;
    friend class Counter<SingleNodeCounter>;
        //template<typename T> friend std::ostream& operator<<(std::ostream& os,
    //          const Counter<T>& obj);
    friend std::ostream& operator<<(std::ostream& os,
                const Counter<SingleNodeCounter>& obj);

        void gather_impl() {
        }
        void reduce_impl() {
        }
protected:
        std::ostream& stream_out(std::ostream& os) const {
                os << "value_=" << "value";
                return os;
        }
        DISALLOW_COPY_AND_ASSIGN(SingleNodeCounter);
};


#ifdef HAVE_MPI


class MPICounter: public Counter<MPICounter> {
public:
        MPICounter(const MPI_Comm& comm) :
                        comm_(comm), gathered_vals_(0), reduced_val_(0), gather_done_(
                                        false), reduce_done_(false) {
        }
        friend class Counter<MPICounter>;
        friend std::ostream& operator<<(std::ostream& os,
                        const Counter<MPICounter>& obj);
protected:
        const MPI_Comm& comm_;
        std::vector<size_t> gathered_vals_;
        size_t reduced_val_;
        std::ostream& stream_out(std::ostream& os) const {
                if (!gather_done_ && !reduce_done_) {
                        //print own data
                        int rank;
                        MPI_Comm_rank(comm_, &rank);
                        os << "company rank," << rank << ", value_," << value_ << std::endl;
                        return os;
                }
                int comm_size;
                MPI_Comm_size(comm_, &comm_size);

                if (gather_done_) {
                        //output gathered values in comma separated list
                        os << "value";
                        std::vector<size_t>::const_iterator it = gathered_vals_.begin();
                        for (int j = 0; j < comm_size; ++j) {
                                os << ',' << *it;
                                ++it;
                        }
                }
                if (reduce_done_) {
                        os << std::endl << "reduced mean=";
                        os
                                        << static_cast<double>(reduced_val_)
                                                        / static_cast<double>(comm_size);
                }
                os << std::endl;
                return os;
        }
        void gather_impl() {
                CHECK(sizeof(unsigned long int) == sizeof(size_t),
                                "mismatch in  mpi and c++ type ");
                int rank;
                int comm_size;
                MPI_Comm_rank(comm_, &rank);
                MPI_Comm_size(comm_, &comm_size);
                if (rank == 0) {
                        gathered_vals_.resize(comm_size, 0);
                        gather_done_ = true;
                }
                if (comm_size > 1) {
                        MPI_Gather(&value_, 1, MPI_UNSIGNED_LONG, &gathered_vals_.front(), 1,
                                        MPI_UNSIGNED_LONG, 0, comm_);
                } else {
                        gathered_vals_[0] = value_;
                }
        }
        void reduce_impl() {
                int rank;
                int comm_size;
                MPI_Comm_rank(comm_, &rank);
                MPI_Comm_size(comm_, &comm_size);

                if (comm_size > 1) {
                        MPI_Reduce(&value_, &reduced_val_, 1, MPI_UNSIGNED_LONG, MPI_SUM, 0,
                                        comm_);
                } else {
                        reduced_val_ = value_;
                }
                if (rank == 0) {
                        reduce_done_ = true;
                }
        }
        bool gather_done_;
        bool reduce_done_;
private:
        DISALLOW_COPY_AND_ASSIGN(MPICounter);
};

#endif

template<typename T>
class CounterList {
public:
	explicit CounterList(size_t size, bool filter = true) :
		size_(size), list_(size,0), filter_(filter) {}

	~CounterList() {}

	void inc(int index, size_t delta = 1) {
		list_[index] += delta;
	}

	size_t get_value(int index) {
		return list_[index];
	}

	void reset() {
		std::fill(list_.begin(), list_.end(), 0);
	}

	void gather() {
		static_cast<T*>(this)->gather_impl();
	}

	void reduce() {
		static_cast<T*>(this)->reduce_impl();
	}

	friend std::ostream& operator<<(std::ostream& os,
					const CounterList<T>& obj){
		return static_cast<const T&>(obj).stream_out(os);
	}

protected:
	std::vector<size_t> list_;
	size_t size_;
	bool filter_;
	DISALLOW_COPY_AND_ASSIGN(CounterList);
};



class SingleNodeCounterList : public CounterList<SingleNodeCounterList>{
public :
	SingleNodeCounterList(size_t size):
		CounterList<SingleNodeCounterList>(size) {}
	friend class CounterList<SingleNodeCounterList>;
	friend std::ostream& operator<<(std::ostream& os,
	                        const CounterList<SingleNodeCounterList>& obj);
protected:
	void reduce_impl() {}

	void gather_impl() {}

	std::ostream& stream_out(std::ostream& os) const{

		int i = 0;
		std::vector<size_t>::const_iterator it = list_.begin();
		while (i < size_) {
				os << i << ',' << *it << std::endl;
				++i;
				++it;
		}
		return os;
	}

private:
	DISALLOW_COPY_AND_ASSIGN(SingleNodeCounterList);
};



#ifdef HAVE_MPI

class MPICounterList: public CounterList<MPICounterList> {
public:
        MPICounterList(const MPI_Comm& comm, size_t size) :
                        CounterList<MPICounterList>(size), comm_(comm), gathered_vals_(0), reduced_vals_(
                                        0), reduce_done_(false), gather_done_(false) {
        }
        friend class CounterList<MPICounterList>;
        friend std::ostream& operator<<(std::ostream& os,
                        const CounterList<MPICounterList>& obj);
protected:
        const MPI_Comm& comm_;
        std::vector<size_t> gathered_vals_;
        std::vector<size_t> reduced_vals_;
        bool gather_done_;
        bool reduce_done_;
        void gather_impl() {
                int rank;
                int comm_size;
                MPI_Comm_rank(comm_, &rank);
                MPI_Comm_size(comm_, &comm_size);
                if (rank == 0) {
                        gathered_vals_.resize(comm_size * size_, 0);
                        gather_done_ = true;
                }
                if (comm_size > 1) {
                        MPI_Gather(&list_.front(), size_, MPI_UNSIGNED_LONG,
                                        &gathered_vals_.front(), size_, MPI_UNSIGNED_LONG, 0, comm_);
                } else {
                        gathered_vals_ = list_;
                }

        }
        void reduce_impl() {
                int rank;
                int comm_size;
                MPI_Comm_rank(comm_, &rank);
                MPI_Comm_size(comm_, &comm_size);
                if (rank == 0) {
                        reduced_vals_.resize(size_, 0);
                        reduce_done_ = true;
                }
                if (comm_size > 1) {
                        MPI_Reduce(&list_.front(), &reduced_vals_.front(), size_,
                                        MPI_UNSIGNED_LONG, MPI_SUM, 0, comm_);
                } else {
                        reduced_vals_ = list_;
                }
        }
        std::ostream& stream_out(std::ostream& os) const{
                if (!gather_done_ && !reduce_done_) {
                        //output own data
                        int i = 0;
                        std::vector<size_t>::const_iterator it = list_.begin();
                        while (i < size_) {
                                os << i << ',' << *it << std::endl;
                                ++i;
                                ++it;
                        }
                        return os;
                }
                int comm_size;
                MPI_Comm_size(comm_, &comm_size);
                if (gather_done_) {
                        std::vector<size_t>::const_iterator it;
                        int i = 0;
                        while (i < size_) {
                                os << i;
                                it = gathered_vals_.begin() + i;
                                for (int j = 0; j < comm_size; ++j) {
                                        os << ',' << *it;
                                        it += size_;
                                }
                                os << std::endl;
                                ++i;
                        }
                        os << std::endl;
                }
                if (reduce_done_) {
                        double dcomm_size = static_cast<double>(comm_size);
                        int i = 0;
                        std::vector<size_t>::const_iterator it = reduced_vals_.begin();
                        while (i < size_) {
                                if (filter_ && *it != 0){
                                os << i << ',' << static_cast<double>(*it) / dcomm_size
                                                << std::endl;
                                }
                                ++i;
                                ++it;
                        }
                        os << std::endl;
                }

                return os;
        }
private:
        DISALLOW_COPY_AND_ASSIGN(MPICounterList);
};

#endif //HAVE_MPI


/** Enhanced "counter" that can be decremented (and may have a value < 0) and remembers its maximum */
template<typename T, typename D>
class MaxCounter {
public:
        MaxCounter(D init = 0) :
                        value_(init), max_(init) {
        }

        ~MaxCounter() {
        }
        ;
        void inc(D delta = 1) {
        	//TODO why this?
                if (std::numeric_limits<D>::max() - value_  <= delta){
                        std::cerr << "value, delta = " << value_ << ',' << delta;
                }
                value_ += delta;
                if (value_ > max_){
                        max_ = value_;
                }
        }
        void dec(long delta = 1){
                value_ -= delta;
                if (value_ > max_){
                        max_ = value_;
                }
        }
        D get_value() {
                return value_;
        }
        void set(D val){
                value_ = val;
                if (value_>max_) {
                        max_ = value_;
                }
        }
        long get_max(){
                return max_;
        }
        void reset() {
                value_ = 0;
                max_=0;
        }
        void gather() {
                static_cast<T*>(this)->gather_impl();
        }
        void reduce() {
                static_cast<T*>(this)->reduce_impl();
        }
        friend std::ostream& operator<<(std::ostream& os,
                        const MaxCounter<T,D>& obj){
        return static_cast<const T&>(obj).stream_out(os);
    }
protected:
        D value_;
        D max_;
        std::ostream& stream_out(std::ostream& os) const {
                os << value_;
                return os;
        }
        DISALLOW_COPY_AND_ASSIGN(MaxCounter);
};


//class MaxCounter {
//public:
//      MaxCounter() :
//                      value_(0), max_(0) {
//      }
//      virtual ~MaxCounter() {
//      }
//      ;
//      void inc(long delta = 1) {
//              value_ += delta;
//              max_ = (value_ > max_) ? value_ : max_;
//      }
//      void set(long value) {
//              value_ = value;
//              max_ = (value_ > max_) ? value_ : max_;
//      }
//      long get_value() {
//              return value_;
//      }
//      long get_max() {
//              return max_;
//      }
//      void reset() {
//              value_ = 0;
//              max_ = 0;
//      }
//      friend std::ostream& operator<<(std::ostream& os, const MaxCounter& obj) {
//              return obj.stream_out(os);
//      }
//protected:
//      long value_;
//      long max_;
//      virtual std::ostream& stream_out(std::ostream& os) const {
//              os << value_ << ',' << max_;
//              return os;
//      }
//      DISALLOW_COPY_AND_ASSIGN(MaxCounter);
//};

#ifdef HAVE_MPI

class MPIMaxCounter: public MaxCounter<MPIMaxCounter,long long> {
public:
        explicit MPIMaxCounter(const MPI_Comm& comm, long long init=0) : MaxCounter(init),
                        comm_(comm), gathered_value_(0), reduced_value_(0), gathered_max_(0), reduced_max_(0)
        , gather_done_(false), reduce_done_(false){
        }
        ~MPIMaxCounter() {
        }

        friend class MaxCounter<MPIMaxCounter, long long>;
        friend std::ostream& operator<<(std::ostream& os,
                        const MaxCounter<MPIMaxCounter, long long>& obj);
protected:

        const MPI_Comm& comm_;
        std::vector<long long> gathered_value_;
        long long reduced_value_;
        std::vector<long long> gathered_max_;
        long long reduced_max_;
        bool gather_done_;
        bool reduce_done_;

        void gather_impl(){
                int rank;
                int comm_size;
                MPI_Comm_rank(comm_, &rank);
                MPI_Comm_size(comm_, &comm_size);
                if (rank == 0){
                        gathered_value_.resize(comm_size,0);
                        gathered_max_.resize(comm_size,0);
                        gather_done_=true;
                }
                if(comm_size>1){
                        MPI_Gather(&value_, 1, MPI_LONG_LONG, &gathered_value_.front(), 1, MPI_LONG_LONG, 0, comm_);
                        MPI_Gather(&max_, 1, MPI_LONG_LONG, &gathered_max_.front(), 1, MPI_LONG_LONG, 0, comm_);
                }
                else{
                        gathered_value_[0] = value_;
                        gathered_max_[0] = max_;
                }
        }
        void reduce_iml(){
                int rank;
                int comm_size;
                MPI_Comm_rank(comm_, &rank);
                MPI_Comm_size(comm_, &comm_size);
                if (rank == 0){
                        reduce_done_=true;
                }
                if(comm_size>1){
                        MPI_Reduce(&value_, &reduced_value_, 1, MPI_LONG_LONG, MPI_SUM, 0, comm_);
                        MPI_Reduce(&max_, &reduced_max_, 1, MPI_LONG_LONG, MPI_MAX, 0, comm_);
                }
                else{
                        reduced_value_ = value_;
                        reduced_max_ = max_;
                }
        }


        std::ostream& stream_out(std::ostream& os) const{
                if (!gather_done_ && !reduce_done_) {
                        //print own data
                        int rank;
                        MPI_Comm_rank(comm_, &rank);
                        os << "company rank," << rank << ", value_," << value_ << " max_," << max_ << std::endl;
                        return os;
                }
                int comm_size;
                MPI_Comm_size(comm_, &comm_size);

                if (gather_done_) {
                        //output gathered values in comma separated list
                        os << "value_";
                        std::vector<long long>::const_iterator it = gathered_value_.begin();
                        for (int j = 0; j < comm_size; ++j) {
                                os << ',' << *it;
                                ++it;
                        }
                        os << std::endl;
                        os << "max_";
                        std::vector<long long>::const_iterator itm = gathered_max_.begin();
                        for (int j = 0; j < comm_size; ++j) {
                                os << ',' << *itm;
                                ++itm;
                        }
                        os << std::endl;
                }
                if (reduce_done_) {
                        os << std::endl << "reduced mean";
                        os << static_cast<double>(reduced_value_)
                                                        / static_cast<double>(comm_size);
                        os << "reduced max";
                        os << reduced_max_;
                        os << std::endl;
                }
                return os;
        }
private:
        DISALLOW_COPY_AND_ASSIGN(MPIMaxCounter);
};

#endif // HAVE_MPI

//class MaxCounterList {
//public:
//      explicit MaxCounterList(size_t size) :
//                      list_(size) {
//      }
//      virtual ~MaxCounterList() {
//      }
//      void inc(int index, size_t delta = 1) {
//              list_[index].inc(delta);
//      }
//      void set(int index, long value) {
//              list_[index].set(value);
//      }
//      size_t get_value(int index) {
//              return list_[index].get_value();
//      }
//      void reset() {
//              for (std::vector<MaxCounter>::iterator it = list_.begin();
//                              it != list_.end(); ++it) {
//                      it->reset();
//              }
//      }
//      friend std::ostream& operator<<(std::ostream& os,
//                      const MaxCounterList& obj) {
//              return obj.stream_out(os);
//      }
//protected:
//      std::vector<MaxCounter> list_;
//      virtual std::ostream& stream_out(std::ostream& os) const;
//private:
//      DISALLOW_COPY_AND_ASSIGN(MaxCounterList);
//};
//
//class PMaxCounterList: public MaxCounterList {
//public:
//      PMaxCounterList(const MPI_Comm& comm, size_t size) :
//                      MaxCounterList(size), comm_(comm), gathered_vals_(0), reduced_vals_(
//                                      0) {
//      }
//      virtual ~PMaxCounterList() {
//      }
//      void gather();
//      void reduce();
//protected:
//      const MPI_Comm& comm_;
//      std::vector<long> gathered_vals_;
//      std::vector<long> reduced_vals_;
//      virtual std::ostream& stream_out(std::ostream& os) const;
//private:
//      DISALLOW_COPY_AND_ASSIGN(PMaxCounterList);
//};


} /* namespace sip */

#endif /* COUNTER_H_ */
