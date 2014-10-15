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

	virtual int predefined_int(const std::string&) = 0;
	virtual double predefined_scalar(const std::string&) = 0;
	virtual PredefContigArray predefined_contiguous_array(const std::string&) = 0;
	virtual PredefIntArray predefined_integer_array(const std::string&) = 0;
	virtual int num_segments(sip::IndexType_t) = 0;

	// Accessor Functions
	virtual const SialProgList& sial_prog_list() = 0;
	virtual const PredefIntMap predefined_int_map() = 0;
	virtual const PredefScalarMap predefined_scalar_map() = 0;
	virtual const SetupSegmentInfoMap segment_map() = 0;
	virtual NamePredefinedContiguousArrayMap name_to_predefined_contiguous_array_map() = 0;
	virtual const PredefIntArrMap predef_int_arr() = 0;
	virtual const FileConfigMap configs() = 0;

	/** performs sanity checks on input
	 * right now, this just checks for the existence of a sial program name
	 */
	virtual bool aces_validate() = 0;

protected:

	DISALLOW_COPY_AND_ASSIGN(SetupReader);

};

} /* namespace setup */

#endif /* SETUP_READER_H_ */
