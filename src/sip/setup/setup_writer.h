/*! Class for writing the symbolic constants, static arrays, and sial program names for aces
 *
 * Usage: (order of method invocations as ebnf grammar.  * means 0 or more)
 *      openWriter_f
 *      (writeSymbolicInt_f | writeStaticArrays_f | writeSialProgramName_f )*
 *      closeWriter_f
 *
 *
 *  Created on: Jun 23, 2013
 *      Author: Beverly Sanders
 *
 *
 *
 */

#ifndef SETUP_WRITER_H_
#define SETUP_WRITER_H_

#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <string>
#include "sip.h"
#include "io_utils.h"
#include "array_constants.h"

namespace setup {


class SetupWriter {
public:
    SetupWriter (std::string, OutputFile *);
    ~SetupWriter ();
//	void init_(const char * job_name);
    void write_header_file();
	void write_data_file();
	void addPredefinedIntHeader(std::string name, int val);
	void addPredefinedIntData(std::string name, int val);
	void addPredefinedContiguousArray(std::string name, int rank, int * dims, double * data);
	void addPredefinedIntegerContiguousArray(std::string name, int rank, int * dims, int * data);
	void addSialProgram(std::string name);
	void addSegmentInfo(sip::IndexType_t index_type_num, int num_segments, int * segment_sizes);
	void addPredefinedScalar(std::string name, double value);
	void addSialFileConfigInfo(std::string sialfile, std::string key, std::string value);
	typedef std::map<std::string, int> PredefInt;
	typedef std::map<std::string, double> PredefScalar;
	typedef std::map<int, std::pair<int, int *> > SegSizeArray;
	typedef SegSizeArray::iterator SegSizeArrayIterator;
	typedef std::vector<std::string> SialProg;
	typedef std::map<std::string, std::pair<int, std::pair<int *, double *> > > PredefArrMap;
	typedef PredefArrMap::iterator PredefArrayIterator;
	typedef std::map<std::string, std::pair<int, std::pair<int *, int *> > > PredefIntArrMap;
	typedef PredefIntArrMap::iterator PredefIntArrayIterator;
	typedef std::map<std::string, std::string> KeyValueMap;
	typedef std::map<std::string, KeyValueMap > FileConfigMap;
private:
	std::string jobname_;
	OutputFile * file;
	PredefInt header_constants_;  //predefined ints to be defined in header file
	PredefInt data_constants_;  //predefined ints to be read from data file
	PredefScalar scalars_;  //predefined scalars
	SialProg sial_programs_;   //list of sial program to execute
	SegSizeArray segments_;  //segment sizes for each predefined index type
	PredefArrMap predef_arr_; 	// Map of predefined arrays
	PredefIntArrMap predef_int_arr_; 	// Map of predefined integer arrays
	FileConfigMap configs_;		// Map of sial files to their configurations in the form of a key-value map
	DISALLOW_COPY_AND_ASSIGN(SetupWriter);
};


} /* namespace setup */

#endif /* SETUP_WRITER_H_ */
