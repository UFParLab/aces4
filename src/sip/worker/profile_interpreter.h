/*
 * profile_interpreter.h
 *
 *  Created on: Oct 1, 2014
 *      Author: njindal
 */

#ifndef PROFILE_INTERPRETER_H_
#define PROFILE_INTERPRETER_H_

#include "sialx_interpreter.h"
#include "profile_timer.h"

namespace sip {

class ProfileTimer;
class SipTables;
class SialPrinter;
class WorkerPersistentArrayManager;

class ProfileInterpreter: public SialxInterpreter {
public:
	ProfileInterpreter(const SipTables& sipTables, ProfileTimer& profile_timer,
			SialPrinter* printer,
			WorkerPersistentArrayManager* persistent_array_manager);
	virtual ~ProfileInterpreter();

	virtual void pre_interpret(int pc);


private:
	ProfileTimer& profile_timer_;
	ProfileTimer::Key last_seen_key_;
	int last_seen_pc_;
	ProfileTimer::Key make_profile_timer_key(opcode_t opcode, std::list<BlockSelector> selector_list);
};

} /* namespace sip */

#endif /* PROFILE_INTERPRETER_H_ */
