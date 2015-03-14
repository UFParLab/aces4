/*
 * loop_manager.h
 *
 *  Created on: Aug 23, 2013
 *      Author: basbas
 */

#ifndef LOOP_MANAGER_H_
#define LOOP_MANAGER_H_

#include "config.h"
#include "aces_defs.h"
#include "sip.h"
#include "array_constants.h"
#include "data_manager.h"

#include "sip_mpi_attr.h"

namespace sip {

class SialxInterpreter;

/*! Base class for loop managers. */
class LoopManager {
public:
	LoopManager();
	virtual ~LoopManager();
	/* sets the value of the managed indices to the next value and returns true
	 * If there are no more values, the loop is done, return false;
	 */
	bool update() {
		return do_update();
	}
	/** reset the value of all managed indices to their undefined value */
	void finalize() {
		do_finalize();
	}
	/** set flag to immediately exit loop */
	void set_to_exit() {
		do_set_to_exit();
	}

	friend std::ostream& operator<<(std::ostream&, const LoopManager &);
protected:
	bool to_exit_;
	virtual std::string to_string() const;
private:
	virtual bool do_update() = 0;
	virtual void do_finalize() = 0;
	virtual void do_set_to_exit();DISALLOW_COPY_AND_ASSIGN(LoopManager);
};

class DoLoop: public LoopManager {
public:
	DoLoop(int index_id, DataManager & data_manager,
			const SipTables & sip_tables);
	virtual ~DoLoop();
	friend std::ostream& operator<<(std::ostream&, const DoLoop &);
protected:
	bool first_time_;
	int index_id_;
	int lower_seg_;
	int upper_bound_;  // first_segment_ <= current_value_ < upper_bound

	DataManager & data_manager_;
	const SipTables & sip_tables_;

	virtual std::string to_string() const;
private:
	virtual bool do_update();
	virtual void do_finalize();

	DISALLOW_COPY_AND_ASSIGN(DoLoop);
};

class SubindexDoLoop: public DoLoop {
public:
	SubindexDoLoop(int subindex_id, DataManager & data_manager,
			const SipTables & sip_tables);
	virtual ~SubindexDoLoop();
	friend std::ostream& operator<<(std::ostream&, const SubindexDoLoop &);
private:
	int parent_id_;
	int parent_value_;
	virtual std::string to_string() const;DISALLOW_COPY_AND_ASSIGN(SubindexDoLoop);
};
class SequentialPardoLoop: public LoopManager {
public:
	SequentialPardoLoop(int num_indices, const int (&index_ids)[MAX_RANK],
			DataManager & data_manager, const SipTables & sip_tables);
	virtual ~SequentialPardoLoop();
	friend std::ostream& operator<<(std::ostream&, const SequentialPardoLoop &);
private:
	virtual std::string to_string() const;
	virtual bool do_update();
	virtual void do_finalize();
	bool first_time_;
	int num_indices_;
	sip::index_selector_t index_id_;
	sip::index_value_array_t lower_seg_;
	sip::index_value_array_t upper_bound_;

	DataManager & data_manager_;
	const SipTables & sip_tables_;

	DISALLOW_COPY_AND_ASSIGN(SequentialPardoLoop);
};

#ifdef HAVE_MPI
class StaticTaskAllocParallelPardoLoop: public LoopManager {
public:
	StaticTaskAllocParallelPardoLoop(int num_indices, const int (&index_ids)[MAX_RANK], DataManager & data_manager, const SipTables & sip_tables,
			int company_rank, int num_workers);
	virtual ~StaticTaskAllocParallelPardoLoop();
	friend std::ostream& operator<<(std::ostream&,
			const StaticTaskAllocParallelPardoLoop &);
private:
	virtual std::string to_string() const;
	virtual bool do_update();
	virtual void do_finalize();
	bool first_time_;
	int num_indices_;
	long iteration_;
	sip::index_selector_t index_id_;
	sip::index_value_array_t lower_seg_;
	sip::index_value_array_t upper_bound_;

    DataManager & data_manager_;
    const SipTables & sip_tables_;
    const int num_workers_;
    const int company_rank_;

	bool increment_indices();
	bool initialize_indices();

	DISALLOW_COPY_AND_ASSIGN(StaticTaskAllocParallelPardoLoop);

};




class BalancedTaskAllocParallelPardoLoop: public LoopManager {
public:
	BalancedTaskAllocParallelPardoLoop(int num_indices,
			const int (&index_ids)[MAX_RANK], DataManager & data_manager,
			const SipTables & sip_tables, SIPMPIAttr& sip_mpi_attr,
			int num_where_clauses, SialxInterpreter* interpreter);
	virtual ~BalancedTaskAllocParallelPardoLoop();
	friend std::ostream& operator<<(std::ostream&,
			const BalancedTaskAllocParallelPardoLoop &);
private:
	virtual std::string to_string() const;
	virtual bool do_update();
	virtual void do_finalize();
	bool first_time_;
	int num_indices_;
	long iteration_;
	index_selector_t index_id_;
	index_value_array_t lower_seg_;
	index_value_array_t upper_bound_;
	int num_where_clauses_;

	DataManager & data_manager_;
	const SipTables & sip_tables_;
	SIPMPIAttr & sip_mpi_attr_;
	int company_rank_;
	int num_workers_;
	SialxInterpreter* interpreter_;

	bool increment_indices();
	bool initialize_indices();

	DISALLOW_COPY_AND_ASSIGN(BalancedTaskAllocParallelPardoLoop);

};
#endif

} /* namespace sip */
#endif /* LOOP_MANAGER_H_ */
