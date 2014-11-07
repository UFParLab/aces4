/*
 * sipmap_interpreter.cpp
 *
 *  Created on: Oct 9, 2014
 *      Author: njindal
 */

#include "sipmap_interpreter.h"
#include "profile_timer_store.h"
#include "sipmap_timer.h"
#include "remote_array_model.h"
#include "loop_manager.h"

namespace sip {

SIPMaPInterpreter::SIPMaPInterpreter(int worker_rank, int num_workers,
		const SipTables& sipTables,
		const RemoteArrayModel& remote_array_model,
		const ProfileTimerStore& profile_timer_store, SIPMaPTimer& sipmap_timer) :
		SialxInterpreter(sipTables, NULL, NULL, NULL),
		worker_rank_(worker_rank),
		num_workers_(num_workers),
		profile_timer_store_(profile_timer_store),
		remote_array_model_(remote_array_model),
		sipmap_timer_(sipmap_timer),
		pardo_section_time_(0.0),
		section_number_(0){
}

SIPMaPInterpreter::~SIPMaPInterpreter() {}


inline ProfileTimer::Key SIPMaPInterpreter::make_profile_timer_key(const std::string& opcode_name, const std::list<BlockSelector>& selector_list){
	std::vector<ProfileTimer::BlockInfo> block_infos;
	block_infos.reserve(selector_list.size());

	std::list<BlockSelector>::const_iterator it = selector_list.begin();
	for (; it != selector_list.end(); ++it){
		const BlockSelector &bsel = *it;
		BlockShape bs;
		int rank = bsel.rank_;
		if (sip_tables_.array_rank(bsel.array_id_) == 0) { //this "array" was declared to be a scalar.  Nothing to remove from selector stack.)
			continue;
		}
		else if (sip_tables_.is_contiguous(bsel.array_id_) && bsel.rank_ == 0) { //this is a static array provided without a selector, block is entire array
			Block::BlockPtr block = data_manager_.contiguous_array_manager_.get_array(bsel.array_id_);
			bs = block->shape();
			// Modify rank so that the correct segments & indices are stored
			// in the profile timer.
			rank = sip_tables_.array_rank(bsel.array_id_);
		} else {
			BlockId bid = get_block_id_from_selector(bsel);
			bs = sip_tables_.shape(bid);
		}
		ProfileTimer::BlockInfo bi (rank, bsel.index_ids_, bs.segment_sizes_);
		block_infos.push_back(bi);
	}
	ProfileTimer::Key key(opcode_name, block_infos);
	return key;
}

inline ProfileTimer::Key SIPMaPInterpreter::make_profile_timer_key(opcode_t opcode, const std::list<BlockSelector>& selector_list){
	return make_profile_timer_key(opcodeToName(opcode), selector_list);
}


double SIPMaPInterpreter::calculate_block_wait_time(
		std::list<BlockSelector> bs_list) {
	double block_wait_time = 0.0;
	double max_block_arrives_at = 0.0;
	std::list<BlockSelector>::iterator it = bs_list.begin();
	for (; it != bs_list.end(); ++it) {
		if (sip_tables_.is_distributed(it->array_id_)
				|| sip_tables_.is_served(it->array_id_)) {
			BlockId block_id = data_manager_.block_id(*it);
			std::map<BlockId, BlockRemoteOp>::iterator pending =
					pending_blocks_map_.find(block_id);
			if (pending != pending_blocks_map_.end()) {
				BlockRemoteOp& block_remote_op = pending->second;
				double block_arrives_at = block_remote_op.time_started
						+ block_remote_op.travel_time;
				if (block_arrives_at > max_block_arrives_at)
					max_block_arrives_at = block_arrives_at;
			}
		}
	}
	if (pardo_section_time_ < max_block_arrives_at)
		block_wait_time = max_block_arrives_at - pardo_section_time_;

	return block_wait_time;
}

void SIPMaPInterpreter::handle_execute_op(int pc) {
	int num_args = arg1(pc) ;
	int func_slot = arg0(pc);

	// Do the super instruction if the number of args is > 1
	// and if the parameters are only written to (all 'w')
	// and if all the arguments are either static or scalar
	bool all_write = true;
	if (num_args >= 1){
		const std::string signature(
				sip_tables_.special_instruction_manager().get_signature(func_slot));
		for (int i=0; i<signature.size(); i++){
			if (signature[i] != 'w'){
				all_write = false;
				break;
			}
		}
	} else {
		all_write = false;
	}
	bool all_scalar_or_static = true;

	std::string opcode_name = sip_tables_.special_instruction_manager().name(func_slot);
	std::list<BlockSelector> bs_list;
	for (int i=0; i<num_args; i++){
		BlockSelector& bs = block_selector_stack_.top();
		bs_list.push_back(bs);
		if (!sip_tables_.is_contiguous(bs.array_id_) && !sip_tables_.is_scalar(bs.array_id_))
			all_scalar_or_static = false;

		block_selector_stack_.pop();
	}
	ProfileTimer::Key key = make_profile_timer_key(opcode_name, bs_list);
	std::pair<long, long> timer_count_pair = profile_timer_store_.get_from_store(key);

	double computation_time = timer_count_pair.first / (double)timer_count_pair.second;
	double block_wait_time = calculate_block_wait_time(bs_list);
	record_block_wait_time(block_wait_time);
	record_total_time(computation_time + block_wait_time);
	pardo_section_time_ += computation_time + block_wait_time;

	if (all_write && all_scalar_or_static){
		std::cout << "Executing Super Instruction "
				<< sip_tables_.special_instruction_manager().name(func_slot)
				<< " at line " << line_number() << std::endl;
		// Push back blocks on stack and execute the super instruction.
		while (!bs_list.empty()){
			BlockSelector bs = bs_list.back();
			block_selector_stack_.push(bs);
			bs_list.pop_back();
		}
		SialxInterpreter::handle_execute_op(pc);
	}

}

void SIPMaPInterpreter::handle_block_copy_op(int pc) {
	std::list<BlockSelector> bs_list;
	bs_list.push_back(block_selector_stack_.top());
	block_selector_stack_.pop();
	bs_list.push_back(BlockSelector(arg0(pc), arg1(pc), index_selectors(pc)));
	ProfileTimer::Key key = make_profile_timer_key(block_copy_op, bs_list);
	std::pair<long, long> timer_count_pair = profile_timer_store_.get_from_store(key);

	double computation_time = timer_count_pair.first / (double)timer_count_pair.second;
	double block_wait_time = calculate_block_wait_time(bs_list);
	record_block_wait_time(block_wait_time);
	record_total_time(computation_time + block_wait_time);
	pardo_section_time_ += computation_time + block_wait_time;

}
void SIPMaPInterpreter::handle_block_permute_op(int pc) {
	std::list<BlockSelector> bs_list;
	bs_list.push_back(block_selector_stack_.top());
	block_selector_stack_.pop();
	bs_list.push_back(block_selector_stack_.top());
	block_selector_stack_.pop();
	ProfileTimer::Key key = make_profile_timer_key(block_permute_op, bs_list);
	std::pair<long, long> timer_count_pair = profile_timer_store_.get_from_store(key);

	double computation_time = timer_count_pair.first / (double)timer_count_pair.second;
	double block_wait_time = calculate_block_wait_time(bs_list);
	record_block_wait_time(block_wait_time);
	record_total_time(computation_time + block_wait_time);
	pardo_section_time_ += computation_time + block_wait_time;
}
void SIPMaPInterpreter::handle_block_fill_op(int pc) {
	std::list<BlockSelector> bs_list;
	expression_stack_.pop();
	bs_list.push_back(BlockSelector(arg0(pc), arg1(pc), index_selectors(pc)));
	ProfileTimer::Key key = make_profile_timer_key(block_fill_op, bs_list);
	std::pair<long, long> timer_count_pair = profile_timer_store_.get_from_store(key);

	double computation_time = timer_count_pair.first / (double)timer_count_pair.second;
	double block_wait_time = calculate_block_wait_time(bs_list);
	record_block_wait_time(block_wait_time);
	record_total_time(computation_time + block_wait_time);
	pardo_section_time_ += computation_time + block_wait_time;
}
void SIPMaPInterpreter::handle_block_scale_op(int pc) {
	std::list<BlockSelector> bs_list;
	expression_stack_.pop();
	bs_list.push_back(BlockSelector(arg0(pc), arg1(pc), index_selectors(pc)));
	ProfileTimer::Key key = make_profile_timer_key(block_scale_op, bs_list);
	std::pair<long, long> timer_count_pair = profile_timer_store_.get_from_store(key);

	double computation_time = timer_count_pair.first / (double)timer_count_pair.second;
	double block_wait_time = calculate_block_wait_time(bs_list);
	record_block_wait_time(block_wait_time);
	record_total_time(computation_time + block_wait_time);
	pardo_section_time_ += computation_time + block_wait_time;
}
void SIPMaPInterpreter::handle_block_scale_assign_op(int pc) {
	std::list<BlockSelector> bs_list;
	bs_list.push_back(block_selector_stack_.top());
	block_selector_stack_.pop();
	bs_list.push_back(BlockSelector(arg0(pc), arg1(pc), index_selectors(pc)));
	expression_stack_.pop();
	ProfileTimer::Key key = make_profile_timer_key(block_scale_assign_op, bs_list);
	std::pair<long, long> timer_count_pair = profile_timer_store_.get_from_store(key);

	double computation_time = timer_count_pair.first / (double)timer_count_pair.second;
	double block_wait_time = calculate_block_wait_time(bs_list);
	record_block_wait_time(block_wait_time);
	record_total_time(computation_time + block_wait_time);
	pardo_section_time_ += computation_time + block_wait_time;
}
void SIPMaPInterpreter::handle_block_accumulate_scalar_op(int pc) {
	std::list<BlockSelector> bs_list;
	expression_stack_.pop();
	bs_list.push_back(BlockSelector(arg0(pc), arg1(pc), index_selectors(pc)));
	ProfileTimer::Key key = make_profile_timer_key(block_accumulate_scalar_op, bs_list);
	std::pair<long, long> timer_count_pair = profile_timer_store_.get_from_store(key);

	double computation_time = timer_count_pair.first / (double)timer_count_pair.second;
	double block_wait_time = calculate_block_wait_time(bs_list);
	record_block_wait_time(block_wait_time);
	record_total_time(computation_time + block_wait_time);
	pardo_section_time_ += computation_time + block_wait_time;
}
void SIPMaPInterpreter::handle_block_add_op(int pc) {
	std::list<BlockSelector> bs_list;
	bs_list.push_back(BlockSelector(arg0(pc), arg1(pc), index_selectors(pc)));
	bs_list.push_back(block_selector_stack_.top());
	block_selector_stack_.pop();
	bs_list.push_back(block_selector_stack_.top());
	block_selector_stack_.pop();
	ProfileTimer::Key key = make_profile_timer_key(block_add_op, bs_list);
	std::pair<long, long> timer_count_pair = profile_timer_store_.get_from_store(key);

	double computation_time = timer_count_pair.first / (double)timer_count_pair.second;
	double block_wait_time = calculate_block_wait_time(bs_list);
	record_block_wait_time(block_wait_time);
	record_total_time(computation_time + block_wait_time);
	pardo_section_time_ += computation_time + block_wait_time;
}
void SIPMaPInterpreter::handle_block_subtract_op(int pc) {
	std::list<BlockSelector> bs_list;
	bs_list.push_back(BlockSelector(arg0(pc), arg1(pc), index_selectors(pc)));
	bs_list.push_back(block_selector_stack_.top());
	block_selector_stack_.pop();
	bs_list.push_back(block_selector_stack_.top());
	block_selector_stack_.pop();
	ProfileTimer::Key key = make_profile_timer_key(block_subtract_op, bs_list);
	std::pair<long, long> timer_count_pair = profile_timer_store_.get_from_store(key);

	double computation_time = timer_count_pair.first / (double)timer_count_pair.second;
	double block_wait_time = calculate_block_wait_time(bs_list);
	record_block_wait_time(block_wait_time);
	record_total_time(computation_time + block_wait_time);
	pardo_section_time_ += computation_time + block_wait_time;
}
void SIPMaPInterpreter::handle_block_contract_op(int pc) {
	std::list<BlockSelector> bs_list;
	bs_list.push_back(BlockSelector(arg0(pc), arg1(pc), index_selectors(pc)));
	bs_list.push_back(block_selector_stack_.top());
	block_selector_stack_.pop();
	bs_list.push_back(block_selector_stack_.top());
	block_selector_stack_.pop();
	ProfileTimer::Key key = make_profile_timer_key(block_contract_op, bs_list);
	std::pair<long, long> timer_count_pair = profile_timer_store_.get_from_store(key);

	double computation_time = timer_count_pair.first / (double)timer_count_pair.second;
	double block_wait_time = calculate_block_wait_time(bs_list);
	record_block_wait_time(block_wait_time);
	record_total_time(computation_time + block_wait_time);
	pardo_section_time_ += computation_time + block_wait_time;

}
void SIPMaPInterpreter::handle_block_contract_to_scalar_op(int pc) {
	std::list<BlockSelector> bs_list;
	bs_list.push_back(block_selector_stack_.top());
	block_selector_stack_.pop();
	bs_list.push_back(block_selector_stack_.top());
	block_selector_stack_.pop();
	expression_stack_.push(0.0);
	ProfileTimer::Key key = make_profile_timer_key(block_contract_to_scalar_op, bs_list);
	std::pair<long, long> timer_count_pair = profile_timer_store_.get_from_store(key);

	double computation_time = timer_count_pair.first / (double)timer_count_pair.second;
	double block_wait_time = calculate_block_wait_time(bs_list);
	record_block_wait_time(block_wait_time);
	record_total_time(computation_time + block_wait_time);
	pardo_section_time_ += computation_time + block_wait_time;

}

void SIPMaPInterpreter::handle_block_load_scalar_op(int pc){
	BlockSelector &bs = block_selector_stack_.top();
	if (sip_tables_.is_contiguous(bs.array_id_)){
		SialxInterpreter::handle_block_load_scalar_op(pc);
	} else {
		block_selector_stack_.pop();
		expression_stack_.push(0.0);
	}
}


#ifdef HAVE_MPI
void SIPMaPInterpreter::handle_sip_barrier_op(int pc){
	pardo_section_times_.push_back(PardoSectionsInfo(line_number(), section_number_, pardo_section_time_));
	section_number_++;
	// Aggregate time from multiple ranks - when combining SIPMaPTimer instances.
	pardo_section_time_ = 0.0;
}

void SIPMaPInterpreter::do_post_sial_program(){
	// -1 is for the line number at the end of the program.
	pardo_section_times_.push_back(PardoSectionsInfo(-1, section_number_, pardo_section_time_));
	section_number_++;
	pardo_section_time_ = 0.0;

	SialxInterpreter::do_post_sial_program();
}


void SIPMaPInterpreter::handle_get_op(int pc){
	/** A small blocking message is sent to fetch the block needed
	 * An async Recv is then posted for the needed block.
	 */
	BlockId id = get_block_id_from_selector_stack();
	BlockRemoteOp brop (pardo_section_time_, remote_array_model_.time_to_get_block(id));
	pending_blocks_map_.insert(std::make_pair(id, brop));
}

void SIPMaPInterpreter::handle_put_accumulate_op(int pc){
	/** Put accumulates are blocking operations */
	BlockId id = get_block_id_from_selector_stack();
	pardo_section_time_ += remote_array_model_.time_to_put_accumulate_block(id);
}

void SIPMaPInterpreter::handle_put_replace_op(int pc) {
	/** Put replaces are blocking operations */
	BlockId id = get_block_id_from_selector_stack();
	pardo_section_time_ += remote_array_model_.time_to_put_replace_block(id);
}

void SIPMaPInterpreter::handle_create_op(int pc){
	/** A no op for now */
	pardo_section_time_ += remote_array_model_.time_to_create_array(arg0(pc));
}

void SIPMaPInterpreter::handle_delete_op(int pc){
	/** A small blocking message is sent to the server */
	pardo_section_time_ += remote_array_model_.time_to_delete_array(arg0(pc));
}

void SIPMaPInterpreter::handle_pardo_op(int &pc){
	/** What a particular worker would do */
	LoopManager* loop = new StaticTaskAllocParallelPardoLoop(arg1(pc),
					index_selectors(pc), data_manager_, sip_tables_,
					worker_rank_, num_workers_);
	loop_start(pc, loop);
}
#endif // HAVE_MPI




SIPMaPTimer SIPMaPInterpreter::merge_sipmap_timers(
			std::vector<SIPMaPInterpreter::PardoSectionsInfoVector_t>& pardo_sections_info_vector,
			std::vector<SIPMaPTimer>& sipmap_timers_vector){


	// Initial checks
	std::size_t workers = pardo_sections_info_vector.size();
	check(workers == sipmap_timers_vector.size(), "Unequal number of pardo sections & sipmap timers when merging");

	// Check to see if number of sections recorded in each pardo_sections_info is the same.
	int sections = pardo_sections_info_vector.at(0).size();
	for (int i=1; i<workers; ++i)
		check(sections == pardo_sections_info_vector.at(i).size(), "Different number of pardo sections on different workers");


	// Calculate Barrier times & put into into sipmap_timers.
	for (int j=0; j<sections; ++j){

		// Get maximum time for a given section_number across all workers
		double max_time = pardo_sections_info_vector.at(0).at(j).time;
		int line = pardo_sections_info_vector.at(0).at(j).line;
		int section_number = pardo_sections_info_vector.at(0).at(j).section;
		for (int i=1; i<workers; ++i){
			const PardoSectionsInfo& ps_info = pardo_sections_info_vector.at(i).at(j);
			check(line == ps_info.line, "Line numbers don't match across workers for a given pardo section_number info instance");
			check(section_number == ps_info.section, "Section numbers don't match across workers for a given pardo section_number info instance");
			if (max_time < ps_info.time)
				max_time = ps_info.time;
		}

		// Calculate all barrier times according to the max time.
		for (int i=0; i<workers; ++i){
			check(max_time - pardo_sections_info_vector.at(i).at(j).time >= 0, "The barrier time can only be positive ! BUG");
			double barrier_time  = max_time - pardo_sections_info_vector.at(i).at(j).time;
			if (line > 0)
				sipmap_timers_vector.at(i).record_time(line, barrier_time, SialxTimer::TOTALTIME);
		}
	}

	// Merge all timers.
	SIPMaPTimer output_timer(sipmap_timers_vector.at(0));
	for (int i=1; i<workers; ++i){
		output_timer.merge_other_timer(sipmap_timers_vector.at(i));
	}

	return output_timer;
}


} /* namespace sip */
