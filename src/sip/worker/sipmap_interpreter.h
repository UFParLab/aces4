/*
 * sipmap_interpreter.h
 *
 *  Created on: Oct 9, 2014
 *      Author: njindal
 */

#ifndef SIPMAP_INTERPRETER_H_
#define SIPMAP_INTERPRETER_H_

#include "sialx_interpreter.h"
#include "profile_timer.h"
#include "profile_timer_store.h"
#include "sipmap_timer.h"

#include <utility>
#include <map>
#include <vector>

namespace sip {
class ProfileTimerStore;
class SipTables;
class RemoteArrayModel;

/**
 * SIP Modeling and Prediction (SIPMaP) interpreter.
 * Models the Sialx Program based on information
 * made available from the profile interpreter run.
 * Profile information is made available through an
 * instance of ProfileTimerStore which encapsulates
 * a sqlite3 database.
 * Time per line is saved into the SIPMaPTimer instance.
 */
class SIPMaPInterpreter: public SialxInterpreter {
public:
	SIPMaPInterpreter(int worker_rank, int num_workers,
			const SipTables& sipTables,
			const RemoteArrayModel& remote_array_model,
			const ProfileTimerStore& profile_timer_store,
			SIPMaPTimer& sipmap_timer);
	virtual ~SIPMaPInterpreter();



//	virtual void handle_jump_if_zero_op(int &pc);	// TODO - To visit if & else parts of the code.


	virtual void handle_broadcast_static_op(int pc) { fail("handle_broadcast_static not modeled!", line_number()); }
	virtual void handle_allocate_op(int pc) { /* No op */ }
	virtual void handle_deallocate_op(int pc) {}
	virtual void handle_allocate_contiguous_op(int pc) { fail ("handle_allocate_contiguous_op not modeled!", line_number()); }
	virtual void handle_deallocate_contiguous_op(int pc) { fail ("handle_deallocate_contiguous_op not modeled!", line_number()); }

	virtual void handle_collective_sum_op(int pc) {  expression_stack_.pop();}
	virtual void handle_assert_same_op(int pc) {}
	virtual void handle_print_string_op(int pc) { control_stack_.pop();}
	virtual void handle_print_scalar_op(int pc) {  expression_stack_.pop();}
	virtual void handle_print_int_op(int pc) { control_stack_.pop();}
	virtual void handle_print_index_op(int pc) { control_stack_.pop();}
	virtual void handle_print_block_op(int pc) { block_selector_stack_.pop(); }
	virtual void handle_println_op(int pc) {}
	virtual void handle_gpu_on_op(int pc) {}
	virtual void handle_gpu_off_op(int pc) {}
	virtual void handle_set_persistent_op(int pc) {}
	virtual void handle_restore_persistent_op(int pc){}


	// Counting time for these opcodes
	// is relevant
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

#endif // HAVE_MPI


	struct PardoSectionsInfo{
		int line;
		int section;
		double time;
		PardoSectionsInfo(int l, int s, int t) : line(l), section(s), time(t) {}
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

	const int worker_rank_;							//! Worker rank for which the interpreter is being run
	const int num_workers_;							//! Total number of workers
	const ProfileTimerStore& profile_timer_store_;	//! Backing store with profile info
	const RemoteArrayModel& remote_array_model_;	//! Encapsulates time to get/put blocks
	SIPMaPTimer& sipmap_timer_;						//! Records time per sialx line


	struct BlockRemoteOp{
		const double travel_time;			//! Time to get/put block
		const double time_started;			//! Time at which get/put was started
		BlockRemoteOp(double tt, double ts) : travel_time(tt), time_started(ts) {}
	};

	std::map<BlockId, BlockRemoteOp> pending_blocks_map_;	//! Set of blocks in transit because of a GET/REQUEST.

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
		sipmap_timer_.record_time(line_number(), time, SialxTimer::TOTALTIME);
	}

	void record_block_wait_time(double time){
		sipmap_timer_.record_time(line_number(), time, SialxTimer::BLOCKWAITTIME);
	}
	/**
	 * Utility method to calculate the block wait time.
	 * Checks whether there are any blocks in transit (from the provided list)
	 * and accordingly calculates the block wait time.
	 * @param bs_list
	 * @return
	 */
	double calculate_block_wait_time(std::list<BlockSelector> bs_list);
};

} /* namespace sip */

#endif /* SIPMAP_INTERPRETER_H_ */
