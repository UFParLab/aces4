/** ArrayTable.h  The classes defined here represent static information about arrays.
 *
 *
 *  Created on: Jun 5, 2013
 *      Author: Beverly Sanders
 */

#ifndef ARRAYDESCRIPTOR_H_
#define ARRAYDESCRIPTOR_H_


#include <map>
#include <vector>
#include <stdexcept>
#include "aces_defs.h"
#include "array_constants.h"
#include "io_utils.h"
#include "block_id.h"
//#include "blocks.h"


//namespace master{
//class SioxReader;
//}
namespace sip{
  class SioxReader;
  class SipTables;
  class DataManager;
  class IndexTable;



/**  An ArrayTableEntry describes an array declared in a SIAL program and contains its name, rank, type
 *   declared indices, and the index into the scalar table if the rank is zero and this entry is actually
 *   a scalar.
 */
class ArrayTableEntry {

public:
	/** Constructor
	 *
	 * @param name  name of array as given in the sial program
	 * @param rank
	 * @param array_type type of array as declared in the sial program.
	 * @param index_selectors  fixed size array holding the index table slots of the corresponding (SIAL) index used to define this array.
	 * @param scalarIndex  if this array is actually a scalar (array_type is scalar_value_t or perdefined_scalar_value_t and rank=0) then this is the
	 * 			slot in the scalar table holding the value.
	 */
	ArrayTableEntry(std::string name, int rank, ArrayType_t array_type,
			index_selector_t index_selectors, int scalarIndex);

	/** Constructor with no arguments.  Generally, an ArrayTableEntry constructed with this constructor will be initialized
	 *  by calling its init method.
	 */
	ArrayTableEntry();

	size_t block_number(const BlockId& id) const;


	BlockId id(int array_id, int block_number) const{
		index_value_array_t index_array;
		int i = MAX_RANK-1;
		for (i ;i >= rank_; i--){
			index_array[i] = unused_index_value;
		}
		int num = block_number;
		for (i; i >= 0; i--){
			int q = num/slice_sizes_[i];
			index_array[i] = q + lower_[i];
			num -= (q*slice_sizes_[i]);
		}
		return BlockId(array_id, index_array);
	}

private:
	/** initializes this ArrayTableEntry with values read from the given InputStream, which should be an .siox file
	 * at the appropriate position.  This is called by the ArrayTable init method */
	void init(setup::InputStream &);

	/**
	 * initializes values that are calculated using values in
	 * various tables read during input.  This requires that
	 * the sip_tables_ have already been read and partially
	 * initialized.
	 *
	 * @param sip_tables
	 */
	void init_calculated_values(const IndexTable& index_table);


	/** number of dimensions of the array */
	int rank_; // dimension of array

	/** declared type of the array. Possible values are found in sip.h */
	ArrayType_t array_type_;

	/** MAX_RANK sized array where each element contains the slot number in the index table of the corresponding SAIL index used to define this array.
	 * 			For example, if the array was declared as local A(lambda, mu) in the SIAL program, the first two elements would
	 * 			be the slots in the index table for lambda and mu, and the rest would have the value "unused_index_slot
	 */
	index_selector_t index_selectors_;

	/** if this array is a scalar, this field contains the index in to the scalar table holding the current value
	 * of this scalar.
	 */
	int scalar_selector_;

	/** these values are calculated using setup-reader info.  It is
	 * not read directly from the .siox file
	 */
	size_t num_blocks_;
	size_t max_block_size_;
	size_t min_block_size_;
	std::vector<long> slice_sizes_;
	std::vector<long> lower_;

	/** the name of the array in the SIAL program */
	std::string name_;

	friend std::ostream& operator<<(std::ostream&,  const ArrayTableEntry &);
	friend class ArrayTable;
	friend class sip::SioxReader;
	friend class sip::SipTables;
	friend class sip::DataManager;

//	DISALLOW_COPY_AND_ASSIGN(ArrayTableEntry);
};


class ArrayTable {
public:
	ArrayTable();
	~ArrayTable();

	/**Inlined function
	 *
	 * @param array_slot
	 * @return name of corresponding array
	 */
	std::string array_name(int array_slot) const {return entries_.at(array_slot).name_;}

	/**
	 * @param name
	 * @return slot number in array table of array with given name
	 */
    int array_slot(const std::string & name) const;

    /** Inlined function
     *
     * @param array_slot
     * @return declared type of array at given slot, possible values in array_constants.h
     */
	ArrayType_t array_type(int array_slot) const {return entries_.at(array_slot).array_type_;}

	/** Inlined function
	 *
	 * @param array_slot
	 * @return rank of array at given slot
	 */
	int rank(int array_slot) const {return entries_.at(array_slot).rank_;}

	/** Inlined function
	 *
	 * @param array_slot
	 * @return reference to selector array of array at given slot.  TODO  make return value const
	 */
	const index_selector_t &index_selectors(const int array_slot) const{return entries_.at(array_slot).index_selectors_;}

	/** Inlined function
	 *
	 * @param array_slot
	 * @return  slot in scalar table containing the value of the indicated rank 0 "array".
	 */
	int scalar_selector(int array_slot) const {return entries_.at(array_slot).scalar_selector_;}

	int id2number(const BlockId& id) const{
		return entries_[id.array_id()].block_number(id);
	}

	size_t num_blocks(int array_id) const {return entries_.at(array_id).num_blocks_;}

	BlockId number2id(int array_id, int num) const{
		return entries_[array_id].id(array_id, num);
	}

	friend std::ostream& operator<<(std::ostream&, const ArrayTable &);


private:
	/**
	 *  Initializes this ArrayTable by reading from the given InputStream, which should be a .siox file
	 *  at the correct position.  This method is called by the SioxReader.read_array_table method.
	 *
	 * @param .siox file
	 */
	void init(setup::InputStream &);

	/** Initializes the num_blocks_ field and max_block
	 * size field in all the ArrayTable entries.  It
	 * also initializes the values used to linearize BlockIds
	 * This method must be called after other required
	 * tables have been initialized since these values are calculated
	 * rather then being directly read from the input files
	 */
	void init_calculated_values(const IndexTable& index_table){
		std::vector<ArrayTableEntry>::iterator it= entries_.begin();
		for (; it != entries_.end(); ++it){
			it->init_calculated_values(index_table);
		}
	}


	/** Vector containing the ArrayTableEntries describing the arrays in the SIAL program */
	std::vector<ArrayTableEntry> entries_;

	/** A map from array name to corresponding slot in the array table*/
	std::map<std::string, int> array_name_slot_map_;  //maps name to selector in array table



    friend class sip::SioxReader;
    friend class sip::SipTables;
    friend class sip::DataManager;

	DISALLOW_COPY_AND_ASSIGN(ArrayTable);
};

} /*namespace sip*/

#endif /* ARRAYDESCRIPTOR_H_ */

