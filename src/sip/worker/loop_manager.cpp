/*
 * loop_manager.cpp
 *
 *  Created on: Aug 23, 2013
 *      Author: basbas
 */

#include "loop_manager.h"
#include <sstream>
#include "interpreter.h"

namespace sip {
LoopManager::LoopManager() :
		to_exit_(false) {
}
LoopManager::~LoopManager() {
}
void LoopManager::do_set_to_exit() {
	to_exit_ = true;
}
std::ostream& operator<<(std::ostream& os, const LoopManager &obj) {
	os << obj.to_string();
	return os;
}

std::string LoopManager::to_string() const {
	return std::string("LoopManager");
}

DoLoop::DoLoop(int index_id, DataManager & data_manager, const SipTables & sip_tables) :
		data_manager_(data_manager), sip_tables_(sip_tables),
		index_id_(index_id), first_time_(true) {
	lower_seg_ = sip_tables_.lower_seg(index_id);
	upper_bound_ = lower_seg_ + sip_tables_.num_segments(index_id);
//	sip::check_and_warn(lower_seg_ < upper_bound_,
//			std::string("doloop has empty range"),
//			Interpreter::global_interpreter->line_number());
}

DoLoop::~DoLoop() {
}

bool DoLoop::do_update() {
//	std::cout << "DEBUG DoLoop:update:46 \n" << this->to_string() << std::endl;
	if (to_exit_)
		return false;
	int current_value;
	if (first_time_) {  //initialize index to lower value
		first_time_ = false;
		sip::check(data_manager_.index_value(index_id_) == DataManager::undefined_index_value,
				"SIAL or SIP error, index " + sip_tables_.index_name(index_id_) + " already has value before loop",
				Interpreter::global_interpreter->line_number());
		current_value = lower_seg_;
	} else { //not the first time through loop.  Get the current value and try to increment it
		current_value = data_manager_.index_value(index_id_);
		++current_value;
	}
	if (current_value < upper_bound_) {
		data_manager_.set_index_value(index_id_, current_value);
		return true;
	}
	//If here on first time through, the range is empty. This leaves index undefined, which is required behavior.
	return false;
}
void DoLoop::do_finalize() {
	data_manager_.set_index_undefined(index_id_);
}

std::string DoLoop::to_string() const {
	std::stringstream ss;
	ss << "index_id_="
			<< sip_tables_.index_name(index_id_);
	ss << ", lower_seg_=" << lower_seg_;
	ss << ", upper_bound_=" << upper_bound_;
	ss << ", current= "
			<< data_manager_.index_value_to_string(index_id_);
	return ss.str();
}

std::ostream& operator<<(std::ostream& os, const DoLoop &obj) {
	os << obj.to_string();
	return os;
}
SubindexDoLoop::SubindexDoLoop(int subindex_id, DataManager & data_manager, const SipTables & sip_tables) :
		DoLoop(subindex_id, data_manager, sip_tables) {
	sip::check(sip_tables_.is_subindex(subindex_id), "Attempting subindex do loop with non-subindex loop variable");
	parent_id_ = sip_tables_.parent_index(subindex_id);
	parent_value_ = data_manager_.index_value(parent_id_);
	lower_seg_ = 1;  //subindices always start at 1
	upper_bound_ = lower_seg_ + sip_tables_.num_subsegments(subindex_id, parent_value_);
	sip::check_and_warn(lower_seg_ < upper_bound_,
			std::string("SubindexDoLoop has empty range"),
			Interpreter::global_interpreter->line_number());
}

std::string SubindexDoLoop::to_string() const {
	std::stringstream ss;
	ss << DoLoop::to_string();
	ss << ", parent_id_=" <<parent_id_;
	ss << ", parent_value_=" <<parent_value_;
	return ss.str();
}

std::ostream& operator<<(std::ostream& os, const SubindexDoLoop &obj) {
	os << obj.to_string();
	return os;
}

SubindexDoLoop::~SubindexDoLoop() {
}
//note that the max number of indices allowed by the implementation is MAX_RANK.  This limitation is
// due to the structure of the pardo instruction inherited from aces3
SequentialPardoLoop::SequentialPardoLoop(int num_indices,
		const int (&index_id)[MAX_RANK], DataManager & data_manager, const SipTables & sip_tables) :
		data_manager_(data_manager), sip_tables_(sip_tables),
		num_indices_(num_indices), first_time_(true) {
	std::copy(index_id + 0, index_id + MAX_RANK, index_id_ + 0);
	for (int i = 0; i < num_indices; ++i) {
		lower_seg_[i] = sip_tables_.lower_seg(
				index_id_[i]);
		upper_bound_[i] = lower_seg_[i]
				+ sip_tables_.num_segments(
						index_id_[i]);
		sip::check(lower_seg_[i] < upper_bound_[i],
				"Pardo loop index "
						+ sip_tables_.index_name(index_id_[i]) + " has empty range",
				Interpreter::global_interpreter->line_number());
	}
//	std::cout << "SequentialPardoLoop::SequentialPardoLoop at line " << Interpreter::global_interpreter->line_number()  << std::endl;
//
//		Interpreter::global_interpreter->set_index_value(index_ids_[i],
//				first_segments_[i]);
}

SequentialPardoLoop::~SequentialPardoLoop() {
}

bool SequentialPardoLoop::do_update() {
//	std::cout << "DEBUG: SequentialPardoLoop:112 \n" << this->to_string()
//			<< std::endl;
	if (to_exit_)
		return false;
	int current_value;
	if (first_time_) {
		first_time_ = false;
		//initialize values of all indices
		for (int i = 0; i < num_indices_; ++i) {
			if (lower_seg_[i] >= upper_bound_[i])
				return false; //this loop has an empty range in at least one dimension.
			sip::check(
					data_manager_.index_value(index_id_[i])
							== DataManager::undefined_index_value,
					"SIAL or SIP error, index "
							+ sip_tables_.index_name(
									index_id_[i])
							+ " already has value before loop",
					Interpreter::global_interpreter->line_number());
					data_manager_.set_index_value(index_id_[i],	lower_seg_[i]);
		}
		return true;
	} else {
		for (int i = 0; i < num_indices_; ++i) {
			current_value = data_manager_.index_value(
					index_id_[i]);
			++current_value;
			if (current_value < upper_bound_[i]) { //increment current index and return
				data_manager_.set_index_value(index_id_[i],	current_value);
				return true;
			} else { //wrap around and handle next index
				data_manager_.set_index_value(index_id_[i],	lower_seg_[i]);
			}
		} //if here, then all indices are at their max value
	}
	return false;
}

void SequentialPardoLoop::do_finalize() {
	for (int i = 0; i < num_indices_; ++i) {
		data_manager_.set_index_undefined(
				index_id_[i]);
	}

}

std::string SequentialPardoLoop::to_string() const {
	std::stringstream ss;
	ss << "Sequential Pardo Loop:  num_indices="<<num_indices_<< std::endl;
	ss << "index_ids_=[";
	for (int i = 0; i < num_indices_; ++i) {
		ss << (i == 0 ? "" : ",")
				<< sip_tables_.index_name(index_id_[i]);
	}
	ss << "] lower_seg_=[";
	for (int i = 0; i < num_indices_; ++i) {
		ss << (i == 0 ? "" : ",") << lower_seg_[i];
	}
	ss << "] upper_bound_=[";
	for (int i = 0; i < num_indices_; ++i) {
		ss << (i == 0 ? "" : ",") << upper_bound_[i];
	}
	ss << "] current= [";
	for (int i = 0; i < num_indices_; ++i) {
		ss << (i == 0 ? "" : ",")
				<< data_manager_.index_value_to_string(
						index_id_[i]);
	}
	ss << "]";
	return ss.str();
}
;


#ifdef HAVE_MPI
StaticTaskAllocParallelPardoLoop::StaticTaskAllocParallelPardoLoop(int num_indices,
		const int (&index_id)[MAX_RANK], DataManager & data_manager, const SipTables & sip_tables,
		int company_rank, int num_workers) :
		data_manager_(data_manager), sip_tables_(sip_tables),
		num_indices_(num_indices), first_time_(true), iteration_(0),
		company_rank_(company_rank), num_workers_(num_workers){

	std::copy(index_id + 0, index_id + MAX_RANK, index_id_ + 0);
	for (int i = 0; i < num_indices; ++i) {
		lower_seg_[i] = sip_tables_.lower_seg(index_id_[i]);
		upper_bound_[i] = lower_seg_[i] + sip_tables_.num_segments(index_id_[i]);
		sip::check(lower_seg_[i] < upper_bound_[i],	"Pardo loop index " + sip_tables_.index_name(index_id_[i]) + " has empty range", Interpreter::global_interpreter->line_number());
	}
}

StaticTaskAllocParallelPardoLoop::~StaticTaskAllocParallelPardoLoop() {}

inline bool StaticTaskAllocParallelPardoLoop::increment_indices() {
	bool more = false; 	// More iterations?
	int current_value;
	for (int i = 0; i < num_indices_; ++i) {
		current_value = data_manager_.index_value(index_id_[i]);
		++current_value;
		if (current_value < upper_bound_[i]) {
			//increment current index and return
			data_manager_.set_index_value(index_id_[i], current_value);
			more = true;
			break;
		} else {
			//wrap around and handle next index
			data_manager_.set_index_value(index_id_[i], lower_seg_[i]);
		}
	} //if here, then all indices are at their max value
	return more;
}

inline bool StaticTaskAllocParallelPardoLoop::initialize_indices() {
	//initialize values of all indices
	bool more_iterations = true;
	for (int i = 0; i < num_indices_; ++i) {
		if (lower_seg_[i] >= upper_bound_[i]){
			more_iterations = false; //this loop has an empty range in at least one dimension.
			return more_iterations;
		}

		sip::check(
				data_manager_.index_value(index_id_[i])
						== DataManager::undefined_index_value,
				"SIAL or SIP error, index "
						+ sip_tables_.index_name(index_id_[i])
						+ " already has value before loop",
				Interpreter::global_interpreter->line_number());
		data_manager_.set_index_value(index_id_[i], lower_seg_[i]);
	}
	more_iterations = true;
	return more_iterations;
}

bool StaticTaskAllocParallelPardoLoop::do_update() {

	if (to_exit_)
		return false;

	if (first_time_) {
		first_time_ = false;
		bool more_iters = initialize_indices();
		while (more_iters && iteration_ % num_workers_ != company_rank_){
			more_iters = increment_indices();
			iteration_++;
		}
		return more_iters;
	} else {
		iteration_++;
		bool more_iters = increment_indices();
		while (more_iters && iteration_ % num_workers_ != company_rank_){
			more_iters = increment_indices();
			iteration_++;
		}
		return more_iters;
	}
}


void StaticTaskAllocParallelPardoLoop::do_finalize() {
	for (int i = 0; i < num_indices_; ++i) {
		data_manager_.set_index_undefined(index_id_[i]);
	}
}

std::string StaticTaskAllocParallelPardoLoop::to_string() const {
	std::stringstream ss;
	ss << "Static Task Allocation Parallel Pardo Loop:  num_indices="<<num_indices_<< std::endl;
	ss << "index_ids_=[";
	for (int i = 0; i < num_indices_; ++i) {
		ss << (i == 0 ? "" : ",")
				<< sip_tables_.index_name(index_id_[i]);
	}
	ss << "] lower_seg_=[";
	for (int i = 0; i < num_indices_; ++i) {
		ss << (i == 0 ? "" : ",") << lower_seg_[i];
	}
	ss << "] upper_bound_=[";
	for (int i = 0; i < num_indices_; ++i) {
		ss << (i == 0 ? "" : ",") << upper_bound_[i];
	}
	ss << "] current= [";
	for (int i = 0; i < num_indices_; ++i) {
		ss << (i == 0 ? "" : ",")
				<< data_manager_.index_value_to_string(
						index_id_[i]);
	}
	ss << "]";
	return ss.str();
}

std::ostream& operator<<(std::ostream& os, const StaticTaskAllocParallelPardoLoop &obj) {
	os << obj.to_string();
	return os;
}

#endif

} /* namespace sip */
