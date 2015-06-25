/*
 * sipmap_interpreter.h
 *
 *  Created on: Oct 9, 2014
 *      Author: njindal
 */

#ifndef SIPMAP_INTERPRETER_H_
#define SIPMAP_INTERPRETER_H_

#include "abstract_control_flow_interpreter.h"
#include "profile_timer.h"
#include "profile_timer_store.h"
#include "sipmap_timer.h"
#include "remote_array_model.h"


#include <utility>
#include <map>
#include <list>
#include <vector>
#include <iostream>

namespace sip {
class ProfileTimerStore;
class SipTables;


class SIPMaPConfig {
public:
	SIPMaPConfig(std::istream& input_file);
	RemoteArrayModel::Parameters& get_parameters() { return parameters_; }
private:
	RemoteArrayModel::Parameters parameters_;
	std::istream& input_file_;
};


/**
 * SIP Modeling and Prediction (SIPMaP) interpreter.
 * Models the Sialx Program based on information
 * made available from the profile interpreter run.
 * Profile information is made available through an
 * instance of ProfileTimerStore which encapsulates
 * a sqlite3 database.
 * Time per line is saved into the SIPMaPTimer instance.
 */
class SIPMaPInterpreter: public AbstractControlFlowInterpreter {
public:
	SIPMaPInterpreter(int worker_rank, int num_workers,
			const SipTables& sipTables,
			const RemoteArrayModel& remote_array_model,
			const ProfileTimerStore& profile_timer_store,
			SIPMaPTimer& sipmap_timer);
	virtual ~SIPMaPInterpreter();


	virtual SialPrinter* get_printer() { return NULL; }
	virtual void post_interpret(int old_pc, int new_pc) {}
	virtual void pre_interpret(int pc) {}


//	virtual void handle_jump_if_zero_op(int &pc);	// TODO - To visit if & else parts of the code.


	virtual void handle_broadcast_static_op(int pc) { fail("handle_broadcast_static not modeled!", line_number()); }
	virtual void handle_allocate_op(int pc) { /* TODO Time to zero out blocks */ }
	virtual void handle_deallocate_op(int pc) {}
	virtual void handle_allocate_contiguous_op(int pc) { fail ("handle_allocate_contiguous_op not modeled!", line_number()); }
	virtual void handle_deallocate_contiguous_op(int pc) { fail ("handle_deallocate_contiguous_op not modeled!", line_number()); }

	virtual void handle_collective_sum_op(int pc) {  expression_stack_.pop(); /* TODO Time for MPI_BCast */ }
	virtual void handle_assert_same_op(int pc) { /* TODO Time for MPI_BCast */ }
	virtual void handle_print_string_op(int pc) { control_stack_.pop();}
	virtual void handle_print_scalar_op(int pc) {  expression_stack_.pop();}
	virtual void handle_print_int_op(int pc) { control_stack_.pop();}
	virtual void handle_print_index_op(int pc) { control_stack_.pop();}
	virtual void handle_print_block_op(int pc) { block_selector_stack_.pop(); }
	virtual void handle_println_op(int pc) {}
	virtual void handle_gpu_on_op(int pc) {}
	virtual void handle_gpu_off_op(int pc) {}


	// Counting time for these opcodes may be relevant
	virtual void handle_do_op(int &pc);
	virtual void handle_enddo_op(int &pc);
	virtual void handle_jump_if_zero_op(int &pc);
	virtual void handle_stop_op(int &pc);
	virtual void handle_exit_op(int &pc);
	virtual void handle_push_block_selector_op(int pc);
	virtual void handle_string_load_literal_op(int pc);
	virtual void handle_int_load_literal_op(int pc);
	virtual void handle_int_load_value_op(int pc);
	virtual void handle_int_store_op(int pc);
	virtual void handle_int_add_op(int pc);
	virtual void handle_int_subtract_op(int pc);
	virtual void handle_int_multiply_op(int pc);
	virtual void handle_int_divide_op(int pc);
	virtual void handle_int_equal_op(int pc);
	virtual void handle_int_nequal_op(int pc);
	virtual void handle_int_le_op(int pc);
	virtual void handle_int_gt_op(int pc);
	virtual void handle_int_lt_op(int pc);
	virtual void handle_int_neg_op(int pc);
	virtual void handle_cast_to_int_op(int pc);
	virtual void handle_index_load_value_op(int pc);
	virtual void handle_scalar_load_value_op(int pc);
	virtual void handle_scalar_store_op(int pc);
	virtual void handle_scalar_add_op(int pc);
	virtual void handle_scalar_subtract_op(int pc);
	virtual void handle_scalar_multiply_op(int pc);
	virtual void handle_scalar_divide_op(int pc);
	virtual void handle_scalar_exp_op(int pc);
	virtual void handle_scalar_eq_op(int pc);
	virtual void handle_scalar_ne_op(int pc);
	virtual void handle_scalar_ge_op(int pc);
	virtual void handle_scalar_le_op(int pc);
	virtual void handle_scalar_gt_op(int pc);
	virtual void handle_scalar_lt_op(int pc);
	virtual void handle_scalar_neg_op(int pc);
	virtual void handle_scalar_sqrt_op(int pc);
	virtual void handle_idup_op(int pc);
	virtual void handle_iswap_op(int pc);
	virtual void handle_sswap_op(int pc);


	// Counting time for these opcodes
	// is relevant & significant
	virtual void handle_block_copy_op(int pc);
	virtual void handle_block_permute_op(int pc);
	virtual void handle_block_fill_op(int pc);
	virtual void handle_block_scale_op(int pc) ;
	virtual void handle_block_scale_assign_op(int pc) ;
	virtual void handle_block_accumulate_scalar_op(int pc);
	virtual void handle_block_add_op(int pc);
	virtual void handle_block_subtract_op(int pc);
	virtual void handle_block_contract_op(int pc);
	virtual void handle_block_contract_to_scalar_op(int pc);
	virtual void handle_block_load_scalar_op(int pc);
	virtual void handle_execute_op(int pc);

	// Methods to override for Model multinode aces4
#ifdef HAVE_MPI
	virtual void handle_sip_barrier_op(int pc);
	virtual void handle_get_op(int pc);
	virtual void handle_put_accumulate_op(int pc);
	virtual void handle_put_replace_op(int pc) ;
	virtual void handle_put_initialize_op(int pc);
	virtual void handle_put_increment_op(int pc);
	virtual void handle_put_scale_op(int pc);
	virtual void handle_set_persistent_op(int pc) ;
	virtual void handle_restore_persistent_op(int pc);
	virtual void handle_create_op(int pc);
	virtual void handle_delete_op(int pc);
	virtual void handle_pardo_op(int &pc);	//! Overriden to simulate 1 worker at a time
	virtual void do_post_sial_program();	//! Overriden to count the time in the final pardo section

#else // HAVE_MPI
	virtual void handle_sip_barrier_op(int pc) {}
	virtual void handle_get_op(int pc) { block_selector_stack_.pop(); }
	virtual void handle_put_accumulate_op(int pc) { block_selector_stack_.pop(); block_selector_stack_.pop();}
	virtual void handle_put_replace_op(int pc) {block_selector_stack_.pop(); block_selector_stack_.pop();}
	virtual void handle_create_op(int pc) {}
	virtual void handle_delete_op(int pc) {}
	virtual void handle_set_persistent_op(int pc) {}
	virtual void handle_restore_persistent_op(int pc){}

#endif // HAVE_MPI


	struct PardoSectionsInfo{
		int pc;
		int section;
		double time;
		double pending_requests;
		PardoSectionsInfo(int p, int s, double t, double r) : pc(p), section(s), time(t), pending_requests(r) {}
	};

	typedef std::vector<PardoSectionsInfo> PardoSectionsInfoVector_t;
	PardoSectionsInfoVector_t& get_pardo_section_times() { return pardo_section_times_; }

	/**
	 * After SIPMaP simulates all the workers, this method
	 * merges all the timers & pardo sections information.
	 * @param pardo_sections_info_vector
	 * @param sipmap_timers_vector
	 * @return
	 */
	static SIPMaPTimer merge_sipmap_timers(
			std::vector<PardoSectionsInfoVector_t>& pardo_sections_info_vector,
			std::vector<SIPMaPTimer>& sipmap_timers_vector);

private:

	int section_number_;									//! Section number being interpreted.
	std::vector<PardoSectionsInfo> pardo_section_times_;	//! [out] Set of times per pardo section.
	std::set<BlockId> cached_blocks_map_;					//! Set of cached blocks in the current pardo section. An overestimate

	const ProfileTimerStore& profile_timer_store_;	//! Backing store with profile info
	const RemoteArrayModel& remote_array_model_;	//! Encapsulates time to get/put blocks
	SIPMaPTimer& sipmap_timer_;						//! Records time per sialx line


	struct BlockRemoteOp{
		const double travel_time;			//! Time to get/put block
		const double time_started;			//! Time at which get/put was started
		BlockRemoteOp(double tt, double ts) : travel_time(tt), time_started(ts) {}
	};

	std::map<BlockId, BlockRemoteOp> pending_blocks_map_;	//! Set of blocks in transit because of a GET/REQUEST.
	std::list<BlockRemoteOp> pending_acks_list_;			//! List of pending acks from PUTs, PUT+, DELETEs, SET/RESTORE_PERSISTENT

	/**
	 * Captures the wall time in a pardo section (statements between 2 barriers).
	 * Initialized to 0 at a sip barrier, at the end of program and at the beginning.
	 */
	double pardo_section_time_;


	/**
	 * Helper method to construct a profile timer key
	 * to get data from the profile timer store
	 * @param opcode_name
	 * @param selector_list
	 * @return
	 */
	ProfileTimer::Key make_profile_timer_key(const std::string& opcode_name,
			const std::list<BlockSelector>& selector_list);

	/**
	 * Converts an opcode to a string and
	 * delegates to SIPMaPInterpreter#make_profile_timer_key(const std::string&, const std::list<BlockSelector>&)
	 * @param opcode
	 * @param selector_list
	 * @return
	 */
	ProfileTimer::Key make_profile_timer_key(opcode_t opcode,
			const std::list<BlockSelector>& selector_list);


	/**
	 * Utility method to record time into
	 * the SipmapTimer instance.
	 * @param time
	 */
	void record_total_time(double time){
		sipmap_timer_[current_pc()].record_total_time(time);
	}

	void record_block_wait_time(double time){
		sipmap_timer_[current_pc()].record_block_wait_time(time);
	}
	/**
	 * Utility method to calculate the block wait time.
	 * Checks whether there are any blocks in transit (from the provided list)
	 * and accordingly calculates the block wait time.
	 * @param bs_list
	 * @return
	 */
	double calculate_block_wait_time(std::list<BlockSelector> bs_list);
	double calculate_pending_request_time();
	void record_zero_block_args_computation_time(opcode_t op);
};

} /* namespace sip */

#endif /* SIPMAP_INTERPRETER_H_ */
