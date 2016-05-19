/*
 * timer.h
 *
 *  Created on: Feb 26, 2016
 *      Author: basbas
 */

#ifndef TIMER_H_
#define TIMER_H_

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

template<typename T, typename R>
class Timer {
public:
        Timer() :
                        total_(R()), start_(R()), max_(R()), num_epochs_(0), on_(false) {
        }
        void start() {
                start_ = get_time();
                CHECK(!on_, "starting timer already on");
                on_ = true;
        }
        R get_time() {
                return static_cast<T*>(this)->get_time_impl();
        }

        double diff(R start, R end) {
                return static_cast<T*>(this)->diff_impl(start, end);
        }
        void pause() {
                double elapsed = diff(start_, get_time());
                total_ += elapsed;
                if (elapsed > max_) {
                        max_ = elapsed;
                }
                CHECK(on_, "pausing time that is not on");
                on_ = false;
        }
        void gather() {
                static_cast<T*>(this)->gather_impl();
        }
        void reduce() {
                static_cast<T*>(this)->reduce_impl();
        }

        double get_mean(){
                return static_cast<T*>(this)->get_mean_impl();
        }

        double get_total() {
                return total_;
        }
        double get_max() {
                return max_;
        }
        unsigned long get_num_epochs() {
                return num_epochs_;
        }

        friend std::ostream& operator<<(std::ostream& os, const Timer<T, R>& obj){
            return static_cast<const T&>(obj).stream_out(os);
    }

protected:
        double total_;
        R start_;
        double max_;
        unsigned long num_epochs_;
        bool on_;
};


/**
 * This class uses CRTP (curiously recurring template pattern).
 *
 * T is the type of the subclass, R is the type returned by the get time method.
 * For MPI_Wtime, R is double, for LinuxTimer that uses time(), the type is time_t.
 */
template<typename T, typename R>
class TimerList {
public:
        TimerList(size_t size) :
                        size_(size), total_(size, 0), start_(size, R()), max_(size,0), num_epochs_(
                                        size, 0), on_(size, false) {
        }
        void start(size_t index) {
                start_.at(index) = get_time();
                CHECK(!on_.at(index), "starting timer already on");
                on_.at(index) = true;
        }

        R get_time() {
                return static_cast<T*>(this)->get_time_impl();
        }

        double diff(R start, R end) {
                return static_cast<T*>(this)->diff_impl(start, end);
        }

        void pause(size_t index) {
                double elapsed = diff(start_.at(index), get_time());
                total_.at(index) += elapsed;
                ++num_epochs_.at(index);
                if (elapsed > max_.at(index)) {
                        max_.at(index) = elapsed;
                }
                CHECK(on_.at(index), "pausing time that is not on");
                on_.at(index) = false;
        }
        void inc(size_t index, double elapsed) {
                total_.at(index) += elapsed;
                ++num_epochs_.at(index);
                if (elapsed > max_.at(index)) {
                        max_.at(index) = elapsed;
                }
                CHECK(!on_.at(index), "incrementing timer that is on");
        }

        void gather() {
                static_cast<T*>(this)->gather_impl();
        }
        void reduce() {
                static_cast<T*>(this)->reduce_impl();
        }

        void print_op_table_stats(std::ostream& os,
                        const SipTables& sip_tables) const {
                static_cast<const T*>(this)->print_op_table_stats_impl(os, sip_tables);
        }

//      const std::vector<double>& get_total() {
//              return total_;
//      }
//      const std::vector<double>& get_max() {
//              return max_;
//      }
//      const std::vector<unsigned long> get_num_epochs() {
//              return num_epochs_;
//      }
//      size_t get_size() {
//              return size_;
//      }

        friend std::ostream& operator<<(std::ostream& os,
                        const TimerList<T, R>& obj){
            return static_cast<const T&>(obj).stream_out(os);
    }

protected:
        std::vector<double> total_;
        std::vector<R> start_;
        std::vector<double> max_;
        std::vector<unsigned long> num_epochs_;
        std::vector<bool> on_;
        size_t size_;
};


class LinuxTimerList: public TimerList<LinuxTimerList, std::time_t> {
public:
        LinuxTimerList(size_t size) :
                        TimerList<LinuxTimerList, std::time_t>(size) {
        }
        ~LinuxTimerList() {
        }
        std::time_t get_time_impl() {
                std::time_t now;
                time(&now);
                return now;
        }

        double diff_impl(std::time_t start, std::time_t end) {
                return std::difftime(end, start);
        }

        void gather_impl() {
        }
        void reduce_impl() {
        }

        std::ostream& stream_out(std::ostream& os) const {
                std::setprecision(30);
                os << "total_,  max_,  num_epochs_" << std::endl;
                std::vector<double>::const_iterator total_iter = total_.begin();
                ;
                std::vector<double>::const_iterator max_iter = max_.begin();
                std::vector<unsigned long>::const_iterator epoch_iter =
                                num_epochs_.begin();
                for (int i = 0; i < size_; ++i) {
                        os << i << ',' << *(total_iter++) << ',' << *(max_iter++) << ','
                                        << *(epoch_iter++) << std::endl;
                }
                return os;
        }

        void print_op_table_stats_impl(std::ostream& os,
                        const SipTables& sip_tables) const {
                os << std::setprecision(30);
                os << "pc, line number, opcode, mean,  max,  num_epochs" << std::endl;
                std::vector<double>::const_iterator total_iter = total_.begin();
                ;
                std::vector<double>::const_iterator max_iter = max_.begin();
                std::vector<unsigned long>::const_iterator epoch_iter =
                                num_epochs_.begin();
                for (int i = 0; i < size_; ++i) {
                        os << i << ',' << sip_tables.line_number(i) << ','
                                        << sip_tables.opcode_name(i) << ',';
                        double mean = *total_iter / *epoch_iter;
                        os << mean << ',' << *max_iter << ',' << *epoch_iter;
                        ++total_iter;
                        ++max_iter;
                        ++epoch_iter;
                }
        }

};


#ifdef HAVE_MPI

class MPITimerList: public TimerList<MPITimerList, double> {
public:
        MPITimerList(const MPI_Comm& comm, size_t size) :
                        TimerList<MPITimerList, double>(size), comm_(comm), gathered_total_(
                                        0), gathered_max_(0), gathered_num_epoch_(0), reduced_mean_(
                                        0), reduced_max_(0), gather_done_(false), reduce_done_(
                                        false) {
        }
        ~MPITimerList() {
        }
        friend class TimerList<MPITimerList, double>;
        friend std::ostream& operator<<(
                        std::ostream& os, const TimerList<MPITimerList, double>& obj);

        double get_time_impl() {
                return MPI_Wtime();
        }
        double diff_impl(double start, double end) {
                return end - start;
        }

        void gather_impl() {
                int rank; //this rank is relative to comm_
                int comm_size;
                MPI_Comm_rank(comm_, &rank);
                MPI_Comm_size(comm_, &comm_size);

                if (rank == 0) {
                        size_t buffsize = size_ * comm_size;
                        gathered_total_.resize(buffsize, 0.0);
                        gathered_max_.resize(buffsize, 0.0);
                        gathered_num_epoch_.resize(buffsize, 0.0);
                        gather_done_ = true;
                }

                if (comm_size > 1) {
                        MPI_Gather(&total_.front(), size_, MPI_DOUBLE, &gathered_total_.front(),
                                        size_, MPI_DOUBLE, 0, comm_);
                        MPI_Gather(&max_.front(), size_, MPI_DOUBLE, &gathered_max_.front(),
                                        size_, MPI_DOUBLE, 0, comm_);
                        MPI_Gather(&num_epochs_.front(), size_, MPI_UNSIGNED_LONG,
                                        &gathered_num_epoch_.front(), size_, MPI_UNSIGNED_LONG, 0,
                                        comm_);

                } else {
                        if (rank == 0){
                        gathered_total_ = total_;
                        gathered_max_ = max_;
                        gathered_num_epoch_ = num_epochs_;
                        }
                }
        }

        void reduce_impl() {
                int rank;
                int comm_size;
                MPI_Comm_rank(comm_, &rank);
                MPI_Comm_size(comm_, &comm_size);

                if (rank == 0) {
                        reduced_mean_.resize(size_, 0.0);
                        reduced_max_.resize(size_, 0.0);
                        reduced_num_epoch_.resize(size_, 0);
                        reduce_done_=true;
                }
                //reduce total and calculate mean
                MPI_Reduce(&total_.front(), &reduced_mean_.front(), size_, MPI_DOUBLE,
                                MPI_SUM, 0, comm_); //this is actually the total. need to divide by #epochs
                MPI_Reduce(&max_.front(), &reduced_max_.front(), size_, MPI_DOUBLE, MPI_MAX,
                                0, comm_);
                MPI_Reduce(&num_epochs_.front(), &reduced_num_epoch_.front(), size_,
                                MPI_UNSIGNED_LONG, MPI_SUM, 0, comm_);
                if (rank == 0) {
                        std::vector<double>::iterator miter = reduced_mean_.begin();
                        std::vector<unsigned long>::iterator niter =
                                        reduced_num_epoch_.begin();
                        for (int j = 0; j < size_; ++j) {
                                if (*niter > 0) {
                                        *miter = *miter / *niter;
                                }
                                ++miter;
                                ++niter;
                        }
                }
        }

        std::ostream& stream_out(std::ostream& os) const {

                if (!gather_done_ && !reduce_done_) {
                        std::setprecision(30);
                        //print own data
                        int rank;
                        MPI_Comm_rank(comm_, &rank);
                        os << "rank=" << rank << std::endl;
                        os << "total_  max_  num_epochs_" << std::endl;
                        std::vector<double>::const_iterator total_iter = total_.begin();
                        ;
                        std::vector<double>::const_iterator max_iter = max_.begin();
                        std::vector<unsigned long>::const_iterator epoch_iter =
                                        num_epochs_.begin();
                        for (int i = 0; i < size_; ++i) {
                                os << i << ',' << *(total_iter++) << ',' << *(max_iter++) << ','
                                                << *(epoch_iter++) << std::endl;
                        }
                        return os;
                }

                int comm_size;
                MPI_Comm_size(comm_, &comm_size);
                int num_double_vals = 2;  //total_ and max_
                int num_unsigned_long_vals = 1; //num_epochs_
                std::vector<double>::const_iterator it;

                if (gather_done_) {
                        //output the gathered values in a comma separated list

                        os << "total" << std::endl;
                        ;
                        int i = 0;
                        while (i < size_) {
                                os << i;
                                it = gathered_total_.begin() + i;
                                for (int j = 0; j < comm_size; ++j) {
                                        os << ',' << *it;
                                        it += size_;
                                }
                                os << std::endl;
                                ++i;
                        }
                        //output the values for max
                        os << "max" << std::endl;
                        i = 0;
                        while (i < size_) {
                                os << i;
                                it = gathered_max_.begin() + i;
                                for (int j = 0; j < comm_size; ++j) {
                                        os << ',' << *it;
                                        it += size_;
                                }
                                os << std::endl;
                                ++i;
                        }
                }

                if (reduce_done_) {
                        std::vector<double>::const_iterator mean_iter =
                                        reduced_mean_.begin();
                        std::vector<double>::const_iterator max_iter = reduced_max_.begin();
                        std::vector<unsigned long>::const_iterator num_epoch_iter =
                                        reduced_num_epoch_.begin();
                        os << "reduced values:  mean, max, num_epoch" << std::endl;
                        for (int i = 0; i != size_; ++i) {
                                os << i << ',' << *mean_iter << ',' << *max_iter << ','
                                                << *num_epoch_iter++ << std::endl;
                                ++mean_iter;
                                ++max_iter;
                                ++num_epoch_iter;
                        }
                }
                return os;
        }

        void print_op_table_stats_impl(std::ostream& os,
                        const SipTables& sip_tables, bool filter = true) const {

                int comm_size;
                MPI_Comm_size(comm_, &comm_size);
                CHECK(reduce_done_, "must call reduce before print_optable_stats");
                os << "pc, line number, opcode, mean,  max,  mean num_epochs";
                os << std::endl;
                std::vector<double>::const_iterator mean_iter = reduced_mean_.begin();
                std::vector<double>::const_iterator max_iter = reduced_max_.begin();
                std::vector<unsigned long>::const_iterator num_epoch_iter =
                                reduced_num_epoch_.begin();
                for (int i = 0; i != size_; ++i) {
                        if (!filter || *num_epoch_iter > 0) {
                                std::setprecision(30);
                                os << i << ',' << sip_tables.line_number(i) << ','
                                                << sip_tables.opcode_name(i) << ',';
                                os << *mean_iter << ',' << *max_iter << ',';
                                std::setprecision(3);
                                os<< (double)(*num_epoch_iter) / comm_size;
                                os << std::endl;
                        }
                        ++mean_iter;
                        ++max_iter;
                        ++num_epoch_iter;
                }
        }

protected:
        const MPI_Comm& comm_;
        std::vector<double> gathered_total_;
        std::vector<double> gathered_max_;
        std::vector<unsigned long> gathered_num_epoch_;
        std::vector<double> reduced_mean_;
        std::vector<double> reduced_max_;
        std::vector<unsigned long> reduced_num_epoch_;
        bool gather_done_;
        bool reduce_done_;

};

#endif // HAVE_MPI

template<typename R>
class NoopTimerList {
public:
        NoopTimerList(size_t size) {}
        void start(size_t index) {
        }
        R get_time() {
                return 0;
        }

        double diff(time_t start, time_t end) {
                return 0;
        }

        void pause(size_t index) {
        }
        void inc(size_t index, double elapsed) {
        }

        double get_mean(){return 0.0;}

        void gather() {
        }
        void reduce() {
        }

        void print_op_table_stats(std::ostream& os,
                        const SipTables& sip_tables) const {
        }

        const std::vector<double>& get_total() {
                return std::vector<double>();
        }
        const std::vector<double>& get_max() {
                return std::vector<double>();
        }
        const std::vector<unsigned long> get_num_epochs() {
                return std::vector<unsigned long>();
        }
        size_t get_size() {
                return 0;
        }

        friend std::ostream& operator<<(std::ostream& os, const NoopTimerList<R>& obj){
        return os;
    }
};


class LinuxTimer: public Timer<LinuxTimer, time_t> {
public:
        LinuxTimer() {
        }
        ~LinuxTimer() {
        }
        friend class Timer<LinuxTimer, time_t>;
        friend std::ostream& operator<<(
                                std::ostream& os, const Timer<LinuxTimer, time_t>& obj);
protected:
        time_t get_time_impl() {
                time_t now;
                time(&now);
                return now;
        }

        double diff_impl(time_t start, time_t end) {
                return difftime(end, start);
        }
        double get_mean_impl(){return total_;}

        void gather_impl() {
        }
        void reduce_impl() {
        }
        std::ostream& stream_out(std::ostream& os) const {
                os << ", total_," << total_;
                os << ", max_," << max_;
                os << ", num_epochs_," << num_epochs_;
                os << std::endl;
                return os;
        }
};


#ifdef HAVE_MPI

class MPITimer: public Timer<MPITimer, double> {
public:
        MPITimer(const MPI_Comm& comm) :
                        comm_(comm), gathered_total_(0), gathered_max_(0), gathered_num_epochs_(
                                        0), reduced_mean_(0), reduced_max_(0), gather_done_(false), reduce_done_(
                                        false) {
        }
        ~MPITimer() {
        }

    friend class Timer<MPITimer, double>;
        friend std::ostream& operator<<(
                        std::ostream& os, const Timer<MPITimer, double>& obj);

protected:

        double get_time_impl() {
                return MPI_Wtime();
        }

        double diff_impl(double start, double end) {
                return end - start;
        }
        double get_mean_impl(){
                CHECK(reduce_done_, "must call reduce before retrieving mean");
                return reduced_mean_;
        }
        const MPI_Comm& comm_;
        std::vector<double> gathered_total_;
        std::vector<double> gathered_max_;
        std::vector<unsigned long> gathered_num_epochs_;
        double reduced_mean_;
        double reduced_max_;
        bool gather_done_;
        bool reduce_done_;

        void gather_impl() {
                int rank;
                int comm_size;
                MPI_Comm_rank(comm_, &rank);
                MPI_Comm_size(comm_, &comm_size);

                //resize destination buffers
                if (rank == 0) {
                        gathered_total_.resize(comm_size, 0.0);
                        gathered_max_.resize(comm_size, 0.0);
                        gathered_num_epochs_.resize(comm_size, 0);
                        gather_done_ = true;
                }
                if (comm_size > 1) {
                        MPI_Gather(&total_, 1, MPI_DOUBLE, &gathered_total_.front(), 1,
                                        MPI_DOUBLE, 0, comm_);
                        MPI_Gather(&max_, 1, MPI_DOUBLE, &gathered_max_.front(), 1,
                                        MPI_DOUBLE, 0, comm_);
                        MPI_Gather(&num_epochs_, 1, MPI_UNSIGNED_LONG,
                                        &gathered_num_epochs_.front(), 1, MPI_UNSIGNED_LONG, 0,
                                        comm_);
                } else {
                        gathered_total_[0] = total_;
                        gathered_max_[0] = max_;
                        gathered_num_epochs_[0] = num_epochs_;
                }

        }
        void reduce_impl() {
                int rank;
                int comm_size;
                MPI_Comm_rank(comm_, &rank);
                MPI_Comm_size(comm_, &comm_size);

                if (comm_size > 1) {
                        //reduce total
                        MPI_Reduce(&total_, &reduced_mean_, 1, MPI_DOUBLE, MPI_SUM, 0,
                                        comm_);
                        //reduce max
                        MPI_Reduce(&max_, &reduced_max_, 1, MPI_DOUBLE, MPI_MAX, 0, comm_);
                } else {
                        reduced_mean_ = total_;
                        reduced_max_ = max_;
                }
                if (rank == 0) {
                        reduce_done_ = true;
                        reduced_mean_ /= comm_size; //calculate mean
                }
        }

        std::ostream& stream_out(std::ostream& os) const {
                std::setprecision(30);
                if (!gather_done_ && !reduce_done_) {
                        //print own data
                        int rank;
                        MPI_Comm_rank(comm_, &rank);
                        os << "rank=" << rank;
                        os << ", total_=" << total_;
                        os << ", max_=" << max_;
                        os << ", num_epochs_=" << num_epochs_;
                        os << std::endl;
                        return os;
                }

                int comm_size;
                MPI_Comm_size(comm_, &comm_size);
                int num_double_vals = 2;  //total_ and max_
                int num_unsigned_long_vals = 1; //num_epochs_
                std::vector<double>::const_iterator it;

                if (gather_done_) {
                        //output the gathered values in a comma separated list
                        os << "total";
                        it = gathered_total_.begin();
                        for (int j = 0; j < comm_size; ++j) {
                                os << ',' << *it;
                                ++it;
                        }
                        os << std::endl;

                        //output the values for max
                        os << "max";
                        it = gathered_max_.begin();
                        for (int j = 0; j < comm_size; ++j) {
                                os << ',' << *it;
                                ++it;
                        }
                        os << std::endl;
                }
                if (reduce_done_) {
                        os << "mean," << reduced_mean_ << std::endl;
                        os << "max epoch," << reduced_max_ << std::endl;
                }
                return os;
        }
private:
        DISALLOW_COPY_AND_ASSIGN(MPITimer);
};

#endif //HAVE_MPI







} /* namespace sip */

#endif /* TIMER_H_ */
