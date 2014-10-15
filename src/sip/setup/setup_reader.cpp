/*
 * setup_reader.cpp
 *
 *  Created on: Oct 15, 2014
 *      Author: njindal
 */

#include "setup_reader.h"
#include <stdexcept>

namespace setup {
SetupReader::SetupReader() {}

SetupReader::~SetupReader() {}

int SetupReader::predefined_int(const std::string& name) {
	PredefIntMap::iterator it = predefined_int_map_.find(name);
	if (it == predefined_int_map_.end()){
		throw std::out_of_range("Could not find predefined integer : " + name);
	}
	return it->second;
}

double SetupReader::predefined_scalar(const std::string& name) {
	PredefScalarMap::iterator it = predefined_scalar_map_.find(name);
	if (it == predefined_scalar_map_.end()){
		throw std::out_of_range("Could not find predefined scalar : " + name);
	}
	return it->second;
}

PredefContigArray SetupReader::predefined_contiguous_array(const std::string& name){
	NamePredefinedContiguousArrayMap::iterator it = name_to_predefined_contiguous_array_map_.find(name);
	if (it == name_to_predefined_contiguous_array_map_.end()){
		throw std::out_of_range("Could not find predefined contiguous array : " + name);
	}
	return it->second;
}

PredefIntArray SetupReader::predefined_integer_array(const std::string& name){
	PredefIntArrMap::iterator it = predef_int_arr_.find(name);
	if (it == predef_int_arr_.end()){
		throw std::out_of_range("Could not find predefined integer array : " + name);
	}
	return it->second;
}


} /* namespace setup */
