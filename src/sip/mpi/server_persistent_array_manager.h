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

namespace sip {

class SIPServer;

/**
 * Data structure used for persistent Scalars and arrays.
 *
 * In the SIAL program:
 *     set_persistent array1 "label1"
 * causes array1 with the string slot for the string literal to be saved
 * in the persistent_array_map_.  In the HAVE_MPI build, if array1 id distributed,
 * messages are sent to all servers to add the entry to the server's persistent_array_map_.
 * Sending these messages is the responsibility of the worker.
 *
 * After the sial program is finished, save_marked_arrays() is invoked at
 * both workers and servers.  The values of marked (i.e. those that are
 * in the persistent_array_map_) scalars are then copied into the scalar_value_map_
 * with the string corresponding to the string_slot as key.  Marked contiguous arrays,
 * and the block map for distributed arrays are MOVED to this class's contiguous_array_map_
 * or distribute_array_map_ respectively. The string itself is used as a key because string
 * slots are assigned by the sial compiler and are only valid in a single sial program.
 *
 * In a subsequent SIAL program, the restore_persisent_array command causes
 * the indicated object to be restored to the SIAL program and removed from the
 * persistent_array data structures.  Scalar values are copied;
 * For contiguous and distributed, pointers to the block or the block map are copied.
 * It is the responsibility of the worker in a parallel program  to check whether
 * the requested object is a scalar or contiguous. If so, restore_persistent_array
 * is called on the local persistent_array_manager.  If it is a distributed/served
 * array, workers send a message to servers to restore the array.
 *
 * A consequence of this design is that  any object can only be restored once.
 * If it is needed again in subsequent SIAL programs, set_persistent needs to be
 * invoked in the current SIAL program. These semantics were chosen to allow clear
 * ownership transfer of allocated memory without unnecessary copying or garbage.
 *
 * Predefined scalars and arrays cannot be made persistent.  Their value are preserved
 * between programs in the SetupReader object already.
 *
 * The SIAL compiler ensures that an object of set_persistent commands is not predefined and
 * is either a scalar or a static, distributed, or served array.
 */
class ServerPersistentArrayManager {

public:

	/**
	 * Type of map for storing persistent distributed and served arrays between SIAL programs.
	 * In parallel implementation., only used at servers
	 */
	typedef std::map<std::string, IdBlockMap<ServerBlock>::PerArrayMap*> LabelDistributedArrayMap;
	/**
	 * Type of map for storing persistent scalars between SIAL programs.  Only used at workers
	 */
	typedef std::map<std::string, double> LabelScalarValueMap;

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
	void save_marked_arrays(SIPServer* runner) ;

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
	 * @param runner
	 * @param array_id
	 * @param string_slot
	 */
	void restore_persistent(SIPServer* runner, int array_id, int string_slot);


	friend std::ostream& operator<< (std::ostream&, const ServerPersistentArrayManager&);

private:

	/** holder for arrays and scalars that have been marked as persistent */
	ArrayIdLabelMap persistent_array_map_;


    /**	/** Invoked by restore_persistent to implement restore_persistent command in
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
	void restore_persistent_distributed(SIPServer* runner, int array_id, int string_slot);

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
