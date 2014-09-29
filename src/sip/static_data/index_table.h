/*! index_table.h  Contains various classes used to store static information about the
 * indices declared in a SIAL program
 *
 * SegmentDescriptor is an abstract class describing how indices of a particular type are divided into segments.
 * Subclasses represent different kinds of indexes.  A single instance of
 * the segment descriptor for each index types defined in SIAL (aoindex, etc.) is created during
 * setup with data provided during program setup.
 *
 * IndexTableEntry describes an index declared in a SIAL program.
 *
 * IndexTable is a vector containing an IndexTableEntry for each index declared in
 * the current SIAL program along with some utility methods.
 *
 *  Created on: Jun 5, 2013
 *      Author: Beverly Sanders
 */

#ifndef ARRAYS_INDEX_TABLE_H_
#define ARRAYS_INDEX_TABLE_H_

#include <vector>
#include <map>
#include <string>
#include <iostream>
#include "array_constants.h"
#include "io_utils.h"

namespace sip {
class IntTable;
class SialxInterpreter;
}
namespace setup {
class SetupReader;
}

namespace sip {

class IndexTable;
class IndexTableEntry;

/*Abstract base class for Segment descriptors.
 * Segment values are visible in SIAL programs and thus start at 1.
 *
 * Example:  an index is partitioned
 * into three segments (with elements numbered in C++ stye) as shown
 *
 *    0 1 2 3 ||  4 5 6 7 8 || 9 10 11
 *
 *   segment   extent  offset(beg_seg = 1)    offset(beg_seg = 2)   offset(beg_seg= 3)
 *    1          4  		0                      undef 					undef
 *    2          5  		4                       0						undef
 *    3          3  		9                       5                         0
 *
 *
 */
class SegmentDescriptor {
public:
	virtual ~SegmentDescriptor() {}
	/*! returns the number of elements in the indicated segment*/
	int extent(int segment) const {return get_extent(segment);}
	/*! returns the offset of segment relative to get_segment in the overall index.  Used for dealing with contiguous arrays
	 */
	int offset(int beg_segment, int segment) const {return get_offset(beg_segment,segment);}
	friend std::ostream& operator<<(std::ostream& os,
			const SegmentDescriptor &desc) {
		os << desc.to_string();
		return os;
	}
private:
	virtual std::string to_string() const=0;
	virtual	int get_extent(int segment) const=0;
	virtual int get_offset(int beg_segment,int segment) const=0;
};


/** For a simple index, the extent is always 1 */
class SimpleIndexSegmentDescriptor: public SegmentDescriptor {
public:
	virtual ~SimpleIndexSegmentDescriptor();
private:
	std::string  to_string() const;
	int get_extent(int segment) const;
	int get_offset(int beg_segment,int segment) const;
};

/** Segment descriptor for index types where the segment sizes are not uniform.
 *  The segment extents are obtained from the setup and explicitly stored in a vector
 */
class NonuniformSegmentDescriptor: public SegmentDescriptor {
public:
	explicit NonuniformSegmentDescriptor(std::vector<int>);
	virtual ~NonuniformSegmentDescriptor();
private:
	std::string  to_string() const;
	int get_extent(int segment) const;
	int get_offset(int beg_segment,int segment) const;
	std::vector<int> seg_extents_;
};

/* Segment descriptor for subindices.  In general, subsegments depend on the particular parent segment.
 * In this implementation, the slot of the parent index is stored, along with the number of subsegments
 * per parent segment.  Then the subsegment extents and offsets within the parent segment are computed
 * when required.
 *
 * Example:  this is the parent Segment Descriptor
 *     0 1 2 3 ||  4 5 6 7 8 || 9 10 11
 *  with num_subsegments = 2
 *
 *  parent_segment_value    segment_value    subsegment_extent   subsegment_offset
  		1                   	1 					2        			0
  		1						2					2					2
  		2						1					3					0
  		2						2					2					3
  		3						1					2					0
  		3						2					1					2

*  The get_extent method looks up the current value of the parent index from the data_manager.
*  It might be better to make this non-callable.
*/
class SubindexSegmentDescriptor: public SegmentDescriptor {
public:
	SubindexSegmentDescriptor(int parent_index_slot, int num_subsegments = DEFAULT_NUM_SUBSEGMENTS);
    virtual ~SubindexSegmentDescriptor();
    int subsegment_extent(int parent_segment, int segment){
    	return get_subsegment_extent(parent_segment, segment);
    }
    int subsegment_offset(int parent_segment_value, int segment_value){
    	return get_subsegment_offset(parent_segment_value, segment_value);
    }
    int num_subsegments(int parent_segment){
    	return get_num_subsegments(parent_segment);
    }
    friend class IndexTable; //TODO REMOVE THIS?
private:
	virtual std::string to_string() const;
	//TODO consider making get_extent unsupported
	virtual	int get_extent(int segment) const; //looks up parent's segment value, then calls subseg version.
	/*! get_offset is unsupported.  Use get_subsegment_offset */
	virtual int get_offset(int beg_segment, int segment) const; //looks up parent's segment value, then calls subseg version.
	virtual int get_subsegment_extent(int parent_segment, int segment);
    virtual int get_subsegment_offset(int parent_segment_value, int segment_value);
    virtual int get_num_subsegments(int parent_segment){return num_subsegments_;}
    int num_subsegments_;
    int parent_index_slot_;
};


class IndexTableEntry {
public:
	/*! /param index_type_t the index type as generated by the compiler
	 *  /param bseg  the first segment for this index, as given in the sial program, OR the parent index if this is a subindex
	 *  /param eseg  the last segment for this index, as given in the sial program, OR 0 if this is a subindex
	 *
	 *  We will actually store bseg and num_seg in the index table.
	 *
	 *  Note, bseg and eseg may be symbolic constants.
	 *
	 *  Conventions (which may not be applied completely consistently)
	 *     index = the thing defined in the sial program
	 *     selector, slot, or id = the slot in the index table associated with an index.  The
	 *         compiler assigns the slot number
	 *     value = the actual value of an index at a particular point in the computation, or -1 if
	 *        undefined
	 *
	 *     Invariant:  after the symbolic constants have been
	 *         initialized with their actual values, the value of any index must be in the
	 *         range defined for the index, i.e. bseg <= value < eseg
	 */
	IndexTableEntry();
	~IndexTableEntry();

	/** returns the number of elements in the given segment (i.e. value of index) */
	int segment_extent(int index_value) const;
	/** returns the total size of this dimension.  Used for contiguous (static) arrays */
    int index_extent() const;
	/** returns the parent index slot of this subindex.  Requires that this entry represent a subindex */
    int parent_index() const;

	/*! two indices are == if the types and sizes are the same.  (the name and segment table ptr not included)
	 * This operations is not yet supported for subindices.
	 */
	bool operator==(const IndexTableEntry& rhs) const;
	/*! if i < j then the range of is included in the range of j*/
	bool operator< (const IndexTableEntry& rhs) const;
	friend std::ostream& operator<<(std::ostream&, const IndexTableEntry &);

private:
	/*! initialize the given IndexTableEntry using data read from the InputStream (reading the siox file) and data from the IntTable maintained by the SetupReader class. */
   static void init(const std::string& name, IndexTableEntry&, IndexTable&, setup::InputStream& siox_file,
			setup::SetupReader&, sip::IntTable&);
//   /*! initializes symbolic values in this IndexTableEntry with values taken from the given IntTable */
//	void init_symbolic(sip::IntTable&);

	/*! name of index as declared in the sial program */
	std::string name_;

	/*! type of index.  Options are defined in array_constants.h */
	IndexType_t index_type_;

	/*! the lower range of segment values included for this index.  The range of a subindex is always 1..num_segments_
	 *  If this is a subindex, then variable is used for the parent
	 *  */
	int lower_seg_; //visible in sial

	/*! number of segments belonging to this index. */
	int num_segments_;  //number of segments

	/*! Pointer to a segment descriptor for the type of this segment
	 * This descriptor is owned by the segment_descriptor_map,
	 * which is responsible for deleting the descriptors
	 */
	SegmentDescriptor * segment_descriptor_ptr_;
	friend class SialxInterpreter;
	friend class IndexTable;
//	DISALLOW_COPY_AND_ASSIGN(IndexTableEntry);
};


/*! Class containing a vector of IndexTableEntrys, one for each index declared in the SIAL program.*/
class IndexTable {
public:
    typedef std::map<IndexType_t, SegmentDescriptor*> SegmentDescriptorMap;
	IndexTable();
	~IndexTable();

	/* initializes this IndexTable with data from the .siox file and setup. */
	void init(setup::InputStream&, setup::SetupReader&,
			sip::IntTable& int_table);

	/*!  returns the number of indices defined in the entire sial program */
	int num_indices() const {return entries_.size();}

	/*! convenience functions providing access to info from the IndexTableEntry selected by index_slot */
    int segment_extent (int index_slot, int index_value) const;
    int index_extent(int index_slot) const;
    int offset_into_contiguous(int index_slot, int index_value) const;
    int segment_range_extent(int index_slot, int lower_index_value, int upper_index_value) const;
    int offset_into_contiguous_region(int index_slot, int index_base, int index_value) const;
    int lower_seg(int index_slot) const;
    int num_segments(int index_slot) const;
    int index_id(std::string name) const;
    std::string index_name(int index_slot) const;
    IndexType_t index_type(int index_slot) const;


    /*! The next set of convenience functions are for subindices */

    bool is_subindex(int index_slot) const;
    /*! returns the index slot of the given subindex's parent */
    int parent(int subindex_slot) const;
    /*! returns the offset in the enlosing block of this segment */
    int subsegment_offset(int subindex_slot, int parent_segment_value, int subsegment_value) const;
    /*! returns the number of elements for this index in the subblock */
    int subsegment_extent(int subindex_slot, int parent_segment_value, int subsegment_value) const;
    /*! The number of subsegments in the parent segment of the given subindex */
    int num_subsegments(int subindex_slot, int parent_segment_value) const;

	friend std::ostream& operator<<(std::ostream&, const IndexTable &);


private:
	std::vector<IndexTableEntry> entries_;
	std::map<std::string, int> name_entry_map_;
	SegmentDescriptorMap segment_descriptors_; //initialized using info from setup
	friend class IndexTableEntry;
	friend class sip::SialxInterpreter;

	DISALLOW_COPY_AND_ASSIGN(IndexTable);
};

} /*namespace sip*/
#endif /* ARRAYS_INDEX_TABLE_H_ */
