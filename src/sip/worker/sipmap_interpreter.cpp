/*
 * sipmap_interpreter.cpp
 *
 *  Created on: Oct 9, 2014
 *      Author: njindal
 */

#include "sipmap_interpreter.h"
#include "profile_timer_store.h"
#include "sipmap_timer.h"
#include "loop_manager.h"
#include "json/json.h"


namespace sip {

SIPMaPConfig::SIPMaPConfig(std::istream& input_file) : input_file_(input_file){
	Json::Value root;   // will contains the root value after parsing.
	Json::Reader reader;
	bool parsingSuccessful = reader.parse( input_file, root );
	if ( !parsingSuccessful ){
		std::stringstream err;
		err << "Parsing of input json file failed !" << reader.getFormattedErrorMessages();
		sip::fail (err.str());
	}
	parameters_.interconnect_alpha 	= root["InterconnectAlpha"].asDouble();
	parameters_.interconnect_beta   = root["InterconnectBeta"].asDouble();
	parameters_.daxpy_alpha			= root["DAXPYAlpha"].asDouble();
	parameters_.daxpy_beta			= root["DAXPYBeta"].asDouble();
}


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
		if (sip_tables_.is_distributed(it->array_id_) || sip_tables_.is_served(it->array_id_)) {
			BlockId block_id = data_manager_.block_id(*it);
			std::map<BlockId, BlockRemoteOp>::iterator pending = pending_blocks_map_.find(block_id);
			if (pending != pending_blocks_map_.end()) {
				BlockRemoteOp& block_remote_op = pending->second;
				double block_arrives_at = block_remote_op.time_started + block_remote_op.travel_time;
				if (block_arrives_at > max_block_arrives_at) {
					max_block_arrives_at = block_arrives_at;
				}
				pending_blocks_map_.erase(pending);	// Delete pending time info after processed.
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
	std::pair<double, long> timer_count_pair = profile_timer_store_.get_from_cache(key);

	double computation_time = timer_count_pair.first / (double)timer_count_pair.second;
	double block_wait_time = calculate_block_wait_time(bs_list);
	record_block_wait_time(block_wait_time);
	record_total_time(computation_time + block_wait_time);
	pardo_section_time_ += computation_time + block_wait_time;

	if (all_write && all_scalar_or_static){
		SIP_LOG(std::cout << "Executing Super Instruction "
				<< sip_tables_.special_instruction_manager().name(func_slot)
				<< " at line " << line_number() << std::endl);
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
	std::pair<double, long> timer_count_pair = profile_timer_store_.get_from_cache(key);

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
	std::pair<double, long> timer_count_pair = profile_timer_store_.get_from_cache(key);

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
	std::pair<double, long> timer_count_pair = profile_timer_store_.get_from_cache(key);

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
	std::pair<double, long> timer_count_pair = profile_timer_store_.get_from_cache(key);

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
	std::pair<double, long> timer_count_pair = profile_timer_store_.get_from_cache(key);

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
	std::pair<double, long> timer_count_pair = profile_timer_store_.get_from_cache(key);

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
	std::pair<double, long> timer_count_pair = profile_timer_store_.get_from_cache(key);

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
	std::pair<double, long> timer_count_pair = profile_timer_store_.get_from_cache(key);

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
	std::pair<double, long> timer_count_pair = profile_timer_store_.get_from_cache(key);

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
	std::pair<double, long> timer_count_pair = profile_timer_store_.get_from_cache(key);

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
double SIPMaPInterpreter::calculate_pending_request_time() {
	// Increment pardo section time after compensating for time spent waiting for acks
	double max_async_ack_arrives_at = 0.0;
	std::list<BlockRemoteOp>::iterator it = pending_acks_list_.begin();
	for (; it != pending_acks_list_.end();) {
		BlockRemoteOp& brop = *it;
		double async_ack_arrives_at = brop.time_started + brop.travel_time;
		if (async_ack_arrives_at > max_async_ack_arrives_at) {
			max_async_ack_arrives_at = async_ack_arrives_at;
		}
		pending_acks_list_.erase(it++);
	}
	double pending_requests = 0.0;
	if (pardo_section_time_ < max_async_ack_arrives_at) {
		pending_requests = max_async_ack_arrives_at - pardo_section_time_;
	}
	return pending_requests;
}
void SIPMaPInterpreter::handle_sip_barrier_op(int pc){

	// Increment pardo section time after compensating for time spent waiting for acks
	double pending_requests = calculate_pending_request_time();
	pardo_section_time_ += pending_requests;

	pardo_section_times_.push_back(PardoSectionsInfo(pc, section_number_, pardo_section_time_, pending_requests));
	section_number_++;
	// Aggregate time from multiple ranks - when combining SIPMaPTimer instances.
	pardo_section_time_ = 0.0;
}
void SIPMaPInterpreter::do_post_sial_program(){
	// -1 is for the line number at the end of the program.
	double pending_requests = calculate_pending_request_time();
	pardo_section_time_ += pending_requests;
	pardo_section_times_.push_back(PardoSectionsInfo(-1, section_number_, pardo_section_time_, pending_requests));
	section_number_++;
	pardo_section_time_ = 0.0;

	SialxInterpreter::do_post_sial_program();
}
void SIPMaPInterpreter::handle_get_op(int pc){
	/** A small blocking message is sent to fetch the block needed
	 * 	An async Recv is then posted for the needed block.
	 * 	Other bookkeeping activities also take place. */

	// Record computation time (time for bookkeeping and to send small message)
	std::list<BlockSelector> bs_list;
	bs_list.push_back(block_selector_stack_.top());
	ProfileTimer::Key key = make_profile_timer_key(get_op, bs_list);
	std::pair<double, long> timer_count_pair = profile_timer_store_.get_from_cache(key);
	double computation_time = timer_count_pair.first / (double)timer_count_pair.second;
	record_total_time(computation_time);

	// Get time to process at server and for block to travel
	BlockId id = get_block_id_from_selector_stack();
	double travel_time = remote_array_model_.time_to_get_block(id);
	pardo_section_time_ += computation_time;
	BlockRemoteOp brop (travel_time, pardo_section_time_);
	pending_blocks_map_.insert(std::make_pair(id, brop));
}
void SIPMaPInterpreter::handle_put_accumulate_op(int pc){
	/** A small blocking message is sent to inform the server of the incoming block.
	 * The block itself is sent asynchronously. An async recv is posted for the ack.
	 * Other bookkeeping  activities also take place.
	 * On the server, an accumulate operation happens on the block */

	// Record computation time (time for bookkeeping and to send small message & asynchronously big message)
	std::list<BlockSelector> bs_list;
	BlockSelector & first = block_selector_stack_.top();
	bs_list.push_back(first);
	block_selector_stack_.pop();
	bs_list.push_back(block_selector_stack_.top());
	block_selector_stack_.push(first);
	ProfileTimer::Key key = make_profile_timer_key(put_accumulate_op, bs_list);
	std::pair<double, long> timer_count_pair = profile_timer_store_.get_from_cache(key);
	double computation_time = timer_count_pair.first / (double)timer_count_pair.second;
	record_total_time(computation_time);

	// Get time to process at server and for ack to travel
	BlockId id = get_block_id_from_selector_stack();						// Remove 1st block selector from stack
	double time_to_get_ack_from_server =
			remote_array_model_.time_to_send_block_to_server(id)			// time to actually send block
					+ remote_array_model_.time_to_put_accumulate_block(id)	// time to accumulate block at server
					+ remote_array_model_.time_to_get_ack_from_server();	// time to get back ack from server
	BlockId id2 = get_block_id_from_selector_stack();						// Remove 2nd block selector from stack
	pardo_section_time_ += computation_time;
	pending_acks_list_.push_back(BlockRemoteOp(time_to_get_ack_from_server, pardo_section_time_));
}
void SIPMaPInterpreter::handle_put_replace_op(int pc) {

	/** A small blocking message is sent to inform the server of the incoming block.
	 * The block itself is sent asynchronously. An async recv is posted for the ack.
	 * Other bookkeeping  activities also take place. */

	// Record computation time (time for bookkeeping and to send small message & asynchronously big message)
	std::list<BlockSelector> bs_list;
	BlockSelector & first = block_selector_stack_.top();
	bs_list.push_back(first);
	block_selector_stack_.pop();
	bs_list.push_back(block_selector_stack_.top());
	block_selector_stack_.push(first);
	ProfileTimer::Key key = make_profile_timer_key(put_replace_op, bs_list);
	std::pair<double, long> timer_count_pair = profile_timer_store_.get_from_cache(key);
	double computation_time = timer_count_pair.first / (double)timer_count_pair.second;
	record_total_time(computation_time);

	/** Put replaces are blocking operations, receipt of acks are async */
	BlockId id = get_block_id_from_selector_stack();						// Remove 1st block selector from stack
	double time_to_get_ack_from_server =
			remote_array_model_.time_to_send_block_to_server(id)			// time to actually send block
					+ remote_array_model_.time_to_put_replace_block(id)		// time to replace block at server
					+ remote_array_model_.time_to_get_ack_from_server();	// time to get back ack from server
	BlockId id2 = get_block_id_from_selector_stack();						// Remove 2nd block selector from stack
	pardo_section_time_ += computation_time;
	pending_acks_list_.push_back(BlockRemoteOp(time_to_get_ack_from_server, pardo_section_time_));
}
void SIPMaPInterpreter::handle_create_op(int pc){
	/** A no op for now */
}
void SIPMaPInterpreter::handle_delete_op(int pc){
	/** A small blocking message is sent to the server, receipt of ack is async, some bookkeeping. */

	// Record computation time (time for bookkeeping and to send small message)
	std::list<BlockSelector> bs_list;
	ProfileTimer::Key key = make_profile_timer_key(delete_op, bs_list);
	std::pair<double, long> timer_count_pair = profile_timer_store_.get_from_cache(key);
	double computation_time = timer_count_pair.first / (double)timer_count_pair.second;
	record_total_time(computation_time);

	pardo_section_time_ += computation_time;
	double time_to_get_ack_from_server =
			remote_array_model_.time_to_delete_array(arg0(pc))
					+ remote_array_model_.time_to_get_ack_from_server();
	pending_acks_list_.push_back(BlockRemoteOp(time_to_get_ack_from_server, pardo_section_time_));
}
void SIPMaPInterpreter::handle_set_persistent_op(int pc) {
	/** A small blocking message is sent to the server, receipt of ack is async, some bookkeeping. */

	// Record computation time (time for bookkeeping and to send small message)
	std::list<BlockSelector> bs_list;
	ProfileTimer::Key key = make_profile_timer_key(set_persistent_op, bs_list);
	std::pair<double, long> timer_count_pair = profile_timer_store_.get_from_cache(key);
	double computation_time = timer_count_pair.first / (double)timer_count_pair.second;
	record_total_time(computation_time);

	pardo_section_time_ += computation_time;
	double time_to_get_ack_from_server =
				remote_array_model_.time_to_set_persistent(arg1(pc))
						+ remote_array_model_.time_to_get_ack_from_server();
	pending_acks_list_.push_back(BlockRemoteOp(time_to_get_ack_from_server, pardo_section_time_));
}
void SIPMaPInterpreter::handle_restore_persistent_op(int pc){
	/** A small blocking message is sent to the server, receipt of ack is async, some bookkeeping. */

	// Record computation time (time for bookkeeping and to send small message)
	std::list<BlockSelector> bs_list;
	ProfileTimer::Key key = make_profile_timer_key(restore_persistent_op, bs_list);
	std::pair<double, long> timer_count_pair = profile_timer_store_.get_from_cache(key);
	double computation_time = timer_count_pair.first / (double)timer_count_pair.second;
	record_total_time(computation_time);

	pardo_section_time_ += computation_time;
	double time_to_get_ack_from_server =
				remote_array_model_.time_to_set_persistent(arg1(pc))
						+ remote_array_model_.time_to_get_ack_from_server();
	pending_acks_list_.push_back(BlockRemoteOp(time_to_get_ack_from_server, pardo_section_time_));
}
void SIPMaPInterpreter::handle_pardo_op(int &pc){
	/** What a particular worker would do */
//	LoopManager* loop = new StaticTaskAllocParallelPardoLoop(arg1(pc),
//					index_selectors(pc), data_manager_, sip_tables_,
//					worker_rank_, num_workers_);
	int num_where_clauses = arg2(pc);
	int num_indices = arg1(pc);
	LoopManager* loop = new BalancedTaskAllocParallelPardoLoop(num_indices,
				index_selectors(pc), data_manager_, sip_tables_, num_where_clauses,
				this, worker_rank_, num_workers_, iteration_);
	loop_start(pc, loop);
}
#endif // HAVE_MPI

void SIPMaPInterpreter::record_zero_block_args_computation_time(opcode_t op) {
	std::list<BlockSelector> bs_list;
	ProfileTimer::Key key = make_profile_timer_key(op, bs_list);
	std::pair<double, long> timer_count_pair = profile_timer_store_.get_from_cache(key);
	double computation_time = timer_count_pair.first / (double) (timer_count_pair.second);
	record_total_time(computation_time);
	pardo_section_time_ += computation_time;
}

void SIPMaPInterpreter::handle_jump_if_zero_op(int &pc)		{ SialxInterpreter::handle_jump_if_zero_op(pc); record_zero_block_args_computation_time(jump_if_zero_op);}
void SIPMaPInterpreter::handle_stop_op(int &pc)				{ SialxInterpreter::handle_stop_op(pc); record_zero_block_args_computation_time(stop_op);	}
void SIPMaPInterpreter::handle_exit_op(int &pc)				{ SialxInterpreter::handle_exit_op(pc); record_zero_block_args_computation_time(exit_op);	}
void SIPMaPInterpreter::handle_push_block_selector_op(int pc){ SialxInterpreter::handle_push_block_selector_op(pc); record_zero_block_args_computation_time(push_block_selector_op);}
void SIPMaPInterpreter::handle_string_load_literal_op(int pc){ SialxInterpreter::handle_string_load_literal_op(pc); record_zero_block_args_computation_time(string_load_literal_op);}
void SIPMaPInterpreter::handle_int_load_literal_op(int pc)	{ SialxInterpreter::handle_int_load_literal_op(pc); record_zero_block_args_computation_time(int_load_literal_op);	}
void SIPMaPInterpreter::handle_int_load_value_op(int pc)	{ SialxInterpreter::handle_int_load_value_op(pc); record_zero_block_args_computation_time(int_load_value_op);	}
void SIPMaPInterpreter::handle_int_store_op(int pc)			{ SialxInterpreter::handle_int_store_op(pc); record_zero_block_args_computation_time(int_store_op);	}
void SIPMaPInterpreter::handle_int_add_op(int pc)			{ SialxInterpreter::handle_int_add_op(pc); record_zero_block_args_computation_time(int_add_op);	}
void SIPMaPInterpreter::handle_int_subtract_op(int pc)		{ SialxInterpreter::handle_int_subtract_op(pc); record_zero_block_args_computation_time(int_subtract_op);	}
void SIPMaPInterpreter::handle_int_multiply_op(int pc)		{ SialxInterpreter::handle_int_multiply_op(pc); record_zero_block_args_computation_time(int_multiply_op);	}
void SIPMaPInterpreter::handle_int_divide_op(int pc)		{ SialxInterpreter::handle_int_divide_op(pc); record_zero_block_args_computation_time(int_divide_op);	}
void SIPMaPInterpreter::handle_int_equal_op(int pc)			{ SialxInterpreter::handle_int_equal_op(pc); record_zero_block_args_computation_time(int_equal_op);	}
void SIPMaPInterpreter::handle_int_nequal_op(int pc)		{ SialxInterpreter::handle_int_nequal_op(pc); record_zero_block_args_computation_time(int_nequal_op);	}
void SIPMaPInterpreter::handle_int_le_op(int pc)			{ SialxInterpreter::handle_int_le_op(pc); record_zero_block_args_computation_time(int_le_op);	}
void SIPMaPInterpreter::handle_int_gt_op(int pc)			{ SialxInterpreter::handle_int_gt_op(pc); record_zero_block_args_computation_time(int_gt_op);	}
void SIPMaPInterpreter::handle_int_lt_op(int pc)			{ SialxInterpreter::handle_int_lt_op(pc); record_zero_block_args_computation_time(int_lt_op);	}
void SIPMaPInterpreter::handle_int_neg_op(int pc)			{ SialxInterpreter::handle_int_neg_op(pc); record_zero_block_args_computation_time(int_neg_op);	}
void SIPMaPInterpreter::handle_cast_to_int_op(int pc)		{ SialxInterpreter::handle_cast_to_int_op(pc); record_zero_block_args_computation_time(cast_to_int_op);	}
void SIPMaPInterpreter::handle_index_load_value_op(int pc)	{ SialxInterpreter::handle_index_load_value_op(pc); record_zero_block_args_computation_time(index_load_value_op);	}
void SIPMaPInterpreter::handle_scalar_load_value_op(int pc)	{ SialxInterpreter::handle_scalar_load_value_op(pc); record_zero_block_args_computation_time(scalar_load_value_op);	}
void SIPMaPInterpreter::handle_scalar_store_op(int pc)		{ SialxInterpreter::handle_scalar_store_op(pc); record_zero_block_args_computation_time(scalar_store_op);	}
void SIPMaPInterpreter::handle_scalar_add_op(int pc)		{ SialxInterpreter::handle_scalar_add_op(pc); record_zero_block_args_computation_time(scalar_add_op);	}
void SIPMaPInterpreter::handle_scalar_subtract_op(int pc)	{ SialxInterpreter::handle_scalar_subtract_op(pc); record_zero_block_args_computation_time(scalar_subtract_op);	}
void SIPMaPInterpreter::handle_scalar_multiply_op(int pc)	{ SialxInterpreter::handle_scalar_multiply_op(pc); record_zero_block_args_computation_time(scalar_multiply_op);	}
void SIPMaPInterpreter::handle_scalar_divide_op(int pc)		{ SialxInterpreter::handle_scalar_divide_op(pc); record_zero_block_args_computation_time(scalar_divide_op);	}
void SIPMaPInterpreter::handle_scalar_exp_op(int pc)		{ SialxInterpreter::handle_scalar_exp_op(pc); record_zero_block_args_computation_time(scalar_exp_op);	}
void SIPMaPInterpreter::handle_scalar_eq_op(int pc)			{ SialxInterpreter::handle_scalar_eq_op(pc); record_zero_block_args_computation_time(scalar_eq_op);	}
void SIPMaPInterpreter::handle_scalar_ne_op(int pc)			{ SialxInterpreter::handle_scalar_ne_op(pc); record_zero_block_args_computation_time(scalar_ne_op);	}
void SIPMaPInterpreter::handle_scalar_ge_op(int pc)			{ SialxInterpreter::handle_scalar_ge_op(pc); record_zero_block_args_computation_time(scalar_ge_op);	}
void SIPMaPInterpreter::handle_scalar_le_op(int pc)			{ SialxInterpreter::handle_scalar_le_op(pc); record_zero_block_args_computation_time(scalar_le_op);	}
void SIPMaPInterpreter::handle_scalar_gt_op(int pc)			{ SialxInterpreter::handle_scalar_gt_op(pc); record_zero_block_args_computation_time(scalar_gt_op);	}
void SIPMaPInterpreter::handle_scalar_lt_op(int pc)			{ SialxInterpreter::handle_scalar_lt_op(pc); record_zero_block_args_computation_time(scalar_lt_op);	}
void SIPMaPInterpreter::handle_scalar_neg_op(int pc)		{ SialxInterpreter::handle_scalar_neg_op(pc); record_zero_block_args_computation_time(scalar_neg_op);	}
void SIPMaPInterpreter::handle_scalar_sqrt_op(int pc)		{ SialxInterpreter::handle_scalar_sqrt_op(pc); record_zero_block_args_computation_time(scalar_sqrt_op);	}
void SIPMaPInterpreter::handle_idup_op(int pc)				{ SialxInterpreter::handle_idup_op(pc); record_zero_block_args_computation_time(idup_op);	}
void SIPMaPInterpreter::handle_iswap_op(int pc)				{ SialxInterpreter::handle_iswap_op(pc); record_zero_block_args_computation_time(iswap_op);	}
void SIPMaPInterpreter::handle_sswap_op(int pc)				{ SialxInterpreter::handle_sswap_op(pc); record_zero_block_args_computation_time(sswap_op);	}



SIPMaPTimer SIPMaPInterpreter::merge_sipmap_timers(
			std::vector<SIPMaPInterpreter::PardoSectionsInfoVector_t>& pardo_sections_info_vector,
			std::vector<SIPMaPTimer>& sipmap_timers_vector){


	// Initial checks
	std::size_t workers = pardo_sections_info_vector.size();
	CHECK(workers == sipmap_timers_vector.size(), "Unequal number of pardo sections & sipmap timers when merging");

	// Check to see if number of sections recorded in each pardo_sections_info is the same.
	int sections = pardo_sections_info_vector.at(0).size();
	for (int i=1; i<workers; ++i)
		check(sections == pardo_sections_info_vector.at(i).size(), "Different number of pardo sections on different workers");


	// Calculate Barrier times & put into into sipmap_timers.
	for (int j=0; j<sections; ++j){

		// Get maximum time for a given section_number across all workers
		double max_time = pardo_sections_info_vector.at(0).at(j).time;
		//std::cout << "max time at " << j << " is " << max_time << std::endl;
		int pc = pardo_sections_info_vector.at(0).at(j).pc;
		int section_number = pardo_sections_info_vector.at(0).at(j).section;
		for (int i=1; i<workers; ++i){
			const PardoSectionsInfo& ps_info = pardo_sections_info_vector.at(i).at(j);
			CHECK(pc == ps_info.pc, "Program counters don't match across workers for a given pardo section_number info instance");
			CHECK(section_number == ps_info.section, "Section numbers don't match across workers for a given pardo section_number info instance");
			if (max_time < ps_info.time)
				max_time = ps_info.time;
		}

		// Calculate all barrier times according to the max time.
		for (int i=0; i<workers; ++i){
			CHECK(max_time - pardo_sections_info_vector.at(i).at(j).time >= 0, "The barrier time can only be positive ! BUG");
			double barrier_time  = max_time - pardo_sections_info_vector.at(i).at(j).time;
			double pending_request_time = pardo_sections_info_vector.at(i).at(j).pending_requests;
			if (pc >= 0){
				sipmap_timers_vector.at(i).timer(pc).record_total_time(barrier_time + pending_request_time);
				sipmap_timers_vector.at(i).timer(pc).record_block_wait_time(pending_request_time);
			}
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
