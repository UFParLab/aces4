/*
 * setup_reader.h
 *
 *  Created on: Oct 15, 2014
 *      Author: njindal
 */

#ifndef SETUP_READER_H_
#define SETUP_READER_H_

#include <map>
#include <string>
#include <utility>
#include "block.h"

namespace setup {

/** Encapsulates all needed data for predefined integer arrays */
typedef struct { int rank; int* dims; int* data; } PredefIntArray;

/** Encapsulates all needed data for predefined double arrays */
// typedef struct { int rank; int* dims; double* data; } PredefDoubleArray; // Not used

/** Encapsulates all needed data for predefined contiguous arrays */
typedef std::pair<int, sip::Block::BlockPtr> PredefContigArray;


class SetupReader {
public:

	SetupReader();
	virtual ~SetupReader();

	typedef std::map<std::string, int> PredefIntMap;
	typedef std::map<std::string, double> PredefScalarMap;
	typedef std::vector<int> SegmentDescriptor;
	typedef std::map<sip::IndexType_t, std::vector<int> > SetupSegmentInfoMap;
	typedef std::vector<std::string> SialProgList;
	typedef std::map<std::string, PredefIntArray > PredefIntArrMap;
	typedef PredefIntArrMap::iterator PredefIntArrayIterator;
	typedef std::map<std::string, std::string> KeyValueMap;
	typedef std::map<std::string, KeyValueMap > FileConfigMap;
	typedef std::map<std::string, PredefContigArray > NamePredefinedContiguousArrayMap;
    typedef std::map<std::string, PredefContigArray >::iterator NamePredefinedContiguousArrayMapIterator;
    typedef std::map<std::string, PredefContigArray >::const_iterator NamePredefinedContiguousArrayMapConstIterator;

	int predefined_int(const std::string&);
	double predefined_scalar(const std::string&);
	PredefContigArray predefined_contiguous_array(const std::string&) ;
	PredefIntArray predefined_integer_array(const std::string&);
	int num_segments(sip::IndexType_t) { sip::fail("num_segments not implemented !"); }

	// Accessor Functions
	const SialProgList& sial_prog_list() { return sial_prog_list_; }
	const PredefIntMap& predefined_int_map() { return predefined_int_map_; }
	const PredefScalarMap& predefined_scalar_map() { return predefined_scalar_map_; }
	const SetupSegmentInfoMap& segment_map() { return segment_map_; }
	NamePredefinedContiguousArrayMap& name_to_predefined_contiguous_array_map() { return name_to_predefined_contiguous_array_map_; }
	const PredefIntArrMap& predef_int_arr() { return predef_int_arr_; }
	const FileConfigMap& configs() { return configs_; }


	/** performs sanity checks on input
	 * right now, this just checks for the existence of a sial program name
	 */
	virtual bool aces_validate() = 0;

protected:

	SialProgList sial_prog_list_;
	PredefIntMap predefined_int_map_;
	PredefScalarMap predefined_scalar_map_;
	SetupSegmentInfoMap segment_map_;
	NamePredefinedContiguousArrayMap name_to_predefined_contiguous_array_map_; // Map of predefined scalar arrays
	PredefIntArrMap predef_int_arr_; 	// Map of predefined integer arrays
	FileConfigMap configs_;		// Map of sial files to their configurations in the form of a key-value map


	DISALLOW_COPY_AND_ASSIGN(SetupReader);

};

} /* namespace setup */

#endif /* SETUP_READER_H_ */
