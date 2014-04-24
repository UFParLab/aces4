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
#include "aces_defs.h"
#include "array_constants.h"
#include "io_utils.h"
//#include "blocks.h"

//namespace master{
//class SioxReader;
//}
namespace sip{
  class SioxReader;
  class SipTables;
  class DataManager;
}
namespace sip {


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

private:
	/** initializes this ArrayTableEntry with values read from the given InputStream, which should be an .siox file
	 * at the appropriate position.  This is called by the ArrayTable init method */
	void init(setup::InputStream &);



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

	/** this value is calculated using setup-reader info.  It is
	 * not read directly from the .siox file
	 */
	//int num_blocks_;

	/** the name of the array in the SIAL program */
	std::string name_;

	friend std::ostream& operator<<(std::ostream&, const ArrayTableEntry &);
	friend class ArrayTable;
	friend class sip::SioxReader;
	friend class sip::SipTables;
	friend class sip::DataManager;
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

	/** Inlined function
	 *
	 * @param name
	 * @return slot number in array table of array with given name
	 */
    int array_slot(const std::string & name) const {return array_name_slot_map_.at(name);}

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

	friend std::ostream& operator<<(std::ostream&, const ArrayTable &);


private:
	/**
	 *  Initializes this ArrayTable by reading from the given InputStream, which should be a .siox file
	 *  at the correct position.  This method is called by the SioxReader.read_array_table method.
	 *
	 * @param .siox file
	 */
	void init(setup::InputStream &);

	/** Initializes the num_blocks_ field in all the ArrayTable entries.
	 * This is called by the siptable constructor after other required
	 * tables have been initialized.
	 */
	void init_num_blocks();

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

