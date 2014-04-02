/*
 * persistent_array_manager.h
 *
 *  Created on: Jan 8, 2014
 *      Author: njindal
 */

#ifndef PERSISTENT_ARRAY_MANAGER_H_
#define PERSISTENT_ARRAY_MANAGER_H_


#ifdef HAVE_MPI
#include "mpi.h"
#include "sip_mpi_attr.h"
#endif //HAVE_MPI

#include <vector>
//#include <set>

#include "blocks.h"
#include "block_manager.h"
#include "id_block_map.h"
#include "sip_tables.h"
#include "contiguous_array_manager.h"

namespace sip {

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
template<typename BLOCK_TYPE, typename RUNNER_TYPE>
class PersistentArrayManager {
	//Legal combinations of type parameters are <Block><Interpreter> and <ServerBlock><Server>

public:

	/**
	 * Type of map for storing persistent contiguous arrays between SIAL programs.  Only used at workers
	 */
	typedef std::map<std::string, Block::BlockPtr> LabelContiguousArrayMap;
	/**
	 * Type of map for storing persistent distributed and served arrays between SIAL programs.
	 * In parallel implementation., only used at servers
	 */
	typedef std::map<std::string, typename IdBlockMap<BLOCK_TYPE>::PerArrayMap*> LabelDistributedArrayMap;
	/**
	 * Type of map for storing persistent scalars between SIAL programs.  Only used at workers
	 */
	typedef std::map<std::string, double> LabelScalarValueMap;

	/**
	 * Type of map for storing the array id and slot of label for scalars and arrays that have been marked persistent.
	 */
	typedef std::map<int, int> ArrayIdLabelMap;	// Map of arrays marked for persistence

	PersistentArrayManager()  {
	}

	~PersistentArrayManager() {
		//TODO
	}

	/** Called to implement SIAL set_persistent command
	 *
	 * @param array_id
	 * @param string_slot
	 */
	void set_persistent(RUNNER_TYPE* runner, int array_id, int string_slot) {
//		std::cout << "set_persistent: array= " << runner->sip_tables()->array_name(array_id) << ", label=" << runner->sip_tables()->string_literal(string_slot) << std::endl;
		std::pair<ArrayIdLabelMap::iterator, bool> ret =
				persistent_array_map_.insert(
						std::pair<int, int>(array_id, string_slot));
		check(ret.second,
				"duplicate save of array in same sial program "
						+ array_name_value(array_id));
		//note that duplicate label for same type of object will
		//be detected during the save process so we don't
		//check for unique labels here.
	}


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
	void save_marked_arrays(RUNNER_TYPE* runner) {
		ArrayIdLabelMap::iterator it;
		for (it = persistent_array_map_.begin();
				it != persistent_array_map_.end(); ++it) {
			int array_id = it->first;
			int string_slot = it->second;
			//DEBUG
//			std::cout << "\nsave marked: array= " << runner->array_name(array_id) << ", label=" << runner->string_literal(string_slot) << std::endl;
			const std::string label = runner->sip_tables()->string_literal(string_slot);
			if (runner->sip_tables()->is_scalar(array_id)) {
				double value = runner->scalar_value(array_id);
//				std::cout << "saving scalar with label " << label << " value is " << value << std::endl;
				save_scalar(label, value);
			} else if (runner->sip_tables()->is_contiguous(array_id)) {
				Block* contiguous_array =
						runner->get_and_remove_contiguous_array(array_id);
//				std::cout << "saving contiguous array with label  "<<  label << " with contents "<< std::endl << *contiguous_array << std::endl;
				save_contiguous(label, contiguous_array);
			} else {
				//in parallel implementation, there won't be any of these on worker.
				typename IdBlockMap<BLOCK_TYPE>::PerArrayMap* per_array_map =
						runner->get_and_remove_per_array_map(array_id);
//				std::cout << " saving distributed array  with label " << label << " and map with " << per_array_map->size() << " blocks" << std::endl;
			    save_distributed(label, per_array_map);
			}
		}
		persistent_array_map_.clear();
	}

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
	void restore_persistent(RUNNER_TYPE* runner, int array_id, int string_slot){
		//DEBUG
		std::cout << "restore_persistent: array= " << runner->array_name(array_id) << ", label=" << runner->string_literal(string_slot) << std::endl;
		if (runner->sip_tables()->is_scalar(array_id))
			restore_persistent_scalar(runner, array_id, string_slot);
		else if (runner->sip_tables()->is_contiguous(array_id))
			restore_persistent_contiguous(runner, array_id, string_slot);
		else
			restore_persistent_distributed(runner, array_id, string_slot);
	}

	/** Invoked by restore_persistent to implement restore_persistent command in
	 * SIAl when the argument is a scalar.  The value associated with the
	 * string literal is copied into the scalar table and the entry removed
	 * from the persistent_array_manager.
	 *
	 * This will only be invoked by workers
	 *
	 * @param worker
	 * @param array_id
	 * @param string_slot
	 */
	void restore_persistent_scalar(RUNNER_TYPE* worker, int array_id,
			int string_slot) {
		std::string label = worker->sip_tables()->string_literal(string_slot);
		LabelScalarValueMap::iterator it = scalar_value_map_.find(label);
		check(it != scalar_value_map_.end(),
				"scalar to restore with label " + label + " not found");
		std::cout<< "restoring scalar " << worker -> array_name(array_id) << "=" << it -> second << std::endl;
		worker->set_scalar_value(array_id, it->second);
		scalar_value_map_.erase(it);
	}

	/** Invoked by restore_persistent to implement restore_persistent command in
	 * SIAl when the argument is a contiguous array.  The array associated
	 * with the string literal is MOVED into the contiguous array table and the
	 * entry removed (which does not delete the block) from the
	 * persistent_array_manager.
	 *
	 * This will only be invoked by workers
	 *
	 * @param worker
	 * @param array_id
	 * @param string_slot
	 */
	void restore_persistent_contiguous(RUNNER_TYPE* worker, int array_id,
			int string_slot) {
		std::string label = worker->sip_tables()->string_literal(string_slot);
		LabelContiguousArrayMap::iterator it = contiguous_array_map_.find(
				label);
		check(it != contiguous_array_map_.end(),
				"contiguous array to restore with label " + label
						+ " not found");
		worker->set_contiguous_array(array_id, it->second);
		contiguous_array_map_.erase(it);
	}


    /**	/** Invoked by restore_persistent to implement restore_persistent command in
	 * SIAl when the argument is a distributed/served array.  The block map associated
	 * with the string literal is MOVED into the distributed array table and the
	 * entry removed (which does not delete the map) from the persistent_array_manager.
	 *
	 * In a parallel instance of aces4, this will be called only on the server, in
	 * a single node instance, it will be called by the single process.
     *
     * @param runner
     * @param array_id
     * @param string_slot
     */
	void restore_persistent_distributed(RUNNER_TYPE* runner,
			int array_id, int string_slot) {
		std::string label = runner->sip_tables()->string_literal(string_slot);
		typename LabelDistributedArrayMap::iterator it = distributed_array_map_.find(
				label);
		check(it != distributed_array_map_.end(),
				"distributed/served array to restore with label " + label
						+ " not found");
		runner->set_per_array_map(array_id, it->second);
		distributed_array_map_.erase(it);
	}

	friend std::ostream& operator<< <>(std::ostream&, const PersistentArrayManager<BLOCK_TYPE, RUNNER_TYPE>&);

private:
	/** holder for saved contiguous arrays*/
	LabelContiguousArrayMap contiguous_array_map_;
	/** holder for saved distributed arrays*/
	LabelDistributedArrayMap distributed_array_map_;
	/** holder for saved scalar values */
	LabelScalarValueMap scalar_value_map_;
	/** holder for arrays and scalars that have been marked as persistent */
	ArrayIdLabelMap persistent_array_map_;

	/** inserts label value pair into map of saved values.
	 * warns if label has already been used. */

	void save_scalar(const std::string label, double value) {
		const std::pair<LabelScalarValueMap::iterator, bool> ret =
				scalar_value_map_.insert(
						std::pair<std::string, double>(label, value));
	if (!check_and_warn(ret.second, "Label " + label + "already used for scalar.  Overwriting previously saved value.")) {
		scalar_value_map_.erase(ret.first);
		scalar_value_map_.insert(std::pair<std::string,double>(label,value));
	}
}

/** inserts label, Block pair into map of saved contiguous arrays.
 * Warns if label has already been used.
 * Fatal error if block is null
 */
void save_contiguous(const std::string label, Block* contig) {
	check(contig != NULL,
			"attempting to save nonexistent contiguous array", current_line());
	const std::pair<LabelContiguousArrayMap::iterator, bool> ret =
			contiguous_array_map_.insert(
					std::pair<std::string, Block*>(label, contig));
	std::cout << "save_contiguous: ret= " << ret.second;
if (!check_and_warn(ret.second, "Label " + label + "already used for contiguous array.  Overwriting previously saved array.")) {
			contiguous_array_map_.erase(ret.first);
			contiguous_array_map_.insert(std::pair<std::string, Block*>(label, contig));
		}

	}
	/** inserts label, map pair into map of saved distributed arrays.
	 * Warns if label has already been used.
	 * Fatal error if map is null
	 */
	void save_distributed(const std::string label,
			typename IdBlockMap<BLOCK_TYPE>::PerArrayMap* map) {
		const std::pair<typename LabelDistributedArrayMap::iterator, bool> ret =
				distributed_array_map_.insert(
						std::pair<std::string,
								typename IdBlockMap<BLOCK_TYPE>::PerArrayMap*>(label,
								map));
	if (!check_and_warn(ret.second,
			"Label " + label + "already used for distributed/served array.  Overwriting previously saved array.")) {
				distributed_array_map_.erase(ret.first);
				distributed_array_map_.insert(std::pair<std::string, typename IdBlockMap<BLOCK_TYPE>::PerArrayMap*>(label,map));
	}
}



DISALLOW_COPY_AND_ASSIGN(PersistentArrayManager);

}
;

	template<typename BLOCK_TYPE, typename RUNNER_TYPE>
	std::ostream& operator<<(std::ostream& os, const PersistentArrayManager<BLOCK_TYPE, RUNNER_TYPE> & obj){
		os << "********PERSISTENT ARRAY MANAGER********" << std::endl;
		os << "Marked arrays: size=" << obj.persistent_array_map_.size() << std::endl;
		typename PersistentArrayManager<BLOCK_TYPE, RUNNER_TYPE>::ArrayIdLabelMap::const_iterator mit;
		for (mit = obj.persistent_array_map_.begin(); mit != obj.persistent_array_map_.end(); ++mit){
			os << mit -> first << ": " << mit -> second << std::endl;
		}
		os << "ScalarValueMap: size=" << obj.scalar_value_map_.size()<<std::endl;
		typename PersistentArrayManager<BLOCK_TYPE, RUNNER_TYPE>::LabelScalarValueMap::const_iterator it;
		for (it = obj.scalar_value_map_.begin(); it != obj.scalar_value_map_.end(); ++it){
			os << it->first << "=" << it->second << std::endl;
		}
		os << "ContiguousArrayMap: size=" << obj.contiguous_array_map_.size() << std::endl;
		typename PersistentArrayManager<BLOCK_TYPE, RUNNER_TYPE>::LabelContiguousArrayMap::const_iterator cit;
		for (cit = obj.contiguous_array_map_.begin(); cit != obj.contiguous_array_map_.end(); ++cit){
			os << cit -> first << std::endl;
		}
		os << "Distributed/ServedArrayMap: size=" << obj.distributed_array_map_.size() << std::endl;
		typename PersistentArrayManager<BLOCK_TYPE, RUNNER_TYPE>::LabelDistributedArrayMap::const_iterator dit;
		for (dit = obj.distributed_array_map_.begin(); dit != obj.distributed_array_map_.end(); ++dit){
			os << dit -> first << std::endl;
			//os << dit -> second << std::endl;
		}
		os<< "*********END OF PERSISTENT ARRAY MANAGER******" << std::endl;
		return os;
	}
} /* namespace sip */

//#include "persistent_array_manager.cpp"

#endif /* PERSISTENT_ARRAY_MANAGER_H_ */
