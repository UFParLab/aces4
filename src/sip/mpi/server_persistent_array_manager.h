/*
 * SERVER_PERSISTENT_array_manager.h
 *
 *  Created on: Apr 21, 2014
 *      Author: njindal
 */

#ifndef SERVER_PERSISTENT_ARRAY_MANAGER_H_
#define SERVER_PERSISTENT_ARRAY_MANAGER_H_

#include "server_block.h"
#include "id_block_map.h"
#include "timer.h"

namespace sip {

class SIPServer;

/**
 * Data structure used to distributed arrays between SIAL programs.
 *
 */
class ServerPersistentArrayManager {

public:

	/**
	 * Type of map for storing persistent distributed and served arrays between SIAL programs.
	 */
	typedef std::map<std::string, IdBlockMap<ServerBlock>::PerArrayMap*> LabelDistributedArrayMap;

	/**
	 * Type of map for storing the array id and slot of label for scalars and arrays that have been marked persistent.
	 */
	typedef std::map<int, int> ArrayIdLabelMap;	// Map of arrays marked for persistence

	ServerPersistentArrayManager() ;
	~ServerPersistentArrayManager() ;

	/** Called to implement SIAL set_persistent command
	 *
	 * @param array_id
	 * @param string_slot
	 */
	void set_persistent(SIPServer* runner, int array_id, int string_slot);

	/**
	 * Called during post processing.
	 * Values of marked scalars are copied into the scalar_value_map_;
	 * marked contiguous and distributed/served arrays are MOVED to
	 * the contiguous_array_map_ and distributed_array_map_ respectively.
	 *
	 * Note that in a  parallel implementation, distributed arrays
	 *  should only be marked at servers. Scalars and contiguous arrays are
	 *  only at workers. We won't bother trying to avoid a few unnecessary tests.
	 */
	void save_marked_arrays(SIPServer* runner, MPITimerList* save_persistent_timers);

	/**
	 * Invoked by worker or server to implement restore_persistent command
	 * in SIAL. The type of the object to restore is checked.
	 * It is the responsibility of the worker to decide whether to send
	 * a message to the server, or handle the request locally. The
	 * server will call this routine to handle a restore_persistent message.
	 *
	 * The runner parameter provides access to the sip_tables to
	 * get the label and check array types.
	 *
	 * This does not do timing since this corresponds to a sial op in the server loop
	 *
	 * @param runner
	 * @param array_id
	 * @param string_slot
	 */
	void restore_persistent(SIPServer* runner, int array_id, int string_slot, int pc);


	friend std::ostream& operator<< (std::ostream&, const ServerPersistentArrayManager&);


private:

	/** holder for arrays and scalars that have been marked as persistent */
	ArrayIdLabelMap persistent_array_map_;


    /**	Invoked by restore_persistent to implement restore_persistent command in
	 * SIAl when the argument is a distributed/served array.  The block map associated
	 * with the string literal is MOVED into the distributed array table and the
	 * entry removed (which does not delete the map) from the persistent_array_manager.
	 * The set_per_array method updates the array id in each block's id.
	 *
	 * In a parallel instance of aces4, this will be called only on the server, in
	 * a single node instance, it will be called by the single process.
     *
     * @param runner
     * @param array_id
     * @param string_slot
     */
	void restore_persistent_distributed(SIPServer* runner, int array_id, int string_slot, int pc);

	/** inserts label, map pair into map of saved distributed arrays.
	 * Warns if label has already been used.
	 * Fatal error if map is null
	 */
	void save_distributed(SIPServer* runner, const int array_id,
			const std::string& label,
			IdBlockMap<ServerBlock>::PerArrayMap* map);


	DISALLOW_COPY_AND_ASSIGN(ServerPersistentArrayManager);

};

} /* namespace sip */

#endif /* SERVER_PERSISTENT_ARRAY_MANAGER_H_ */
